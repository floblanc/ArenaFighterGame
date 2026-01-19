#include "PurgatoriumLexInputConfig.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogPurgatoriumLexInput, Log, All);

UPurgatoriumLexInputConfig::UPurgatoriumLexInputConfig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UInputAction* UPurgatoriumLexInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FPurgatoriumLexInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogPurgatoriumLexInput, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UPurgatoriumLexInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FPurgatoriumLexInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogPurgatoriumLexInput, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

