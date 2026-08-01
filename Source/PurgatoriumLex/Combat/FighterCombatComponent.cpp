// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FighterCombatComponent.h"
#include "Combat/LightAttackMoveSet.h"
#include "Animation/AnimMontage.h"

UFighterCombatComponent::UFighterCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UFighterCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	FFighterSimState Initial;
	Initial.Posture = EPosture::E_Neutral;
	Initial.PostureBaseFramesDelay = PostureBaseFramesDelay;
	Initial.PosturePenaltyPerChange = PosturePenaltyPerChange;
	Initial.PostureMaxPenalty = PostureMaxPenalty;
	Initial.Health = 100.f;
	Sim.Reset(Initial);
	Sim.SetMoveSet(LightAttackMoveSet);

	LastBroadcastPosture = Initial.Posture;
	LastBroadcastPhase = ECombatMovePhase::None;
}

void UFighterCombatComponent::ApplyConfigToSim()
{
	FFighterSimState& S = Sim.GetStateMutable();
	S.PostureBaseFramesDelay = PostureBaseFramesDelay;
	S.PosturePenaltyPerChange = PosturePenaltyPerChange;
	S.PostureMaxPenalty = PostureMaxPenalty;
	Sim.SetMoveSet(LightAttackMoveSet);
}

void UFighterCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RunFixedTicks(DeltaTime);
}

void UFighterCombatComponent::SetPostureStick(const FVector2D& Stick, bool bOverrideActive, bool bAllowMovementPosture)
{
	PendingInput.PostureStick = Stick;
	PendingInput.bPostureOverrideActive = bOverrideActive;
	PendingInput.bAllowMovementPosture = bAllowMovementPosture;
}

void UFighterCombatComponent::RequestNeutralPosture()
{
	PendingInput.bRequestNeutralPosture = true;
}

void UFighterCombatComponent::PressLightAttack()
{
	PendingInput.bLightAttackPressed = true;
}

void UFighterCombatComponent::RunFixedTicks(float DeltaTime)
{
	const float FixedDt = 1.f / static_cast<float>(FMath::Max(1, SimTickRate));
	AccumulatedTime += DeltaTime;

	// WHY a catch-up cap: a long hitch would otherwise run hundreds of sim frames
	// in one go (spiral of death). Local play prefers a soft reset over melting.
	// Rollback netcode will not use this path the same way.
	const int32 MaxCatchUpFrames = FMath::Max(1, SimTickRate / 4);
	int32 FramesThisUpdate = 0;

	while (AccumulatedTime >= FixedDt && FramesThisUpdate < MaxCatchUpFrames)
	{
		AccumulatedTime -= FixedDt;
		++FramesThisUpdate;

		const EPosture PostureBefore = Sim.GetState().Posture;
		const ECombatMovePhase PhaseBefore = Sim.GetState().MovePhase;
		const bool bWasAttacking = Sim.GetState().HasFlag(FighterStateFlags::Attacking);

		Sim.TickFrame(PendingInput);

		const bool bAttackJustStarted =
			!bWasAttacking && Sim.GetState().HasFlag(FighterStateFlags::Attacking);
		const EPosture AttackPosture = Sim.GetState().AttackSnapshotPosture;

		// WHY clear edges after one sim tick: a render frame may cover multiple
		// sim ticks; without clearing, one physical press would fire N attacks.
		PendingInput.bLightAttackPressed = false;
		PendingInput.bRequestNeutralPosture = false;

		NotifyPresentation(PostureBefore, PhaseBefore, bAttackJustStarted, AttackPosture);
	}

	if (FramesThisUpdate >= MaxCatchUpFrames)
	{
		AccumulatedTime = 0.f;
	}
}

void UFighterCombatComponent::NotifyPresentation(
	EPosture PrevPosture,
	ECombatMovePhase PrevPhase,
	bool bAttackJustStarted,
	EPosture AttackPosture)
{
	const FFighterSimState& S = Sim.GetState();

	if (S.Posture != LastBroadcastPosture)
	{
		LastBroadcastPosture = S.Posture;
		OnPostureChanged.Broadcast(S.Posture, S.Frame);
	}

	if (S.MovePhase != LastBroadcastPhase)
	{
		LastBroadcastPhase = S.MovePhase;
		OnMovePhaseChanged.Broadcast(S.MovePhase, S.Frame);
	}

	if (bAttackJustStarted)
	{
		UAnimMontage* Montage = nullptr;
		if (LightAttackMoveSet)
		{
			const FLightAttackMoveDef Def = LightAttackMoveSet->FindMove(AttackPosture);
			if (!Def.Montage.IsNull())
			{
				// Sync load is OK for prototype; replace with async / preloaded refs later.
				Montage = Def.Montage.LoadSynchronous();
			}
		}
		OnLightAttackStarted.Broadcast(AttackPosture, Montage, S.Frame);
	}

	// Silence unused in non-delegate paths
	(void)PrevPosture;
	(void)PrevPhase;
}
