#!/usr/bin/env python3
"""Move lang/{en,zh}/pages/* into the matching UI module's lang/{en,zh}/."""

from __future__ import annotations

import re
import shutil
import sys
from collections import OrderedDict
from pathlib import Path

DEFINE_RE = re.compile(r"^#define\s+([A-Z][A-Z0-9_]+)\b")
COMMENTED_DEFINE_RE = re.compile(r"^//\s*#define\s+([A-Z][A-Z0-9_]+)\b")

# pages/<rel> -> UI dir relative to overlay/ui/<product>/
PAGE_TO_UI = {
    "home/base": "home",
    "home/custom": "home",
    "home/saver": "home_idle",
    "home/mode": "home",
    "home/level": "home_level_adjust",
    "home/both_level": "home_both_level_adjust",
    "home/speed": "home_speed_adjust",
    "home/low_power": "low_power",
    "pump/base": "pump_home",
    "pump/custom": "pump_home",
    "pump/mode": "pump_home",
    "pump/level": "pump_level_adjust",
    "pump/both_level": "pump_both_level_adjust",
    "pump/speed": "pump_speed_adjust",
    "pump/stop_hold": "pump_stop_hold",
    "pump/pause_timeout": "pump_pause_timeout",
    "pump/finish_timeout": "pump_finish_timeout",
    "pump/will_finish": "pump_will_finish",
    "pump/finish": "pump_finish",
    "pump/tips": "page_popup",
    "rhythm": "rhythm_confirm",
    "settings": "settings_success",
    "power/power_off_hold": "power_off_hold",
    "power/power_off": "power_off",
    "power/power_on": "power_on_hold",
    "power/airplane": "airplane_mode",
    "power/travel_lock": "top_popup",
    "power/low_power": "low_power",
    "power/power_off_charge": "power_off_charge",
    "charge": "charge",
    "ota": "ota",
    "factory": "factory_reset",
    "sys_error": "system_abnormal",
    "ble": "top_popup",
    "battery": "battery",
    "theme_guide": "theme_guide",
    "product_tests": "product_tests",
    "guide/intro": "guide",
    "guide/qrcode": "guide",
    "guide/finish": "guide",
    "guide/mode_home": "guide",
    "guide/mode_switch": "guide",
    "guide/pump_home": "guide",
    "guide/pump_start": "guide",
    "guide/pump_pause": "guide",
    "guide/pump_finish": "guide",
    "guide/speed": "guide",
    "guide/level_side": "guide",
    "guide/level_both": "guide",
}

FALLBACK_UI = {
    "home/common": "home",
    "pump_home/common": "pump_home",
    "home/custom": "home",
    "pump_home/custom": "pump_home",
}

INCLUDE_ORDER = [
    "lang/common",
    "progress_arc",
    "home",
    "home_idle",
    "home_level_adjust",
    "home_both_level_adjust",
    "home_speed_adjust",
    "low_power",
    "pump_home",
    "pump_level_adjust",
    "pump_both_level_adjust",
    "pump_speed_adjust",
    "pump_stop_hold",
    "pump_pause_timeout",
    "pump_finish_timeout",
    "pump_will_finish",
    "pump_finish",
    "page_popup",
    "rhythm_confirm",
    "settings_success",
    "power_off_hold",
    "power_off",
    "power_on_hold",
    "airplane_mode",
    "ble",
    "battery",
    "top_popup",
    "power_off_charge",
    "charge",
    "ota",
    "factory_reset",
    "restart",
    "system_abnormal",
    "pump_seal_leak",
    "pump_fit_not_detected",
    "theme_guide",
    "guide",
    "product_tests",
]


def resolve_ui(ui_root: Path, ui_rel: str | None) -> Path | None:
    if not ui_rel:
        return None
    path = ui_root / ui_rel
    if path.is_dir() or ui_rel in (
        "progress_arc",
        "page_popup",
        "top_popup",
    ):
        if path.is_dir():
            return path
    fallback = FALLBACK_UI.get(ui_rel, "MISSING")
    if fallback == "MISSING":
        parent = path.parent
        if parent != ui_root and parent.is_dir():
            return path
        print(f"skip missing UI dir: {ui_rel}", file=sys.stderr)
        return None
    if fallback is None:
        return None
    alt = ui_root / fallback
    if alt.is_dir():
        return alt
    return None


def extract_defines(text: str) -> list[str]:
    lines = []
    for raw in text.splitlines():
        stripped = raw.strip()
        if DEFINE_RE.match(stripped) or COMMENTED_DEFINE_RE.match(stripped):
            lines.append(raw.rstrip())
    return lines


def guard_of(lang: str, ui_rel: str, kind: str) -> str:
    stem = ui_rel.replace("/", "_").upper()
    return f"__UI_LANG_{lang.upper()}_{stem}_{kind.upper()}_H__"


def write_lang_file(path: Path, lang: str, ui_rel: str, kind: str, lines: list[str]) -> None:
    existing = extract_defines(path.read_text(encoding="utf-8")) if path.is_file() else []
    merged: OrderedDict[str, str] = OrderedDict()
    for line in existing + lines:
        m = DEFINE_RE.match(line.strip()) or COMMENTED_DEFINE_RE.match(line.strip())
        if not m:
            continue
        merged.setdefault(m.group(1), line)
    body_lines = list(merged.values())
    if not body_lines:
        return
    guard = guard_of(lang, ui_rel, kind)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        "\n".join(
            [
                f"#ifndef {guard}",
                f"#define {guard}",
                "",
                f"/* {ui_rel} — 改这里后重新 pack，不要改页面 C */",
                "",
                *body_lines,
                "",
                f"#endif /* {guard} */",
                "",
            ]
        ),
        encoding="utf-8",
    )


def page_stem(ui_rel: str) -> str:
    parts = [p for p in ui_rel.split("/") if p]
    if len(parts) >= 3 and parts[1] == "base":
        parts = [parts[0], *parts[2:]]
    return "_".join(parts)


def page_lang_filename(ui_rel: str, kind: str) -> str:
    return f"{page_stem(ui_rel)}_{kind}.h"


def include_of(lang: str, ui_rel: str, kind: str) -> str:
    return page_lang_filename(ui_rel, kind)


def write_aggregator(path: Path, lang: str, kind: str, includes: list[str]) -> None:
    guard = f"__UI_LANG_{lang.upper()}_{kind.upper()}_H__"
    title = (
        "文案源。宏名跨语言必须一致，只改引号内译文。"
        if kind == "strings"
        else "坐标/行距/最大宽。头文件按子页命名，由 GNUmakefile -I 短引用。"
    )
    path.write_text(
        "\n".join(
            [
                f"#ifndef {guard}",
                f"#define {guard}",
                "",
                "#ifdef __cplusplus",
                'extern "C" {',
                "#endif",
                "",
                f"/* {title}",
                " * 打包后对应运行时表；页面通过 ui_lang_bind.h 取值。 */",
                "",
                *[f'#include "{rel}"' for rel in includes],
                "",
                "#ifdef __cplusplus",
                "}",
                "#endif",
                f"#endif // {guard}",
                "",
            ]
        ),
        encoding="utf-8",
    )


def relocate_lang_tree(ui_root: Path) -> None:
    lang_root = ui_root / "lang"
    if not any((lang_root / lang / "pages").is_dir() for lang in ("en", "zh")):
        print(f"{ui_root}: already relocated (no lang/*/pages)")
        return
    present: dict[tuple[str, str], set[str]] = {("strings", "en"): set(), ("strings", "zh"): set(),
                                                 ("layout", "en"): set(), ("layout", "zh"): set()}

    for lang in ("en", "zh"):
        pages = lang_root / lang / "pages"
        if not pages.is_dir():
            continue

        common_str = pages / "common" / "strings.h"
        if common_str.is_file():
            dest = lang_root / "common" / lang / page_lang_filename("lang/common", "strings")
            write_lang_file(dest, lang, "lang/common", "strings", extract_defines(common_str.read_text(encoding="utf-8")))
            present[("strings", lang)].add("lang/common")

        common_lay = pages / "common" / "layout.h"
        if common_lay.is_file():
            dest_ui = resolve_ui(ui_root, "progress_arc")
            if dest_ui is not None:
                dest = dest_ui / "lang" / lang / page_lang_filename("progress_arc", "layout")
                write_lang_file(
                    dest, lang, "progress_arc", "layout",
                    extract_defines(common_lay.read_text(encoding="utf-8")),
                )
                present[("layout", lang)].add("progress_arc")

        seal_str = pages / "seal" / "strings.h"
        seal_lay = pages / "seal" / "layout.h"
        for src, kind in ((seal_str, "strings"), (seal_lay, "layout")):
            if not src.is_file():
                continue
            leak: list[str] = []
            fit: list[str] = []
            for line in extract_defines(src.read_text(encoding="utf-8")):
                name = (DEFINE_RE.match(line.strip()) or COMMENTED_DEFINE_RE.match(line.strip())).group(1)
                if name.startswith("PUMP_FIT_"):
                    fit.append(line)
                else:
                    leak.append(line)
            if leak:
                dest = ui_root / "pump_seal_leak" / "lang" / lang / page_lang_filename("pump_seal_leak", kind)
                write_lang_file(dest, lang, "pump_seal_leak", kind, leak)
                present[(kind, lang)].add("pump_seal_leak")
            if fit:
                dest = ui_root / "pump_fit_not_detected" / "lang" / lang / page_lang_filename("pump_fit_not_detected", kind)
                write_lang_file(dest, lang, "pump_fit_not_detected", kind, fit)
                present[(kind, lang)].add("pump_fit_not_detected")

        for src in sorted(pages.rglob("*.h")):
            rel = src.relative_to(pages).as_posix()
            if rel.startswith("common/") or rel.startswith("seal/"):
                continue
            parts = rel.split("/")
            kind = parts[-1].replace(".h", "")
            page_key = "/".join(parts[:-1])
            ui_rel = PAGE_TO_UI.get(page_key)
            if ui_rel is None:
                raise RuntimeError(f"unmapped page {page_key} ({src})")
            dest_ui = resolve_ui(ui_root, ui_rel)
            if dest_ui is None:
                continue
            mapped = dest_ui.relative_to(ui_root).as_posix()
            dest = dest_ui / "lang" / lang / page_lang_filename(mapped, kind)
            write_lang_file(dest, lang, mapped, kind, extract_defines(src.read_text(encoding="utf-8")))
            present[(kind, lang)].add(mapped)

        shutil.rmtree(pages)

        for kind in ("strings", "layout"):
            found = present[(kind, lang)]
            includes = [
                include_of(lang, ui_rel, kind)
                for ui_rel in INCLUDE_ORDER
                if ui_rel in found
            ]
            write_aggregator(lang_root / lang / f"{kind}.h", lang, kind, includes)
            print(f"{lang_root / lang / kind}.h: {len(includes)} includes")


def main() -> int:
    root = Path(__file__).resolve().parents[5]
    for ui in ("v3_ui_v7", "v3pro_ui_v7"):
        ui_root = root / "overlay" / "ui" / ui
        if (ui_root / "lang").is_dir():
            relocate_lang_tree(ui_root)
    return 0


if __name__ == "__main__":
    sys.exit(main())
