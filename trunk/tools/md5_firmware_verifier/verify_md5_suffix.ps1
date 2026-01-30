<#
.SYNOPSIS
  Validate filename MD5-sampled suffix (4 bytes / 8 hex) for *.bin files.

.DESCRIPTION
  This script verifies files named like:
    *_<8hex>_(APP|BOOT|OTA|RES).bin
  It extracts the trailing 8-hex token from the filename and compares it with
  the actual value computed from file content.

  Sampling rule (must match build_helper.sh):
    - Compute MD5 digest (16 bytes)
    - Fold whole MD5 into 4 bytes via XOR per byte:
        out[i] = md5[i] ^ md5[i+4] ^ md5[i+8] ^ md5[i+12]  (i=0..3)
    - Output 4 bytes as 8 hex (lowercase)

.USAGE
  PowerShell:
    .\verify_md5_suffix.ps1 -Path "D:\some\folder"
    .\verify_md5_suffix.ps1 -Path "D:\some\file_APP.bin"

  Double-click / Explorer: right-click "Run with PowerShell" (may need policy).
#>

[CmdletBinding()]
param(
  [Parameter(Position = 0)]
  [string]$Path = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$Script:LogPath = $null
try {
  $Script:LogPath = Join-Path -Path $PSScriptRoot -ChildPath "verify_md5_suffix.log"
} catch {
  try { $Script:LogPath = Join-Path -Path $env:TEMP -ChildPath "verify_md5_suffix.log" } catch { }
}

function Write-Log {
  param([Parameter(Mandatory = $true)][string]$Message)
  try {
    if (-not $Script:LogPath) { return }
    $ts = (Get-Date).ToString("yyyy-MM-dd HH:mm:ss.fff")
    Add-Content -LiteralPath $Script:LogPath -Value ("[$ts] $Message")
  } catch { }
}

Write-Log "Script loaded. PSVersion=$($PSVersionTable.PSVersion) PSEdition=$($PSVersionTable.PSEdition) PathParam='$Path'"
try { [Console]::Out.WriteLine("verify_md5_suffix.ps1 started. PSVersion=$($PSVersionTable.PSVersion) PSEdition=$($PSVersionTable.PSEdition)") } catch { }

function Show-ErrorDialog {
  param([Parameter(Mandatory = $true)][string]$Message)
  try {
    Add-Type -AssemblyName System.Windows.Forms | Out-Null
    [void][System.Windows.Forms.MessageBox]::Show(
      $Message,
      "MD5 Suffix Verifier - Error",
      [System.Windows.Forms.MessageBoxButtons]::OK,
      [System.Windows.Forms.MessageBoxIcon]::Error
    )
  } catch {
    # ignore (no GUI available)
  }
}

function Get-ExpectedSuffixFromName {
  param([Parameter(Mandatory = $true)][string]$FileName)
  $m = [regex]::Match($FileName, '(?i)([0-9a-f]{8})_(APP|BOOT|OTA|RES)\.bin$')
  if (-not $m.Success) { return $null }
  return $m.Groups[1].Value.ToLowerInvariant()
}

function Get-Md5DigestBytes {
  param([Parameter(Mandatory = $true)][string]$FilePath)
  $md5 = [System.Security.Cryptography.MD5]::Create()
  try {
    $fs = [System.IO.File]::OpenRead($FilePath)
    try { return $md5.ComputeHash($fs) }
    finally { $fs.Dispose() }
  } finally {
    $md5.Dispose()
  }
}

function Get-ActualSampledSuffix {
  param([Parameter(Mandatory = $true)][string]$FilePath)
  $bytes = Get-Md5DigestBytes -FilePath $FilePath
  if ($bytes.Length -ne 16) { throw "MD5 digest length unexpected: $($bytes.Length)" }

  $o0 = ([int]$bytes[0])  -bxor ([int]$bytes[4])  -bxor ([int]$bytes[8])  -bxor ([int]$bytes[12])
  $o1 = ([int]$bytes[1])  -bxor ([int]$bytes[5])  -bxor ([int]$bytes[9])  -bxor ([int]$bytes[13])
  $o2 = ([int]$bytes[2])  -bxor ([int]$bytes[6])  -bxor ([int]$bytes[10]) -bxor ([int]$bytes[14])
  $o3 = ([int]$bytes[3])  -bxor ([int]$bytes[7])  -bxor ([int]$bytes[11]) -bxor ([int]$bytes[15])

  return ("{0:x2}{1:x2}{2:x2}{3:x2}" -f $o0, $o1, $o2, $o3)
}

function Get-TargetFiles {
  param([Parameter(Mandatory = $true)][string]$InputPath)
  if (Test-Path -LiteralPath $InputPath -PathType Leaf) {
    return ,(Get-Item -LiteralPath $InputPath)
  }
  if (Test-Path -LiteralPath $InputPath -PathType Container) {
    return Get-ChildItem -LiteralPath $InputPath -Recurse -File -Filter *.bin
  }
  throw "Path not found: $InputPath"
}

function New-ResultRow {
  param(
    [Parameter(Mandatory = $true)][string]$FilePath,
    [string]$Expected,
    [string]$Actual,
    [Parameter(Mandatory = $true)][string]$Status,
    [string]$Reason
  )
  return [pscustomobject]@{
    File    = $FilePath
    Expected = $Expected
    Actual   = $Actual
    Status   = $Status
    Reason   = $Reason
  }
}

function SafeStr {
  param([Parameter(ValueFromPipeline = $true)][object]$Value)
  if ($null -eq $Value) { return "" }
  $s = [string]$Value
  # Remove NUL which can break WinForms bindings
  return $s -replace "`0", ""
}

function New-ResultsDataTable {
  throw "Deprecated: DataTable binding disabled for maximum compatibility."
}

function Invoke-Verification {
  param([Parameter(Mandatory = $true)][string]$TargetPath)

  try {
    $files = Get-TargetFiles -InputPath $TargetPath
    $results = New-Object System.Collections.Generic.List[object]

    foreach ($f in $files) {
      $expected = Get-ExpectedSuffixFromName -FileName $f.Name
      if (-not $expected) {
        $results.Add((New-ResultRow -FilePath (SafeStr $f.FullName) -Expected "" -Actual "" -Status "FAIL" -Reason "No 8-hex suffix before _APP/_BOOT/_OTA/_RES"))
        continue
      }

      try {
        $actual = Get-ActualSampledSuffix -FilePath $f.FullName
        $status = if ($actual -eq $expected) { "PASS" } else { "FAIL" }
        $reason = if ($actual -eq $expected) { "" } else { "Mismatch" }
      } catch {
        $actual = ""
        $status = "FAIL"
        $reason = SafeStr $_.Exception.Message
      }

      $results.Add((New-ResultRow -FilePath (SafeStr $f.FullName) -Expected (SafeStr $expected) -Actual (SafeStr $actual) -Status $status -Reason (SafeStr $reason)))
    }

    $passCount = @($results | Where-Object { $_.Status -eq "PASS" }).Count
    $failCount = @($results | Where-Object { $_.Status -eq "FAIL" }).Count
    $totalCount = $results.Count

    return [pscustomobject]@{
      Results   = $results
      Pass      = $passCount
      Fail      = $failCount
      Total     = $totalCount
    }
  } catch {
    $pos = ""
    try { $pos = $_.InvocationInfo.PositionMessage } catch { }
    $stk = ""
    try { $stk = $_.ScriptStackTrace } catch { }
    throw ("Invoke-Verification failed. " + (SafeStr $_.Exception.Message) + "`n" + (SafeStr $pos) + "`n" + (SafeStr $stk) + "`n" +
      "Tip: If this is a factory PC, ensure Windows PowerShell 5.1 has .NET Framework/System.Data available.")
  }
}

try {
  Add-Type -AssemblyName System.Windows.Forms | Out-Null
  Add-Type -AssemblyName System.Drawing | Out-Null
  [System.Windows.Forms.Application]::EnableVisualStyles()
  [System.Windows.Forms.Application]::SetCompatibleTextRenderingDefault($false)
  Write-Log "GUI start. ApartmentState=$([System.Threading.Thread]::CurrentThread.ApartmentState). InitialPath='$Path'"
  try { [Console]::Out.WriteLine("GUI start. ApartmentState=$([System.Threading.Thread]::CurrentThread.ApartmentState). InitialPath='$Path'") } catch { }

  $form = New-Object System.Windows.Forms.Form
  $form.Text = "MD5 Suffix Verifier (PASS/FAIL)"
  $form.Width = 1200
  $form.Height = 650
  $form.StartPosition = "CenterScreen"

  # Layout: avoid Dock overlap issues (top row + grid row)
  $layout = New-Object System.Windows.Forms.TableLayoutPanel
  $layout.Dock = "Fill"
  $layout.RowCount = 2
  $layout.ColumnCount = 1
  $layout.Padding = 0
  $layout.Margin = 0
  [void]$layout.ColumnStyles.Add((New-Object System.Windows.Forms.ColumnStyle([System.Windows.Forms.SizeType]::Percent, 100)))
  [void]$layout.RowStyles.Add((New-Object System.Windows.Forms.RowStyle([System.Windows.Forms.SizeType]::Absolute, 40)))
  [void]$layout.RowStyles.Add((New-Object System.Windows.Forms.RowStyle([System.Windows.Forms.SizeType]::Percent, 100)))
  $form.Controls.Add($layout)

  $topPanel = New-Object System.Windows.Forms.Panel
  $topPanel.Dock = "Fill"
  $topPanel.Margin = 0
  $layout.Controls.Add($topPanel, 0, 0)

  $btnPick = New-Object System.Windows.Forms.Button
  $btnPick.Text = "Select folder..."
  $btnPick.Width = 90
  $btnPick.Left = 10
  $btnPick.Top = 8
  $topPanel.Controls.Add($btnPick)

  $txtPath = New-Object System.Windows.Forms.TextBox
  $txtPath.Left = 110
  $txtPath.Top = 10
  $txtPath.Width = 720
  $txtPath.ReadOnly = $false
  if (-not [string]::IsNullOrWhiteSpace($Path)) {
    $txtPath.Text = $Path
  }
  $topPanel.Controls.Add($txtPath)

  $btnVerify = New-Object System.Windows.Forms.Button
  $btnVerify.Text = "Verify"
  $btnVerify.Width = 70
  $btnVerify.Left = 840
  $btnVerify.Top = 8
  $topPanel.Controls.Add($btnVerify)

  $lblSummary = New-Object System.Windows.Forms.Label
  $lblSummary.Left = 920
  $lblSummary.Top = 12
  $lblSummary.AutoSize = $true
  $lblSummary.Text = "PASS: 0  FAIL: 0  TOTAL: 0"
  $topPanel.Controls.Add($lblSummary)

  $grid = New-Object System.Windows.Forms.DataGridView
  $grid.Dock = "Fill"
  $grid.ReadOnly = $true
  $grid.AllowUserToAddRows = $false
  $grid.AllowUserToDeleteRows = $false
  $grid.AutoGenerateColumns = $false
  $grid.AutoSizeColumnsMode = "None"
  $grid.ScrollBars = "Both"
  $grid.RowHeadersVisible = $false
  $grid.SelectionMode = "FullRowSelect"
  $grid.AutoSizeRowsMode = "None"
  $grid.DefaultCellStyle.WrapMode = [System.Windows.Forms.DataGridViewTriState]::False
  $grid.ShowCellToolTips = $true
  $grid.Margin = 0
  $layout.Controls.Add($grid, 0, 1)

  # Define columns explicitly (avoid DataTable dependency)
  $colFile = New-Object System.Windows.Forms.DataGridViewTextBoxColumn
  $colFile.Name = "File"
  $colFile.HeaderText = "File"
  $colFile.AutoSizeMode = [System.Windows.Forms.DataGridViewAutoSizeColumnMode]::None
  $colFile.Width = 820
  $colFile.MinimumWidth = 520

  $colExpected = New-Object System.Windows.Forms.DataGridViewTextBoxColumn
  $colExpected.Name = "Expected"
  $colExpected.HeaderText = "Expected"
  $colExpected.AutoSizeMode = [System.Windows.Forms.DataGridViewAutoSizeColumnMode]::AllCells

  $colActual = New-Object System.Windows.Forms.DataGridViewTextBoxColumn
  $colActual.Name = "Actual"
  $colActual.HeaderText = "Actual"
  $colActual.AutoSizeMode = [System.Windows.Forms.DataGridViewAutoSizeColumnMode]::AllCells

  $colStatus = New-Object System.Windows.Forms.DataGridViewTextBoxColumn
  $colStatus.Name = "Status"
  $colStatus.HeaderText = "Status"
  $colStatus.AutoSizeMode = [System.Windows.Forms.DataGridViewAutoSizeColumnMode]::AllCells

  $colReason = New-Object System.Windows.Forms.DataGridViewTextBoxColumn
  $colReason.Name = "Reason"
  $colReason.HeaderText = "Reason"
  $colReason.AutoSizeMode = [System.Windows.Forms.DataGridViewAutoSizeColumnMode]::AllCells

  # PowerShell 5.1 may treat AddRange(@(...)) as object[] and fail conversion, so add columns one by one.
  [void]$grid.Columns.Add($colFile)
  [void]$grid.Columns.Add($colExpected)
  [void]$grid.Columns.Add($colActual)
  [void]$grid.Columns.Add($colStatus)
  [void]$grid.Columns.Add($colReason)

  $grid.add_CellToolTipTextNeeded({
    param($sender, $e)
    try {
      if ($e.RowIndex -ge 0 -and $e.ColumnIndex -ge 0) {
        $val = $sender.Rows[$e.RowIndex].Cells[$e.ColumnIndex].Value
        if ($null -ne $val) { $e.ToolTipText = [string]$val }
      }
    } catch { }
  })

  $grid.add_CellFormatting({
    param($sender, $e)
    if ($sender.Columns[$e.ColumnIndex].Name -eq "Status") {
      $val = $e.Value
      if ($val -eq "PASS") {
        $e.CellStyle.BackColor = [System.Drawing.Color]::FromArgb(198, 239, 206)
        $e.CellStyle.ForeColor = [System.Drawing.Color]::FromArgb(0, 97, 0)
      } elseif ($val -eq "FAIL") {
        $e.CellStyle.BackColor = [System.Drawing.Color]::FromArgb(255, 199, 206)
        $e.CellStyle.ForeColor = [System.Drawing.Color]::FromArgb(156, 0, 6)
      }
    }
  })

  # Initial empty grid
  $grid.Rows.Clear()

  $btnPick.add_Click({
    $dlg = New-Object System.Windows.Forms.FolderBrowserDialog
    $dlg.Description = "Select folder to verify"
    if (-not [string]::IsNullOrWhiteSpace($txtPath.Text)) {
      $dlg.SelectedPath = $txtPath.Text
    }
    if ($dlg.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) {
      $txtPath.Text = $dlg.SelectedPath
    }
  })

  $btnVerify.add_Click({
    $target = $txtPath.Text
    if ([string]::IsNullOrWhiteSpace($target)) {
      Show-ErrorDialog -Message "Please select a folder to verify first."
      return
    }
    try {
      $btnVerify.Enabled = $false
      $btnPick.Enabled = $false
      $form.Cursor = [System.Windows.Forms.Cursors]::WaitCursor
      $lblSummary.Text = "Verifying..."
      [System.Windows.Forms.Application]::DoEvents()

      Write-Log "Verify start. Target='$target'"
      $r = Invoke-Verification -TargetPath $target
      Write-Log "Verify done. Pass=$($r.Pass) Fail=$($r.Fail) Total=$($r.Total)"
      $grid.Rows.Clear()
      foreach ($it in $r.Results) {
        [void]$grid.Rows.Add(@(
          (SafeStr $it.File),
          (SafeStr $it.Expected),
          (SafeStr $it.Actual),
          (SafeStr $it.Status),
          (SafeStr $it.Reason)
        ))
      }
      $grid.Refresh()
      $grid.ClearSelection()
      if ($grid.Rows.Count -gt 0) { $grid.FirstDisplayedScrollingRowIndex = 0 }
      $lblSummary.Text = ("PASS: {0}  FAIL: {1}  TOTAL: {2}" -f $r.Pass, $r.Fail, $r.Total)
    } catch {
      $err = SafeStr $_.Exception.ToString()
      $pos = ""
      try { $pos = SafeStr $_.InvocationInfo.PositionMessage } catch { }
      $stk = ""
      try { $stk = SafeStr $_.ScriptStackTrace } catch { }
      Write-Log ("Verify error: " + $err)
      if ($pos) { Write-Log ("Verify error position: " + $pos) }
      if ($stk) { Write-Log ("Verify error stack: " + $stk) }
      Show-ErrorDialog -Message ($err + "`n" + $pos)
      $lblSummary.Text = "Verify failed"
    } finally {
      $form.Cursor = [System.Windows.Forms.Cursors]::Default
      $btnVerify.Enabled = $true
      $btnPick.Enabled = $true
    }
  })

  [void]$form.ShowDialog()
} catch {
  $msg = $_.Exception.ToString()
  Write-Log ("Fatal error: " + $msg)
  try { [Console]::Error.WriteLine($msg) } catch { }
  Show-ErrorDialog -Message $msg
  exit 1
}

