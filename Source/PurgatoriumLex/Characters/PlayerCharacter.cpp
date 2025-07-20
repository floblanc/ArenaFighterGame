


#include "PlayerCharacter.h"
#include "AbilitySystem/PurgatoriumLexAbilitySystemComponent.h"
#include "AbilitySystem/PurgatoriumLexAttributeSet.h"
#include "Player/PurgatoriumLexPlayerState.h"
#include "UI/PurgatoriumLexHUD.h"

//#include "PurgatoriumLexMacros.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystemComponent();
	GiveDefaultAbilities();
	InitDefaultAttributes();
	InitHUD();
}

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilitySystemComponent();
	InitDefaultAttributes();
	InitHUD();
}

void APlayerCharacter::InitAbilitySystemComponent()
{
	APurgatoriumLexPlayerState* PurgatoriumLexPlayerState = GetPlayerState<APurgatoriumLexPlayerState>();
	check(PurgatoriumLexPlayerState);
	AbilitySystemComponent = CastChecked<UPurgatoriumLexAbilitySystemComponent>(PurgatoriumLexPlayerState->GetAbilitySystemComponent());
	AbilitySystemComponent->InitAbilityActorInfo(PurgatoriumLexPlayerState, this);
	AttributeSet = PurgatoriumLexPlayerState->GetAttributeSet();
}

void APlayerCharacter::InitHUD() const
{
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (APurgatoriumLexHUD* PurgatoriumLexHUD = Cast<APurgatoriumLexHUD>(PlayerController->GetHUD()))
		{
			PurgatoriumLexHUD->Init();
		}
	}
}

//// Called when the game starts or when spawned
//void APlayerCharacter::BeginPlay()
//{
//	Super::BeginPlay();
//	
//	UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
//	UE_LOG(LogTemp, Warning, TEXT("PrimaryActorTick.bCanEverTick = %s"), PrimaryActorTick.bCanEverTick ? TEXT("true") : TEXT("false"));
//	PRINT("Hello, %s", *FString(__FUNCTION__));
//	FVector TargetLocation(2200.f, 770.f, 150.f);
//	SPHERE(TargetLocation);
//	LINE(GetActorLocation(), TargetLocation);
//	
//}
//
//// Called every frame
//void APlayerCharacter::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//	SPHERE_TICK(GetActorLocation());
//	LINE_TICK(GetActorLocation(), FVector(2200.f, 770.f, 150.f));
//}

//// Called to bind functionality to input
//void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}


