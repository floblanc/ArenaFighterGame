// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ArenaFighterGameCharacter.generated.h"

UENUM(BlueprintType)
enum class EPosture : uint8
{
    NEUTRAL UMETA(DisplayName = "NEUTRAL"),
    UP UMETA(DisplayName = "UP"),
    DOWN UMETA(DisplayName = "DOWN"),
    LEFT UMETA(DisplayName = "LEFT"),
    RIGHT UMETA(DisplayName = "RIGHT"),
    UPLEFT UMETA(DisplayName = "UPLEFT"),
    UPRIGHT UMETA(DisplayName = "UPRIGHT"),
    DOWNLEFT UMETA(DisplayName = "DOWNLEFT"),
    DOWNRIGHT UMETA(DisplayName = "DOWNRIGHT"),
};

UCLASS(config=Game)
class AArenaFighterGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	AArenaFighterGameCharacter();

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
	void HandlePostureInputY(float Value);
	void HandlePostureInputX(float Value);

	void LightAttack();
	void HeavyAttack();
	void SpecialAttack();
	
	void Guard();
	void BreakGuard();

	void StartRunning();
	void StopRunning();

	void StartGuarding();
	void StopGuarding();

	void LockUnlockCameraOnEnemy();

	/** MappingContext Default*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputMappingContext* DefaultMappingContext;

	/** MappingContext Fighting*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputMappingContext* FightingMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* LookAction;

	/** Run Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* RunAction;

	/** Dash Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* DashAction;

	/** Posture Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* PostureAction;

	/** LightAttack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* LightAttackAction;

	/** HeavyAttack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* HeavyAttackAction;

	/** SpecialAttack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* SpecialAttackAction;

	/** Guard Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* GuardAction;

	/** BreakGuard Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* BreakGuardAction;

	/** Lock/Unlock Camera Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* LockUnlockAction;
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	void MoveActionStopped();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Called for dash input
	void Dash();
	
	// To add mapping context
	virtual void BeginPlay();

	// Called every frame
	virtual void Tick(float deltaTime) override;

	//The BluePrint Tick's
	UFUNCTION(BlueprintImplementableEvent, Category = "Tick")
	void BPTick(float DeltaTime);

	// Called for Posture Action
	void PostureActionTriggered(const FInputActionValue& Value);
	void ChangePosture(const FInputActionValue& Value);
	void SetPostureToNeutral();
	void PostureActionStopped();
	void TryChangePostureByDefaultMovement(const FInputActionValue& Value);

	void LockCameraOnCharacterBack();
	void UnlockCharacterBackFromCamera();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float WalkingSpeed; // Default walk speed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float RunningSpeed; // Default run speed
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsRunning;

	// Dash variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float DashDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsGuarding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	EPosture ActualPosture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsPostureActionActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	bool bIsCameraLockedOnEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	bool bIsCameraLockedOnCharacterBack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	TArray<AActor*> lockOnCandidates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	AActor* lockedOnActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	float targetingHeighOffset;
};

