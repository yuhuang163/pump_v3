#!/bin/bash

# 检查是否提供了目录参数
if [ -z "$1" ]; then
    echo "Usage: $0 <directory>"
    exit 1
fi

#!/bin/bash
# 检查参数个数是否为2
if [ $# -ne 2 ]; then
    echo "此脚本需要恰好两个参数。"
    exit 1
else
    echo "第一个参数为：$1"
    echo "第二个参数为：$2"
fi

# 获取目录参数
DIRECTORY=$1
YUV_FORMAT=$2

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
            if [[ "$FILE" =~ \.(jpg|jpeg)$  ]] ; then
                echo "Found text file: $FILE"
                # 在这里添加对文件的操作，例如：
                ./jpgconv "$YUV_FORMAT" $FILE "$out_dir/$(basename "$FILE")" 
                
            fi
        fi
    done
}

# 开始遍历
traverse_directory "$DIRECTORY"