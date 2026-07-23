"""Factory / Remote GATT UUIDs aligned with firmware services.

Supports both 16-bit and 128-bit UUID spellings.

Firmware currently declares 128-bit UUIDs as little-endian byte arrays.
Bleak expects UUID strings in standard (big-endian) form.

Factory (ROOT …02000x): drivers/ble/services/factory_service.h
Remote  (ROOT …03000x): drivers/ble/services/remot_service.h
"""

from __future__ import annotations

import re
from typing import Union

# Bluetooth SIG Base UUID for 16-bit expansion
_BT_BASE = "0000{short}-0000-1000-8000-00805f9b34fb"

# ---------------------------------------------------------------------------
# Factory (原始收发页)
# ---------------------------------------------------------------------------
FACTORY_SERVICE_UUID = "524f4f54-9000-0080-0010-000000020001"
FACTORY_TX_CHAR_UUID = "524f4f54-9000-0080-0010-000000020002"  # Notify
FACTORY_RX_CHAR_UUID = "524f4f54-9000-0080-0010-000000020003"  # Write / Write CMD

# ---------------------------------------------------------------------------
# Remote (图片资源 OTA 双向通道)
# ---------------------------------------------------------------------------
REMOTE_SERVICE_UUID = "524f4f54-9000-0080-0010-000000030001"
REMOTE_TX_CHAR_UUID = "524f4f54-9000-0080-0010-000000030002"  # Notify (Device→Host)
REMOTE_RX_CHAR_UUID = "524f4f54-9000-0080-0010-000000030003"  # Write (Host→Device)


def normalize_uuid(value: Union[str, int]) -> str:
    """Normalize 16-bit or 128-bit UUID to canonical 128-bit lowercase string.

    Accepted forms:
      - int: 0x180F
      - "180F", "0x180F", "180f"
      - "524f4f54-9000-0080-0010-000000020001"
      - "524f4f54900000800010000000020001" (no dashes)
    """
    if isinstance(value, int):
        if not 0 <= value <= 0xFFFF:
            raise ValueError(f"16-bit UUID out of range: {value:#x}")
        return _BT_BASE.format(short=f"{value:04x}")

    if value is None:
        raise ValueError("UUID is empty")

    text = str(value).strip().lower()
    if not text:
        raise ValueError("UUID is empty")

    text = text.replace(" ", "")
    if text.startswith("0x"):
        text = text[2:]
    text = text.replace("-", "")

    if not re.fullmatch(r"[0-9a-f]+", text):
        raise ValueError(f"invalid UUID hex: {value!r}")

    if len(text) == 4:
        return _BT_BASE.format(short=text)

    if len(text) == 32:
        return (
            f"{text[0:8]}-{text[8:12]}-{text[12:16]}-"
            f"{text[16:20]}-{text[20:32]}"
        )

    raise ValueError(
        f"UUID must be 16-bit (4 hex) or 128-bit (32 hex), got {len(text)} hex: {value!r}"
    )


def uuid_equal(a: Union[str, int], b: Union[str, int]) -> bool:
    """Compare two UUIDs after normalizing 16-bit / 128-bit forms."""
    try:
        return normalize_uuid(a) == normalize_uuid(b)
    except ValueError:
        return False


def uuid_in(needle: Union[str, int], haystack) -> bool:
    """True if needle matches any UUID in haystack (16/128-bit tolerant)."""
    for item in haystack:
        if uuid_equal(needle, item):
            return True
    return False


# Pre-normalized defaults for Bleak APIs
FACTORY_SERVICE_UUID_N = normalize_uuid(FACTORY_SERVICE_UUID)
FACTORY_TX_CHAR_UUID_N = normalize_uuid(FACTORY_TX_CHAR_UUID)
FACTORY_RX_CHAR_UUID_N = normalize_uuid(FACTORY_RX_CHAR_UUID)

REMOTE_SERVICE_UUID_N = normalize_uuid(REMOTE_SERVICE_UUID)
REMOTE_TX_CHAR_UUID_N = normalize_uuid(REMOTE_TX_CHAR_UUID)
REMOTE_RX_CHAR_UUID_N = normalize_uuid(REMOTE_RX_CHAR_UUID)
