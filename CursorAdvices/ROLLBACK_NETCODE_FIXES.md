# Rollback Netcode Compatibility Fixes

This document explains all the changes made to prepare the codebase for rollback netcode implementation.

## Overview

Rollback netcode requires **deterministic** code execution - the same inputs must produce the same results on all machines. This document details all fixes applied to ensure compatibility.

---

## Critical Fixes Applied

### 1. ✅ Removed Non-Deterministic `FDateTime::Now()` Calls

**Problem:** `FDateTime::Now()` returns system time, which is different on each machine and changes every frame. This breaks determinism.

**Location:** `PlayerCharacter.cpp` lines 158, 299

**Fix:** Removed all `FDateTime::Now()` calls from gameplay code.

**Why:** System time is non-deterministic and will cause desyncs during rollback.

---

### 2. ✅ Moved Camera Lock-On Logic from Tick() to Movement Update

**Problem:** Camera lock-on was running in `Tick()`, which:
- Runs at variable frame rates
- Not tied to movement/physics updates
- Can cause timing differences between clients

**Location:** 
- **Before:** `PlayerCharacter::Tick()` 
- **After:** `PlayerCharacter::UpdateCameraLockOn()` called from `CharacterMovementComponent::OnMovementUpdated`

**Fix Applied:**
```cpp
// In constructor - bind to movement update delegate
GetCharacterMovement()->OnMovementUpdated.AddUObject(this, &APlayerCharacter::UpdateCameraLockOn);

// New function - called deterministically with movement
void APlayerCharacter::UpdateCameraLockOn()
{
    // Camera lock-on logic here
}
```

**Why This Location:**
1. **Deterministic Timing:** `OnMovementUpdated` fires at the same physics step on all clients
2. **Movement Synchronized:** Camera updates happen with movement, ensuring consistency
3. **Rollback Compatible:** Movement is already part of the rollback system, so camera follows naturally
4. **Performance:** Only runs when movement actually updates, not every frame

**Alternative Locations Considered:**
- ❌ **Tick()** - Variable frame rate, not deterministic
- ❌ **PlayerController::UpdateRotation()** - Not tied to movement
- ✅ **OnMovementUpdated** - Perfect! Tied to physics, deterministic

---

### 3. ✅ Fixed Static Array in ProcessAbilityInput

**Problem:** Using `static TArray` can persist state across rollbacks, causing incorrect behavior.

**Location:** `PurgatoriumLexAbilitySystemComponent::ProcessAbilityInput()` line 65

**Before:**
```cpp
static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
```

**After:**
```cpp
TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;  // Local, not static
```

**Why:** Static variables persist across function calls and rollbacks, causing state corruption. Local variables are recreated each call, ensuring clean state.

---

### 4. ✅ Moved Ability Input Processing from Tick to Input Events

**Problem:** Processing input in `TickComponent()` means:
- Input timing depends on frame rate
- Can miss or duplicate inputs
- Not frame-accurate for rollback

**Location:**
- **Before:** `PurgatoriumLexAbilitySystemComponent::TickComponent()`
- **After:** `PlayerCharacter::Input_AbilityInputTagPressed/Released()`

**Fix Applied:**
```cpp
// In input handlers - process immediately
void APlayerCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (UPurgatoriumLexAbilitySystemComponent* ASC = ...)
    {
        ASC->AbilityInputTagPressed(InputTag);
        ASC->ProcessAbilityInput(0.0f, false);  // Process immediately!
    }
}
```

**Why:** 
- Input events fire at exact frame boundaries
- Processing immediately ensures frame-accurate timing
- Rollback can precisely track when abilities activated

**Note:** `TickComponent()` still has a fallback for backward compatibility, but it only processes if there are pending inputs.

---

### 5. ✅ Removed Excessive Debug Logging

**Problem:** Excessive `UE_LOG` calls in gameplay code:
- Performance impact
- Can cause timing differences
- Not needed for production

**Location:** Multiple locations in `PlayerCharacter.cpp`

**Fix:** Removed debug logs that used `FDateTime::Now()` and excessive movement logging.

**Why:** Logging can affect performance and timing, which can cause desyncs.

---

## Summary of Changes

| Issue | Location | Fix | Impact |
|-------|----------|-----|--------|
| **Non-deterministic time** | `Tick()` | Removed `FDateTime::Now()` | ✅ Critical |
| **Camera in Tick** | `Tick()` | Moved to `OnMovementUpdated` | ✅ Critical |
| **Static arrays** | `ProcessAbilityInput()` | Changed to local | ✅ Critical |
| **Input in Tick** | `TickComponent()` | Moved to input handlers | ✅ Critical |
| **Excessive logging** | Multiple | Removed debug logs | ⚠️ Moderate |

---

## Camera Lock-On Location Explanation

### Why `OnMovementUpdated` is the Best Choice

**`CharacterMovementComponent::OnMovementUpdated`** is a delegate that fires:
- After movement physics calculations complete
- At the same physics step on all clients
- Synchronized with the rollback system
- Only when movement actually changes

**Benefits:**
1. ✅ **Deterministic:** Runs at same physics step on all clients
2. ✅ **Synchronized:** Camera updates with movement, ensuring consistency
3. ✅ **Rollback Compatible:** Movement is part of rollback, camera follows naturally
4. ✅ **Performance:** Only runs when needed, not every frame
5. ✅ **Frame-Accurate:** Tied to physics, not render frame rate

**How It Works:**
```
Movement Update → OnMovementUpdated Delegate → UpdateCameraLockOn() → Camera Rotation
     ↓
Rollback System (saves/restores movement state)
     ↓
Camera state is automatically included in rollback!
```

---

## Remaining Considerations for Full Rollback Implementation

These fixes prepare the code, but you'll also need:

1. **Deterministic Random Numbers:** Use seeded RNG if you add randomness
2. **Character Movement Settings:** Configure `UCharacterMovementComponent` for rollback
3. **Ability System State:** Ensure GAS state is properly saved/restored
4. **Input Buffering:** Implement input buffering system
5. **State Serialization:** Implement save/restore for game state

---

## Testing Rollback Compatibility

To verify these fixes work with rollback:

1. **Determinism Test:** Run same inputs on multiple clients, verify same results
2. **Rollback Test:** Simulate network delay, verify state restores correctly
3. **Camera Test:** Verify camera lock-on works correctly during rollback
4. **Ability Test:** Verify abilities activate at correct frames during rollback

---

## Notes

- All changes maintain backward compatibility
- Camera lock-on behavior is unchanged, just moved to better location
- Ability system still works the same, just processes input more accurately
- No gameplay changes, only architectural improvements for rollback

