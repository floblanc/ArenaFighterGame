# Counter-Strike 2 Crouch Speed Penalty Mechanics

## Overview

Counter-Strike 2 implements a **static crouch speed penalty** that reduces movement speed when crouching. Unlike staling systems (like roll staling in Smash Bros), this is a **constant penalty** that doesn't accumulate or worsen with repeated use.

## Speed Ratios

Based on CS:GO mechanics (which carry over to CS2):

- **Running**: 100% (base speed)
- **Walking**: ~52% of running speed
- **Crouching**: ~34% of running speed

### Example with Knife (Fastest Weapon)

- **Running**: 260 units/second
- **Walking**: ~130 units/second (52% of running)
- **Crouching**: ~85 units/second (34% of running)

## Crouch Speed Penalty Details

**Penalty Magnitude:**
- Crouching is approximately **34% of running speed** (66% reduction)
- Crouching is approximately **65% of walking speed** (35% slower than walking)

**Key Characteristics:**
- **Static penalty**: Always ~34% speed when crouched (doesn't change)
- **No fatigue**: Doesn't get worse with repeated crouching
- **Instant application**: Speed changes immediately when crouching/uncrouching
- **No reset timer**: No cooldown needed to restore speed

## Gameplay Implications

### Advantages of Crouching

1. **Stealth**: Crouching is quieter than walking, useful for sneaking
2. **Accuracy**: Crouching improves accuracy and reduces recoil
3. **Lower profile**: Makes player hitbox smaller and harder to hit

### Disadvantages

1. **Mobility trade-off**: Much slower movement for better accuracy/stealth
2. **Vulnerability**: Slower movement makes you an easier target
3. **Positioning**: Harder to reposition quickly during combat

## Weapon-Specific Modifiers

- Each weapon has its own speed multiplier
- **Knife**: Fastest (260 u/s running)
- **Heavy weapons** (AWP, etc.): Slower base speed
- Crouch penalty applies as a **percentage** of the weapon's base speed

Example:
- Knife crouch: 85 u/s (34% of 260)
- AWP crouch: ~34% of AWP's base running speed (which is slower than knife)

## Differences from Staling Systems

| Aspect | CS2 Crouch Penalty | Staling Systems (e.g., Roll Staling) |
|--------|-------------------|-------------------------------------|
| **Type** | Static reduction | Accumulating penalty |
| **Behavior** | Always same speed when crouched | Gets worse with repeated use |
| **Reset** | Instant (when uncrouching) | Requires time without using |
| **Purpose** | Trade-off for accuracy/stealth | Prevent spam/exploitation |

## Strategic Usage

1. **Corner peeking**: Crouch to peek corners more safely
2. **Holding angles**: Crouch for better accuracy when holding positions
3. **Silent movement**: Crouch walk for stealthy rotations
4. **Recoil control**: Crouch during spray to reduce recoil
5. **Avoid spam**: Don't spam crouch - it's always slow, no benefit to rapid toggling

## Technical Notes

- Speed changes are **instantaneous** (no transition time)
- No accumulation or fatigue system
- Penalty is **multiplicative** with weapon speed modifiers
- Works independently of other movement mechanics (walking, running)

## References

- Based on CS:GO movement mechanics (carried over to CS2)
- Values may vary slightly between CS:GO and CS2, but fundamental ratios remain similar
- Community testing and documentation
