#include "PurgatoriumLexGameplayTags.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"

namespace PurgatoriumLexGameplayTags
{
	// Input Tags - These map to InputActions and are used as Input IDs for Gameplay Abilities
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_LightAttack, "InputTag.LightAttack", "Light Attack input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_ChargeAttack, "InputTag.ChargeAttack", "Charge Attack input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_SpecialAttack, "InputTag.SpecialAttack", "Special Attack input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Guard, "InputTag.Guard", "Guard input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Parry, "InputTag.Parry", "Parry input tag for abilities (triggered on Guard release).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Roll, "InputTag.Roll", "Roll input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Jump, "InputTag.Jump", "Jump input tag for abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look, "InputTag.Look", "Look input tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Run, "InputTag.Run", "Run input tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Posture, "InputTag.Posture", "Posture input tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_LockUnlock, "InputTag.LockUnlock", "Lock/Unlock camera input tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_BreakGuard, "InputTag.BreakGuard", "Break Guard input tag for abilities.");

	// State System Tags (authoritative state via loose tags on ASC)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Grounded, "State.Grounded", "Character is on the ground.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Airborne, "State.Airborne", "Character is airborne / not grounded.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Attacking, "State.Attacking", "Character is currently attacking.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Hitstun, "State.Hitstun", "Character is in hitstun and cannot act.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Blocking, "State.Blocking", "Character is blocking/guarding.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Rolling, "State.Rolling", "Character is rolling/dodging.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_PostureChanging, "State.PostureChanging", "Character is currently changing posture (transition/delay pending).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Grabbing, "State.Grabbing", "Character is grabbing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_KnockedDown, "State.KnockedDown", "Character is knocked down (on their back).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Invulnerable, "State.Invulnerable", "Character is invulnerable (cannot take damage).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_SuperArmor, "State.SuperArmor", "Character has super armor (absorbs hits without flinching).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Parrying, "State.Parrying", "Character is actively parrying (parry window active).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Parried, "State.Parried", "Character just parried an attack (recovery/advantage window).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_CounterHit, "State.CounterHit", "Character is in counter-hit state (increased damage/knockback).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Stunned, "State.Stunned", "Character is stunned (general stun state).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Jumping, "State.Jumping", "Character is actively jumping.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_WallSplat, "State.WallSplat", "Character bounced off wall.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_GroundBounce, "State.GroundBounce", "Character bounced off ground.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_GettingUp, "State.GettingUp", "Character is getting up from knockdown.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Tech, "State.Tech", "Character is performing a tech (quick recovery from knockdown/hit).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_BeingGrabbed, "State.BeingGrabbed", "Character is being grabbed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_GrabBreak, "State.GrabBreak", "Character is breaking out of a grab.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Taunting, "State.Taunting", "Character is taunting.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead", "Character is dead.");

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

