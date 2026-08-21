# Blueprint / editor modification checklist

Apply these in the Unreal Editor after pulling the combat-sim / decomposition C++.  
Agents cannot author `.uasset` for you.

**Related:** `Source/PurgatoriumLex/Combat/CombatSim_REVIEW.md`, `Characters/PlayerCharacter_Decomposition.md`

---

## 0. Compile first

Engine pin is **UE 5.8** (`EngineAssociation` + Target `V7` / `Unreal5_8`). Open/generate with **Unreal Engine 5.8**, not 5.7.

**Pre-Blueprint C++ hygiene (already applied in Source):**
- `StateFlags` is `int32` (UHT rejects `uint32` on `BlueprintType` structs)
- `GetLockOnCandidates` returns `TArray` by value (not `const TArray&`)
- `FFighterFrameInput` is **not** BlueprintType (C++ latch only)
- `FFighterSimState` fields are `BlueprintReadOnly` snapshots (no `EditAnywhere`)
- `playerHealth` marked deprecated → use AttributeSet Health
- `FighterCombat` exposes **Bonus Delay** + **Reset Frames** (not only base/penalty)
- Character ticks **after** `FighterCombat` so `ActualPosture` matches this sim frame
- Missing MoveSet posture rows log a warning (fallback timings still apply)
- Dual health: HUD = AttributeSet; `FFighterSimState.Health` is placeholder until hitboxes

1. Close PIE / hot-reload carefully if needed  
2. Right-click `.uproject` → **Switch Unreal Engine version…** → **5.8** (if the launcher still shows 5.7)  
3. **Generate Visual Studio / project files**, then build `PurgatoriumLexEditor`  
4. First open may ask to rebuild / convert assets — accept for this project copy  
5. Open the project; fix any **Blueprint compile errors** from removed properties (see §4)

Primary pawn BP (typical):  
`Content/PurgatoriumLex/Characters/Player/BP_PlayerCharacterForLyraAnimation`  
(and/or `BP_PlayerCharacter` if you still use it)

---

## 1. Required — light attack DataAsset (combat sim)

Without this, light attack still runs with **hardcoded fallback timings** and logs a warning; montages won’t play from data.

| Step | Action |
|------|--------|
| 1 | **Content Browser** → Right-click → **Miscellaneous → Data Asset** |
| 2 | Pick class **`LightAttackMoveSet`** |
| 3 | Name e.g. `DA_LightAttack_Default` (folder suggestion: `Content/PurgatoriumLex/Combat/` or under Player) |
| 4 | Open it → **Moves** array: add one row per posture you care about (`Neutral`, `Up`, `Down`, `Left`, `Right`, `DownLeft`, `DownRight`) |
| 5 | Per row set **Startup / Active / Recovery** frames, **Damage**, optional **Montage** (soft ref) |
| 6 | Open player character BP → select component **`FighterCombat`** |
| 7 | Set **Light Attack Move Set** = your Data Asset |
| 8 | Optional on pawn: **Auto Play Sim Attack Montage** = true (default) so C++ plays the montage on attack start |

**Do not** add a big Event Graph for light attack. InputTag.LightAttack is routed to the sim in C++.

---

## 2. Required — posture presentation (usually already done)

| Step | Action |
|------|--------|
| 1 | AnimBP Event Graph: read **`ActualPosture`** from the character (same as before) |
| 2 | Drive upper-body tilt / layered blend from that enum |

Details: [PostureShoulderAnimationSetup.md](PostureShoulderAnimationSetup.md)

**Do not** write `ActualPosture` from Blueprint — C++ overwrites it from the sim each tick.  
Category is now **Combat Sim \| Presentation** (`VisibleAnywhere` / read-only).

---

## 3. Required if you tuned lock-on on the character — move settings

Lock-on fields **left the character**. They live on component **`LockOnCamera`**.

| Old (on character — removed) | New (on `LockOnCamera`) |
|------------------------------|-------------------------|
| `bIsCameraLockedOnEnemy` | Use pure **`Is Camera Locked On Enemy`** (or component `Is Locked On Enemy`) |
| `bIsCameraLockedOnCharacterBack` | Use **`Is Camera Locked On Character Back`** |
| `lockedOnActor` | Use **`Get Locked On Actor`** |
| `lockOnCandidates` | Component: **Get Lock On Candidates** (read-only) |
| `targetingHeighOffset` | **Targeting Height Offset** |
| `LockOnMaxDistance` | **Lock On Max Distance** |
| `LockOnFOVDegrees` | **Lock On FOV Degrees** (reserved; filter still uses screen projection) |

After compile:

1. Open player BP → select **`LockOnCamera`**  
2. Re-enter any distance / height values you had customized  
3. In any Blueprint graph that **got** those old variables: replace with the getters above  
4. Save / compile BP

Boom + FollowCamera stay on the pawn (unchanged).

---

## 4. Fix broken BP references (deleted C++ properties)

If the character BP or child graphs show **unknown / missing pins**, clear or rewire:

| Removed from character | Where it went / what to do |
|------------------------|----------------------------|
| `bUseFighterCombatSim` | Deleted — sim is always on; remove BP branches |
| `bRouteLightAttackToCombatSim` | Deleted — light attack always → sim |
| `PostureBaseFramesDelay`, `PostureBonusFramesDelay` | Tune **`FighterCombat` → Posture Base / Bonus Frames Delay** |
| `PosturePenalty`, `PostureMaxPenaltyValue`, `PostureStalePenalty`, `PostureResetFrames` | Tune **`FighterCombat` → Penalty / Max Penalty / Reset Frames**; live stale value is sim-only |
| Character posture pending / staling Tick fields | Gone — no BP action |
| Lock-on fields listed in §3 | Move to `LockOnCamera` / getters |

Also: if you had Event Graph logic that **called ASC for LightAttack**, remove it — C++ handles `InputTag.LightAttack`.

---

## 5. Optional — input / abilities (GAS leftovers)

| Item | Notes |
|------|--------|
| `DA_PlayerInputConfig` | Keep **InputTag.LightAttack** mapped so Enhanced Input still fires the tag; C++ diverts it to the sim |
| `Ability Input Mappings` on character | You may **remove** a mapping that granted `GA_Kick` (or similar) on `InputTag.LightAttack` if Kick was your light attack — otherwise Kick and light attack fight for the same tag |
| Bufferable tags | C++ default is **Roll only**; LightAttack is not buffered via ASC anymore |
| Kick / SelfDamage assets | Can stay for experiments; not required for sim light attack |

---

## 6. Optional — posture delay feel on `FighterCombat`

On **`FighterCombat`** (not the old Character Movement category):

- **Sim Tick Rate** (default 60)  
- **Posture Base Frames Delay** / **Posture Bonus Frames Delay**  
- **Posture Penalty Per Change** / **Posture Max Penalty** / **Posture Reset Frames**

---

## 7. Smoke test (PIE)

| Check | Expect |
|-------|--------|
| Lock-on | Toggle lock; camera tracks enemy; fighting IMC still switches |
| Passive posture | Locked on back + move stick → `ActualPosture` / tilt changes |
| Active posture | Posture action overrides while locked on back |
| Light attack | Log `[Sim] LightAttack START/ACTIVE/END`; montage if assigned |
| No move set | Fallback timings + warning that MoveSet is missing |
| AnimBP | Upper-body tilt follows `ActualPosture` without BP writing it |

---

## Not required for this PR

- Hitbox Blueprints / AnimNotifies as combat authority  
- New Gameplay Ability graphs for light attack  
- GGPO / network settings  
- Rewiring roll / tech / charge (still on character for now)
