<#
Rename frame images in the current folder with configurable rules (prefix/extension/padding/offset).

Example (your mapping: frame_0005.png -> charge_ant_00004.bin):
  powershell -ExecutionPolicy Bypass -File .\rename-frames.ps1 -SourcePrefix "frame_" -TargetPrefix "charge_ant_" -SourceExtension ".png" -TargetExtension ".bin" -Offset -1 -PadWidth 5

After preview looks correct, apply for real:
  powershell -ExecutionPolicy Bypass -File .\rename-frames.ps1 -Apply -SourcePrefix "frame_" -TargetPrefix "charge_ant_" -SourceExtension ".png" -TargetExtension ".bin" -Offset -1 -PadWidth 5

Notes:
  -Offset: targetIndex = sourceIndex + Offset  (0005 -> 0004 means Offset = -1)
  -PadWidth: number zero-padding width (charge_ant_00003.bin uses 5)
#>

[CmdletBinding()]
param(
  [string]$SourcePrefix = "frame_",
  [string]$SourceExtension = ".png",
  [string]$TargetPrefix = "charge_ant_",
  [string]$TargetExtension = ".bin",
  [int]$Offset = -1,
  [int]$PadWidth = 5,
  [switch]$Apply
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Normalize-Extension([string]$ext) {
  if ([string]::IsNullOrWhiteSpace($ext)) { return "" }
  if ($ext.StartsWith(".")) { return $ext }
  return "." + $ext
}

$SourceExtension = Normalize-Extension $SourceExtension
$TargetExtension = Normalize-Extension $TargetExtension

$srcRegex = "^" + [regex]::Escape($SourcePrefix) + "(?<n>\d+)" + [regex]::Escape($SourceExtension) + "$"

$items = Get-ChildItem -File | ForEach-Object {
  $name = $_.Name
  if ($name -match $srcRegex) {
    [pscustomobject]@{
      File = $_
      SourceNumber = [int]$Matches["n"]
    }
  }
} | Sort-Object SourceNumber

if (-not $items -or $items.Count -eq 0) {
  throw "No source files matched: prefix='$SourcePrefix' extension='$SourceExtension' (cwd: $(Get-Location))"
}

$plans = $items | ForEach-Object {
  $targetNumber = $_.SourceNumber + $Offset
  if ($targetNumber -lt 0) {
    throw "Computed negative target index: source=$($_.SourceNumber) offset=$Offset -> target=$targetNumber"
  }
  $numText = $targetNumber.ToString(("D" + $PadWidth))
  $targetName = "{0}{1}{2}" -f $TargetPrefix, $numText, $TargetExtension
  [pscustomobject]@{
    Source = $_.File.Name
    SourceFullPath = $_.File.FullName
    SourceNumber = $_.SourceNumber
    Target = $targetName
    TargetFullPath = (Join-Path -Path (Get-Location) -ChildPath $targetName)
  }
}

# Check duplicate targets
$dupes = $plans | Group-Object Target | Where-Object { $_.Count -gt 1 }
if ($dupes) {
  $msg = ($dupes | ForEach-Object { "'$($_.Name)' x$($_.Count)" }) -join ", "
  throw "Duplicate target filenames detected: $msg (check Offset/PadWidth/prefix settings)"
}

# Check collisions with existing files
$collisions = $plans | Where-Object { Test-Path -LiteralPath $_.TargetFullPath }
if ($collisions) {
  $examples = ($collisions | Select-Object -First 10 | ForEach-Object { $_.Target }) -join ", "
  throw "Target filename(s) already exist (would overwrite): e.g. $examples (total $($collisions.Count)). Move files or adjust params."
}

# Preview
$plans | Select-Object Source, Target | Format-Table -AutoSize | Out-String | Write-Host

if (-not $Apply) {
  Write-Host "`nPreview only (no changes). Re-run with -Apply to rename for real."
  exit 0
}

Write-Host "`nRenaming (two-phase to avoid conflicts)..."

$guid = [guid]::NewGuid().ToString("N")
$tempPrefix = "__tmp_rename_${guid}_"

# Phase 1: rename all to temp names
for ($i = 0; $i -lt $plans.Count; $i++) {
  $p = $plans[$i]
  $tempName = "{0}{1}.tmp" -f $tempPrefix, $i.ToString("D6")
  Rename-Item -LiteralPath $p.SourceFullPath -NewName $tempName
  $plans[$i] = $p | Select-Object *, @{Name="Temp";Expression={$tempName}}, @{Name="TempFullPath";Expression={(Join-Path (Get-Location) $tempName)}}
}

# Phase 2: temp -> final
for ($i = 0; $i -lt $plans.Count; $i++) {
  $p = $plans[$i]
  Rename-Item -LiteralPath $p.TempFullPath -NewName $p.Target
}

Write-Host "Done. Renamed $($plans.Count) files."
