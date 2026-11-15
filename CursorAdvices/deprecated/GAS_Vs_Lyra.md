> **DEPRECATED FOR IMPLEMENTATION** — kept for learning.  
> Condensed from the old Lyra comparison docs. Combat no longer extends this path.  
> See [GAS_What_It_Is.md](GAS_What_It_Is.md), [GAS_Input_And_Abilities.md](GAS_Input_And_Abilities.md).

# GAS era: this project vs Lyra

PurgatoriumLex’s GAS/input layer was **inspired by Lyra**, then **simplified**.

---

## Big picture

| Topic | Lyra | This project (GAS era) |
|-------|------|-------------------------|
| Where input binds | `ULyraHeroComponent` + pawn extension | Directly in `APlayerCharacter::SetupPlayerInputComponent` |
| InputConfig | `ULyraInputConfig` | `UPurgatoriumLexInputConfig` (same idea: Native + Ability arrays) |
| InputComponent | `ULyraInputComponent` | `UPurgatoriumLexInputComponent` (same bind helpers) |
| ASC input | Custom activation policies on `ULyraGameplayAbility` | Simpler: standard activate on press |
| Init sequencing | GameFramework component manager | Manual PossessedBy / BeginPlay |

**Why simpler here:** one character codebase, fewer moving parts, easier to learn. Lyra’s components scale a large sample; they were not required for a solo arena fighter prototype.

---

## What stayed the same (conceptually)

- Map Enhanced Input Actions → **GameplayTags**  
- Ability presses call `AbilityInputTagPressed` / release counterpart  
- Queues of pressed/released spec handles, then `ProcessAbilityInput`  

---

## What differed (important)

1. **No Lyra activation policy enum** — no `WhileInputActive` / custom ability base required.  
2. **ASC access** — character/PlayerState directly, not through pawn extension helpers.  
3. **Grant + input tag** — this project used `GrantAbilityWithInputTag` + a **local map** `InputTag → handles`. Lyra tends to put identity on the **Spec tags** (better for GAS replication). That map issue was called out in architecture review.  
4. **Less input-blocked tag machinery** — Lyra clears input when a block tag is present; this ASC was thinner.

---

## Takeaway after choosing rollback

Lyra/GAS pattern is a solid **UE replication** template.  
It is the wrong long-term **authority** for GGPO-style combat.  
The useful residue: **InputConfig-style data** and “one button → named action,” now aimed at the **sim + DataAsset**, not `UGameplayAbility`.
