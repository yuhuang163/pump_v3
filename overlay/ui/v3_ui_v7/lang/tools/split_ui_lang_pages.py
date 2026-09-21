#!/usr/bin/env python3
"""Split lang/{en,zh}/strings.h and layout.h into pages/<page>/<theme> files."""

from __future__ import annotations

import re
import sys
from collections import OrderedDict
from pathlib import Path

DEFINE_RE = re.compile(r"^#define\s+([A-Z][A-Z0-9_]+)\b")
COMMENTED_DEFINE_RE = re.compile(r"^//\s*#define\s+([A-Z][A-Z0-9_]+)\b")

# Longest prefix first.
LAYOUT_RULES = (
    ("THEME_GUIDE_", "pages/theme_guide/layout.h"),
    ("PROGRESS_ARC_", "pages/common/layout.h"),
    ("POPUP_WINDOW_BLE_", "pages/ble/layout.h"),
    ("SYS_ABNORMAL_", "pages/sys_error/layout.h"),
    ("HOME_CUSTOM_", "pages/home/custom/layout.h"),
    ("HOME_SAVER_", "pages/home/saver/layout.h"),
    ("HOME_MODE_PAGE_", "pages/home/mode/layout.h"),
    ("HOME_BOTH_LEVEL_", "pages/home/both_level/layout.h"),
    ("HOME_LEVEL_", "pages/home/level/layout.h"),
    ("HOME_SPEED_", "pages/home/speed/layout.h"),
    ("HOME_MAIN_", "pages/home/base/layout.h"),
    ("PUMP_HOME_CUSTOM_", "pages/pump/custom/layout.h"),
    ("PUMP_HOME_MAIN_", "pages/pump/base/layout.h"),
    ("PUMP_MODE_PAGE_", "pages/pump/mode/layout.h"),
    ("PUMP_BOTH_LEVEL_", "pages/pump/both_level/layout.h"),
    ("PUMP_LEVEL_", "pages/pump/level/layout.h"),
    ("PUMP_SPEED_", "pages/pump/speed/layout.h"),
    ("PUMP_STOP_HOLD_", "pages/pump/stop_hold/layout.h"),
    ("PUMP_PAUSE_TIMEOUT_", "pages/pump/pause_timeout/layout.h"),
    ("PUMP_FINISH_TIMEOUT_", "pages/pump/finish_timeout/layout.h"),
    ("PUMP_WILL_FINISH_", "pages/pump/will_finish/layout.h"),
    ("PUMP_FINISH_", "pages/pump/finish/layout.h"),
    ("PUMP_RUNING_30M_", "pages/pump/tips/layout.h"),
    ("PUMP_REST_MODE_", "pages/pump/tips/layout.h"),
    ("PUMP_SEAL_", "pages/seal/layout.h"),
    ("PUMP_FIT_", "pages/seal/layout.h"),
    ("GUIDE_INTRO_", "pages/guide/intro/layout.h"),
    ("GUIDE_QRCODE_", "pages/guide/qrcode/layout.h"),
    ("GUIDE_FINISH_", "pages/guide/finish/layout.h"),
    ("GUIDE_MODE_HOME_", "pages/guide/mode_home/layout.h"),
    ("GUIDE_MODE_SWITCH_", "pages/guide/mode_switch/layout.h"),
    ("GUIDE_PUMP_HOME_", "pages/guide/pump_home/layout.h"),
    ("GUIDE_PUMP_START_", "pages/guide/pump_start/layout.h"),
    ("GUIDE_PUMP_PAUSE_", "pages/guide/pump_pause/layout.h"),
    ("GUIDE_PUMP_FINISH_", "pages/guide/pump_finish/layout.h"),
    ("GUIDE_SPEED_", "pages/guide/speed/layout.h"),
    ("GUIDE_LEVEL_SIDE_", "pages/guide/level_side/layout.h"),
    ("GUIDE_LEVEL_BOTH_", "pages/guide/level_both/layout.h"),
    ("GUIDE_BOTH_LEVEL_", "pages/guide/level_both/layout.h"),
    ("RHYTHM_", "pages/rhythm/layout.h"),
    ("SETTINGS_", "pages/settings/layout.h"),
    ("AIRPLANE_", "pages/power/airplane/layout.h"),
    ("TRAVEL_LOCK_", "pages/power/travel_lock/layout.h"),
    ("POWER_ON_HOLD_", "pages/power/power_on/layout.h"),
    ("POWER_OFF_HOLD_", "pages/power/power_off_hold/layout.h"),
    ("POWER_OFF_CHARGE_", "pages/power/power_off_charge/layout.h"),
    ("POWER_OFF_TEXT_", "pages/power/power_off/layout.h"),
    ("LOW_POWER_", "pages/power/low_power/layout.h"),
    ("CHARGE_", "pages/charge/layout.h"),
    ("OTA_", "pages/ota/layout.h"),
    ("FACTORY_RESET_", "pages/factory/layout.h"),
)

STRING_RULES = (
    ("UI_STR_", "pages/common/strings.h"),
    ("THEME_GUIDE_", "pages/theme_guide/strings.h"),
    ("PRODUCT_TESTS_", "pages/product_tests/strings.h"),
    ("HOME_CUSTOM_", "pages/pump/custom/strings.h"),
    ("HOME_SAVER_", "pages/home/saver/strings.h"),
    ("HOME_BOTH_LEVEL_", "pages/home/both_level/strings.h"),
    ("HOME_LEVEL_", "pages/home/level/strings.h"),
    ("HOME_SPEED_", "pages/home/speed/strings.h"),
    ("HOME_LOW_POWER_", "pages/home/low_power/strings.h"),
    ("HOME_MAIN_", "pages/home/base/strings.h"),
    ("PUMP_HOME_CUSTOM_", "pages/pump/custom/strings.h"),
    ("PUMP_HOME_MAIN_", "pages/pump/base/strings.h"),
    ("PUMP_BOTH_LEVEL_", "pages/pump/both_level/strings.h"),
    ("PUMP_LEVEL_", "pages/pump/level/strings.h"),
    ("PUMP_SPEED_", "pages/pump/speed/strings.h"),
    ("PUMP_STOP_HOLD_", "pages/pump/stop_hold/strings.h"),
    ("PUMP_PAUSE_TIMEOUT_", "pages/pump/pause_timeout/strings.h"),
    ("PUMP_FINISH_TIMEOUT_", "pages/pump/finish_timeout/strings.h"),
    ("PUMP_WILL_FINISH_", "pages/pump/will_finish/strings.h"),
    ("PUMP_FINISH_", "pages/pump/finish/strings.h"),
    ("PUMP_RUNING_30M_", "pages/pump/tips/strings.h"),
    ("PUMP_REST_MODE_", "pages/pump/tips/strings.h"),
    ("PUMP_SEAL_", "pages/seal/strings.h"),
    ("PUMP_FIT_", "pages/seal/strings.h"),
    ("GUIDE_INTRO_", "pages/guide/intro/strings.h"),
    ("GUIDE_QRCODE_", "pages/guide/qrcode/strings.h"),
    ("GUIDE_FINISH_", "pages/guide/finish/strings.h"),
    ("GUIDE_MODE_SWITCH_", "pages/guide/mode_switch/strings.h"),
    ("GUIDE_PUMP_HOME_", "pages/guide/pump_home/strings.h"),
    ("GUIDE_PUMP_START_", "pages/guide/pump_start/strings.h"),
    ("GUIDE_PUMP_PAUSE_", "pages/guide/pump_pause/strings.h"),
    ("GUIDE_PUMP_FINISH_", "pages/guide/pump_finish/strings.h"),
    ("GUIDE_SPEED_", "pages/guide/speed/strings.h"),
    ("GUIDE_LEVEL_SIDE_", "pages/guide/level_side/strings.h"),
    ("GUIDE_LEVEL_BOTH_", "pages/guide/level_both/strings.h"),
    ("GUIDE_BOTH_LEVEL_", "pages/guide/level_both/strings.h"),
    ("RHYTHM_", "pages/rhythm/strings.h"),
    ("SETTINGS_", "pages/settings/strings.h"),
    ("AIRPLANE_", "pages/power/airplane/strings.h"),
    ("TRAVEL_LOCK_", "pages/power/travel_lock/strings.h"),
    ("POWER_ON_HOLD_", "pages/power/power_on/strings.h"),
    ("POWER_OFF_HOLD_", "pages/power/power_off_hold/strings.h"),
    ("POWER_OFF_CHARGE_", "pages/power/power_off_charge/strings.h"),
    ("POWER_OFF_TEXT_", "pages/power/power_off/strings.h"),
    ("RESTARTING_", "pages/power/power_off/strings.h"),
    ("LOW_POWER_", "pages/power/low_power/strings.h"),
    ("CHARGE_", "pages/charge/strings.h"),
    ("OTA_", "pages/ota/strings.h"),
    ("FACTORY_RESET_", "pages/factory/strings.h"),
    ("SYS_ERROR_", "pages/sys_error/strings.h"),
    ("TEMP_ERROR_", "pages/sys_error/strings.h"),
    ("BLE_", "pages/ble/strings.h"),
)

INCLUDE_ORDER = [
    "pages/common/strings.h",
    "pages/common/layout.h",
    "pages/home/base/strings.h",
    "pages/home/base/layout.h",
    "pages/home/custom/layout.h",
    "pages/home/saver/strings.h",
    "pages/home/saver/layout.h",
    "pages/home/mode/layout.h",
    "pages/home/level/strings.h",
    "pages/home/level/layout.h",
    "pages/home/both_level/strings.h",
    "pages/home/both_level/layout.h",
    "pages/home/speed/strings.h",
    "pages/home/speed/layout.h",
    "pages/home/low_power/strings.h",
    "pages/pump/base/strings.h",
    "pages/pump/base/layout.h",
    "pages/pump/custom/strings.h",
    "pages/pump/custom/layout.h",
    "pages/pump/mode/layout.h",
    "pages/pump/level/strings.h",
    "pages/pump/level/layout.h",
    "pages/pump/both_level/strings.h",
    "pages/pump/both_level/layout.h",
    "pages/pump/speed/strings.h",
    "pages/pump/speed/layout.h",
    "pages/pump/stop_hold/strings.h",
    "pages/pump/stop_hold/layout.h",
    "pages/pump/pause_timeout/strings.h",
    "pages/pump/pause_timeout/layout.h",
    "pages/pump/finish_timeout/strings.h",
    "pages/pump/finish_timeout/layout.h",
    "pages/pump/will_finish/strings.h",
    "pages/pump/will_finish/layout.h",
    "pages/pump/finish/strings.h",
    "pages/pump/finish/layout.h",
    "pages/pump/tips/strings.h",
    "pages/pump/tips/layout.h",
    "pages/rhythm/strings.h",
    "pages/rhythm/layout.h",
    "pages/settings/strings.h",
    "pages/settings/layout.h",
    "pages/power/power_off_hold/strings.h",
    "pages/power/power_off_hold/layout.h",
    "pages/power/power_off/strings.h",
    "pages/power/power_off/layout.h",
    "pages/power/power_on/strings.h",
    "pages/power/power_on/layout.h",
    "pages/power/airplane/strings.h",
    "pages/power/airplane/layout.h",
    "pages/power/travel_lock/strings.h",
    "pages/power/travel_lock/layout.h",
    "pages/power/low_power/strings.h",
    "pages/power/low_power/layout.h",
    "pages/power/power_off_charge/strings.h",
    "pages/power/power_off_charge/layout.h",
    "pages/charge/strings.h",
    "pages/charge/layout.h",
    "pages/ota/strings.h",
    "pages/ota/layout.h",
    "pages/factory/strings.h",
    "pages/factory/layout.h",
    "pages/sys_error/strings.h",
    "pages/sys_error/layout.h",
    "pages/ble/strings.h",
    "pages/ble/layout.h",
    "pages/seal/strings.h",
    "pages/seal/layout.h",
    "pages/theme_guide/strings.h",
    "pages/theme_guide/layout.h",
    "pages/guide/intro/strings.h",
    "pages/guide/intro/layout.h",
    "pages/guide/qrcode/strings.h",
    "pages/guide/qrcode/layout.h",
    "pages/guide/finish/strings.h",
    "pages/guide/finish/layout.h",
    "pages/guide/mode_home/layout.h",
    "pages/guide/mode_switch/strings.h",
    "pages/guide/mode_switch/layout.h",
    "pages/guide/pump_home/strings.h",
    "pages/guide/pump_home/layout.h",
    "pages/guide/pump_start/strings.h",
    "pages/guide/pump_start/layout.h",
    "pages/guide/pump_pause/strings.h",
    "pages/guide/pump_pause/layout.h",
    "pages/guide/pump_finish/strings.h",
    "pages/guide/pump_finish/layout.h",
    "pages/guide/speed/strings.h",
    "pages/guide/speed/layout.h",
    "pages/guide/level_side/strings.h",
    "pages/guide/level_side/layout.h",
    "pages/guide/level_both/strings.h",
    "pages/guide/level_both/layout.h",
    "pages/product_tests/strings.h",
]


def match_group(name: str, rules: tuple[tuple[str, str], ...]) -> str:
    for prefix, rel in rules:
        if name.startswith(prefix):
            return rel
    raise KeyError(name)


def guard_of(lang: str, rel: str) -> str:
    stem = rel.replace("pages/", "").replace("/", "_").replace(".h", "").upper()
    return f"__UI_LANG_{lang.upper()}_{stem}_H__"


def write_page_file(path: Path, lang: str, kind: str, rel: str, lines: list[str]) -> None:
    guard = guard_of(lang, rel)
    note = rel.replace("pages/", "").replace(f"/{kind}.h", "").replace(".h", "")
    body = "\n".join(lines)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        "\n".join(
            [
                f"#ifndef {guard}",
                f"#define {guard}",
                "",
                f"/* {note} — 改这里后重新 pack，不要改页面 C */",
                "",
                body,
                "",
                f"#endif /* {guard} */",
                "",
            ]
        ),
        encoding="utf-8",
    )


def write_aggregator(path: Path, lang: str, kind: str, includes: list[str]) -> None:
    guard = f"__UI_LANG_{lang.upper()}_{kind.upper()}_H__"
    title = "文案源。宏名跨语言必须一致，只改引号内译文。" if kind == "strings" else (
        "坐标/行距/最大宽。按 pages/<窗口>/<主题或子页>/ 拆分。"
    )
    inc_lines = [f'#include "{rel}"' for rel in includes]
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
                *inc_lines,
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


def split_file(src: Path, lang: str, kind: str, rules: tuple[tuple[str, str], ...]) -> set[str]:
    groups: OrderedDict[str, list[str]] = OrderedDict()
    unknown: list[str] = []
    for raw in src.read_text(encoding="utf-8").splitlines():
        line = raw.rstrip()
        stripped = line.strip()
        m = DEFINE_RE.match(stripped) or COMMENTED_DEFINE_RE.match(stripped)
        if not m:
            continue
        name = m.group(1)
        try:
            rel = match_group(name, rules)
        except KeyError:
            unknown.append(name)
            continue
        groups.setdefault(rel, []).append(line if line.startswith("#define") or line.startswith("//") else stripped)
    if unknown:
        raise RuntimeError(f"{src}: ungrouped macros: {unknown}")
    for rel, lines in groups.items():
        write_page_file(src.parent / rel, lang, kind, rel, lines)
    return set(groups)


def split_lang_dir(lang_dir: Path) -> None:
    for lang in ("en", "zh"):
        base = lang_dir / lang
        str_groups = split_file(base / "strings.h", lang, "strings", STRING_RULES)
        lay_groups = split_file(base / "layout.h", lang, "layout", LAYOUT_RULES)
        present = str_groups | lay_groups
        str_inc = [rel for rel in INCLUDE_ORDER if rel in present and rel.endswith("strings.h")]
        lay_inc = [rel for rel in INCLUDE_ORDER if rel in present and rel.endswith("layout.h")]
        write_aggregator(base / "strings.h", lang, "strings", str_inc)
        write_aggregator(base / "layout.h", lang, "layout", lay_inc)
        print(f"{base}: {len(str_inc)} string files, {len(lay_inc)} layout files")


def main() -> int:
    print(
        "split_ui_lang_pages.py 已停用。文案/坐标在各 UI 模块 lang/<zh|en>/ 下，"
        "由 lang/<zh|en>/{strings,layout}.h 汇总。需要搬家请用 relocate_ui_lang.py。",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
