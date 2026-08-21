> **DEPRECATED FOR IMPLEMENTATION** — kept for learning.  
> Explains the **old** Lyra-style wiring still present in some Source files.  
> New combat actions should go through the fighter sim, not new GAs.  
> See [GAS_What_It_Is.md](GAS_What_It_Is.md), [../CombatSim_Design.md](../CombatSim_Design.md).

# GAS era: input → abilities

How this project wired **Enhanced Input → GameplayTag → ASC → GameplayAbility**.

---

## Pipeline (concept)

```
Key/button (IMC)
  → UInputAction (e.g. IA_LightAttack)
  → InputConfig maps Action → InputTag.LightAttack
  → Character Input_AbilityInputTagPressed(Tag)
  → ASC AbilityInputTagPressed / ProcessAbilityInput
  → Matching granted GameplayAbility activates
```

**Native vs ability actions** in InputConfig:

- **Native** — Move, Look, Jump, Posture, Lock… handled by character functions  
- **Ability** — LightAttack, Guard… forwarded to ASC by tag  

---

## Pieces in this project

| Piece | Role |
|-------|------|
| `UPurgatoriumLexInputConfig` | DA: InputAction ↔ GameplayTag |
| `UPurgatoriumLexInputComponent` | Bind native + ability actions from config |
| `UPurgatoriumLexAbilitySystemComponent` | Tag press/release queues + activate |
| `PurgatoriumLexGameplayTags` | Native `InputTag.*` definitions |
| Character `AbilityInputMappings` | Grant GA class + bind to InputTag |

---

## Old setup checklist (historical)

1. Create/configure **InputConfig** DA (Ability + Native rows).  
2. Character BP: **Input Component Class** = `PurgatoriumLexInputComponent`.  
3. Assign InputConfig on character.  
4. IMC maps keys → Input Actions.  
5. Create `UGameplayAbility` (BP or C++).  
6. Grant via **Ability Input Mappings** (class + InputTag) or DefaultAbilities.  
7. Optional gate: `CanActivateAbilityForInputTag` on character.

### Old “add SpecialAttack → GA_FireBall” steps

| Step | Where |
|------|--------|
| Create ability | Content / C++ `UGameplayAbility` |
| Input tag | `PurgatoriumLexGameplayTags` (`InputTag.SpecialAttack` existed) |
| InputConfig row | IA_SpecialAttack → InputTag.SpecialAttack |
| IMC | Key → IA_SpecialAttack |
| Character mappings | GA_FireBall → InputTag.SpecialAttack |
| Optional gate | `CanActivateAbilityForInputTag` branch |

---

## Code flow when a key was pressed

1. Enhanced Input fires the bound action  
2. InputComponent → `Input_AbilityInputTagPressed(InputTag)`  
3. Character may gate (`CanActivateAbilityForInputTag`) or buffer (GAS-era buffer)  
4. ASC finds specs for that tag and activates  

Guard **release** was special-cased to dispatch `InputTag.Parry` (design idea worth keeping even outside GAS).

---

## Footguns learned (still relevant as history)

- Custom `InputTag → SpecHandle` **side map** on ASC is not the same as putting tags on the **ability Spec** (Lyra-style); replication/client activation can disagree.  
- Character must actually use `PurgatoriumLexInputComponent` if code requires it.  
- Meta attributes like **Damage** should not replicate (cleanup done in AttributeSet).  

---

## Troubleshooting (if you still poke old Kick GA)

- InputConfig tag must match grant mapping / ability expectation  
- Ability must be granted (authority)  
- Cost / cooldown / `CanActivateAbility` may block  
- Light attack is sim-owned now — it **never reaches** ASC (`InputTag_LightAttack` → `FighterCombat->PressLightAttack`)  

---

## References (Epic)

- [First 60 minutes with GAS](https://dev.epicgames.com/community/learning/tutorials/8Xn9/unreal-engine-epic-for-indies-your-first-60-minutes-with-gameplay-ability-system)  
- [Lyra input settings](https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-input-settings-in-unreal-engine)
