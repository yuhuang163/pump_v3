#!/bin/bash
# 批量重命名动画帧 PNG：保留原文件名中的 5 位序号，替换为指定前缀
#
# 示例:
#   资源-吸乳模式360px_00000.png  ->  X_00000.png
#   资源-吸乳模式360px_00059.png  ->  X_00059.png
#
# 用法:
#   ./png_seq_rename.sh <TARGET_DIR> <PREFIX> [--dry-run]
#
# 参数:
#   TARGET_DIR   要处理的目录
#   PREFIX       新文件名前缀（如 X、C、S），生成格式: {PREFIX}_{5位序号}.{原扩展名}
#   --dry-run    可选：仅预览，不实际改名

# 禁止 source/. 执行：脚本内 exit / set -e 会关掉或污染当前交互 shell（表现为“闪退”）
if [[ "${BASH_SOURCE[0]}" != "$0" ]]; then
    echo "Error: 请用 bash 或 ./ 执行本脚本，不要用 source 或 '.'" >&2
    echo "  正确: ./tools/png_seq_rename.sh <TARGET_DIR> <PREFIX> [--dry-run]" >&2
    echo "  错误: . tools/png_seq_rename.sh ..." >&2
    return 1
fi

set -euo pipefail

usage() {
    echo "用法: $0 <TARGET_DIR> <PREFIX> [--dry-run]"
    echo ""
    echo "  TARGET_DIR  要处理的目录"
    echo "  PREFIX      新文件名前缀，生成格式: {PREFIX}_{5位序号}.{原扩展名}"
    echo "              例如 PREFIX=C 时: C_00000.png, C_00001.png, ..."
    echo "  --dry-run   可选：仅预览将要执行的重命名，不实际改名"
    echo ""
    echo "说明:"
    echo "  - 从原文件名末尾提取 5 位数字序号（如 _00059），序号保持不变"
    echo "  - 文件扩展名保持不变"
    echo "  - 已符合目标格式的文件将跳过"
    echo ""
    echo "示例:"
    echo "  $0 ../v3_res_v6/base/animation/long_suction/png X"
    echo "  $0 ../v3_res_v6/base/animation/mixed_suction/png C --dry-run"
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR=""
PREFIX=""
DRY_RUN=0

for arg in "$@"; do
    case "$arg" in
        -h|--help)
            usage
            exit 0
            ;;
        --dry-run)
            DRY_RUN=1
            ;;
        -*)
            echo "未知参数: $arg"
            usage
            exit 1
            ;;
        *)
            if [ -z "$TARGET_DIR" ]; then
                TARGET_DIR="$arg"
            elif [ -z "$PREFIX" ]; then
                PREFIX="$arg"
            else
                echo "多余参数: $arg"
                usage
                exit 1
            fi
            ;;
    esac
done

if [ -z "$TARGET_DIR" ] || [ -z "$PREFIX" ]; then
    echo "Error: 必须指定 TARGET_DIR 和 PREFIX"
    usage
    exit 1
fi

# 支持相对路径（相对于当前工作目录或脚本目录）
if [[ "$TARGET_DIR" != /* ]]; then
    if [ -d "$TARGET_DIR" ]; then
        TARGET_DIR="$(cd "$TARGET_DIR" && pwd)"
    elif [ -d "${SCRIPT_DIR}/${TARGET_DIR}" ]; then
        TARGET_DIR="$(cd "${SCRIPT_DIR}/${TARGET_DIR}" && pwd)"
    fi
fi

if [ ! -d "$TARGET_DIR" ]; then
    echo "Error: 目录不存在: $TARGET_DIR"
    exit 1
fi

if [[ "$PREFIX" == */* ]]; then
    echo "Error: 第二个参数是 PREFIX（如 S、X、C），不是输出目录"
    echo "  本脚本在 TARGET_DIR 内原地改名，格式: {PREFIX}_00000.png"
    echo "  示例: $0 v3_res_v7/base/animation/stimulation/png S"
    exit 1
fi

if [[ ! "$PREFIX" =~ ^[A-Za-z0-9_+-]+$ ]]; then
    echo "Error: PREFIX 仅允许字母、数字及 _ + -"
    exit 1
fi

echo "================================================"
echo "目标目录: $TARGET_DIR"
echo "新前缀:   ${PREFIX}_#####"
echo "模式:     $([ "$DRY_RUN" -eq 1 ] && echo "dry-run(预览)" || echo "执行改名")"
echo "================================================"

renamed=0
skipped=0

# 收集待处理文件：按序号数值排序，保证处理顺序与帧序一致
mapfile -d '' -t files < <(
    find "$TARGET_DIR" -maxdepth 1 -type f -print0 | sort -z
)

# 两阶段改名，避免目标名与现有文件冲突
declare -a pending_src=()
declare -a pending_orig=()
declare -a pending_dest=()

for src in "${files[@]}"; do
    [ -n "$src" ] || continue

    base="$(basename "$src")"
    ext="${base##*.}"
    name_no_ext="${base%.*}"

    # 已符合目标格式: PREFIX_00000
    if [[ "$name_no_ext" =~ ^${PREFIX}_[0-9]{5}$ ]]; then
        echo "跳过(已是目标格式): $base"
        skipped=$((skipped + 1))
        continue
    fi

    # 提取末尾 5 位序号（支持 xxx_00059 或 xxx-00059 等）
    if [[ "$name_no_ext" =~ _([0-9]{5})$ ]]; then
        seq="${BASH_REMATCH[1]}"
    elif [[ "$name_no_ext" =~ -([0-9]{5})$ ]]; then
        seq="${BASH_REMATCH[1]}"
    elif [[ "$name_no_ext" =~ ([0-9]{5})$ ]]; then
        seq="${BASH_REMATCH[1]}"
    else
        echo "跳过(未找到5位序号): $base"
        skipped=$((skipped + 1))
        continue
    fi

    dest="${TARGET_DIR}/${PREFIX}_${seq}.${ext}"

    if [ "$src" = "$dest" ]; then
        skipped=$((skipped + 1))
        continue
    fi

    if [ -e "$dest" ] && [ "$(realpath "$src")" != "$(realpath "$dest")" ]; then
        echo "Error: 目标文件已存在，中止: $dest"
        exit 1
    fi

    pending_src+=("$src")
    pending_orig+=("$base")
    pending_dest+=("$dest")
done

if [ "$DRY_RUN" -eq 1 ]; then
    for i in "${!pending_dest[@]}"; do
        echo "改名: ${pending_orig[$i]} -> $(basename "${pending_dest[$i]}")"
    done
    renamed=${#pending_dest[@]}
else
    # 阶段1: 改为临时名
    tmp_suffix=".renaming_$$"
    for i in "${!pending_src[@]}"; do
        src="${pending_src[$i]}"
        tmp="${src}${tmp_suffix}"
        mv -f -- "$src" "$tmp"
        pending_src[$i]="$tmp"
    done

    # 阶段2: 临时名改为最终名
    for i in "${!pending_dest[@]}"; do
        src="${pending_src[$i]}"
        dest="${pending_dest[$i]}"
        mv -f -- "$src" "$dest"
        echo "改名: ${pending_orig[$i]} -> $(basename "$dest")"
        renamed=$((renamed + 1))
    done
fi

echo "================================================"
echo "完成：改名 $renamed 个，跳过 $skipped 个"
echo "================================================"
