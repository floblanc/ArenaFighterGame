

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PurgatoriumLexAttributeSet.generated.h"

#define NUMERIC_VALUE(AttributeSetName, PropertyName) \
	AttributeSetName->Get##PropertyName##Attribute().GetNumericValue(AttributeSetName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAttributeChangedEvent, UAttributeSet*, AttributeSet, float, OldValue, float, NewValue);

UCLASS()
class PURGATORIUMLEX_API UPurgatoriumLexAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UPurgatoriumLexAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// Current health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Ability | Gameplay Attribute") //, meta = (HideFromModifiers)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UPurgatoriumLexAttributeSet, Health);

	// Upper limit for health value
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Ability | Gameplay Attribute") //, meta = (HideFromModifiers)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UPurgatoriumLexAttributeSet, MaxHealth);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UPurgatoriumLexAttributeSet, Stamina);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UPurgatoriumLexAttributeSet, MaxStamina);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS_BASIC(UPurgatoriumLexAttributeSet, Strength);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStrength, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MaxStrength;
	ATTRIBUTE_ACCESSORS_BASIC(UPurgatoriumLexAttributeSet, MaxStrength);

	/**
	 * Meta attribute: temporary GE magnitude, converted to -Health in PostGameplayEffectExecute.
	 * WHY not replicated: clients should never need the transient "Damage" value —
	 * only the resulting Health. Replicating meta attrs is a common GAS footgun.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UPurgatoriumLexAttributeSet, Damage);

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
	
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength);

	UFUNCTION()
	void OnRep_MaxStrength(const FGameplayAttributeData& OldMaxStrength);

	// OnRep_Damage removed: meta Damage is no longer replicated.
};
