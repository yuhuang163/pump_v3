#!/bin/bash
# PNG转BAJ固定参数版本：a4 + YUV422 + 质量99 + 不缩放
# 用法: ./png2baj_fixed.sh <输入目录> [输出目录]

# 检查参数个数
if [ $# -lt 1 ] || [ $# -gt 2 ]; then
    echo "用法: $0 <INPUT_DIRECTORY> [OUTPUT_DIRECTORY]"
    echo ""
    echo "  INPUT_DIRECTORY: 要转换的文件夹（包含PNG文件）"
    echo "  OUTPUT_DIRECTORY (可选): 输出目录。默认在INPUT_DIRECTORY下创建out_<INPUT_DIRECTORY_NAME>。"
    echo ""
    echo "固定参数："
    echo "  ALPHA: a4 (4位Alpha通道)"
    echo "  COMPRESS: 1 (使用RLE压缩)"
    echo "  QUALITY: 99 (最高质量)"
    echo "  SAMPLES: 422 (YUV422采样模式)"
    echo "  缩放: 不缩放，保持原始尺寸"
    exit 1
fi

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 固定参数
ALPHA="a4"
COMPRESS="1"
QUALITY="99"
SAMPLES="422"

# 获取输入输出目录
INPUT_DIRECTORY=$1
CUSTOM_OUTPUT_DIR=${2}

# 检查输入目录是否存在
if [ ! -d "$INPUT_DIRECTORY" ]; then
    echo "Error: Input directory $INPUT_DIRECTORY does not exist."
    exit 1
fi

# 确定输出目录
if [ -z "$CUSTOM_OUTPUT_DIR" ]; then
    OUTPUT_DIR="out_$(basename "$INPUT_DIRECTORY")"
else
    OUTPUT_DIR="$CUSTOM_OUTPUT_DIR"
fi

if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
fi

echo "================================================"
echo "固定参数配置："
echo "  Alpha模式: $ALPHA"
echo "  压缩模式: RLE (1)"
echo "  JPEG质量: $QUALITY"
echo "  YUV采样: $SAMPLES"
echo "  图片缩放: 不缩放（保持原始尺寸）"
echo "================================================"
echo "输入目录: $INPUT_DIRECTORY"
echo "输出目录: $OUTPUT_DIR"
echo "================================================"
echo ""

# 遍历目录中的文件
traverse_directory() {
    local DIR=$1
    for FILE in "$DIR"/*; do
        if [ -d "$FILE" ]; then
            # 如果是目录，递归遍历
            traverse_directory "$FILE"
        elif [ -f "$FILE" ]; then
            # 如果是文件，检查文件类型
            if [[ "$FILE" =~ \.(png|PNG)$  ]] ; then
                echo "Found PNG file: $FILE"
                local base_name=$(basename "$FILE" | sed 's/\.[^.]*$//')
                
                # 先拷贝原始文件到输出目录的临时文件
                local temp_file="$OUTPUT_DIR/${base_name}_temp.png"
                cp "$FILE" "$temp_file"
                
                # 获取图片尺寸（用于文件名）
                # 优先使用 identify 命令（ImageMagick 6），fallback 到 magick identify（ImageMagick 7）
                local width=$(identify -format "%w" "$temp_file" 2>/dev/null)
                local height=$(identify -format "%h" "$temp_file" 2>/dev/null)
                
                if [ -z "$width" ] || [ -z "$height" ]; then
                    echo "警告: 无法使用identify获取图片尺寸，尝试magick命令"
                    width=$(magick identify -format "%w" "$temp_file" 2>/dev/null)
                    height=$(magick identify -format "%h" "$temp_file" 2>/dev/null)
                fi
                
                if [ ! -z "$width" ] && [ ! -z "$height" ]; then
                    echo "图片尺寸: ${width}x${height} (不缩放)"
                else
                    echo "警告: 无法获取图片尺寸，将使用原始文件名"
                    width="unknown"
                    height="unknown"
                fi
                
                # 生成输出文件名，包含尺寸、alpha、质量和采样模式
                local output_file="$OUTPUT_DIR/${base_name}_${width}x${height}_${ALPHA}_q${QUALITY}_${SAMPLES}.baj"
                
                # 执行转换命令
                "$SCRIPT_DIR/png2baj" $ALPHA $COMPRESS $QUALITY $SAMPLES "$temp_file" "$output_file"
                
                if [ $? -eq 0 ]; then
                    echo "✓ 转换成功: $output_file"
                else
                    echo "✗ 转换失败: $FILE"
                fi
                echo ""
                
                # 删除临时文件
                rm -f "$temp_file"
            fi
        fi
    done
}

# 开始遍历
traverse_directory "$INPUT_DIRECTORY"

echo "================================================"
echo "转换完成！"
echo "输出目录: $OUTPUT_DIR"
echo "================================================"

