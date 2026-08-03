> **DEPRECATED FOR IMPLEMENTATION** — kept for learning.  
> Combat authority is the fixed-tick sim (`Source/PurgatoriumLex/Combat/`), not GAS.  
> Current direction: [../Architecture_Decisions.md](../Architecture_Decisions.md).

# What is GAS (Gameplay Ability System)?

Epic’s framework (plugin `GameplayAbilities`) for **modular gameplay actions**: abilities, effects, attributes, cues, tags.

---

## Main pieces

| Piece | Role |
|-------|------|
| **ASC** (`UAbilitySystemComponent`) | Owns granted abilities, tags, applies effects |
| **AttributeSet** | Numerics (Health, Stamina, …) + prediction/replication hooks |
| **GameplayAbility** | Activatable action (cost, cooldown, tasks, montage) |
| **GameplayEffect** | Mods to attributes/tags (damage, buff, duration) |
| **GameplayCue** | Mostly presentation (VFX/SFX) from events |
| **GameplayTag** | Hierarchical labels (input, state, filtering) |

Typical flow: **grant → activate (input/event) → tasks → apply GE → attributes change → cues**.

---

## Why projects use it

- Data-driven powers without giant character `switch`es  
- Built-in prediction for **UE multiplayer** (server authority)  
- Shared language with Lyra / many UE samples  

This project once used a **Lyra-inspired** path: Enhanced Input → InputTag → ASC.  
Details: [GAS_Input_And_Abilities.md](GAS_Input_And_Abilities.md), [GAS_Vs_Lyra.md](GAS_Vs_Lyra.md).

---

## Why it was demoted as combat authority here

Rollback needs a small serializable sim (`FFighterSimState` + `TickFrame`).  
GAS montages, timers, GE prediction target a different net model.

**What survived the idea:** modular move definitions → `ULightAttackMoveSet` + shared sim verbs.  
See [../CombatSim_Design.md](../CombatSim_Design.md).

Do not add new **combat** GameplayAbilities if rollback remains the online plan.
