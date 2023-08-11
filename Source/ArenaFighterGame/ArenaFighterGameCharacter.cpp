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


//////////////////////////////////////////////////////////////////////////
// AArenaFighterGameCharacter

AArenaFighterGameCharacter::AArenaFighterGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	bIsRunning = false;
	WalkingSpeed = 400.f;
	RunningSpeed = 800.f;

	DashDistance = 1000.0f;
	PostureDeadZoneSize = 0.25f;

	SetPostureToNeutral();
	
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f; 
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
	// ----myCode--- Posture = EPosture::UP;

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

// ----myCode-- void AArenaFighterGameCharacter::Tick(float DeltaTime)
// ----myCode-- {
// ----myCode-- 	Super::Tick(DeltaTime);
// ----myCode-- 	UpdatePosture();
// ----myCode-- }

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
		
		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AArenaFighterGameCharacter::Look);

		//Running
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Triggered, this, &AArenaFighterGameCharacter::StartRunning);
		
		//Stop Running when stop moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AArenaFighterGameCharacter::StopRunning);
		
		//Dash detection
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::Dash);

		//Change Posture detection
		EnhancedInputComponent->BindAction(PostureAction, ETriggerEvent::Triggered, this, &AArenaFighterGameCharacter::ChangePosture);
		EnhancedInputComponent->BindAction(PostureAction, ETriggerEvent::Completed, this, &AArenaFighterGameCharacter::SetPostureToNeutral);
	
		//Light Attack
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::LightAttack);

		//Heavy Attack
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::HeavyAttack);

		//Special Attack
		EnhancedInputComponent->BindAction(SpecialAttackAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::SpecialAttack);

		//Guard
		EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::Guard);
	
		//GuardBreak
		EnhancedInputComponent->BindAction(BreakGuardAction, ETriggerEvent::Started, this, &AArenaFighterGameCharacter::BreakGuard);
	}
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AArenaFighterGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if (IsPostureNeutral) //OU INPUT RELIÉ A L'ACTION DE POSTURE MAIS AVEC UNE PRIORITÉ MOINDRE
		{
			ChangePosture(Value);
		}

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
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

void AArenaFighterGameCharacter::UnlockCameraFromCharacterBack()
{
	bUseControllerRotationYaw = false;
}

void AArenaFighterGameCharacter::LockCameraToCharacterBack()
{
	bUseControllerRotationYaw = true;
}

void AArenaFighterGameCharacter::StartRunning()
{
	UnlockCameraFromCharacterBack();

	// Check if character is already running
	bIsRunning = true;

	GetCharacterMovement()->MaxWalkSpeed = RunningSpeed; // Set running speed
}

void AArenaFighterGameCharacter::StopRunning()
{
	if (true) // TODO: In future, change true with check isCameraLocked
	{
		LockCameraToCharacterBack();
	}

	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed; // Reset to walking speed
}

void AArenaFighterGameCharacter::Dash()
{
	// Check if the controller is valid
	if (Controller == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Controller is nullptr. Cannot perform dash."));
		return;
	}

	// Get the last movement input vector
	FVector MovementVector = GetLastMovementInputVector();

	if (MovementVector.IsNearlyZero()) // If there's no movement input, set default to forward
	{
		MovementVector = FVector(1.f, 0.f, 0.f);
	}

	// Normalize the vector
	MovementVector.Normalize();

	// Snap to the closest cardinal direction
	if (FMath::Abs(MovementVector.X) > FMath::Abs(MovementVector.Y))
	{
		MovementVector.Y = 0.f;
	}
	else
	{
		MovementVector.X = 0.f;
	}

	FVector DashVector = MovementVector * DashDistance;
	
	// Apply the impulse to the character movement component
	GetCharacterMovement()->AddImpulse(DashVector, true);

	UE_LOG(LogTemp, Warning, TEXT("Dashing!"));
}

void AArenaFighterGameCharacter::ChangePosture(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	IsPostureNeutral = false;

	if (Controller != nullptr)
	{
		if (MovementVector.Size() < PostureDeadZoneSize)  // Consider input threshold as per your requirement
		{
			// Input is neutral
			SetPostureToNeutral();
			return ;
		}

		float AngleRad = FMath::Atan2(MovementVector.Y, MovementVector.X);  // Get angle in radians [-PI, PI]
		float AngleDeg = FMath::RadiansToDegrees(AngleRad);  // Convert to degrees [-180, 180]
		if (AngleDeg < 0.f) AngleDeg += 360.f;  // Convert to [0, 360]

		// Normalize angle to [0, 8] and round to nearest whole number
		int RoundedAngle = FMath::RoundToInt(AngleDeg / 45.f);

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
			default:  // Case 8 wraps around to Up
				ActualPosture = EPosture::NEUTRAL;
		}
	}
}

void AArenaFighterGameCharacter::SetPostureToNeutral()
{
	ActualPosture = EPosture::NEUTRAL;
	IsPostureNeutral = true;
}

void AArenaFighterGameCharacter::LightAttack() {}
void AArenaFighterGameCharacter::HeavyAttack() {}
void AArenaFighterGameCharacter::SpecialAttack() {}
void AArenaFighterGameCharacter::Guard() {}
void AArenaFighterGameCharacter::BreakGuard() {}
