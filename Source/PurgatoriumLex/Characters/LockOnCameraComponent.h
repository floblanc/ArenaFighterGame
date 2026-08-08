// Copyright Epic Games, Inc. All Rights Reserved.
//
// WHY THIS COMPONENT (PlayerCharacter decomposition Phase A)
// ----------------------------------------------------------
// Lock-on / "camera on back" is presentation + locomotion *feel*, not combat
// authority. It must not live in FFighterSimState. Keeping it on the god-class
// pawn mixed Tick rules with camera math. This component owns lock-on state
// and look-at updates; the character only asks IsLockedOn* for posture/move.
//
// Camera boom + FollowCamera stay on the pawn (attachment hierarchy).
// Fighting IMC / RequestNeutralPosture stay on the character via delegates.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnCameraComponent.generated.h"

class APlayerCharacter;
class UCameraComponent;
class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLockOnEnemyChanged, bool, bLockedOnEnemy, AActor*, LockedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnBackChanged, bool, bLockedOnBack);

UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class PURGATORIUMLEX_API ULockOnCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULockOnCameraComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock On", Meta = (ClampMin = "0.0"))
	float TargetingHeightOffset = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock On", Meta = (ClampMin = "100", ClampMax = "5000"))
	float LockOnMaxDistance = 2000.f;

	/** Reserved / tune later (candidate filter currently uses screen projection). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock On", Meta = (ClampMin = "5", ClampMax = "90"))
	float LockOnFOVDegrees = 45.f;

	UPROPERTY(BlueprintAssignable, Category = "Lock On|Events")
	FOnLockOnEnemyChanged OnLockOnEnemyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Lock On|Events")
	FOnLockOnBackChanged OnLockOnBackChanged;

	UFUNCTION(BlueprintPure, Category = "Lock On")
	bool IsLockedOnEnemy() const { return bIsLockedOnEnemy; }

	UFUNCTION(BlueprintPure, Category = "Lock On")
	bool IsLockedOnBack() const { return bIsLockedOnBack; }

	UFUNCTION(BlueprintPure, Category = "Lock On")
	AActor* GetLockedOnActor() const { return LockedOnActor; }

	UFUNCTION(BlueprintPure, Category = "Lock On")
	const TArray<AActor*>& GetLockOnCandidates() const { return LockOnCandidates; }

	/**
	 * Toggle enemy lock-on.
	 * @param bAllowOrientToBack  false while sprinting (match old StartRunning / StopRunning rules).
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void ToggleLockOnEnemy(bool bAllowOrientToBack = true);

	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void LockToCharacterBack();

	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void UnlockFromCharacterBack();

	/** Look-at update while locked on an enemy. Safe no-op if not locked. */
	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void UpdateLockOnLookAt();

	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void RefreshLockOnCandidates();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On", meta = (AllowPrivateAccess = "true"))
	bool bIsLockedOnEnemy = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On", meta = (AllowPrivateAccess = "true"))
	bool bIsLockedOnBack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On", meta = (AllowPrivateAccess = "true"))
	AActor* LockedOnActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On", meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> LockOnCandidates;

	APlayerCharacter* GetOwnerPlayer() const;
	UCameraComponent* ResolveFollowCamera() const;
	UCharacterMovementComponent* ResolveMovement() const;
};
