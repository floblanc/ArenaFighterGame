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
// Sticky fields (directions, allow flags) vs edges (pressed this frame) are intentional:
//   edges must clear after one sim tick so resim does not re-trigger forever.
//
// WHY NOT BlueprintType (UE 5.8):
//   Intent is latched in C++ only. Exposing a writable BlueprintType input struct
//   invites Event Graph "authority" that bypasses the sim. UPROPERTY() still marks
//   fields for UHT/serialization without Make/Break in BP.

USTRUCT()
struct PURGATORIUMLEX_API FFighterFrameInput
{
	GENERATED_BODY()

	/**
	 * Locomotion intent as a 2D direction (WASD, left stick, etc. — device-agnostic).
	 * Reserved: movement-in-sim later. Unused by rules today (CMC still owns locomotion).
	 */
	UPROPERTY()
	FVector2D MoveDirection = FVector2D::ZeroVector;

	/**
	 * Direction used to pick posture (same vector space as MoveDirection).
	 * WHY one direction field + two bools (instead of two vectors):
	 *   Caller (character) already decides whether the value came from dedicated
	 *   posture input or movement (passive lock-on). Sim only needs the resulting
	 *   vector + policy flags — keeps angle math in one place.
	 */
	UPROPERTY()
	FVector2D PostureDirection = FVector2D::ZeroVector;

	/** Dedicated posture held => overwrites movement-based posture (your design). */
	UPROPERTY()
	bool bPostureOverrideActive = false;

	/** Lock-on "camera on back" allows movement direction to drive posture. */
	UPROPERTY()
	bool bAllowMovementPosture = false;

	/** Edge: pressed this frame (not held). WHY edge: fighters buffer presses, not "button down" spam. */
	UPROPERTY()
	bool bLightAttackPressed = false;

	/** Edge: return to neutral (move released / posture released with your existing rules). */
	UPROPERTY()
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
	UPROPERTY(BlueprintReadOnly, Category = "Sim")
	int32 Frame = 0;

	/** Authority posture. WHY: continuous visual tilt AND combat queries share one value. */
	UPROPERTY(BlueprintReadOnly, Category = "Posture")
	EPosture Posture = EPosture::E_Neutral;

	UPROPERTY(BlueprintReadOnly, Category = "Posture")
	EPosture PendingPosture = EPosture::E_Neutral;

	/** -1 = none. WHY frame stamp (not a float timer): same delay on every machine/resim. */
	UPROPERTY(BlueprintReadOnly, Category = "Posture")
	int32 PostureChangeRequestFrame = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Posture")
	int32 PostureBaseFramesDelay = 3;

	UPROPERTY(BlueprintReadOnly, Category = "Posture")
	int32 PostureBonusFramesDelay = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Posture")
	float PostureStalePenalty = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Posture")
	float PosturePenaltyPerChange = 0.08f;

	UPROPERTY(BlueprintReadOnly, Category = "Posture")
	float PostureMaxPenalty = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Posture")
	int32 LastPostureChangeFrame = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Posture")
	int32 PostureResetFrames = 60;

	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	ECombatMovePhase MovePhase = ECombatMovePhase::None;

	/**
	 * Posture frozen at light-attack press.
	 * WHY snapshot: continuous posture still updates for idle/walk visuals and next actions,
	 * but the current swing's direction/hit rules must not drift mid-animation.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	EPosture AttackSnapshotPosture = EPosture::E_Neutral;

	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	int32 MoveLocalFrame = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	int32 StartupFrames = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	int32 ActiveFrames = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	int32 RecoveryFrames = 0;

	/** Copied from move data at start; applied when hitboxes exist. */
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	float PendingAttackDamage = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float Health = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 HitstunRemaining = 0;

	/**
	 * Packed combat conditions (see FighterStateFlags).
	 * WHY int32 not uint32: UHT/BlueprintType structs cannot expose uint32 (UE 5.8).
	 * Bits used today fit in signed 32; C++ helpers still take unsigned masks.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 StateFlags = static_cast<int32>(FighterStateFlags::None);

	bool HasFlag(uint32 Flag) const { return (static_cast<uint32>(StateFlags) & Flag) != 0; }
	void SetFlag(uint32 Flag) { StateFlags = static_cast<int32>(static_cast<uint32>(StateFlags) | Flag); }
	void ClearFlag(uint32 Flag) { StateFlags = static_cast<int32>(static_cast<uint32>(StateFlags) & ~Flag); }
};

// ---------------------------------------------------------------------------
// Shared direction -> posture mapping
// ---------------------------------------------------------------------------
// Angle after Atan2, normalized to [0, 360]: 0° = +X (right), CCW toward +Y (forward).
//
//                    Up (56..125)
//                       |
//     Left (126..195) --+-- Right (346..360 OR 0..55)
//                       |
//         DownLeft   Down   DownRight
//        (196..245)(246..295)(296..345)

struct PURGATORIUMLEX_API FCombatPostureMath
{
	static bool IsAngleInRange(int32 Angle, int32 MinInclusive, int32 MaxInclusive)
	{
		return Angle >= MinInclusive && Angle <= MaxInclusive;
	}

	/** Device-agnostic direction (stick / WASD / …). Near-zero => Neutral. */
	static EPosture PostureFromDirection(const FVector2D& Direction)
	{
		if (Direction.SizeSquared() < 0.01f)
		{
			return EPosture::E_Neutral;
		}

		float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
		if (AngleDeg < 0.f)
		{
			AngleDeg += 360.f;
		}
		const int32 Angle = FMath::Clamp(FMath::RoundToInt(AngleDeg), 0, 360);

		if (IsAngleInRange(Angle, 56, 125))  { return EPosture::E_Up; }
		if (IsAngleInRange(Angle, 126, 195)) { return EPosture::E_Left; }
		if (IsAngleInRange(Angle, 196, 245)) { return EPosture::E_DownLeft; }
		if (IsAngleInRange(Angle, 246, 295)) { return EPosture::E_Down; }
		if (IsAngleInRange(Angle, 296, 345)) { return EPosture::E_DownRight; }
		if (IsAngleInRange(Angle, 346, 360) || IsAngleInRange(Angle, 0, 55))
		{
			return EPosture::E_Right;
		}

		// Unreachable when Angle is in [0, 360] — defensive only.
		return EPosture::E_Neutral;
	}
};
