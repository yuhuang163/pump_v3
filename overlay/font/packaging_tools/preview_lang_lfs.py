#!/usr/bin/env python3
"""Preview a language-pack littlefs image or OTA container.

Usage:
  python3 preview_lang_lfs.py overlay/font/bin/font_v3/zh/V3PRO_..._LFS.bin
  python3 preview_lang_lfs.py ..._PACK.bin --json
  python3 preview_lang_lfs.py ..._LFS.bin -o /tmp/lang.html --open
"""

from __future__ import annotations

import argparse
import base64
import html
import json
import re
import shutil
import struct
import subprocess
import sys
import webbrowser
from collections import defaultdict
from pathlib import Path

import pack_lang as pack

KIND_NAMES = {
    pack.LANG_OBJ_FONT: "FONT",
    pack.LANG_OBJ_IMG: "IMG",
    pack.LANG_OBJ_PAGE_STR: "PAGE_STR",
    pack.LANG_OBJ_PAGE_WGT: "PAGE_WGT",
}
ITEM_KIND = {0: "TEXT", 1: "ICON", 2: "SCALAR"}
FONT_ROLES = {
    0: "HINT/19",
    1: "BODY_SM/22",
    2: "TITLE/26",
    3: "TITLE_MD/29",
    4: "TITLE_LG/33",
}
IMG_NAMES = {
    0: "mode_select_tip",
    1: "level_adjust_tip",
    2: "pump_break_tip",
    3: "guide_skip_tip",
    4: "pump_resume",
}
ALIGN_NAMES = {
    0: "DEFAULT",
    1: "TOP_LEFT",
    2: "TOP_MID",
    3: "TOP_RIGHT",
    4: "BOTTOM_LEFT",
    5: "BOTTOM_MID",
    6: "BOTTOM_RIGHT",
    7: "LEFT_MID",
    8: "RIGHT_MID",
    9: "CENTER",
}
SCREEN_W = 360
SCREEN_H = 360
ENUM_ITEM_RE = re.compile(r"^\s*(UI_[A-Z0-9_]+)(?:\s*=\s*(-?\d+))?\s*,")


def crc32_fw(data: bytes) -> int:
    return pack.crc32_fw(data)


def parse_c_enum(path: Path) -> dict[int, str]:
    out: dict[int, str] = {}
    if not path.is_file():
        return out
    next_v = 0
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = ENUM_ITEM_RE.match(line)
        if not m:
            continue
        name = m.group(1)
        if name.endswith("_COUNT"):
            continue
        if m.group(2) is not None:
            next_v = int(m.group(2))
        out[next_v] = name
        next_v += 1
    return out


def load_id_maps(repo: Path, ui: str) -> dict[str, dict[int, str]]:
    ui_name = "v3pro_ui_v7" if ui == "v3pro" else "v3_ui_v7"
    lang_dir = repo / "overlay" / "ui" / ui_name / "lang"
    pages = {i: name for i, name in enumerate(pack.load_gen_module(repo).UI_PAGE_NAMES)}
    return {
        "str": parse_c_enum(lang_dir / "ui_str_id.h"),
        "widget": parse_c_enum(lang_dir / "ui_widget_id.h"),
        "layout": parse_c_enum(lang_dir / "ui_layout_id.h"),
        "page": pages,
    }


def key_label(key: int, pages: dict[int, str]) -> str:
    if key == pack.LANG_FDS_KEY:
        return "LANG manifest"
    if key == pack.READY_FDS_KEY:
        return "RDY1 commit"
    if (key & 0xFFFFFF00) == 0x464E5400:
        role = key & 0xFF
        return f"FONT {FONT_ROLES.get(role, str(role))}"
    if (key & 0xFFFFFF00) == 0x46494D00:
        img_id = key & 0xFF
        return f"IMG {IMG_NAMES.get(img_id, str(img_id))}"
    if (key & 0xFFFF0000) == 0x50530000:
        page = key & 0xFFFF
        return f"PAGE_STR {pages.get(page, str(page))}"
    if (key & 0xFFFF0000) == 0x50570000:
        page = key & 0xFFFF
        return f"PAGE_WGT {pages.get(page, str(page))}"
    return f"0x{key:08X}"


def read_lfs_files(path: Path) -> dict[int, bytes]:
    try:
        from littlefs import LittleFS
        from littlefs.context import UserContextFile
    except ImportError as exc:
        raise SystemExit(
            "Missing dependency: littlefs-python. Install it with:\n"
            "  python3 -m pip install --user littlefs-python"
        ) from exc

    ctx = UserContextFile(str(path), create=False)
    fs = LittleFS(
        context=ctx,
        block_size=pack.LFS_BLOCK_SIZE,
        block_count=pack.LFS_BLOCK_COUNT,
        read_size=pack.LFS_READ_SIZE,
        prog_size=pack.LFS_PROG_SIZE,
        block_cycles=pack.LFS_BLOCK_CYCLES,
        cache_size=pack.LFS_CACHE_SIZE,
        lookahead_size=pack.LFS_LOOKAHEAD_SIZE,
        mount=False,
    )
    fs.mount()
    files: dict[int, bytes] = {}
    try:
        for name in fs.listdir("/"):
            try:
                key = int(name)
            except ValueError:
                continue
            with fs.open(name, "rb") as fp:
                files[key] = bytes(fp.read())
    finally:
        fs.unmount()
    return files


def parse_pack_container(data: bytes) -> dict[int, bytes]:
    if len(data) < pack.HEADER_STRUCT.size:
        raise SystemExit("file too small to be a LANG container")
    hdr = unpack_header(data)
    files = {pack.LANG_FDS_KEY: data[: hdr["manifest_size"]]}
    off = pack.HEADER_STRUCT.size
    entries = []
    for _ in range(hdr["entry_count"]):
        key, size, crc, kind, obj_id, rsv = pack.ENTRY_STRUCT.unpack_from(data, off)
        entries.append((key, size, crc, kind, obj_id))
        off += pack.ENTRY_STRUCT.size
    payload_off = off
    for key, size, crc, kind, obj_id in entries:
        blob = data[payload_off : payload_off + size]
        if len(blob) != size:
            raise SystemExit(f"truncated object key=0x{key:08X}")
        files[key] = blob
        payload_off += size
    return files


def unpack_header(data: bytes) -> dict:
    fields = pack.HEADER_STRUCT.unpack_from(data, 0)
    return {
        "magic": fields[0],
        "schema_major": fields[1],
        "schema_minor": fields[2],
        "product_id": fields[3],
        "ui_version": fields[4],
        "lang_id": fields[5],
        "entry_count": fields[6],
        "pack_version": fields[7],
        "payload_size": fields[8],
        "payload_crc32": fields[9],
        "manifest_size": fields[10],
    }


def unpack_ready(data: bytes) -> dict:
    magic, hdr_magic, pack_ver, payload_size, payload_crc, lang_id, schema, flag_crc = (
        pack.READY_STRUCT.unpack_from(data, 0)
    )
    ready_wo_crc = struct.pack(
        "<IIIIIHH", magic, hdr_magic, pack_ver, payload_size, payload_crc, lang_id, schema
    )
    return {
        "magic": magic,
        "hdr_magic": hdr_magic,
        "pack_version": pack_ver,
        "payload_size": payload_size,
        "payload_crc32": payload_crc,
        "lang_id": lang_id,
        "schema_major": schema,
        "flag_crc32": flag_crc,
        "flag_crc_ok": crc32_fw(ready_wo_crc) == flag_crc,
    }


def parse_manifest(data: bytes) -> tuple[dict, list[dict]]:
    hdr = unpack_header(data)
    entries = []
    off = pack.HEADER_STRUCT.size
    for _ in range(hdr["entry_count"]):
        if off + pack.ENTRY_STRUCT.size > len(data):
            break
        key, size, crc, kind, obj_id, rsv = pack.ENTRY_STRUCT.unpack_from(data, off)
        entries.append(
            {
                "key": key,
                "size": size,
                "crc32": crc,
                "kind": kind,
                "id": obj_id,
            }
        )
        off += pack.ENTRY_STRUCT.size
    return hdr, entries


def parse_items(payload: bytes) -> list[dict]:
    items = []
    off = 0
    while off + pack.ITEM_STRUCT.size <= len(payload):
        (
            item_id,
            kind,
            font_role,
            x,
            y,
            max_w,
            obj_align,
            text_align,
            line_space,
            opa,
            color_rgb,
            flags,
            rsv,
            text_bytes,
        ) = pack.ITEM_STRUCT.unpack_from(payload, off)
        text_off = off + pack.ITEM_STRUCT.size
        raw = payload[text_off : text_off + text_bytes]
        text = raw.split(b"\x00", 1)[0].decode("utf-8", errors="replace")
        total = pack.align4(pack.ITEM_STRUCT.size + text_bytes)
        items.append(
            {
                "id": item_id,
                "kind": kind,
                "font_role": font_role,
                "x": x,
                "y": y,
                "max_w": max_w,
                "obj_align": obj_align,
                "text_align": text_align,
                "line_space": line_space,
                "opa": opa,
                "color_rgb": color_rgb,
                "flags": flags,
                "text": text,
            }
        )
        if total <= 0:
            break
        off += total
    return items


def parse_page_file(data: bytes) -> dict:
    magic, page_id, item_count, payload_bytes, crc = pack.PAGE_HDR_STRUCT.unpack_from(data, 0)
    payload = data[pack.PAGE_HDR_STRUCT.size :]
    crc_ok = crc32_fw(payload) == crc
    items = parse_items(payload[:payload_bytes] if payload_bytes <= len(payload) else payload)
    return {
        "magic": magic,
        "page_id": page_id,
        "item_count": item_count,
        "payload_bytes": payload_bytes,
        "crc32": crc,
        "crc_ok": crc_ok,
        "items": items,
    }


def detect_and_read(path: Path) -> dict[int, bytes]:
    data = path.read_bytes()
    if len(data) >= 4 and struct.unpack_from("<I", data, 0)[0] == pack.MAGIC:
        return parse_pack_container(data)
    return read_lfs_files(path)


def build_model(files: dict[int, bytes], id_maps: dict[str, dict[int, str]]) -> dict:
    pages = id_maps["page"]
    manifest = files.get(pack.LANG_FDS_KEY, b"")
    header, entries = parse_manifest(manifest) if manifest else ({}, [])
    ready = unpack_ready(files[pack.READY_FDS_KEY]) if pack.READY_FDS_KEY in files else None

    for entry in entries:
        blob = files.get(entry["key"], b"")
        entry["present"] = entry["key"] in files
        entry["actual_size"] = len(blob)
        entry["crc_ok"] = bool(blob) and crc32_fw(blob) == entry["crc32"]
        entry["label"] = key_label(entry["key"], pages)

    page_files = []
    images = []
    fonts = []
    orphans = []
    known_keys = {e["key"] for e in entries} | {pack.LANG_FDS_KEY, pack.READY_FDS_KEY}

    for key, blob in sorted(files.items()):
        if key == pack.LANG_FDS_KEY or key == pack.READY_FDS_KEY:
            continue
        if (key & 0xFFFF0000) in (0x50530000, 0x50570000):
            parsed = parse_page_file(blob)
            kind = "str" if (key & 0xFFFF0000) == 0x50530000 else "wgt"
            names = id_maps["str"] if kind == "str" else None
            for item in parsed["items"]:
                if kind == "str":
                    item["name"] = names.get(item["id"], f"STR#{item['id']}")
                elif item["kind"] == 2:
                    item["name"] = id_maps["layout"].get(item["id"], f"LAY#{item['id']}")
                else:
                    item["name"] = id_maps["widget"].get(item["id"], f"W#{item['id']}")
                item["kind_name"] = ITEM_KIND.get(item["kind"], str(item["kind"]))
                item["align_name"] = ALIGN_NAMES.get(item["obj_align"], str(item["obj_align"]))
                role = item["font_role"]
                item["font_name"] = "inherit" if role == 0xFF else FONT_ROLES.get(role, str(role))
            parsed["key"] = key
            parsed["kind"] = kind
            parsed["page_name"] = pages.get(parsed["page_id"], str(parsed["page_id"]))
            parsed["size"] = len(blob)
            page_files.append(parsed)
        elif (key & 0xFFFFFF00) == 0x46494D00:
            img_id = key & 0xFF
            images.append(
                {
                    "key": key,
                    "id": img_id,
                    "name": IMG_NAMES.get(img_id, f"img_{img_id}"),
                    "size": len(blob),
                    "png": blob.startswith(b"\x89PNG"),
                    "data": blob,
                }
            )
        elif (key & 0xFFFFFF00) == 0x464E5400:
            role = key & 0xFF
            fonts.append(
                {
                    "key": key,
                    "role": role,
                    "name": FONT_ROLES.get(role, str(role)),
                    "size": len(blob),
                    "crc32": crc32_fw(blob),
                }
            )
        elif key not in known_keys:
            orphans.append({"key": key, "size": len(blob)})

    page_files.sort(key=lambda p: (p["page_id"], p["kind"]))
    return {
        "header": header,
        "ready": ready,
        "entries": entries,
        "pages": page_files,
        "images": images,
        "fonts": fonts,
        "orphans": orphans,
        "file_count": len(files),
        "total_bytes": sum(len(v) for v in files.values()),
    }


def item_css_pos(item: dict) -> str:
    x, y = item["x"], item["y"]
    align = item["obj_align"]
    if align in (2, 5, 9):
        left = SCREEN_W / 2 + x
        return f"left:{left}px;top:{y}px;transform:translateX(-50%);"
    if align in (3, 6, 8):
        return f"right:{SCREEN_W - x}px;top:{y}px;"
    return f"left:{x}px;top:{y}px;"


def jsonable(model: dict) -> dict:
    out = dict(model)
    out["images"] = [
        {
            "key": img["key"],
            "id": img["id"],
            "name": img["name"],
            "size": img["size"],
            "png": img["png"],
        }
        for img in model["images"]
    ]
    return out


def write_html(model: dict, src: Path) -> str:
    hdr = model["header"]
    ready = model["ready"]
    lang = "zh" if hdr.get("lang_id") == 1 else "en" if hdr.get("lang_id") == 0 else str(hdr.get("lang_id"))
    pages_by_id: dict[int, dict[str, dict]] = defaultdict(dict)
    for page in model["pages"]:
        pages_by_id[page["page_id"]][page["kind"]] = page

    rows = []
    for entry in model["entries"]:
        ok = "ok" if entry.get("crc_ok") else "bad"
        rows.append(
            "<tr>"
            f"<td><code>0x{entry['key']:08X}</code></td>"
            f"<td>{html.escape(entry['label'])}</td>"
            f"<td>{KIND_NAMES.get(entry['kind'], entry['kind'])}</td>"
            f"<td>{entry['size']}</td>"
            f"<td class='{ok}'>{ok}</td>"
            "</tr>"
        )

    img_html = []
    for img in model["images"]:
        if img["png"]:
            b64 = base64.b64encode(img["data"]).decode("ascii")
            img_html.append(
                "<figure>"
                f"<img src='data:image/png;base64,{b64}' alt='{html.escape(img['name'])}'/>"
                f"<figcaption>{html.escape(img['name'])} · {img['size']} B</figcaption>"
                "</figure>"
            )
        else:
            img_html.append(
                f"<p>{html.escape(img['name'])}: {img['size']} B (not PNG)</p>"
            )

    font_rows = "".join(
        f"<tr><td>{html.escape(f['name'])}</td><td>{f['size']}</td>"
        f"<td><code>{f['crc32']:08X}</code></td></tr>"
        for f in model["fonts"]
    )

    page_sections = []
    for page_id in sorted(pages_by_id):
        group = pages_by_id[page_id]
        name = (group.get("str") or group.get("wgt") or {}).get("page_name", str(page_id))
        str_page = group.get("str")
        wgt_page = group.get("wgt")
        markers = []
        table_rows = []
        if str_page:
            for item in str_page["items"]:
                text = item["text"].replace("\n", "↵")
                markers.append(
                    f"<div class='lab' style='{item_css_pos(item)}' title='{html.escape(item['name'])}'>"
                    f"{html.escape(text or item['name'])}</div>"
                )
                table_rows.append(
                    "<tr>"
                    f"<td>{html.escape(item['name'])}</td>"
                    f"<td class='txt'>{html.escape(item['text'])}</td>"
                    f"<td>{item['x']},{item['y']}</td>"
                    f"<td>{html.escape(item['align_name'])}</td>"
                    f"<td>{item['max_w']}</td>"
                    f"<td>{item['line_space']}</td>"
                    "</tr>"
                )
        wgt_rows = []
        if wgt_page:
            for item in wgt_page["items"]:
                extra = item["x"] if item["kind"] == 2 else f"{item['x']},{item['y']}"
                wgt_rows.append(
                    "<tr>"
                    f"<td>{html.escape(item['name'])}</td>"
                    f"<td>{html.escape(item['kind_name'])}</td>"
                    f"<td>{extra}</td>"
                    f"<td>{html.escape(item['align_name'])}</td>"
                    "</tr>"
                )
        crc_note = []
        if str_page:
            crc_note.append(f"str crc {'ok' if str_page['crc_ok'] else 'BAD'} ({str_page['item_count']} items)")
        if wgt_page:
            crc_note.append(f"wgt crc {'ok' if wgt_page['crc_ok'] else 'BAD'} ({wgt_page['item_count']} items)")
        page_sections.append(
            f"<section class='page' id='page-{page_id}'>"
            f"<h2>{html.escape(name)}</h2>"
            f"<p class='meta'>{' · '.join(crc_note)}</p>"
            "<div class='split'>"
            f"<div class='screen'>{''.join(markers)}</div>"
            "<div class='tables'>"
            "<h3>文案</h3>"
            "<table><thead><tr><th>ID</th><th>文本</th><th>x,y</th><th>align</th><th>max_w</th><th>ls</th></tr></thead>"
            f"<tbody>{''.join(table_rows) or '<tr><td colspan=6>无</td></tr>'}</tbody></table>"
            "<h3>组件 / 标量</h3>"
            "<table><thead><tr><th>ID</th><th>kind</th><th>pos/value</th><th>align</th></tr></thead>"
            f"<tbody>{''.join(wgt_rows) or '<tr><td colspan=4>无</td></tr>'}</tbody></table>"
            "</div></div></section>"
        )

    ready_line = "missing"
    if ready:
        ready_line = (
            f"lang={ready['lang_id']} ver={ready['pack_version']} "
            f"schema={ready['schema_major']} flag_crc={'ok' if ready['flag_crc_ok'] else 'BAD'}"
        )

    toc = "".join(
        f"<a href='#page-{pid}'>{html.escape(pages_by_id[pid].get('str', pages_by_id[pid].get('wgt', {})).get('page_name', str(pid)))}</a>"
        for pid in sorted(pages_by_id)
    )

    return f"""<!DOCTYPE html>
<html lang="zh">
<head>
<meta charset="utf-8"/>
<title>语言包预览 · {html.escape(src.name)}</title>
<style>
:root {{ color-scheme: light; --fg:#1b1b1b; --muted:#666; --line:#ddd; --acc:#0b57d0; --bg:#f6f6f4; }}
body {{ margin:0; font:14px/1.45 ui-sans-serif,system-ui,sans-serif; color:var(--fg); background:var(--bg); }}
header {{ padding:20px 24px 8px; }}
h1 {{ font-size:20px; margin:0 0 8px; }}
.meta, figcaption {{ color:var(--muted); font-size:12px; }}
nav {{ display:flex; flex-wrap:wrap; gap:8px; padding:0 24px 16px; }}
nav a {{ color:var(--acc); text-decoration:none; border:1px solid var(--line); padding:4px 8px; border-radius:4px; background:#fff; }}
section {{ background:#fff; margin:0 24px 24px; padding:16px; border:1px solid var(--line); }}
table {{ border-collapse:collapse; width:100%; font-size:12px; }}
th, td {{ border-bottom:1px solid var(--line); padding:4px 6px; text-align:left; vertical-align:top; }}
td.txt {{ white-space:pre-wrap; max-width:280px; }}
.ok {{ color:#0b7a0b; }} .bad {{ color:#b00020; font-weight:600; }}
.split {{ display:flex; gap:16px; flex-wrap:wrap; }}
.screen {{ position:relative; width:{SCREEN_W}px; height:{SCREEN_H}px; background:#111; color:#f3f3f3;
  border-radius:50%; overflow:hidden; flex:0 0 {SCREEN_W}px; }}
.lab {{ position:absolute; font-size:11px; line-height:1.2; white-space:pre; max-width:200px;
  color:#fff; text-shadow:0 0 2px #000; pointer-events:none; }}
.tables {{ flex:1; min-width:320px; }}
.imgs {{ display:flex; flex-wrap:wrap; gap:16px; }}
figure {{ margin:0; }}
img {{ max-width:240px; background:#222; }}
code {{ font-family:ui-monospace,monospace; }}
</style>
</head>
<body>
<header>
<h1>语言包预览</h1>
<p class="meta">{html.escape(str(src))}<br>
product=0x{hdr.get('product_id', 0):02X} ui=0x{hdr.get('ui_version', 0):02X}
lang={html.escape(lang)} schema={hdr.get('schema_major')}.{hdr.get('schema_minor')}
pack_ver={hdr.get('pack_version')} files={model['file_count']} payload={model['total_bytes']} B<br>
ready: {html.escape(ready_line)}</p>
</header>
<nav>{toc}</nav>
<section>
<h2>对象清单</h2>
<table><thead><tr><th>key</th><th>名称</th><th>kind</th><th>size</th><th>crc</th></tr></thead>
<tbody>{''.join(rows)}</tbody></table>
</section>
<section>
<h2>字库</h2>
<table><thead><tr><th>role</th><th>size</th><th>crc32</th></tr></thead>
<tbody>{font_rows or '<tr><td colspan=3>无（英文包不带字库）</td></tr>'}</tbody></table>
</section>
<section>
<h2>图片文字</h2>
<div class="imgs">{''.join(img_html) or '<p>无</p>'}</div>
</section>
{''.join(page_sections)}
</body>
</html>
"""


def print_summary(model: dict, src: Path) -> None:
    hdr = model["header"]
    print(f"file: {src}")
    print(
        f"product=0x{hdr.get('product_id', 0):02X} ui=0x{hdr.get('ui_version', 0):02X} "
        f"lang={hdr.get('lang_id')} schema={hdr.get('schema_major')}.{hdr.get('schema_minor')} "
        f"objects={hdr.get('entry_count')} stored={model['total_bytes']} B"
    )
    if model["ready"]:
        r = model["ready"]
        print(f"ready: crc={'ok' if r['flag_crc_ok'] else 'BAD'} lang={r['lang_id']}")
    for entry in model["entries"]:
        mark = "ok" if entry.get("crc_ok") else "BAD"
        print(f"  0x{entry['key']:08X}  {entry['label']:28s} {entry['size']:7d}  {mark}")
    print(f"pages={len({p['page_id'] for p in model['pages']})} fonts={len(model['fonts'])} imgs={len(model['images'])}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Preview language-pack LFS / PACK images")
    parser.add_argument("image", type=Path, help="*_LFS.bin or *_PACK.bin")
    parser.add_argument("--ui", choices=("v3", "v3pro"), default="v3pro")
    parser.add_argument("-o", "--output", type=Path, help="HTML output path")
    parser.add_argument("--json", action="store_true", help="print JSON instead of HTML")
    parser.add_argument("--open", action="store_true", help="open HTML in a browser")
    parser.add_argument("--no-html", action="store_true", help="only print the file list")
    args = parser.parse_args()

    image = args.image.resolve()
    if not image.is_file():
        raise SystemExit(f"not found: {image}")

    repo = Path(__file__).resolve().parents[3]
    files = detect_and_read(image)
    if not files:
        raise SystemExit("no FDS integer files found in image")
    model = build_model(files, load_id_maps(repo, args.ui))
    print_summary(model, image)

    if args.json:
        print(json.dumps(jsonable(model), ensure_ascii=False, indent=2))
        return 0
    if args.no_html:
        return 0

    html_path = args.output or image.with_name(image.stem + ".preview.html")
    html_path.write_text(write_html(model, image), encoding="utf-8")
    print(f"wrote {html_path}")
    if args.open:
        open_preview(html_path)
    return 0


def open_preview(html_path: Path) -> None:
    """Open the HTML file without going through file:// (Cursor/Node url.parse warning)."""
    path = str(html_path.resolve())
    opener = shutil.which("xdg-open")
    if opener:
        subprocess.Popen(
            [opener, path],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            start_new_session=True,
        )
        return
    webbrowser.open(path)


if __name__ == "__main__":
    sys.exit(main())
