#!/usr/bin/env python3
"""
Map section size summarizer

- 统计并汇总 .text / .data / .bss / .rodata 大小
- 尽量避免因总行与子行同时计入而导致的重复统计：
  - 优先累计来自目标文件/静态库成员（包含 .o/.obj/.a( 等特征）的行
  - 若没有发现上述“叶子行”，则退回使用可能的总行数值

用法：
  python3 scripts/map_section_sizes.py [<path/to/file.map>] [--out-dir DIR] [--top-files N]
  若未传入 <path/to/file.map>，将使用脚本内部默认路径。
  脚本仅生成“按文件汇总”的 Markdown 表格（.text/.rodata/.data/.bss），不再输出总数据统计或合并报告。

输出：
  按大小从大到小排序，显示字节与KB
"""

from __future__ import annotations

import argparse
import os
import re
from collections import defaultdict
from typing import Dict, Tuple, List


SECTION_BASES = [".text", ".data", ".bss", ".rodata"]

# 默认 map 文件路径（可按需修改）
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_MAP_PATH = os.path.abspath(os.path.join(SCRIPT_DIR, "..", "project", "bin", "ble_only_lt.map"))


def extract_source_identifier(line: str) -> str:
    # 先匹配 libxxx.a(member.o) 形式
    m = re.search(r"([^\s]*?\.a\([^\)]+\.o\))", line)
    if m:
        return m.group(1)
    # 再匹配任意 *.o 或 *.obj（含路径）
    m = re.search(r"([^\s]*?\.(?:o|obj))\b", line)
    if m:
        return m.group(1)
    # 回退：使用末尾 token
    tokens = line.strip().split()
    return tokens[-1] if tokens else line.strip()


def collect_leaf_contributions(map_path: str) -> Dict[str, Dict[str, int]]:
    # 返回：{ source_path: { section_base: size_sum, ... }, ... }
    section_lead = re.compile(r"^\s*(\.[A-Za-z0-9_\.]+)\b")
    number_finder = re.compile(r"0x[0-9A-Fa-f]+|\d+")
    per_source: Dict[str, Dict[str, int]] = {}

    with open(map_path, 'r', encoding='utf-8', errors='ignore') as f:
        pending_subsection: Tuple[str, bool] | None = None
        pending_source: str | None = None
        for raw_line in f:
            line = raw_line.rstrip("\n")
            m = section_lead.match(line)
            if m:
                base_or_sub = m.group(1)
                base = detect_base_section(base_or_sub)
                if base is None:
                    pending_subsection = None
                    pending_source = None
                    continue
                search_region = line[m.end():]
                nums = number_finder.findall(search_region)
                if len(nums) >= 2:
                    size_val = parse_size_token(nums[1])
                    if size_val > 0 and looks_like_object_contribution(line):
                        src = extract_source_identifier(line)
                        if src not in per_source:
                            per_source[src] = {s: 0 for s in SECTION_BASES}
                        per_source[src][base] += size_val
                        pending_subsection = None
                        pending_source = None
                        continue
                # 两行形式
                if base_or_sub.count('.') >= 2 or base_or_sub.startswith(base + '.'):
                    pending_subsection = (base, True)
                    pending_source = None
                else:
                    pending_subsection = None
                    pending_source = None
                continue

            if pending_subsection is not None:
                base, _ = pending_subsection
                nums = number_finder.findall(line)
                if len(nums) >= 2:
                    size_val = parse_size_token(nums[1])
                    if size_val > 0 and looks_like_object_contribution(line):
                        src = extract_source_identifier(line)
                        if src not in per_source:
                            per_source[src] = {s: 0 for s in SECTION_BASES}
                        per_source[src][base] += size_val
                pending_subsection = None
                pending_source = None

    return per_source


def render_markdown_by_file(per_source: Dict[str, Dict[str, int]]) -> str:
    # 计算总和用于排序
    rows: List[Tuple[str, int, Dict[str, int]]] = []
    for src, sec_map in per_source.items():
        total = sum(sec_map.get(s, 0) for s in SECTION_BASES)
        rows.append((src, total, sec_map))
    rows.sort(key=lambda x: x[1], reverse=True)

    def fmt_kb(n: int) -> str:
        return f"{n/1024.0:.2f} KB"

    # 表头
    out: List[str] = []
    out.append("| File | .text | .rodata | .data | .bss | Total |")
    out.append("|---|---:|---:|---:|---:|---:|")

    for src, total, sec_map in rows:
        text_sz = sec_map.get(".text", 0)
        ro_sz = sec_map.get(".rodata", 0)
        data_sz = sec_map.get(".data", 0)
        bss_sz = sec_map.get(".bss", 0)
        out.append(
            f"| {src} | {fmt_kb(text_sz)} | {fmt_kb(ro_sz)} | {fmt_kb(data_sz)} | {fmt_kb(bss_sz)} | {fmt_kb(total)} |"
        )

    return "\n".join(out) + "\n"


def parse_size_token(token: str) -> int:
    token = token.strip().lower()
    try:
        if token.startswith("0x"):
            return int(token, 16)
        return int(token, 10)
    except ValueError:
        return 0


def detect_base_section(section_token: str) -> str | None:
    token = section_token.strip()
    if not token.startswith('.'):
        return None
    # 只取基础节名：.text/.data/.bss/.rodata
    for base in SECTION_BASES:
        if token == base or token.startswith(base + '.') or token.startswith(base + ' '):
            return base
    return None


def looks_like_object_contribution(line: str) -> bool:
    # 判断该行是否看起来像来自目标文件/静态库成员的“叶子行”，以避免统计总行
    # 更严格：要求出现 .o/.obj 或 存在 .a(XXX.o)
    return bool(re.search(r"(\.o\b|\.obj\b|\.a\([^\)]+\.o\))", line))


def parse_map_file(path: str, mode: str = "auto", debug: bool = False, top_n: int = 20) -> Dict[str, int]:
    sum_leaf: Dict[str, int] = defaultdict(int)
    sum_total_like: Dict[str, int] = defaultdict(int)

    # 提取：<section> <addr> <size> ...
    # 我们不强依赖列对齐，只要能拿到两个数字（第一个通常为地址，第二个为大小）
    section_lead = re.compile(r"^\s*(\.[A-Za-z0-9_\.]+)\b")
    number_finder = re.compile(r"0x[0-9A-Fa-f]+|\d+")

    debug_records_leaf: List[Tuple[str, int, str]] = []  # (base, size, line)
    debug_records_total: List[Tuple[str, int, str]] = []

    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        pending_subsection: Tuple[str, bool] | None = None  # (base_section, expect_numbers_next_line)
        for raw_line in f:
            line = raw_line.rstrip("\n")

            m = section_lead.match(line)
            if m:
                base_or_sub = m.group(1)
                base = detect_base_section(base_or_sub)
                if base is None:
                    pending_subsection = None
                    continue

                # 只在节名之后查找数字，避免将节名中的数字（如 .bss.le_fc.4 的 4）误当成地址或大小
                search_region = line[m.end():]
                nums = number_finder.findall(search_region)
                if len(nums) >= 2:
                    size_val = parse_size_token(nums[1])
                    if size_val > 0:
                        if looks_like_object_contribution(line):
                            sum_leaf[base] += size_val
                            if debug:
                                debug_records_leaf.append((base, size_val, line.strip()))
                        else:
                            sum_total_like[base] += size_val
                            if debug:
                                debug_records_total.append((base, size_val, line.strip()))
                        pending_subsection = None
                        continue

                # 处理两行形式：第一行只有名称（如 .text.func），下一行才有地址与大小
                if base_or_sub.count('.') >= 2 or base_or_sub.startswith(base + '.'):
                    pending_subsection = (base, True)
                else:
                    pending_subsection = None
                continue

            # 如果上一行是子节名，这一行尝试读取数值
            if pending_subsection is not None:
                base, _ = pending_subsection
                # 下一行也可能以空白开头，不再包含节名，直接整体查找
                nums = number_finder.findall(line)
                if len(nums) >= 2:
                    size_val = parse_size_token(nums[1])
                    if size_val > 0:
                        if looks_like_object_contribution(line):
                            sum_leaf[base] += size_val
                            if debug:
                                debug_records_leaf.append((base, size_val, line.strip()))
                        else:
                            sum_total_like[base] += size_val
                            if debug:
                                debug_records_total.append((base, size_val, line.strip()))
                pending_subsection = None

    # 选择最终结果
    result: Dict[str, int] = {}
    for base in SECTION_BASES:
        leaf_sum = sum_leaf.get(base, 0)
        total_sum = sum_total_like.get(base, 0)
        if mode == "leaf":
            result[base] = leaf_sum
        elif mode == "total":
            result[base] = total_sum
        else:  # auto
            result[base] = leaf_sum if leaf_sum > 0 else total_sum
    return result

    # Note: debug output handled in main


def format_size(nbytes: int) -> str:
    kb = nbytes / 1024.0
    return f"{nbytes} B\t({kb:.2f} KB)"


def main() -> None:
    parser = argparse.ArgumentParser(description="Summarize per-file section sizes from a GNU ld .map file")
    parser.add_argument("map_file", nargs='?', default=DEFAULT_MAP_PATH, help=f"Path to .map file（默认: {DEFAULT_MAP_PATH}）")
    parser.add_argument("--out-dir", default=None, help="结果输出目录；默认自动定位 project/bin，若未找到则使用 map 文件所在目录")
    parser.add_argument("--top-files", type=int, default=0, help="仅输出前 N 个文件（按总大小降序）；0 为全部")
    parser.add_argument("--debug", action="store_true", help="输出最大若干贡献行以调试")
    parser.add_argument("--top", type=int, default=20, help="调试模式下显示 Top N，默认 20")
    args = parser.parse_args()

    map_path = os.path.abspath(args.map_file)
    if not os.path.isfile(map_path):
        raise SystemExit(f"Map 文件不存在: {map_path}")

    def determine_out_dir(map_file_path: str, user_out_dir: str | None) -> str:
        if user_out_dir:
            out_dir = os.path.abspath(user_out_dir)
            os.makedirs(out_dir, exist_ok=True)
            return out_dir
        # 尝试在路径中查找名为 project 的目录，并使用其中的 bin 目录
        cur_dir = os.path.dirname(os.path.abspath(map_file_path))
        parts = cur_dir.split(os.sep)
        if "project" in parts:
            idx = parts.index("project")
            candidate = os.sep.join(parts[: idx + 1])
            out_dir = os.path.join(candidate, "bin")
            try:
                os.makedirs(out_dir, exist_ok=True)
                return out_dir
            except Exception:
                pass
        # 回退到 map 所在目录
        return cur_dir

    out_dir = determine_out_dir(map_path, args.out_dir)

    base = os.path.splitext(os.path.basename(map_path))[0]

    # 仅生成“按文件汇总”的 Markdown 表格
    per_source = collect_leaf_contributions(map_path)
    # 若限制 top，则裁剪
    if args.top_files and args.top_files > 0:
        # 重排后裁剪
        ordered = sorted(
            per_source.items(),
            key=lambda kv: sum(kv[1].get(s, 0) for s in SECTION_BASES),
            reverse=True,
        )
        per_source = {k: v for k, v in ordered[: args.top_files]}
    md_text = render_markdown_by_file(per_source)
    md_path = os.path.join(out_dir, f"{base}_per_file_sections.md")
    with open(md_path, "w", encoding="utf-8") as wf:
        wf.write(md_text)
    print(f"Wrote: {md_path}")

    if args.debug:
        print("\n[DEBUG] Top 贡献（leaf）:")
        # 重新解析以获取记录，不重复实现：简单重新读取并填充记录
        # 由于上面函数返回后无法获取 debug_records_*，这里简单地在调试下再次扫描并打印记录
        # 为避免重复实现，复制轻量扫描逻辑：
        section_lead = re.compile(r"^\s*(\.[A-Za-z0-9_\.]+)\b")
        number_finder = re.compile(r"0x[0-9A-Fa-f]+|\d+")
        dbg_leaf: List[Tuple[str, int, str]] = []
        dbg_total: List[Tuple[str, int, str]] = []
        with open(map_path, 'r', encoding='utf-8', errors='ignore') as f:
            pending_subsection: Tuple[str, bool] | None = None
            for raw_line in f:
                line = raw_line.rstrip("\n")
                m = section_lead.match(line)
                if m:
                    base_or_sub = m.group(1)
                    base = detect_base_section(base_or_sub)
                    if base is None:
                        pending_subsection = None
                        continue
                    search_region = line[m.end():]
                    nums = number_finder.findall(search_region)
                    if len(nums) >= 2:
                        size_val = parse_size_token(nums[1])
                        if size_val > 0:
                            if looks_like_object_contribution(line):
                                dbg_leaf.append((base, size_val, line.strip()))
                            else:
                                dbg_total.append((base, size_val, line.strip()))
                            pending_subsection = None
                            continue
                    if base_or_sub.count('.') >= 2 or base_or_sub.startswith(base + '.'):
                        pending_subsection = (base, True)
                    else:
                        pending_subsection = None
                    continue
                if pending_subsection is not None:
                    base, _ = pending_subsection
                    nums = number_finder.findall(line)
                    if len(nums) >= 2:
                        size_val = parse_size_token(nums[1])
                        if size_val > 0:
                            if looks_like_object_contribution(line):
                                dbg_leaf.append((base, size_val, line.strip()))
                            else:
                                dbg_total.append((base, size_val, line.strip()))
                    pending_subsection = None

        for title, arr in [("leaf", dbg_leaf), ("total", dbg_total)]:
            arr.sort(key=lambda x: x[1], reverse=True)
            print(f"\n[DEBUG] Top {min(args.top, len(arr))} - {title}：")
            for base, size_val, src in arr[: args.top]:
                print(f"{base}\t{size_val}\t{src}")


if __name__ == "__main__":
    main()


