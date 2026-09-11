#!/usr/bin/env python3
"""Deprecated path. Forwards to overlay/font/packaging_tools/pack_lang.py."""

from __future__ import annotations

import os
import sys
from pathlib import Path

NEW = Path(__file__).resolve().parents[4] / "font" / "packaging_tools" / "pack_lang.py"
print(f"note: language pack tool moved to {NEW}", file=sys.stderr)
os.execv(sys.executable, [sys.executable, str(NEW), *sys.argv[1:]])
