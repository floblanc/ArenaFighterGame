# PlayerCharacter decomposition

How to peel the god-class without creating a second mess.

**Rule:** `APlayerCharacter` is a **composition root** (socket strip). It feeds intent and reads presentation. It does not own combat clocks.

Authoritative combat notes: `Source/PurgatoriumLex/Combat/CombatSim_REVIEW.md`.

---

## Target shape

```
APlayerCharacter (thin pawn)
├── CameraBoom + FollowCamera     attachment / presentation hardware
├── ULockOnCameraComponent        lock-on + camera-on-back   ← Phase A (done)
├── UFighterCombatComponent       combat sim bridge          ← done
├── CMC                           UE locomotion
├── ASC / AttributeSet            starve; not combat authority
└── (later) more verbs → sim      guard / roll / tech…
```

---

## Cut along these axes

| Question | If yes | If no |
|----------|--------|-------|
| Must GGPO restore this value? | Sim state / sibling sim blob | Pawn or presentation component |
| Is it a frame delay / phase / window? | Near combat sim | Not on character Tick as authority |
| Is it camera / AnimBP / HUD? | Presentation component or mirror field | Never in restore blob |

---

## Phases

| Phase | Extract | Status |
|-------|---------|--------|
| **A** | Lock-on camera + move enums/structs out of giant header | **Done** (`ULockOnCameraComponent`, `PlayerCharacterTypes.h`) |
| **B** | Guard / charge / special → sim + DataAssets (same as light) | After light vertical slice |
| **C** | Roll + staling → sim when dodge is combat-relevant | After hit/block slice |
| **D** | Tech windows → sim when knockdown exists | After knockdown |
| **E** | Combat input → growing `FFighterFrameInput`; drop GAS buffer for combat tags | With netcode shape |

**Do not** extract roll/tech into components that still use wall-clock as authority — that only relocates the god.

---

## Phase A API (what the pawn still does)

- Owns boom + follow camera (attachment)
- Forwards LockUnlock input → `LockOnCamera->ToggleLockOnEnemy(!bIsRunning)`
- Listens: Fighting IMC add/remove; `RequestNeutralPosture` when unlocking from back
- Asks: `IsCameraLockedOnEnemy()` / `IsCameraLockedOnCharacterBack()` / `GetLockedOnActor()` for move/posture

Tune lock distance / height offset on the **LockOnCamera** component in the editor.

**Editor checklist:** `CursorAdvices/Blueprint_Modification_Checklist.md`

---

## Anti-patterns

- Parallel systems (fields on character **and** component for the same truth)
- “Manager” that still knows everything
- Camera / AnimNotify clocks inside the combat restore blob
- Splitting cpp by line count instead of by authority
