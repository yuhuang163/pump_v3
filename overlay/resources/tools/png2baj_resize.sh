#!/bin/bash
# 检查参数个数是否为5-7
if [ $# -lt 5 ] || [ $# -gt 7 ]; then
    echo "此脚本需要5到7个参数。"
    echo "用法: $0 <alpha> <compress> <quality> <samples> <directory> [max_size] [output_dir]"
    echo "  alpha: a4或a8"
    echo "  compress: 1表示使用rle"
    echo "  quality: 0-99，99表示损失最小"
    echo "  samples: 420/422/444"
    echo "  directory: 要转换的文件夹"
    echo "  max_size: (可选)最大尺寸，默认352，必须是16的倍数"
    echo "  output_dir: (可选)输出目录，默认为 out_<输入目录名>"
    exit 1
else
    echo "第一个参数为：$1"
    echo "第二个参数为：$2"
    echo "第三个参数为：$3"
    echo "第四个参数为：$4"
    echo "第五个参数为：$5"
    if [ $# -ge 6 ]; then
        echo "第六个参数（最大尺寸）为：$6"
    fi
    if [ $# -eq 7 ]; then
        echo "第七个参数（输出目录）为：$7"
    fi
fi

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 获取目录参数
ALPHA=$1
COMPRESS=$2
QUALITY=$3
SAMPLES=$4
DIRECTORY=$5
MAX_SIZE=${6:-352}  # 默认352，可以通过第6个参数配置
OUTPUT_DIR=$7  # 可选的输出目录

# 检查MAX_SIZE是否是16的倍数
if [ $((MAX_SIZE % 16)) -ne 0 ]; then
    echo "警告: MAX_SIZE ($MAX_SIZE) 不是16的倍数，将向下对齐到 $((MAX_SIZE / 16 * 16))"
    MAX_SIZE=$((MAX_SIZE / 16 * 16))
fi

echo "使用最大尺寸: ${MAX_SIZE}x${MAX_SIZE}"

# 检查目录是否存在
if [ ! -d "$DIRECTORY" ]; then
    echo "Error: Directory $DIRECTORY does not exist."
    exit 1
fi

# 遍历目录中的文件
traverse_directory() {
    local DIR=$1
    # 如果指定了OUTPUT_DIR则使用，否则使用默认的out_目录名
    local out_dir
    if [ ! -z "$OUTPUT_DIR" ]; then
        out_dir="$OUTPUT_DIR"
    else
        out_dir="out_$(basename "$DIR")"
    fi

    if [ ! -d "$out_dir" ]; then
        mkdir -p "$out_dir"
    fi
    
    echo "输出目录: $out_dir"
    for FILE in "$DIR"/*; do
        if [ -d "$FILE" ]; then
            # 如果是目录，递归遍历
            traverse_directory "$FILE"
        elif [ -f "$FILE" ]; then
            # 如果是文件，检查文件类型
            if [[ "$FILE" =~ \.(png|PNG)$  ]] ; then
                echo "Found PNG file: $FILE"
                # 获取文件名（不含路径和扩展名）
                local base_name=$(basename "$FILE" | sed 's/\.[^.]*$//')
                
                # 先拷贝原始文件到输出目录
                local temp_file="$out_dir/${base_name}_temp.png"
                cp "$FILE" "$temp_file"
                
                # 检查图片尺寸，如果超过MAX_SIZE则缩放
                local width=$(identify -format "%w" "$temp_file" 2>/dev/null)
                local height=$(identify -format "%h" "$temp_file" 2>/dev/null)
                
                if [ ! -z "$width" ] && [ ! -z "$height" ]; then
                    if [ $width -gt $MAX_SIZE ] || [ $height -gt $MAX_SIZE ]; then
                        echo "Resizing from ${width}x${height} to max ${MAX_SIZE}x${MAX_SIZE} (16-pixel aligned)"
                        # 优先使用magick命令（ImageMagick 7），fallback到convert（兼容旧版本）
                        if command -v magick >/dev/null 2>&1; then
                            magick "$temp_file" -resize ${MAX_SIZE}x${MAX_SIZE}\> "$temp_file"
                        else
                            convert "$temp_file" -resize ${MAX_SIZE}x${MAX_SIZE}\> "$temp_file"
                        fi
                        # 重新获取缩放后的尺寸
                        width=$(identify -format "%w" "$temp_file" 2>/dev/null)
                        height=$(identify -format "%h" "$temp_file" 2>/dev/null)
                    else
                        echo "Image size ${width}x${height} is OK, no resize needed"
                    fi
                fi
                
                # 生成输出文件名，包含尺寸信息和质量信息
                local output_file="$out_dir/${base_name}_${width}x${height}_${ALPHA}_q${QUALITY}_${SAMPLES}.baj"
                # 执行转换命令
                "$SCRIPT_DIR/png2baj" $ALPHA $COMPRESS $QUALITY $SAMPLES "$temp_file" "$output_file"
                
                # 删除临时文件
                rm -f "$temp_file"
            fi
        fi
    done
}

# 开始遍历
traverse_directory "$DIRECTORY"
