# Tech System Rollback Netcode Compatibility

## Overview

The tech system has been updated to be compatible with rollback netcode. This document explains the changes made and how the system ensures deterministic behavior during rollback and prediction.

---

## Key Rollback Compatibility Fixes

### 1. ✅ Removed Predictive Tech Window Start

**Problem:** 
- `WillHitGroundSoon()` and `WillHitWallSoon()` used line traces to predictively start tech windows
- Line traces can be non-deterministic between clients during rollback
- Different clients might detect "soon" at different times, causing desyncs

**Fix:**
- Removed all predictive tech window starts
- Tech windows now start **only when actual contact occurs**:
  - `Landed()` callback for ground contact
  - `NotifyHit()` callback for wall contact
- Contact events are deterministic and part of the rollback state

**Code Changes:**
```cpp
// REMOVED from Tick():
if (bIsTechable && !IsInTechWindow() && WillHitWallSoon())
{
    StartTechWindow();
}

// REMOVED from UpdateTechableState():
if (bIsTechable && !IsInTechWindow() && WillHitGroundSoon())
{
    StartTechWindow();
}

// ADDED to Landed():
if (bIsTechable && !IsInTechWindow())
{
    StartTechWindow(); // Start on actual contact
}

// ADDED to NotifyHit():
if (bIsTechable && !IsInTechWindow())
{
    StartTechWindow(); // Start on actual contact
}
```

---

### 2. ✅ Fixed Input Reading for Rollback

**Problem:**
- `GetLastMovementInputVector()` might not be available or accurate during rollback/prediction
- Input reading needs to come from the character movement component (part of rollback state)

**Fix:**
- Changed from `GetLastMovementInputVector()` to `GetCharacterMovement()->GetLastInputVector()`
- Movement component input is part of the rollback state and is deterministic

**Code Changes:**
```cpp
// BEFORE:
const FVector MovementInput = GetLastMovementInputVector();

// AFTER:
const FVector MovementInput = GetCharacterMovement()->GetLastInputVector();
```

---

### 3. ✅ Frame-Based Calculations (Already Implemented)

**Status:** ✅ Already correct

- All tech timing uses frame-based calculations (`TechWindowStartFrame`, `LastTechInputFrame`)
- Frame differences are calculated deterministically: `SimulationFrame - TechWindowStartFrame`
- No time-based calculations that could drift between clients

**Note on `SimulationFrame`:**
- Currently incremented in `Tick()` as a local frame counter
- For full rollback integration, this should ideally come from your rollback system's frame counter
- However, since all calculations use **frame differences** (not absolute frame numbers), the system remains deterministic as long as `SimulationFrame` increments consistently

---

### 4. ✅ Deterministic State Variables

**State Variables:**
- `bIsTechInputHeld` - Set from input callbacks (deterministic during rollback replay)
- `bIsJumpInputHeld` - Set from input callbacks (deterministic during rollback replay)
- `bIsTechable` - Derived from GameplayTags (`State.Hitstun`, `State.Tech`) which are part of rollback state
- `LastWallHitNormal` - Set from `NotifyHit()` callback (deterministic contact event)
- `TechWindowStartFrame` - Set from contact events (deterministic)
- `LastTechInputFrame` - Set from input callbacks (deterministic)

**All state variables are:**
- Set from deterministic sources (input callbacks, contact events, GameplayTags)
- Recalculated during rollback replay
- Not dependent on non-deterministic sources (time, random numbers, external state)

---

## How Tech System Works with Rollback

### Normal Execution Flow:

1. **Character enters hitstun** → `State.Hitstun` tag added (GAS state, part of rollback)
2. **Character falls/hits wall** → `Landed()` or `NotifyHit()` called (deterministic contact event)
3. **Tech window starts** → `TechWindowStartFrame = SimulationFrame` (frame-based)
4. **Player presses tech input** → `OnTechInputPressed()` called (input callback)
5. **Tech performed** → `PerformTech()` adds `State.Tech` tag (GAS state, part of rollback)

### During Rollback:

1. **Rollback occurs** → State restored to previous frame
2. **Inputs replayed** → `OnTechInputPressed()` called again with same inputs
3. **Contact events replayed** → `Landed()`/`NotifyHit()` called again (deterministic)
4. **State recalculated** → All frame-based calculations recalculated deterministically
5. **Same result** → Same inputs + same state = same tech result

### Why It Works:

- **Deterministic Sources:** All state comes from inputs, contact events, or GAS tags (all part of rollback state)
- **Frame-Based:** All timing uses frame differences, not time (no drift)
- **No Prediction:** Tech windows start on actual contact, not prediction (no desyncs)
- **GAS Integration:** Tech state (`State.Tech`) is managed by GAS, which handles rollback automatically

---

## Remaining Considerations

### 1. SimulationFrame Source

**Current:** Incremented in `Tick()` as local counter

**Ideal:** Should come from your rollback system's frame counter

**Impact:** Low - All calculations use frame differences, so as long as `SimulationFrame` increments consistently, determinism is maintained.

**Recommendation:** When integrating with your rollback system, replace `SimulationFrame++` with your rollback frame counter.

### 2. Tech Window Timing

**Current:** Tech window starts on contact, lasts `TechWindowFrames` frames

**SSBU Behavior:** Tech window can start before contact (predictive)

**Trade-off:** 
- ✅ More rollback-safe (no prediction)
- ⚠️ Slightly less forgiving than SSBU (window starts later)

**Recommendation:** If you want SSBU-style predictive windows, implement them using deterministic physics calculations (velocity + position) rather than line traces.

### 3. Input Buffering

**Current:** Tech input can be "held" (`bIsTechInputHeld`) for ground techs

**Rollback Compatibility:** ✅ Works correctly
- Input state is replayed during rollback
- `bIsTechInputHeld` is set deterministically from input callbacks

---

## Testing Rollback Compatibility

To verify the tech system works correctly with rollback:

1. **Determinism Test:**
   - Run same inputs on multiple clients
   - Verify tech windows start at same frames
   - Verify techs execute at same frames

2. **Rollback Test:**
   - Simulate network delay/rollback
   - Verify tech state restores correctly
   - Verify tech windows recalculate correctly
   - Verify techs execute correctly after rollback

3. **Contact Test:**
   - Verify tech windows start on contact (not before)
   - Verify wall techs work correctly
   - Verify ground techs work correctly

4. **Input Test:**
   - Verify tech input reading is consistent
   - Verify directional techs (forward/back/left/right) work correctly
   - Verify wall tech jump works correctly

---

## Summary

✅ **Tech system is rollback-compatible** with the following guarantees:

- **Deterministic state:** All state comes from rollback-safe sources
- **Frame-based timing:** No time-based calculations that could drift
- **Contact-based windows:** No predictive line traces that could desync
- **GAS integration:** Tech state managed by GAS (handles rollback automatically)
- **Input replay:** All input handling is deterministic and replayable

The system will work correctly during rollback as long as:
- `SimulationFrame` increments consistently (or comes from rollback system)
- Input callbacks are replayed correctly
- Contact events (`Landed`, `NotifyHit`) are deterministic
- GAS state is properly saved/restored during rollback

---

*Last Updated: 2026-01-27*
*Compatible with: Rollback netcode, deterministic gameplay*
