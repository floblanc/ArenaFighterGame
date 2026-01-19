


#include "PlayerCharacter.h"
#include "AbilitySystem/PurgatoriumLexAbilitySystemComponent.h"
#include "AbilitySystem/PurgatoriumLexAttributeSet.h"
#include "Player/PurgatoriumLexPlayerState.h"
#include "UI/PurgatoriumLexHUD.h"
#include "Input/PurgatoriumLexInputComponent.h"
#include "Input/PurgatoriumLexInputConfig.h"
#include "PurgatoriumLexGameplayTags.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"

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
	bUseControllerRotationYaw = false; // lock camera behind character
	bUseControllerRotationRoll = false;

	bIsRunning = false;
	WalkingSpeed = 400.f;
	RunningSpeed = 800.f;

	DashDistance = 750.0f;

	SetPostureToNeutral();
	bIsCameraLockedOnCharacterBack = false;
	bIsCameraLockedOnEnemy = false;
	
	lockedOnActor = nullptr;
	targetingHeighOffset = 30.0f; //Can be prototyped to MAX_CAMERA_HEIGHT au corps à corps -> et peut être créer un MIN_CAMERA_HEIGHT pour les longue distances et modifier le calcul (mettre en fonction) pour assurer le comportement (fonction pour camera a mettre dans un autre fichier?) -> valeurs parametrables par le joueur???.

	playerHealth = 1.00f;
	bAttackHasBeenUsed = false;
	bIsRigthPunchHitboxActive = false;
	bIsInAttackAnimation = false;
	bIsCharging = false;
	maxInputHoldTime = 3.5f;

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

	// set the player tag
	Tags.Add(FName("Player"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystemComponent();
	GiveDefaultAbilities();
	InitDefaultAttributes();
	InitHUD();

	BP_TryInitFloatingHealthBar();
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
 
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Perform the BluePrint Tick logic
	//BPTick(DeltaTime);
	
	// Update camera lock-on deterministically
	// This is called from Tick() but is still deterministic for rollback netcode because:
	// - It only uses actor positions and rotations (part of rollback state)
	// - All calculations are pure functions of rollback state
	// - As long as inputs (actor positions) are deterministic, output (camera rotation) is deterministic
	UpdateCameraLockOn();
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UE_LOG(LogTemplateCharacter, Log, TEXT("[Input Setup] Starting input component setup for %s"), *GetNameSafe(this));

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogTemplateCharacter, Log, TEXT("[Input Setup] Added DefaultMappingContext: %s"), *GetNameSafe(DefaultMappingContext));
			}
			else
			{
				UE_LOG(LogTemplateCharacter, Warning, TEXT("[Input Setup] DefaultMappingContext is NULL! Input will not work. Please assign an Input Mapping Context in the character blueprint."));
			}
		}
		else
		{
			UE_LOG(LogTemplateCharacter, Error, TEXT("[Input Setup] Failed to get EnhancedInputLocalPlayerSubsystem! Enhanced Input may not be enabled."));
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("[Input Setup] Failed to get PlayerController! Input setup cannot continue."));
		return;
	}

	// Cast to our custom input component (required for InputConfig/tag bindings)
	UPurgatoriumLexInputComponent* PurgatoriumLexInputComponent = Cast<UPurgatoriumLexInputComponent>(PlayerInputComponent);
	if (!PurgatoriumLexInputComponent)
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("[Input Setup] '%s' Failed to find PurgatoriumLexInputComponent! Current InputComponent type: %s. Please ensure the InputComponent is set to PurgatoriumLexInputComponent in the character blueprint."), 
			*GetNameSafe(this), *GetNameSafe(PlayerInputComponent->GetClass()));
		return;
	}
	UE_LOG(LogTemplateCharacter, Log, TEXT("[Input Setup] Successfully cast to PurgatoriumLexInputComponent"));

	// New-way only: InputConfig is mandatory
	if (!InputConfig)
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("[Input Setup] '%s' InputConfig is not set. Please assign a UPurgatoriumLexInputConfig on the character blueprint."), *GetNameSafe(this));
		return;
	}
	UE_LOG(LogTemplateCharacter, Log, TEXT("[Input Setup] InputConfig found: %s"), *GetNameSafe(InputConfig));

	// Log InputConfig contents for debugging
	if (InputConfig)
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("[Input Setup] NativeInputActions count: %d"), InputConfig->NativeInputActions.Num());
		UE_LOG(LogTemplateCharacter, Log, TEXT("[Input Setup] AbilityInputActions count: %d"), InputConfig->AbilityInputActions.Num());
	}

	// Bind ability actions (InputAction -> InputTag), then route to ASC via Input_AbilityInputTagPressed/Released.
	PurgatoriumLexInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ AbilityInputBindHandles);
	UE_LOG(LogTemplateCharacter, Log, TEXT("[Input Setup] Bound %d ability actions"), AbilityInputBindHandles.Num());

	// Bind native (non-ability) actions via tags.
	int32 NativeBindCount = 0;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Move, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ThisClass::Look, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Jump, ETriggerEvent::Triggered, this, &ThisClass::DoJumpStart, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Jump, ETriggerEvent::Completed, this, &ThisClass::DoJumpEnd, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Run, ETriggerEvent::Triggered, this, &ThisClass::StartRunning, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Dash, ETriggerEvent::Started, this, &ThisClass::Dash, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Posture, ETriggerEvent::Triggered, this, &ThisClass::PostureActionTriggered, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_Posture, ETriggerEvent::Completed, this, &ThisClass::PostureActionStopped, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	if (PurgatoriumLexInputComponent->BindNativeAction(InputConfig, PurgatoriumLexGameplayTags::InputTag_LockUnlock, ETriggerEvent::Started, this, &ThisClass::LockUnlockCameraOnEnemy, /*bLogIfNotFound=*/ true)) NativeBindCount++;
	
	UE_LOG(LogTemplateCharacter, Log, TEXT("[Input Setup] Bound %d native actions"), NativeBindCount);
	UE_LOG(LogTemplateCharacter, Log, TEXT("[Input Setup] Input setup complete for %s"), *GetNameSafe(this));
}


void APlayerCharacter::Move(const FInputActionValue& Value)
{
	bIsMoving = ((!(bIsInAttackAnimation)) || GetCharacterMovement()->IsFalling()) && !bIsCharging;
	UE_LOG(LogTemp, Warning, TEXT("bIsMoving : %d (DETAILS : bIsInAttackAnimation = %d and GetCharacterMovement()->IsFalling() = %d)"), bIsMoving, bIsInAttackAnimation, GetCharacterMovement()->IsFalling());

	// Change Posture by default movement
	if (bIsCameraLockedOnCharacterBack && bIsMoving && (bIsPostureActionActive == false))
	{
		ChangePosture(Value);
		UE_LOG(LogTemp, Warning, TEXT("ChangeDefault posture"));
	}
	
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr && bIsMoving)
	{
		if (bIsCameraLockedOnEnemy)
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
		
		double distance = (lockedOnActor->GetActorLocation() - GetActorLocation()).Size(); // entre 70-100 et 1000-1500 environ -> 70 = collé, 100 = très proche
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

void APlayerCharacter::DoMoveAroundSomethingProgressive(float Right, float Forward)
{
	if (!lockedOnActor || !IsValid(lockedOnActor) || !Controller)
	{
		return;
	}

	// Get direction from character to locked enemy (horizontal plane only)
	FVector ToEnemy = lockedOnActor->GetActorLocation() - GetActorLocation();
	ToEnemy.Z = 0.0f; // Ignore height difference
	const float DistanceToEnemy = ToEnemy.Size();

	ToEnemy.Normalize();

	// Calculate tangent vector (perpendicular to ToEnemy, for circling around)
	// This is the direction for pure left/right movement (grey circle path)
	FVector TangentVector = FVector::CrossProduct(ToEnemy, FVector::UpVector);
	TangentVector.Normalize();

	// Calculate input magnitude for movement speed
	const float InputMagnitude = FMath::Sqrt(Right * Right + Forward * Forward);
	
	// Progressive blending based on input:
	// - Pure left/right (Forward ≈ 0): Mostly tangent (circling)
	// - Pure forward (Right ≈ 0): Mostly toward enemy
	// - Pure back (Right ≈ 0, Forward < 0): Mostly away from enemy
	// - Diagonal: Blend proportionally
	
	// Normalize inputs to get direction weights
	// Forward component: how much toward/away from enemy
	// Right component: how much circling around enemy
	const float ForwardWeight = FMath::Abs(Forward) / FMath::Max(InputMagnitude, 0.001f);
	const float RightWeight = FMath::Abs(Right) / FMath::Max(InputMagnitude, 0.001f);
	
	// Blend the two directions proportionally
	// More forward = more toward enemy, more right = more circling
	FVector MovementDirection;
	
	if (Forward >= 0.0f)
	{
		// Forward or diagonal forward: blend toward enemy + circling
		MovementDirection = (ToEnemy * ForwardWeight) + (TangentVector * FMath::Sign(Right) * RightWeight);
	}
	else
	{
		// Backward or diagonal backward: blend away from enemy + circling
		FVector AwayFromEnemy = -ToEnemy;
		MovementDirection = (AwayFromEnemy * ForwardWeight) + (TangentVector * FMath::Sign(Right) * RightWeight);
	}
	
	MovementDirection.Normalize();

	// Apply movement with input magnitude (proportional to stick tilt)
	AddMovementInput(MovementDirection, InputMagnitude);
}

void APlayerCharacter::DoMoveAroundSomethingUE5Assistant(float Right, float Forward)
{
	if (!lockedOnActor || !IsValid(lockedOnActor) || !Controller)
	{
		return;
	}

	FVector EnemyLoc = lockedOnActor->GetActorLocation();
	FVector MyLoc = GetActorLocation();

	// Radial direction: from enemy to player (outward)
	FVector ToPlayer = MyLoc - EnemyLoc;
	ToPlayer.Z = 0.0f;

	if (ToPlayer.IsNearlyZero())
	{
		// Fallback to default movement to avoid issues
		AddMovementInput(GetActorForwardVector(), Forward);
		AddMovementInput(GetActorRightVector(), Right);
		return;
	}

	// Radial direction (outward from enemy)
	FVector RadialDir = ToPlayer.GetSafeNormal();

	// Tangent direction (perpendicular, for orbiting)
	FVector TangentDir = FVector::CrossProduct(FVector::UpVector, RadialDir);
	TangentDir.Normalize();

	// Orbit sideways with horizontal stick input (X = Right)
	AddMovementInput(TangentDir, Right);

	// Move in/out with vertical stick input (Y = Forward)
	// Forward = positive = inward (toward enemy) - inverted for intuitive control
	// Back = negative = outward (away from enemy)
	AddMovementInput(-RadialDir, Forward);

	// Face enemy smoothly
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(MyLoc, EnemyLoc);
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
	SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
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
	// signal the character to jump
	if (!bAttackHasBeenUsed && !bIsInAttackAnimation)
	{
		Jump();
	}
}

void APlayerCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void APlayerCharacter::UnlockCharacterBackFromCamera()
{
	bUseControllerRotationYaw = false;
	bIsCameraLockedOnCharacterBack = false;
	SetPostureToNeutral();
}

void APlayerCharacter::LockCameraOnCharacterBack()
{
	bUseControllerRotationYaw = true;
	bIsCameraLockedOnCharacterBack = true;
}

void APlayerCharacter::StartRunning()
{
	UnlockCharacterBackFromCamera();

	// Check if character is already running
	bIsRunning = true;

	GetCharacterMovement()->MaxWalkSpeed = RunningSpeed; // Set running speed
}

void APlayerCharacter::StopRunning()
{
	if (bIsCameraLockedOnEnemy)
	{
		LockCameraOnCharacterBack();
	}

	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed; // Reset to walking speed
}

void APlayerCharacter::Dash()
{
	// Check if the controller is valid
	if (Controller != nullptr && !bIsCharging)
	{
		// Get the last movement input vector
		FVector MovementVector = GetCharacterMovement()->GetLastInputVector();
		UE_LOG(LogTemp, Warning, TEXT("Last Input Vector : (%f, %f, %f)"), MovementVector.X, MovementVector.Y, MovementVector.Z);

		if (MovementVector.IsNearlyZero()) // If there's no movement input, set default to zero
		{
			MovementVector = FVector::ZeroVector;
		}

		// Normalize the vector
		MovementVector.Normalize();

		// Snap to the closest cardinal direction
		// if (FMath::Abs(MovementVector.X) > FMath::Abs(MovementVector.Y))
		// {
		// 	MovementVector = ForwardDirection * MovementVector.Y;
		// }
		// else
		// {
		// 	MovementVector = RightDirection * MovementVector.X;
		// }

		FVector DashVector = MovementVector * DashDistance;
		
		// Apply the impulse to the character movement component
		GetCharacterMovement()->AddImpulse(DashVector, true);

		UE_LOG(LogTemp, Warning, TEXT("Dashing!"));
	}
}

void APlayerCharacter::PostureActionTriggered(const FInputActionValue& Value)
{
	if (bIsCameraLockedOnCharacterBack)
	{
		bIsPostureActionActive = true;
		ChangePosture(Value);
	}
}

void APlayerCharacter::ChangePosture(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr && !bIsCharging)
	{
		float AngleRad = FMath::Atan2(MovementVector.Y, MovementVector.X);  // Get angle in radians [-PI, PI]
		float AngleDeg = FMath::RadiansToDegrees(AngleRad);  // Convert to degrees [-180, 180]
		
		UE_LOG(LogTemp, Warning, TEXT("\nAngle between -180 and 180 for Actual Posture : %f\n"), AngleDeg);
		
		if (AngleDeg < 0.f) AngleDeg += 360.f;  // Convert to [0, 360]

		UE_LOG(LogTemp, Warning, TEXT("\nAngle not Rounded for Actual Posture : %f\n"), AngleDeg);

		int finalAngle = FMath::RoundToInt(AngleDeg);
		UE_LOG(LogTemp, Warning, TEXT("Angle for Actual Posture : %i\n"), finalAngle);

		// Determine the rounded direction
		// switch (finalAngle)
		// {
		// 	case 15 ... 65:
		// 		ActualPosture = EPosture::E_DownRight;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_DownRight\n"));
		// 		break;
		// 	case 66 ... 115:
		// 		ActualPosture = EPosture::E_Down;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Down\n"));
		// 		break;
		// 	case 116 ... 165:
		// 		ActualPosture = EPosture::E_DownLeft;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_DownLeft\n"));
		// 		break;
		// 	case 166 ... 235:
		// 		ActualPosture = EPosture::E_Left;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Left\n"));
		// 		break;
		// 	case 236 ... 305:
		// 		ActualPosture = EPosture::E_Up;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Up\n"));
		// 		break;
		// 	case 306 ... 360:
		// 	case 0 ... 14:
		// 		ActualPosture = EPosture::E_Right;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Right\n"));
		// 		break;
		// }
		// switch (finalAngle)
		// {
		// 	case 296 ... 345:
		// 		ActualPosture = EPosture::E_DownRight;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_DownRight\n"));
		// 		break;
		// 	case 246 ... 295:
		// 		ActualPosture = EPosture::E_Down;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Down\n"));
		// 		break;
		// 	case 196 ... 245:
		// 		ActualPosture = EPosture::E_DownLeft;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_DownLeft\n"));
		// 		break;
		// 	case 126 ... 195:
		// 		ActualPosture = EPosture::E_Left;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Left\n"));
		// 		break;
		// 	case 56 ... 125:
		// 		ActualPosture = EPosture::E_Up;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Up\n"));
		// 		break;
		// 	case 346 ... 360:
		// 	case 0 ... 55:
		// 		ActualPosture = EPosture::E_Right;
		// 		UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Right\n"));
		// 		break;
		// }
		//////////////TRUC MOCHE POUR WINDOWS///////////////////
		if (finalAngle >= 296 && finalAngle <= 345)
		{
			ActualPosture = EPosture::E_DownRight;
			UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_DownRight\n"));
		}
		else if (finalAngle >= 246 && finalAngle <= 295)
		{
			ActualPosture = EPosture::E_Down;
			UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Down\n"));
		}
		else if (finalAngle >= 196 && finalAngle <= 245)
		{
			ActualPosture = EPosture::E_DownLeft;
			UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_DownLeft\n"));
		}
		else if (finalAngle >= 126 && finalAngle <= 195)
		{
			ActualPosture = EPosture::E_Left;
			UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Left\n"));
		}
		else if (finalAngle >= 56 && finalAngle <= 125)
		{
			ActualPosture = EPosture::E_Up;
			UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Up\n"));
		}
		else if ((finalAngle >= 346 && finalAngle <= 360) || (finalAngle >= 0 && finalAngle <= 55))
		{
			ActualPosture = EPosture::E_Right;
			UE_LOG(LogTemp, Warning, TEXT("NEW Posture : E_Right\n"));
		}
	}
}

void APlayerCharacter::SetPostureToNeutral()
{
	ActualPosture = EPosture::E_Neutral;
	UE_LOG(LogTemp, Warning, TEXT("---------\nNEUTRAL POSTURE\n---------\n"));
}

void APlayerCharacter::MoveActionStopped()
{
	bIsMoving = false;
	if (bIsPostureActionActive == false)
	{
		SetPostureToNeutral();
	}
}

void APlayerCharacter::PostureActionStopped()
{
	bIsPostureActionActive = false;
	if (bIsMoving == false)
	{
		SetPostureToNeutral();
	}
}

void APlayerCharacter::TryChangePostureByDefaultMovement(const FInputActionValue& Value)
{
	// Change Posture by default movement
	if ( bIsCameraLockedOnCharacterBack && (bIsPostureActionActive == false) )
	{
		ChangePosture(Value);
		UE_LOG(LogTemp, Warning, TEXT("ChangeDefault posture"));
	}
}

bool APlayerCharacter::IsEnemy(int id)
{
	return (id == TeamId);
}

bool APlayerCharacter::IsEnemy(APlayerCharacter *fighter)
{
	return (fighter->GetTeamId() == TeamId);
}

int  APlayerCharacter::GetTeamId()
{
	return (TeamId);
}
void APlayerCharacter::SetTeamId(int teamId)
{
	TeamId = teamId;
}

void APlayerCharacter::UpdateCameraLockOn()
{
	// This function is called from Tick() but is deterministic for rollback netcode
	// It only uses actor positions and rotations (which are part of rollback state)
	// All calculations are pure functions of rollback state, ensuring determinism
	
	if (bIsCameraLockedOnEnemy && lockedOnActor && IsValid(lockedOnActor))
	{
		// Calculate distance for camera height adjustment
		const float Distance = (lockedOnActor->GetActorLocation() - GetActorLocation()).Size();
		
		// Calculate look-at rotation
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
			GetActorLocation(), 
			lockedOnActor->GetActorLocation()
		);
		
		// Adjust pitch based on distance (closer = higher pitch)
		LookAtRotation.Pitch -= (targetingHeighOffset - Distance / 100.0f);
		
		// Apply rotation to controller
		if (AController* MyController = GetController())
		{
			MyController->SetControlRotation(LookAtRotation);
		}
	}
}

void APlayerCharacter::LockUnlockCameraOnEnemy()
{
	if (bIsCameraLockedOnEnemy)
	{
		//UnlockCameraFromEnemy
		bIsCameraLockedOnEnemy = false;
		lockedOnActor = nullptr;
		UnlockCharacterBackFromCamera();
		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->RemoveMappingContext(FightingMappingContext);
			}
		}
	}
	else
	{
		//LockCameraOnEnemy
		if (lockOnCandidates.Num() > 0)
		{
			lockedOnActor = lockOnCandidates[0]; // TODO: wrap ça dans une fonction SelectEnemyToLock??
			if (lockedOnActor)
			{
				bIsCameraLockedOnEnemy = true;
				if (bIsRunning == false)
				{
					LockCameraOnCharacterBack();
				}

				if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
					{
						Subsystem->AddMappingContext(FightingMappingContext, 1);
					}
				}
			}
		}
	}
}

void APlayerCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UPurgatoriumLexAbilitySystemComponent* ASC = Cast<UPurgatoriumLexAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->AbilityInputTagPressed(InputTag);
		// Process input immediately for rollback netcode compatibility (frame-accurate input)
		ASC->ProcessAbilityInput(0.0f, false);
	}
}

void APlayerCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UPurgatoriumLexAbilitySystemComponent* ASC = Cast<UPurgatoriumLexAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->AbilityInputTagReleased(InputTag);
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
	BindActionIfValid(MoveAction, ETriggerEvent::Completed, &APlayerCharacter::MoveActionStopped);
	BindActionIfValid(LookAction, ETriggerEvent::Triggered, &APlayerCharacter::Look);

	// Jump
	BindActionIfValid(JumpAction, ETriggerEvent::Triggered, &APlayerCharacter::DoJumpStart);
	BindActionIfValid(JumpAction, ETriggerEvent::Completed, &APlayerCharacter::DoJumpEnd);

	// Movement States
	BindActionIfValid(RunAction, ETriggerEvent::Triggered, &APlayerCharacter::StartRunning);
	BindActionIfValid(DashAction, ETriggerEvent::Started, &APlayerCharacter::Dash);

	// Posture
	BindActionIfValid(PostureAction, ETriggerEvent::Triggered, &APlayerCharacter::PostureActionTriggered);
	BindActionIfValid(PostureAction, ETriggerEvent::Completed, &APlayerCharacter::PostureActionStopped);

	// Combat
	BindActionIfValid(LightAttackAction, ETriggerEvent::Started, &APlayerCharacter::LightAttack);
	BindActionIfValid(ChargeHeavyAttackAction, ETriggerEvent::Started, &APlayerCharacter::ChargeHeavyAttack);
	BindActionIfValid(HeavyAttackAction, ETriggerEvent::Completed, &APlayerCharacter::HeavyAttack);
	BindActionIfValid(SpecialAttackAction, ETriggerEvent::Started, &APlayerCharacter::SpecialAttack);
	BindActionIfValid(GuardAction, ETriggerEvent::Started, &APlayerCharacter::Guard);
	BindActionIfValid(BreakGuardAction, ETriggerEvent::Started, &APlayerCharacter::BreakGuard);

	// Camera Control
	BindActionIfValid(LockUnlockAction, ETriggerEvent::Started, &APlayerCharacter::LockUnlockCameraOnEnemy);
}

void APlayerCharacter::LightAttack() {
	UE_LOG(LogTemp, Warning, TEXT("LightAttack\n"));
	
	// New-way note:
	// Light attack should be triggered via InputConfig -> AbilityInputActions -> InputTag.LightAttack,
	// which routes into Input_AbilityInputTagPressed/Released and the ASC.
	// This function is now kept only for debugging / legacy callers.
	bAttackHasBeenUsed = true;
	UE_LOG(LogTemp, Warning, TEXT("ATTACKING\n"));
	//TakeDamages(0.02f);
}

void APlayerCharacter::ChargeHeavyAttack() {
	if (!bIsCharging && !(GetCharacterMovement()->IsFalling()))
	{
		UE_LOG(LogTemp, Warning, TEXT("ChargeHeavyAttack\n"));
		bIsCharging = true;
		GetWorld()->GetTimerManager().SetTimer(inputHeldTimer, this, &APlayerCharacter::HeavyAttack, maxInputHoldTime, false);
		UE_LOG(LogTemp, Warning, TEXT("ATTACKING\n"));		
	}
}

void APlayerCharacter::HeavyAttack() {
	UE_LOG(LogTemp, Warning, TEXT("HeavyAttack\n"));
	bAttackHasBeenUsed = true;
	UE_LOG(LogTemp, Warning, TEXT("ATTACKING\n"));
	//TakeDamages(0.05f);
}


void APlayerCharacter::SpecialAttack()
{
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
	bIsGuarding = true;
	UE_LOG(LogTemp, Warning, TEXT("Guard\n"));
}

void APlayerCharacter::BreakGuard()
{
	UE_LOG(LogTemp, Warning, TEXT("BreakGuard\n"));
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