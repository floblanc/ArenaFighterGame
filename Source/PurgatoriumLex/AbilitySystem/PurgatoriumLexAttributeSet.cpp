


#include "PurgatoriumLexAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

// Sets default values
UPurgatoriumLexAttributeSet::UPurgatoriumLexAttributeSet()
{
	// InitHealth(100.0f);
	// InitMaxHealth(100.0f);
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
	// Damage is a meta attribute (transient during GE execution) — do not replicate.
}

void UPurgatoriumLexAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	
	Super::PreAttributeChange(Attribute, NewValue);
}

void UPurgatoriumLexAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UPurgatoriumLexAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Convert into -Health and then clamp
		const float DamageValue = GetDamage();
		const float OldHealthValue = GetHealth();
		const float MaxHealthValue = GetMaxHealth();
		const float NewHealthValue = FMath::Clamp(OldHealthValue - DamageValue, 0.0f, MaxHealthValue);
 
		if (OldHealthValue != NewHealthValue)
		{
			// Set the new health after clamping to min-max
			SetHealth(NewHealthValue);
		}
 
		// Clear the meta attribute that temporarily held damage
		SetDamage(0.0f);
	}
}


void UPurgatoriumLexAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, Health, OldValue);
}

void UPurgatoriumLexAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, MaxHealth, OldMaxHealth);
}

void UPurgatoriumLexAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, Stamina, OldStamina);
}

void UPurgatoriumLexAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, MaxStamina, OldMaxStamina);
}

void UPurgatoriumLexAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, Strength, OldStrength);
}

void UPurgatoriumLexAttributeSet::OnRep_MaxStrength(const FGameplayAttributeData& OldMaxStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPurgatoriumLexAttributeSet, MaxStrength, OldMaxStrength);
}

