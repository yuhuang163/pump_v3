"""BleFactoryTool entry point (source + PyInstaller frozen)."""

from __future__ import annotations

import sys
from pathlib import Path


def app_base_dir() -> Path:
    if getattr(sys, "frozen", False):
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parent


def main() -> None:
    # Ensure local imports work when frozen / run as script
    base = app_base_dir()
    if str(base) not in sys.path:
        sys.path.insert(0, str(base))

    from gui_app import run_app

    print(f"BleFactoryTool starting, base={base}")
    run_app()


if __name__ == "__main__":
    main()
