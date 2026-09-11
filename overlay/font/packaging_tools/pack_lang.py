#!/usr/bin/env python3
"""Pack UI language objects into FDS_EXT keys + a whole-pack container.

Each font role, page-string table, page-widget table and font_img is one
littlefs integer key. English Latin fonts stay in firmware (common) and
are not packed. A LANG container still supports whole-pack OTA.
"""

from __future__ import annotations

import argparse
import binascii
import hashlib
import importlib.util
import re
import struct
import sys
from datetime import datetime
from pathlib import Path

HEADER_STRUCT = struct.Struct("<I6H4I8I")
ENTRY_STRUCT = struct.Struct("<III BBH")
READY_STRUCT = struct.Struct("<IIIIIHHI")
PAGE_HDR_STRUCT = struct.Struct("<IHHII")
ITEM_STRUCT = struct.Struct("<HBBhhHBBbBHBBH")

MAGIC = 0x474E414C
READY_MAGIC = 0x31594452
PAGE_STR_MAGIC = 0x53524731
PAGE_WGT_MAGIC = 0x57475431
LANG_FDS_KEY = 0x4C414E47
READY_FDS_KEY = 0x52445931
SCHEMA_MAJOR = 2
SCHEMA_MINOR = 0

LANG_OBJ_FONT = 1
LANG_OBJ_IMG = 2
LANG_OBJ_PAGE_STR = 3
LANG_OBJ_PAGE_WGT = 4

LFS_BLOCK_SIZE = 4096
LFS_BLOCK_COUNT = 384
LFS_READ_SIZE = 4
LFS_PROG_SIZE = 4
LFS_CACHE_SIZE = 2048
LFS_LOOKAHEAD_SIZE = 16
LFS_BLOCK_CYCLES = 500
FDS_EXT_START_ADDR = 0xE00000

ROLE_HINT = 0
ROLE_BODY_SM = 1
ROLE_TITLE = 2
ROLE_TITLE_MD = 3
ROLE_TITLE_LG = 4

IMG_MODE_SELECT = 0
IMG_LEVEL_ADJUST = 1
IMG_PUMP_BREAK = 2
IMG_GUIDE_SKIP = 3
IMG_PUMP_RESUME = 4

# Language-specific fonts only. Latin/Aeonik and NUM live in firmware common.
FONT_FILES = {
    "zh": {
        ROLE_HINT: "lv_font_noto_sans_zh_19_medium.bin",
        ROLE_BODY_SM: "lv_font_noto_sans_zh_22_medium.bin",
        ROLE_TITLE: "lv_font_noto_sans_zh_26_medium.bin",
        ROLE_TITLE_MD: "lv_font_noto_sans_zh_29_medium.bin",
        ROLE_TITLE_LG: "lv_font_noto_sans_zh_33_medium.bin",
    },
    "en": {},
}

IMG_FILES = {
    IMG_MODE_SELECT: ("mode_select_tip.png", "mode_select_tip_220x63.txt"),
    IMG_LEVEL_ADJUST: ("level_adjust_tip.png", "level_adjust_tip_232x63.txt"),
    IMG_PUMP_BREAK: ("pump_break_tip.png", "pump_break_tip_220x63.txt"),
    IMG_GUIDE_SKIP: ("guide_skip_tip.png", "guide_skip_tip_220x63.txt"),
    IMG_PUMP_RESUME: ("pump_resume.png", "pump_resume_138x28.txt"),
}


def font_fds_key(role: int) -> int:
    return 0x464E5400 | (role & 0xFF)


def img_fds_key(img_id: int) -> int:
    return 0x46494D00 | (img_id & 0xFF)


def page_str_fds_key(page: int) -> int:
    return 0x50530000 | (page & 0xFFFF)


def page_wgt_fds_key(page: int) -> int:
    return 0x50570000 | (page & 0xFFFF)


def write_fds_ext_lfs_image(output: Path, files: dict[int, bytes]) -> None:
    try:
        from littlefs import LittleFS
        from littlefs.context import UserContextFile
    except ImportError as exc:
        raise SystemExit(
            "Missing dependency: littlefs-python. Install it with:\n"
            "  python3 -m pip install --user littlefs-python"
        ) from exc

    image_size = LFS_BLOCK_SIZE * LFS_BLOCK_COUNT
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(b"\xff" * image_size)

    ctx = UserContextFile(str(output), create=False)
    fs = LittleFS(
        context=ctx,
        block_size=LFS_BLOCK_SIZE,
        block_count=LFS_BLOCK_COUNT,
        read_size=LFS_READ_SIZE,
        prog_size=LFS_PROG_SIZE,
        block_cycles=LFS_BLOCK_CYCLES,
        cache_size=LFS_CACHE_SIZE,
        lookahead_size=LFS_LOOKAHEAD_SIZE,
        mount=False,
    )
    fs.format()
    fs.mount()
    try:
        for key, data in files.items():
            with fs.open(str(key), "wb") as fp:
                fp.write(data)
            print(f"  lfs file={key} (0x{key:08X}) size={len(data)}")
    finally:
        fs.unmount()


def load_c_hex_txt(path: Path) -> bytes:
    vals = re.findall(r"0x([0-9a-fA-F]{2})", path.read_text(encoding="utf-8"))
    return bytes(int(v, 16) for v in vals)


def load_font_img(img_dir: Path, png_name: str, txt_name: str) -> bytes | None:
    png = img_dir / "png" / png_name
    if png.is_file():
        return png.read_bytes()
    txt = img_dir / "txt" / txt_name
    if txt.is_file():
        data = load_c_hex_txt(txt)
        return data if data else None
    return None


def load_gen_module(repo: Path):
    path = repo / "overlay" / "ui" / "v3_ui_v7" / "lang" / "tools" / "gen_ui_lang_tables.py"
    spec = importlib.util.spec_from_file_location("gen_ui_lang_tables", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def unescape_c_string(raw: str) -> bytes:
    token = raw.strip()
    if token.startswith('"') and token.endswith('"'):
        token = token[1:-1]
    token = token.replace("\\n", "\n").replace("\\t", "\t").replace('\\"', '"').replace("\\\\", "\\")
    return token.encode("utf-8") + b"\x00"


def crc32_fw(data: bytes) -> int:
    return (binascii.crc32(data) ^ 0xFFFFFFFF) & 0xFFFFFFFF


def align4(n: int) -> int:
    return (n + 3) & ~3


def resolve_string(token: str, strings: dict[str, str]) -> bytes:
    for cand in (f"UI_STR_{token}", token):
        if cand in strings:
            raw = strings[cand]
            if raw in strings:
                return unescape_c_string(strings[raw])
            return unescape_c_string(raw)
    return b"\x00"


def widget_fields(gen, stem: str, resolved: dict[str, int], lay_keys) -> tuple[int, int, int, int, int, int]:
    kind = 1 if gen.widget_kind(stem) == "ICON" else 0
    x = int(resolved.get(f"{stem}_X", 0))
    y = int(resolved.get(f"{stem}_ALIGN_Y", resolved.get(f"{stem}_Y", 0)))
    line_space = int(resolved.get(f"{stem}_LINE_SPACE", 0))
    max_w = int(resolved.get(f"{stem}_MAX_W", 0))
    x = max(-32768, min(32767, x))
    y = max(-32768, min(32767, y))
    line_space = max(-128, min(127, line_space))
    max_w = max(0, min(65535, max_w))
    obj_align = 1 if f"{stem}_X" in lay_keys else 2
    text_align = 2 if kind == 0 else 0
    return kind, obj_align, x, y, line_space, max_w, text_align


def pack_item(item_id: int, kind: int, font_role: int, x: int, y: int, max_w: int,
              obj_align: int, text_align: int, line_space: int, text: bytes) -> bytes:
    text_bytes = len(text)
    raw = ITEM_STRUCT.pack(
        item_id, kind, font_role, x, y, max_w, obj_align, text_align,
        line_space, 255, 0, 3, 0, text_bytes,
    ) + text
    return raw + b"\x00" * (align4(len(raw)) - len(raw))


def build_page_file(magic: int, page_id: int, items: list[bytes]) -> bytes:
    payload = b"".join(items)
    body = PAGE_HDR_STRUCT.pack(magic, page_id, len(items), len(payload), 0) + payload
    crc = crc32_fw(body[16:])
    return PAGE_HDR_STRUCT.pack(magic, page_id, len(items), len(payload), crc) + payload


def format_file_version(version: int) -> str:
    return f"V{version // 100:02d}.{version % 100:02d}"


def main() -> int:
    parser = argparse.ArgumentParser(description="Pack UI language objects for FDS_EXT")
    parser.add_argument("--ui", choices=("v3", "v3pro"), default="v3pro")
    parser.add_argument("--lang", choices=("zh", "en"), default="zh")
    parser.add_argument("--font-root", type=Path, default=None)
    parser.add_argument("--font-dir", type=Path, default=None)
    parser.add_argument("--img-dir", type=Path, default=None)
    parser.add_argument("--product-id", type=lambda v: int(v, 0), default=None)
    parser.add_argument("--ui-version", type=lambda v: int(v, 0), default=0x07)
    parser.add_argument("--pack-version", type=lambda v: int(v, 0), default=1)
    parser.add_argument("--file-version", type=lambda v: int(v, 0), default=100)
    parser.add_argument("--out-dir", type=Path, default=None)
    parser.add_argument("--no-lfs-image", action="store_true")
    parser.add_argument("--no-pack-bin", action="store_true")
    args = parser.parse_args()

    if HEADER_STRUCT.size != 64:
        raise SystemExit(f"header size {HEADER_STRUCT.size} != 64")
    if ITEM_STRUCT.size != 20:
        raise SystemExit(f"item size {ITEM_STRUCT.size} != 20")

    tools_dir = Path(__file__).resolve().parent
    repo = tools_dir.parents[2]
    font_root = (args.font_root or (repo / "overlay" / "font" / "font_v3")).resolve()
    ui_name = "v3pro_ui_v7" if args.ui == "v3pro" else "v3_ui_v7"
    lang_dir = repo / "overlay" / "ui" / ui_name / "lang"
    gen = load_gen_module(repo)

    product_id = args.product_id if args.product_id is not None else (0x09 if args.ui == "v3pro" else 0x08)
    name_prefix = "V3PRO" if args.ui == "v3pro" else "V3"

    strings = gen.parse_defines(lang_dir / args.lang / "strings.h")
    layout = gen.parse_defines(lang_dir / args.lang / "layout.h")
    en_str = gen.parse_defines(lang_dir / "en" / "strings.h")
    zh_str = gen.parse_defines(lang_dir / "zh" / "strings.h")
    str_keys = gen.union_keys(en_str, zh_str)
    en_lay = gen.parse_defines(lang_dir / "en" / "layout.h")
    zh_lay = gen.parse_defines(lang_dir / "zh" / "layout.h")
    lay_keys = set(gen.union_keys(en_lay, zh_lay))
    resolved = gen.resolve_layout(layout)

    widgets: dict = {}
    scalars: list[str] = []
    for name in gen.union_keys(en_lay, zh_lay):
        classified = gen.classify_layout(name)
        if classified is None:
            continue
        kind, stem, field = classified
        if kind == "scalar":
            if name not in scalars:
                scalars.append(name)
            continue
        widgets.setdefault(stem, {})
        widgets[stem][field] = 0

    str_id = {key: i for i, key in enumerate(str_keys)}
    widget_id = {stem: i for i, stem in enumerate(widgets)}
    lay_id = {name: i for i, name in enumerate(scalars)}

    page_str_items: dict[int, list[bytes]] = {i: [] for i in range(len(gen.UI_PAGE_NAMES))}
    page_wgt_items: dict[int, list[bytes]] = {i: [] for i in range(len(gen.UI_PAGE_NAMES))}

    for key in str_keys:
        page = gen.page_id_of(key)
        stem = gen.str_widget_stem(key, widgets)
        if stem:
            kind, obj_align, x, y, line_space, max_w, text_align = widget_fields(
                gen, stem, resolved, lay_keys
            )
        else:
            kind, obj_align, x, y, line_space, max_w, text_align = 0, 2, 0, 0, 0, 0, 2
        token = key[len("UI_STR_") :] if key.startswith("UI_STR_") else key
        text = resolve_string(token, strings)
        page_str_items[page].append(
            pack_item(str_id[key], kind, 0xFF, x, y, max_w, obj_align, text_align, line_space, text)
        )

    for stem in widgets:
        page = gen.page_id_of(stem)
        kind, obj_align, x, y, line_space, max_w, text_align = widget_fields(
            gen, stem, resolved, lay_keys
        )
        page_wgt_items[page].append(
            pack_item(widget_id[stem], kind, 0xFF, x, y, max_w, obj_align, text_align, line_space, b"")
        )

    for name in scalars:
        page = gen.page_id_of(name)
        value = int(resolved.get(name, 0))
        value = max(-32768, min(32767, value))
        page_wgt_items[page].append(
            pack_item(lay_id[name], 2, 0xFF, value, 0, 0, 0, 0, 0, b"")
        )

    objects: list[tuple[int, int, int, bytes]] = []  # key, kind, id, data

    for page, items in page_str_items.items():
        if not items:
            continue
        objects.append((page_str_fds_key(page), LANG_OBJ_PAGE_STR, page,
                        build_page_file(PAGE_STR_MAGIC, page, items)))
    for page, items in page_wgt_items.items():
        if not items:
            continue
        objects.append((page_wgt_fds_key(page), LANG_OBJ_PAGE_WGT, page,
                        build_page_file(PAGE_WGT_MAGIC, page, items)))

    font_dir = (args.font_dir or (font_root / "lang" / args.lang / "bin")).resolve()
    if font_dir.is_dir():
        for role, name in FONT_FILES.get(args.lang, {}).items():
            path = font_dir / name
            if path.is_file():
                objects.append((font_fds_key(role), LANG_OBJ_FONT, role, path.read_bytes()))
            else:
                print(f"skip missing font {path}")
    else:
        print(f"skip font-dir missing {font_dir}")

    img_dir = (args.img_dir or (font_root / "lang" / args.lang / "font_img")).resolve()
    for img_id, (png_name, txt_name) in IMG_FILES.items():
        data = load_font_img(img_dir, png_name, txt_name)
        if data:
            objects.append((img_fds_key(img_id), LANG_OBJ_IMG, img_id, data))
        else:
            print(f"skip missing font_img {png_name} / {txt_name} in {img_dir}")

    entries = b"".join(
        ENTRY_STRUCT.pack(key, len(data), crc32_fw(data), kind, obj_id, 0)
        for key, kind, obj_id, data in objects
    )
    payload = entries + b"".join(data for _, _, _, data in objects)
    header = HEADER_STRUCT.pack(
        MAGIC,
        SCHEMA_MAJOR,
        SCHEMA_MINOR,
        product_id,
        args.ui_version,
        1 if args.lang == "zh" else 0,
        len(objects),
        args.pack_version,
        len(payload),
        crc32_fw(payload),
        HEADER_STRUCT.size + len(entries),
        *([0] * 8),
    )
    container = header + payload
    manifest = header + entries
    lang_id = 1 if args.lang == "zh" else 0

    ready_wo_crc = struct.pack(
        "<IIIIIHH",
        READY_MAGIC,
        MAGIC,
        args.pack_version,
        len(payload),
        crc32_fw(payload),
        lang_id,
        SCHEMA_MAJOR,
    )
    ready = READY_STRUCT.pack(
        READY_MAGIC,
        MAGIC,
        args.pack_version,
        len(payload),
        crc32_fw(payload),
        lang_id,
        SCHEMA_MAJOR,
        crc32_fw(ready_wo_crc),
    )

    lfs_files = {LANG_FDS_KEY: manifest, READY_FDS_KEY: ready}
    for key, _, _, data in objects:
        lfs_files[key] = data

    md5_short = hashlib.md5(container).hexdigest()[:8]
    date_str = datetime.now().strftime("%Y%m%d")
    ver_str = format_file_version(args.file_version)
    stem = f"{name_prefix}_{date_str}_{ver_str}_{md5_short}"

    out_dir = args.out_dir or (repo / "overlay" / "font" / "bin" / font_root.name / args.lang)
    out_dir.mkdir(parents=True, exist_ok=True)
    for pat in ("*_LANG.bin", "*_RDY.bin", "*_LFS.bin", "*_PACK.bin",
                "lang_pack.bin", "lang_ready.bin", "lang_pack_lfs.bin"):
        for stale in out_dir.glob(pat):
            stale.unlink()

    if not args.no_pack_bin:
        pack_named = out_dir / f"{stem}_PACK.bin"
        pack_named.write_bytes(container)
        print(f"wrote {pack_named} (OTA TYPE=0x04 container, {len(container)} bytes)")

    fonts = sum(1 for _, kind, _, _ in objects if kind == LANG_OBJ_FONT)
    imgs = sum(1 for _, kind, _, _ in objects if kind == LANG_OBJ_IMG)
    pages = sum(1 for _, kind, _, _ in objects if kind in (LANG_OBJ_PAGE_STR, LANG_OBJ_PAGE_WGT))
    print(f"container {len(container)} bytes, objects={len(objects)} pages={pages} fonts={fonts} imgs={imgs}")

    if not args.no_lfs_image:
        lfs_named = out_dir / f"{stem}_LFS.bin"
        write_fds_ext_lfs_image(lfs_named, lfs_files)
        print(f"wrote {lfs_named} ({lfs_named.stat().st_size} bytes)")
        print(
            f"flash FDS_EXT littlefs image at 0x{FDS_EXT_START_ADDR:06X}, "
            f"size 0x{LFS_BLOCK_SIZE * LFS_BLOCK_COUNT:X}"
        )
        print(f"LANG_LFS_OUTPUT={lfs_named}")

    print("FDS keys: LANG=manifest  RDY1=commit  FNT+role  FIM+id  PS+page  PW+page")
    return 0


if __name__ == "__main__":
    sys.exit(main())
