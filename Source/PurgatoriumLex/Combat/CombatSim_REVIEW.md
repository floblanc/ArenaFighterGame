# Combat Sim — Review Guide

This PR adds a **fixed-tick fighter combat simulation** in C++ and wires posture + light-attack *timing* into it. It does **not** add hitboxes, block, or rollback netcode yet.

Use this doc to judge **architecture and intent**, not only “does it compile.” Disagreeing with a “why” is useful feedback.

**How to use code refs:** each point lists `path:line` (or a short range). Open that spot in the IDE while reading.

---

## UE5 naming conventions (quick cheat sheet)

Epic’s usual C++ prefixes (not enforced by the compiler — convention + Unreal Header Tool):

| Prefix | Meaning | Example in this PR |
|--------|---------|-------------------|
| `U` | `UObject`-derived (assets, components, most engine objects) | `UFighterCombatComponent`, `ULightAttackMoveSet` |
| `A` | `AActor`-derived (things placeable in a level / with a transform root) | `APlayerCharacter` |
| `F` | Plain C++ struct/class **not** in the UObject hierarchy (or reflected `USTRUCT`) | `FFighterSimState`, `FFighterCombatSim`, `FVector` |
| `E` | Enum | `EPosture`, `ECombatMovePhase` |
| `I` | Interface (`UINTERFACE` + `IMyInterface`) | (none here yet) |
| `T` | Template helper | `TArray`, `TObjectPtr`, `TWeakObjectPtr` |
| `b` | Bool member | `bLightAttackPressed` |
| `Get` / `Set` | Accessors | `GetSimPosture` |

Also:

- Member functions: `PascalCase` (`TickFrame`, `StartLightAttack`)
- Modules / API macros: `PURGATORIUMLEX_API`
- Blueprint-facing types often need `USTRUCT`/`UCLASS` + `GENERATED_BODY()` so UHT generates reflection

**Why `FFighterSimState` uses `F`:** it is a value blob (copyable POD-ish reflected struct), not a `UObject`. Same reason `FVector` is `F`, not `U`.

---

## Architecture decisions (WHY) + code map + FAQ

### 1) Why a custom sim instead of GAS for combat?

**Why:** Rollback needs rewindable combat authority. GAS (`UGameplayAbility`, GE prediction, montage tasks) is built for server auth + prediction, not “restore state and resim N frames.”

**Where in code:**

| Idea | Location |
|------|----------|
| Sim owns light-attack timing | `Combat/FighterCombatSim.cpp` → `TickLightAttack` / `StartLightAttack` / `AdvanceCurrentMove` |
| Light attack never reaches ASC | `Characters/PlayerCharacter.cpp` → `Input_AbilityInputTagPressed` (`InputTag_LightAttack` → `PressLightAttack`) |
| Posture / light attack always sim | No dual-path flags — legacy character posture Tick deleted |
| Old GAS still present | `AbilitySystem/*`, `GrantAbilitiesWithInputTags`, `Build.cs` → `"GameplayAbilities"` (Kick etc. until migrated) |

#### FAQ — Should GAS be completely removed?

**Not in this PR. Eventually for combat: yes, if rollback stays the priority.**

Recommended sequence (not all at once):

1. **Now (done in this PR):** posture + light-attack timing go through the sim only. Dual-path flags removed.
2. **After hitboxes + local 1v1 feel good:** stop granting combat GAs; delete Kick/SelfDamage combat abilities from character defaults.
3. **Later cleanup:** remove ASC from the fighter hot path if nothing else needs it; drop `GameplayAbilities` module dependency only when AttributeSet/HUD/etc. are replaced or unused.
4. **Optional keep:** GAS (or a thin leftover) for *non-rewindable* meta (menus, unlocks) — not required.

Removing GAS tomorrow would burn time rewriting HUD/attributes without making light attacks better. **Starve it, then delete.**

---

### 2) Why plain `FFighterSimState` (no Actor pointers)?

**Why:** Rollback copies/hashes/restores a blob. `AActor*`, component pointers, montage instances are world/presentation glue — bad inside the authority state.

**Where in code:**

| Idea | Location |
|------|----------|
| State definition | `Combat/CombatTypes.h` → `struct FFighterSimState` |
| Explicit “no Actor*” rationale in header | same file, comment block above the struct |
| Sim mutates only this blob | `Combat/FighterCombatSim.h` → member `FFighterSimState State` |
| Presentation mirrors out | `Characters/PlayerCharacter.cpp` → `SyncPresentationFromCombatSim` |

#### FAQ — Naming (`F` prefix)?

See cheat sheet above. `F` = non-UObject (or `USTRUCT` value type). Correct for this.

#### FAQ — Should state be as small as possible? How were fields chosen?

**Yes — small is a goal.** Rule of thumb:

**Put in state only what you must restore to continue the fight correctly after a rollback.**

| In `FFighterSimState` now | Why |
|---------------------------|-----|
| `Frame` | All delays/timing key off sim time |
| Posture + pending + delay stamps + staling fields | Your posture system is combat + visual authority |
| Move phase + local frame + startup/active/recovery + snapshot posture + pending damage | Light attack machine |
| `Health`, `HitstunRemaining`, `StateFlags` | Needed as soon as hits exist; cheap to carry early |

| Kept out of state (on purpose) | Why |
|--------------------------------|-----|
| Mesh, AnimBP, montage pointers | Presentation |
| Camera / lock-on actor | Not required to resolve “did hit connect” (yet); can stay outside or use an **id** later |
| Move *definitions* (full DataAsset) | Shared read-only data; store a **move id** or copy only active timings into state when attack starts (what we do: copy ints into state at `StartLightAttack`) |
| Enhanced Input devices | Input is a separate `FFighterFrameInput` stream |

**Honest critique:** posture *tuning constants* (`PosturePenaltyPerChange`, `PostureMaxPenalty`, delays) currently live *inside* the state struct. For a minimal rollback blob, those could be **config outside state** (only live values like `Posture`, `Pending*`, `PostureStalePenalty` need restore). Leaving constants in state is slightly fat but simple for v1 — fair to slim later.

---

### 3) Why `FFighterCombatSim` is not a `UObject`?

**Why:** Rules should not depend on UE object lifetime, GC, or actor Tick. One mutation API: `TickFrame`.

**Where in code:**

| Idea | Location |
|------|----------|
| Non-UObject sim class | `Combat/FighterCombatSim.h` → `class FFighterCombatSim` |
| Only mutation entry | `TickFrame` in `.h` / `.cpp` |
| UObject bridge (component) | `Combat/FighterCombatComponent.h` → `UFighterCombatComponent` owns an `FFighterCombatSim Sim` |

#### FAQ — What is a `UObject`? Why does UE use it?

Unreal’s runtime is built around **`UObject`**: the base of almost everything the engine reflects, serializes, GCs, and exposes to Blueprints.

Rough mental model (if you know normal C++ well):

| Concept | Plain C++ | Unreal `UObject` world |
|---------|-----------|-------------------------|
| Allocate | `new` / stack / your allocator | Prefer `NewObject<T>()`, `CreateDefaultSubobject`, asset loading |
| Destroy | `delete` / RAII | Garbage collected (refs from other UObjects / roots keep alive) |
| “What fields exist?” | Compiler only | **Reflection** via UHT (`UPROPERTY`, `UFUNCTION`) — editor, BP, replication, serialization |
| Polymorphism in editor | Headers | `UCLASS` hierarchy visible in Content Browser / Details |

**Typical reasons something is a `UObject`:**

1. Must appear in the **editor** as an asset or component (`UDataAsset`, `UActorComponent`)
2. Must be usable from **Blueprint**
3. Must be **serialized** into a `.uasset` / map
4. Must **replicate** over the network the UE way
5. Must participate in **GC** with soft/hard refs

**Typical reasons something should NOT be a `UObject`:**

1. Hot deterministic sim core (this fighter sim)
2. Math / POD (`FVector`, your frame input)
3. You want value semantics: copy the whole state in one assignment

So: **`UFighterCombatComponent`** = UObject (hangs on actor, ticks, has UPROPERTY move set).  
**`FFighterCombatSim`** = plain class with an `F` name (rules + state).  
**`ULightAttackMoveSet`** = UObject DataAsset (you edit rows in the editor).

That split is intentional: editor-facing shell vs rewindable core.

---

### 4) Why fixed 60 Hz instead of variable `Tick`?

**Why:** Move data is in **frames**. Variable delta makes “8 startup frames” mean different durations and breaks cross-machine determinism.

**Where in code:**

| Idea | Location |
|------|----------|
| Tick rate property (default 60) | `Combat/FighterCombatComponent.h` → `SimTickRate` |
| Accumulate `DeltaTime` → N fixed steps | `Combat/FighterCombatComponent.cpp` → `RunFixedTicks` |
| Catch-up cap (anti spiral-of-death) | same function, `MaxCatchUpFrames` |
| One sim step = one frame of rules | `Combat/FighterCombatSim.cpp` → `TickFrame` (`++State.Frame`) |
| Move timings in frames | `Combat/LightAttackMoveSet.h` → `StartupFrames` / `ActiveFrames` / `RecoveryFrames` |
| Advance phase by local frame count | `Combat/FighterCombatSim.cpp` → `AdvanceCurrentMove` |

**Honest limit (also in code comments):** catch-up still starts from **render** `DeltaTime`. Real rollback will call `TickFrame` once per sim frame from the netcode layer, not from component catch-up.

---

### 5) Why posture lives in the sim (AnimBP only reads)?

**Why:** One authority for tilt + attack/block direction. Mesh must not invent combat truth.

**Where in code:**

| Idea | Location |
|------|----------|
| Posture fields in state | `CombatTypes.h` → `FFighterSimState::Posture` etc. |
| Posture rules | `FighterCombatSim.cpp` → `TickPosture` / `RequestPostureChange` |
| Character feeds direction only | `PlayerCharacter.cpp` → `ProcessPostureInput` → `SetPostureDirection` |
| Mirror for AnimBP | `PlayerCharacter.cpp` → `SyncPresentationFromCombatSim` → `ActualPosture = …` |
| Attack snapshot | `FighterCombatSim.cpp` → `StartLightAttack(State.Posture)` + `AttackSnapshotPosture` |

---

### 6) Why bitflags instead of ASC tags / character bools?

**Why:** Named like tags, packed in the restore blob, no ASC myths about replication.

**Where in code:**

| Idea | Location |
|------|----------|
| Flag definitions | `CombatTypes.h` → `namespace FighterStateFlags` |
| Set/clear on attack | `FighterCombatSim.cpp` → `StartLightAttack` / `EndCurrentMove` |
| Presentation sync | `PlayerCharacter.cpp` → `SyncPresentationFromCombatSim` (no ASC tag mirror) |

---

### 7) Why `ULightAttackMoveSet` (DataAsset) instead of 7 GameplayAbilities?

**Why:** One button → one sim action → data lookup by posture. Modular without GA graphs.

**Where in code:**

| Idea | Location |
|------|----------|
| Row = posture + frames + damage + soft montage | `Combat/LightAttackMoveSet.h` → `FLightAttackMoveDef` |
| Table asset | `ULightAttackMoveSet` |
| Lookup at attack start | `FighterCombatSim.cpp` → `StartLightAttack` → `MoveSet->FindMove` |
| Assign asset on component | `FighterCombatComponent.h` → `LightAttackMoveSet` |

#### FAQ — Your architecture idea (generic action + per-fighter resolution)

Yes — **that is exactly the intended shape**, and you are not fighting this PR:

```
Input: LightAttack
   → Sim: "start light attack" (shared rules for every fighter)
   → Parameter: current posture (snapshotted)
   → Resolution data: THIS fighter's move set row (frames, damage, montage)
```

Map to code:

- **Generic action** = `FFighterCombatSim::TickLightAttack` / `StartLightAttack` (same code for all cast members)
- **Parameter** = `EPosture` snapshot (`AttackSnapshotPosture`)
- **Unique resolution per fighter** = which `ULightAttackMoveSet` you assign on that character’s `FighterCombat` component (Manny vs future boss = different Data Assets, same sim)

Later you can add `UHeavyAttackMoveSet`, guard tables, etc., or one `UFighterMoveList` asset with sections. Same pattern: **shared verb, per-character noun data**.

That is how most multi-character fighters scale: engine code is shared; **frame data + anims are data**.

---

### 8) Why delete the dual-path flags in this PR?

**Why:** Dual paths were migration scaffolding. Keeping them after the sim is the agreed authority reintroduces the exact mess this restart is meant to clear (two clocks, two staling fields, “which path am I on?”).

**Where deleted:** `bUseFighterCombatSim` / `bRouteLightAttackToCombatSim`; character `ProcessPendingPostureChange` / posture staling fields; ASC `State.PostureChanging` presentation poke.

Tune posture delays / move set on `UFighterCombatComponent` only.

---

### 9) Why mirror `ActualPosture`?

**Why:** Zero AnimBP rewrite. Mirror is a cache, not authority.

**Where:** `SyncPresentationFromCombatSim`; AnimBP keeps reading `ActualPosture`.

---

### 10) Why no hitboxes in this PR?

**Why:** Phase machine first; Active frames are the intentional hook.

**Where:** `AdvanceCurrentMove` Active branch comment (“hitboxes would run here”).

---

## What you might reasonably disagree with

| Choice | Alternative | Code touchpoint |
|--------|-------------|-----------------|
| Sim on component per pawn | Match object owning two sims | `UFighterCombatComponent` |
| 60 Hz | 30/64/120 | `SimTickRate` |
| Tuning constants inside state | Config outside restore blob | `FFighterSimState` posture penalty fields |
| Soft montage on DataAsset | Presentation picks anim by move id only | `FLightAttackMoveDef::Montage` |
| Keep GAS modules for now | Rip ASC immediately | `PurgatoriumLex.Build.cs`, `AbilitySystem/` |
| Config tunables still inside state blob | Move delays/penalties to config-only | `ApplyPostureConfig` / `FFighterSimState` |

---

## Blueprint / editor steps (keep BP small)

1. Compile C++  
2. Create Data Asset class **`LightAttackMoveSet`** → fill postures + optional montages  
3. Assign on **FighterCombat → Light Attack Move Set**  
4. PIE: posture via `ActualPosture`; logs `START/ACTIVE/END`

No Event Graph required.

---

## Intentionally NOT in this PR

Hitboxes, block/parry, GGPO, full GAS removal.

---

## Rollback honesty

Scaffolding only: fixed step + copyable state + edge inputs + debug hash.  
Not rollback yet: still driven from render `DeltaTime`, CMC movement, no save/load net API.
