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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Run Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* RunAction;

	/** Dash Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DashAction;

	/** Posture Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* PostureAction;

	/** LightAttack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LightAttackAction;

	/** HeavyAttack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* HeavyAttackAction;

	/** SpecialAttack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SpecialAttackAction;

	/** Guard Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* GuardAction;

	/** BreakGuard Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* BreakGuardAction;


	/** Lock/Unlock Camera Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LockUnlockAction;

public:
	AArenaFighterGameCharacter();

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

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

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Called for dash input
	void Dash();
	
	// To add mapping context
	virtual void BeginPlay();

	// Called for Posture Action
	void ChangePosture(const FInputActionValue& Value);
	void SetPostureToNeutral();

	void LockCameraOnCharacterBack();
	void UnlockCharacterBackFromCamera();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	float WalkingSpeed; // Default walk speed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	float RunningSpeed; // Default run speed
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsRunning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsCameraLockedOnEnemy;

	// Dash variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	float DashDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
		bool bIsGuarding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
		EPosture ActualPosture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
		bool bIsPostureNeutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
		float PostureDeadZoneSize;
};

