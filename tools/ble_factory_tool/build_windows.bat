@echo off
setlocal
cd /d "%~dp0"

echo === BleFactoryTool Windows build ===
where python >nul 2>nul
if errorlevel 1 (
  echo ERROR: python not found in PATH
  exit /b 1
)

if not exist ".venv\Scripts\python.exe" (
  echo Creating venv...
  python -m venv .venv
)

call ".venv\Scripts\activate.bat"
python -m pip install --upgrade pip
pip install -r requirements.txt

echo Building exe...
pyinstaller --noconfirm ble_factory_tool.spec
if errorlevel 1 (
  echo BUILD FAILED
  exit /b 1
)

echo.
echo Output: %cd%\dist\BleFactoryTool.exe
echo Done.
endlocal
