# Adding a New Ability (e.g. SpecialAttack → GA_FireBall)

This guide matches the [Your First 60 Minutes with Gameplay Ability System](https://dev.epicgames.com/community/learning/tutorials/8Xn9/unreal-engine-epic-for-indies-your-first-60-minutes-with-gameplay-ability-system) tutorial and how this project wires input to GAS (Enhanced Input → InputTag → ASC activates ability).

---

## 1. Create the ability asset/class

- Create **GA_FireBall** (Blueprint Gameplay Ability, or C++ class derived from `UGameplayAbility`).
- In the ability:
  - Add any **cost/cooldown** gameplay effects you want.
  - Implement the fireball logic (spawn projectile, apply damage, etc.).

---

## 2. Ensure you have an input tag for it

The project already declares **InputTag_SpecialAttack** in `PurgatoriumLexGameplayTags.h` / `.cpp`.

If you add a **new** input tag later:

- Declare it in **`Source/PurgatoriumLex/PurgatoriumLexGameplayTags.h`**
- Define it in **`Source/PurgatoriumLex/PurgatoriumLexGameplayTags.cpp`**

---

## 3. Bind the input action to the input tag (InputConfig)

In your **Input Config** data asset (e.g. `DA_InputConfig`):

- Add (or confirm) an entry in **Ability Input Actions**:
  - **Input Action**: `IA_SpecialAttack`
  - **Input Tag**: `InputTag.SpecialAttack`

---

## 4. Map a key/button to that Input Action (IMC)

In your **Input Mapping Context** (e.g. `IMC_Default` or `IMC_Fighting`):

- Map the key/button (e.g. `E`, right mouse, controller button) to **IA_SpecialAttack**.

---

## 5. Grant the ability and tie it to the same input tag (Character Blueprint)

In **BP_CharacterPlayer** (Class Defaults):

- Under **Abilities → Ability Input Mappings**, add a row:
  - **Ability Class**: `GA_FireBall`
  - **Input Tag**: `InputTag.SpecialAttack`

At runtime, `GrantAbilitiesWithInputTags()` runs and the ASC stores this mapping so that when **InputTag.SpecialAttack** is pressed, **GA_FireBall** is activated.

---

## 6. Optional: Gate when the ability can run

If you want to block SpecialAttack in certain states (e.g. not while jumping):

- In **`PlayerCharacter.cpp`**, inside **`CanActivateAbilityForInputTag_Implementation`**, add a branch for the new tag and a helper if needed, e.g.:

  ```cpp
  if (InputTag == InputTag_SpecialAttack) return CanPerformSpecialAttack();
  ```

- Implement **`CanPerformSpecialAttack()`** / **`CanPerformSpecialAttack_Implementation()`** with your conditions (same pattern as **CanPerformLightAttack**).

---

## 7. Test checklist

- Press the mapped key → flow should be:
  1. IMC key mapping → **IA_SpecialAttack**
  2. InputConfig → **InputTag.SpecialAttack**
  3. **Input_AbilityInputTagPressed(InputTag.SpecialAttack)**
  4. **CanActivateAbilityForInputTag** (if you added a gate)
  5. **ASC->AbilityInputTagPressed** → **GA_FireBall** activates

If nothing happens, check:

- **IMC**: key is mapped to the correct Input Action.
- **InputConfig**: Ability Input Actions has a row for that action and **InputTag.SpecialAttack**.
- **Ability Input Mappings**: BP_CharacterPlayer has **GA_FireBall** + **InputTag.SpecialAttack**.
- Ability activation: cost, cooldown, or **CanActivateAbility** might be blocking.

---

## Summary table

| Step | Where | What |
|------|--------|------|
| 1 | Content / C++ | Create **GA_FireBall** (ability logic, cost, cooldown) |
| 2 | `PurgatoriumLexGameplayTags` | Use existing **InputTag_SpecialAttack** (or add new tag) |
| 3 | Input Config (DA) | **Ability Input Actions**: IA_SpecialAttack → InputTag.SpecialAttack |
| 4 | Input Mapping Context | Map key/button → **IA_SpecialAttack** |
| 5 | BP_CharacterPlayer | **Ability Input Mappings**: GA_FireBall → InputTag.SpecialAttack |
| 6 | `PlayerCharacter.cpp` (optional) | **CanActivateAbilityForInputTag** + **CanPerformSpecialAttack** |
| 7 | Play | Test and fix IMC / InputConfig / mappings / ability activation |
