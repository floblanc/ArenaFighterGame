# Blueprint / editor modification checklist

Do these in the **Unreal Editor** after this branch’s C++ builds against **UE 5.8**.  
Agents cannot create or save `.uasset` for you.

Pull latest `cursor/fighter-combat-sim-1fdf` first so this file matches the repo (older local copies of this checklist were wrong).

---

## Facts (verified from project files)

| Item | Value |
|------|--------|
| Engine | `PurgatoriumLex.uproject` → `EngineAssociation: "5.8"` (install: `E:\Epic Games\UE_5.8`) |
| GameMode | `/Game/PurgatoriumLex/Game/BP_PurgatoriumLexGameMode` |
| **Pawn to edit** | **`/Game/PurgatoriumLex/Characters/Player/BP_PlayerCharacter`** (GameMode `DefaultPawnClass`) |
| AnimBP already on that pawn | `/Game/PurgatoriumLex/Characters/Mannequins/Anims/Unarmed/ABP_Posture` |
| Input config already assigned | `/Game/PurgatoriumLex/Input/DA_PlayerInputConfig` |
| C++ parent | `PlayerCharacter` (`/Script/PurgatoriumLex.PlayerCharacter`) |

Ignore `/Game/PurgatoriumLex/Characters/Player/BP_PlayerCharacterForLyraAnimation` unless you deliberately PIE with that BP.

---

## A. Compile (block everything else until this works)

1. Associate the `.uproject` with **UE 5.8** (right‑click → **Switch Unreal Engine version…** → **5.8** if needed).
2. Build **`PurgatoriumLexEditor`** (Visual Studio **2022** C++ tools, or Editor compile after generate). VS 2026 alone is not enough for UBT.
3. Open the project. If the editor asks to rebuild modules, accept.
4. In Content Browser open **`BP_PlayerCharacter`** → **Compile**.  
   - If Compile fails on missing pins, fix those first (section **D**).  
   - Save.

**Check:** In the Blueprint editor **Components** list you should see inherited **`FighterCombat`** and **`LockOnCamera`**.  
Do **not** use **Add** → Component for those — they come from the C++ constructor (`CreateDefaultSubobject`).  
If they are missing after a clean build, the `PurgatoriumLex` module is not loading (wrong engine / failed compile).

---

## B. Required — create and assign light-attack data

Without this, light attack still runs (hardcoded fallback frames) but you get log warnings and **no montage from data**.

### B1. Create the Data Asset

1. Content Browser → go under **`PurgatoriumLex`** (path `/Game/PurgatoriumLex/...`).  
   Optionally create a folder **`Combat`** (`/Game/PurgatoriumLex/Combat` — it does **not** exist yet).
2. Right‑click empty space → **Miscellaneous** → **Data Asset**  
   (Epic UE 5.8: [Data Assets](https://dev.epicgames.com/documentation/unreal-engine/data-assets-in-unreal-engine)).
3. In the class picker, choose **`LightAttackMoveSet`**  
   (native class `ULightAttackMoveSet` — if it is not listed, section **A** failed).
4. Name it e.g. **`DA_LightAttack_Default`**. Open it.

### B2. Fill `Moves`

On the asset, category **Light Attacks** → array **Moves**:

1. Add one array element per posture you care about (all seven if you want full coverage).
2. On each element set:

| Details label | Notes |
|---------------|--------|
| **Posture** | Use DisplayNames: **NEUTRAL**, **UP**, **DOWN**, **LEFT**, **RIGHT**, **DOWNLEFT**, **DOWNRIGHT** (not `Neutral` / `DownLeft`) |
| **Startup Frames** | int |
| **Active Frames** | int |
| **Recovery Frames** | int |
| **Damage** | float |
| **Montage** | optional soft AnimMontage |

3. Save the Data Asset.

### B3. Assign it on the pawn

1. Open **`BP_PlayerCharacter`**.
2. Components → select **`FighterCombat`**.
3. Details → category **Combat Sim** → **Light Attack Move Set** → pick `DA_LightAttack_Default`.
4. Optional (pawn root, not the component): select the **BP_PlayerCharacter (self)** root → **Combat Sim** → **Auto Play Sim Attack Montage** (default **true** = C++ plays the montage when the sim starts an attack).
5. Compile + Save.

**Do not** add Event Graph logic for light-attack authority. Input tag `InputTag.LightAttack` is handled in C++ (`PressLightAttack` on the sim).

---

## C. Required — stop Kick fighting the sim on the same tag

`DA_PlayerInputConfig` maps **IA_LightAttack** → **`InputTag.LightAttack`**.  
C++ sends that tag to the **sim**.  
`BP_PlayerCharacter` still grants **`GA_Kick`** through **Ability Input Mappings** with the same tag → Kick and sim light attack conflict.

1. Open **`BP_PlayerCharacter`**.
2. Open **Class Defaults** (toolbar **Class Defaults**, or edit defaults — these fields are `EditDefaultsOnly`).
3. Category **Abilities** → **Ability Input Mappings**.
4. Remove any row where:
   - **Ability Class** = `GA_Kick` (or similar Kick ability), and  
   - **Input Tag** = `InputTag.LightAttack`
5. Compile + Save.

Keep **Input Config** = `DA_PlayerInputConfig` (category **Enhanced Input**) so the tag still fires.

---

## D. Only if Compile shows broken pins

These C++ properties were **removed** from the character. Clear / delete broken nodes that still reference them:

- `bUseFighterCombatSim`, `bRouteLightAttackToCombatSim`
- Old posture delay / penalty properties on the **character root** (tuning is on **`FighterCombat`** now)
- Old lock-on fields on the character (`targetingHeighOffset`, `bIsCameraLockedOnEnemy`, `lockedOnActor`, …)

**Posture tuning** (optional feel): select **`FighterCombat`** → **Combat Sim** / **Combat Sim | Posture**:

- **Sim Tick Rate**
- **Posture Base Frames Delay**
- **Posture Bonus Frames Delay**
- **Posture Penalty Per Change**
- **Posture Max Penalty**
- **Posture Reset Frames**

**Lock-on tuning** (optional): select **`LockOnCamera`** → **Lock On**:

- **Targeting Height Offset**
- **Lock On Max Distance**
- **Lock On FOV Degrees** (unused by filter today; screen projection is used)

If a Blueprint graph needs lock state, use character pures (**Camera** category):

- **Is Camera Locked On Enemy**
- **Is Camera Locked On Character Back**
- **Get Locked On Actor**

---

## E. Posture AnimBP — verify, don’t rebuild

`BP_PlayerCharacter` already points at **`ABP_Posture`**.

1. PIE or open **`ABP_Posture`**.
2. Confirm upper-body tilt follows character **`ActualPosture`** (`Combat Sim | Presentation`, read-only; C++ overwrites it each tick).
3. Only if tilt is wrong: follow [PostureShoulderAnimationSetup.md](PostureShoulderAnimationSetup.md).  
   **Do not** write `ActualPosture` from Blueprint.

---

## F. Smoke test (PIE)

Play with the GameMode default pawn (`BP_PlayerCharacter`).

| Check | Expect |
|-------|--------|
| Light attack | Log contains `[Sim] LightAttack START` |
| With MoveSet + Montage + Auto Play on | Montage plays |
| MoveSet empty / missing row | Warning + fallback timings |
| Lock-on | Toggle lock; camera tracks; fighting IMC still switches |
| Posture | Lock on back + move/posture stick → tilt via `ActualPosture` |

---

## Not required now

- Hitbox / AnimNotify as combat **authority**
- New Gameplay Ability graphs for light attack
- GGPO settings
- Rewiring roll / tech / charge
