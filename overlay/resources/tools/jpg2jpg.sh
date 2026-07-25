#!/bin/bash

# 检查参数个数是否为2
if [ $# -ne 2 ]; then
    echo "Usage: $0 <待转换目录> <YUV格式>"
    echo "  YUV格式可选: YUV444 / YUV422 / YUV420"
    exit 1
fi

echo "第一个参数为：$1"
echo "第二个参数为：$2"

# 获取目录参数
DIRECTORY=$1
YUV_FORMAT=$2

# jpgconv 与本脚本同目录（不依赖当前工作目录）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JPGCONV="$SCRIPT_DIR/jpgconv"

if [ ! -x "$JPGCONV" ]; then
    echo "Error: jpgconv not found or not executable: $JPGCONV"
    echo "Try: chmod +x \"$JPGCONV\""
    exit 1
fi

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
            if [[ "$FILE" =~ \.(jpg|jpeg)$ ]]; then
                echo "Found jpg file: $FILE"
                "$JPGCONV" "$YUV_FORMAT" "$FILE" "$out_dir/$(basename "$FILE")"
            fi
        fi
    done
}

# 开始遍历
traverse_directory "$DIRECTORY"
