#include "PurgatoriumLexGameplayTags.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"

namespace PurgatoriumLexGameplayTags
{
	// Input Tags - These map to InputActions and are used as Input IDs for Gameplay Abilities
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_LightAttack, "InputTag.LightAttack", "Light Attack input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_HeavyAttack, "InputTag.HeavyAttack", "Heavy Attack input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_SpecialAttack, "InputTag.SpecialAttack", "Special Attack input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Guard, "InputTag.Guard", "Guard input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Dash, "InputTag.Dash", "Dash input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Jump, "InputTag.Jump", "Jump input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look, "InputTag.Look", "Look input tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Run, "InputTag.Run", "Run input tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Posture, "InputTag.Posture", "Posture input tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_LockUnlock, "InputTag.LockUnlock", "Lock/Unlock camera input tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_BreakGuard, "InputTag.BreakGuard", "Break Guard input tag for abilities.");

	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogTemp, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}

