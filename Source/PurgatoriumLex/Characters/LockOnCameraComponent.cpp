// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/LockOnCameraComponent.h"
#include "Characters/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "CollisionQueryParams.h"

DEFINE_LOG_CATEGORY_STATIC(LogLockOnCamera, Log, All);

ULockOnCameraComponent::ULockOnCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void ULockOnCameraComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULockOnCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateLockOnLookAt();
}

APlayerCharacter* ULockOnCameraComponent::GetOwnerPlayer() const
{
	return Cast<APlayerCharacter>(GetOwner());
}

UCameraComponent* ULockOnCameraComponent::ResolveFollowCamera() const
{
	if (const APlayerCharacter* Player = GetOwnerPlayer())
	{
		return Player->GetFollowCamera();
	}
	return GetOwner() ? GetOwner()->FindComponentByClass<UCameraComponent>() : nullptr;
}

UCharacterMovementComponent* ULockOnCameraComponent::ResolveMovement() const
{
	if (APlayerCharacter* Player = GetOwnerPlayer())
	{
		return Player->GetCharacterMovement();
	}
	return nullptr;
}

void ULockOnCameraComponent::LockToCharacterBack()
{
	if (UCharacterMovementComponent* MoveComp = ResolveMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = true;
	}

	const bool bWasLocked = bIsLockedOnBack;
	bIsLockedOnBack = true;
	if (!bWasLocked)
	{
		OnLockOnBackChanged.Broadcast(true);
	}
}

void ULockOnCameraComponent::UnlockFromCharacterBack()
{
	if (UCharacterMovementComponent* MoveComp = ResolveMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->bUseControllerDesiredRotation = false;
	}

	const bool bWasLocked = bIsLockedOnBack;
	bIsLockedOnBack = false;
	if (bWasLocked)
	{
		OnLockOnBackChanged.Broadcast(false);
	}
}

void ULockOnCameraComponent::RefreshLockOnCandidates()
{
	LockOnCandidates.Empty();

	APlayerCharacter* OwnerPlayer = GetOwnerPlayer();
	UWorld* World = GetWorld();
	if (!OwnerPlayer || !World)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPlayer->GetController());
	if (!PC)
	{
		return;
	}

	const FVector MyLoc = OwnerPlayer->GetActorLocation();
	UCameraComponent* Cam = ResolveFollowCamera();
	const FVector ViewOrigin = Cam ? Cam->GetComponentLocation() : MyLoc;
	const FVector ViewDirection = OwnerPlayer->GetControlRotation().Vector();

	FVector2D ViewportSize(1.f, 1.f);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		ViewportSize.X = FMath::Max(1.f, ViewportSize.X);
		ViewportSize.Y = FMath::Max(1.f, ViewportSize.Y);
	}

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerCharacter::StaticClass(), Found);

	for (AActor* Actor : Found)
	{
		if (Actor == OwnerPlayer || !IsValid(Actor))
		{
			continue;
		}

		APlayerCharacter* Other = Cast<APlayerCharacter>(Actor);
		if (!Other || !OwnerPlayer->IsEnemyPlayer(Other))
		{
			continue;
		}

		const FVector OtherLoc = Other->GetActorLocation();
		const float DistSq = FVector::DistSquared(OtherLoc, MyLoc);
		if (DistSq > LockOnMaxDistance * LockOnMaxDistance)
		{
			continue;
		}

		const FVector ToEnemy = (OtherLoc - ViewOrigin).GetSafeNormal();
		if (FVector::DotProduct(ViewDirection, ToEnemy) <= 0.f)
		{
			continue;
		}

		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(OtherLoc, ScreenPos, true))
		{
			continue;
		}
		if (ScreenPos.X < 0.f || ScreenPos.X > ViewportSize.X || ScreenPos.Y < 0.f || ScreenPos.Y > ViewportSize.Y)
		{
			continue;
		}

		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(OwnerPlayer);
		TraceParams.AddIgnoredActor(Other);
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, ViewOrigin, OtherLoc + ToEnemy * 50.f, ECC_Visibility, TraceParams))
		{
			continue;
		}

		LockOnCandidates.Add(Actor);
	}

	LockOnCandidates.Sort([MyLoc](const AActor& A, const AActor& B)
	{
		return FVector::DistSquared(MyLoc, A.GetActorLocation()) < FVector::DistSquared(MyLoc, B.GetActorLocation());
	});
}

void ULockOnCameraComponent::UpdateLockOnLookAt()
{
	APlayerCharacter* OwnerPlayer = GetOwnerPlayer();
	if (!OwnerPlayer || !bIsLockedOnEnemy || !IsValid(LockedOnActor))
	{
		return;
	}

	const float Distance = (LockedOnActor->GetActorLocation() - OwnerPlayer->GetActorLocation()).Size();
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
		OwnerPlayer->GetActorLocation(),
		LockedOnActor->GetActorLocation());
	LookAtRotation.Pitch -= (TargetingHeightOffset - Distance / 100.0f);

	if (AController* MyController = OwnerPlayer->GetController())
	{
		MyController->SetControlRotation(LookAtRotation);
	}
}

void ULockOnCameraComponent::ToggleLockOnEnemy(bool bAllowOrientToBack)
{
	APlayerCharacter* OwnerPlayer = GetOwnerPlayer();
	if (!OwnerPlayer)
	{
		return;
	}

	UE_LOG(LogLockOnCamera, Log, TEXT("[LockOn] Toggle (currently locked=%s)"), bIsLockedOnEnemy ? TEXT("yes") : TEXT("no"));

	if (bIsLockedOnEnemy)
	{
		bIsLockedOnEnemy = false;
		LockedOnActor = nullptr;
		UnlockFromCharacterBack();
		OnLockOnEnemyChanged.Broadcast(false, nullptr);
		return;
	}

	RefreshLockOnCandidates();
	if (LockOnCandidates.Num() <= 0)
	{
		UE_LOG(LogLockOnCamera, Warning, TEXT("[LockOn] No enemy candidate in range/view"));
		return;
	}

	LockedOnActor = LockOnCandidates[0];
	bIsLockedOnEnemy = true;
	if (bAllowOrientToBack)
	{
		LockToCharacterBack();
	}
	OnLockOnEnemyChanged.Broadcast(true, LockedOnActor);
}
