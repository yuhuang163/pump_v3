@echo off
setlocal EnableExtensions EnableDelayedExpansion

REM GUI verifier launcher for Windows
REM Usage:
REM   verify_md5_suffix_gui.bat "D:\path\to\folder"
REM   verify_md5_suffix_gui.bat "D:\path\to\file_APP.bin"
REM If omitted, uses current directory.

set "SCRIPT_DIR=%~dp0"
set "TARGET=%~1"
if "%TARGET%"=="" set "TARGET="

REM Launcher log (helps diagnose "flash exit")
set "LAUNCH_LOG=%SCRIPT_DIR%verify_md5_suffix_launcher.log"
>>"%LAUNCH_LOG%" echo [%date% %time%] launch. script="%SCRIPT_DIR%verify_md5_suffix.ps1" target="%TARGET%"

set "VBS=%SCRIPT_DIR%verify_md5_suffix_gui.vbs"
set "WSCRIPT=%SystemRoot%\System32\wscript.exe"

if exist "%VBS%" (
  >>"%LAUNCH_LOG%" echo [%date% %time%] vbs found: "%VBS%"
  >>"%LAUNCH_LOG%" echo [%date% %time%] running wscript: "%WSCRIPT%"
  "%WSCRIPT%" //nologo "%VBS%" "%SCRIPT_DIR%verify_md5_suffix.ps1" "%TARGET%" "%LAUNCH_LOG%"
  set "RC=!errorlevel!"
  >>"%LAUNCH_LOG%" echo [%date% %time%] wscript done. exitCode=!RC!
) else (
  >>"%LAUNCH_LOG%" echo [%date% %time%] ERROR: vbs not found: "%VBS%"
  >>"%LAUNCH_LOG%" echo [%date% %time%] fallback to powershell direct start
  "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -STA -WindowStyle Hidden -File "%SCRIPT_DIR%verify_md5_suffix.ps1" -Path "%TARGET%" >> "%LAUNCH_LOG%" 2>&1
  set "RC=!errorlevel!"
  >>"%LAUNCH_LOG%" echo [%date% %time%] powershell done. exitCode=!RC!
)

endlocal
exit /b 0

