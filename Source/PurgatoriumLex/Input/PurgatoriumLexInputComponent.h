#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "PurgatoriumLexInputConfig.h"
#include "PurgatoriumLexInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;

/**
 * UPurgatoriumLexInputComponent
 *
 *	Component used to manage input mappings and bindings using an input config data asset.
 *	This extends EnhancedInputComponent to provide helper methods for binding abilities via Input ID.
 */
UCLASS(Config = Input)
class PURGATORIUMLEX_API UPurgatoriumLexInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	UPurgatoriumLexInputComponent(const FObjectInitializer& ObjectInitializer);

	void AddInputMappings(const UPurgatoriumLexInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	void RemoveInputMappings(const UPurgatoriumLexInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	template<class UserClass, typename FuncType>
	bool BindNativeAction(const UPurgatoriumLexInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UPurgatoriumLexInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	void RemoveBinds(TArray<uint32>& BindHandles);
};


template<class UserClass, typename FuncType>
bool UPurgatoriumLexInputComponent::BindNativeAction(const UPurgatoriumLexInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
		return true;
	}
	return false;
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UPurgatoriumLexInputComponent::BindAbilityActions(const UPurgatoriumLexInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FPurgatoriumLexInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
			}

			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}

