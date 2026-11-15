# Tech (knockdown recovery) — status + design

> **Status (2026-11-14, vs Source):** Partially implemented on **`APlayerCharacter`**, **not** in `FFighterCombatSim`.  
> Uses **`SimulationFrame`** windows + **ASC loose tags** (`State.Tech`, etc.).  
> **Do not expand** this until the light-attack vertical slice (hit → block → stun) works — see [Architecture_Decisions.md](Architecture_Decisions.md) roadmap and `PlayerCharacter_Decomposition.md` Phase D.  
> Target authority later: sim flags / sim windows, **not** ASC tags.

---

## What “tech” means

Defensive recovery on knockdown / wall splat: press in a short window → roll/stand instead of lying there for oki.

Common types: ground tech, air tech, wall tech (directional variants).

---

## What the code does today

| Piece | Location | Notes |
|-------|----------|--------|
| Window length / lockout | `TechWindowFrames`, `TechLockoutFrames` on character | Frame ints; defaults 11 / 40 |
| Window start | `StartTechWindow` from **Landed** / **NotifyHit** | Predictive “will hit soon” path was removed on purpose |
| Input | Guard press while techable → `OnTechInputPressed` | Also hold detection |
| Success | `PerformTech(ETechType)` | Adds `State.Tech` via ASC; clears other State.* tags; logs; montage/movement left to BP/ability |
| Gate | `CanTech()` | Checks `bIsTechable`, lockout, `State.Tech` on ASC |

This is **playable scaffolding**, not GGPO-ready combat authority. ASC loose tags and character Tick `SimulationFrame` are the same class of problem called out in [deprecated/Early_Rollback_Habits.md](deprecated/Early_Rollback_Habits.md).

There is **no** `InputTag.Tech` in `PurgatoriumLexGameplayTags` — tech piggybacks Guard today.

---

## Design rules when you migrate it into the sim

1. Tech window + lockout live in **`FFighterSimState`** (or a sibling combat blob), advanced only in `TickFrame`.  
2. Tech success sets a **sim flag / phase**, not `ASC->AddLooseGameplayTag` as truth.  
3. Presentation (montage, CMC impulse) listens to sim events — same pattern as light attack.  
4. Keep contact-driven windows (`Landed` / hit notify → latch “contact this frame” into **input or a one-frame sim event**), not predictive traces as authority.

---

## Do not follow (old advice)

- “Add `State.Tech` on ASC and you’re rollback compatible” — false.  
- “Buffer `InputTag_Tech`” — tag does not exist; don’t invent a GAS ability for tech while combat is sim-owned.  
- Expanding tech polish before hitboxes — blocked by roadmap.

---

## References

- SSBU-style windows / lockout (design inspiration only)  
- [Rollback_And_GGPO.md](Rollback_And_GGPO.md)  
- `Source/PurgatoriumLex/Characters/PlayerCharacter.cpp` (Tech System section)  
- `Source/PurgatoriumLex/Characters/PlayerCharacter_Decomposition.md` Phase D
