# Blueprint / editor modification checklist

Do these in the **Unreal Editor** after this branch’s C++ builds against **UE 5.8**.  
Agents cannot create or save `.uasset` for you.

Pull latest `cursor/fighter-combat-sim-1fdf` first so this file matches the repo.

**Accuracy (AGENTS.md):** engine menu/API claims below cite Epic 5.8 docs or this repo’s C++ / Content. Where cloud cannot open the asset (Git LFS pointer) or the local engine install, the step is marked **uncertain**.

---

## Facts (verified from project files)

| Item | Value | Source |
|------|--------|--------|
| Engine | `EngineAssociation: "5.8"` (install on your PC: `E:\Epic Games\UE_5.8`) | `PurgatoriumLex.uproject` |
| GameMode | `/Game/PurgatoriumLex/Game/BP_PurgatoriumLexGameMode` | Content asset path |
| **Pawn to edit** | **`/Game/PurgatoriumLex/Characters/Player/BP_PlayerCharacter`** | GameMode `DefaultPawnClass` (asset string) |
| AnimBP path on that pawn | `/Game/PurgatoriumLex/Characters/Mannequins/Anims/Unarmed/ABP_Posture` | `BP_PlayerCharacter` reference |
| Input config assigned | `/Game/PurgatoriumLex/Input/DA_PlayerInputConfig` | `BP_PlayerCharacter` + `InputConfig` |
| C++ parent | `PlayerCharacter` (`/Script/PurgatoriumLex.PlayerCharacter`) | `PlayerCharacter.h` |

Ignore `/Game/PurgatoriumLex/Characters/Player/BP_PlayerCharacterForLyraAnimation` unless you deliberately PIE with that BP.

---

## A. Compile (block everything else until this works)

1. Associate the `.uproject` with **UE 5.8** (on Windows: right‑click → **Switch Unreal Engine version…** → **5.8** via Unreal Version Selector — exact menu wording not re-verified without opening Windows here).
2. Build **`PurgatoriumLexEditor`**. For UE **5.8**, Epic documents **Visual Studio 2022 (17.14+)** or **Visual Studio 2026 (18.0+)** with the **C++ / Game development with C++** workloads (not an IDE-only install).  
   Cite: [Setting Up Visual Studio](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine) (5.8 compatibility table). Targets: `BuildSettingsVersion.V7` / `EngineIncludeOrderVersion.Unreal5_8`.
3. Open the project. If the editor asks to rebuild modules, accept.
4. In Content Browser open **`BP_PlayerCharacter`** → **Compile**.  
   - If Compile fails on missing pins, fix those first (section **D**).  
   - Save.

**Check:** In the Blueprint editor **Components** list you should see inherited **`FighterCombat`** and **`LockOnCamera`**.  
They are created in C++ as `CreateDefaultSubobject` with those names (`PlayerCharacter.cpp`).  
Do **not** **Add** another instance — both classes are also `BlueprintSpawnableComponent`, so Add would create a **second** component and break the single-authority setup.  
If they are missing after a clean build, the `PurgatoriumLex` module is not loading (wrong engine / failed compile).

---

## B. Required — create and assign light-attack data

Without this, light attack still runs (hardcoded fallback frames) but you get log warnings and **no montage from data**.

### B1. Create the Data Asset

1. Content Browser → go under **`PurgatoriumLex`** (path `/Game/PurgatoriumLex/...`).  
   Optionally create a folder **`Combat`** (`/Game/PurgatoriumLex/Combat` — it does **not** exist in Content yet).
2. Right‑click empty space → **Miscellaneous** → **Data Asset**  
   Cite: Epic UE 5.8 [Data Assets](https://dev.epicgames.com/documentation/en-us/unreal-engine/data-assets-in-unreal-engine) (“right-click … Miscellaneous > Data Asset”).
3. In the class picker, choose **`LightAttackMoveSet`**  
   (native `ULightAttackMoveSet` : `UDataAsset` — if it is not listed, section **A** failed).
4. Name it e.g. **`DA_LightAttack_Default`**. Open it.

### B2. Fill `Moves`

On the asset, category **Light Attacks** → array **Moves**  
(`ULightAttackMoveSet::Moves`, `Category = "Light Attacks"`, `LightAttackMoveSet.h`).

1. Add one array element per posture you care about (all seven if you want full coverage).
2. On each element (`FLightAttackMoveDef`), Details labels come from property names; nested categories from C++:

| Details label | C++ category | Notes |
|---------------|--------------|--------|
| **Posture** | Move | DisplayNames: **NEUTRAL**, **UP**, **DOWN**, **LEFT**, **RIGHT**, **DOWNLEFT**, **DOWNRIGHT** (`UMETA` on `EPosture` in `CombatTypes.h`) |
| **Startup Frames** | Timing | int32 |
| **Active Frames** | Timing | int32 |
| **Recovery Frames** | Timing | int32 |
| **Damage** | Damage | float |
| **Montage** | Presentation | optional `TSoftObjectPtr<UAnimMontage>` |

3. Save the Data Asset.

### B3. Assign it on the pawn

1. Open **`BP_PlayerCharacter`**.
2. Components → select **`FighterCombat`**.
3. Details → category **Combat Sim** → **Light Attack Move Set** → pick `DA_LightAttack_Default`  
   (`UFighterCombatComponent::LightAttackMoveSet`).
4. Optional (pawn root, not the component): select the **BP_PlayerCharacter (self)** root → **Combat Sim** → **Auto Play Sim Attack Montage**  
   (`bAutoPlaySimAttackMontage`, default **true** = C++ plays the montage when the sim starts an attack).
5. Compile + Save.

**Do not** add Event Graph logic for light-attack authority. Tag `InputTag.LightAttack` is handled in C++ (`Input_AbilityInputTagPressed` → `FighterCombat->PressLightAttack()`).

---

## C. Required — clear Kick from `InputTag.LightAttack`

`DA_PlayerInputConfig` maps **IA_LightAttack** → **`InputTag.LightAttack`** (asset strings).  
C++ sends that tag to the **sim** and **returns early** — it does **not** forward LightAttack to the ASC today (`PlayerCharacter.cpp`).

`BP_PlayerCharacter` still has **`GA_Kick`** under **Ability Input Mappings** with that tag (Content + `AbilityInputMappings` / `EditDefaultsOnly`). Kick is **granted** but should **not** activate from the same press path while the early-return stays. Remove the row anyway so Class Defaults match sim ownership and you don’t re-enable Kick by accident.

1. Open **`BP_PlayerCharacter`**.
2. Open **Class Defaults** (toolbar **Class Defaults** — fields are `EditDefaultsOnly`).
3. Category **Abilities** → **Ability Input Mappings**.
4. Remove any row where:
   - **Ability Class** = `GA_Kick` (struct field `AbilityClass`), and  
   - **Input Tag** = `InputTag.LightAttack` (struct field `InputTag`)
5. Compile + Save.

Keep **Input Config** = `DA_PlayerInputConfig` (category **Enhanced Input**) so the tag still fires.

---

## D. Only if Compile shows broken pins

These C++ properties were **removed** from the character (not present on current `BP_PlayerCharacter` binary either). Clear / delete broken nodes that still reference them if your local BP is older:

- `bUseFighterCombatSim`, `bRouteLightAttackToCombatSim`
- Old posture delay / penalty properties on the **character root** (tuning is on **`FighterCombat`** now)
- Old lock-on fields on the character (`targetingHeighOffset`, `bIsCameraLockedOnEnemy`, `lockedOnActor`, …)

**Posture tuning** (optional feel): select **`FighterCombat`** → **Combat Sim** / nested **Posture** (`Category = "Combat Sim|Posture"`):

- **Sim Tick Rate**
- **Posture Base Frames Delay**
- **Posture Bonus Frames Delay**
- **Posture Penalty Per Change**
- **Posture Max Penalty**
- **Posture Reset Frames**

**Lock-on tuning** (optional): select **`LockOnCamera`** → **Lock On**:

- **Targeting Height Offset**
- **Lock On Max Distance**
- **Lock On FOV Degrees** (reserved; filter uses screen projection today — `LockOnCameraComponent.h` / `.cpp`)

If a Blueprint graph needs lock state, use character pures (**Camera** category):

- **Is Camera Locked On Enemy**
- **Is Camera Locked On Character Back**
- **Get Locked On Actor**

---

## E. Posture AnimBP — verify, don’t rebuild

`BP_PlayerCharacter` references **`ABP_Posture`** (verified).

**Uncertain in cloud:** `ABP_Posture.uasset` is a **Git LFS pointer** here, so the AnimGraph/`ActualPosture` wiring cannot be binary-verified in this environment. On your machine (real file or LFS pulled):

1. PIE or open **`ABP_Posture`**.
2. Confirm upper-body tilt follows character **`ActualPosture`** (`Combat Sim | Presentation`, read-only; C++ overwrites it each tick — `PlayerCharacter.h`).
3. Only if tilt is wrong: follow [PostureShoulderAnimationSetup.md](PostureShoulderAnimationSetup.md).  
   **Do not** write `ActualPosture` from Blueprint.

---

## F. Smoke test (PIE)

Play with the GameMode default pawn (`BP_PlayerCharacter`).

| Check | Expect | Source |
|-------|--------|--------|
| Light attack | Output Log contains `[Sim] LightAttack START` | `FighterCombatSim.cpp` (`LogFighterCombatSim`, Warning) |
| With MoveSet + Montage + Auto Play on | Montage plays | `bAutoPlaySimAttackMontage` + `OnLightAttackStarted` |
| MoveSet unset | Warning: No LightAttackMoveSet — hardcoded fallback | `FighterCombatSim.cpp` |
| MoveSet missing posture row | Warning from `[LightAttackMoveSet]` + fallback timings | `LightAttackMoveSet.cpp` |
| Lock-on | Toggle lock; camera tracks; fighting IMC still switches | `LockOnCamera` + character handlers |
| Posture | Lock on back + move/posture stick → tilt via `ActualPosture` | Presentation sync (AnimBP verify on your PC) |

---

## Not required now

- Hitbox / AnimNotify as combat **authority**
- New Gameplay Ability graphs for light attack
- GGPO settings
- Rewiring roll / tech / charge
