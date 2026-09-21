#!/usr/bin/env python3
"""Generate stable string/widget/layout IDs and firmware default tables.

Source of truth: each UI module's lang/{en,zh}/{strings,layout}.h,
aggregated by lang/{en,zh}/strings.h and layout.h.
Run from repo: python3 overlay/ui/v3_ui_v7/lang/tools/gen_ui_lang_tables.py
Also generates v3pro_ui_v7 if that tree exists.
Existing ui_*_id.h order is preserved; new macros are appended.
"""

from __future__ import annotations

import html
import re
import sys
from collections import OrderedDict
from pathlib import Path

DEFINE_RE = re.compile(r"^#define\s+([A-Z][A-Z0-9_]+)\s+(.+?)\s*(?:/\*.*)?(?://.*)?$")
INCLUDE_RE = re.compile(r'^#include\s+"([^"]+)"')
ENUM_TOKEN_RE = re.compile(r"^\s+([A-Z][A-Z0-9_]*)(?:\s*=\s*0)?\s*,")
GUARD_RE = re.compile(r"^__UI_LANG_")

SCALAR_SUFFIXES = ("_GAP", "_SHIFT_Y")

UI_PAGE_NAMES = [
    "COMMON",
    "HOME",
    "PUMP",
    "GUIDE",
    "THEME_GUIDE",
    "RHYTHM",
    "SETTINGS",
    "POWER",
    "CHARGE",
    "OTA",
    "FACTORY",
    "SYS_ERROR",
    "BLE",
    "BATTERY",
    "SEAL",
    "PRODUCT_TESTS",
]

# Longest prefix first.
UI_PAGE_PREFIX_RULES = (
    ("THEME_GUIDE", "THEME_GUIDE_"),
    ("PRODUCT_TESTS", "PRODUCT_TESTS_"),
    ("FACTORY", "FACTORY_RESET_"),
    ("SEAL", "PUMP_SEAL_"),
    ("SEAL", "PUMP_FIT_"),
    ("GUIDE", "GUIDE_"),
    ("HOME", "HOME_"),
    ("PUMP", "PUMP_"),
    ("RHYTHM", "RHYTHM_"),
    ("SETTINGS", "SETTINGS_"),
    ("CHARGE", "CHARGE_"),
    ("OTA", "OTA_"),
    ("BLE", "BASE_PUMP_HOME_BLE_"),
    ("BLE", "CUSTOM_PUMP_HOME_BLE_"),
    ("BLE", "BASE_BLE_"),
    ("BLE", "CUSTOM_BLE_"),
    ("BLE", "BLE_"),
    ("BATTERY", "BASE_PUMP_HOME_BATTERY_"),
    ("BATTERY", "CUSTOM_PUMP_HOME_BATTERY_"),
    ("BATTERY", "BASE_BATTERY_"),
    ("BATTERY", "CUSTOM_BATTERY_"),
    ("BATTERY", "BATTERY_"),
    ("SYS_ERROR", "SYS_ERROR_"),
    ("SYS_ERROR", "TEMP_ERROR_"),
    ("POWER", "POWER_"),
    ("POWER", "AIRPLANE_"),
    ("POWER", "TRAVEL_"),
    ("POWER", "LOW_POWER_"),
    ("POWER", "RESTARTING_"),
    ("COMMON", "UI_STR_"),
    ("COMMON", "PROGRESS_ARC_"),
)


def page_name_of(macro: str) -> str:
    for page, prefix in UI_PAGE_PREFIX_RULES:
        if macro.startswith(prefix):
            return page
    return "COMMON"


def page_id_of(macro: str) -> int:
    return UI_PAGE_NAMES.index(page_name_of(macro))


def str_widget_stem(str_key: str, widgets: dict) -> str | None:
    """Map a string macro to a layout widget stem when they share a name."""
    candidates = [str_key]
    if str_key.startswith("UI_STR_"):
        return None
    for suf in ("_TEXT_FMT", "_TEXT", "_STR"):
        if str_key.endswith(suf):
            candidates.append(str_key[: -len(suf)])
    for cand in candidates:
        if cand in widgets:
            return cand
    return None


def resolve_include(from_path: Path, inc: str, search_root: Path | None) -> Path:
    direct = from_path.parent / inc
    if direct.is_file():
        return direct
    name = Path(inc).name
    lang = from_path.parent.name
    if search_root is None:
        return direct
    hits = [p for p in search_root.rglob(name) if p.is_file() and p.parent.name == lang]
    if len(hits) == 1:
        return hits[0]
    if len(hits) > 1:
        raise RuntimeError(f"{from_path}: ambiguous include {inc}: {hits}")
    return direct


def parse_defines(
    path: Path,
    _seen: set[Path] | None = None,
    search_root: Path | None = None,
) -> OrderedDict[str, str]:
    items: OrderedDict[str, str] = OrderedDict()
    path = Path(path)
    if not path.is_file():
        return items
    if search_root is None and path.parent.name in ("en", "zh") and path.parent.parent.name == "lang":
        search_root = path.parent.parent.parent
    seen = _seen if _seen is not None else set()
    resolved = path.resolve()
    if resolved in seen:
        return items
    seen.add(resolved)
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        inc = INCLUDE_RE.match(line)
        if inc:
            child = resolve_include(path, inc.group(1), search_root).resolve()
            for name, value in parse_defines(child, seen, search_root).items():
                items.setdefault(name, value)
            continue
        m = DEFINE_RE.match(line)
        if not m:
            continue
        name, value = m.group(1), m.group(2).strip()
        if GUARD_RE.search(name) or name.startswith("__"):
            continue
        items.setdefault(name, value)
    return items


def read_enum_tokens(path: Path, prefix: str) -> list[str]:
    tokens: list[str] = []
    if not path.is_file():
        return tokens
    for raw in path.read_text(encoding="utf-8").splitlines():
        m = ENUM_TOKEN_RE.match(raw)
        if not m:
            continue
        name = m.group(1)
        if not name.startswith(prefix) or name.endswith("_COUNT") or name == f"{prefix}COUNT":
            continue
        tokens.append(name[len(prefix) :])
    return tokens


def stabilize_keys(
    existing_tokens: list[str],
    keys: list[str],
    to_token,
) -> list[str]:
    token_to_key = {to_token(k): k for k in keys}
    out: list[str] = []
    seen: set[str] = set()
    for tok in existing_tokens:
        key = token_to_key.get(tok)
        if key is None or key in seen:
            continue
        out.append(key)
        seen.add(key)
    for key in keys:
        if key not in seen:
            out.append(key)
            seen.add(key)
    return out


def str_enum_token(macro: str) -> str:
    if macro.startswith("UI_STR_"):
        return macro[len("UI_STR_") :]
    return macro


def union_keys(*dicts: OrderedDict[str, str]) -> list[str]:
    seen: OrderedDict[str, None] = OrderedDict()
    for d in dicts:
        for k in d:
            seen.setdefault(k, None)
    return list(seen.keys())


def eval_layout_value(name: str, raw: str, resolved: dict[str, int]) -> int | None:
    token = raw.strip()
    if token in resolved:
        return resolved[token]
    expr = token
    for key in sorted(resolved, key=len, reverse=True):
        expr = re.sub(rf"\b{key}\b", str(resolved[key]), expr)
    expr = expr.replace("(", " ").replace(")", " ")
    if re.fullmatch(r"[\s0-9+\-]+", expr):
        return int(eval(expr, {"__builtins__": {}}, {}))
    return None


def resolve_layout(defines: OrderedDict[str, str]) -> dict[str, int]:
    resolved: dict[str, int] = {}
    pending = list(defines.items())
    for _ in range(8):
        leftover = []
        for name, raw in pending:
            val = eval_layout_value(name, raw, resolved)
            if val is None:
                leftover.append((name, raw))
            else:
                resolved[name] = val
        pending = leftover
        if not pending:
            break
    if pending:
        raise RuntimeError(f"unresolved layout macros: {[n for n, _ in pending]}")
    return resolved


def classify_layout(name: str) -> tuple[str, str, str] | None:
    """Return (kind, stem, field) or None for skip. kind is widget|scalar."""
    if name.endswith(SCALAR_SUFFIXES):
        return ("scalar", name, "value")
    if name.endswith("_LINE_SPACE"):
        return ("widget", name[: -len("_LINE_SPACE")], "line_space")
    if name.endswith("_MAX_W"):
        return ("widget", name[: -len("_MAX_W")], "max_w")
    if name.endswith("_ALIGN_Y"):
        return ("widget", name[: -len("_ALIGN_Y")], "y")
    if name.endswith("_ICON_X"):
        return ("widget", name[: -len("_X")], "x")
    if name.endswith("_ICON_Y"):
        return ("widget", name[: -len("_Y")], "y")
    if name.endswith("_X"):
        return ("widget", name[: -len("_X")], "x")
    if name.endswith("_Y"):
        return ("widget", name[: -len("_Y")], "y")
    return ("scalar", name, "value")


def widget_kind(stem: str) -> str:
    return "ICON" if "ICON" in stem or "BLE_UI" in stem else "TEXT"


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")
    print(f"wrote {path}")


def emit_enum(
    guard: str,
    enum_name: str,
    prefix: str,
    names: list[str],
    count_name: str,
    file_comment: str,
    item_comments: dict[str, str] | None = None,
) -> str:
    lines = [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        f"/* {file_comment} */",
        "/* 本文件由 gen_ui_lang_tables.py 生成，不要手改。对照表见同目录 ui_lang_id_map.html */",
        "",
        "#ifdef __cplusplus",
        'extern "C" {',
        "#endif",
        "",
        f"typedef enum {{",
    ]
    for i, name in enumerate(names):
        suffix = " = 0" if i == 0 else ""
        note = (item_comments or {}).get(name, "")
        if note:
            lines.append(f"    {prefix}{name}{suffix}, /* {note} */")
        else:
            lines.append(f"    {prefix}{name}{suffix},")
    lines += [
        f"    {count_name}",
        f"}} {enum_name};",
        "",
        "#ifdef __cplusplus",
        "}",
        "#endif",
        f"#endif /* {guard} */",
        "",
    ]
    return "\n".join(lines)


def layout_fields_of(stem: str, lay_keys: list[str]) -> list[str]:
    out = []
    for name in lay_keys:
        classified = classify_layout(name)
        if classified and classified[0] == "widget" and classified[1] == stem:
            out.append(name)
    return out


def strings_for_stem(stem: str, str_keys: list[str], widgets) -> list[str]:
    return [key for key in str_keys if str_widget_stem(key, widgets) == stem]


def emit_id_map_html(
    ui_name: str,
    str_keys: list[str],
    widgets: OrderedDict[str, dict[str, int]],
    scalars: list[str],
    lay_keys: list[str],
) -> str:
    nav = " ".join(f'<a href="#p-{name}">{html.escape(name)}</a>' for name in UI_PAGE_NAMES)
    sections = []
    str_index = {key: i for i, key in enumerate(str_keys)}
    wgt_index = {stem: i for i, stem in enumerate(widgets)}
    lay_index = {name: i for i, name in enumerate(scalars)}

    for page in UI_PAGE_NAMES:
        str_items = [k for k in str_keys if page_name_of(k) == page]
        wgt_items = [s for s in widgets if page_name_of(s) == page]
        lay_items = [n for n in scalars if page_name_of(n) == page]
        if not str_items and not wgt_items and not lay_items:
            continue
        blocks = [f'<h2 id="p-{html.escape(page)}">{html.escape(page)}</h2>']
        if str_items:
            rows = []
            for key in str_items:
                stem = str_widget_stem(key, widgets)
                pair = f"<code>UI_W_{html.escape(stem)}</code>" if stem else "—"
                rows.append(
                    "<tr>"
                    f"<td>{str_index[key]}</td>"
                    f"<td><code>{html.escape(key)}</code></td>"
                    f"<td><code>UI_STR_ID_{html.escape(str_enum_token(key))}</code></td>"
                    f"<td>{pair}</td>"
                    "</tr>"
                )
            blocks.append(
                "<h3>文案</h3><table><thead><tr>"
                "<th>ID</th><th>strings.h</th><th>枚举</th><th>配对控件</th>"
                "</tr></thead><tbody>"
                + "".join(rows)
                + "</tbody></table>"
            )
        if wgt_items:
            rows = []
            for stem in wgt_items:
                fields = layout_fields_of(stem, lay_keys)
                texts = strings_for_stem(stem, str_keys, widgets)
                rows.append(
                    "<tr>"
                    f"<td>{wgt_index[stem]}</td>"
                    f"<td><code>UI_W_{html.escape(stem)}</code></td>"
                    f"<td>{html.escape(widget_kind(stem))}</td>"
                    f"<td><code>{html.escape(', '.join(fields) or '—')}</code></td>"
                    f"<td><code>{html.escape(', '.join(texts) or '—')}</code></td>"
                    "</tr>"
                )
            blocks.append(
                "<h3>控件坐标</h3><table><thead><tr>"
                "<th>ID</th><th>枚举</th><th>类型</th><th>layout.h</th><th>配对文案</th>"
                "</tr></thead><tbody>"
                + "".join(rows)
                + "</tbody></table>"
            )
        if lay_items:
            rows = []
            for name in lay_items:
                rows.append(
                    "<tr>"
                    f"<td>{lay_index[name]}</td>"
                    f"<td><code>{html.escape(name)}</code></td>"
                    f"<td><code>UI_LAY_{html.escape(name)}</code></td>"
                    "</tr>"
                )
            blocks.append(
                "<h3>标量 GAP / SHIFT</h3><table><thead><tr>"
                "<th>ID</th><th>layout.h</th><th>枚举</th>"
                "</tr></thead><tbody>"
                + "".join(rows)
                + "</tbody></table>"
            )
        sections.append("\n".join(blocks))

    return f"""<!DOCTYPE html>
<html lang="zh">
<head>
<meta charset="utf-8">
<title>{html.escape(ui_name)} 语言表（按页）</title>
<style>
body {{ font-family: sans-serif; margin: 24px; color: #222; }}
h1,h2,h3 {{ font-weight: 600; }}
h2 {{ margin-top: 36px; border-bottom: 1px solid #ddd; padding-bottom: 6px; }}
nav a {{ margin-right: 10px; }}
table {{ border-collapse: collapse; width: 100%; margin: 8px 0 20px; }}
th,td {{ border: 1px solid #ddd; padding: 6px 8px; text-align: left; font-size: 13px; }}
th {{ background: #f4f4f4; }}
code {{ font-size: 12px; }}
.how {{ background: #fff8e5; padding: 12px 16px; border-radius: 8px; line-height: 1.7; }}
.flow {{ background: #f7f7f7; padding: 12px 16px; border-radius: 8px; line-height: 1.6; margin-top: 12px; }}
</style>
</head>
<body>
<h1>{html.escape(ui_name)} 语言表（按页对照）</h1>
<div class="how">
<b>怎么加，不要改 ui_*_id.h / ui_lang_bind.h</b><br>
1. <b>只改文案</b>：在 <code>en/strings.h</code> 和 <code>zh/strings.h</code> 改同一宏的引号内容。<br>
2. <b>只改已有控件坐标</b>：在两种语言的 <code>layout.h</code> 改 <code>STEM_Y</code> / <code>_X</code> / <code>_LINE_SPACE</code> / <code>_MAX_W</code>。<br>
3. <b>新增文案</b>：两种语言的 <code>strings.h</code> <u>文件末尾</u>追加同名宏，前缀用页面名（<code>HOME_</code> / <code>PUMP_</code>…）。<br>
4. <b>新增控件</b>：<code>layout.h</code> 末尾追加 <code>STEM_Y</code>（必要），可选 <code>STEM_X</code>、<code>STEM_LINE_SPACE</code>、<code>STEM_MAX_W</code>；名字里带 <code>ICON</code> 会当成图标。<br>
5. 然后 <code>python3 overlay/ui/v3_ui_v7/lang/tools/gen_ui_lang_tables.py</code>，再 <code>./pack.sh</code>。<br>
新宏必须<strong>追加在文件末尾</strong>，插到中间会打乱后面所有 ID，旧语言包会错位。
</div>
<div class="flow">
页面 C 只写宏名。bind 把文案宏变成 <code>ui_str(UI_STR_ID_*)</code>，把 <code>STEM_Y</code> 变成 <code>ui_widget(UI_W_STEM)-&gt;y</code>。
圆屏预览：<code>overlay/font/packaging_tools/preview.sh --open</code>
完整说明：<code>docs/architecture/ui/lang-edit.md</code>
</div>
<p>跳转：{nav}</p>
{''.join(sections)}
</body></html>
"""


def emit_default_c(ui_name: str, str_keys: list[str], widgets: OrderedDict, scalars: list[str]) -> str:
    widget_inits = []
    for stem, fields in widgets.items():
        kind = widget_kind(stem)
        x = fields.get("x", 0)
        y = fields.get("y", 0)
        line_space = fields.get("line_space", 0)
        max_w = fields.get("max_w", 0)
        obj_align = 1 if x else 2  # TOP_LEFT / TOP_MID
        wid = f"UI_W_{stem}"
        widget_inits.append(
            "    [%s] = {\n"
            "        .kind = UI_WIDGET_KIND_%s,\n"
            "        .obj_align = %d,\n"
            "        .x = %d,\n"
            "        .y = %d,\n"
            "        .opa = 255,\n"
            "        .text_align = %s,\n"
            "        .line_space = %d,\n"
            "        .font_role = 0xFF,\n"
            "        .max_w = %d,\n"
            "        .color_rgb = 0,\n"
            "        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,\n"
            "    },"
            % (
                wid,
                kind,
                obj_align,
                x,
                y,
                "1" if kind == "TEXT" else "0",  # LV_TEXT_ALIGN_CENTER
                line_space,
                max_w,
            )
        )

    str_inits = []
    for key in str_keys:
        str_inits.append(
            f"#ifdef {key}\n"
            f"    [UI_STR_ID_{key}] = {key},\n"
            f"#endif"
        )

    scalar_inits = []
    for name in scalars:
        scalar_inits.append(
            f"#ifdef {name}\n"
            f"    [UI_LAY_{name}] = (int16_t)({name}),\n"
            f"#endif"
        )

    return f"""\
/* Generated for {ui_name}. Do not edit. Source: lang/{{en,zh}}/strings.h layout.h */
#include "product_info.h"
#include "ui_lang_runtime.h"
#include "lv_user_font.h"

#if (PRODUCT_LANGUAGE_USED == PRODUCT_LANGUAGE_EN)
#include "en/strings.h"
#include "en/layout.h"
#include "en/fonts.h"
#elif (PRODUCT_LANGUAGE_USED == PRODUCT_LANGUAGE_ZH)
#include "zh/strings.h"
#include "zh/layout.h"
#include "zh/fonts.h"
#else
#error "Unsupported PRODUCT_LANGUAGE_USED"
#endif

const char *const g_ui_str_default[UI_STR_ID_COUNT] = {{
{chr(10).join(str_inits)}
}};

const ui_widget_style_t g_ui_widget_default[UI_W_COUNT] = {{
{chr(10).join(widget_inits)}
}};

const int16_t g_ui_layout_default[UI_LAY_COUNT] = {{
{chr(10).join(scalar_inits)}
}};

const lv_font_t *const g_ui_font_default[UI_FONT_ROLE_COUNT] = {{
    [UI_FONT_ROLE_HINT] = UI_FONT_HINT_FW,
    [UI_FONT_ROLE_BODY_SM] = UI_FONT_BODY_SM_FW,
    [UI_FONT_ROLE_TITLE] = UI_FONT_TITLE_FW,
    [UI_FONT_ROLE_TITLE_MD] = UI_FONT_TITLE_MD_FW,
    [UI_FONT_ROLE_TITLE_LG] = UI_FONT_TITLE_LG_FW,
    [UI_FONT_ROLE_LATIN_19] = UI_FONT_AEONIK_19_FW,
    [UI_FONT_ROLE_LATIN_26] = UI_FONT_AEONIK_FW,
    [UI_FONT_ROLE_LATIN_29] = UI_FONT_AEONIK_29_FW,
    [UI_FONT_ROLE_NUM_MD] = UI_FONT_NUM_MD_FW,
    [UI_FONT_ROLE_NUM_LG] = UI_FONT_NUM_LG_FW,
}};
"""


def classify_layout_keys(lay_keys: list[str]) -> tuple[OrderedDict[str, dict[str, int]], list[str]]:
    widgets: OrderedDict[str, dict[str, int]] = OrderedDict()
    scalars: list[str] = []
    for name in lay_keys:
        classified = classify_layout(name)
        if classified is None:
            continue
        kind, stem, field = classified
        if kind == "scalar":
            if name not in scalars:
                scalars.append(name)
            continue
        widgets.setdefault(stem, {})
        widgets[stem][field] = 0
    return widgets, scalars


def load_lang_index(lang_dir: Path) -> tuple[
    list[str],
    OrderedDict[str, dict[str, int]],
    list[str],
    OrderedDict[str, str],
    OrderedDict[str, str],
    OrderedDict[str, str],
    OrderedDict[str, str],
    list[str],
]:
    """Parse en/zh sources and keep previously generated ID order."""
    en_str = parse_defines(lang_dir / "en" / "strings.h")
    zh_str = parse_defines(lang_dir / "zh" / "strings.h")
    en_lay = parse_defines(lang_dir / "en" / "layout.h")
    zh_lay = parse_defines(lang_dir / "zh" / "layout.h")
    str_keys = stabilize_keys(
        read_enum_tokens(lang_dir / "ui_str_id.h", "UI_STR_ID_"),
        union_keys(en_str, zh_str),
        str_enum_token,
    )
    lay_keys = union_keys(en_lay, zh_lay)
    widgets, scalars = classify_layout_keys(lay_keys)
    widget_stems = stabilize_keys(
        read_enum_tokens(lang_dir / "ui_widget_id.h", "UI_W_"),
        list(widgets.keys()),
        lambda stem: stem,
    )
    widgets = OrderedDict((stem, widgets[stem]) for stem in widget_stems)
    scalars = stabilize_keys(
        read_enum_tokens(lang_dir / "ui_layout_id.h", "UI_LAY_"),
        scalars,
        lambda name: name,
    )
    return str_keys, widgets, scalars, en_str, zh_str, en_lay, zh_lay, lay_keys


def generate_for_ui(lang_dir: Path) -> None:
    str_keys, widgets, scalars, _en_str, _zh_str, en_lay, _zh_lay, lay_keys = load_lang_index(
        lang_dir
    )
    # Default table numeric values come from the English layout; Chinese
    # firmware still compiles zh/layout.h into the table via C macros.
    en_resolved = resolve_layout(en_lay)
    for name, val in en_resolved.items():
        classified = classify_layout(name)
        if classified is None or classified[0] != "widget":
            continue
        _, stem, field = classified
        if stem in widgets:
            widgets[stem][field] = val

    write_text(
        lang_dir / "ui_str_id.h",
        emit_enum(
            "__UI_STR_ID_H__",
            "ui_str_id_e",
            "UI_STR_ID_",
            [str_enum_token(k) for k in str_keys],
            "UI_STR_ID_COUNT",
            "文案下标。页面用 ui_str(id)；源宏在各 UI 的 lang/<zh|en>/<子页>_strings.h",
            {str_enum_token(k): f"{k}, 页 {page_name_of(k)}" for k in str_keys},
        ),
    )
    write_text(
        lang_dir / "ui_widget_id.h",
        emit_enum(
            "__UI_WIDGET_ID_H__",
            "ui_widget_id_e",
            "UI_W_",
            list(widgets.keys()),
            "UI_W_COUNT",
            "控件样式下标。ui_widget(id) 取坐标/行距等；源宏在 lang/<zh|en>/layout.h",
            {stem: f"{page_name_of(stem)} {widget_kind(stem)}" for stem in widgets},
        ),
    )
    write_text(
        lang_dir / "ui_layout_id.h",
        emit_enum(
            "__UI_LAYOUT_ID_H__",
            "ui_layout_id_e",
            "UI_LAY_",
            scalars,
            "UI_LAY_COUNT",
            "不挂在单个控件上的 layout 标量（_GAP / _SHIFT_Y 等）",
            {name: page_name_of(name) for name in scalars},
        ),
    )
    write_text(
        lang_dir / "ui_page_id.h",
        emit_enum(
            "__UI_PAGE_ID_H__",
            "ui_page_id_e",
            "UI_PAGE_",
            UI_PAGE_NAMES,
            "UI_PAGE_COUNT",
            "语言包按页拆文件。FDS：PAGE_STR=0x50530000|page，PAGE_WGT=0x50570000|page",
            {name: f"0x{i:04X}" for i, name in enumerate(UI_PAGE_NAMES)},
        ),
    )
    write_text(
        lang_dir / "ui_lang_id_map.html",
        emit_id_map_html(lang_dir.parent.name, str_keys, widgets, scalars, lay_keys),
    )

    # Default C uses macros not the python snapshot for strings/scalars.
    # Widget numeric snapshot is also overwritten by a macro-based init below.
    widget_macro_inits = []
    for stem, fields in widgets.items():
        kind = widget_kind(stem)
        x_macro = f"{stem}_X" if f"{stem}_X" in lay_keys else None
        y_macro = f"{stem}_Y" if f"{stem}_Y" in lay_keys else None
        if f"{stem}_ALIGN_Y" in lay_keys:
            y_macro = f"{stem}_ALIGN_Y"
        ls_macro = f"{stem}_LINE_SPACE" if f"{stem}_LINE_SPACE" in lay_keys else None
        mw_macro = f"{stem}_MAX_W" if f"{stem}_MAX_W" in lay_keys else None
        def cond_num(macro: str | None, c_cast: str) -> str:
            if not macro:
                return "            0"
            return (
                f"#ifdef {macro}\n"
                f"            ({c_cast})({macro})\n"
                f"#else\n"
                f"            0\n"
                f"#endif"
            )

        obj_align = "1" if x_macro else "2"
        text_align = "2" if kind == "TEXT" else "0"
        widget_macro_inits.append(
            "    [UI_W_%s] = {\n"
            "        .kind = UI_WIDGET_KIND_%s,\n"
            "        .obj_align = %s,\n"
            "        .x =\n%s\n"
            "        ,\n"
            "        .y =\n%s\n"
            "        ,\n"
            "        .opa = 255,\n"
            "        .text_align = %s,\n"
            "        .line_space =\n%s\n"
            "        ,\n"
            "        .font_role = 0xFF,\n"
            "        .max_w =\n%s\n"
            "        ,\n"
            "        .color_rgb = 0,\n"
            "        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,\n"
            "    },"
            % (
                stem,
                kind,
                obj_align,
                cond_num(x_macro, "int16_t"),
                cond_num(y_macro, "int16_t"),
                text_align,
                cond_num(ls_macro, "int8_t"),
                cond_num(mw_macro, "uint16_t"),
            )
        )

    str_inits = "\n".join(
        f"#ifdef {key}\n    [UI_STR_ID_{str_enum_token(key)}] = {key},\n#endif" for key in str_keys
    )
    scalar_inits = "\n".join(
        f"#ifdef {name}\n    [UI_LAY_{name}] = (int16_t)({name}),\n#endif" for name in scalars
    )

    default_c = f"""\
/* 固件默认语言表（无外部包或包失效时使用）。由 gen_ui_lang_tables.py 生成，不要手改。
 * 数值来自当前 PRODUCT_LANGUAGE 的 strings.h / layout.h / fonts.h。 */
#include "product_info.h"
#include "ui_lang_runtime.h"
#include "lv_user_font.h"

#if (PRODUCT_LANGUAGE_USED == PRODUCT_LANGUAGE_EN)
#include "en/strings.h"
#include "en/layout.h"
#include "en/fonts.h"
#elif (PRODUCT_LANGUAGE_USED == PRODUCT_LANGUAGE_ZH)
#include "zh/strings.h"
#include "zh/layout.h"
#include "zh/fonts.h"
#else
#error "Unsupported PRODUCT_LANGUAGE_USED"
#endif

const char *const g_ui_str_default[UI_STR_ID_COUNT] = {{
{str_inits}
}};

const ui_widget_style_t g_ui_widget_default[UI_W_COUNT] = {{
{chr(10).join(widget_macro_inits)}
}};

const int16_t g_ui_layout_default[UI_LAY_COUNT] = {{
{scalar_inits}
}};

const lv_font_t *const g_ui_font_default[UI_FONT_ROLE_COUNT] = {{
    [UI_FONT_ROLE_HINT] = UI_FONT_HINT_FW,
    [UI_FONT_ROLE_BODY_SM] = UI_FONT_BODY_SM_FW,
    [UI_FONT_ROLE_TITLE] = UI_FONT_TITLE_FW,
    [UI_FONT_ROLE_TITLE_MD] = UI_FONT_TITLE_MD_FW,
    [UI_FONT_ROLE_TITLE_LG] = UI_FONT_TITLE_LG_FW,
    [UI_FONT_ROLE_LATIN_19] = UI_FONT_AEONIK_19_FW,
    [UI_FONT_ROLE_LATIN_26] = UI_FONT_AEONIK_FW,
    [UI_FONT_ROLE_LATIN_29] = UI_FONT_AEONIK_29_FW,
    [UI_FONT_ROLE_NUM_MD] = UI_FONT_NUM_MD_FW,
    [UI_FONT_ROLE_NUM_LG] = UI_FONT_NUM_LG_FW,
}};
"""
    write_text(lang_dir / "ui_lang_default.c", default_c)

    bind_lines = [
        "#ifndef __UI_LANG_BIND_H__",
        "#define __UI_LANG_BIND_H__",
        "",
        "/* 运行时绑定：页面仍写 strings.h/layout.h 里的旧宏名，这里改成查表。",
        " * 加文案/坐标请改对应 UI 的 lang/<zh|en>/<子页>_strings.h / <子页>_layout.h 后跑 gen_ui_lang_tables.py，不要手改本文件。",
        " * 对照：同目录 ui_lang_id_map.html（按页）。 */",
        "",
    ]
    for page in UI_PAGE_NAMES:
        page_strs = [k for k in str_keys if page_name_of(k) == page]
        if not page_strs:
            continue
        bind_lines += [f"/* ===== {page} 文案 -> ui_str ===== */", ""]
        for key in page_strs:
            token = str_enum_token(key)
            bind_lines += [
                f"#undef {key}",
                f"#define {key} ui_str(UI_STR_ID_{token})",
                "",
            ]
    bind_lines += ["/* ===== 标量 -> ui_layout_i16 ===== */", ""]
    for name in lay_keys:
        classified = classify_layout(name)
        if classified is None or classified[0] != "scalar":
            continue
        bind_lines += [
            f"#undef {name}",
            f"#define {name} ui_layout_i16(UI_LAY_{name})",
            "",
        ]
    for page in UI_PAGE_NAMES:
        page_lays = []
        for name in lay_keys:
            classified = classify_layout(name)
            if classified and classified[0] == "widget" and page_name_of(classified[1]) == page:
                page_lays.append((name, classified[1], classified[2]))
        if not page_lays:
            continue
        bind_lines += [f"/* ===== {page} 控件字段 -> ui_widget ===== */", ""]
        for name, stem, field in page_lays:
            c_field = {
                "x": "x",
                "y": "y",
                "line_space": "line_space",
                "max_w": "max_w",
            }.get(field)
            if not c_field:
                continue
            c_cast = {
                "x": "int16_t",
                "y": "int16_t",
                "line_space": "int8_t",
                "max_w": "uint16_t",
            }[c_field]
            bind_lines += [
                f"#undef {name}",
                f"#define {name} (({c_cast})(ui_widget(UI_W_{stem})->{c_field}))",
                "",
            ]
    bind_lines += ["#endif /* __UI_LANG_BIND_H__ */", ""]
    write_text(lang_dir / "ui_lang_bind.h", "\n".join(bind_lines))
    print(f"{lang_dir.name}: {len(str_keys)} strings, {len(widgets)} widgets, {len(scalars)} scalars")


def main() -> int:
    root = Path(__file__).resolve().parents[5]
    targets = [
        root / "overlay" / "ui" / "v3_ui_v7" / "lang",
        root / "overlay" / "ui" / "v3pro_ui_v7" / "lang",
    ]
    for lang_dir in targets:
        if lang_dir.is_dir():
            generate_for_ui(lang_dir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
