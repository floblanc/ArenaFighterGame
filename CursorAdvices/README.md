# CursorAdvices — Project notes index

Scratchpad / learning notes for **PurgatoriumLex** (arena fighter, UE5, C++-first).

These are **not** engine docs and not always in sync with code. Prefer code + this map.

---

## Current direction (read these first)

| File | Subject |
|------|---------|
| [Architecture_Decisions.md](Architecture_Decisions.md) | GAS vs rollback choice, roadmap, what not to build yet |
| [CombatSim_Design.md](CombatSim_Design.md) | Fixed-tick sim, state blob, move DataAssets, code map |
| [Rollback_And_GGPO.md](Rollback_And_GGPO.md) | What “rollback-ready” means, GGPO later, honesty about old notes |
| [Posture_System.md](Posture_System.md) | Posture as sim authority + always-on visual + attack snapshot |
| [UE5_Basics_UObject_And_Naming.md](UE5_Basics_UObject_And_Naming.md) | `U`/`A`/`F` naming, what UObject is and why |

**Implementation review in Source (authoritative for the sim PR):**  
`Source/PurgatoriumLex/Combat/CombatSim_REVIEW.md`

**AnimBP posture tilt (still valid):**  
[PostureShoulderAnimationSetup.md](PostureShoulderAnimationSetup.md)

---

## Mechanics notes (ideas, not all wired)

| File | Subject |
|------|---------|
| [RollStalingMechanics.md](RollStalingMechanics.md) | Roll / dodge staling |
| [StaleMovesMechanics.md](StaleMovesMechanics.md) | Move staling concepts |
| [TechImplementation.md](TechImplementation.md) | Tech (SSBU-style) ideas |
| [CS2CrouchSpeedPenalty.md](CS2CrouchSpeedPenalty.md) | Reference: static crouch penalty |

---

## Deprecated for implementation (kept for learning)

Cleaned and merged under **[`deprecated/`](deprecated/README.md)** — one subject per file.

| File | Subject |
|------|---------|
| [deprecated/GAS_What_It_Is.md](deprecated/GAS_What_It_Is.md) | What Epic’s GAS is |
| [deprecated/GAS_Input_And_Abilities.md](deprecated/GAS_Input_And_Abilities.md) | Old InputTag → ASC pipeline + add-ability checklist |
| [deprecated/GAS_Vs_Lyra.md](deprecated/GAS_Vs_Lyra.md) | Lyra vs this project (simplified) |
| [deprecated/Tags_As_State_ASC.md](deprecated/Tags_As_State_ASC.md) | Tags-as-state via ASC → evolved to sim flags |
| [deprecated/Input_Buffering_GAS_Era.md](deprecated/Input_Buffering_GAS_Era.md) | Client ability buffer + frame-stamp lesson |
| [deprecated/Early_Rollback_Habits.md](deprecated/Early_Rollback_Habits.md) | Useful determinism habits vs oversold claims |

**Do not** extend combat through GAS if rollback remains the priority.

---

## House rules for this folder

1. **One clear subject per file.**
2. If advice conflicts with `Source/PurgatoriumLex/Combat/`, **code wins**.
3. Mark thrown approaches **deprecated**, don’t delete explanations you still want.
4. Deprecated material lives under `deprecated/` with its own README.
