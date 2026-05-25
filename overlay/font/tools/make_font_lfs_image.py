#!/usr/bin/env python3
"""Create a littlefs image containing per-size LVGL font packages."""

from __future__ import annotations

import argparse
from pathlib import Path

try:
    from littlefs import LittleFS
    from littlefs.context import UserContextFile
except ImportError as exc:  # pragma: no cover - host dependency check
    raise SystemExit(
        "Missing dependency: littlefs-python. Install it with:\n"
        "  python3 -m pip install --user littlefs-python"
    ) from exc


def parse_entry(value: str) -> tuple[int, int, Path]:
    parts = value.split(":", 2)
    if len(parts) != 3:
        raise argparse.ArgumentTypeError("entry format must be <font_size>:<db_id>:<package_path>")
    font_size = int(parts[0], 0)
    db_id = int(parts[1], 0)
    package_path = Path(parts[2])
    if not package_path.is_file():
        raise argparse.ArgumentTypeError(f"package file does not exist: {package_path}")
    return font_size, db_id, package_path


def create_image(
    output: Path,
    entries: list[tuple[int, int, Path]],
    block_size: int,
    block_count: int,
    read_size: int,
    prog_size: int,
    cache_size: int,
    lookahead_size: int,
    block_cycles: int,
) -> None:
    image_size = block_size * block_count
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(b"\xff" * image_size)

    ctx = UserContextFile(str(output), create=False)
    fs = LittleFS(
        context=ctx,
        block_size=block_size,
        block_count=block_count,
        read_size=read_size,
        prog_size=prog_size,
        block_cycles=block_cycles,
        cache_size=cache_size,
        lookahead_size=lookahead_size,
        mount=False,
    )
    fs.format()
    fs.mount()
    try:
        for font_size, db_id, package_path in entries:
            data = package_path.read_bytes()
            # APP_FDS_KEY_STRING_TYPE is 0, so integer keys are decimal file names.
            with fs.open(str(db_id), "wb") as fp:
                fp.write(data)
            print(f"  font_size={font_size}, db_id=0x{db_id:08X}, file={db_id}, size={len(data)}")
    finally:
        fs.unmount()


def main() -> int:
    parser = argparse.ArgumentParser(description="Create an external littlefs image for packed LVGL fonts.")
    parser.add_argument("-o", "--output", type=Path, required=True, help="output littlefs image")
    parser.add_argument("--entry", action="append", type=parse_entry, required=True, help="<font_size>:<db_id>:<package_path>")
    parser.add_argument("--block-size", type=lambda v: int(v, 0), default=4096)
    parser.add_argument("--block-count", type=lambda v: int(v, 0), default=256)
    parser.add_argument("--read-size", type=lambda v: int(v, 0), default=4)
    parser.add_argument("--prog-size", type=lambda v: int(v, 0), default=4)
    parser.add_argument("--cache-size", type=lambda v: int(v, 0), default=2048)
    parser.add_argument("--lookahead-size", type=lambda v: int(v, 0), default=16)
    parser.add_argument("--block-cycles", type=lambda v: int(v, 0), default=500)
    args = parser.parse_args()

    create_image(
        args.output,
        args.entry,
        args.block_size,
        args.block_count,
        args.read_size,
        args.prog_size,
        args.cache_size,
        args.lookahead_size,
        args.block_cycles,
    )
    print(f"FONT_LFS_IMAGE={args.output}")
    print(f"FONT_LFS_IMAGE_SIZE={args.output.stat().st_size}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
