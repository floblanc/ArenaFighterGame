


#include "PurgatoriumLexCharacterBase.h"
#include "AbilitySystem/PurgatoriumLexAbilitySystemComponent.h"


// Sets default values
APurgatoriumLexCharacterBase::APurgatoriumLexCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

UAbilitySystemComponent* APurgatoriumLexCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UPurgatoriumLexAttributeSet* APurgatoriumLexCharacterBase::GetAttributeSet() const
{
	return AttributeSet;
}

void APurgatoriumLexCharacterBase::GiveDefaultAbilities()
{
	check(AbilitySystemComponent);
	if (!HasAuthority()) return;

	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
	{
		const FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

void APurgatoriumLexCharacterBase::InitDefaultAttributes() const
{
	if (!AbilitySystemComponent || !DefaultAttributeEffect) { return;  }

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1.f, EffectContext);

	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

//// Called when the game starts or when spawned
//void APurgatoriumLexCharacterBase::BeginPlay()
//{
//	Super::BeginPlay();
//	
//}

//// Called every frame
//void APurgatoriumLexCharacterBase::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}
//
//// Called to bind functionality to input
//void APurgatoriumLexCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}


