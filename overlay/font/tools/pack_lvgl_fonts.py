#!/usr/bin/env python3
"""Wrap one LVGL binary font file with a small metadata header.

Package layout, little-endian:
    header: magic(u32), package_size(u32), package_crc32(u32)
    data: the original LVGL binary font
"""

from __future__ import annotations

import argparse
import binascii
import re
import struct
from dataclasses import dataclass
from pathlib import Path


MAGIC = 0x5046564C  # "LVFP" little-endian
HEADER_STRUCT = struct.Struct("<III")
DEFAULT_DB_ID = 0xAFFA5892
DEFAULT_MAX_SIZE = 0x100000

THIS_DIR = Path(__file__).resolve().parent
FONT_DIR = THIS_DIR.parent
DEFAULT_INPUT_DIR = FONT_DIR / "bin"
DEFAULT_OUTPUT = DEFAULT_INPUT_DIR / "lv_font_pack.bin"
DEFAULT_MANIFEST = THIS_DIR / "lv_font_pack_manifest.h"


@dataclass(frozen=True)
class FontFile:
    name: str
    path: Path
    data: bytes


def parse_int(value: str) -> int:
    return int(value, 0)


def c_identifier(value: str) -> str:
    text = re.sub(r"[^0-9A-Za-z_]", "_", value)
    text = re.sub(r"^lv_font_", "", text, flags=re.IGNORECASE)
    if not text or text[0].isdigit():
        text = "_" + text
    return text.upper()


def collect_fonts(input_dir: Path, pattern: str, keep_extension: bool) -> list[FontFile]:
    files = sorted(path for path in input_dir.glob(pattern) if path.is_file())
    if not files:
        raise SystemExit(f"No font files matched {input_dir / pattern}")

    fonts: list[FontFile] = []
    names: set[str] = set()
    for path in files:
        name = path.name if keep_extension else path.stem
        if name in names:
            raise SystemExit(f"Duplicate font package name: {name}")
        names.add(name)
        fonts.append(FontFile(name=name, path=path, data=path.read_bytes()))
    return fonts


def build_package(font: FontFile) -> bytes:
    package_size = HEADER_STRUCT.size + len(font.data)
    header = HEADER_STRUCT.pack(
        MAGIC,
        package_size,
        0,
    )
    package = header + font.data
    package_crc = binascii.crc32(package) & 0xFFFFFFFF
    return HEADER_STRUCT.pack(
        MAGIC,
        package_size,
        package_crc,
    ) + font.data


def write_manifest(
    path: Path,
    font: FontFile,
    db_id: int,
    max_size: int,
) -> None:
    lines = [
        "#ifndef __LV_FONT_PACK_MANIFEST_H__",
        "#define __LV_FONT_PACK_MANIFEST_H__",
        "",
        "#ifdef __cplusplus",
        'extern "C" {',
        "#endif",
        "",
        f"#define LV_FONT_PACK_DB_ID              (0x{db_id:08X}u)",
        f"#define LV_FONT_PACK_MAX_PACKAGE_SIZE   ({max_size}u)",
        f"#define LV_FONT_PACK_MAGIC              (0x{MAGIC:08X}u)",
        "",
    ]
    lines.append(f'#define FONT_NAME_{c_identifier(font.name)} "{font.name}"')
    lines.extend(
        [
            "",
            "#ifdef __cplusplus",
            "}",
            "#endif",
            "",
            "#endif",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Wrap one LVGL .bin font with metadata.")
    parser.add_argument("-i", "--input-dir", type=Path, default=DEFAULT_INPUT_DIR, help="directory containing .bin fonts")
    parser.add_argument("-o", "--output", type=Path, default=DEFAULT_OUTPUT, help="output package path")
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST, help="generated C config header")
    parser.add_argument("--pattern", default="*.bin", help="input glob pattern")
    parser.add_argument("--db-id", type=parse_int, default=DEFAULT_DB_ID, help="database key used by the demo code")
    parser.add_argument("--max-size", type=parse_int, default=DEFAULT_MAX_SIZE, help="maximum package size in bytes")
    parser.add_argument("--keep-extension", action="store_true", help="use full file names instead of stems as font names")
    args = parser.parse_args()

    input_dir = args.input_dir.resolve()
    fonts = collect_fonts(input_dir, args.pattern, args.keep_extension)
    if len(fonts) != 1:
        raise SystemExit(f"Expected exactly one font for one key, matched {len(fonts)} files")

    font = fonts[0]
    package = build_package(font)
    if len(package) > args.max_size:
        raise SystemExit(
            f"Package is {len(package)} bytes, larger than max size {args.max_size} bytes. "
            "Increase --max-size only if the partition has enough free space."
        )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(package)
    write_manifest(args.manifest, font, args.db_id, args.max_size)

    print(f"Packed {font.name} into {args.output} ({len(package)} bytes)")
    print(f"Manifest: {args.manifest}")
    print(f"Database key: 0x{args.db_id:08X}")
    print(f"  {font.name}: size={len(font.data)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
