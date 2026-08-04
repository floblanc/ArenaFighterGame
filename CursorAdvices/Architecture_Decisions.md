# Architecture decisions (current)

Decisions from design reviews for PurgatoriumLex. One page: what we chose and why.

---

## Product shape

- Third-person **versus** arena fighter
- **Oriented combat** (For Honor / KCD-like) with **6 postures + neutral**
- Posture from movement (passive, while lock-on) and dedicated direction input (active overwrite)
- C++-first; Blueprint only for small wiring / AnimBP / DataAssets
- Future: local splitscreen 1v1, then online with **rollback**

---

## Priority decision: rollback over GAS for combat

| Liked about GAS | Problem for this game |
|-----------------|------------------------|
| Modular abilities, character ≠ hard-coded moves | Built for UE replication / prediction, not GGPO-style rewind |
| InputTag → activate ability | Montages, timers, GE side effects don’t rewind cleanly |

**Choice:** combat **authority** = custom fixed-tick sim.  
**Keep from GAS idea:** modular **data** (move tables per fighter).  
**GAS status:** freeze new combat features on it; starve, then remove from hot path. Optional later only for non-rewindable meta (not required).

See also: [Rollback_And_GGPO.md](Rollback_And_GGPO.md), [deprecated/GAS_What_It_Is.md](deprecated/GAS_What_It_Is.md).

---

## Dual state (bools vs tags)

Old `bIsAttacking` / `bIsGuarding` on the character were fast prototypes.

**Direction:** named combat conditions in **sim state** (bitflags / future small tag set **inside** `FFighterSimState`), not parallel bools and not ASC loose tags as authority.

ASC tags may still appear as transitional mirrors — not truth.

---

## Posture is central

- Always visual (idle/walk upper-body tilt) — AnimBP reads posture
- Also drives attack / defense / maybe grabs later
- **Sim owns posture**; AnimBP / `ActualPosture` mirror for presentation
- On attack press: **snapshot** posture (swing doesn’t drift mid-anim)

→ [Posture_System.md](Posture_System.md)

---

## Roadmap (order)

0. **Written rule:** sim authority; GAS frozen for combat; splitscreen before online  
1. **Sim skeleton:** state + fixed tick + posture in sim *(started in `Source/.../Combat/`)*  
2. **Vertical slice:** light attack by posture → active frames → hit → block match → stun  
3. **Decompose** `PlayerCharacter` (god-class)  
4. **Local 1v1** splitscreen  
5. **GGPO / rollback session** after local determinism (hash/replay) works  

**Do not** expand tech / roll staling / grab tags until the vertical slice works.

---

## Multi-character moves (shared verb, unique data)

```
LightAttack (same code for all fighters)
  + posture snapshot
  → this fighter’s DataAsset row (frames, damage, montage)
```

That is normal fighter design, not “generic programming” in the C++ sense.

→ [CombatSim_Design.md](CombatSim_Design.md)

---

## Blueprint surface (keep small)

1. Create `LightAttackMoveSet` Data Asset  
2. Assign on `FighterCombat` component  
3. AnimBP: keep reading `ActualPosture` if already set up  

No big ability Blueprint graphs for core combat.
