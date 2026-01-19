#include "PurgatoriumLexInputComponent.h"
#include "EnhancedInputSubsystems.h"

UPurgatoriumLexInputComponent::UPurgatoriumLexInputComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPurgatoriumLexInputComponent::AddInputMappings(const UPurgatoriumLexInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void UPurgatoriumLexInputComponent::RemoveInputMappings(const UPurgatoriumLexInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void UPurgatoriumLexInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}

