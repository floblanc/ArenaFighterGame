# Roll Staling Mechanics (Super Smash Bros. Ultimate)

## Overview

In **Super Smash Bros. Ultimate**, rolls (and all dodges) have a **staling system** that penalizes repeated use. Unlike the Stale Moves system for attacks (which affects damage/knockback), **dodge staling** affects:
- **Duration** (how long the roll lasts)
- **Intangibility frames** (when invincibility starts)

This system is shared across **all dodge types**: rolls, spot dodges, and air dodges. Overusing any type of dodge affects all others.

## How It Works

### Penalty Formula

The duration of a roll is calculated as:
```
Duration = BaseDuration × (1 + Penalty)
```

Where:
- **BaseDuration** = The normal duration when fresh
- **Penalty** = Accumulated penalty value (starts at 0)

### Penalty Accumulation

**Forward Rolls:**
- Each forward roll adds **+0.06** to the penalty
- Maximum penalty: **0.3** (reached after **5 consecutive forward rolls**)

**Back Rolls:**
- Each back roll adds **+0.1** to the penalty
- Maximum penalty: **0.5** (reached after **5 consecutive back rolls**)

**Important:** Back rolls stale **faster and more severely** than forward rolls.

### Effects When Staled

1. **Increased Duration:**
   - Roll lasts longer (more lag/endlag)
   - Example: A 26-frame forward roll becomes 34 frames when fully stale (26 × 1.3 = 33.8)

2. **Delayed Intangibility:**
   - When **fully stale**, intangibility starts **4 frames later**
   - Example: If fresh intangibility is frames 4-12, fully stale would be frames 8-12
   - This makes rolls much worse for escaping pressure

3. **Shared Penalty:**
   - Using a roll increases penalty for **all dodges** (rolls, spot dodges, air dodges)
   - Using a spot dodge or air dodge also increases the penalty for rolls

### Reset Condition

The penalty resets when:
- Player spends **roughly 1 second** without using **any type of dodge** (roll, spot dodge, or air dodge)
- The reset applies to **all dodges** simultaneously

## Examples

### Forward Roll Staling

**Fresh Forward Roll:**
- Duration: 26 frames
- Intangibility: Frames 4-12
- Penalty: 0.0

**After 1 Forward Roll:**
- Duration: 26 × 1.06 = **27.56 frames** (~28 frames)
- Intangibility: Still frames 4-12 (not fully stale yet)
- Penalty: 0.06

**After 5 Forward Rolls (Fully Stale):**
- Duration: 26 × 1.3 = **33.8 frames** (~34 frames)
- Intangibility: **Frames 8-12** (4 frames later)
- Penalty: 0.3 (capped)

### Back Roll Staling

**Fresh Back Roll:**
- Duration: 32 frames
- Intangibility: Frames 4-14
- Penalty: 0.0

**After 1 Back Roll:**
- Duration: 32 × 1.1 = **35.2 frames** (~35 frames)
- Intangibility: Still frames 4-14
- Penalty: 0.1

**After 5 Back Rolls (Fully Stale):**
- Duration: 32 × 1.5 = **48 frames**
- Intangibility: **Frames 8-14** (4 frames later)
- Penalty: 0.5 (capped)

## Key Differences from Attack Staling

| Aspect | Attack Staling | Dodge Staling |
|--------|----------------|---------------|
| **What it affects** | Damage & Knockback | Duration & Intangibility |
| **Queue system** | 9-slot queue | Simple counter (0-5 uses) |
| **Reset condition** | Being KO'd, ~10 different moves | ~1 second without dodging |
| **Shared system** | No (attacks only) | Yes (all dodges share penalty) |
| **Maximum effect** | ~53% damage reduction | 30-50% duration increase + 4 frame delay |

## Strategic Implications

1. **Roll Spamming is Punished:**
   - After 5 rolls, you're much more vulnerable
   - Intangibility starts later, making escape harder
   - Longer endlag makes punishment easier

2. **Back Rolls Stale Faster:**
   - Back rolls become worse more quickly
   - Forward rolls are slightly safer to spam (but still bad)

3. **All Dodges Share Penalty:**
   - Can't "reset" by switching to spot dodge or air dodge
   - Must wait ~1 second without any dodge to reset

4. **Mix-Up Required:**
   - Players must use movement, attacks, or shields instead of constant dodging
   - Encourages diverse defensive options

## Reference

- [SSBWiki - Roll](https://www.ssbwiki.com/Roll)
- Based on Super Smash Bros. Ultimate mechanics
