# Stale Moves Mechanics Summary (Super Smash Bros.)

## Overview

**Stale Moves** (also called "Stale-Move Negation" or "Repetition Effect") is a mechanic that reduces the damage and knockback of moves when they are used repeatedly. It encourages players to use their full moveset instead of spamming the same attack.

## How It Works

### Queue System (Melee onward)
- **9-slot queue**: Tracks the last 9 moves that successfully connected
- When a move connects, it's added to the queue
- If a move already exists in the queue and connects again, its damage/knockback is reduced based on:
  - **How recently it was used** (more recent = more penalty)
  - **How many times it appears in the queue** (more appearances = more penalty)
- Reductions from multiple queue positions **stack additively**
- When a 10th move connects, it replaces the oldest entry in the queue

### Freshness Bonus (Brawl/SSB4/Ultimate)
- Moves **not** in the queue gain a **1.05x damage bonus**
- This means attacks rarely deal exactly base damage—they're either fresh (+5%) or stale (reduced)

### Maximum Staleness
- If a move fills all 9 slots (used 9 times in a row), maximum reduction varies by game:
  - **Melee**: ~45% reduction (55% of original damage)
  - **Brawl**: ~54% reduction (46% of original damage)
  - **SSB4**: ~47% reduction (53% of original damage)
  - **Ultimate**: ~53% reduction (47% of original damage)

## What Gets Staled

### ✅ Moves That Enter the Queue
- **Attacks that connect** with opponents or damageable objects
- **Whole attack stales**, not individual hitboxes (weak hitbox stales the whole move)
- **Variations of the same move** share staleness:
  - Charged vs. uncharged versions (e.g., Giant Punch)
  - Ground vs. air versions
  - Different hitboxes of multi-hit moves
- **Projectiles** stale when they connect
- **Items** (in Brawl): Batter items stale, thrown items don't (SSB4+)
- **Shield hits** (Ultimate only): Moves stale when hitting shields, but with 0.85× reduction factor

### ❌ Moves That Do NOT Stale
- **Moves that miss** (don't connect)
- **Shield hits** (Melee/Brawl/SSB4): Hitting shields doesn't stale
- **Invincible opponents**: Hitting invincible frames doesn't stale
- **Counters/Absorbers**: Counter-attacks don't consider incoming attack's staleness
- **Reflected projectiles**: Don't stale the reflector move
- **Grab aerials** (prior to SSB4): Clawshot, Hookshot, Grapple Beam, Rope Snake
- **Grab releases**: Donkey Kong's cargo throw (Melee/Brawl)
- **Taunts**: Most taunts don't stale (except Kazuya's side taunt in Ultimate)
- **Landing hitboxes** (Brawl only)
- **Recoil damage**: Self-damage doesn't stale
- **Items** (Ultimate): Items no longer enter the queue (except batter items)

## Special Cases

### Multi-Hit Moves
- **Melee**: Later hitboxes can be affected by staleness if earlier hitboxes connect (bug)
- **Brawl+**: Later hitboxes are protected from staling during the move's execution
- Only counts **once** in the queue regardless of how many hits connect

### Projectiles
- Each projectile fired has staleness set at **fire time**
- If first projectile connects, move enters queue; other projectiles update staleness when hitbox changes
- Multiple projectiles per animation: Only one per animation loop counts in queue

### Transformations
- **Melee**: Zelda/Sheik share queue, no reset on Transform
- **Brawl**: Transformations reset the queue (Zelda/Sheik, Samus/ZSS, Pokémon Trainer)
- **SSB4**: No transformations
- **Ultimate**: Transformations don't reset queue (Pokémon Trainer shares queue across all 3 Pokémon)

### Ice Climbers
- **Melee**: Share one queue, both affect it equally, KO of either resets queue
- **Brawl**: Share queue, but only Popo affects it
- **Ultimate**: Both affect queue, but timing matters (Nana reads queue when her attack hits)

## Resets

- **Being KO'd**: Resets all moves to fresh (Melee onward)
- **Transformations**: Reset queue in Brawl only
- **Training Mode**: Stale moves disabled (except Ultimate if option enabled)

## Rolls/Dodges

**Important**: Rolls and dodges are **NOT** in the stale queue. They are defensive actions that:
- Don't deal damage
- Are not "attacks" or "moves" in the context of the stale system
- The stale system only tracks **attacks that connect**

## Objects That Affect Staling

### ✅ Objects That Add to Queue
- Ducks (Duck Hunt stage)
- Balloons (Smashville, Town and City)
- Statues (Castle Siege)
- Fleshy blob (Brinstar)
- Character-generated objects: Peach's Vegetables, Luma, Mechakoopa, Wario Bike, Link's Bombs, Burst Grenade, Substitute log, Header, Gyro, Bonus Fruit, Power Pellet

### ❌ Objects That Don't Affect Queue
- Hylian Shield
- Zelda's Phantom
- Villager's tree/balloons/Lloid Rocket
- Gordos
- Pikmin
- Guardian Orbitars
- Duck Hunt's can/frisbee/Wild Gunman
- Pac-Man's Fire Hydrant

## Game-Specific Details

### Melee
- Staleness **ignores knockback** for non-projectiles (only affects damage)
- Knockback reduction is minimal (based on damage reduction)
- Stale Moves bonus: -2000 points for using one move 40%+ of the time

### Brawl
- Staleness affects **both damage and knockback** (much more severe)
- Can be exploited: Staling combo throws allows combos at higher percents
- Not present in single-player modes (except Home-Run Contest)
- Items always affected (even if dropped and picked up again)

### SSB4
- Knockback reduction is **70% less effective** than damage reduction
- Staleness is generally a non-factor for KO moves
- Items: Only batter items stale, thrown items don't
- Character-produced items don't stale but still enter queue (can refresh other moves)

### Ultimate
- Moves stale when hitting shields (with 0.85× reduction factor)
- Items don't enter queue (except batter items)
- Training Mode option to toggle stale moves
- Maximum staleness: ~53% reduction

## References

- [SSBWiki - Stale Moves](https://www.ssbwiki.com/Stale_Moves)
- [Smashpedia - Stale-Move Negation](https://supersmashbros.fandom.com/wiki/Stale-Move_Negation)
