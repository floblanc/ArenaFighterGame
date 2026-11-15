> **DEPRECATED FOR IMPLEMENTATION** — kept for learning.  
> Describes the **GAS-era client ability input buffer** on `PlayerCharacter`.  
> Future buffer belongs in the **sim input stream**, not ASC retry loops.  
> See [../Rollback_And_GGPO.md](../Rollback_And_GGPO.md), [../CombatSim_Design.md](../CombatSim_Design.md).

# Input buffering (GAS era)

## What problem it solved

When light attack (etc.) failed a gate (“airborne”, “already attacking”) or ASC did not activate, the press felt lost.  
**Buffering** kept the intent for N frames and retried.

Feel feature — especially as implemented: **client-side**, not the online authority.

---

## Core idea

```
Press LightAttack
  → gate fail or ASC no-activate
  → store { InputTag, BufferedFrame }
  → each tick: if still in window and gate ok → try activate again
  → success or expiry → remove
```

Defaults discussed: ~6 frames buffer; bufferable tags e.g. LightAttack, Roll.

---

## Lesson: don’t store a decrementing “frames remaining”

**Bad for any rewind story:**

```cpp
int32 FramesRemaining; // decremented each Tick
```

After rollback/resim, Tick count can disagree with that counter.

**Better:**

```cpp
int32 BufferedFrame; // when it was queued
// Remaining = BufferWindow - (CurrentFrame - BufferedFrame)
```

Expiry becomes a pure function of frame indices.

---

## How it hooked GAS (historical)

1. `Input_AbilityInputTagPressed` — buffer if gate fails or `ProcessAbilityInput` activates nothing.  
2. Character `Tick` — expire + `TryActivateAbilitiesByInputTag`.  
3. Explicitly **not replicated**; server only saw successful GAS activations.

That split is OK for “juice” under UE replication.  
Under **GGPO**, buffered intents should be part of **sim input / state** you save and load — not a parallel client array fighting ASC.

---

## What to reuse later

| Keep | Drop / rethink |
|------|----------------|
| Frame-stamped queue entries | Decrementing remaining counters |
| Short window for attack/roll feel | Coupling buffer consume to ASC |
| One clear bufferable set of actions | Claiming client buffer = rollback-ready |

When the sim owns light attack, a buffer is: “edge was pressed on frame F; try start move while `CanStart` within F..F+N” inside `FFighterCombatSim` / match input.
