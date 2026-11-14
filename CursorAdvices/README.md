# CursorAdvices — Project notes index

Scratchpad / learning notes for **PurgatoriumLex** (arena fighter, UE5, C++-first).

These are **not** engine docs. Prefer **`Source/` + Content + Epic UE 5.8 docs**.  
If a checklist step does not match the Details panel, trust the C++ `UPROPERTY` / `UMETA(DisplayName)` over an older note.

**Status labels used below:**

| Label | Meaning |
|-------|---------|
| **Current** | Matches Source; follow this |
| **Editor** | Human Blueprint steps after C++ compile |
| **Design (future)** | Intentional; not implemented — do not build yet |
| **Reference** | External game research; not a project checklist |
| **Partial / debt** | Some code exists; not sim authority / not finished |
| **Deprecated** | Learning only — do not implement combat this way |

---

## Current direction (read these first)

| File | Status | Subject |
|------|--------|---------|
| [Architecture_Decisions.md](Architecture_Decisions.md) | **Current** | GAS vs rollback, roadmap, what not to build yet |
| [CombatSim_Design.md](CombatSim_Design.md) | **Current** | Fixed-tick sim, state blob, move DataAssets |
| [Rollback_And_GGPO.md](Rollback_And_GGPO.md) | **Current** | What “rollback-ready” means; GGPO later |
| [Posture_System.md](Posture_System.md) | **Current** | Sim authority + `ActualPosture` + snapshot |
| [Posture_Spam_Exhaustion_Design.md](Posture_Spam_Exhaustion_Design.md) | **Design (future)** | Anti-spam / exhaustion (not implemented) |
| [UE5_Basics_UObject_And_Naming.md](UE5_Basics_UObject_And_Naming.md) | **Current** | `U`/`A`/`F` naming, UObject vs sim |
| [UE5_8_Docs_Map.md](UE5_8_Docs_Map.md) | **Current** | Official UE 5.8 docs index (Epic = engine API truth) |
| [Blueprint_Modification_Checklist.md](Blueprint_Modification_Checklist.md) | **Editor** | Verified steps after combat/lock-on C++ |

**Implementation review in Source (authoritative for the sim PR):**  
`Source/PurgatoriumLex/Combat/CombatSim_REVIEW.md`

**God-class peel plan:**  
`Source/PurgatoriumLex/Characters/PlayerCharacter_Decomposition.md`

**AnimBP posture tilt:**  
[PostureShoulderAnimationSetup.md](PostureShoulderAnimationSetup.md) — **Current** how-to; prefer existing **`ABP_Posture`**

---

## Mechanics notes (check status before coding)

| File | Status | Subject |
|------|--------|---------|
| [RollStalingMechanics.md](RollStalingMechanics.md) | **Reference** + **Partial** on character | SSBU dodge staling; character has a port, not in sim |
| [TechImplementation.md](TechImplementation.md) | **Partial / debt** | Character+ASC tech windows; migrate to sim later |
| [StaleMovesMechanics.md](StaleMovesMechanics.md) | **Reference** | Smash attack-queue staling — not in project |
| [CS2CrouchSpeedPenalty.md](CS2CrouchSpeedPenalty.md) | **Reference** | Static crouch speed — not wired |

---

## Deprecated for implementation (kept for learning)

Under **[`deprecated/`](deprecated/README.md)** — one subject per file.

| File | Subject |
|------|---------|
| [deprecated/GAS_What_It_Is.md](deprecated/GAS_What_It_Is.md) | What Epic’s GAS is |
| [deprecated/GAS_Input_And_Abilities.md](deprecated/GAS_Input_And_Abilities.md) | Old InputTag → ASC; LightAttack now diverted to sim |
| [deprecated/GAS_Vs_Lyra.md](deprecated/GAS_Vs_Lyra.md) | Lyra vs this project (simplified) |
| [deprecated/Tags_As_State_ASC.md](deprecated/Tags_As_State_ASC.md) | ASC tags as state → sim flags for combat |
| [deprecated/Input_Buffering_GAS_Era.md](deprecated/Input_Buffering_GAS_Era.md) | Client ability buffer + frame-stamp lesson |
| [deprecated/Early_Rollback_Habits.md](deprecated/Early_Rollback_Habits.md) | Useful determinism habits vs oversold claims |

**Do not** extend combat through GAS if rollback remains the priority.

---

## Local-only files

Your machine may have large dumps such as `UE58_Engine_Tree_Index.md` (engine tree listing). Those are **indexes**, not architecture advice. Prefer [UE5_8_Docs_Map.md](UE5_8_Docs_Map.md) + Epic docs for engine facts. Cloud agents only see what is in the Git workspace.

---

## House rules for this folder

1. **One clear subject per file.** Put a status banner at the top when the file is not **Current**.  
2. If advice conflicts with `Source/PurgatoriumLex/Combat/`, **code wins** for architecture.  
3. If advice conflicts with **Epic UE 5.8 documentation / API** on engine facts, **Epic wins** — update our notes. Index: [UE5_8_Docs_Map.md](UE5_8_Docs_Map.md).  
4. Mark thrown approaches **deprecated**, don’t delete explanations you still want.  
5. Deprecated material lives under `deprecated/` with its own README.  
6. **Roadmap gate:** no new roll/tech/staling/grab work until light-attack hit → block → stun exists in the sim.
