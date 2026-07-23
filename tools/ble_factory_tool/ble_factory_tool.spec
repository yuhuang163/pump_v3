# -*- mode: python ; coding: utf-8 -*-
# PyInstaller spec for BleFactoryTool (Windows, onefile, console=True)
# Build on Windows only:  pyinstaller --noconfirm ble_factory_tool.spec

block_cipher = None

a = Analysis(
    ['main.py'],
    pathex=[],
    binaries=[],
    datas=[],
    hiddenimports=[
        'bleak',
        'bleak.assigned_numbers',
        'bleak.backends',
        'bleak.backends.winrt',
        'bleak.backends.winrt.client',
        'bleak.backends.winrt.scanner',
        'bleak.backends.winrt.characteristic',
        'bleak.backends.winrt.descriptor',
        'bleak.backends.winrt.service',
        'bleak.backends.winrt.utils',
        'winrt',
        'winrt.windows.devices.bluetooth',
        'winrt.windows.devices.bluetooth.advertisement',
        'winrt.windows.devices.bluetooth.genericattributeprofile',
        'winrt.windows.foundation',
        'winrt.windows.foundation.collections',
        'winrt.windows.storage.streams',
        'factory_uuids',
        'ble_worker',
        'gui_app',
        'comm_protocol',
        'ota_img_res_client',
    ],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='BleFactoryTool',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,  # GUI + debug console window
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
