# Tech Implementation Guide

## What is Tech?

**Tech** (short for "technical recovery" or "tech roll") is a defensive mechanic in fighting games that allows players to quickly recover from knockdowns or hits by pressing a button at the right time. It prevents opponents from getting guaranteed follow-ups after knockdowns.

## Common Types of Tech

### 1. **Ground Tech** (Tech Roll)
- Player is knocked down
- Press button during a **tech window** (usually right before hitting ground or immediately after)
- Character quickly rolls/recovers instead of lying down
- Prevents ground oki (okizeme) setups

### 2. **Air Tech** (Air Recovery)
- Player is hit while airborne
- Press button during hitstun/knockback
- Character recovers mid-air instead of falling
- Prevents air combos/guaranteed follow-ups

### 3. **Wall Tech**
- Player hits a wall during knockback
- Press button during wall impact
- Character bounces off wall and recovers instead of wall splatting
- Prevents wall combos

## Implementation Approach

### Core Mechanics

1. **Tech Window**: Time window where tech input is valid
   - Usually **before** hitting ground/wall (predictive)
   - Or **immediately after** impact (reactive)
   - Frame-based for rollback compatibility

2. **Tech Input**: Button press (often any attack button or specific button)
   - Can be directional (tech left/right/neutral)
   - Can be buffered (input buffering system helps here)

3. **Tech State**: 
   - Add `State.Tech` tag when tech is successful
   - Remove when tech animation completes
   - Use for ability gating (can't attack during tech recovery)

### Implementation Steps

#### Step 1: Detect Techable Situations

```cpp
// When character is about to hit ground/wall or is in hitstun
bool bCanTech = false;

// Ground tech: check if falling and about to hit ground
if (IsFalling() && GetCharacterMovement()->IsFalling() && 
    WillHitGroundSoon() && HasStateTag(State_KnockedDown))
{
    bCanTech = true;
}

// Air tech: check if in hitstun/knockback
if (HasStateTag(State_Hitstun) && IsAirborne())
{
    bCanTech = true;
}

// Wall tech: check if about to hit wall
if (WillHitWallSoon() && HasStateTag(State_Airborne))
{
    bCanTech = true;
}
```

#### Step 2: Tech Window Management

```cpp
// Frame-based tech window (rollback compatible)
int32 TechWindowStartFrame = -1;
int32 TechWindowDurationFrames = 10; // ~10 frames window

// Start tech window when techable situation detected
void StartTechWindow()
{
    TechWindowStartFrame = SimulationFrame;
    // Could also add a visual/audio cue here
}

// Check if currently in tech window
bool IsInTechWindow() const
{
    if (TechWindowStartFrame < 0) return false;
    int32 FramesElapsed = SimulationFrame - TechWindowStartFrame;
    return FramesElapsed < TechWindowDurationFrames;
}
```

#### Step 3: Tech Input Detection

```cpp
// In input handler (e.g., when attack button pressed)
void OnTechInput()
{
    if (IsInTechWindow())
    {
        PerformTech();
    }
    else
    {
        // Buffer the input (your existing input buffering system)
        BufferAbilityInput(InputTag_Tech); // If you add this input tag
    }
}
```

#### Step 4: Perform Tech

```cpp
void PerformTech()
{
    using namespace PurgatoriumLexGameplayTags;
    
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;
    
    // Add Tech state tag
    ASC->AddLooseGameplayTag(State_Tech);
    
    // Remove knockdown/airborne states
    ASC->RemoveLooseGameplayTag(State_KnockedDown);
    ASC->RemoveLooseGameplayTag(State_Airborne);
    
    // Play tech animation/montage
    // PlayTechMontage();
    
    // Cancel tech window
    TechWindowStartFrame = -1;
    
    // Remove Tech tag after animation completes (via montage notify or timer)
    // This should be frame-based for rollback compatibility
}
```

#### Step 5: Tech Recovery Completion

```cpp
// Called when tech animation completes (montage notify or frame-based timer)
void OnTechRecoveryComplete()
{
    using namespace PurgatoriumLexGameplayTags;
    
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (ASC)
    {
        ASC->RemoveLooseGameplayTag(State_Tech);
    }
    
    // Character can now act normally
}
```

## Integration with Existing Systems

### Input Buffering
Your existing input buffering system (`AbilityInputBuffer`) can handle tech inputs:
- If tech window isn't active yet, buffer the input
- When tech window starts, check buffer and consume if valid
- This makes tech feel responsive even with tight windows

### Ability Gating
Use `State.Tech` in ability activation rules:
```cpp
// Abilities blocked during tech recovery
Ability->ActivationBlockedTags.AddTag(State_Tech);
```

### Rollback Compatibility
- Use **frame-based** tech windows (`SimulationFrame`)
- Store tech window start frame, not time
- Tech state changes are deterministic based on frame numbers

## Example Flow

1. **Character gets hit** → enters hitstun/knockback
2. **Tech window starts** → `StartTechWindow()` called
3. **Player presses tech button** → `OnTechInput()` called
4. **If in window** → `PerformTech()` executes
   - Add `State.Tech` tag
   - Play tech animation
   - Cancel tech window
5. **Tech animation completes** → `OnTechRecoveryComplete()` called
   - Remove `State.Tech` tag
   - Character can act normally

## Design Considerations

### Tech Window Timing
- **Too early**: Players tech before they need to (feels bad)
- **Too late**: Players can't react in time (feels unfair)
- **Common**: 5-15 frames window, often starts slightly before impact

### Tech Direction
- **Neutral tech**: Stay in place
- **Forward tech**: Roll forward
- **Backward tech**: Roll backward
- Use movement input direction to determine tech direction

### Tech Limitations
- **Tech cooldown**: Prevent tech spam (can't tech immediately after teching)
- **Techable vs Non-Techable**: Some knockdowns are untechable (hard knockdowns)
- **Tech on wake-up**: Some games allow tech on wake-up from knockdown

## References

- Common in fighting games: Street Fighter, Tekken, Guilty Gear, etc.
- Frame-based timing for rollback netcode compatibility
- Often integrated with input buffering for responsive feel
