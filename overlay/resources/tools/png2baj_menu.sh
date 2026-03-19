#!/bin/bash
# PNG转BAJ带菜单版本：可选 a4/a8、YUV422/YUV444/动态YUV
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
echo "  图片缩放: 不缩放（保持原始尺寸）"
echo "================================================"
echo "输入目录: $INPUT_DIRECTORY"
echo "输出目录: $OUTPUT_DIR"
echo "================================================"
echo ""

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
