// Copyright Epic Games, Inc. All Rights Reserved.
//
// REVIEW STEP 3/5 — Pure fixed-tick fighter sim (no Actor, no world time).
//
// WHY A NON-UOBJECT CLASS
// -----------------------
// Combat rules should be testable and rewindable without UE actor lifetime,
// GC, or latent abilities. FFighterCombatSim only mutates FFighterSimState
// through TickFrame(Input). Same inputs + same state => same next state
// (goal; we still need discipline elsewhere — CMC, floats, etc.).
//
// WHY TickFrame IS THE ONLY MUTATION ENTRY
// ----------------------------------------
// Rollback resim calls the same function with recorded inputs. If character
// code also mutates posture/attack phase "on the side," resim cannot rebuild
// truth. Character should only *feed input* and *read state* for presentation.
//
// WHY HITBOXES ARE NOT HERE YET
// -----------------------------
// Phase machine first. Collision belongs in Active frames next — intentionally
// not in AnimNotify as authority (notifies are presentation clocks).

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTypes.h"
#include "Combat/LightAttackMoveSet.h"

class PURGATORIUMLEX_API FFighterCombatSim
{
public:
	FFighterCombatSim() = default;

	void Reset(const FFighterSimState& InitialState);
	void SetMoveSet(const ULightAttackMoveSet* InMoveSet);

	const FFighterSimState& GetState() const { return State; }
	FFighterSimState& GetStateMutable() { return State; }

	/**
	 * Advance exactly one sim frame.
	 * @return true if posture or move phase changed (cheap signal for presentation).
	 */
	bool TickFrame(const FFighterFrameInput& Input);

	/** Debug aid for future desync hunting — not a networking protocol. */
	uint32 ComputeStateHash() const;

private:
	void TickPosture(const FFighterFrameInput& Input);
	bool RequestPostureChange(EPosture Target);
	void ProcessPendingPostureChange();
	void TickPostureStalingReset();

	void TickLightAttack(const FFighterFrameInput& Input);
	void StartLightAttack(EPosture SnapshotPosture);
	void AdvanceCurrentMove();
	void EndCurrentMove();

	bool CanStartLightAttack() const;

	FFighterSimState State;

	/**
	 * WHY weak ptr to move set:
	 * Definitions are editor data (UObject). Sim must not own/keep them alive;
	 * if the asset is GC'd or unset we fall back to hardcoded timings.
	 */
	TWeakObjectPtr<const ULightAttackMoveSet> MoveSet;
};
