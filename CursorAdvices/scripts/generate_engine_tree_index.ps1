# Generate a local-only directory index of an Unreal Engine install.
# Navigation aid only — open real headers for API truth.
#
# Usage:
#   .\CursorAdvices\scripts\generate_engine_tree_index.ps1
#   .\CursorAdvices\scripts\generate_engine_tree_index.ps1 -EngineRoot "E:\Epic Games\UE_5.8"

param(
    [string]$EngineRoot = $env:ENGINE_ROOT
)

$ErrorActionPreference = "Stop"
$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$OutDir = Join-Path $RepoRoot "CursorAdvices\local"
$OutFile = Join-Path $OutDir "UE58_Engine_Tree_Index.md"

if (-not $EngineRoot) {
    $candidates = @(
        "E:\Epic Games\UE_5.8",
        "C:\Program Files\Epic Games\UE_5.8"
    )
    foreach ($c in $candidates) {
        if (Test-Path $c) { $EngineRoot = $c; break }
    }
}

if (-not $EngineRoot -or -not (Test-Path $EngineRoot)) {
    Write-Error "Engine root not found. Pass -EngineRoot 'E:\Epic Games\UE_5.8'"
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$roots = @(
    (Join-Path $EngineRoot "Engine\Source"),
    (Join-Path $EngineRoot "Engine\Shaders"),
    (Join-Path $EngineRoot "Engine\Config"),
    (Join-Path $EngineRoot "Engine\Plugins")
) | Where-Object { Test-Path $_ }

$ext = @("*.h", "*.hpp", "*.cpp", "*.cs", "*.usf", "*.ush", "*.ini")
$files = foreach ($r in $roots) {
    Get-ChildItem -Path $r -Recurse -File -Include $ext -ErrorAction SilentlyContinue |
        ForEach-Object { $_.FullName.Substring($EngineRoot.Length).TrimStart("\", "/") }
}

$stamp = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
$lines = @(
    "# UE engine tree index (local, generated)",
    "",
    "Generated: $stamp UTC",
    "",
    "Engine root: ``$EngineRoot``",
    "",
    "> Navigation only. Open the actual file for signatures/behavior. See ``UE_Verification_Rules.md``.",
    "",
    "``````"
) + ($files | Sort-Object) + @("``````")

Set-Content -Path $OutFile -Value $lines -Encoding UTF8
Write-Host "Wrote $OutFile ($($lines.Count) lines)"
