


#include "PurgatoriumLexHUD.h"
#include "AttributesWidget.h"

void APurgatoriumLexHUD::Init()
{
	AttributeWidget = CreateWidget<UAttributesWidget>(GetOwningPlayerController(), AttributeWidgetClass);
	AttributeWidget->BindToAttributes();
	AttributeWidget->AddToViewport();
}