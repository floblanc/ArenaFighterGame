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

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
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
	// ----myCode--- GetCharacterMovement()->MaxWalkSpeed = 150.f;
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




