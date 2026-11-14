# Blueprint / editor modification checklist

Apply these in the Unreal Editor **after a successful C++ compile** of this branch against **UE 5.8**.

Agents cannot author `.uasset` for you.

**Verified against (repo + Content scrape, 2026-11-14):**

| Source of truth | Value |
|-----------------|--------|
| Engine pin | `PurgatoriumLex.uproject` → `"EngineAssociation": "5.8"` |
| Targets | `DefaultBuildSettings = V7`, `IncludeOrderVersion = Unreal5_8` |
| Default GameMode | `/Game/PurgatoriumLex/Game/BP_PurgatoriumLexGameMode` (`Config/DefaultEngine.ini`) |
| **Default pawn** | **`BP_PlayerCharacter`** (GameMode `DefaultPawnClass`) — **not** the Lyra-named BP |
| AnimBP on that pawn | `/Game/PurgatoriumLex/Characters/Mannequins/Anims/Unarmed/ABP_Posture` |
| Mesh on that pawn | `/Game/PurgatoriumLex/Characters/Mannequins/Meshes/SKM_Quinn_Simple` |
| Input config | `/Game/PurgatoriumLex/Input/DA_PlayerInputConfig` |
| C++ parent | `/Script/PurgatoriumLex.PlayerCharacter` |

Secondary BP (same parent, also wired):  
`Content/PurgatoriumLex/Characters/Player/BP_PlayerCharacterForLyraAnimation`  
Only edit that if you actually PIE with it. Day-to-day play uses **`BP_PlayerCharacter`**.

Related code docs: `Source/PurgatoriumLex/Combat/CombatSim_REVIEW.md`, `Characters/PlayerCharacter_Decomposition.md`.

---

## Honest limits

- This cloud agent **cannot read** `E:\Epic Games\UE_5.8`. Engine menu names below match Epic’s **UE 5.8 Data Assets** docs + your project’s reflected C++ names.
- If a class does **not** appear in a picker, you are almost always **not compiling / not loading** the `PurgatoriumLex` module — fix build first; do not invent Blueprint workarounds.

---

## 0. Compile on UE 5.8 first

1. Project must open with engine at `E:\Epic Games\UE_5.8` (Launcher / `.uproject` → **Switch Unreal Engine version…** → **5.8** if needed).
2. Generate project files from the `.uproject`, then build **`PurgatoriumLexEditor`** (Visual Studio **2022** C++ toolchain is what UBT expects; VS 2026 alone is not enough).
3. Open the editor only after that build succeeds.
4. Open **`BP_PlayerCharacter`**, Compile. Fix any red nodes / missing pins (see §4). Save.

Native components created in C++ (`FighterCombat`, `LockOnCamera`) appear on the BP **after** the parent C++ class loads. You do **not** Add Component for those — select them in the Components panel.

---

## 1. Required — `LightAttackMoveSet` Data Asset

Without an assigned move set, light attack still runs with **hardcoded fallback timings** and logs warnings; montages will not come from data.

| Step | Exact action (UE 5.8) |
|------|------------------------|
| 1 | Content Browser → folder of your choice (e.g. create `Content/PurgatoriumLex/Combat/` — **that folder does not exist yet**) |
| 2 | Right-click → **Miscellaneous → Data Asset** ([Epic 5.8 Data Assets](https://dev.epicgames.com/documentation/unreal-engine/data-assets-in-unreal-engine)) |
| 3 | Pick class **`LightAttackMoveSet`** (C++: `ULightAttackMoveSet`, path `/Script/PurgatoriumLex.LightAttackMoveSet`) |
| 4 | Name e.g. `DA_LightAttack_Default` |
| 5 | Open asset → **Moves** array. Add one element per posture you care about |
| 6 | Per row, set enum **Posture** using the **DisplayName** values from C++ (see table below) |
| 7 | Per row: **Startup Frames**, **Active Frames**, **Recovery Frames**, **Damage**, optional **Montage** (`TSoftObjectPtr`) |
| 8 | Open **`BP_PlayerCharacter`** → Components → select **`FighterCombat`** |
| 9 | Details → category **Combat Sim** → **Light Attack Move Set** = your Data Asset |
| 10 | Optional on the **pawn** (not the component): **Auto Play Sim Attack Montage** (default true) |

### Posture enum labels in the editor

C++ enum is `EPosture`. Details / dropdown shows **`UMETA(DisplayName=…)`**, not the C++ enumerator name:

| C++ | What you see in Details |
|-----|-------------------------|
| `E_Neutral` | **NEUTRAL** |
| `E_Up` | **UP** |
| `E_Down` | **DOWN** |
| `E_Left` | **LEFT** |
| `E_Right` | **RIGHT** |
| `E_DownLeft` | **DOWNLEFT** |
| `E_DownRight` | **DOWNRIGHT** |

Do **not** look for labels like `Neutral` / `DownLeft` — those are not the DisplayNames.

**Do not** build an Event Graph for light-attack authority. Tag `InputTag.LightAttack` is diverted to the sim in `APlayerCharacter::Input_AbilityInputTagPressed`.

---

## 2. Posture presentation (AnimBP)

Default pawn already references **`ABP_Posture`**.

| Step | Action |
|------|--------|
| 1 | Open `ABP_Posture` |
| 2 | Event Graph: read **`ActualPosture`** from the owning `PlayerCharacter` (category **Combat Sim \| Presentation**, `BlueprintReadOnly`) |
| 3 | Drive upper-body tilt / layered blend from that enum |

How-to: [PostureShoulderAnimationSetup.md](PostureShoulderAnimationSetup.md)

**Do not** write `ActualPosture` from Blueprint — C++ overwrites it each tick from the sim (`SyncPresentationFromCombatSim`).

---

## 3. Lock-on settings (only if you had custom values)

Lock-on **tunables** live on component **`LockOnCamera`** (`ULockOnCameraComponent`), created in C++ as subobject name `LockOnCamera`.

| C++ property | Details label on `LockOnCamera` |
|--------------|----------------------------------|
| `TargetingHeightOffset` | **Targeting Height Offset** |
| `LockOnMaxDistance` | **Lock On Max Distance** |
| `LockOnFOVDegrees` | **Lock On FOV Degrees** (reserved; candidate filter still uses screen projection) |

### Reading lock-on from Blueprint graphs

Prefer the **character** pure functions (wrappers already on `APlayerCharacter`):

| Goal | Character node (Category **Camera**) |
|------|--------------------------------------|
| Locked on enemy? | **Is Camera Locked On Enemy** |
| Camera on back? | **Is Camera Locked On Character Back** |
| Locked actor | **Get Locked On Actor** |

Or call the component: **Is Locked On Enemy**, **Is Locked On Back**, **Get Locked On Actor**, **Get Lock On Candidates**.

Boom + **Follow Camera** stay on the pawn.

**Note:** A Content scrape of `BP_PlayerCharacter` did **not** find saved defaults for the old lock-on property names (`targetingHeighOffset`, `bIsCameraLockedOnEnemy`, etc.). If your local BP never customized them, §3 is “open component, confirm defaults,” not a big rewire.

---

## 4. Broken pins / Kick vs sim light attack

### Deleted dual-path flags

C++ removed `bUseFighterCombatSim` / `bRouteLightAttackToCombatSim`. They are **not** present in the current `BP_PlayerCharacter` asset strings either. If you still see those names, you are on an **old local BP** — delete the broken nodes.

### Posture tuning moved to `FighterCombat`

Do **not** look for posture delay/penalty on the character root. On **`FighterCombat`** → category **Combat Sim | Posture**:

- **Posture Base Frames Delay**
- **Posture Bonus Frames Delay**
- **Posture Penalty Per Change**
- **Posture Max Penalty**
- **Posture Reset Frames**

Also **Sim Tick Rate** under **Combat Sim**.

### Kick still bound to the same input tag (real conflict)

Both player BPs still reference:

- `AbilityInputMappings`
- `GA_Kick`
- `InputTag.LightAttack`

And `DA_PlayerInputConfig` maps **IA_LightAttack** → **InputTag.LightAttack**.

C++ sends that tag to the **sim**, but Kick may still be **granted** via Ability Input Mappings. In **`BP_PlayerCharacter`** Details → **Abilities** → **Ability Input Mappings**: remove any row that pairs **`GA_Kick`** with **`InputTag.LightAttack`** (or stop granting Kick for now). Otherwise Kick and sim light attack fight.

Keep **InputConfig** = `DA_PlayerInputConfig` so Enhanced Input still fires the tag.

### Input component class

`BP_PlayerCharacter` already has **Override Input Component Class** = `PurgatoriumLexInputComponent`.  
`Config/DefaultInput.ini` **DefaultInputComponentClass** is also set to `/Script/PurgatoriumLex.PurgatoriumLexInputComponent` (not plain `EnhancedInputComponent`), so tag binding helpers work even if a pawn forgets the override.

---

## 5. Optional — buffer / leftovers

| Item | Facts |
|------|--------|
| Bufferable tags | If the BP leaves **Bufferable Input Tags** empty, C++ BeginPlay defaults to **Roll only** |
| Kick / SelfDamage assets | Can remain in Content; not required for sim light attack |
| `InputTag.Dash` in `DA_PlayerInputConfig` | Present in the Data Asset strings; **no** matching C++ tag in `PurgatoriumLexGameplayTags` — ignore or clean later |

---

## 6. Smoke test (PIE)

Use GameMode default pawn (**`BP_PlayerCharacter`**).

| Check | Expect |
|-------|--------|
| Output Log | `[Sim] LightAttack START…` on press (and missing-row / missing-MoveSet warnings if data incomplete) |
| Montage | Plays only if MoveSet row has **Montage** and **Auto Play Sim Attack Montage** is true |
| Lock-on | Toggle lock; camera tracks; fighting IMC still switches on lock |
| Passive posture | Locked on back + move → `ActualPosture` / `ABP_Posture` tilt |
| Active posture | Posture action overrides while locked on back |

---

## Not required for this PR

- Hitbox Blueprints / AnimNotifies as combat **authority**
- New Gameplay Ability graphs for light attack
- GGPO / network settings
- Rewiring roll / tech / charge (still on character)
