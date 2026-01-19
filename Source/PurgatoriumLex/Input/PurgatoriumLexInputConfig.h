#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PurgatoriumLexInputConfig.generated.h"

class UInputAction;

/**
 * FPurgatoriumLexInputAction
 *
 *	Struct used to map an input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct FPurgatoriumLexInputAction
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * UPurgatoriumLexInputConfig
 *
 *	Non-mutable data asset that contains input configuration properties.
 *	This allows you to map InputActions to GameplayTags, which can then be used
 *	to trigger Gameplay Abilities via Input ID.
 */
UCLASS(BlueprintType, Const)
class PURGATORIUMLEX_API UPurgatoriumLexInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	UPurgatoriumLexInputConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "PurgatoriumLex|Input")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, Category = "PurgatoriumLex|Input")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	// List of input actions used by the owner. These input actions are mapped to a gameplay tag and must be manually bound.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FPurgatoriumLexInputAction> NativeInputActions;

	// List of input actions used by the owner. These input actions are mapped to a gameplay tag and are automatically bound to abilities with matching input tags.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FPurgatoriumLexInputAction> AbilityInputActions;
};

