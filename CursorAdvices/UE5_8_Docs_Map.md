# Unreal Engine 5.8 — Official documentation authority

**Rule for this project:** Epic’s **UE 5.8** documentation and C++ API Reference are the **primary technical authority** for engine APIs, reflection, modules, Gameplay Framework, Blueprints, animation tooling, and packaging.

Architecture choices that *intentionally diverge* from Epic defaults (e.g. combat authority = custom fixed-tick sim instead of GAS) are recorded in `Architecture_Decisions.md` and `CombatSim_Design.md`. Those override Epic *product patterns*, not Epic *API truth*.

Pinned hubs:

- [Unreal Engine 5.8 Documentation](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-documentation)
- [Programming with C++](https://dev.epicgames.com/documentation/unreal-engine/programming-with-cplusplus-in-unreal-engine)
- [C++ API Reference](https://dev.epicgames.com/documentation/unreal-engine/API?lang=en-US)

Always prefer `application_version=5.8` / 5.8-labelled pages when links offer a version picker.

---

## Top-level 5.8 doc tree (official hub)

From the [5.8 documentation index](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-documentation):

| Section | Use for this project |
|---------|----------------------|
| What's New | Version deltas (5.7 → 5.8 toolchain, UHT, etc.) |
| Understanding the Basics | Editor / project fundamentals |
| Working with Content | Assets, DataAssets, import |
| Building Virtual Worlds | Levels / maps |
| Designing Visuals… | Materials / lighting (presentation only) |
| AI Features… | Not combat authority |
| Creating Visual Effects | Niagara (presentation) |
| Gameplay Tutorials | Patterns to steal carefully |
| Blueprints Visual Scripting | Minimal surface (checklist) |
| **Programming with C++** | **Primary daily reference** |
| Gameplay Systems | Abilities, tags, movement — know Epic’s model before diverging |
| Mobile Development | Later |
| Animating Characters and Objects | AnimBP / montages / posture tilt |
| Creating User Interfaces | UMG / HUD |
| Setting Up Your Production Pipeline | Build, VS, packaging |
| Testing and Optimizing… | Perf later |
| Sharing and Releasing Projects | Shipping later |
| Samples and Tutorials | Lyra / samples as *examples*, not gospel |

---

## Programming with C++ (must-know)

Hub: [Programming with C++](https://dev.epicgames.com/documentation/unreal-engine/programming-with-cplusplus-in-unreal-engine)

| Topic | Official page | Project mapping |
|-------|---------------|-----------------|
| Reflection | [Reflection System](https://dev.epicgames.com/documentation/unreal-engine/reflection-system-in-unreal-engine) | `UCLASS` / `UPROPERTY` / `UFUNCTION` / `UENUM` / `USTRUCT` = what Editor + UHT see |
| Structs | [Structs](https://dev.epicgames.com/documentation/unreal-engine/structs-in-unreal-engine) | `FFighterSimState`, `FFighterFrameInput`, move defs. `BlueprintType` ⇒ BP-safe property types only (`int32` not `uint32`) |
| Metadata | [Metadata Specifiers](https://dev.epicgames.com/documentation/unreal-engine/metadata-specifiers-in-unreal-engine) | `meta=(ClampMin=…)`, Categories, TitleProperty |
| Containers | [Containers](https://dev.epicgames.com/documentation/unreal-engine/containers-in-unreal-engine) | Prefer `TArray` / `TMap` / `TSet` over STL in reflected / gameplay code |
| Gameplay Architecture | [Gameplay Architecture](https://dev.epicgames.com/documentation/unreal-engine/gameplay-architecture-in-unreal-engine) | Actor / Object / Component hierarchy; our sim is *beside* this, bridged by components |
| Delegates | [Delegates](https://dev.epicgames.com/documentation/unreal-engine/delegates-in-unreal-engine) | `OnLightAttackStarted`, lock-on events |
| Coding Standard | Coding Standard (under Programming with C++) | Epic naming (`U`/`A`/`F`/`E`/`b`) — see also `UE5_Basics_UObject_And_Naming.md` |
| Exposing to BP | “Exposing Gameplay Elements to Blueprints” (Programming) | Keep BP surface thin: DataAsset assign + AnimBP reads |

Related overview: [Programming with CPP](https://dev.epicgames.com/documentation/unreal-engine/programming-with-cpp-in-unreal-engine) — classes, interfaces, `UCLASS`/`UFUNCTION`/`UPROPERTY`/`USTRUCT`/`UINTERFACE`.

### Struct rules we rely on (official)

From [Structs in Unreal Engine](https://dev.epicgames.com/documentation/unreal-engine/structs-in-unreal-engine):

- A reflected struct is a **`UStruct`**, not a `UObject` → **not GC’d as an object**, no `UFUNCTION` *on the struct itself*.
- Members need `UPROPERTY` to be visible to reflection / Blueprints.
- `BlueprintType` exposes the struct as a BP variable type (Make/Break when properties are Blueprint-readable).
- **UStructs are not replicated as wholes**; individual `UPROPERTY` replication rules apply when used inside actors — our combat authority does **not** use UE replication for the sim blob.

---

## C++ API Reference (modules / plugins)

Index: [Unreal Engine C++ API Reference](https://dev.epicgames.com/documentation/unreal-engine/API?lang=en-US)

Engine software is packaged as:

1. **Modules** — Developer / Editor / Runtime  
2. **Plugins** — optional modules + content enabled per project  

### Runtime modules we touch (or will)

| Module | Role here |
|--------|-----------|
| `Core` / `CoreUObject` | Types, `UObject`, reflection |
| `Engine` | Actors, Components, CMC, World |
| `InputCore` / Enhanced Input plugin | Devices → intent |
| `NetCore` | Engine net primitives (GGPO is *outside* this for combat auth) |
| `UMG` | HUD / attributes UI |
| `GameplayTags` / `GameplayTasks` | Tags + async tasks (ASC era leftovers) |

### Plugins we already enable / depend on

| Plugin / module | Role here |
|-----------------|-----------|
| `EnhancedInput` | IMC / IA → character / sim input latch |
| `GameplayAbilities` | Starved for combat; still on project for Kick/HUD path |
| ModelingToolsEditorMode | Editor-only (uproject) |

Search the API by class name when unsure (`ACharacter`, `UActorComponent`, `UDataAsset`, `UAnimInstance`, …).

---

## Gameplay Systems & movement (know Epic’s model)

| Topic | Official page | Note for us |
|-------|---------------|-------------|
| Character Movement (networked) | [Understanding Networked Movement in the CMC](https://dev.epicgames.com/documentation/unreal-engine/understanding-networked-movement-in-the-character-movement-component-for-unreal-engine) | CMC owns locomotion replication in Epic’s model. Our combat sim does **not** replace CMC yet; GGPO later must confront this. |
| GAS + root motion | Same CMC page | Epic syncs montages/root motion via abilities. We play montages as **presentation** from sim events — intentional divergence. |
| Gameplay Ability System | Gameplay Systems / Ability System docs + API `GameplayAbilities` | Reference when reading old Kick; do not expand combat on it. |

---

## Animation (presentation)

Hub: [Animating Characters and Objects](https://dev.epicgames.com/documentation/unreal-engine/animating-characters-and-objects-in-unreal-engine)

| Topic | Use |
|-------|-----|
| Skeletal Mesh Animation System / AnimBP | Read `ActualPosture`; layered upper-body tilt |
| Montages | Soft refs on `LightAttackMoveSet`; play from C++ on sim event |
| Control Rig / Sequencer | Out of combat-authority scope |

Local how-to still valid: `PostureShoulderAnimationSetup.md`.

---

## Blueprints (minimal)

Hub: Blueprints Visual Scripting (from 5.8 index).

Project policy: **no Event Graph for light-attack authority**. Editor work = DataAsset + property retargets. Checklist: `Blueprint_Modification_Checklist.md`.

---

## Production / toolchain

| Topic | Where |
|-------|--------|
| VS / C++ project setup | Production Pipeline + Epic “Setting Up Visual Studio” pages for **5.8** |
| Build targets | Our `*.Target.cs` (`V7`, `Unreal5_8`) must match installed engine |
| Generate project files | Engine BatchFiles via `.uproject` — produces `.sln` (keep); ignore Automation / `.slnx` |

---

## How answers should be derived (agent + human)

1. **Engine fact** (API, specifier, module, AnimBP node, CMC behavior) → open the matching **5.8** doc or API page; cite it.  
2. **Project architecture** (who owns combat truth, rollback plan) → `Architecture_Decisions.md` / `Combat/` code.  
3. If Epic pattern conflicts with rollback (GAS as combat authority, AnimNotify as hit authority) → **state the conflict**, keep our decision, use Epic only for the surrounding APIs.

This file is the **index**, not a dump of every Epic page. The “whole documentation” lives on Epic’s site; this map is how we stay precise against it without inventing engine behavior.
