// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaFighterGameCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"

#include "Misc/DateTime.h"

//////////////////////////////////////////////////////////////////////////
// AArenaFighterGameCharacter

AArenaFighterGameCharacter::AArenaFighterGameCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false; //try here?
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	bIsRunning = false;
	WalkingSpeed = 400.f;
	RunningSpeed = 800.f;

	DashDistance = 1500.0f;

	SetPostureToNeutral();
	bIsCameraLockedOnCharacterBack = false;
	bIsCameraLockedOnEnemy = false;
	
	lockedOnActor = nullptr;
	targetingHeighOffset = 30.0f; //Can be prototyped to MAX_CAMERA_HEIGHT au corps à corps -> et peut être créer un MIN_CAMERA_HEIGHT pour les longue distances et modifier le calcul (mettre en fonction) pour assurer le comportement (fonction pour camera a mettre dans un autre fichier?) -> valeurs parametrables par le joueur???.

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f; // TODO: Ajust the value to something not so permissive but still worth for DI
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// ----myCode--- PrimaryActorTick.bCanEverTick = true;
	// ----myCode--- bUseControllerRotationYaw = false;
	// ----myCode--- GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AArenaFighterGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	// ----myCode--- Posture = EPosture::NEUTRAL;

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AArenaFighterGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Perform the BluePrint Tick logic
	BPTick(DeltaTime);

	//UpdatePosture() animation?;

	if (bIsCameraLockedOnEnemy)
	{
		float distance = (lockedOnActor->GetActorLocation() - GetActorLocation()).Size(); // entre 70-100 et 1000-1500 environ -> 70 = collé, 100 = très proche
		// ----distance used to calculate camera Height (Pitch)---
		FRotator lookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), lockedOnActor->GetActorLocation());
		lookAtRotation.Pitch -= (targetingHeighOffset - distance / 100);
		GetController()->SetControlRotation(lookAtRotation);
		UE_LOG(LogTemp, Warning, TEXT("---------------\t\t\tRotation on Enemy Time:\t\t %s.%d"), *FDateTime::Now().ToString(), FDateTime::Now().GetMillisecond());
		UE_LOG(LogTemp, Warning, TEXT("Distance from lockedEnemy : %f\n"), distance);
		FVector MovementVec = GetPendingMovementInputVector();
		UE_LOG(LogTemp, Warning, TEXT("IN TICK -- Pending Input Vector : (%f, %f, %f)"), MovementVec.X, MovementVec.Y, MovementVec.Z);

		MovementVec = GetCharacterMovement()->GetLastInputVector();;
		UE_LOG(LogTemp, Warning, TEXT("IN TICK -- Last Input Vector : (%f, %f, %f)"), MovementVec.X, MovementVec.Y, MovementVec.Z);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AArenaFighterGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AArenaFighterGameCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AArenaFighterGameCharacter::MoveActionStopped);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AArenaFighterGameCharacter::Look);

		//Running
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Triggered, this, &AArenaFighterGameCharacter::StartRunning);
		
		//Stop Running when stop moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AArenaFighterGameCharacter::StopRunning);
		
		//Dash detection
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::Dash);

		//Change Posture detection
		EnhancedInputComponent->BindAction(PostureAction, ETriggerEvent::Triggered, this, &AArenaFighterGameCharacter::PostureActionTriggered);
		EnhancedInputComponent->BindAction(PostureAction, ETriggerEvent::Completed, this, &AArenaFighterGameCharacter::PostureActionStopped);
	
		//Light Attack
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::LightAttack);

		//Heavy Attack
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::HeavyAttack);

		//Special Attack
		EnhancedInputComponent->BindAction(SpecialAttackAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::SpecialAttack);

		//Guard
		EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::Guard);
	
		//BreakGuard
		EnhancedInputComponent->BindAction(BreakGuardAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::BreakGuard);

		//LockUnlockCameraOnEnemy
		EnhancedInputComponent->BindAction(LockUnlockAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::LockUnlockCameraOnEnemy);

		//TakeDamages
		EnhancedInputComponent->BindAction(TakeDamagesAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::StartDamage);

		//Heal
		EnhancedInputComponent->BindAction(HealAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::StartHealing);	
	}
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AArenaFighterGameCharacter::Move(const FInputActionValue& Value)
{
	bIsMoving = true;
	// Change Posture by default movement
	if ( bIsCameraLockedOnCharacterBack && (bIsPostureActionActive == false) )
	{
		ChangePosture(Value);
		UE_LOG(LogTemp, Warning, TEXT("ChangeDefault posture"));
	}
	
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		double TimeUnitToDiviceVelocity = 5.5; // arbitraire mais 5.25 (6.0 Max ?? Min)semble idéal pour velocity 400/800
		double originalXStepSize = MovementVector.X * GetCharacterMovement()->Velocity.Size() / TimeUnitToDiviceVelocity;
		UE_LOG(LogTemp, Warning, TEXT("originalXStepSize : %f"), originalXStepSize);

		if (bIsCameraLockedOnEnemy && originalXStepSize != 0.0)
		{
			// Get the maximum physics substep delta time.
			
			double distance = (lockedOnActor->GetActorLocation() - GetActorLocation()).Size(); // entre 70-100 et 1000-1500 environ -> 70 = collé, 100 = très proche
			UE_LOG(LogTemp, Warning, TEXT("distance : %f"), distance);
			UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed : %f"), GetCharacterMovement()->MaxWalkSpeed);
			UE_LOG(LogTemp, Warning, TEXT("MovementVector.X: %f"), MovementVector.X);
			// double originalXStepSize = (GetCharacterMovement()->GetLastUpdateLocation() - GetActorLocation()).Size();
			UE_LOG(LogTemp, Warning, TEXT("originalXStepSize : %f"), originalXStepSize);
			double radianTargetAngle = originalXStepSize / distance;
			UE_LOG(LogTemp, Warning, TEXT("radianTargetAngle: %f"), radianTargetAngle);
			double angle = FMath::RadiansToDegrees(radianTargetAngle / 2.0);
			UE_LOG(LogTemp, Warning, TEXT("angle : %f"), angle);
			double newStepSize = FMath::Sin(radianTargetAngle / 2.0) * distance * 2.0;
			UE_LOG(LogTemp, Warning, TEXT("newStepSize: %f"), newStepSize);
			
			MovementVector.X = MovementVector.X * newStepSize / originalXStepSize;

			UE_LOG(LogTemp, Warning, TEXT("MovementVector.X: %f"), MovementVector.X);
			UE_LOG(LogTemp, Warning, TEXT("MovementVector.Y: %f"), MovementVector.Y);
			
			angle *= -1.0;

			UE_LOG(LogTemp, Warning, TEXT("angle : %f"), angle);

			UE_LOG(LogTemp, Warning, TEXT("---------------\t\t\tAngle Calcul Time:\t\t %s.%d"), *FDateTime::Now().ToString(), FDateTime::Now().GetMillisecond());
			UE_LOG(LogTemp, Warning, TEXT("Angle value: %f"), angle);
			UE_LOG(LogTemp, Warning, TEXT("Distance from lockedEnemy : %f\n"), distance);

			RightDirection = RightDirection.RotateAngleAxis(angle, FVector::ZAxisVector);
		}

		UE_LOG(LogTemp, Warning, TEXT("MovementVector.X: %f"), MovementVector.X);
		UE_LOG(LogTemp, Warning, TEXT("MovementVector.Y: %f"), MovementVector.Y);
		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
		FVector MovementVec = GetPendingMovementInputVector();
		UE_LOG(LogTemp, Warning, TEXT("IN MOVE -- Pending Input Vector : (%f, %f, %f)"), MovementVec.X, MovementVec.Y, MovementVec.Z);

		MovementVec = GetCharacterMovement()->GetLastInputVector();;
		UE_LOG(LogTemp, Warning, TEXT("IN MOVE -- Last Input Vector : (%f, %f, %f)"), MovementVec.X, MovementVec.Y, MovementVec.Z);
	}
}

void AArenaFighterGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AArenaFighterGameCharacter::UnlockCharacterBackFromCamera()
{
	bUseControllerRotationYaw = false;
	bIsCameraLockedOnCharacterBack = false;
	SetPostureToNeutral();
}

void AArenaFighterGameCharacter::LockCameraOnCharacterBack()
{
	bUseControllerRotationYaw = true;
	bIsCameraLockedOnCharacterBack = true;
}

void AArenaFighterGameCharacter::StartRunning()
{
	UnlockCharacterBackFromCamera();

	// Check if character is already running
	bIsRunning = true;

	GetCharacterMovement()->MaxWalkSpeed = RunningSpeed; // Set running speed
}

void AArenaFighterGameCharacter::StopRunning()
{
	if (bIsCameraLockedOnEnemy)
	{
		LockCameraOnCharacterBack();
	}

	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed; // Reset to walking speed
}

void AArenaFighterGameCharacter::Dash()
{
	// Check if the controller is valid
	if (Controller != nullptr)
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

void AArenaFighterGameCharacter::PostureActionTriggered(const FInputActionValue& Value)
{
	if (bIsCameraLockedOnCharacterBack)
	{
		bIsPostureActionActive = true;
		ChangePosture(Value);
	}
}

void AArenaFighterGameCharacter::ChangePosture(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		float AngleRad = FMath::Atan2(MovementVector.Y, MovementVector.X);  // Get angle in radians [-PI, PI]
		float AngleDeg = FMath::RadiansToDegrees(AngleRad);  // Convert to degrees [-180, 180]
		if (AngleDeg < 0.f) AngleDeg += 360.f;  // Convert to [0, 360]

		UE_LOG(LogTemp, Warning, TEXT("\nAngle not Rounded for Actual Posture : %f\n"), AngleDeg);

		// Normalize angle to [0, 7] and round to nearest whole number
		int RoundedAngle = FMath::RoundToInt(AngleDeg / 45.f) % 8;

		UE_LOG(LogTemp, Warning, TEXT("Angle for Actual Posture : %i\n"), RoundedAngle);

		// Determine the rounded direction
		switch (RoundedAngle)
		{
			case 0:  // Right
				ActualPosture = EPosture::RIGHT;
			case 1:  // UpRight
				ActualPosture = EPosture::UPRIGHT;
			case 2:  // Up
				ActualPosture = EPosture::UP;
			case 3:  // UpLeft
				ActualPosture = EPosture::UPLEFT;
			case 4:  // Left
				ActualPosture = EPosture::LEFT;
			case 5:  // DownLeft
				ActualPosture = EPosture::DOWNLEFT;
			case 6:  // Down
				ActualPosture = EPosture::DOWN;
			case 7:  // DownRight
				ActualPosture = EPosture::DOWNRIGHT;
		}
	}
}

void AArenaFighterGameCharacter::SetPostureToNeutral()
{
	ActualPosture = EPosture::NEUTRAL;
	UE_LOG(LogTemp, Warning, TEXT("---------\nNEUTRAL POSTURE\n---------\n"));
}

void AArenaFighterGameCharacter::MoveActionStopped()
{
	bIsMoving = false;
	if (bIsPostureActionActive == false)
	{
		SetPostureToNeutral();
	}
}

void AArenaFighterGameCharacter::PostureActionStopped()
{
	bIsPostureActionActive = false;
	if (bIsMoving == false)
	{
		SetPostureToNeutral();
	}
}

void AArenaFighterGameCharacter::TryChangePostureByDefaultMovement(const FInputActionValue& Value)
{
	// Change Posture by default movement
	if ( bIsCameraLockedOnCharacterBack && (bIsPostureActionActive == false) )
	{
		ChangePosture(Value);
		UE_LOG(LogTemp, Warning, TEXT("ChangeDefault posture"));
	}
}

bool AArenaFighterGameCharacter::IsEnemy(int id)
{
	return (id == TeamId);
}

bool AArenaFighterGameCharacter::IsEnemy(AArenaFighterGameCharacter *fighter)
{
	return (fighter->GetTeamId() == TeamId);
}

int  AArenaFighterGameCharacter::GetTeamId()
{
	return (TeamId);
}
void AArenaFighterGameCharacter::SetTeamId(int teamId)
{
	TeamId = teamId;
}

void AArenaFighterGameCharacter::LockUnlockCameraOnEnemy()
{
	if (bIsCameraLockedOnEnemy)
	{
		//UnlockCameraFromEnemy
		UE_LOG(LogTemp, Warning, TEXT("CAMERA UNLOCKED\n"));
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

void AArenaFighterGameCharacter::LightAttack() {
	UE_LOG(LogTemp, Warning, TEXT("LightAttack\n"));
}

void AArenaFighterGameCharacter::HeavyAttack() {
	UE_LOG(LogTemp, Warning, TEXT("HeavyAttack\n"));
}


void AArenaFighterGameCharacter::SpecialAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("SpecialAttack\n"));
}

void AArenaFighterGameCharacter::Guard()
{
	UE_LOG(LogTemp, Warning, TEXT("Guard\n"));
}

void AArenaFighterGameCharacter::BreakGuard()
{
	UE_LOG(LogTemp, Warning, TEXT("BreakGuard\n"));
}