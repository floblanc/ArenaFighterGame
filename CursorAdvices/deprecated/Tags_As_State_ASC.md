> **DEPRECATED FOR IMPLEMENTATION (as ASC authority)** — kept for learning.  
> The *idea* (named states, no duplicate bools) remains.  
> **Storage** moved to sim bitflags / `FFighterSimState`, not ASC loose tags as truth.  
> See [../CombatSim_Design.md](../CombatSim_Design.md), [../Architecture_Decisions.md](../Architecture_Decisions.md).

# Tags as state (ASC-era idea)

## Goal (still good)

- One concept → one state label (`Attacking`, `Hitstun`, …)  
- No parallel `bIsAttacking` + tag meaning the same thing  
- Gate actions by required/blocked conditions  

## How it was proposed with GAS

1. Declare tags (`State.Grounded`, `State.Attacking`, `State.Hitstun`, …) in ini + native C++.  
2. Posture direction stays an **`EPosture` enum**; only `State.PostureChanging` as a busy flag.  
3. Set/clear via `ASC->AddLooseGameplayTag` / `RemoveLooseGameplayTag`.  
4. Query with `HasMatchingGameplayTag`.  
5. GAS abilities use **ActivationRequiredTags** / **ActivationBlockedTags**.

```cpp
// Historical pattern
ASC->AddLooseGameplayTag(State_Attacking);
if (ASC->HasMatchingGameplayTag(State_Hitstun)) { /* blocked */ }
```

## Why ASC loose tags are a weak authority for this game

- Loose tags are easy to use **locally**; people often assume replication they don’t have.  
- Not a clean **copy/hash/restore** blob for GGPO.  
- Dual systems appeared anyway (bools + tags).  

## What replaced it for combat

| Old (ASC) | Current (sim) |
|-----------|----------------|
| `State.Attacking` loose tag | `FighterStateFlags::Attacking` on `FFighterSimState` |
| `State.PostureChanging` | `FighterStateFlags::PostureChanging` |
| Gate via ability tag rules | `CanStartLightAttack` / phase checks in `FFighterCombatSim` |

Transitional code may still **mirror** `State.PostureChanging` onto ASC for old BP queries — that mirror is not authority.

## Migration lesson (still valid)

Pick **one** source of truth.  
If you add a flag/bit/tag, delete the boolean that meant the same thing when you touch that system.
