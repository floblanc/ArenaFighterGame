

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PurgatoriumLexHUD.generated.h"

class UAttributesWidget;
/**
 * 
 */
UCLASS()
class PURGATORIUMLEX_API APurgatoriumLexHUD : public AHUD
{
	GENERATED_BODY()
public:
	void Init();

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributesWidget> AttributeWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UAttributesWidget> AttributeWidget;
};
