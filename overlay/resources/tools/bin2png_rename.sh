#!/bin/bash
# 一键将目录下的 .bin 文件后缀改为 .png（仅重命名，不做格式转换）
# 默认处理 overlay/resources/img/ 目录
#
# 用法:
#   ./bin2png_rename.sh [TARGET_DIR] [--dry-run] [--force]
#
# 参数:
#   TARGET_DIR   可选：要处理的目录，默认：<脚本目录>/../img
#   --dry-run    可选：只打印将要执行的重命名操作，不实际改名
#   --force      可选：如果目标 .png 已存在，则覆盖

set -euo pipefail

usage() {
    echo "用法: $0 [TARGET_DIR] [--dry-run] [--force]"
    echo ""
    echo "  TARGET_DIR  可选：要处理的目录（默认：<脚本目录>/../img）"
    echo "  --dry-run   可选：仅预览将要执行的重命名，不实际改名"
    echo "  --force     可选：若目标 .png 已存在，则覆盖"
    echo ""
    echo "说明：此脚本仅修改文件名后缀（.bin -> .png），不进行图片内容格式转换。"
}

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="${SCRIPT_DIR}/../img"
DRY_RUN=0
FORCE=0

# 解析参数（支持把第一个非flag参数当作目录）
for arg in "$@"; do
    case "$arg" in
        -h|--help)
            usage
            exit 0
            ;;
        --dry-run)
            DRY_RUN=1
            ;;
        --force)
            FORCE=1
            ;;
        *)
            # 第一个非flag参数作为目录
            if [[ "$arg" != -* ]]; then
                TARGET_DIR="$arg"
            else
                echo "未知参数: $arg"
                usage
                exit 1
            fi
            ;;
    esac
done

if [ ! -d "$TARGET_DIR" ]; then
    echo "Error: 目录不存在: $TARGET_DIR"
    exit 1
fi

echo "================================================"
echo "目标目录: $TARGET_DIR"
echo "模式: $([ "$DRY_RUN" -eq 1 ] && echo "dry-run(预览)" || echo "执行改名")"
echo "覆盖: $([ "$FORCE" -eq 1 ] && echo "是(--force)" || echo "否(存在则跳过)")"
echo "================================================"

renamed=0
skipped=0

# 使用 -print0 处理包含空格/括号的文件名
while IFS= read -r -d '' src; do
    dest="${src%.*}.png"

    if [ -e "$dest" ] && [ "$FORCE" -ne 1 ]; then
        echo "跳过(已存在): $dest"
        skipped=$((skipped + 1))
        continue
    fi

    if [ "$DRY_RUN" -eq 1 ]; then
        echo "mv -- '$src' '$dest'"
    else
        mv -f -- "$src" "$dest"
    fi

    renamed=$((renamed + 1))
done < <(find "$TARGET_DIR" -type f \( -name "*.bin" -o -name "*.BIN" \) -print0)

echo "================================================"
echo "完成：改名 $renamed 个，跳过 $skipped 个"
echo "================================================"


