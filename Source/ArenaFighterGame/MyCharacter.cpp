#include "MyCharacter.h"
#include "Math/Vector2D.h"

AMyCharacter::AMyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->MaxWalkSpeed = 150.f;
}

void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();
    Posture = EPosture::UP;
}

void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdatePosture();
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
    PlayerInputComponent->BindAxis("PostureForward", this, &AMyCharacter::HandlePostureInputY);
    PlayerInputComponent->BindAxis("PostureRight", this, &AMyCharacter::HandlePostureInputX);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("LightAttack", IE_Pressed, this, &AMyCharacter::LightAttack);
    PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, this, &AMyCharacter::HeavyAttack);
    PlayerInputComponent->BindAction("SpecialAttack", IE_Pressed, this, &AMyCharacter::SpecialAttack);
    PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AMyCharacter::Dash);

    PlayerInputComponent->BindAction("Guard", IE_Pressed, this, &AMyCharacter::StartGuarding);
    PlayerInputComponent->BindAction("Guard", IE_Released, this, &AMyCharacter::StopGuarding);

    PlayerInputComponent->BindAction("BreakGuard", IE_Pressed, this, &AMyCharacter::BreakGuard);

    PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AMyCharacter::StartRunning);
    // Bind other inputs to the corresponding actions
}

void AMyCharacter::MoveForward(float Value)
{
    if ((Controller != NULL) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void AMyCharacter::MoveRight(float Value)
{
    if ((Controller != NULL) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void AMyCharacter::HandlePostureInput(float RightStickInputX, float RightStickInputY, bool IsRightStickNeutral)
{
    RightStickInput = FVector2D(RightStickInputX, RightStickInputY);
}

void AMyCharacter::UpdatePosture()
{
    // Implement how the posture is updated based on RightStickInput and IsRightStickNeutral
}

void AMyCharacter::LightAttack()
{
    // Implement light attack
}

void AMyCharacter::HeavyAttack()
{
    // Implement heavy attack
}

void AMyCharacter::Dash()
{
    // Implement dash
    bIsDashing = true;
}

void AMyCharacter::Guard()
{
    // Implement guard
    bIsGuarding = true;
}

void AMyCharacter::BreakGuard()
{
    // Implement guard break
    bIsGuarding = false;
}
