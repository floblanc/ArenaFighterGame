


#include "PurgatoriumLexPlayerState.h"
#include "AbilitySystem/PurgatoriumLexAbilitySystemComponent.h"
#include "AbilitySystem/PurgatoriumLexAttributeSet.h"


// Sets default values
APurgatoriumLexPlayerState::APurgatoriumLexPlayerState()
{
	SetNetUpdateFrequency(60.f); //100.f

	AbilitySystemComponent = CreateDefaultSubobject<UPurgatoriumLexAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UPurgatoriumLexAttributeSet>("AttributeSet");
}

UAbilitySystemComponent* APurgatoriumLexPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UPurgatoriumLexAttributeSet* APurgatoriumLexPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

//// Called when the game starts or when spawned
//void APurgatoriumLexPlayerState::BeginPlay()
//{
//	Super::BeginPlay();
//	
//}

//// Called every frame
//void APurgatoriumLexPlayerState::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}


