# Rollback and GGPO

What rollback means for this project, what is prepared, what is not, and where GGPO fits.

---

## Goal

Online 1v1 with **rollback netcode** (predict remote input, on late input rewind and resim).  
Local splitscreen first — same sim, two input slots.

---

## GGPO — is it possible in UE5?

**Yes.** GGPO (or ggpo-x / a GGPO-style session) can wrap a deterministic sim:

1. Save fighter state  
2. Apply inputs  
3. Advance one sim frame  
4. On misprediction: load older state, resim with corrected inputs  
5. Present mesh/camera/audio from the latest predicted/confirmed state  

GGPO does **not** replace Unreal. It orchestrates **your** `Save` / `Load` / `Advance(input)`.

### Hard parts in UE (expect these)

- `CharacterMovement` / physics are hostile as pure rollback state  
- Montages / audio need display-only handling on rewind  
- Serialize **your** blob (`FFighterSimState`), not the whole `UWorld`

### Does GGPO make rollback “easier”?

- **Easier than inventing the session from zero:** yes  
- **Easier than building a deterministic sim:** no — GGPO assumes that exists  

Order: local sim → same inputs ⇒ same state hash → then GGPO.

---

## What the Combat/ sim already prepares

| Ready as scaffolding | Missing for real GGPO |
|----------------------|------------------------|
| Fixed tick (60 Hz target) | Network session + input delay |
| `FFighterSimState` copyable blob | `Save`/`Load` callbacks wired to GGPO |
| `FFighterFrameInput` | Remote input exchange |
| Single `TickFrame` entry | Drive ticks from session, not only component `DeltaTime` |
| `ComputeStateHash` stub | Peer checksum compare |
| Combat leaving GAS | Movement / lock-on policy in or out of sim |

See `Source/PurgatoriumLex/Combat/` and [CombatSim_Design.md](CombatSim_Design.md).

---

## Honesty about older “rollback” notes

Older scattered files were merged into  
[`deprecated/Early_Rollback_Habits.md`](deprecated/Early_Rollback_Habits.md).

**Useful habits:** avoid wall-clock in combat, prefer frame indices, avoid predictive traces for windows, no sneaky `static` gameplay state.

**Not sufficient:** Tick frame counters, client GAS buffers, ASC tags as GGPO readiness.

This file + the Combat sim are the current story.

---

## Practical next steps toward GGPO

1. Finish local vertical slice (hit + block) in the sim  
2. Add explicit `SaveState` / `LoadState` on the sim API  
3. Replay or dual-instance hash test (same inputs → same hash)  
4. Integrate GGPO (or equivalent) around that API  
5. Presentation: don’t let anim time decide combat on resim  

Do **not** add the library before (1)–(3) unless you want desync debugging with no foundation.
