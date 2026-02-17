# Gameplay Tags as State System (GAS / UE5)

## Goal

Use **GameplayTags as the authoritative state machine** for your character so that:

- **All states are tags**
- **No duplicated booleans** for the same concept (avoid `bIsAttacking`, `bIsRolling`, etc.)
- Abilities are gated purely via **Required / Blocked** tag rules
- State changes are applied via `ASC->AddLooseGameplayTag()` / `ASC->RemoveLooseGameplayTag()`

This keeps your rules consistent across C++, Blueprint, and GAS ability activation logic.

## 1) Define core tags (central list)

In this project, tags are declared in:

- `Config/DefaultGameplayTags.ini` (editor-visible list)
- `Source/PurgatoriumLex/PurgatoriumLexGameplayTags.h/.cpp` (native tags)

### 1.1 State tags

Recommended baseline (added):

- `State.Grounded`
- `State.Airborne`
- `State.Attacking`
- `State.Hitstun`
- `State.Blocking`
- `State.Rolling`

### 1.2 Posture (recommended for your project)

- **Use `EPosture` enum** for posture direction (animation-facing state).
- Use a single **blocking/busy tag** while posture is changing:
  - `State.PostureChanging`

## 2) Apply states via loose tags (ASC)

States should live on the **Ability System Component** so they:

- Replicate (when relevant)
- Participate automatically in ability activation gating
- Are queryable from anywhere using tag checks

### 2.1 Setting state

Example pattern (C++):

```cpp
// pseudo-code
ASC->AddLooseGameplayTag(PurgatoriumLexGameplayTags::State_Attacking);
ASC->RemoveLooseGameplayTag(PurgatoriumLexGameplayTags::State_Attacking);
```

### 2.2 Querying state

Use tag queries instead of booleans:

```cpp
ASC->HasMatchingGameplayTag(PurgatoriumLexGameplayTags::State_Hitstun);
```

## 3) State rules via tags (no more if-chains)

In GAS abilities, configure:

- **ActivationRequiredTags**
- **ActivationBlockedTags**

### Example: “Only attack while grounded”

```cpp
Ability->ActivationRequiredTags.AddTag(PurgatoriumLexGameplayTags::State_Grounded);
```

### Example: “Cannot attack while hitstunned”

```cpp
Ability->ActivationBlockedTags.AddTag(PurgatoriumLexGameplayTags::State_Hitstun);
```

Result:

- ✅ You can’t attack while hitstunned
- ✅ You don’t need `if (bIsAttacking)` checks scattered around

## 4) Blocking state: posture transitions

If “changing posture” should block other actions, treat it as a **state tag**:

- Add `State.PostureChanging` when a posture change is queued/transitioning
- Remove it when the posture change is applied

## 5) Migration guideline (practical)

### 5.1 Choose one source of truth

If you keep a boolean like `bIsGuarding`, do **not** also keep `State.Blocking`.
Pick tags as the truth.

### 5.2 Replace booleans with tag writes

- When you previously set `bIsCharging = true`, replace with:
  - `State.Attacking` / `State.Blocking` / etc. as appropriate

### 5.3 Replace boolean reads with tag checks

- `if (bIsCharging)` → `if (ASC->HasMatchingGameplayTag(State_Attacking))`

## 6) Notes for rollback-friendly gameplay

Tags themselves are deterministic **if you set/remove them deterministically** (frame-based logic, animation events, ability events).

Avoid driving critical tag changes from non-deterministic sources (system time, timers with variable step, etc.).

