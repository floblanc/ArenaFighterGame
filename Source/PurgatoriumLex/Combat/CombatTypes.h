// Copyright Epic Games, Inc. All Rights Reserved.
//
// REVIEW STEP 1/5 — Shared combat types (no behavior).
//
// WHY THIS FILE EXISTS
// --------------------
// Combat rules need a single "vocabulary" (posture, phases, input, state) that:
//   1) AnimBP / Blueprints can read for presentation,
//   2) the sim can mutate as authority,
//   3) a future rollback layer can copy/hash without pulling in Actors.
//
// WHY PLAIN STRUCTS (not Actor members as authority)
// -------------------------------------------------
// If posture/HP/attack phase live only on APlayerCharacter as ad-hoc bools,
// you cannot restore a fighter to frame N without replaying the whole world.
// Putting the rewindable bits in FFighterSimState is the architectural bet:
//   presentation follows state; state does not follow the mesh/AnimBP.
//
// Read order: this file -> LightAttackMoveSet -> FighterCombatSim -> Component -> PlayerCharacter.

#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

// ---------------------------------------------------------------------------
// Posture
// ---------------------------------------------------------------------------
// WHY a UENUM (not a free int / string tags only):
//   - AnimBP already blends idle/walk overlays from this enum.
//   - Combat (attack snapshot, later block match) must use the SAME type.
// WHY moved here from PlayerCharacter.h:
//   - Sim must not include the whole character to know posture.
//   - UE Blueprint path stays /Script/PurgatoriumLex.EPosture (module + name).

UENUM(BlueprintType)
enum class EPosture : uint8
{
	E_Neutral   UMETA(DisplayName = "NEUTRAL"),
	E_Up        UMETA(DisplayName = "UP"),
	E_Down      UMETA(DisplayName = "DOWN"),
	E_Left      UMETA(DisplayName = "LEFT"),
	E_Right     UMETA(DisplayName = "RIGHT"),
	E_DownLeft  UMETA(DisplayName = "DOWNLEFT"),
	E_DownRight UMETA(DisplayName = "DOWNRIGHT"),
};

// ---------------------------------------------------------------------------
// Move phase
// ---------------------------------------------------------------------------
// WHY startup / active / recovery (instead of "montage notify did something"):
//   - Fighter feel and rollback both need frame budgets independent of anim length.
//   - Active is the only phase that should ever run hit detection (next PR).
//   - Hitstun/Blockstun listed early so state machine shape is visible; unused until hits exist.

UENUM(BlueprintType)
enum class ECombatMovePhase : uint8
{
	None      UMETA(DisplayName = "None"),
	Startup   UMETA(DisplayName = "Startup"),
	Active    UMETA(DisplayName = "Active"),
	Recovery  UMETA(DisplayName = "Recovery"),
	Hitstun   UMETA(DisplayName = "Hitstun"),
	Blockstun UMETA(DisplayName = "Blockstun"),
};

// ---------------------------------------------------------------------------
// Sim-owned flags ("tags" without ASC)
// ---------------------------------------------------------------------------
// WHY bitflags instead of ASC loose tags or PlayerCharacter bools:
//   - Same readability goal as "tags as state" (named conditions).
//   - Packed inside FFighterSimState => trivial to copy/hash for rollback.
//   - ASC loose tags are a poor authority: not cleanly rewindable, easy to desync
//     from "real" combat, and people often assume they replicate when they do not.
// Prefer adding a bit here over adding bIsX on the character for combat meaning.

namespace FighterStateFlags
{
	static constexpr uint32 None             = 0;
	static constexpr uint32 Attacking        = 1u << 0;
	static constexpr uint32 PostureChanging  = 1u << 1;
	static constexpr uint32 Blocking         = 1u << 2;
	static constexpr uint32 Hitstun          = 1u << 3;
	static constexpr uint32 Invulnerable     = 1u << 4;
}

// ---------------------------------------------------------------------------
// Per-frame input
// ---------------------------------------------------------------------------
// WHY a dedicated input struct (not reading Enhanced Input inside the sim):
//   - Rollback replays INPUTS, not UE input devices.
//   - Sim stays free of PlayerController / World time.
// Sticky fields (sticks, allow flags) vs edges (pressed this frame) are intentional:
//   edges must clear after one sim tick so resim does not re-trigger forever.

USTRUCT(BlueprintType)
struct PURGATORIUMLEX_API FFighterFrameInput
{
	GENERATED_BODY()

	/** Reserved: movement-in-sim later. Unused by rules today on purpose (CMC still owns locomotion). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVector2D MoveStick = FVector2D::ZeroVector;

	/**
	 * Stick used to pick posture.
	 * WHY one stick field + two bools (instead of two sticks):
	 *   Caller (character) already decides whether the value came from dedicated
	 *   posture input or movement (passive lock-on). Sim only needs the resulting
	 *   vector + policy flags — keeps angle math in one place.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVector2D PostureStick = FVector2D::ZeroVector;

	/** Dedicated posture held => overwrites movement-based posture (your design). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bPostureOverrideActive = false;

	/** Lock-on "camera on back" allows movement stick to drive posture. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bAllowMovementPosture = false;

	/** Edge: pressed this frame (not held). WHY edge: fighters buffer presses, not "button down" spam. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bLightAttackPressed = false;

	/** Edge: return to neutral (move released / posture released with your existing rules). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bRequestNeutralPosture = false;
};

// ---------------------------------------------------------------------------
// Fighter sim state (authority blob)
// ---------------------------------------------------------------------------
// WHY this is the center of the architecture:
//   Everything combat-critical for one fighter should eventually live here
//   (or in a match struct that owns two of these).
// WHY no UObject* / AActor* fields:
//   Those cannot be part of a portable save/restore without indirection tables.
// Health/hitstun are present early so the next hitbox PR has somewhere to write —
//   not because we already simulate damage exchanges.

USTRUCT(BlueprintType)
struct PURGATORIUMLEX_API FFighterSimState
{
	GENERATED_BODY()

	/** Monotonic sim frame. WHY: buffer expiry, posture delay, move timing all key off this — not FPlatformTime. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sim")
	int32 Frame = 0;

	/** Authority posture. WHY: continuous visual tilt AND combat queries share one value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Posture")
	EPosture Posture = EPosture::E_Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Posture")
	EPosture PendingPosture = EPosture::E_Neutral;

	/** -1 = none. WHY frame stamp (not a float timer): same delay on every machine/resim. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Posture")
	int32 PostureChangeRequestFrame = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Posture")
	int32 PostureBaseFramesDelay = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Posture")
	int32 PostureBonusFramesDelay = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Posture")
	float PostureStalePenalty = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Posture")
	float PosturePenaltyPerChange = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Posture")
	float PostureMaxPenalty = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Posture")
	int32 LastPostureChangeFrame = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Posture")
	int32 PostureResetFrames = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	ECombatMovePhase MovePhase = ECombatMovePhase::None;

	/**
	 * Posture frozen at light-attack press.
	 * WHY snapshot: continuous posture still updates for idle/walk visuals and next actions,
	 * but the current swing's direction/hit rules must not drift mid-animation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	EPosture AttackSnapshotPosture = EPosture::E_Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	int32 MoveLocalFrame = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	int32 StartupFrames = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	int32 ActiveFrames = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	int32 RecoveryFrames = 0;

	/** Copied from move data at start; applied when hitboxes exist. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float PendingAttackDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 HitstunRemaining = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	uint32 StateFlags = FighterStateFlags::None;

	bool HasFlag(uint32 Flag) const { return (StateFlags & Flag) != 0; }
	void SetFlag(uint32 Flag) { StateFlags |= Flag; }
	void ClearFlag(uint32 Flag) { StateFlags &= ~Flag; }
};

// ---------------------------------------------------------------------------
// Shared stick -> posture mapping
// ---------------------------------------------------------------------------
// WHY extracted from PlayerCharacter:
//   Legacy path and sim must use identical windows or feel/debug will diverge.
// Angle bands match your existing ProcessPostureInput conventions on purpose
// (behavior parity > "cleaner math" in this migration step).

struct PURGATORIUMLEX_API FCombatPostureMath
{
	static EPosture PostureFromStick(const FVector2D& Stick)
	{
		if (Stick.SizeSquared() < 0.01f)
		{
			return EPosture::E_Neutral;
		}

		float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Stick.Y, Stick.X));
		if (AngleDeg < 0.f)
		{
			AngleDeg += 360.f;
		}
		const int32 FinalAngle = FMath::RoundToInt(AngleDeg);

		if (FinalAngle >= 296 && FinalAngle <= 345) { return EPosture::E_DownRight; }
		if (FinalAngle >= 246 && FinalAngle <= 295) { return EPosture::E_Down; }
		if (FinalAngle >= 196 && FinalAngle <= 245) { return EPosture::E_DownLeft; }
		if (FinalAngle >= 126 && FinalAngle <= 195) { return EPosture::E_Left; }
		if (FinalAngle >= 56  && FinalAngle <= 125) { return EPosture::E_Up; }
		return EPosture::E_Right; // 346..360 and 0..55
	}
};
