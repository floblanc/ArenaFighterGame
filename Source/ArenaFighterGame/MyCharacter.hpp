#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

UENUM(BlueprintType)
enum class EPosture : uint8
{
    UP UMETA(DisplayName = "UP"),
    DOWN UMETA(DisplayName = "DOWN"),
    LEFT UMETA(DisplayName = "LEFT"),
    RIGHT UMETA(DisplayName = "RIGHT"),
    UPLEFT UMETA(DisplayName = "UPLEFT"),
    UPRIGHT UMETA(DisplayName = "UPRIGHT"),
    DOWNLEFT UMETA(DisplayName = "DOWNLEFT"),
    DOWNRIGHT UMETA(DisplayName = "DOWNRIGHT"),
};

UCLASS()
class YOURPROJECT_API AMyCharacter : public ACharacter
{
    GENERATED_BODY()

private:
    UPROPERTY()
    bool bIsRunning;

    UPROPERTY()
    bool bIsGuarding;

    UPROPERTY()
    EPosture Posture;

public:
    AMyCharacter();

    void MoveForward(float Value); // Move Back = Value < 0.0
    void MoveRight(float Value); // Move Left = Value < 0.0;
    void HandlePostureInputY(float Value);
    void HandlePostureInputX(float Value);

    void LightAttack();
    void HeavyAttack();
    void SpecialAttack();
    void Dash();
    void Guard();
    void BreakGuard();

    void StartRunning();
    void StopRunning();

    void StartGuarding();
    void StopGuarding();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
