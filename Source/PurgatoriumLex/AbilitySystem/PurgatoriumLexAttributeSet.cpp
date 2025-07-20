


#include "PurgatoriumLexAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

// Sets default values
UPurgatoriumLexAttributeSet::UPurgatoriumLexAttributeSet()
{
	//InitHealth(80.f);
}

void UPurgatoriumLexAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPurgatoriumLexAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPurgatoriumLexAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPurgatoriumLexAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPurgatoriumLexAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPurgatoriumLexAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPurgatoriumLexAttributeSet, MaxStrength, COND_None, REPNOTIFY_Always);
}

void UPurgatoriumLexAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	//if (Attribute == GetHealthAttribute())
	//{
	//	NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	//}
}

void UPurgatoriumLexAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	//if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	//{
	//	SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	//}
}


void UPurgatoriumLexAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, Health, OldHealth);
}

void UPurgatoriumLexAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, MaxHealth, OldMaxHealth);
}

void UPurgatoriumLexAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, Stamina, OldStamina);
}

void UPurgatoriumLexAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, MaxStamina, OldMaxStamina);
}

void UPurgatoriumLexAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, Strength, OldStrength);
}

void UPurgatoriumLexAttributeSet::OnRep_MaxStrength(const FGameplayAttributeData& OldMaxStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, MaxStrength, OldMaxStrength);
}


