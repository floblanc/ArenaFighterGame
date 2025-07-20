

#pragma once

#include "CoreMinimal.h"
#include "PurgatoriumLexCharacterBase.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class PURGATORIUMLEX_API APlayerCharacter : public APurgatoriumLexCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

private:
	void InitAbilitySystemComponent();
	void InitHUD() const;

//protected:
	//// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

//public:	
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;
//
//	// Called to bind functionality to input
//	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
