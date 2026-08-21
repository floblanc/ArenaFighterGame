> **HISTORICAL / PARTIALLY OUTDATED** — kept for learning.  
> Early notes claimed broad “rollback compatibility.” That oversold the work.  
> Current plan: [../Rollback_And_GGPO.md](../Rollback_And_GGPO.md) + `Source/PurgatoriumLex/Combat/`.  
> Below: **habits worth keeping** vs **claims to ignore**.

# Early rollback notes — habits vs hype

Merged from old `ROLLBACK_NETCODE_FIXES`, tech rollback notes, and related advice.

---

## Habits worth keeping

| Habit | Why |
|-------|-----|
| Avoid `FDateTime::Now()` / wall clock in combat rules | Different machines, non-replayable |
| Prefer **frame indices** over float timers for windows | Same language as fighter data + GGPO |
| Avoid `static` locals holding gameplay arrays in hot paths | Hidden state across calls/resims |
| Prefer **contact callbacks** over predictive traces for tech windows | “Will hit soon” traces diverge easily |
| Read movement input from movement component state you control | Not ad-hoc last-vector guesses |
| Cut log spam in hot paths | Noise ≠ determinism, hurts profiling |

These are **necessary hygiene**, not a rollback implementation.

---

## Claims that were oversold

| Claim | Reality |
|-------|---------|
| `SimulationFrame++` in actor `Tick` = rollback-ready | Still tied to variable render/update pacing unless a fixed sim owns the counter |
| Moving camera lock-on off Tick = netcode done | Presentation timing ≠ fighter state serialize/resim |
| Processing GAS input on press = GGPO | GAS still not a rewindable combat core |
| Client ability buffer + frame stamp = online buffer | Feel-only unless inside shared sim state |
| Tech system “rollback compatible” via tags/frames | Tech still sat on character + ASC; not a GGPO session |

---

## Tech-specific note (historical)

Advice shifted tech windows to **Landed / NotifyHit** instead of predictive line traces, and used frame stamps for window/lockout.  
Directionally sensible for determinism **once** tech lives in the sim.  
Until then: treat `TechImplementation.md` (mechanics) as design; don’t trust old “compatibility” checklists.

---

## What “ready for GGPO” actually requires

1. Fixed-tick combat sim with copyable state *(started)*  
2. `Save` / `Load` / `Advance(input)` API  
3. Same inputs → same state hash in one process  
4. Then GGPO (or equivalent) session  

See [../Rollback_And_GGPO.md](../Rollback_And_GGPO.md).
