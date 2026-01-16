#!/bin/bash
# 检查参数个数是否为5
if [ $# -ne 5 ]; then
    echo "此脚本需要恰好五个参数。"
    exit 1
else
    echo "第一个参数为：$1"
    echo "第二个参数为：$2"
    echo "第三个参数为：$3"
    echo "第四个参数为：$4"
    echo "第五个参数为：$5"
fi

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 获取目录参数
ALPHA=$1
COMPRESS=$2
QUALITY=$3
SAMPLES=$4
DIRECTORY=$5

# 检查目录是否存在
if [ ! -d "$DIRECTORY" ]; then
    echo "Error: Directory $DIRECTORY does not exist."
    exit 1
fi

# 遍历目录中的文件
traverse_directory() {
    local DIR=$1
    local out_dir="out_$(basename "$DIR")"

    if [ ! -d "$out_dir" ]; then
        mkdir "$out_dir"
    fi
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
                # 生成输出文件名，使用 .baj 后缀
                local output_file="$out_dir/${base_name}_${ALPHA}_${SAMPLES}.baj"
                # 执行转换命令
                "$SCRIPT_DIR/png2baj" $ALPHA $COMPRESS $QUALITY $SAMPLES "$FILE" "$output_file"
            fi
        fi
    done
}

# 开始遍历
traverse_directory "$DIRECTORY"