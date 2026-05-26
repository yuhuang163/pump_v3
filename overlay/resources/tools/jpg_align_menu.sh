#!/bin/bash
# JPG/PNG转JPG，对齐与质量配置重新压缩工具
# 用法: ./jpg_align_menu.sh <输入目录> [输出目录]

set -u

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
    echo "用法: $0 <INPUT_DIRECTORY> [OUTPUT_DIRECTORY]"
    echo ""
    echo "  INPUT_DIRECTORY: 要转换的文件夹（包含JPG/JPEG/PNG文件）"
    echo "  OUTPUT_DIRECTORY (可选): 输出目录，默认创建 out_<INPUT_DIRECTORY_NAME>"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INPUT_DIRECTORY=$1
CUSTOM_OUTPUT_DIR=${2:-}

if [ ! -d "$INPUT_DIRECTORY" ]; then
    echo "错误: 输入目录不存在 - $INPUT_DIRECTORY"
    exit 1
fi

if command -v magick >/dev/null 2>&1; then
    IM_CMD="magick"
elif command -v convert >/dev/null 2>&1; then
    IM_CMD="convert"
else
    echo "错误: 未找到 ImageMagick，请先安装 magick 或 convert"
    exit 1
fi

if command -v identify >/dev/null 2>&1; then
    IDENTIFY_CMD="identify"
elif command -v magick >/dev/null 2>&1; then
    IDENTIFY_CMD="magick identify"
else
    echo "错误: 未找到 identify，无法读取图片尺寸"
    exit 1
fi

echo "================================================"
echo "请选择 JPG 对齐尺寸："
echo "  1) 352x352（360屏居中，四周4px边距）"
echo "  2) 368x368（16对齐，可能超出360屏）"
echo "  3) 保持原始比例，最大边限制到352并向下16对齐"
echo "  4) 保持原始比例，最大边限制到368并向下16对齐"
echo "  5) 自定义固定尺寸（强制拉伸到 WxH）"
echo "  6) 自定义最大边（等比缩放，宽高分别向下16对齐）"
echo "================================================"
read -r -p "请输入选项 [1-6]: " size_choice

SIZE_MODE=""
TARGET_W=0
TARGET_H=0
MAX_SIZE=0

case "$size_choice" in
    1) SIZE_MODE="fixed"; TARGET_W=352; TARGET_H=352 ;;
    2) SIZE_MODE="fixed"; TARGET_W=368; TARGET_H=368 ;;
    3) SIZE_MODE="fit_align_down"; MAX_SIZE=352 ;;
    4) SIZE_MODE="fit_align_down"; MAX_SIZE=368 ;;
    5)
        SIZE_MODE="fixed"
        read -r -p "请输入目标宽度: " TARGET_W
        read -r -p "请输入目标高度: " TARGET_H
        ;;
    6)
        SIZE_MODE="fit_align_down"
        read -r -p "请输入最大边尺寸: " MAX_SIZE
        ;;
    *)
        echo "无效选项，退出."
        exit 1
        ;;
esac

echo ""
echo "================================================"
echo "请选择 JPG 质量："
echo "  1) 95（推荐，体积较小）"
echo "  2) 99（高质量）"
echo "  3) 100（最接近原图，体积最大）"
echo "  4) 自定义 1-100"
echo "================================================"
read -r -p "请输入选项 [1-4]: " quality_choice

case "$quality_choice" in
    1) QUALITY=95 ;;
    2) QUALITY=99 ;;
    3) QUALITY=100 ;;
    4) read -r -p "请输入质量 [1-100]: " QUALITY ;;
    *)
        echo "无效选项，退出."
        exit 1
        ;;
esac

if ! [[ "$QUALITY" =~ ^[0-9]+$ ]] || [ "$QUALITY" -lt 1 ] || [ "$QUALITY" -gt 100 ]; then
    echo "错误: 质量必须是 1-100 的整数"
    exit 1
fi

echo ""
echo "================================================"
echo "请选择 YUV 采样格式："
echo "  1) YUV444 / 1x1（质量最高，体积较大）"
echo "  2) YUV422 / 2x1（推荐，兼容性和体积平衡）"
echo "  3) YUV420 / 2x2（体积最小）"
echo "================================================"
read -r -p "请输入选项 [1-3]: " sampling_choice

case "$sampling_choice" in
    1) SAMPLING_FACTOR="1x1"; SAMPLING_NAME="444" ;;
    2) SAMPLING_FACTOR="2x1"; SAMPLING_NAME="422" ;;
    3) SAMPLING_FACTOR="2x2"; SAMPLING_NAME="420" ;;
    *)
        echo "无效选项，退出."
        exit 1
        ;;
esac

echo ""
echo "================================================"
echo "请选择透明背景填充色（PNG转JPG时使用）："
echo "  1) black（适合黑色背景UI）"
echo "  2) white（适合白色背景图片）"
echo "  3) 自定义颜色（例如 #000000 / #ffffff / none）"
echo "================================================"
read -r -p "请输入选项 [1-3]: " background_choice

case "$background_choice" in
    1) BACKGROUND_COLOR="black" ;;
    2) BACKGROUND_COLOR="white" ;;
    3) read -r -p "请输入背景颜色: " BACKGROUND_COLOR ;;
    *)
        echo "无效选项，退出."
        exit 1
        ;;
esac

if [ -z "$CUSTOM_OUTPUT_DIR" ]; then
    OUTPUT_DIR="out_$(basename "$INPUT_DIRECTORY")"
else
    OUTPUT_DIR="$CUSTOM_OUTPUT_DIR"
fi

mkdir -p "$OUTPUT_DIR"

align_down_16() {
    local value=$1
    local aligned=$((value / 16 * 16))
    if [ "$aligned" -lt 16 ]; then
        aligned=16
    fi
    echo "$aligned"
}

get_dimension() {
    local file=$1
    local format=$2
    $IDENTIFY_CMD -format "$format" "$file" 2>/dev/null
}

calc_fit_size() {
    local width=$1
    local height=$2
    local max_size=$3
    local fit_w fit_h

    if [ "$width" -ge "$height" ]; then
        fit_w=$max_size
        fit_h=$((height * max_size / width))
    else
        fit_h=$max_size
        fit_w=$((width * max_size / height))
    fi

    fit_w=$(align_down_16 "$fit_w")
    fit_h=$(align_down_16 "$fit_h")
    echo "${fit_w}x${fit_h}"
}

convert_one_file() {
    local file=$1
    local base_name output_file width height target_size target_w target_h

    base_name=$(basename "$file")
    base_name=${base_name%.*}

    width=$(get_dimension "$file" "%w")
    height=$(get_dimension "$file" "%h")

    if [ -z "$width" ] || [ -z "$height" ]; then
        echo "警告: 无法读取尺寸，跳过 - $file"
        return 1
    fi

    if [ "$SIZE_MODE" = "fixed" ]; then
        target_w=$TARGET_W
        target_h=$TARGET_H
    else
        target_size=$(calc_fit_size "$width" "$height" "$MAX_SIZE")
        target_w=${target_size%x*}
        target_h=${target_size#*x}
    fi

    if ! [[ "$target_w" =~ ^[0-9]+$ ]] || ! [[ "$target_h" =~ ^[0-9]+$ ]]; then
        echo "错误: 目标尺寸无效 - ${target_w}x${target_h}"
        return 1
    fi

    if [ $((target_w % 16)) -ne 0 ] || [ $((target_h % 16)) -ne 0 ]; then
        echo "警告: 目标尺寸 ${target_w}x${target_h} 不是16对齐，建议调整"
    fi

    output_file="$OUTPUT_DIR/${base_name}_${target_w}x${target_h}_q${QUALITY}_${SAMPLING_NAME}.jpg"

    echo "Found image file: $file"
    echo "  原始尺寸: ${width}x${height}"
    echo "  输出尺寸: ${target_w}x${target_h}"
    echo "  质量/采样: q${QUALITY}/YUV${SAMPLING_NAME}"

    "$IM_CMD" "$file" \
        -resize "${target_w}x${target_h}!" \
        -background "$BACKGROUND_COLOR" \
        -alpha remove \
        -alpha off \
        -interlace none \
        -strip \
        -quality "$QUALITY" \
        -sampling-factor "$SAMPLING_FACTOR" \
        "$output_file"

    if [ $? -eq 0 ]; then
        echo "  ✓ 转换成功: $output_file"
    else
        echo "  ✗ 转换失败: $file"
        return 1
    fi
    echo ""
}

echo ""
echo "================================================"
echo "当前配置："
if [ "$SIZE_MODE" = "fixed" ]; then
    echo "  尺寸模式: 固定尺寸 ${TARGET_W}x${TARGET_H}"
else
    echo "  尺寸模式: 等比缩放，最大边 ${MAX_SIZE}，向下16对齐"
fi
echo "  JPG质量: $QUALITY"
echo "  YUV采样: $SAMPLING_NAME ($SAMPLING_FACTOR)"
echo "  透明背景: $BACKGROUND_COLOR"
echo "  输入目录: $INPUT_DIRECTORY"
echo "  输出目录: $OUTPUT_DIR"
echo "================================================"
echo ""

success_count=0
fail_count=0

shopt -s nullglob
traverse_directory() {
    local dir=$1
    local file

    for file in "$dir"/*; do
        if [ -d "$file" ]; then
            traverse_directory "$file"
        elif [ -f "$file" ]; then
            case "$file" in
                *.jpg|*.jpeg|*.JPG|*.JPEG|*.png|*.PNG)
                    if convert_one_file "$file"; then
                        success_count=$((success_count + 1))
                    else
                        fail_count=$((fail_count + 1))
                    fi
                    ;;
            esac
        fi
    done
}

traverse_directory "$INPUT_DIRECTORY"

echo "================================================"
echo "转换完成: 成功 ${success_count} 个, 失败 ${fail_count} 个"
echo "输出目录: $OUTPUT_DIR"
echo "================================================"

if [ "$fail_count" -ne 0 ] || [ "$success_count" -eq 0 ]; then
    exit 1
fi
