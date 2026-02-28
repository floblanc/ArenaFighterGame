# Posture Shoulder Animation Setup (Enum → Mixed Animations)

## The idea

- You already have an **enum** in C++ (e.g. `EPosture`: Neutral, Up, Down, Left, Right, DownLeft, DownRight) and a variable on the character (e.g. `ActualPosture`).
- You want **mixed animations**: the same Idle and Walk as base, but the **upper body (shoulders/spine)** changes with the current posture.
- So: **base motion (Idle / Walk) + posture overlay (upper body only)**, with the enum choosing which overlay is active.

No extra C++ is required for the animation side; you only need to read the enum in the Animation Blueprint and drive blending there.

---

## What you need (high level)

1. **Base pose** – Comes from a **State Machine** that switches between your existing **Idle** and **Walk** Animation Sequences.
2. **Posture overlay** – One of 6 poses (Up, Down, Left, Right, DownLeft, DownRight). You need to **create** these 6 poses in the editor (as sequences or as poses in a Pose Asset).
3. **Blending** – A **Layered Blend per Bone** node: base = State Machine output, overlay = posture pose(s), with a **Branch Filter** (or **Blend Mask**) so only upper-body bones use the overlay. The **enum** decides which overlay has weight 1 and the rest 0.
4. **Enum in the Blueprint** – In the Animation Blueprint **Event Graph**, read `ActualPosture` from the character and store it in a **variable**. In the **Anim Graph**, use that variable to set the blend **alphas** (or curve values) so only the selected posture is visible.

---

## UE5 terms (short reference)

| Term | Meaning |
|------|--------|
| **Animation Sequence** | Asset with keyframes for the Skeleton (imported or edited in **Animation Sequence Editor**). |
| **Animation Blueprint** | Asset that controls animation at runtime. Has **Anim Graph** (pose blending) and **Event Graph** (logic, variables). Parent class: **AnimInstance**. |
| **Anim Graph** | Graph where you build the final pose (State Machine, **Layered Blend per Bone**, etc.). |
| **Event Graph** | Graph where you use **Event Blueprint Update Animation** and set variables (e.g. from the Character). |
| **State Machine** | Anim Graph node with states (e.g. Idle, Walk) and transitions. Output is one pose. |
| **Layered Blend per Bone** | Blends a **Base Pose** with one or more **Blend Poses** on a per-bone basis. You add pins with **Add Blend Pin**. Each blend has an **Alpha**. Use **Branch Filter** (or **Blend Mask**) so only certain bones (e.g. spine and children) use the blend pose. |
| **Branch Filter** | On **Layered Blend per Bone**: bone name + **Blend Depth**. Only that bone and its children down to that depth take the blend pose; rest stay from base. |
| **Pose Asset** | Asset that stores one or more named poses. Can be blended by **curve weights**. |
| **Pose Blender** | Anim Graph node from dragging a **Pose Asset** in. Output pose is a blend of the asset’s poses; blend is driven by **animation curves** (same names as pose names). |
| **Modify Curve** | Anim Graph node that sets curve values from float inputs. Used to drive **Pose Blender** from variables (e.g. from your enum). |
| **Animation Sequence Editor** | Open by double-clicking an Animation Sequence. Toolbar, **Skeleton Tree**, viewport, timeline. Use **Bone Manipulation** and **Add Key** to add keyframes (and additive tracks when applicable). |
| **Skeleton Tree** | List of bones in Animation Sequence Editor / Skeleton Editor. |
| **Create Asset** | Toolbar in Skeleton / Animation Sequence Editor: e.g. **Create Pose Asset** from current pose. |

---

## Process overview

```
Character (C++) ActualPosture (enum)
        ↓
Animation Blueprint Event Graph: set variable "Posture"
        ↓
Anim Graph:
  1) State Machine (Idle / Walk) → base pose
  2) Six posture poses (from sequences OR from one Pose Asset)
  3) Layered Blend per Bone: Base = (1), Blend Poses = (2), Alphas = from Posture
  4) Branch Filter = upper body only (e.g. spine_01, depth 2)
        ↓
Final Animation Pose
```

You can create the 6 postures in two ways:

- **Path A – Six Animation Sequences**: One small sequence per posture (shoulders posed). Easy to author and wire: 6 blend pins, 6 alphas from enum.
- **Path B – One Pose Asset with 6 poses**: One asset with 6 named poses; **Pose Blender** + **Modify Curve** to set curve weights from the enum; one blend pin for “posture overlay”.

---

## Path A vs Path B: pros, cons, and sources

### Path A – Six Animation Sequences

| Pros | Cons |
|------|------|
| **Familiar workflow** – Same editor as Idle/Walk (Animation Sequence Editor). No curve setup. | **More assets** – Six separate sequences to manage, name, and version. |
| **Direct control** – Each posture is a full sequence: you see exactly what plays, frame by frame. | **Redundant data** – Each sequence stores full skeleton; only upper body is used by the blend (Branch Filter). Slightly more memory than 6 poses in one asset. |
| **Simple Anim Graph** – One **Sequence Player** per posture, one **Add Blend Pin** per posture, alphas from a **Switch on Enum**. No curves or Pose Blender. | **Six blend pins** – Layered Blend per Bone has 6 pins and 6 alphas to wire; more connections, but straightforward. |
| **Easy to tweak** – Open one sequence, move bones, Add Key; no curve names to match. | **No built-in in-between blending** – To blend between two postures (e.g. Up → Left) you'd need extra logic (e.g. lerp alphas over time). |
| **Works with any skeleton** – No Pose Asset or curve naming; just duplicate, pose, and hook up. | **Copy-paste posture edits** – Changing "all postures a bit more forward" means editing 6 assets unless you use a shared approach. |
| **No curve dependency** – Enum → alphas only; no **Modify Curve** or curve names. | |

**Best when:** You want minimal concepts (sequences + blend pins only), one posture active at a time, and easy per-posture tweaks in the Animation Sequence Editor.

**Sources:** Unreal Engine 5 Documentation — Animation Sequences, Animation Sequence Editor, Layered Blend per Bone (FAnimNode_LayeredBoneBlend: add blend pins, Branch Filter).

---

### Path B – One Pose Asset with 6 poses

| Pros | Cons |
|------|------|
| **Single asset** – One Pose Asset; easier to manage, share, and reference. | **Curve-driven** – Pose Blender needs **animation curves** (names = pose names). You must drive them (e.g. **Modify Curve** from enum); one more concept. |
| **Less redundant data** – Poses are stored as named poses + weights; no full timeline per posture. Often lighter than 6 full sequences. | **Setup steps** – Create Pose Asset, add 6 poses, name them to match curves, then wire **Modify Curve** → Pose Blender → one blend pin. |
| **One blend pin** – Layered Blend per Bone has one "posture overlay" pin; alpha = 1 when posture ≠ Neutral, 0 when Neutral. | **Curve name coupling** – Pose names in the asset must match curve names used in the Anim Graph (e.g. `Posture_Up`). Renames must be consistent. |
| **Natural for in-between blending** – Pose Asset is designed for weighted blending; you can set two curve weights to 0.5 each for a blend between postures (e.g. smooth transition). | **Less "see the animation"** – You edit poses in the Pose Asset UI, not a timeline; some animators prefer sequences. |
| **Reusable** – Same Pose Asset can be used in other Blueprints (e.g. different character with same skeleton). | **Pose Blender requirement** – Output only correct when curves are driven; if curves are missing or wrong, pose can be wrong or empty. |
| **Fits curve-based pipelines** – If you already use **Animation Curves** or **Modify Curve** elsewhere, this fits the same model. | **Documentation spread** – Workflow spans Pose Assets, Pose Blender, Modify Curve; more docs to check. |

**Best when:** You want one asset, fewer blend pins, optional smooth blending between postures, or a curve-driven animation style.

**Sources:** Unreal Engine 5 Documentation — Animation Pose Assets, Creating a Pose Asset, Pose Blender, Animation Blueprint Modify Curve, Curve Driven Animation.

---

### Quick comparison

| Criteria | Path A (6 sequences) | Path B (1 Pose Asset) |
|----------|----------------------|------------------------|
| **Number of assets** | 6 | 1 |
| **Anim Graph complexity** | 6 blend pins, 6 alphas, 6 Sequence Players | 1 blend pin, 1 Pose Blender, Modify Curve, curve names |
| **Enum → blend** | Switch on Enum → set 6 alpha variables | Switch on Enum → set 6 curve values (Modify Curve) |
| **Authoring** | Animation Sequence Editor, Bone Manipulation, Add Key | Pose Asset editor, create/add poses, name poses |
| **Memory** | 6 full sequences (only upper body used at runtime) | One asset, 6 poses (typically smaller) |
| **Blend between postures** | Manual (lerp alphas over time) | Natural (set two curve weights between 0 and 1) |
| **Learning curve** | Lower (sequences + blend pins only) | Higher (Pose Asset, curves, Pose Blender) |

---

Below: first connect the enum and the base blend (Part 1–2), then choose Path A or B for creating the poses (Part 3), then wire the Anim Graph (Part 4).

---

## Part 1: Animation Blueprint and enum

1. **Create the Animation Blueprint**
   - Content Browser → right-click → **Animation** → **Animation Blueprint**.
   - Pick your character **Skeleton**. Parent Class: **AnimInstance**. Create and open it.

2. **Variable for posture**
   - In **My Blueprint**, add a variable `Posture`, type **Enum** → your `EPosture` enum.
   - Optionally: **Instance Editable** (to test in preview), **Blueprint Read Only** (set only from code/Event Graph).

3. **Read enum from character**
   - In **Event Graph**, use **Event Blueprint Update Animation**.
   - **Try Get Pawn Owner** → **Cast to** your Character class → get `ActualPosture` → **Set** `Posture` (your variable).

If the enum does not appear, ensure it is `UENUM(BlueprintType)` in C++ and the project is compiled.

---

## Part 2: Base motion (Idle / Walk) in Anim Graph

1. In **Anim Graph**, add a **State Machine** (e.g. name **Locomotion**).
2. Open it and add two states:
   - **Idle** – use your Idle **Animation Sequence** (e.g. **Sequence Player** or state output).
   - **Walk** – use your Walk **Animation Sequence**.
3. Add transitions (e.g. Idle ↔ Walk) using a **Speed** (or similar) variable and transition rules (e.g. Speed > 0 → Walk; else Idle). Make sure **Speed** is set in Event Graph from the Character movement.
4. Leave the State Machine **output** unconnected for now; it will become the **Base Pose** of **Layered Blend per Bone** in Part 4.

---

## Part 3: Create the 6 posture poses

Pick one path.

---

### Path A: Six Animation Sequences (one pose per sequence)

Good if you prefer to work only with sequences and simple blend pins.

1. **Duplicate your Idle** Animation Sequence six times. Name them e.g. `AM_Posture_Up`, `AM_Posture_Down`, `AM_Posture_Left`, `AM_Posture_Right`, `AM_Posture_DownLeft`, `AM_Posture_DownRight`.

2. **Edit each sequence** (double-click → **Animation Sequence Editor**):
   - In the **Skeleton Tree**, select the bones you want to tilt (e.g. `spine_01`, `spine_02`, `clavicle_l`, `clavicle_r` — names depend on your Skeleton).
   - In the viewport, use **Bone Manipulation** to rotate those bones into the desired posture (Up = lean back, Down = forward, Left/Right = tilt sides, DownLeft/DownRight = combine).
   - In the Toolbar, click **Add Key** so the pose is stored (additive track or normal track, depending on editor behavior).
   - Keep the pose for the whole duration (e.g. key at 0 and at end). Save.

3. **Result**: You have 6 Animation Sequences. Each will be played (e.g. looping) and fed as one **Blend Pose** into **Layered Blend per Bone** (Part 4). You will add **6 blend pins** and drive their **Alphas** from the enum.

---

### Path B: One Pose Asset with 6 poses

Good if you want one asset and curve-driven blending.

1. **Create a Pose Asset** that will hold all 6 postures:
   - Either: open your **Skeleton** in **Skeleton Editor**, pose the character, then Toolbar → **Create Asset** → **Create Pose Asset**, and add more poses to the same asset.
   - Or: right-click an **Animation Sequence** (e.g. Idle) in Content Browser → **Create** → **Create Pose Asset** to start from a frame; then add/rename poses in the Pose Asset so you have exactly 6 poses with names that match **curve names** you will use (e.g. `Posture_Up`, `Posture_Down`, `Posture_Left`, `Posture_Right`, `Posture_DownLeft`, `Posture_DownRight`).

2. **Author each of the 6 poses** in the Pose Asset (edit bone transforms for spine/shoulders so each named pose matches the posture you want).

3. **Result**: One Pose Asset with 6 named poses. In the Anim Graph you will use a **Pose Blender** (drag this Pose Asset in) and drive its curves with a **Modify Curve** node so that the curve with the same name as the active posture = 1, others = 0. That gives you a single “posture overlay” pose to feed into **one** blend pin (Part 4).

---

## Part 4: Mix base + posture (Layered Blend per Bone)

1. **Add Layered Blend per Bone**
   - In **Anim Graph**, add **Layered Blend per Bone**.
   - Connect **Base Pose** ← output of your **State Machine** (Idle/Walk).
   - Connect **Layered Blend per Bone** output → **Final Animation Pose**.

2. **Restrict blend to upper body**
   - Select **Layered Blend per Bone**. In **Details**:
   - Use **Branch Filter** (default): add a filter, **Bone** = first spine bone (e.g. `spine_01`), **Blend Depth** = 2 (so spine and its children — shoulders, arms — use the blend; legs stay from base).
   - Or use a **Blend Mask** asset that includes only upper-body bones. Skeleton bone names can be checked in **Skeleton Editor**.

3. **Wire posture into the blend**

   **If you used Path A (6 sequences):**
   - Right-click **Layered Blend per Bone** → **Add Blend Pin** six times (6 blend pose pins).
   - For each pin: drag the matching **Animation Sequence** into the graph, add a **Sequence Player** (looping) if needed, connect its pose to that pin’s **Blend Pose** input.
   - Create 6 **float** variables (e.g. `Alpha_Up`, `Alpha_Down`, …). In **Event Graph** (**Event Blueprint Update Animation**), add **Switch on Enum** (your `Posture` variable). For each enum case, **Set** the matching alpha to `1.0` and the other five to `0.0`.
   - In **Anim Graph**, connect each of these 6 variables to the **Alpha** input of the corresponding blend pin. So: enum → 6 alphas → 6 blend pins; only the selected posture has alpha 1.

   **If you used Path B (1 Pose Asset):**
   - Right-click **Layered Blend per Bone** → **Add Blend Pin** once.
   - Drag your **Pose Asset** into the Anim Graph to create a **Pose Blender**. Connect **Pose Blender** output → the single **Blend Pose** input.
   - Add a **Modify Curve** node. Add 6 curve pins; name them exactly like the poses in the Pose Asset (e.g. `Posture_Up`, `Posture_Down`, …). Feed the **Modify Curve** output (or the curve context) into the **Pose Blender** so it uses these curves. In the Anim Graph, use **Switch on Enum** (Posture) and for each case output six floats (1.0 for the selected pose, 0.0 for others) and connect them to the **Modify Curve** curve value inputs. (If your version uses a different way to feed curves into Pose Blender, use the same idea: enum → one curve = 1, rest = 0.)
   - Set this single blend pin’s **Alpha** to `1.0` when `Posture != Neutral`, and `0.0` when `Posture == Neutral` (so in Neutral you see only base Idle/Walk).

4. **Compile and test**
   - Assign the Animation Blueprint to your character’s **Skeletal Mesh** (Details → Anim Class / Animation Blueprint).
   - In preview or PIE, change `ActualPosture` (or the `Posture` variable if Instance Editable) and confirm: base stays Idle/Walk, only shoulders/spine change with the enum.

---

## Summary

- **Idea**: Enum selects one of 6 upper-body postures; that posture is **mixed** with Idle/Walk so only the upper body changes.
- **Flow**: Character `ActualPosture` → Animation Blueprint variable → Anim Graph uses it to set **Layered Blend per Bone** alphas (Path A) or **Modify Curve** → Pose Blender (Path B). Base pose = State Machine (Idle/Walk). Posture overlay = 6 sequences (Path A) or 1 Pose Asset with 6 poses (Path B). **Branch Filter** (or Blend Mask) limits the overlay to upper body only.
- **Creating the 6 poses**: Path A = 6 duplicated Idle sequences, each edited in **Animation Sequence Editor** (Bone Manipulation + **Add Key**). Path B = one **Pose Asset** with 6 named poses, created from Skeleton Editor or from an Animation Sequence (**Create** → **Create Pose Asset**).

This is the full process from “I have an enum and want mixed animations” to a working setup in UE5.

---

## Documentation links (sources)

- **Animation Sequences:** [dev.epicgames.com/documentation/.../animation-sequences-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-sequences-in-unreal-engine)
- **Animation Sequence Editor:** [dev.epicgames.com/documentation/.../animation-sequence-editor-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-sequence-editor-in-unreal-engine)
- **Animation Blueprints:** [dev.epicgames.com/documentation/.../animation-blueprints-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprints-in-unreal-engine)
- **Layered Blend per Bone (API):** [FAnimNode_LayeredBoneBlend](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/AnimGraphRuntime/FAnimNode_LayeredBoneBlend)
- **Blend Masks and Blend Profiles:** [dev.epicgames.com/documentation/.../blend-masks-and-blend-profiles-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/blend-masks-and-blend-profiles-in-unreal-engine)
- **Animation Pose Assets:** [dev.epicgames.com/documentation/.../animation-pose-assets-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-pose-assets-in-unreal-engine)
- **Creating a Pose Asset:** [dev.epicgames.com/documentation/.../creating-a-pose-asset-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-a-pose-asset-in-unreal-engine)
- **Pose Blender:** [dev.epicgames.com/documentation/.../pose-blender-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/pose-blender-in-unreal-engine)
- **Animation Blueprint Modify Curve:** [dev.epicgames.com/documentation/.../animation-blueprint-modify-curve-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-modify-curve-in-unreal-engine)
- **Curve Driven Animation:** [dev.epicgames.com/documentation/.../curve-driven-animation-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/curve-driven-animation-in-unreal-engine)

*Last updated: 2026-01-27*
