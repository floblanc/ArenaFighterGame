// Purgatorium Lex by Florian Blanchard


#include "AttributesWidget.h"
#include "AbilitySystem/PurgatoriumLexAttributeSet.h"
#include "Player/PurgatoriumLexPlayerState.h"

void UAttributesWidget::BindToAttributes()
{
	const APurgatoriumLexPlayerState* PurgatoriumLexPlayerState = Cast<APurgatoriumLexPlayerState>(GetOwningPlayerState());
	if (!PurgatoriumLexPlayerState) return;

	UAbilitySystemComponent* ASC = PurgatoriumLexPlayerState->GetAbilitySystemComponent();
	const UPurgatoriumLexAttributeSet* PurgatoriumLexAS = PurgatoriumLexPlayerState->GetAttributeSet();

	// Initial Attributes
	HealthPercent = NUMERIC_VALUE(PurgatoriumLexAS, Health) / NUMERIC_VALUE(PurgatoriumLexAS, MaxHealth);
	StaminaPercent = NUMERIC_VALUE (PurgatoriumLexAS, Stamina) / NUMERIC_VALUE(PurgatoriumLexAS, MaxStamina);
    HealthText = FText::FromString(
        FString::Printf(TEXT("%.0f / %.0f"), NUMERIC_VALUE(PurgatoriumLexAS, Health), NUMERIC_VALUE(PurgatoriumLexAS, MaxHealth))
    );

	// Attribute Changes
	ASC->GetGameplayAttributeValueChangeDelegate(PurgatoriumLexAS->GetHealthAttribute()).AddLambda(
		[this, PurgatoriumLexAS](const FOnAttributeChangeData& Data)->void
		{
			const float NewHealth = Data.NewValue;
            const float MaxHealthValue = NUMERIC_VALUE(PurgatoriumLexAS, MaxHealth);
			
			HealthPercent = MaxHealthValue > 0.f ? NewHealth / MaxHealthValue : 0.f;
			// Format "Current / Max"
			HealthText = FText::FromString(
				FString::Printf(TEXT("%.0f / %.0f"), NewHealth, MaxHealthValue)
			);
		});

	ASC->GetGameplayAttributeValueChangeDelegate(PurgatoriumLexAS->GetStaminaAttribute()).AddLambda(
		[this, PurgatoriumLexAS](const FOnAttributeChangeData& Data)->void
		{
			StaminaPercent = Data.NewValue / NUMERIC_VALUE(PurgatoriumLexAS, MaxStamina);
		});
}
