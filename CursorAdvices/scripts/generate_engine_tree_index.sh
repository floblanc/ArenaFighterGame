#!/usr/bin/env bash
# Generate a local-only directory index of an Unreal Engine install.
# Navigation aid only — open real headers for API truth.
#
# Usage:
#   ./CursorAdvices/scripts/generate_engine_tree_index.sh
#   ./CursorAdvices/scripts/generate_engine_tree_index.sh "/path/to/UE_5.8"
#   ENGINE_ROOT="E:/Epic Games/UE_5.8" ./CursorAdvices/scripts/generate_engine_tree_index.sh

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT_DIR="$ROOT/CursorAdvices/local"
OUT_FILE="$OUT_DIR/UE58_Engine_Tree_Index.md"

ENGINE_ROOT_DEFAULT=""
if [[ -d "/e/Epic Games/UE_5.8" ]]; then
  ENGINE_ROOT_DEFAULT="/e/Epic Games/UE_5.8"
elif [[ -d "E:/Epic Games/UE_5.8" ]]; then
  ENGINE_ROOT_DEFAULT="E:/Epic Games/UE_5.8"
elif [[ -d "/Users/Shared/Epic Games/UE_5.8" ]]; then
  ENGINE_ROOT_DEFAULT="/Users/Shared/Epic Games/UE_5.8"
fi

ENGINE_ROOT="${1:-${ENGINE_ROOT:-$ENGINE_ROOT_DEFAULT}}"

if [[ -z "$ENGINE_ROOT" || ! -d "$ENGINE_ROOT" ]]; then
  echo "Engine root not found. Pass the UE install path as arg 1 or set ENGINE_ROOT." >&2
  echo "Example: $0 \"E:/Epic Games/UE_5.8\"" >&2
  exit 1
fi

mkdir -p "$OUT_DIR"

{
  echo "# UE engine tree index (local, generated)"
  echo
  echo "Generated: $(date -u +%Y-%m-%dT%H:%M:%SZ) UTC"
  echo
  echo "Engine root: \`$ENGINE_ROOT\`"
  echo
  echo "> Navigation only. Open the actual file for signatures/behavior. See \`UE_Verification_Rules.md\`."
  echo
  echo '```'
  # Prefer Source + useful config; skip huge Binary/Derived caches if present under install
  if command -v rg >/dev/null 2>&1; then
    (cd "$ENGINE_ROOT" && find Engine/Source Engine/Shaders Engine/Config Engine/Plugins \
      -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.cpp' -o -name '*.cs' -o -name '*.usf' -o -name '*.ush' -o -name '*.ini' \) \
      2>/dev/null | sort)
  else
    (cd "$ENGINE_ROOT" && find Engine/Source Engine/Shaders Engine/Config Engine/Plugins \
      -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.cpp' -o -name '*.cs' -o -name '*.usf' -o -name '*.ush' -o -name '*.ini' \) \
      2>/dev/null | sort)
  fi
  echo '```'
} > "$OUT_FILE"

echo "Wrote $OUT_FILE ($(wc -l < "$OUT_FILE") lines)"
