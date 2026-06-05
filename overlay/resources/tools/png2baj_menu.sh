#!/bin/bash
# PNG转BAJ带菜单版本：可选 a4/a8、YUV422/YUV444/动态YUV、图片缩放
# 用法: ./png2baj_menu.sh <输入目录> [输出目录]

# 检查参数个数
if [ $# -lt 1 ] || [ $# -gt 2 ]; then
    echo "用法: $0 <INPUT_DIRECTORY> [OUTPUT_DIRECTORY]"
    echo ""
    echo "  INPUT_DIRECTORY: 要转换的文件夹（包含PNG文件）"
    echo "  OUTPUT_DIRECTORY (可选): 输出目录。默认在INPUT_DIRECTORY下创建out_<INPUT_DIRECTORY_NAME>。"
    exit 1
fi

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 获取输入输出目录
INPUT_DIRECTORY=$1
CUSTOM_OUTPUT_DIR=${2}

# 检查输入目录是否存在
if [ ! -d "$INPUT_DIRECTORY" ]; then
    echo "Error: Input directory $INPUT_DIRECTORY does not exist."
    exit 1
fi

# 显示菜单并获取用户选择
echo "================================================"
echo "请选择转换模式："
echo "  1) a4 + YUV422"
echo "  2) a8 + YUV422"
echo "  3) a4 + YUV444"
echo "  4) a8 + YUV444"
echo "  5) a4 + 动态YUV (尺寸≤336用444，>336用422)"
echo "  6) a8 + 动态YUV (尺寸≤336用444，>336用422)"
echo "================================================"
read -p "请输入选项 [1-6]: " choice

# 校验选项并设置 ALPHA、是否动态YUV
case "$choice" in
    1) ALPHA="a4"; DYNAMIC_YUV=0; SAMPLES="422" ;;
    2) ALPHA="a8"; DYNAMIC_YUV=0; SAMPLES="422" ;;
    3) ALPHA="a4"; DYNAMIC_YUV=0; SAMPLES="444" ;;
    4) ALPHA="a8"; DYNAMIC_YUV=0; SAMPLES="444" ;;
    5) ALPHA="a4"; DYNAMIC_YUV=1; SAMPLES="" ;;
    6) ALPHA="a8"; DYNAMIC_YUV=1; SAMPLES="" ;;
    *)
        echo "无效选项，退出."
        exit 1
        ;;
esac

echo ""
echo "================================================"
echo "请选择图片缩放："
echo "  1) 不缩放（保持原始尺寸）"
echo "  2) 352x352（强制拉伸）"
echo "  3) 368x368（强制拉伸）"
echo "  4) 保持原始比例，最大边352，向下16对齐"
echo "  5) 保持原始比例，最大边368，向下16对齐"
echo "  6) 自定义固定尺寸（强制拉伸到 WxH）"
echo "  7) 自定义最大边（等比缩放，宽高分别向下16对齐）"
echo "  8) 自定义最大边（等比缩放，无需16对齐）"
echo "================================================"
read -r -p "请输入选项 [1-8]: " scale_choice

SIZE_MODE=""
TARGET_W=0
TARGET_H=0
MAX_SIZE=0

case "$scale_choice" in
    1) SIZE_MODE="none" ;;
    2) SIZE_MODE="fixed"; TARGET_W=352; TARGET_H=352 ;;
    3) SIZE_MODE="fixed"; TARGET_W=368; TARGET_H=368 ;;
    4) SIZE_MODE="fit_align_down"; MAX_SIZE=352 ;;
    5) SIZE_MODE="fit_align_down"; MAX_SIZE=368 ;;
    6)
        SIZE_MODE="fixed"
        read -r -p "请输入目标宽度: " TARGET_W
        read -r -p "请输入目标高度: " TARGET_H
        ;;
    7)
        SIZE_MODE="fit_align_down"
        read -r -p "请输入最大边尺寸: " MAX_SIZE
        ;;
    8)
        SIZE_MODE="fit"
        read -r -p "请输入最大边尺寸: " MAX_SIZE
        ;;
    *)
        echo "无效选项，退出."
        exit 1
        ;;
esac

if [ "$SIZE_MODE" = "fixed" ]; then
    if ! [[ "$TARGET_W" =~ ^[0-9]+$ ]] || ! [[ "$TARGET_H" =~ ^[0-9]+$ ]] || \
       [ "$TARGET_W" -lt 16 ] || [ "$TARGET_H" -lt 16 ]; then
        echo "错误: 目标尺寸必须是大于等于16的整数"
        exit 1
    fi
fi

if [ "$SIZE_MODE" = "fit_align_down" ] || [ "$SIZE_MODE" = "fit" ]; then
    if ! [[ "$MAX_SIZE" =~ ^[0-9]+$ ]] || [ "$MAX_SIZE" -lt 1 ]; then
        echo "错误: 最大边尺寸必须是大于等于1的整数"
        exit 1
    fi
fi

if [ "$SIZE_MODE" != "none" ]; then
    if command -v magick >/dev/null 2>&1; then
        IM_CMD="magick"
    elif command -v convert >/dev/null 2>&1; then
        IM_CMD="convert"
    else
        echo "错误: 缩放需要 ImageMagick，请先安装 magick 或 convert"
        exit 1
    fi
fi

# 固定参数
COMPRESS="1"
QUALITY="99"
YUV_SIZE_THRESHOLD=336

# 确定输出目录
if [ -z "$CUSTOM_OUTPUT_DIR" ]; then
    OUTPUT_DIR="out_$(basename "$INPUT_DIRECTORY")"
else
    OUTPUT_DIR="$CUSTOM_OUTPUT_DIR"
fi

if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
fi

echo ""
echo "================================================"
echo "当前配置："
echo "  Alpha模式: $ALPHA"
echo "  压缩模式: RLE (1)"
echo "  JPEG质量: $QUALITY"
if [ "$DYNAMIC_YUV" -eq 1 ]; then
    echo "  YUV采样: 动态 (尺寸≤${YUV_SIZE_THRESHOLD} → 444, >${YUV_SIZE_THRESHOLD} → 422)"
else
    echo "  YUV采样: $SAMPLES"
fi
if [ "$SIZE_MODE" = "none" ]; then
    echo "  图片缩放: 不缩放（保持原始尺寸）"
elif [ "$SIZE_MODE" = "fixed" ]; then
    echo "  图片缩放: 固定尺寸 ${TARGET_W}x${TARGET_H}（强制拉伸）"
elif [ "$SIZE_MODE" = "fit_align_down" ]; then
    echo "  图片缩放: 等比缩放，最大边 ${MAX_SIZE}，向下16对齐"
else
    echo "  图片缩放: 等比缩放，最大边 ${MAX_SIZE}，无需16对齐"
fi
echo "================================================"
echo "输入目录: $INPUT_DIRECTORY"
echo "输出目录: $OUTPUT_DIR"
echo "================================================"
echo ""

align_down_16() {
    local value=$1
    local aligned=$((value / 16 * 16))
    if [ "$aligned" -lt 16 ]; then
        aligned=16
    fi
    echo "$aligned"
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

calc_fit_size_no_align() {
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

    if [ "$fit_w" -lt 1 ]; then fit_w=1; fi
    if [ "$fit_h" -lt 1 ]; then fit_h=1; fi
    echo "${fit_w}x${fit_h}"
}

apply_resize() {
    local file=$1
    local width=$2
    local height=$3
    local target_w target_h target_size

    if [ "$SIZE_MODE" = "none" ]; then
        echo "$width $height"
        return 0
    fi

    if [ "$SIZE_MODE" = "fixed" ]; then
        target_w=$TARGET_W
        target_h=$TARGET_H
    elif [ "$SIZE_MODE" = "fit_align_down" ]; then
        target_size=$(calc_fit_size "$width" "$height" "$MAX_SIZE")
        target_w=${target_size%x*}
        target_h=${target_size#*x}
    else
        target_size=$(calc_fit_size_no_align "$width" "$height" "$MAX_SIZE")
        target_w=${target_size%x*}
        target_h=${target_size#*x}
    fi

    if [ "$width" -eq "$target_w" ] && [ "$height" -eq "$target_h" ]; then
        echo "  尺寸已是 ${target_w}x${target_h}，无需缩放" >&2
        echo "$target_w $target_h"
        return 0
    fi

    echo "  缩放: ${width}x${height} → ${target_w}x${target_h}" >&2
    "$IM_CMD" "$file" -resize "${target_w}x${target_h}!" "$file"

    echo "$target_w $target_h"
}

# 根据尺寸决定 YUV 格式（仅动态模式使用）
# 图片尺寸指宽高中的较大值；≤336 用 444，>336 用 422
get_samples_for_size() {
    local w=$1
    local h=$2
    local max_size
    if [ "$w" -gt "$h" ]; then
        max_size=$w
    else
        max_size=$h
    fi
    if [ "$max_size" -le "$YUV_SIZE_THRESHOLD" ]; then
        echo "444"
    else
        echo "422"
    fi
}

# 遍历目录中的文件
traverse_directory() {
    local DIR=$1
    for FILE in "$DIR"/*; do
        if [ -d "$FILE" ]; then
            traverse_directory "$FILE"
        elif [ -f "$FILE" ]; then
            if [[ "$FILE" =~ \.(png|PNG)$ ]]; then
                echo "Found PNG file: $FILE"
                local base_name=$(basename "$FILE" | sed 's/\.[^.]*$//')

                local temp_file="$OUTPUT_DIR/${base_name}_temp.png"
                cp "$FILE" "$temp_file"

                local width=$(identify -format "%w" "$temp_file" 2>/dev/null)
                local height=$(identify -format "%h" "$temp_file" 2>/dev/null)

                if [ -z "$width" ] || [ -z "$height" ]; then
                    echo "警告: 无法使用identify获取图片尺寸，尝试magick命令"
                    width=$(magick identify -format "%w" "$temp_file" 2>/dev/null)
                    height=$(magick identify -format "%h" "$temp_file" 2>/dev/null)
                fi

                if [ -z "$width" ] || [ -z "$height" ]; then
                    echo "警告: 无法获取图片尺寸，将使用原始文件名"
                    width="unknown"
                    height="unknown"
                fi

                if [ "$width" != "unknown" ] && [ "$height" != "unknown" ]; then
                    local resized_dims
                    resized_dims=$(apply_resize "$temp_file" "$width" "$height")
                    width=${resized_dims%% *}
                    height=${resized_dims#* }
                fi

                # 动态YUV：根据尺寸选择 422 或 444
                local samples_use="$SAMPLES"
                if [ "$DYNAMIC_YUV" -eq 1 ]; then
                    if [ "$width" != "unknown" ] && [ "$height" != "unknown" ]; then
                        samples_use=$(get_samples_for_size "$width" "$height")
                    else
                        samples_use="422"
                    fi
                fi

                if [ ! -z "$width" ] && [ "$width" != "unknown" ] && [ ! -z "$height" ] && [ "$height" != "unknown" ]; then
                    echo "图片尺寸: ${width}x${height} (YUV: $samples_use)"
                fi

                local output_file="$OUTPUT_DIR/${base_name}_${width}x${height}_${ALPHA}_q${QUALITY}_${samples_use}.baj"

                "$SCRIPT_DIR/png2baj" $ALPHA $COMPRESS $QUALITY $samples_use "$temp_file" "$output_file"

                if [ $? -eq 0 ]; then
                    echo "✓ 转换成功: $output_file"
                else
                    echo "✗ 转换失败: $FILE"
                fi
                echo ""

                rm -f "$temp_file"
            fi
        fi
    done
}

traverse_directory "$INPUT_DIRECTORY"

echo "================================================"
echo "转换完成！"
echo "输出目录: $OUTPUT_DIR"
echo "================================================"
