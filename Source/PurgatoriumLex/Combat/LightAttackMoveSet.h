// Copyright Epic Games, Inc. All Rights Reserved.
//
// REVIEW STEP 2/5 — Move definitions as data.
//
// WHY A DATA ASSET INSTEAD OF GAMEPLAY ABILITY / BLUEPRINT ABILITY
// ---------------------------------------------------------------
// You liked GAS for modularity ("character does not hardcode every attack").
// Rollback makes UGameplayAbility a bad *authority* for combat timing.
// ULightAttackMoveSet keeps the modularity: designers edit rows in the editor;
// the sim only reads integers (and presentation reads Soft montage pointers).
//
// WHY ONE ROW PER POSTURE (not seven ability classes)
// ---------------------------------------------------
// One light button -> one sim action -> table lookup by snapshotted posture.
// Avoids copy-paste GA graphs and keeps tuning (frames/damage) in one place.
//
// WHY MONTAGE IS SOFT + OPTIONAL
// ------------------------------
// Hit/miss must never depend on whether a montage loaded. SoftObjectPtr lets
// content assign anims without hard-referencing them from C++. Sim ignores it;
// the component/character may play it for feel only.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Combat/CombatTypes.h"
#include "LightAttackMoveSet.generated.h"

class UAnimMontage;

/** One light attack variant for a single posture. */
USTRUCT(BlueprintType)
struct PURGATORIUMLEX_API FLightAttackMoveDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Move")
	EPosture Posture = EPosture::E_Neutral;

	/** Frames before hitbox is live. WHY frame counts: shared language with rollback + balancing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0"))
	int32 StartupFrames = 8;

	/** Frames where hit detection should run (hitboxes not implemented yet). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "1"))
	int32 ActiveFrames = 3;

	/** Frames after active before the fighter can act again. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0"))
	int32 RecoveryFrames = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
	float Damage = 10.f;

	/** Presentation only — see file header WHY. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UAnimMontage> Montage;
};

UCLASS(BlueprintType)
class PURGATORIUMLEX_API ULightAttackMoveSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light Attacks", meta = (TitleProperty = "Posture"))
	TArray<FLightAttackMoveDef> Moves;

	/**
	 * Lookup by posture.
	 * WHY fallback defaults if a row is missing: PIE should still exercise the
	 * phase machine; missing art/data must not silently hard-crash iteration.
	 * Log noise in the sim warns when the whole MoveSet asset is unset.
	 */
	UFUNCTION(BlueprintPure, Category = "Light Attacks")
	FLightAttackMoveDef FindMove(EPosture Posture) const;
};
