// Purgatorium Lex by Florian Blanchard

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Blueprint/UserWidget.h"
#include "AttributesWidget.generated.h"

/**
 * 
 */
UCLASS()
class PURGATORIUMLEX_API UAttributesWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Attributes")
	void BindToAttributes();

protected:

	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	float HealthPercent;

	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	float StaminaPercent;

	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	FText HealthText;
};
