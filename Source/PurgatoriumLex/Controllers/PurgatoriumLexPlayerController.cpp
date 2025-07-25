// Copyright Epic Games, Inc. All Rights Reserved.


#include "PurgatoriumLexPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Characters/PlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

void APurgatoriumLexPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Contexts
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}

void APurgatoriumLexPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &APurgatoriumLexPlayerController::OnPawnDestroyed);
}

void APurgatoriumLexPlayerController::SetRespawnTransform(const FTransform& NewRespawn)
{
	// save the new respawn transform
	RespawnTransform = NewRespawn;
}

void APurgatoriumLexPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// spawn a new character at the respawn transform
	if (APlayerCharacter* RespawnedCharacter = GetWorld()->SpawnActor<APlayerCharacter>(CharacterClass, RespawnTransform))
	{
		// possess the character
		Possess(RespawnedCharacter);
	}
}