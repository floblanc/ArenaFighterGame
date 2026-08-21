# UE5 basics — naming and UObject

Short reference for a first Unreal project when you already know C++.

---

## Naming prefixes (Epic convention)

| Prefix | Meaning | Examples in this project |
|--------|---------|---------------------------|
| `U` | `UObject`-derived | `UFighterCombatComponent`, `ULightAttackMoveSet`, `UAnimMontage` |
| `A` | `AActor`-derived (level / root transform) | `APlayerCharacter` |
| `F` | Plain struct/class or reflected `USTRUCT` value | `FFighterSimState`, `FVector`, `FFighterCombatSim` |
| `E` | Enum | `EPosture`, `ECombatMovePhase` |
| `I` | Interface | (none in Combat yet) |
| `T` | Template | `TArray`, `TObjectPtr`, `TWeakObjectPtr` |
| `b` | Bool member | `bLightAttackPressed` |

Functions: `PascalCase`. This is convention + UHT friendliness, not enforced by the C++ compiler alone.

---

## What is a `UObject`?

Base of almost everything Unreal **reflects**, **GCs**, **serializes** to assets/maps, exposes to **Blueprints**, or **replicates** the engine way.

| Concern | Typical C++ | Unreal UObject world |
|---------|-------------|----------------------|
| Create | `new` / stack | `NewObject`, `CreateDefaultSubobject`, load asset |
| Destroy | `delete` / RAII | Garbage collection (refs keep alive) |
| Know fields at runtime | Compiler only | UHT reflection (`UPROPERTY`, `UFUNCTION`) |
| Edit in editor | Custom tools | Details panel, Content Browser |

### When something should be a `UObject`

- Editor asset or component (`UDataAsset`, `UActorComponent`)  
- Blueprint access  
- Saved in `.uasset` / placed in a map  
- UE-style replication  

### When it should not

- Deterministic combat core you want to copy/hash/`TickFrame`  
- Math / POD (`FVector`, frame input)  
- Value semantics: `StateA = StateB`  

**This project’s split:**

- `UFighterCombatComponent`, `ULightAttackMoveSet` → UObject shells  
- `FFighterCombatSim`, `FFighterSimState` → rewind-friendly core  

---

## Pointers you’ll see in UE

| Type | Role |
|------|------|
| Raw `UObject*` | Possible, but lifetime easy to get wrong |
| `TObjectPtr<T>` | Preferred UPROPERTY object ref (editor + tracking) |
| `TWeakObjectPtr<T>` | Non-owning; OK if asset goes away (sim → move set) |
| `TSoftObjectPtr<T>` | Path to asset; load when needed (montage on move def) |

Combat **state** avoids storing Actor/UObject pointers as authority so rollback can restore a blob without the whole world graph.

---

## See also

- [CombatSim_Design.md](CombatSim_Design.md)  
- `Source/PurgatoriumLex/Combat/CombatSim_REVIEW.md`
