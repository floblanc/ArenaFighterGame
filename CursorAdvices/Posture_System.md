# Posture system

Oriented combat postures: design rules and where they live in code.

---

## Design

- **6 directions + neutral:** Up, Down, Left, Right, DownLeft, DownRight, Neutral  
- **Passive:** movement direction while lock-on (“camera on back”)  
- **Active:** dedicated posture input overwrites movement  
- **Always visual:** idle/walk upper body tilts with current posture (AnimBP)  
- **Combat:** attack/block/parry (later grabs) use posture; attack **snapshots** on press  

Anti-spam (future): [Posture_Spam_Exhaustion_Design.md](Posture_Spam_Exhaustion_Design.md).

---

## Authority vs presentation

| Role | Owner |
|------|--------|
| True current posture | `FFighterSimState::Posture` (sim) |
| Mirror for AnimBP | `APlayerCharacter::ActualPosture` (synced each tick when sim enabled) |
| Swing direction for current move | `AttackSnapshotPosture` (frozen at light-attack start) |

**Rule:** sim writes → character mirrors → AnimBP reads.  
Do not treat Blueprint writes to `ActualPosture` as authority while `bUseFighterCombatSim` is true (overwrite next sync).

---

## AnimBP tilt

Still valid how-to for layered upper-body blend:  
[PostureShoulderAnimationSetup.md](PostureShoulderAnimationSetup.md)

No change required if AnimBP already reads `ActualPosture`.

---

## Delay and staling

- Posture changes can be delayed N sim frames (`PostureBaseFramesDelay`, etc.)  
- Repeated changes accumulate staling penalty (duration / intangibility helpers)  
- One pending change at a time (direction noise during delay is dropped) — same rule as old character code  

Live values belong in sim state; see [CombatSim_Design.md](CombatSim_Design.md).

---

## Code map

| Concern | Place |
|---------|--------|
| Enum + state fields | `Source/PurgatoriumLex/Combat/CombatTypes.h` |
| Direction → posture | `FCombatPostureMath::PostureFromDirection` |
| Sim posture tick | `FighterCombatSim.cpp` → `TickPosture` |
| Character feeds direction | `PlayerCharacter.cpp` → `ProcessPostureInput` |
| Mirror for mesh | `SyncPresentationFromCombatSim` |
| Snapshot on attack | `StartLightAttack(State.Posture)` |
