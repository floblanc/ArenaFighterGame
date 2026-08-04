// Copyright Epic Games, Inc. All Rights Reserved.
//
// REVIEW STEP 4/5 — UE bridge between the pure sim and the game world.
//
// WHY A COMPONENT (not putting TickFrame on APlayerCharacter)
// -----------------------------------------------------------
// PlayerCharacter is already a god-object. Combat fixed-step + input latch +
// presentation events are a separable concern. Later, a match object could own
// two FFighterCombatSim instances without this component; the component is the
// convenient UE attachment for local PIE today.
//
// WHY THE COMPONENT DOES NOT OWN RULES
// ------------------------------------
// It only: accumulates DeltaTime -> N fixed ticks, holds PendingInput, fires
// delegates when state changes. If you find combat if-statements growing here,
// push them down into FFighterCombatSim instead.
//
// WHY EVENTS INSTEAD OF PLAYING MONTAGE INSIDE THE SIM
// ----------------------------------------------------
// Sim must stay free of USkeletalMesh / AnimInstance. Presentation listens
// (C++ character and/or small BP) and may play montage/VFX. That keeps
// "did the attack happen" independent of "did the anim play."
//
// HONEST LIMIT: fixed ticks are still driven by render DeltaTime catch-up.
// Real rollback will push one TickFrame per confirmed/predicted sim frame.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/CombatTypes.h"
#include "Combat/FighterCombatSim.h"
#include "FighterCombatComponent.generated.h"

class ULightAttackMoveSet;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFighterPostureChanged, EPosture, NewPosture, int32, SimFrame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFighterLightAttackStarted, EPosture, SnapshotPosture, UAnimMontage*, Montage, int32, SimFrame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFighterMovePhaseChanged, ECombatMovePhase, NewPhase, int32, SimFrame);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PURGATORIUMLEX_API UFighterCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFighterCombatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * WHY 60: common fighter / rollback default (1 frame ~= 16.67ms).
	 * Change if you want; keep move data expressed in frames at that rate.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Sim", meta = (ClampMin = "1", ClampMax = "240"))
	int32 SimTickRate = 60;

	/** WHY DataAsset ref here: pawn/config chooses which moveset; sim only reads it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Sim")
	TObjectPtr<ULightAttackMoveSet> LightAttackMoveSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Sim|Posture", meta = (ClampMin = "0"))
	int32 PostureBaseFramesDelay = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Sim|Posture", meta = (ClampMin = "0.0"))
	float PosturePenaltyPerChange = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Sim|Posture", meta = (ClampMin = "0.0"))
	float PostureMaxPenalty = 0.5f;

	UPROPERTY(BlueprintAssignable, Category = "Combat Sim|Events")
	FOnFighterPostureChanged OnPostureChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat Sim|Events")
	FOnFighterLightAttackStarted OnLightAttackStarted;

	UPROPERTY(BlueprintAssignable, Category = "Combat Sim|Events")
	FOnFighterMovePhaseChanged OnMovePhaseChanged;

	UFUNCTION(BlueprintPure, Category = "Combat Sim")
	EPosture GetSimPosture() const { return Sim.GetState().Posture; }

	UFUNCTION(BlueprintPure, Category = "Combat Sim")
	ECombatMovePhase GetMovePhase() const { return Sim.GetState().MovePhase; }

	UFUNCTION(BlueprintPure, Category = "Combat Sim")
	int32 GetSimFrame() const { return Sim.GetState().Frame; }

	UFUNCTION(BlueprintPure, Category = "Combat Sim")
	FFighterSimState GetSimStateCopy() const { return Sim.GetState(); }

	UFUNCTION(BlueprintPure, Category = "Combat Sim")
	bool IsAttacking() const { return Sim.GetState().HasFlag(FighterStateFlags::Attacking); }

	/** Character writes intent here; sim consumes on fixed ticks. */
	void SetPostureDirection(const FVector2D& Direction, bool bOverrideActive, bool bAllowMovementPosture);
	void RequestNeutralPosture();
	void PressLightAttack();

	/** WHY: Actor BeginPlay runs after Component BeginPlay — re-push delays/move set. */
	UFUNCTION(BlueprintCallable, Category = "Combat Sim")
	void ApplyConfigToSim();

	const FFighterFrameInput& GetPendingInput() const { return PendingInput; }

private:
	void RunFixedTicks(float DeltaTime);
	void NotifyPresentation(EPosture PrevPosture, ECombatMovePhase PrevPhase, bool bAttackJustStarted, EPosture AttackPosture);

	FFighterCombatSim Sim;
	FFighterFrameInput PendingInput;
	float AccumulatedTime = 0.f;
	EPosture LastBroadcastPosture = EPosture::E_Neutral;
	ECombatMovePhase LastBroadcastPhase = ECombatMovePhase::None;
};
