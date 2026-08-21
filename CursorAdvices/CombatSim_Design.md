# Combat sim design

Fixed-tick fighter simulation: current combat architecture.

**Detailed review + FAQ + code maps:**  
`Source/PurgatoriumLex/Combat/CombatSim_REVIEW.md`  
(Prefer that file when reviewing a PR; this note is the short CursorAdvices version.)

---

## Layers

| Layer | Type | Role |
|-------|------|------|
| `FFighterSimState` | `F` / `USTRUCT` | Rewindable authority blob (no Actor pointers) |
| `FFighterFrameInput` | `F` / `USTRUCT` | Per-frame input (directions + edge buttons) |
| `FFighterCombatSim` | plain `F` class | `TickFrame(Input)` only mutation entry |
| `UFighterCombatComponent` | `U` component | 60 Hz catch-up from `DeltaTime`, events |
| `ULightAttackMoveSet` | `U` DataAsset | Per-posture timings / damage / soft montage |

Presentation (AnimBP, montage play) **reads** state / events. It does not invent hit/miss.

---

## Why not UGameplayAbility for light attacks

Modularity without rewindable authority: DataAsset rows instead of 7 GAs.  
Same button → sim starts “light attack” → `FindMove(Posture)`.

Deprecated GAS how-tos remain under [README.md](README.md) deprecated list.

---

## State sizing rule

Put in `FFighterSimState` only what you must **restore after rollback** to continue the fight.

- **In:** frame, posture (+ pending/live staling), move phase/timings/snapshot, health/hitstun/flags  
- **Out:** mesh, montages as authority, camera actors, full move-definition assets  
- **At attack start:** copy ints from DataAsset into state (active move timings)

Tuning constants currently live inside the struct for simplicity; can move to external config later.

---

## Fixed 60 Hz

Move data is in **frames**. Variable actor `Tick` deltas would make “8 startup” mean different real times and break determinism.

- Property: `UFighterCombatComponent::SimTickRate` (default 60)  
- Loop: `RunFixedTicks`  
- Rules step: `FFighterCombatSim::TickFrame`

**Limit today:** catch-up still driven by render `DeltaTime`. GGPO will call `TickFrame` from the session, not from that catch-up, once wired.

---

## Character ↔ sim contract

| Piece | Role |
|-------|------|
| `UFighterCombatComponent` | Owns sim, fixed ticks, posture delay / move-set config |
| `ActualPosture` | Presentation mirror for AnimBP (`VisibleAnywhere` / read-only) |
| `bAutoPlaySimAttackMontage` | Optional: play montage from move set on attack start |
| Light attack input | Always → `PressLightAttack()` (not ASC) |

No dual-path flags: posture + light attack are sim-only.

---

## Not in sim yet

Hitboxes, block/parry resolution, opponent damage exchange, GGPO save/load API.
