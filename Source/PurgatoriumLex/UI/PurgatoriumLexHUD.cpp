


#include "PurgatoriumLexHUD.h"
#include "AttributesWidget.h"

void APurgatoriumLexHUD::Init()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	// ------- TOP BAR (Local Player) -------
	if (AttributeWidgetClass)
	{
		AttributeWidget = CreateWidget<UAttributesWidget>(PlayerController, AttributeWidgetClass);
		AttributeWidget->AddToViewport();
		AttributeWidget->BindToAttributes();
	}

}