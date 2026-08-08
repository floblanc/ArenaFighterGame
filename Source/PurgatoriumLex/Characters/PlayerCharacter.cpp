


#include "PlayerCharacter.h"
#include "AbilitySystem/PurgatoriumLexAbilitySystemComponent.h"
#include "AbilitySystem/PurgatoriumLexAttributeSet.h"
#include "Player/PurgatoriumLexPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "UI/PurgatoriumLexHUD.h"
#include "Input/PurgatoriumLexInputComponent.h"
#include "Input/PurgatoriumLexInputConfig.h"
#include "PurgatoriumLexGameplayTags.h"
#include "Combat/FighterCombatComponent.h"
#include "Combat/LightAttackMoveSet.h"
#include "Characters/LockOnCameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

// #include "PurgatoriumLexMacros.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false; // lock camera behind character if true, but never use it because bUseControllerDesiredRotation and bOrientRotationToMovement are used instead.
	bUseControllerRotationRoll = false;

	bIsRunning = false;
	WalkingSpeed = 400.f;
	RunningSpeed = 800.f;

	// Posture defaults (avoid calling RequestNeutralPosture() here because it relies on initialized state)
	ActualPosture = EPosture::E_Neutral;
	bIsPostureActionActive = false;

	playerHealth = 1.00f;
	bAttackHasBeenUsed = false;
	bIsRigthPunchHitboxActive = false;
	bIsInAttackAnimation = false;
	bIsCharging = false;
	maxInputHoldTime = 3.5f;
	ChargeAttackStartTime = 0.f;
	MinChargeTime = 0.2f;
	SimulationFrame = 0;
	
	// Roll staling defaults
	RollMinPenalty = 0.06f;
	RollMaxPenalty = 0.1f;
	RollMaxPenaltyValue = 0.5f;
	RollStalePenalty = 0.0f;
	LastDodgeFrame = -1;
	RollResetFrames = 60; // ~1 second at 60fps

	// Tech system defaults (SSBU-style)
	TechWindowFrames = 11;
	TechLockoutFrames = 40;
	TechKnockbackThreshold = 6.0f;
	TechWindowStartFrame = -1;
	LastTechInputFrame = -1;
	bIsTechInputHeld = false;
	bIsJumpInputHeld = false;
	bIsTechable = false;
	LastWallHitNormal = FVector::ZeroVector;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f; // TODO: Ajust the value to something not so permissive but still worth for DI
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// create the orbiting camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Fixed-tick combat sim (posture + light attack). Tunable on the component.
	FighterCombat = CreateDefaultSubobject<UFighterCombatComponent>(TEXT("FighterCombat"));
	bAutoPlaySimAttackMontage = true;

	// Lock-on / camera-on-back (presentation). Tune distance / height on this component.
	LockOnCamera = CreateDefaultSubobject<ULockOnCameraComponent>(TEXT("LockOnCamera"));

	// set the player tag
	Tags.Add(FName("Player"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// TEMPORARY: Controlled character gets team from player index (BeginPlay may have run before we had PlayerState).
	if (APlayerState* PS = GetPlayerState())
	{
		SetTeamId(PS->GetPlayerId() % 2);
	}

	InitAbilitySystemComponent();
	GiveDefaultAbilities();
	GrantAbilitiesWithInputTags();
	InitDefaultAttributes();
	InitHUD();

	BP_TryInitFloatingHealthBar();
}

void APlayerCharacter::GrantAbilitiesWithInputTags()
{
	UPurgatoriumLexAbilitySystemComponent* LexASC = Cast<UPurgatoriumLexAbilitySystemComponent>(AbilitySystemComponent);
	if (!LexASC || !HasAuthority()) return;

	for (const FAbilityInputMapping& Mapping : AbilityInputMappings)
	{
		if (!Mapping.AbilityClass || !Mapping.InputTag.IsValid()) continue;

		LexASC->GrantAbilityWithInputTag(Mapping.AbilityClass, Mapping.InputTag, 1);
	}
}

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilitySystemComponent();
	InitDefaultAttributes();
	InitHUD();

	BP_TryInitFloatingHealthBar();
}

void APlayerCharacter::InitAbilitySystemComponent()
{
	APurgatoriumLexPlayerState* PurgatoriumLexPlayerState = GetPlayerState<APurgatoriumLexPlayerState>();
	check(PurgatoriumLexPlayerState);
	AbilitySystemComponent = CastChecked<UPurgatoriumLexAbilitySystemComponent>(PurgatoriumLexPlayerState->GetAbilitySystemComponent());
	AbilitySystemComponent->InitAbilityActorInfo(PurgatoriumLexPlayerState, this);
	AttributeSet = PurgatoriumLexPlayerState->GetAttributeSet();
}

void APlayerCharacter::InitHUD() const
{
	UE_LOG(LogTemp, Warning, TEXT("InitHUD called for %s"), *GetName());
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (APurgatoriumLexHUD* PurgatoriumLexHUD = Cast<APurgatoriumLexHUD>(PlayerController->GetHUD()))
		{
			PurgatoriumLexHUD->Init();
		}
	}
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Default bufferable tags if none set in Blueprint (Roll only — LightAttack is sim-owned)
	using namespace PurgatoriumLexGameplayTags;
	if (BufferableInputTags.Num() == 0)
	{
		BufferableInputTags.Add(InputTag_Roll);
	}

	// ========== TEMPORARY: Team assignation. Remove when teams come from lobby/champ select. ==========
	if (APlayerState* PS = GetPlayerState())
	{
		SetTeamId(PS->GetPlayerId() % 2);
	}
	else
	{
		SetTeamId(1); // Unpossessed = enemy team for lock-on
	}
	// ========== END TEMPORARY ==========

	if (FighterCombat)
	{
		FighterCombat->OnLightAttackStarted.AddDynamic(this, &APlayerCharacter::HandleSimLightAttackStarted);
		FighterCombat->ApplyConfigToSim();
	}

	if (LockOnCamera)
	{
		LockOnCamera->OnLockOnEnemyChanged.AddDynamic(this, &APlayerCharacter::HandleLockOnEnemyChanged);
		LockOnCamera->OnLockOnBackChanged.AddDynamic(this, &APlayerCharacter::HandleLockOnBackChanged);
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Input buffer: client-side feel for remaining GAS-era tags (e.g. Roll) — not combat authority
	SimulationFrame++;

	if (FighterCombat)
	{
		SyncPresentationFromCombatSim();
	}
	
	// Process roll staling reset (reset penalty after RollResetFrames without dodging)
	if (RollStalePenalty > 0.0f && LastDodgeFrame >= 0)
	{
		const int32 FramesSinceLastDodge = SimulationFrame - LastDodgeFrame;
		
		if (FramesSinceLastDodge >= RollResetFrames)
		{
			UE_LOG(LogTemp, Log, TEXT("[Roll Staling] Penalty reset (%d frames since last dodge, frame %d)"), FramesSinceLastDodge, SimulationFrame);
			RollStalePenalty = 0.0f;
			LastDodgeFrame = -1;
		}
	}
	
	// Remove expired entries
	int32 ExpiredCount = 0;
	AbilityInputBuffer.RemoveAll([this, &ExpiredCount](const FBufferedAbilityInput& E) {
		const int32 Remaining = E.GetFramesRemaining(SimulationFrame, AbilityInputBufferFrames);
		const bool bExpired = Remaining <= 0;
		if (bExpired)
		{
			ExpiredCount++;
			UE_LOG(LogTemp, Log, TEXT("[Input Buffer] Entry expired: %s (was buffered at frame %d, current frame %d)"), 
				*E.InputTag.ToString(), E.BufferedFrame, SimulationFrame);
		}
		return bExpired;
	});
	
	// Try to consume buffer entries (retry failed activations)
	UPurgatoriumLexAbilitySystemComponent* ASC = Cast<UPurgatoriumLexAbilitySystemComponent>(GetAbilitySystemComponent());
	if (ASC && AbilityInputBuffer.Num() > 0)
	{
		for (int32 i = AbilityInputBuffer.Num() - 1; i >= 0; --i)
		{
			const FBufferedAbilityInput& Entry = AbilityInputBuffer[i];
			const int32 Remaining = Entry.GetFramesRemaining(SimulationFrame, AbilityInputBufferFrames);
			
			if (CanActivateAbilityForInputTag(Entry.InputTag))
			{
				if (ASC->TryActivateAbilitiesByInputTag(Entry.InputTag))
				{
					UE_LOG(LogTemp, Warning, TEXT("[Input Buffer] ✅ CONSUMED: %s activated from buffer (was buffered at frame %d, consumed at frame %d, %d frames remaining)"), 
						*Entry.InputTag.ToString(), Entry.BufferedFrame, SimulationFrame, Remaining);
					AbilityInputBuffer.RemoveAt(i);  // Success: remove from buffer
				}
				else
				{
					UE_LOG(LogTemp, VeryVerbose, TEXT("[Input Buffer] Retrying %s: gate passed but ASC didn't activate (frame %d, %d frames remaining)"), 
						*Entry.InputTag.ToString(), SimulationFrame, Remaining);
				}
			}
			else
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("[Input Buffer] Retrying %s: gate still blocked (frame %d, %d frames remaining)"), 
					*Entry.InputTag.ToString(), SimulationFrame, Remaining);
			}
		}
	}

	// Update techable state (check if character should be able to tech)
	UpdateTechableState();

	// Process tech window expiration (frame-based for rollback compatibility)
	if (TechWindowStartFrame >= 0)
	{
		const int32 FramesElapsed = SimulationFrame - TechWindowStartFrame;
		if (FramesElapsed >= TechWindowFrames)
		{
			// Tech window expired without tech being performed
			EndTechWindow();
		}
	}

	// NOTE: Removed predictive tech window start (WillHitWallSoon) for rollback compatibility
	// Predictive line traces can be non-deterministic between clients during rollback.
	// Tech windows are now started only when actual contact occurs (Landed/NotifyHit callbacks).
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UE_LOG(LogTemp, Warning, TEXT("[Input Setup] Starting input component setup for %s"), *GetNameSafe(this));

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogTemp, Warning, TEXT("[Input Setup] Added DefaultMappingContext: %s"), DefaultMappingContext ? *DefaultMappingContext->GetName() : TEXT("null"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Input Setup] DefaultMappingContext is NULL! Input will not work. Please assign an Input Mapping Context in the character blueprint."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Input Setup] Failed to get EnhancedInputLocalPlayerSubsystem! Enhanced Input may not be enabled."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Input Setup] Failed to get PlayerController! Input setup cannot continue."));
		return;
	}

	// Cast to our custom input component (required for InputConfig/tag bindings)
	UPurgatoriumLexInputComponent* PurgatoriumLexInputComponent = Cast<UPurgatoriumLexInputComponent>(PlayerInputComponent);
	if (!PurgatoriumLexInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[Input Setup] '%s' Failed to find PurgatoriumLexInputComponent! Current InputComponent type: %s. Please ensure the InputComponent is set to PurgatoriumLexInputComponent in the character blueprint."), 
			*GetNameSafe(this), *GetNameSafe(PlayerInputComponent->GetClass()));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[Input Setup] Successfully cast to PurgatoriumLexInputComponent"));

	// New-way only: InputConfig is mandatory
	if (!InputConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("[Input Setup] '%s' InputConfig is not set. Please assign a UPurgatoriumLexInputConfig on the character blueprint."), *GetNameSafe(this));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[Input Setup] InputConfig found: %s"), *GetNameSafe(InputConfig));

	// Log InputConfig contents for debugging
	if (InputConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Input Setup] NativeInputActions count: %d"), InputConfig->NativeInputActions.Num());
		UE_LOG(LogTemp, Warning, TEXT("[Input Setup] AbilityInputActions count: %d"), InputConfig->AbilityInputActions.Num());
	}

	// Bind ability actions (InputAction -> InputTag), then route to ASC via Input_AbilityInputTagPressed/Released.
	PurgatoriumLexInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ AbilityInputBindHandles);
	UE_LOG(LogTemp, Warning, TEXT("[Input Setup] Bound %d ability actions"), AbilityInputBindHandles.Num());

	// Bind native (non-ability) actions via tags.
	int32 NativeBindCount = 0;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Move, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Move, ETriggerEvent::Completed, this, &ThisClass::MoveActionStopped, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ThisClass::Look, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Jump, ETriggerEvent::Triggered, this, &ThisClass::DoJumpStart, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Jump, ETriggerEvent::Completed, this, &ThisClass::DoJumpEnd, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Run, ETriggerEvent::Triggered, this, &ThisClass::StartRunning, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Roll, ETriggerEvent::Started, this, &ThisClass::Roll, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Posture, ETriggerEvent::Triggered, this, &ThisClass::PostureActionTriggered, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Posture, ETriggerEvent::Completed, this, &ThisClass::PostureActionStopped, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_LockUnlock, ETriggerEvent::Started, this, &ThisClass::LockUnlockCameraOnEnemy, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	
	UE_LOG(LogTemp, Warning, TEXT("[Input Setup] Bound %d native actions"), NativeBindCount);
	UE_LOG(LogTemp, Warning, TEXT("[Input Setup] Input setup complete for %s"), *GetNameSafe(this));
}


void APlayerCharacter::Move(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Native): Move"));
	bIsMoving = ((!(bIsInAttackAnimation)) || GetCharacterMovement()->IsFalling()) && !bIsCharging;

	// Change Posture by default movement
	if (IsCameraLockedOnCharacterBack() && bIsMoving && (bIsPostureActionActive == false))
	{
		ProcessPostureInput(Value);
		UE_LOG(LogTemp, Warning, TEXT("ChangeDefault posture"));
	}
	
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr && bIsMoving)
	{
		if (IsCameraLockedOnEnemy())
		{
			// route the input
			DoMoveAroundSomething(MovementVector.X, MovementVector.Y);
		}
		else
		{
			// route the input
			DoMove(MovementVector.X, MovementVector.Y);
		}
	}
}

void APlayerCharacter::DoMoveAroundSomething(float Right, float Forward)
{
	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// get right vector 
	FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	double TimeUnitToDiviceVelocity = 5.5; // arbitraire mais 5.25 (6.0 Max ?? Min)semble idéal pour velocity 400/800
	double originalXStepSize = Right * GetCharacterMovement()->Velocity.Size() / TimeUnitToDiviceVelocity;
	UE_LOG(LogTemp, Warning, TEXT("originalXStepSize : %f"), originalXStepSize);

	if (originalXStepSize != 0.0)
	{
		// Get the maximum physics substep delta time.
		AActor* LockedActor = GetLockedOnActor();
		if (!LockedActor)
		{
			AddMovementInput(ForwardDirection, Forward);
			AddMovementInput(RightDirection, Right);
			return;
		}
		
		double distance = (LockedActor->GetActorLocation() - GetActorLocation()).Size(); // entre 70-100 et 1000-1500 environ -> 70 = collé, 100 = très proche
		UE_LOG(LogTemp, Warning, TEXT("distance : %f"), distance);
		UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed : %f"), GetCharacterMovement()->MaxWalkSpeed);
		UE_LOG(LogTemp, Warning, TEXT("Right: %f"), Right);
		// double originalXStepSize = (GetCharacterMovement()->GetLastUpdateLocation() - GetActorLocation()).Size();
		UE_LOG(LogTemp, Warning, TEXT("originalXStepSize : %f"), originalXStepSize);
		double radianTargetAngle = originalXStepSize / distance;
		UE_LOG(LogTemp, Warning, TEXT("radianTargetAngle: %f"), radianTargetAngle);
		double angle = FMath::RadiansToDegrees(radianTargetAngle / 2.0);
		UE_LOG(LogTemp, Warning, TEXT("angle : %f"), angle);
		double newStepSize = FMath::Sin(radianTargetAngle / 2.0) * distance * 2.0;
		UE_LOG(LogTemp, Warning, TEXT("newStepSize: %f"), newStepSize);
		
		Right = Right * newStepSize / originalXStepSize;

		UE_LOG(LogTemp, Warning, TEXT("Right: %f"), Right);
		UE_LOG(LogTemp, Warning, TEXT("Forward: %f"), Forward);
		
		angle *= -1.0;

		UE_LOG(LogTemp, Warning, TEXT("angle : %f"), angle);

		UE_LOG(LogTemp, Warning, TEXT("Angle value: %f"), angle);
		UE_LOG(LogTemp, Warning, TEXT("Distance from lockedEnemy : %f\n"), distance);

		RightDirection = RightDirection.RotateAngleAxis(angle, FVector::ZAxisVector);
	}

	UE_LOG(LogTemp, Warning, TEXT("Right: %f"), Right);
	UE_LOG(LogTemp, Warning, TEXT("Forward: %f"), Forward);
	// add movement 
	AddMovementInput(ForwardDirection, Forward);
	AddMovementInput(RightDirection, Right);
	FVector MovementVec = GetPendingMovementInputVector();
	UE_LOG(LogTemp, Warning, TEXT("IN MOVE -- Pending Input Vector : (%f, %f, %f)"), MovementVec.X, MovementVec.Y, MovementVec.Z);

	MovementVec = GetCharacterMovement()->GetLastInputVector();;
	UE_LOG(LogTemp, Warning, TEXT("IN MOVE -- Last Input Vector : (%f, %f, %f)"), MovementVec.X, MovementVec.Y, MovementVec.Z);

}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Native): Look"));
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void APlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void APlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void APlayerCharacter::DoJumpStart()
{
	bIsJumpInputHeld = true;
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Native): Jump"));
	// signal the character to jump
	if (!bAttackHasBeenUsed && !bIsInAttackAnimation)
	{
		Jump();
	}
}

void APlayerCharacter::DoJumpEnd()
{
	bIsJumpInputHeld = false;
	UE_LOG(LogTemp, Log, TEXT("[Input] Released (Native): Jump"));
	// signal the character to stop jumping
	StopJumping();
}

void APlayerCharacter::StartRunning()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Native): Run"));
	if (LockOnCamera)
	{
		LockOnCamera->UnlockFromCharacterBack();
	}

	// Check if character is already running
	bIsRunning = true;

	GetCharacterMovement()->MaxWalkSpeed = RunningSpeed; // Set running speed
}

void APlayerCharacter::StopRunning()
{
	if (LockOnCamera && LockOnCamera->IsLockedOnEnemy())
	{
		LockOnCamera->LockToCharacterBack();
	}

	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed; // Reset to walking speed
}

void APlayerCharacter::Roll()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Native): Roll"));
	
	// Calculate roll direction based on movement input or character facing
	if (Controller != nullptr)
	{
		// Get movement input direction
		FVector MovementInput = GetLastMovementInputVector();
		if (MovementInput.SizeSquared() > 0.01f)
		{
			// Normalize and get forward component relative to character facing
			MovementInput.Normalize();
			FVector ForwardVector = GetActorForwardVector();
			float ForwardDot = FVector::DotProduct(MovementInput, ForwardVector);
			
			// Clamp ForwardDot to [-1, 1] range
			ForwardDot = FMath::Clamp(ForwardDot, -1.0f, 1.0f);
			
			// Calculate penalty increment based on direction
			// ForwardDot: 1.0 = pure forward (min penalty), -1.0 = pure backward (max penalty)
			float PenaltyIncrement = CalculateRollPenaltyIncrement(ForwardDot);
			
			// Accumulate penalty
			RollStalePenalty = FMath::Min(RollStalePenalty + PenaltyIncrement, RollMaxPenaltyValue);
			
			// Update last dodge frame (frame-based for rollback compatibility)
			LastDodgeFrame = SimulationFrame;
			
			UE_LOG(LogTemp, Log, TEXT("[Roll Staling] Roll executed - ForwardDot: %.2f, PenaltyIncrement: %.3f, TotalPenalty: %.3f, DurationMultiplier: %.3f, IntangibilityDelay: %d"), 
				ForwardDot, PenaltyIncrement, RollStalePenalty, GetRollDurationMultiplier(), GetRollIntangibilityDelay());
		}
		else
		{
			// No movement input - assume neutral roll (use average penalty)
			float AveragePenalty = (RollMinPenalty + RollMaxPenalty) * 0.5f;
			RollStalePenalty = FMath::Min(RollStalePenalty + AveragePenalty, RollMaxPenaltyValue);
			LastDodgeFrame = SimulationFrame;
			
			UE_LOG(LogTemp, Log, TEXT("[Roll Staling] Roll executed (no movement input) - AveragePenalty: %.3f, TotalPenalty: %.3f, DurationMultiplier: %.3f"), 
				AveragePenalty, RollStalePenalty, GetRollDurationMultiplier());
		}
	}
	
	// TODO: implement roll movement/animation or bind to ability
	// The staling system is ready - apply GetRollDurationMultiplier() to roll duration
	// and GetRollIntangibilityDelay() to intangibility start frame
}

float APlayerCharacter::CalculateRollPenaltyIncrement(float ForwardVectorDot) const
{
	// ForwardVectorDot: 1.0 = pure forward (min penalty), -1.0 = pure backward (max penalty)
	// Map from [1, -1] to [MinPenalty, MaxPenalty]
	// Using linear interpolation: when ForwardDot = 1 -> MinPenalty, when ForwardDot = -1 -> MaxPenalty
	
	// Normalize ForwardDot from [1, -1] to [0, 1] where 0 = forward, 1 = backward
	float NormalizedDot = (1.0f - ForwardVectorDot) * 0.5f;
	
	// Interpolate between MinPenalty and MaxPenalty
	float PenaltyIncrement = FMath::Lerp(RollMinPenalty, RollMaxPenalty, NormalizedDot);
	
	return PenaltyIncrement;
}

int32 APlayerCharacter::GetRollIntangibilityDelay() const
{
	// When fully stale (penalty >= max), delay is 4 frames
	// Linearly interpolate from 0 (fresh) to 4 (fully stale)
	if (RollStalePenalty <= 0.0f)
	{
		return 0;
	}
	
	// Calculate delay based on penalty ratio (0.0 to 1.0)
	float PenaltyRatio = FMath::Clamp(RollStalePenalty / RollMaxPenaltyValue, 0.0f, 1.0f);
	int32 Delay = FMath::RoundToInt(PenaltyRatio * 4.0f);
	
	return Delay;
}

int32 APlayerCharacter::GetPostureIntangibilityDelay() const
{
	if (!FighterCombat)
	{
		return 0;
	}

	const FFighterSimState SimState = FighterCombat->GetSimStateCopy();
	const float Penalty = SimState.PostureStalePenalty;
	const float MaxPenalty = FighterCombat->PostureMaxPenalty;

	if (Penalty <= 0.0f || MaxPenalty <= 0.0f)
	{
		return 0;
	}

	const float PenaltyRatio = FMath::Clamp(Penalty / MaxPenalty, 0.0f, 1.0f);
	return FMath::RoundToInt(PenaltyRatio * 4.0f);
}

float APlayerCharacter::GetPostureDurationMultiplier() const
{
	if (!FighterCombat)
	{
		return 1.0f;
	}
	return 1.0f + FighterCombat->GetSimStateCopy().PostureStalePenalty;
}

void APlayerCharacter::PostureActionTriggered(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Native): Posture"));
	if (IsCameraLockedOnCharacterBack())
	{
		bIsPostureActionActive = true;
		ProcessPostureInput(Value);
	}
}

void APlayerCharacter::ProcessPostureInput(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller == nullptr || bIsCharging || !FighterCombat)
	{
		return;
	}

	// Character owns devices; sim owns posture truth.
	FighterCombat->SetPostureDirection(
		MovementVector,
		/*bOverrideActive=*/ bIsPostureActionActive,
		/*bAllowMovementPosture=*/ IsCameraLockedOnCharacterBack() && !bIsPostureActionActive);
}

bool APlayerCharacter::RequestPostureChange(EPosture TargetPosture)
{
	if (!FighterCombat)
	{
		return false;
	}

	if (TargetPosture == EPosture::E_Neutral)
	{
		FighterCombat->RequestNeutralPosture();
		return true;
	}

	const FVector2D FakeDirection = [&]() -> FVector2D
	{
		switch (TargetPosture)
		{
		case EPosture::E_Up:        return FVector2D(0.f, 1.f);
		case EPosture::E_Down:      return FVector2D(0.f, -1.f);
		case EPosture::E_Left:      return FVector2D(-1.f, 0.f);
		case EPosture::E_Right:     return FVector2D(1.f, 0.f);
		case EPosture::E_DownLeft:  return FVector2D(-1.f, -1.f);
		case EPosture::E_DownRight: return FVector2D(1.f, -1.f);
		default:                    return FVector2D::ZeroVector;
		}
	}();
	FighterCombat->SetPostureDirection(FakeDirection, true, false);
	return true;
}

void APlayerCharacter::SyncPresentationFromCombatSim()
{
	if (!FighterCombat)
	{
		return;
	}

	const FFighterSimState SimState = FighterCombat->GetSimStateCopy();
	ActualPosture = SimState.Posture;
	bIsInAttackAnimation = SimState.HasFlag(FighterStateFlags::Attacking);
}

void APlayerCharacter::HandleSimLightAttackStarted(EPosture SnapshotPosture, UAnimMontage* Montage, int32 SimFrame)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[CombatSim] Presentation: LightAttack started posture=%d frame=%d montage=%s"),
		(int32)SnapshotPosture, SimFrame, *GetNameSafe(Montage));

	if (!bAutoPlaySimAttackMontage || !Montage)
	{
		return;
	}

	if (GetMesh())
	{
		PlayAnimMontage(Montage);
	}
}

void APlayerCharacter::RequestNeutralPosture()
{
	RequestPostureChange(EPosture::E_Neutral);
}

void APlayerCharacter::MoveActionStopped()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Released (Native): Move"));
	bIsMoving = false;
	StopRunning(); // Stop running when stop moving
	if (bIsPostureActionActive == false)
	{
		RequestNeutralPosture();
	}
}

void APlayerCharacter::PostureActionStopped()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Released (Native): Posture"));
	bIsPostureActionActive = false;
	if (bIsMoving == false)
	{
		RequestNeutralPosture();
	}
}

void APlayerCharacter::TryChangePostureByDefaultMovement(const FInputActionValue& Value)
{
	// Change Posture by default movement
	if (IsCameraLockedOnCharacterBack() && (bIsPostureActionActive == false))
	{
		ProcessPostureInput(Value);
		UE_LOG(LogTemp, Warning, TEXT("ChangeDefault posture"));
	}
}

bool APlayerCharacter::IsEnemyPlayer(const APlayerCharacter* Other) const
{
	return Other && (Other->GetTeamId() != TeamId);
}

bool APlayerCharacter::IsCameraLockedOnEnemy() const
{
	return LockOnCamera && LockOnCamera->IsLockedOnEnemy();
}

bool APlayerCharacter::IsCameraLockedOnCharacterBack() const
{
	return LockOnCamera && LockOnCamera->IsLockedOnBack();
}

AActor* APlayerCharacter::GetLockedOnActor() const
{
	return LockOnCamera ? LockOnCamera->GetLockedOnActor() : nullptr;
}

void APlayerCharacter::HandleLockOnEnemyChanged(bool bLockedOnEnemy, AActor* LockedActor)
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController || !FightingMappingContext)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	if (bLockedOnEnemy)
	{
		Subsystem->AddMappingContext(FightingMappingContext, 1);
		UE_LOG(LogTemp, Log, TEXT("[LockOn] Enemy locked: %s"), *GetNameSafe(LockedActor));
	}
	else
	{
		Subsystem->RemoveMappingContext(FightingMappingContext);
		UE_LOG(LogTemp, Log, TEXT("[LockOn] Enemy unlocked"));
	}
}

void APlayerCharacter::HandleLockOnBackChanged(bool bLockedOnBack)
{
	if (!bLockedOnBack)
	{
		RequestNeutralPosture();
	}
}

void APlayerCharacter::LockUnlockCameraOnEnemy()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Native): LockUnlock"));
	if (LockOnCamera)
	{
		LockOnCamera->ToggleLockOnEnemy(/*bAllowOrientToBack=*/ !bIsRunning);
	}
}

bool APlayerCharacter::CanActivateAbilityForInputTag_Implementation(FGameplayTag InputTag) const
{
	using namespace PurgatoriumLexGameplayTags;

	if (InputTag == InputTag_LightAttack)   return CanPerformLightAttack();
	// if (InputTag == InputTag_SpecialAttack) return CanPerformSpecialAttack();

	return true; // no gate for other tags
}

bool APlayerCharacter::CanPerformLightAttack_Implementation() const
{
	return FighterCombat
		&& !FighterCombat->IsAttacking()
		&& GetCharacterMovement()
		&& !GetCharacterMovement()->IsFalling();
}

void APlayerCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Ability): %s"), *InputTag.ToString());

	using namespace PurgatoriumLexGameplayTags;

	// Light attack is sim-owned (not GAS). Other tags may still use ASC until migrated.
	if (InputTag == InputTag_LightAttack)
	{
		if (!FighterCombat)
		{
			UE_LOG(LogTemp, Error, TEXT("[CombatSim] LightAttack pressed but FighterCombat missing"));
			return;
		}
		if (!CanActivateAbilityForInputTag(InputTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("[CombatSim] LightAttack gated — not sent to sim"));
			return;
		}
		FighterCombat->PressLightAttack();
		return;
	}

	if (!CanActivateAbilityForInputTag(InputTag))
	{
		if (IsInputTagBufferable(InputTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Input Buffer] Gate blocked %s - buffering for %d frames"), *InputTag.ToString(), AbilityInputBufferFrames);
			BufferAbilityInput(InputTag);
		}
		return;
	}

	UPurgatoriumLexAbilitySystemComponent* ASC = Cast<UPurgatoriumLexAbilitySystemComponent>(GetAbilitySystemComponent());
	if (ASC)
	{
		ASC->AbilityInputTagPressed(InputTag);
		const bool bActivated = ASC->ProcessAbilityInput(0.0f, false);
		if (IsInputTagBufferable(InputTag) && !bActivated)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Input Buffer] ASC didn't activate %s - buffering for %d frames"), *InputTag.ToString(), AbilityInputBufferFrames);
			BufferAbilityInput(InputTag);
		}
	}
}

void APlayerCharacter::BufferAbilityInput(FGameplayTag InputTag)
{
	if (!IsInputTagBufferable(InputTag) || AbilityInputBufferFrames <= 0) return;
	
	// Remove expired entries first
	int32 ExpiredCount = 0;
	AbilityInputBuffer.RemoveAll([this, &ExpiredCount](const FBufferedAbilityInput& E) {
		const bool bExpired = E.GetFramesRemaining(SimulationFrame, AbilityInputBufferFrames) <= 0;
		if (bExpired)
		{
			ExpiredCount++;
		}
		return bExpired;
	});
	if (ExpiredCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Input Buffer] Removed %d expired entries (Frame %d)"), ExpiredCount, SimulationFrame);
	}
	
	// Add new entry with current frame number (client-side only - feel improvement)
	const int32 FramesRemaining = AbilityInputBufferFrames;
	AbilityInputBuffer.Add(FBufferedAbilityInput(InputTag, SimulationFrame));
	UE_LOG(LogTemp, Warning, TEXT("[Input Buffer] Buffered %s at frame %d (expires in %d frames, buffer size: %d)"), 
		*InputTag.ToString(), SimulationFrame, FramesRemaining, AbilityInputBuffer.Num());
}

bool APlayerCharacter::IsInputTagBufferable(FGameplayTag InputTag) const
{
	return InputTag.IsValid() && BufferableInputTags.Contains(InputTag);
}

void APlayerCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	using namespace PurgatoriumLexGameplayTags;

	// Guard release is repurposed to drive Parry abilities via InputTag.Parry
	const FGameplayTag DispatchTag = (InputTag == InputTag_Guard) ? InputTag_Parry : InputTag;

	UE_LOG(LogTemp, Log, TEXT("[Input] Released (Ability): %s (Dispatch: %s)"), *InputTag.ToString(), *DispatchTag.ToString());
	if (UPurgatoriumLexAbilitySystemComponent* ASC = Cast<UPurgatoriumLexAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->AbilityInputTagReleased(DispatchTag);
		// Process input immediately for rollback netcode compatibility (frame-accurate input)
		ASC->ProcessAbilityInput(0.0f, false);
	}
}

void APlayerCharacter::SetupLegacyInputBindings(UEnhancedInputComponent* EnhancedInputComponent)
{
	check(EnhancedInputComponent);

	// Helper lambda to safely bind an action if it exists
	auto BindActionIfValid = [EnhancedInputComponent, this](const UInputAction* Action, ETriggerEvent TriggerEvent, auto Func)
	{
		if (Action)
		{
			EnhancedInputComponent->BindAction(Action, TriggerEvent, this, Func);
		}
	};

	// Movement & Camera
	BindActionIfValid(MoveAction, ETriggerEvent::Triggered, &APlayerCharacter::Move);
	BindActionIfValid(MoveAction, ETriggerEvent::Completed, &APlayerCharacter::MoveActionStopped); // also calls StopRunning
	BindActionIfValid(LookAction, ETriggerEvent::Triggered, &APlayerCharacter::Look);

	// Jump
	BindActionIfValid(JumpAction, ETriggerEvent::Triggered, &APlayerCharacter::DoJumpStart);
	BindActionIfValid(JumpAction, ETriggerEvent::Completed, &APlayerCharacter::DoJumpEnd);

	// Movement States
	BindActionIfValid(RunAction, ETriggerEvent::Triggered, &APlayerCharacter::StartRunning);

	// Posture
	BindActionIfValid(PostureAction, ETriggerEvent::Triggered, &APlayerCharacter::PostureActionTriggered);
	BindActionIfValid(PostureAction, ETriggerEvent::Completed, &APlayerCharacter::PostureActionStopped);

	// Combat
	BindActionIfValid(LightAttackAction, ETriggerEvent::Started, &APlayerCharacter::LightAttack);
	// Single IA: Started = start charge, Completed = release (attack or cancel if held < MinChargeTime)
	BindActionIfValid(ChargeAttackAction, ETriggerEvent::Started, &APlayerCharacter::StartChargeAttack);
	BindActionIfValid(ChargeAttackAction, ETriggerEvent::Completed, &APlayerCharacter::ChargeAttack);
	BindActionIfValid(SpecialAttackAction, ETriggerEvent::Started, &APlayerCharacter::SpecialAttack);
	BindActionIfValid(GuardAction, ETriggerEvent::Started, &APlayerCharacter::Guard);
	BindActionIfValid(GuardAction, ETriggerEvent::Completed, &APlayerCharacter::GuardReleased);
	BindActionIfValid(BreakGuardAction, ETriggerEvent::Started, &APlayerCharacter::BreakGuard);

	// Camera Control
	BindActionIfValid(LockUnlockAction, ETriggerEvent::Started, &APlayerCharacter::LockUnlockCameraOnEnemy);
}

void APlayerCharacter::LightAttack() {
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Legacy/Ability): LightAttack"));
	// New-way note:
	// Light attack should be triggered via InputConfig -> AbilityInputActions -> InputTag.LightAttack,
	// which routes into Input_AbilityInputTagPressed/Released and the ASC.
	// This function is now kept only for debugging / legacy callers.
	bAttackHasBeenUsed = true;
	UE_LOG(LogTemp, Warning, TEXT("ATTACKING\n"));
	//TakeDamages(0.02f);
}

void APlayerCharacter::StartChargeAttack() {
	UE_LOG(LogTemp, Log, TEXT("[Input] ChargeAttack Pressed (start charge)"));
	if (!bIsCharging && !GetCharacterMovement()->IsFalling())
	{
		bIsCharging = true;
		ChargeAttackStartTime = GetWorld()->GetTimeSeconds();
		GetWorld()->GetTimerManager().SetTimer(inputHeldTimer, this, &APlayerCharacter::ChargeAttack, maxInputHoldTime, false);
	}
}

void APlayerCharacter::ChargeAttack() {
	// Clear timer so it never fires again (we were either released or auto-released at max hold)
	GetWorld()->GetTimerManager().ClearTimer(inputHeldTimer);

	if (!bIsCharging)
	{
		return;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - ChargeAttackStartTime;
	if (Elapsed < MinChargeTime)
	{
		// Early release: cancel charge, no attack
		UE_LOG(LogTemp, Log, TEXT("[Input] ChargeAttack Released (cancel, held %.2fs < %.2fs)"), Elapsed, MinChargeTime);
		bIsCharging = false;
		return;
	}

	// Commit: execute charged attack
	UE_LOG(LogTemp, Log, TEXT("[Input] ChargeAttack Released (attack, held %.2fs)"), Elapsed);
	bAttackHasBeenUsed = true;
	bIsCharging = false;
	// TakeDamages(0.05f); // placeholder for actual attack
}


void APlayerCharacter::SpecialAttack()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Legacy/Ability): SpecialAttack"));
	if (bIsCharging)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpecialAttack\n"));
		bAttackHasBeenUsed = true;
		bIsCharging = false;
		UE_LOG(LogTemp, Warning, TEXT("ATTACKING\n"));
		//TakeDamages(0.04f);
	}
	
}

void APlayerCharacter::Guard()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Legacy/Ability): Guard"));
	
	// Check if character is in techable state - if so, handle tech input instead
	if (bIsTechable || IsInTechWindow())
	{
		OnTechInputPressed();
		return;
	}

	// Normal guard behavior
	bIsGuarding = true;
}

void APlayerCharacter::GuardReleased()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Released (Legacy/Ability): Guard"));
	
	// Handle tech input release
	OnTechInputReleased();
	
	// Normal guard release behavior
	bIsGuarding = false;
}

void APlayerCharacter::BreakGuard()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] Triggered (Legacy/Ability): BreakGuard"));
}

// ========================================================================
// TECH SYSTEM IMPLEMENTATION (SSBU-style)
// ========================================================================

bool APlayerCharacter::CanTech() const
{
	using namespace PurgatoriumLexGameplayTags;
	
	// Must be in techable state (tumbling/reeling)
	if (!bIsTechable)
	{
		return false;
	}

	// Check if in lockout period
	if (LastTechInputFrame >= 0)
	{
		const int32 FramesSinceLastInput = SimulationFrame - LastTechInputFrame;
		if (FramesSinceLastInput < TechLockoutFrames)
		{
			return false;
		}
	}

	// Check if already teching
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC && ASC->HasMatchingGameplayTag(State_Tech))
	{
		return false;
	}

	return true;
}

bool APlayerCharacter::IsInTechWindow() const
{
	return TechWindowStartFrame >= 0 && 
		   (SimulationFrame - TechWindowStartFrame) < TechWindowFrames;
}

void APlayerCharacter::OnTechInputPressed()
{
	bIsTechInputHeld = true;
	LastTechInputFrame = SimulationFrame;

	// If in tech window, perform tech immediately
	if (IsInTechWindow() && CanTech())
	{
		// Determine tech type based on input and contact
		// Use movement input from character movement component (rollback-safe)
		const FVector MovementInput = GetCharacterMovement()->GetLastInputVector();
		const float InputForward = MovementInput.X; // X component is forward/backward in Unreal
		const float InputRight = MovementInput.Y; // Y component is right/left in Unreal
		const bool bIsWallContact = !LastWallHitNormal.IsZero();
		const ETechType TechType = DetermineTechType(bIsWallContact, InputForward, InputRight);
		
		PerformTech(TechType);
	}
	// NOTE: Removed predictive tech window start for rollback compatibility
	// Tech windows are started only when actual contact occurs (Landed/NotifyHit callbacks).
	// This ensures deterministic behavior across all clients during rollback.
}

void APlayerCharacter::OnTechInputReleased()
{
	bIsTechInputHeld = false;
}

void APlayerCharacter::PerformTech(ETechType TechType)
{
	using namespace PurgatoriumLexGameplayTags;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Add Tech state tag
	ASC->AddLooseGameplayTag(State_Tech);

	// Remove knockdown/airborne states
	ASC->RemoveLooseGameplayTag(State_KnockedDown);
	ASC->RemoveLooseGameplayTag(State_Airborne);
	ASC->RemoveLooseGameplayTag(State_Hitstun);

	// Remove wall splat/ground bounce if present
	ASC->RemoveLooseGameplayTag(State_WallSplat);
	ASC->RemoveLooseGameplayTag(State_GroundBounce);

	// Reset techable state
	bIsTechable = false;

	// End tech window
	EndTechWindow();

	// Apply tech lockout
	LastTechInputFrame = SimulationFrame;

	// Handle tech type-specific behavior
	switch (TechType)
	{
	case ETechType::E_Standard:
		UE_LOG(LogTemp, Log, TEXT("[Tech] Performed STANDARD tech"));
		// Standard tech: character bounces up and recovers standing
		// Animation/movement handled by Blueprint or ability system
		break;

	case ETechType::E_RollForward:
		UE_LOG(LogTemp, Log, TEXT("[Tech] Performed ROLL FORWARD tech"));
		// Rolling tech forward: character rolls forward during recovery
		// Movement handled by Blueprint or ability system
		break;

	case ETechType::E_RollBackward:
		UE_LOG(LogTemp, Log, TEXT("[Tech] Performed ROLL BACKWARD tech"));
		// Rolling tech backward: character rolls backward during recovery
		// Movement handled by Blueprint or ability system
		break;

	case ETechType::E_RollLeft:
		UE_LOG(LogTemp, Log, TEXT("[Tech] Performed ROLL LEFT tech"));
		// Rolling tech left: character rolls left during recovery
		// Movement handled by Blueprint or ability system
		break;

	case ETechType::E_RollRight:
		UE_LOG(LogTemp, Log, TEXT("[Tech] Performed ROLL RIGHT tech"));
		// Rolling tech right: character rolls right during recovery
		// Movement handled by Blueprint or ability system
		break;

	case ETechType::E_Wall:
		UE_LOG(LogTemp, Log, TEXT("[Tech] Performed WALL tech"));
		// Wall tech: character bounces off wall and recovers
		// Cancel momentum, apply recovery
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
		break;

	case ETechType::E_WallJump:
		UE_LOG(LogTemp, Log, TEXT("[Tech] Performed WALL JUMP tech"));
		// Wall tech jump: character wall jumps during tech
		// Jump handled by Blueprint or ability system
		// Note: In SSBU, wall tech jump can be performed by holding jump input during wall tech
		break;
	}

	// Tech recovery completion will be handled by Blueprint/montage notify
	// Remove State.Tech tag when tech animation completes
}

void APlayerCharacter::UpdateTechableState()
{
	using namespace PurgatoriumLexGameplayTags;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		bIsTechable = false;
		return;
	}

	// Character is techable if:
	// 1. In hitstun (tumbling/reeling state)
	// 2. Not already teching
	// 3. Not already on ground (for ground tech) OR airborne (for wall tech)
	const bool bInHitstun = ASC->HasMatchingGameplayTag(State_Hitstun);
	const bool bIsTeched = ASC->HasMatchingGameplayTag(State_Tech);
	const bool bIsGrounded = GetCharacterMovement()->IsMovingOnGround();
	const bool bIsFalling = GetCharacterMovement()->IsFalling();

	bIsTechable = bInHitstun && !bIsTeched && (bIsFalling || bIsGrounded);

	// NOTE: Removed predictive tech window start (WillHitGroundSoon) for rollback compatibility
	// Predictive line traces can be non-deterministic between clients during rollback.
	// Tech windows are now started only when actual contact occurs (Landed callback).
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	using namespace PurgatoriumLexGameplayTags;
	
	// Start tech window on ground contact (rollback-safe: contact events are deterministic)
	if (bIsTechable && !IsInTechWindow())
	{
		StartTechWindow();
	}
	
	// Check if tech input was buffered or is currently held
	if (bIsTechInputHeld || IsInTechWindow())
	{
		// Check if we can tech this landing
		if (CanTech())
		{
			// Determine tech type based on input direction
			// Use movement input from character movement component (rollback-safe)
			const FVector MovementInput = GetCharacterMovement()->GetLastInputVector();
			const float InputForward = MovementInput.X; // X component is forward/backward in Unreal
			const float InputRight = MovementInput.Y; // Y component is right/left in Unreal
			const ETechType TechType = DetermineTechType(false, InputForward, InputRight); // false = ground contact
			
			PerformTech(TechType);
			return;
		}
	}

	// Normal landing (no tech)
	// Character enters knockdown state if in hitstun
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC && ASC->HasMatchingGameplayTag(State_Hitstun))
	{
		ASC->AddLooseGameplayTag(State_KnockedDown);
		ASC->AddLooseGameplayTag(State_GroundBounce);
	}
}

void APlayerCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	using namespace PurgatoriumLexGameplayTags;
	
	// Only process wall hits when airborne (not ground contact - that's handled by Landed())
	if (!GetCharacterMovement()->IsFalling() && !GetCharacterMovement()->IsMovingOnGround())
	{
		return; // Not airborne, ignore
	}

	// Check if hit surface is roughly vertical (wall, not floor/ceiling)
	const float WallDot = FMath::Abs(FVector::DotProduct(HitNormal, FVector::UpVector));
	if (WallDot > 0.7f)
	{
		return; // Surface is more horizontal than vertical (floor/ceiling, not wall)
	}

	// This is a wall hit
	LastWallHitNormal = HitNormal;

	// Start tech window on wall contact (rollback-safe: contact events are deterministic)
	if (bIsTechable && !IsInTechWindow())
	{
		StartTechWindow();
	}

	// Check if tech input was buffered or is currently held
	if (bIsTechInputHeld || IsInTechWindow())
	{
		// Check if we can tech this wall hit
		if (CanTech())
		{
			// Check knockback speed threshold (SSBU: untechable if speed >= 6.0)
			const float KnockbackSpeed = GetCharacterMovement()->Velocity.Size();
			if (KnockbackSpeed >= TechKnockbackThreshold)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Tech] Wall hit untechable: knockback speed %.2f >= threshold %.2f"), 
					KnockbackSpeed, TechKnockbackThreshold);
				// Enter wall splat state (untechable)
				UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
				if (ASC)
				{
					ASC->AddLooseGameplayTag(State_WallSplat);
				}
				return;
			}

			// Determine tech type (wall tech or wall tech jump)
			const ETechType TechType = bIsJumpInputHeld ? ETechType::E_WallJump : ETechType::E_Wall;
			
			PerformTech(TechType);
			return;
		}
	}

	// Normal wall hit (no tech)
	// Character enters wall splat state if in hitstun
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC && ASC->HasMatchingGameplayTag(State_Hitstun))
	{
		ASC->AddLooseGameplayTag(State_WallSplat);
	}
}

void APlayerCharacter::StartTechWindow()
{
	if (IsInTechWindow())
	{
		return; // Already in tech window
	}

	TechWindowStartFrame = SimulationFrame;
	UE_LOG(LogTemp, VeryVerbose, TEXT("[Tech] Tech window started at frame %d"), SimulationFrame);
}

void APlayerCharacter::EndTechWindow()
{
	if (TechWindowStartFrame < 0)
	{
		return; // No active tech window
	}

	TechWindowStartFrame = -1;
	UE_LOG(LogTemp, VeryVerbose, TEXT("[Tech] Tech window ended at frame %d"), SimulationFrame);
}

bool APlayerCharacter::WillHitGroundSoon() const
{
	if (!GetCharacterMovement()->IsFalling())
	{
		return false;
	}

	// Perform a line trace downward to check if ground is close
	const FVector StartLocation = GetActorLocation();
	const FVector EndLocation = StartLocation + FVector(0.0f, 0.0f, -200.0f); // Check 200 units below

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Pawn,
		QueryParams
	);

	if (bHit)
	{
		// Estimate time to impact based on vertical velocity
		const float VerticalVelocity = GetCharacterMovement()->Velocity.Z;
		if (VerticalVelocity < 0.0f) // Falling
		{
			const float DistanceToGround = FMath::Abs(StartLocation.Z - HitResult.Location.Z);
			const float TimeToImpact = DistanceToGround / FMath::Abs(VerticalVelocity);
			// Consider "soon" as within ~0.2 seconds (12 frames at 60fps)
			return TimeToImpact <= 0.2f;
		}
	}

	return false;
}

bool APlayerCharacter::WillHitWallSoon() const
{
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		return false; // Only check for walls when airborne
	}

	// Perform a line trace in movement direction to check if wall is close
	const FVector Velocity = GetCharacterMovement()->Velocity;
	if (Velocity.SizeSquared() < 100.0f) // Too slow to matter
	{
		return false;
	}

	const FVector StartLocation = GetActorLocation();
	const FVector VelocityDirection = Velocity.GetSafeNormal();
	const FVector EndLocation = StartLocation + VelocityDirection * 150.0f; // Check 150 units ahead

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Pawn,
		QueryParams
	);

	if (bHit)
	{
		// Check if hit surface is roughly vertical (wall, not floor/ceiling)
		const float WallDot = FMath::Abs(FVector::DotProduct(HitResult.Normal, FVector::UpVector));
		return WallDot < 0.7f; // Surface is more horizontal than vertical (wall-like)
	}

	return false;
}

ETechType APlayerCharacter::DetermineTechType(bool bIsWallContact, float InputForward, float InputRight) const
{
	if (bIsWallContact)
	{
		// Wall tech: check if jump input is held for wall tech jump
		return bIsJumpInputHeld ? ETechType::E_WallJump : ETechType::E_Wall;
	}
	else
	{
		// Ground tech: check input direction for rolling tech
		// Priority: forward/backward takes precedence over left/right
		const float Threshold = 0.5f; // Deadzone threshold
		
		// Check forward/backward first
		if (InputForward > Threshold)
		{
			return ETechType::E_RollForward;
		}
		else if (InputForward < -Threshold)
		{
			return ETechType::E_RollBackward;
		}
		// Then check left/right
		else if (InputRight < -Threshold)
		{
			return ETechType::E_RollLeft;
		}
		else if (InputRight > Threshold)
		{
			return ETechType::E_RollRight;
		}
		else
		{
			return ETechType::E_Standard;
		}
	}
}



//void APlayerCharacter::TakeDamages(float damageAmount)
//{
//	UE_LOG(LogTemp, Warning, TEXT("We are taking damages for %f points\n"), damageAmount);
//	playerHealth -= damageAmount;
//
//	if (playerHealth < 0.00f)
//	{
//		playerHealth = 0.00f;
//	}
//	UE_LOG(LogTemp, Warning, TEXT("playerHealth is now: %f points\n"), playerHealth);
//}
//
//void APlayerCharacter::StartDamage()
//{
//	//TakeDamages(0.03f);
//}
//
//void APlayerCharacter::Heal(float healAmount)
//{
//
//	UE_LOG(LogTemp, Warning, TEXT("We are healing for %f points, team id: %d\n"), healAmount, TeamId);
//	playerHealth += healAmount;
//
//	if (playerHealth > 1.00f)
//	{
//		playerHealth = 1.00f;
//	}
//	UE_LOG(LogTemp, Warning, TEXT("playerHealth is now: %f points, team id: %d\n"), healAmount, TeamId);
//}
//
//void APlayerCharacter::StartHealing()
//{
//	Heal(0.10f);
//}