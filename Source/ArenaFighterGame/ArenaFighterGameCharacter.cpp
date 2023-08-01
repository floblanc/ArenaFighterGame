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
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	bIsRunning = false;
	WalkingSpeed = 400.f;
	RunningSpeed = 800.f;

	DashDistance = 1500.0f;
	DashTimeout = 0.5f;
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
		EnhancedInputComponent->BindAction(TapMoveAction, ETriggerEvent::Triggered, this, &AArenaFighterGameCharacter::CheckDoubleTapToDash);

		//Change Posture detection
		EnhancedInputComponent->BindAction(PostureAction, ETriggerEvent::Triggered, this, &AArenaFighterGameCharacter::ChangePosture);
		EnhancedInputComponent->BindAction(PostureAction, ETriggerEvent::Completed, this, &AArenaFighterGameCharacter::SetPostureToNeutral);
	}
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// ----myCode-- PlayerInputComponent->BindAxis("MoveForward", this, &AArenaFighterGameCharacter::MoveForward);
	// ----myCode-- PlayerInputComponent->BindAxis("MoveRight", this, &AArenaFighterGameCharacter::MoveRight);
	// ----myCode-- PlayerInputComponent->BindAxis("PostureForward", this, &AArenaFighterGameCharacter::HandlePostureInputY);
	// ----myCode-- PlayerInputComponent->BindAxis("PostureRight", this, &AArenaFighterGameCharacter::HandlePostureInputX);
	// ----myCode-- 
	// ----myCode-- PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	// ----myCode-- PlayerInputComponent->BindAction("LightAttack", IE_Pressed, this, &AArenaFighterGameCharacter::LightAttack);
	// ----myCode-- PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, this, &AArenaFighterGameCharacter::HeavyAttack);
	// ----myCode-- PlayerInputComponent->BindAction("SpecialAttack", IE_Pressed, this, &AArenaFighterGameCharacter::SpecialAttack);
	// ----myCode-- PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AArenaFighterGameCharacter::Dash);
	// ----myCode-- 
	// ----myCode-- PlayerInputComponent->BindAction("Guard", IE_Pressed, this, &AArenaFighterGameCharacter::StartGuarding);
	// ----myCode-- PlayerInputComponent->BindAction("Guard", IE_Released, this, &AArenaFighterGameCharacter::StopGuarding);
	// ----myCode-- 
	// ----myCode-- PlayerInputComponent->BindAction("BreakGuard", IE_Pressed, this, &AArenaFighterGameCharacter::BreakGuard);
	// ----myCode-- 
	// ----myCode-- PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AArenaFighterGameCharacter::StartRunning);
	// ----myCode-- // Bind other inputs to the corresponding actions
}

void AArenaFighterGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if (IsPostureNeutral)
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

void AArenaFighterGameCharacter::StartRunning()
{
	// Check if character is already running
	if (bIsRunning)
	{
		StopRunning();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("START RUNNING\n"));
		bIsRunning = true;
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed; // Set running speed
	}
}

void AArenaFighterGameCharacter::StopRunning()
{
	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed; // Reset to walking speed
}

void AArenaFighterGameCharacter::Dash(const FVector2D& MoveDirection)
{
	if (Controller != nullptr)
	{
		// Round the input to get a unit vector
		FVector2D RoundedVector = MoveDirection.GetSafeNormal();

		// Find out which way is forward and right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Construct the dash vector using the controller's forward and right vectors
		FVector DashVector = ForwardDirection * RoundedVector.Y + RightDirection * RoundedVector.X;
		DashVector.Normalize();
		DashVector *= DashDistance;

		// Apply the impulse
		GetCharacterMovement()->AddImpulse(DashVector, true);
	}
}

//void AArenaFighterGameCharacter::Dash(const FVector2D& MoveDirection)
//{
//	if (Controller != nullptr)
//	{
//		FVector2D RoundedVector = MoveDirection.RoundToVector();
//	
//		FVector RoundedDashVector(RoundedVector.X, RoundedVector.Y, 0.0f);
//
//		// Create a vector that represents the direction in which to dash
//		FVector DashVector = RoundedDashVector * DashDistance;
//
//		// find out which way is forward
//		const FRotator Rotation = Controller->GetControlRotation();
//		const FRotator YawRotation(0, Rotation.Yaw, 0);
//
//		// get forward vector
//		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
//
//		// get right vector 
//		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
//
//		DashVector += ForwardDirection + RightDirection;
//
//		// Move the character
//		GetCharacterMovement()->AddImpulse(DashVector, true);
//	}
//}

void AArenaFighterGameCharacter::CheckDoubleTapToDash(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	MovementVector.Normalize();
	UE_LOG(LogTemp, Warning, TEXT("-------------------------------------------------------\n"));
	UE_LOG(LogTemp, Warning, TEXT("MovementVector.X = %f | MovementVector.Y = %f\n"), MovementVector.X , MovementVector.Y);
	UE_LOG(LogTemp, Warning, TEXT("LastMoveDirection.X = %f | LastMoveDirection.Y = %f\n"), LastMoveDirection.X , LastMoveDirection.Y);
	UE_LOG(LogTemp, Warning, TEXT("Distance entre les 2 = %f\n"), FVector2D::Distance(MovementVector , LastMoveDirection));
	UE_LOG(LogTemp, Warning, TEXT("Distance carré entre les 2 = %f\n"), FVector2D::DistSquared(MovementVector , LastMoveDirection));
	UE_LOG(LogTemp, Warning, TEXT("-------------------------------------------------------\n"));
	if (MovementVector.Equals(LastMoveDirection, 0.80f)) // Adjust the tolerance as needed
	{
		DashCounter++;
		if (DashCounter == 2)
		{
			Dash(MovementVector);
			UE_LOG(LogTemp, Warning, TEXT("Dash Done!!\n"));
		}
	}
	else
	{
		LastMoveDirection = MovementVector;
		UE_LOG(LogTemp, Warning, TEXT("Double Tap too far from each other!!\n"));
	}
	GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, this, &AArenaFighterGameCharacter::ResetDashCounter, DashTimeout);
}

void AArenaFighterGameCharacter::ResetDashCounter()
{
	DashCounter = 0;
	UE_LOG(LogTemp, Warning, TEXT("Counter Reset!!\n"));
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
					ActualPosture = EPosture::UP;
			}
		}
	}

void AArenaFighterGameCharacter::SetPostureToNeutral()
{
	ActualPosture = EPosture::NEUTRAL;
	IsPostureNeutral = true;
}