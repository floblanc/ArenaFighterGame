

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "PurgatoriumLexAbilitySystemComponent.generated.h"

/**
 * UPurgatoriumLexAbilitySystemComponent
 *
 *	Extended Ability System Component that supports Input ID binding via GameplayTags.
 *	This allows abilities to be triggered by Enhanced Input Actions through Input Tags.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PURGATORIUMLEX_API UPurgatoriumLexAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPurgatoriumLexAbilitySystemComponent();

	/**
	 * Called when an input tag is pressed. This will find all abilities with matching Input Tags
	 * and queue them for activation.
	 */
	UFUNCTION(BlueprintCallable, Category = "PurgatoriumLex|Ability")
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/**
	 * Called when an input tag is released. This will find all abilities with matching Input Tags
	 * and handle their release logic.
	 */
	UFUNCTION(BlueprintCallable, Category = "PurgatoriumLex|Ability")
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/**
	 * Processes ability input. Should be called deterministically when input events occur,
	 * NOT from Tick for rollback netcode compatibility.
	 * It activates abilities that have their input pressed and handles input release.
	 */
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	/**
	 * Clears all pending ability input.
	 */
	void ClearAbilityInput();

	/**
	 * Grants an ability and binds it to an input tag. Use this so input triggers the correct ability.
	 */
	UFUNCTION(BlueprintCallable, Category = "PurgatoriumLex|Ability")
	FGameplayAbilitySpecHandle GrantAbilityWithInputTag(TSubclassOf<UGameplayAbility> AbilityClass, const FGameplayTag& InputTag, int32 Level = 1);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	// Input tag -> ability spec handles. Filled by GrantAbilityWithInputTag; used for activation by input tag.
	TMap<FGameplayTag, TArray<FGameplayAbilitySpecHandle>> InputTagToSpecHandles;
};
