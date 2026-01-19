

#include "PurgatoriumLexAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbilitySpec.h"

// Sets default values for this component's properties
UPurgatoriumLexAbilitySystemComponent::UPurgatoriumLexAbilitySystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}

// Called when the game starts
void UPurgatoriumLexAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UPurgatoriumLexAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Note: ProcessAbilityInput() is now called from input event handlers (Input_AbilityInputTagPressed/Released)
	// rather than from Tick for rollback netcode compatibility. This ensures frame-accurate input processing.
	// If no input events occurred this frame, we still need to process any queued inputs.
	// However, for rollback netcode, input should be processed immediately on input events, not in Tick.
	// This TickComponent call is kept for backward compatibility but ProcessAbilityInput should ideally
	// be called from input handlers instead.
	
	// Only process if there are pending inputs (for backward compatibility)
	if (InputPressedSpecHandles.Num() > 0 || InputReleasedSpecHandles.Num() > 0)
	{
		ProcessAbilityInput(DeltaTime, false);
	}
}

void UPurgatoriumLexAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		// Find all abilities that have this Input Tag and queue them for activation
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UPurgatoriumLexAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		// Find all abilities that have this Input Tag and handle their release
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UPurgatoriumLexAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// Use local array instead of static for rollback netcode compatibility
	// Static arrays can persist state across rollbacks, causing incorrect behavior
	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	//
	// Process all abilities that had their input pressed this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					// Check if ability can be activated
					const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
					if (AbilitySpec->Ability->CanActivateAbility(AbilitySpec->Handle, ActorInfo))
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	//
	// Try to activate all the abilities that are from presses.
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	//
	// Process all abilities that had their input released this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;
				
				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input release event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	// Clear the input arrays for next frame
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UPurgatoriumLexAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}


