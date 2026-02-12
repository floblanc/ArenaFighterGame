# Input Buffering Rollback Compatibility Fix

## Problem

**Current implementation stores `FramesRemaining` which is decremented each Tick:**
```cpp
struct FBufferedAbilityInput {
    FGameplayTag InputTag;
    int32 FramesRemaining = 0;  // ❌ Problem: This gets decremented, can't rollback correctly
};
```

**Why this breaks rollback:**
- When rollback happens, Tick may have run a different number of times
- `FramesRemaining` becomes stale/wrong after rollback
- Buffer state doesn't match the actual frame count

---

## Solution: Store Frame Number Instead

**Store the frame when input was buffered, calculate remaining on demand:**

```cpp
struct FBufferedAbilityInput {
    FGameplayTag InputTag;
    int32 BufferedFrame = 0;  // ✅ Frame number when buffered (part of rollback state)
    
    // Calculate remaining frames on demand (deterministic)
    int32 GetFramesRemaining(int32 CurrentFrame, int32 BufferFrames) const {
        return FMath::Max(0, BufferFrames - (CurrentFrame - BufferedFrame));
    }
};
```

**Why this works:**
- Frame numbers are part of rollback state (everyone agrees on frame N)
- Calculation is deterministic: `CurrentFrame - BufferedFrame` is always correct
- Rollback can recalculate `FramesRemaining` correctly from saved frame numbers

---

## Implementation: Client-Side Only (Option A - Selected)

**✅ Selected approach: Client-side only for feel improvement**

- **Buffer is local-only** (not replicated)
- Server/rollback only sees **successful activations** (already handled by GAS)
- Failed activations are purely client-side feel improvements
- Frame number storage still used for robustness (handles Tick inconsistencies)

**Why this works:**
- Rollback replays **inputs** (not buffer state)
- When input is replayed, buffer logic runs again deterministically
- Same input → same gate check → same buffer decision → same result
- Server doesn't need to know about failed activation attempts

### Alternative: Rollback-Compatible Buffer (Option B - Not Used)

**If buffer state affects gameplay (e.g. server needs to know about buffered inputs):**

1. **Store frame number instead of remaining:**
```cpp
struct FBufferedAbilityInput {
    FGameplayTag InputTag;
    int32 BufferedFrame = 0;  // Frame when buffered
    
    FBufferedAbilityInput(const FGameplayTag& Tag, int32 CurrentFrame) 
        : InputTag(Tag), BufferedFrame(CurrentFrame) {}
};
```

2. **Get current frame from rollback system:**
```cpp
int32 GetCurrentFrame() const {
    // Get from your rollback system (e.g. GGPO frame counter)
    // Or use GetWorld()->GetGameState()->GetServerWorldTimeSeconds() converted to frames
    return RollbackFrameCounter;  // Example - implement based on your rollback system
}
```

3. **Calculate remaining on demand:**
```cpp
void APlayerCharacter::Tick(float DeltaTime) {
    const int32 CurrentFrame = GetCurrentFrame();
    
    // Remove expired entries
    AbilityInputBuffer.RemoveAll([CurrentFrame, this](const FBufferedAbilityInput& E) {
        return E.GetFramesRemaining(CurrentFrame, AbilityInputBufferFrames) <= 0;
    });
    
    // Try consume buffer
    for (int32 i = AbilityInputBuffer.Num() - 1; i >= 0; --i) {
        const FBufferedAbilityInput& Entry = AbilityInputBuffer[i];
        if (CanActivateAbilityForInputTag(Entry.InputTag) && 
            ASC->TryActivateAbilitiesByInputTag(Entry.InputTag)) {
            AbilityInputBuffer.RemoveAt(i);
        }
    }
}
```

4. **Buffer with frame number:**
```cpp
void APlayerCharacter::BufferAbilityInput(FGameplayTag InputTag) {
    if (!IsInputTagBufferable(InputTag) || AbilityInputBufferFrames <= 0) return;
    
    const int32 CurrentFrame = GetCurrentFrame();
    AbilityInputBuffer.Add(FBufferedAbilityInput(InputTag, CurrentFrame));
}
```

---

## Current Implementation: Client-Side Only

**✅ Implemented: Client-side only for feel improvement**

### Why Client-Side Works:

1. **Server doesn't care about failed activations**
   - Server only processes successful ability activations
   - Failed attempts (gate blocked, on cooldown) are local feel improvements

2. **GAS handles replication**
   - When buffered input successfully activates, GAS replicates the ability activation
   - Server sees the result, not the buffer attempt

3. **Simpler implementation**
   - No need to track frame numbers
   - No need to replicate buffer state
   - Current implementation works as-is

4. **Rollback still works**
   - Rollback replays **inputs** (not buffer state)
   - When input is replayed, buffer logic runs again deterministically
   - Same input → same gate check → same buffer decision → same result

### When You'd Need Option B:

- **Server needs to predict buffered inputs** (rare)
- **Buffer affects gameplay state** (e.g. "queued attack" visible to other players)
- **Rollback system requires explicit buffer state** (depends on your rollback implementation)

---

## Current Implementation Assessment

**Your current implementation is rollback-compatible IF:**

✅ **Buffer is client-side only** (not replicated)
✅ **Buffer consumption uses rollback state** (`CanActivateAbilityForInputTag`, ASC state)
✅ **Frame-based** (not time-based)
✅ **Deterministic operations** (no timers, no time queries)

**Potential issue:**
- `FramesRemaining` decremented in Tick could drift if Tick runs inconsistently
- **Fix:** Use frame number storage (Option B) OR ensure buffer is client-side only (Option A)

---

## Implementation Status

**✅ Implemented: Client-side only with frame number storage**

**Current implementation:**
- Uses `BufferedFrame` (frame number storage) for robustness
- Not replicated (client-side only)
- Documented in code comments
- Works with rollback (replays inputs deterministically)

**Code documentation:**
```cpp
/** Pending ability inputs to try activating each Tick until they expire or succeed.
 *  CLIENT-SIDE ONLY: This is purely for feel improvement (responsive input when gates block activation).
 *  Not replicated - server only sees successful activations (handled by GAS replication).
 *  Rollback compatibility: When inputs are replayed, buffer is recreated deterministically. */
UPROPERTY(BlueprintReadOnly, Category = "Input Buffer")
TArray<FBufferedAbilityInput> AbilityInputBuffer;
```

---

## Summary

| Aspect | Implementation | Status |
|--------|----------------|--------|
| **Storage** | `BufferedFrame` (frame number) | ✅ Implemented |
| **Replication** | Not replicated (client-side only) | ✅ Implemented |
| **Rollback** | Works (replays inputs deterministically) | ✅ Compatible |
| **Complexity** | Simple (frame number storage) | ✅ Good |
| **Use case** | Feel improvement | ✅ Client-side only |

**Result:** Client-side input buffering for feel improvement. Rollback-compatible because inputs are replayed deterministically, recreating buffer state correctly.
