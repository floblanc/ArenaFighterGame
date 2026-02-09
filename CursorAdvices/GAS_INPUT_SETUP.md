# Gameplay Ability System with Enhanced Input via Input ID - Setup Guide

This document explains how to set up and use the Gameplay Ability System (GAS) with Enhanced Input via Input ID in your PurgatoriumLex project.

## Overview

The implementation follows the Lyra project pattern, allowing you to:
1. Map Enhanced Input Actions to GameplayTags (Input IDs)
2. Automatically trigger Gameplay Abilities when InputActions are pressed
3. Use a data-driven approach with InputConfig assets

## Architecture

### Key Components

1. **UPurgatoriumLexInputConfig** - Data asset that maps InputActions to GameplayTags
2. **UPurgatoriumLexInputComponent** - Extended EnhancedInputComponent with helper methods
3. **UPurgatoriumLexAbilitySystemComponent** - Extended ASC with Input ID support
4. **PurgatoriumLexGameplayTags** - Defines Input Tags used as Input IDs

### How It Works

1. **InputConfig Setup**: Create an InputConfig data asset that maps InputActions to GameplayTags
2. **Ability Setup**: In your Gameplay Ability Blueprint, set the Input Tag to match the InputConfig mapping
3. **Automatic Binding**: The InputComponent automatically binds InputActions to abilities with matching Input Tags
4. **Activation**: When an InputAction is pressed, it triggers `AbilityInputTagPressed`, which finds and activates abilities with matching Input Tags

## Setup Steps

### Step 1: Create InputConfig Data Asset

1. In the Content Browser, right-click and create a new **Data Asset**
2. Select **PurgatoriumLexInputConfig** as the class
3. Name it (e.g., `DA_PlayerInputConfig`)

### Step 2: Configure InputConfig

1. Open your InputConfig data asset
2. In the **Ability Input Actions** array, add entries for each ability:
   - **Input Action**: Select your Enhanced Input Action (e.g., `IA_LightAttack`)
   - **Input Tag**: Select the corresponding GameplayTag (e.g., `InputTag.LightAttack`)

Example mappings:
- `IA_LightAttack` → `InputTag.LightAttack`
- `IA_ChargeAttack` → `InputTag.ChargeAttack`
- `IA_Dash` → `InputTag.Dash`
- `IA_Guard` → `InputTag.Guard`

3. In the **Native Input Actions** array, add entries for non-ability inputs:
   - `IA_Move` → `InputTag.Move`
   - `IA_Look` → `InputTag.Look`
   - `IA_Jump` → `InputTag.Jump`
   - etc.

### Step 3: Set InputComponent Class in Character Blueprint

1. Open your Player Character Blueprint
2. In the **Class Defaults**, find **Input Component Class**
3. Set it to **PurgatoriumLexInputComponent** (instead of EnhancedInputComponent)

### Step 4: Assign InputConfig to Character

1. In your Player Character Blueprint's **Class Defaults**
2. Find the **Input Config** property
3. Assign your `DA_PlayerInputConfig` data asset

### Step 5: Create Gameplay Abilities with Input Tags

1. Create a new Gameplay Ability Blueprint (or use existing ones)
2. In the ability's **Activation** settings:
   - Set **Activation Policy** to **On Input Event** (or use **Try Activate Ability on Input Event**)
   - Set **Input Tag** to match your InputConfig mapping (e.g., `InputTag.LightAttack`)

**Important**: The Input Tag in the ability must exactly match the Input Tag in your InputConfig's Ability Input Actions array.

### Step 6: Grant Abilities to Character

In your character's **Default Abilities** array (or via code/blueprint), grant the abilities you want to use.

## Code Flow

### When Input is Pressed:

1. **Enhanced Input** detects InputAction press
2. **PurgatoriumLexInputComponent** calls `Input_AbilityInputTagPressed(InputTag)`
3. **PlayerCharacter** forwards to `AbilitySystemComponent->AbilityInputTagPressed(InputTag)`
4. **PurgatoriumLexAbilitySystemComponent** finds all abilities with matching Input Tag
5. **ProcessAbilityInput** (called each frame) activates matching abilities

### When Input is Released:

1. Similar flow, but calls `AbilityInputTagReleased(InputTag)`
2. Abilities can handle release logic in their blueprints

## Example: Light Attack Ability

### InputConfig Setup:
```
Ability Input Actions:
  - Input Action: IA_LightAttack
  - Input Tag: InputTag.LightAttack
```

### Ability Blueprint Setup:
```
Activation:
  - Input Tag: InputTag.LightAttack
  - Activation Policy: On Input Event
```

### Result:
When the player presses the key bound to `IA_LightAttack`, the ability with `InputTag.LightAttack` will automatically activate.

## Available Input Tags

The following Input Tags are defined in `PurgatoriumLexGameplayTags`:

- `InputTag.LightAttack`
- `InputTag.ChargeAttack`
- `InputTag.SpecialAttack`
- `InputTag.Guard`
- `InputTag.Dash`
- `InputTag.Jump`
- `InputTag.Move`
- `InputTag.Look`
- `InputTag.Run`
- `InputTag.Posture`
- `InputTag.LockUnlock`
- `InputTag.BreakGuard`

You can add more tags in `PurgatoriumLexGameplayTags.h` and `.cpp` if needed.

## Migration from Legacy System

The implementation maintains backward compatibility:
- If `InputConfig` is not set, the character falls back to direct InputAction bindings
- Existing abilities using `TryActivateAbilityByClass` will continue to work
- You can migrate gradually, one ability at a time

## Troubleshooting

### Abilities Not Activating

1. **Check InputConfig**: Ensure the Input Tag in InputConfig matches the Input Tag in the ability
2. **Check InputComponent**: Verify the character uses `PurgatoriumLexInputComponent`
3. **Check Ability Granted**: Ensure the ability is granted to the character
4. **Check Input Tag**: The Input Tag must be set in the ability's activation settings
5. **Check Logs**: Look for errors in the Output Log

### Input Actions Not Working

1. **Check Mapping Context**: Ensure your InputMappingContext is added to the subsystem
2. **Check InputConfig**: Verify InputActions are mapped in InputConfig
3. **Check Native vs Ability**: Ensure actions are in the correct array (Native vs Ability Input Actions)

## Additional Notes

- The system processes ability input each frame in `TickComponent`
- Abilities are activated automatically when their Input Tag matches a pressed InputAction
- You can have multiple abilities with the same Input Tag (they'll all try to activate)
- Use ability tags and activation groups to control which abilities can activate together

## References

- [Epic's GAS Tutorial](https://dev.epicgames.com/community/learning/tutorials/8Xn9/unreal-engine-epic-for-indies-your-first-60-minutes-with-gameplay-ability-system)
- [GAS Best Practices - Input ID](https://dev.epicgames.com/community/learning/tutorials/DPpd/unreal-engine-gameplay-ability-system-best-practices-for-setup#howdoesgivingandtriggeringabilitiesviainputidwork?)
- [Lyra Input Settings](https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-input-settings-in-unreal-engine?application_version=5.0)

