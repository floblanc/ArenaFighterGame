// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PurgatoriumLexPlayerController.generated.h"

class UInputMappingContext;
class APlayerCharacter;

/**
 *  Simple Player Controller for a third person combat game
 *  Manages input mappings
 *  Respawns the player character at the checkpoint when it's destroyed
 */
UCLASS(abstract)
class APurgatoriumLexPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input mapping context for this player */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Respawn")
	TSubclassOf<APlayerCharacter> CharacterClass;

	/** Transform to respawn the character at. Can be set to create checkpoints */
	FTransform RespawnTransform;

protected:

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

public:

	/** Updates the character respawn transform */
	void SetRespawnTransform(const FTransform& NewRespawn);

protected:

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

};
