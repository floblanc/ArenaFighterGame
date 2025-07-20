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
	void BindToAttributes();

protected:
	UPROPERTY(BlueprintReadOnly)
	float HealthPercent;

	UPROPERTY(BlueprintReadOnly)
	float StaminaPercent;
};
