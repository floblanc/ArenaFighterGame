

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "PurgatoriumLexPlayerState.generated.h"

class UPurgatoriumLexAbilitySystemComponent;
class UPurgatoriumLexAttributeSet;

UCLASS()
class PURGATORIUMLEX_API APurgatoriumLexPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APurgatoriumLexPlayerState();
	//~IAbilitySystemInterface interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface
	virtual UPurgatoriumLexAttributeSet* GetAttributeSet() const;

protected:
	UPROPERTY()
	TObjectPtr<UPurgatoriumLexAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UPurgatoriumLexAttributeSet> AttributeSet;

	//// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

public:	
	//// Called every frame
	//virtual void Tick(float DeltaTime) override;

	
	
};
