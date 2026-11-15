# UE verification rules (accuracy over memory)

**Status: Current** — agents and humans must follow this for any Unreal engine claim.

LLMs are **not** a reliable encyclopedia of Unreal. Treat model knowledge as a hypothesis. **Cite a real source or say you are unsure.**

Pinned engine for this repo today: **UE 5.8**  
(`PurgatoriumLex.uproject` → `EngineAssociation: "5.8"`; Targets → `BuildSettingsVersion.V7` / `EngineIncludeOrderVersion.Unreal5_8`)

When you upgrade (5.9, 5.10, UE6…), update the pin + [UE5_8_Docs_Map.md](UE5_8_Docs_Map.md) (rename/replace for the new version) **before** trusting old notes.

---

## Authority ladder (highest wins)

| Rank | Source | Use for |
|------|--------|---------|
| 1 | **Installed engine headers / source** on your machine | Signatures, defaults, UHT behavior, exact includes |
| 2 | **Epic docs / API for that exact version** | Concepts, menus, supported workflows |
| 3 | **This repo `Source/` + Content** | Our architecture, property names, BP wiring |
| 4 | **CursorAdvices** | Project policy / checklists — only if they match 1–3 |
| 5 | Model memory / random web / old UE4 blogs | Never as sole authority |

Architecture that **intentionally diverges** from Epic (sim vs GAS, etc.) lives in [Architecture_Decisions.md](Architecture_Decisions.md) / `Combat/` — that overrides Epic *product patterns*, not Epic *API truth*.

---

## Cite or don’t claim

For every engine fact in an answer or a CursorAdvices edit, include at least one:

- Epic URL with the **correct version** (prefer `application_version=5.8` / 5.8-labelled pages today), **or**
- Engine path, e.g. `Engine/Source/Runtime/Engine/Classes/GameFramework/Pawn.h`, **or**
- Project path, e.g. `Source/PurgatoriumLex/Combat/CombatTypes.h:230`

If you cannot open a source, write **uncertain** and what to open. Do **not** invent Content Browser menu paths, Details labels, or API names.

---

## Local engine install (your PC)

Fill in / keep current:

| Machine | Typical path |
|---------|----------------|
| Windows | `E:\Epic Games\UE_5.8` |
| macOS | (set when you install; e.g. Epic Launcher UE_5.8 app bundle / shared Engine) |

**Cloud agents cannot read these disks.** For engine-header verification, use a **local Cursor agent** (or paste the header). Cloud agents may only use: this Git repo + Epic versioned docs + any index file you **commit** (prefer not to commit huge dumps).

### Optional local index (navigation only)

A tree listing helps *find* files; it is **not** proof of behavior.

1. Run [scripts/generate_engine_tree_index.sh](scripts/generate_engine_tree_index.sh) (or the `.ps1`) against your install.  
2. Output defaults to `CursorAdvices/local/UE58_Engine_Tree_Index.md` (gitignored).  
3. Agents: use the index to locate a header, then **open the header**.

Do **not** treat the index as architecture advice.

---

## Docs map

Versioned Epic hub index for this pin: [UE5_8_Docs_Map.md](UE5_8_Docs_Map.md)

After an upgrade:

1. Change `EngineAssociation` + Target include/build versions.  
2. Replace/rename the docs map for the new version; keep the old one under `deprecated/` if useful.  
3. Regenerate the local engine tree index.  
4. Smoke: compile `PurgatoriumLexEditor`, open `BP_PlayerCharacter`, fix broken pins.  
5. Re-verify [Blueprint_Modification_Checklist.md](Blueprint_Modification_Checklist.md) against the new editor (menus change).

---

## Task prompt habit (paste into agent requests)

```
Engine pin: UE 5.8 (see uproject).
Follow CursorAdvices/UE_Verification_Rules.md.
For engine facts: cite Epic 5.8 docs or installed engine headers — no memory-only claims.
Project architecture: Source/PurgatoriumLex/Combat wins over Epic combat patterns.
```

---

## Editor / Details labels

Details panel text comes from C++:

- Property name → spaced label (`LightAttackMoveSet` → **Light Attack Move Set**)
- `UMETA(DisplayName=…)` on enums (e.g. `NEUTRAL`, not `Neutral`)
- `Category = "Combat Sim|Posture"` → nested under **Combat Sim**

Prefer reading the `UPROPERTY` / `UFUNCTION` over any checklist wording if they disagree — then fix the checklist.
