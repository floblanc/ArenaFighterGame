// Copyright Epic Games, Inc. All Rights Reserved.
//
// Small shared types formerly buried in PlayerCharacter.h.
// Keep the pawn header about composition, not every enum/struct.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PlayerCharacterTypes.generated.h"

class UGameplayAbility;

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	E_Idle		UMETA(DisplayName = "IDLE"),
	E_Walking	UMETA(DisplayName = "WALKING"),
	E_Running	UMETA(DisplayName = "RUNNING"),
	E_Rolling	UMETA(DisplayName = "ROLLING"),
	E_Hitting	UMETA(DisplayName = "HITTING"),
	E_Other		UMETA(DisplayName = "OTHER"),
};

UENUM(BlueprintType)
enum class ETechType : uint8
{
	E_Standard		UMETA(DisplayName = "STANDARD"),
	E_RollForward	UMETA(DisplayName = "ROLL FORWARD"),
	E_RollBackward	UMETA(DisplayName = "ROLL BACKWARD"),
	E_RollLeft		UMETA(DisplayName = "ROLL LEFT"),
	E_RollRight		UMETA(DisplayName = "ROLL RIGHT"),
	E_Wall			UMETA(DisplayName = "WALL"),
	E_WallJump		UMETA(DisplayName = "WALL JUMP"),
};

/** Maps a Gameplay Ability to an Input Tag (grant + bind in one place). */
USTRUCT(BlueprintType)
struct FAbilityInputMapping
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<class UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * Client-side ability input buffer entry (feel only — not combat authority).
 * Not replicated; not a rollback restore blob.
 */
USTRUCT(BlueprintType)
struct FBufferedAbilityInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Buffer")
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Buffer")
	int32 BufferedFrame = 0;

	FBufferedAbilityInput() = default;
	FBufferedAbilityInput(const FGameplayTag& Tag, int32 Frame) : InputTag(Tag), BufferedFrame(Frame) {}

	int32 GetFramesRemaining(int32 CurrentFrame, int32 BufferFrames) const
	{
		return FMath::Max(0, BufferFrames - (CurrentFrame - BufferedFrame));
	}
};
