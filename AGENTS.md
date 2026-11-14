# Agent instructions (PurgatoriumLex)

## Unreal accuracy

1. Read and follow **`CursorAdvices/UE_Verification_Rules.md`**.
2. Engine pin is whatever `PurgatoriumLex.uproject` → `EngineAssociation` says (currently **5.8**).
3. For **engine facts**: cite Epic docs for that version **or** installed engine headers. No memory-only API/menu claims.
4. For **combat architecture**: `Source/PurgatoriumLex/Combat/` and `CursorAdvices/Architecture_Decisions.md` win over Epic “use GAS” patterns.
5. Editor labels come from C++ `UPROPERTY` / `UMETA(DisplayName)` — prefer those over stale checklists; then fix the checklist.
6. Cloud agents **cannot** read `E:\Epic Games\…`. Use Epic versioned docs + this repo, or ask for a local agent / pasted header. Optional local index: `CursorAdvices/local/` (gitignored) via `CursorAdvices/scripts/generate_engine_tree_index.*`.

## Docs map

`CursorAdvices/UE5_8_Docs_Map.md` — Epic hub index for the current pin. Replace when upgrading the engine.
