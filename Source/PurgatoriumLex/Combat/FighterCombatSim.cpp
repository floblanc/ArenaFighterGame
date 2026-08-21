// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FighterCombatSim.h"

DEFINE_LOG_CATEGORY_STATIC(LogFighterCombatSim, Log, All);

void FFighterCombatSim::Reset(const FFighterSimState& InitialState)
{
	State = InitialState;
	State.Frame = 0;
	State.MovePhase = ECombatMovePhase::None;
	State.MoveLocalFrame = 0;
	State.ClearFlag(FighterStateFlags::Attacking);
	State.ClearFlag(FighterStateFlags::PostureChanging);
}

void FFighterCombatSim::SetMoveSet(const ULightAttackMoveSet* InMoveSet)
{
	MoveSet = InMoveSet;
}

void FFighterCombatSim::ApplyPostureConfig(
	int32 BaseDelayFrames,
	int32 BonusDelayFrames,
	float PenaltyPerChange,
	float MaxPenalty,
	int32 ResetFrames)
{
	State.PostureBaseFramesDelay = BaseDelayFrames;
	State.PostureBonusFramesDelay = BonusDelayFrames;
	State.PosturePenaltyPerChange = PenaltyPerChange;
	State.PostureMaxPenalty = MaxPenalty;
	State.PostureResetFrames = ResetFrames;
}

bool FFighterCombatSim::TickFrame(const FFighterFrameInput& Input)
{
	const EPosture PostureBefore = State.Posture;
	const ECombatMovePhase PhaseBefore = State.MovePhase;

	++State.Frame;

	// Order is a design choice you can dispute:
	// 1) expire staling, 2) apply posture, 3) start/advance attacks.
	// WHY posture before attack start: light attack snapshots posture *this frame*
	// after stick processing, so a same-frame posture+attack reads the new stance
	// only if the delay is 0; with delay>0, snapshot is still the old posture
	// until the pending change applies — usually what you want for fairness.
	TickPostureStalingReset();
	TickPosture(Input);
	TickLightAttack(Input);

	return PostureBefore != State.Posture || PhaseBefore != State.MovePhase;
}

uint32 FFighterCombatSim::ComputeStateHash() const
{
	// Simple deterministic mix — enough for debug, replace with a real serializer later.
	uint32 Hash = 2166136261u;
	auto Mix = [&Hash](uint32 V)
	{
		Hash ^= V;
		Hash *= 16777619u;
	};

	Mix(static_cast<uint32>(State.Frame));
	Mix(static_cast<uint32>(State.Posture));
	Mix(static_cast<uint32>(State.PendingPosture));
	Mix(static_cast<uint32>(State.PostureChangeRequestFrame));
	Mix(static_cast<uint32>(State.MovePhase));
	Mix(static_cast<uint32>(State.AttackSnapshotPosture));
	Mix(static_cast<uint32>(State.MoveLocalFrame));
	Mix(static_cast<uint32>(State.StateFlags));
	{
		uint32 HealthBits = 0;
		FMemory::Memcpy(&HealthBits, &State.Health, sizeof(HealthBits));
		Mix(HealthBits);
	}
	return Hash;
}

// ---------------------------------------------------------------------------
// Posture
// ---------------------------------------------------------------------------

void FFighterCombatSim::TickPosture(const FFighterFrameInput& Input)
{
	ProcessPendingPostureChange();

	if (Input.bRequestNeutralPosture)
	{
		RequestPostureChange(EPosture::E_Neutral);
		return;
	}

	const bool bCanDrivePosture =
		Input.bPostureOverrideActive ||
		(Input.bAllowMovementPosture && !Input.bPostureOverrideActive);

	if (!bCanDrivePosture)
	{
		return;
	}

	// Dedicated posture direction wins; otherwise movement direction (caller fills PostureDirection).
	if (Input.PostureDirection.SizeSquared() < 0.01f)
	{
		return;
	}

	const EPosture Target = FCombatPostureMath::PostureFromDirection(Input.PostureDirection);
	RequestPostureChange(Target);
}

bool FFighterCombatSim::RequestPostureChange(EPosture Target)
{
	// WHY ignore if a change is already pending:
	// Matches your previous character rule — prevents stick noise from queueing
	// a chain of delayed transitions. Tradeoff: fast flicks during delay are dropped.
	if (State.PostureChangeRequestFrame >= 0)
	{
		return false;
	}
	if (Target == State.Posture)
	{
		return false;
	}

	State.PostureStalePenalty = FMath::Min(
		State.PostureStalePenalty + State.PosturePenaltyPerChange,
		State.PostureMaxPenalty);
	State.LastPostureChangeFrame = State.Frame;

	const int32 TotalDelay = State.PostureBaseFramesDelay + State.PostureBonusFramesDelay;
	if (TotalDelay > 0)
	{
		State.PendingPosture = Target;
		State.PostureChangeRequestFrame = State.Frame;
		State.SetFlag(FighterStateFlags::PostureChanging);
		UE_LOG(LogFighterCombatSim, Verbose,
			TEXT("[Sim] Queue posture %d -> %d (delay %d, frame %d)"),
			(int32)State.Posture, (int32)Target, TotalDelay, State.Frame);
		return true;
	}

	State.Posture = Target;
	State.ClearFlag(FighterStateFlags::PostureChanging);
	UE_LOG(LogFighterCombatSim, Log, TEXT("[Sim] Posture now %d (frame %d)"), (int32)State.Posture, State.Frame);
	return true;
}

void FFighterCombatSim::ProcessPendingPostureChange()
{
	if (State.PostureChangeRequestFrame < 0)
	{
		return;
	}

	const int32 TotalDelay = State.PostureBaseFramesDelay + State.PostureBonusFramesDelay;
	const int32 Elapsed = State.Frame - State.PostureChangeRequestFrame;
	if (Elapsed >= TotalDelay)
	{
		State.Posture = State.PendingPosture;
		State.PostureChangeRequestFrame = -1;
		State.PendingPosture = EPosture::E_Neutral;
		State.ClearFlag(FighterStateFlags::PostureChanging);
		UE_LOG(LogFighterCombatSim, Log, TEXT("[Sim] Applied posture %d (frame %d)"), (int32)State.Posture, State.Frame);
	}
}

void FFighterCombatSim::TickPostureStalingReset()
{
	if (State.PostureStalePenalty <= 0.f || State.LastPostureChangeFrame < 0)
	{
		return;
	}
	if (State.Frame - State.LastPostureChangeFrame >= State.PostureResetFrames)
	{
		State.PostureStalePenalty = 0.f;
		State.LastPostureChangeFrame = -1;
	}
}

// ---------------------------------------------------------------------------
// Light attack (timing only — no hit detection yet)
// ---------------------------------------------------------------------------

bool FFighterCombatSim::CanStartLightAttack() const
{
	if (State.HasFlag(FighterStateFlags::Attacking))
	{
		return false;
	}
	if (State.HitstunRemaining > 0 || State.HasFlag(FighterStateFlags::Hitstun))
	{
		return false;
	}
	// Optional: block while posture is mid-transition. Uncomment if you want that rule.
	// if (State.HasFlag(FighterStateFlags::PostureChanging)) return false;
	return true;
}

void FFighterCombatSim::TickLightAttack(const FFighterFrameInput& Input)
{
	if (State.HitstunRemaining > 0)
	{
		--State.HitstunRemaining;
		if (State.HitstunRemaining <= 0)
		{
			State.ClearFlag(FighterStateFlags::Hitstun);
			State.MovePhase = ECombatMovePhase::None;
		}
		return;
	}

	if (State.HasFlag(FighterStateFlags::Attacking))
	{
		AdvanceCurrentMove();
		return;
	}

	if (Input.bLightAttackPressed && CanStartLightAttack())
	{
		// WHY snapshot State.Posture (not stick): continuous posture can keep
		// updating for visuals; the move's combat identity is locked at press
		// (For Honor / KCD-style oriented attacks).
		StartLightAttack(State.Posture);
	}
}

void FFighterCombatSim::StartLightAttack(EPosture SnapshotPosture)
{
	FLightAttackMoveDef Def;
	if (const ULightAttackMoveSet* Set = MoveSet.Get())
	{
		Def = Set->FindMove(SnapshotPosture);
	}
	else
	{
		Def = FLightAttackMoveDef{};
		Def.Posture = SnapshotPosture;
		Def.StartupFrames = 8;
		Def.ActiveFrames = 3;
		Def.RecoveryFrames = 12;
		Def.Damage = 10.f;
		UE_LOG(LogFighterCombatSim, Warning,
			TEXT("[Sim] No LightAttackMoveSet assigned — using hardcoded fallback timings."));
	}

	State.AttackSnapshotPosture = SnapshotPosture;
	State.StartupFrames = Def.StartupFrames;
	State.ActiveFrames = Def.ActiveFrames;
	State.RecoveryFrames = Def.RecoveryFrames;
	State.PendingAttackDamage = Def.Damage;
	State.MoveLocalFrame = 0;
	State.MovePhase = ECombatMovePhase::Startup;
	State.SetFlag(FighterStateFlags::Attacking);

	UE_LOG(LogFighterCombatSim, Warning,
		TEXT("[Sim] LightAttack START posture=%d startup=%d active=%d recovery=%d dmg=%.1f frame=%d"),
		(int32)SnapshotPosture, State.StartupFrames, State.ActiveFrames, State.RecoveryFrames,
		State.PendingAttackDamage, State.Frame);
}

void FFighterCombatSim::AdvanceCurrentMove()
{
	++State.MoveLocalFrame;

	const int32 StartupEnd = State.StartupFrames;
	const int32 ActiveEnd = StartupEnd + State.ActiveFrames;
	const int32 RecoveryEnd = ActiveEnd + State.RecoveryFrames;

	if (State.MoveLocalFrame < StartupEnd)
	{
		State.MovePhase = ECombatMovePhase::Startup;
	}
	else if (State.MoveLocalFrame < ActiveEnd)
	{
		if (State.MovePhase != ECombatMovePhase::Active)
		{
			UE_LOG(LogFighterCombatSim, Log,
				TEXT("[Sim] LightAttack ACTIVE (hitboxes would run here) frame=%d local=%d"),
				State.Frame, State.MoveLocalFrame);
		}
		State.MovePhase = ECombatMovePhase::Active;
		// WHY here (not AnimNotify as authority):
		// Notifies are good presentation clocks but bad rollback clocks (mesh/anim
		// time can diverge). Next PR should run collision from this phase using
		// AttackSnapshotPosture; notifies may still spawn VFX.
	}
	else if (State.MoveLocalFrame < RecoveryEnd)
	{
		State.MovePhase = ECombatMovePhase::Recovery;
	}
	else
	{
		EndCurrentMove();
	}
}

void FFighterCombatSim::EndCurrentMove()
{
	UE_LOG(LogFighterCombatSim, Log, TEXT("[Sim] LightAttack END frame=%d"), State.Frame);
	State.MovePhase = ECombatMovePhase::None;
	State.MoveLocalFrame = 0;
	State.StartupFrames = 0;
	State.ActiveFrames = 0;
	State.RecoveryFrames = 0;
	State.PendingAttackDamage = 0.f;
	State.ClearFlag(FighterStateFlags::Attacking);
}
