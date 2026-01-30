Option Explicit

' Hidden launcher for verify_md5_suffix.ps1
' Usage:
'   wscript.exe //nologo verify_md5_suffix_gui.vbs <ps1Path> <targetPath> <logPath>

Dim args : Set args = WScript.Arguments
If args.Count < 3 Then
  WScript.Quit 2
End If

Dim ps1Path : ps1Path = args.Item(0)
Dim targetPath : targetPath = args.Item(1)
Dim logPath : logPath = args.Item(2)

Dim fso : Set fso = CreateObject("Scripting.FileSystemObject")

Sub LogLine(s)
  On Error Resume Next
  Dim t
  Set t = fso.OpenTextFile(logPath, 8, True) ' ForAppending
  If Not t Is Nothing Then t.WriteLine s
  If Not t Is Nothing Then t.Close
  On Error GoTo 0
End Sub

Dim shell : Set shell = CreateObject("WScript.Shell")

Dim cmd
cmd = """" & "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" & """" & _
      " -NoProfile -ExecutionPolicy Bypass -STA -WindowStyle Hidden -File " & _
      """" & ps1Path & """" & " -Path " & """" & targetPath & """"

LogLine "[" & Now & "] vbs start powershell: " & cmd

' Redirect output by executing through cmd.exe and appending to logPath
Dim cmd2
cmd2 = "cmd.exe /c " & """" & cmd & " >> " & """" & logPath & """" & " 2>&1" & """"
LogLine "[" & Now & "] vbs cmd: " & cmd2

Dim rc
rc = shell.Run(cmd2, 0, True) ' 0 = hidden, True = wait

LogLine "[" & Now & "] vbs done. exitCode=" & CStr(rc)

WScript.Quit rc

