#!/bin/bash
###
### 资源打包脚本
### 使用方法: ./pack.sh [V1|V2] [config.json] [output.bin]
###

PACK_SH_SOURCED=0
if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    PACK_SH_SOURCED=1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN_DIR="$PARENT_DIR/bin"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"

show_usage() {
    echo "使用方法:"
    echo "  ./pack.sh [-m] [-d resource_dir] [V1|V2|V3PRO_V2] [config.json] [output.bin]"
    echo ""
    echo "选项:"
    echo "  -m, --menu               通过数字菜单选择资源目录"
    echo "  -d, --dir DIR            显式指定资源目录，可使用目录名、相对路径或绝对路径"
    echo "  -h, --help               显示帮助信息"
    echo ""
    echo "示例:"
    echo "  ./pack.sh                                       # 无参数时通过编号交互选择资源目录"
    echo "  ./pack.sh V1                                    # 打包 V1 版本（使用默认配置）"
    echo "  ./pack.sh V2                                    # 打包 V2 版本（V3/V3Pro 项目自动选择资源目录）"
    echo "  ./pack.sh V3PRO_V2                             # 显式打包 V3PRO_V2 目录"
    echo "  ./pack.sh --menu                               # 通过编号交互选择资源目录"
    echo "  ./pack.sh -d V3PRO_V2                          # 通过目录选项切换资源目录"
    echo "  ./pack.sh --dir /abs/path/to/V2                # 使用绝对路径指定资源目录"
    echo "  ./pack.sh V1 ../V1/resource_config.json        # 指定配置文件"
    echo "  ./pack.sh ../V1/resource_config.json           # 兼容旧方式（自动检测版本）"
}

list_packable_resource_dirs() {
    local dir
    local count=0
    PACKABLE_RESOURCE_DIRS=()
    for dir in "$PARENT_DIR"/*; do
        if [ -d "$dir" ] && [ -f "$dir/resource_config.json" ]; then
            PACKABLE_RESOURCE_DIRS+=("$dir")
            count=$((count + 1))
        fi
    done
    return $count
}

select_resource_dir_from_menu() {
    local choice=""
    local idx=0
    list_packable_resource_dirs
    if [ ${#PACKABLE_RESOURCE_DIRS[@]} -eq 0 ]; then
        echo "❌ 错误: 未找到可打包的资源目录" >&2
        return 1
    fi

    echo "================ 资源目录选择 ================" >&2
    for idx in "${!PACKABLE_RESOURCE_DIRS[@]}"; do
        printf "%d) %s\n" "$((idx + 1))" "$(basename "${PACKABLE_RESOURCE_DIRS[$idx]}")" >&2
    done
    echo "0) 取消" >&2
    echo "---------------------------------------------" >&2

    while true; do
        read -rp "请选择资源目录编号 [0-${#PACKABLE_RESOURCE_DIRS[@]}]: " choice >&2
        if [ "$choice" = "0" ]; then
            return 1
        fi
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le "${#PACKABLE_RESOURCE_DIRS[@]}" ]; then
            printf "%s" "${PACKABLE_RESOURCE_DIRS[$((choice - 1))]}"
            return 0
        fi
        echo "无效选择，请重新输入。" >&2
    done
}

resolve_resource_dir_arg() {
    local dir_arg="$1"
    if [ -d "$dir_arg" ]; then
        (cd "$dir_arg" && pwd)
    elif [ -d "$PARENT_DIR/$dir_arg" ]; then
        (cd "$PARENT_DIR/$dir_arg" && pwd)
    else
        printf "%s" ""
    fi
}

resolve_version_dir() {
    local version="$1"
    local device_name=""
    case "$version" in
        V1) printf "%s" "$PARENT_DIR/V1" ;;
        V2)
            device_name="$(resolve_root_device_name)"
            if [ "$device_name" = "V3Pro" ] && [ -d "$PARENT_DIR/V3PRO_V2" ]; then
                printf "%s" "$PARENT_DIR/V3PRO_V2"
            else
                printf "%s" "$PARENT_DIR/V2"
            fi
            ;;
        v3pro_V2|V3PRO_V2) printf "%s" "$PARENT_DIR/V3PRO_V2" ;;
        *) printf "%s" "$PARENT_DIR/$version" ;;
    esac
}

detect_version_from_path() {
    local path="$1"
    case "$path" in
        */V1/*) printf "%s" "V1" ;;
        */V2/*|*/v3pro_V2/*|*/V3PRO_V2/*) printf "%s" "V2" ;;
        *) printf "%s" "" ;;
    esac
}

resolve_root_device_name() {
    # 从 boards/product_info.h 解析 ROOT_DEVICE_NAME（优先使用预处理器展开，适配 ROOT_PRODUCT_ID 等间接宏）
    local info_file="$PROJECT_DIR/boards/product_info.h"
    if [ -f "$info_file" ]; then
        local compiler=""
        compiler="$(command -v gcc 2>/dev/null || command -v cc 2>/dev/null || true)"
        if [ -n "$compiler" ]; then
            local out=""
            out="$(
                printf '#include "%s"\nROOT_DEVICE_NAME\n' "$info_file" \
                    | "$compiler" -E -P -xc -I "$PROJECT_DIR/boards" -I "$PROJECT_DIR" - 2>/dev/null \
                    | awk 'NF{print; exit}' \
                    | tr -d '\r'
            )" || true
            out="${out#\"}"
            out="${out%\"}"
            if [ -n "$out" ] && [ "$out" != "ROOT_DEVICE_NAME" ]; then
                printf "%s" "$out"
                return 0
            fi
        fi
    fi

    # 回退：按 ROOT_PRODUCT_ID 简单映射
    local root_id_token=""
    root_id_token="$(grep -E '^#define[[:space:]]+ROOT_PRODUCT_ID[[:space:]]+' "$info_file" 2>/dev/null | awk '{print $3}' | tail -n1 || true)"
    case "$root_id_token" in
        ROOT_PRODUCT_V3PRO_ID|1) printf "%s" "V3Pro" ;;
        *) printf "%s" "V3" ;;
    esac
}

resolve_default_resource_version() {
    local info_file="$PROJECT_DIR/boards/product_info.h"
    if [ -f "$info_file" ]; then
        local compiler=""
        compiler="$(command -v gcc 2>/dev/null || command -v cc 2>/dev/null || true)"
        if [ -n "$compiler" ]; then
            local out=""
            out="$(
                printf '#include "%s"\nROOT_RES_PATH\n' "$info_file" \
                    | "$compiler" -E -P -xc -I "$PROJECT_DIR/boards" -I "$PROJECT_DIR" - 2>/dev/null \
                    | awk 'NF{print; exit}' \
                    | tr -d '\r'
            )" || true
            out="${out#\"}"
            out="${out%\"}"
            if [[ "$out" =~ ^V[12]$ ]]; then
                printf "%s" "$out"
                return 0
            fi
        fi
    fi

    printf "%s" "V1"
}

pack_main() {
set -e

### 解析参数
VERSION=""
CONFIG_FILE=""
OUTPUT_FILE=""
RESOURCE_DIR=""
RESOURCE_DIR_ARG=""
USE_MENU=0

while [ $# -gt 0 ]; do
    case "$1" in
        -m|--menu)
            USE_MENU=1
            shift
            ;;
        -d|--dir|--resource-dir)
            if [ $# -lt 2 ]; then
                echo "❌ 错误: $1 需要指定资源目录"
                echo ""
                show_usage
                return 1
            fi
            RESOURCE_DIR_ARG="$2"
            shift 2
            ;;
        -h|--help)
            show_usage
            return 0
            ;;
        *)
            break
            ;;
    esac
done

if [ "$USE_MENU" -eq 1 ]; then
    RESOURCE_DIR="$(select_resource_dir_from_menu)"
    if [ -z "$RESOURCE_DIR" ]; then
        echo "已取消资源打包。"
        return 1
    fi
fi

if [ -n "$RESOURCE_DIR_ARG" ]; then
    RESOURCE_DIR="$(resolve_resource_dir_arg "$RESOURCE_DIR_ARG")"
    if [ -z "$RESOURCE_DIR" ]; then
        echo "❌ 错误: 资源目录不存在: $RESOURCE_DIR_ARG"
        echo ""
        show_usage
        return 1
    fi
fi

# 检查第一个参数是否是版本号（V1 / V2 / V3PRO_V2）
if [ $# -gt 0 ] && [[ "$1" =~ ^(V1|V2|v3pro_V2|V3PRO_V2)$ ]]; then
    VERSION="$1"
    shift
fi

# 未指定参数时，默认通过菜单选择资源目录
if [ -z "$RESOURCE_DIR" ] && [ -z "$VERSION" ] && [ $# -eq 0 ]; then
    RESOURCE_DIR="$(select_resource_dir_from_menu)"
    if [ -z "$RESOURCE_DIR" ]; then
        echo "已取消资源打包。"
        return 0
    fi
fi

# 如果显式指定了资源目录，优先使用该目录
if [ -n "$RESOURCE_DIR" ]; then
    CONFIG_FILE="${1:-$RESOURCE_DIR/resource_config.json}"
    HEADER_FILE="$RESOURCE_DIR/resource_id_define.h"

    if [ $# -gt 1 ]; then
        OUTPUT_FILE="$2"
    else
        mkdir -p "$BIN_DIR/$(basename "$RESOURCE_DIR")"
        OUTPUT_FILE="$BIN_DIR/$(basename "$RESOURCE_DIR")/resource_pack.bin"
    fi
# 如果指定了版本，使用版本目录下的配置文件
elif [ -n "$VERSION" ]; then
    VERSION_DIR="$(resolve_version_dir "$VERSION")"
    CONFIG_FILE="${1:-$VERSION_DIR/resource_config.json}"
    HEADER_FILE="$VERSION_DIR/resource_id_define.h"
    
    # 生成默认输出文件名（基于版本）
    if [ $# -gt 1 ]; then
        OUTPUT_FILE="$2"
    else
        # 输出到 bin/<V1|V2> 目录
        mkdir -p "$BIN_DIR/$VERSION"
        OUTPUT_FILE="$BIN_DIR/$VERSION/resource_pack.bin"
    fi
else
    # 兼容旧的使用方式：直接指定配置文件
    CONFIG_FILE="${1:-$PARENT_DIR/resource_config.json}"
    HEADER_FILE="${CONFIG_FILE%/*}/resource_id_define.h"
    
    # 如果配置文件在 V1 或 V2 目录下，自动检测版本
    VERSION="$(detect_version_from_path "$CONFIG_FILE")"
    if [ -n "$VERSION" ]; then
        HEADER_FILE="${CONFIG_FILE%/*}/resource_id_define.h"
    fi
    
    if [ $# -gt 1 ]; then
        OUTPUT_FILE="$2"
    else
        # 如果检测到版本，输出到 bin/<V1|V2> 目录
        if [ -n "$VERSION" ]; then
            mkdir -p "$BIN_DIR/$VERSION"
            OUTPUT_FILE="$BIN_DIR/$VERSION/resource_pack.bin"
        else
            OUTPUT_FILE="${2:-$PARENT_DIR/resource_pack.bin}"
        fi
    fi
fi

# 确保输出目录存在
OUTPUT_DIR="$(dirname "$OUTPUT_FILE")"
if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
fi

### 显示信息
echo "=================================================="
echo "🚀 资源打包工具"
echo "=================================================="
if [ -n "$VERSION" ]; then
    echo "版本:     $VERSION"
fi
if [ -n "$RESOURCE_DIR" ]; then
    echo "资源目录: $RESOURCE_DIR"
fi
echo "配置文件: $CONFIG_FILE"
echo "输出文件: $OUTPUT_FILE"
echo "头文件:   $HEADER_FILE"
echo ""

### 检查配置文件
if [ ! -f "$CONFIG_FILE" ]; then
    echo "❌ 错误: 配置文件不存在: $CONFIG_FILE"
    echo ""
    show_usage
    return 1
fi

### 检查头文件
if [ ! -f "$HEADER_FILE" ]; then
    echo "⚠️  警告: 头文件不存在: $HEADER_FILE"
    echo "   将使用默认配置或从 resource_id_define.h 查找"
    HEADER_FILE=""
fi

### 执行打包
# 确定 base_dir：如果指定了版本或检测到版本，使用版本目录
BASE_DIR=""
if [ -n "$RESOURCE_DIR" ]; then
    BASE_DIR="$RESOURCE_DIR"
elif [ -n "$VERSION" ]; then
    BASE_DIR="$(resolve_version_dir "$VERSION")"
elif [ -n "$(detect_version_from_path "$CONFIG_FILE")" ]; then
    # 从配置文件路径提取版本目录
    BASE_DIR="$(dirname "$CONFIG_FILE")"
fi

# 调用 Python 脚本
echo "CRC策略: 打包时按单个资源计算 CRC16 并写入 obj_info"
PACK_EXIT_CODE=0
if [ -n "$BASE_DIR" ]; then
    # 传递 base_dir 参数
    set +e
    PACK_LOG="$(python3 "$SCRIPT_DIR/pack_resources.py" \
        "$CONFIG_FILE" \
        "$OUTPUT_FILE" \
        "$HEADER_FILE" \
        "$BASE_DIR" 2>&1)"
    PACK_EXIT_CODE=$?
    set -e
else
    set +e
    PACK_LOG="$(python3 "$SCRIPT_DIR/pack_resources.py" \
        "$CONFIG_FILE" \
        "$OUTPUT_FILE" \
        "$HEADER_FILE" 2>&1)"
    PACK_EXIT_CODE=$?
    set -e
fi

echo "$PACK_LOG"

# 提取最终输出文件名（如果脚本输出了）
ORIGINAL_FINAL_OUTPUT="$(echo "$PACK_LOG" | grep -E '^OUTPUT_FILE_FINAL=' | tail -n 1 | cut -d= -f2-)"
if [ -n "$ORIGINAL_FINAL_OUTPUT" ]; then
    # 如果最终输出文件不在目标 bin 目录，需要移动
    TARGET_BIN_DIR="$OUTPUT_DIR"
    if [[ "$ORIGINAL_FINAL_OUTPUT" != "$TARGET_BIN_DIR"/* ]]; then
        FINAL_FILENAME="$(basename "$ORIGINAL_FINAL_OUTPUT")"
        FINAL_OUTPUT_FILE="$TARGET_BIN_DIR/$FINAL_FILENAME"
        
        # 如果原文件存在，移动到新位置
        if [ -f "$ORIGINAL_FINAL_OUTPUT" ]; then
            mkdir -p "$TARGET_BIN_DIR"
            mv "$ORIGINAL_FINAL_OUTPUT" "$FINAL_OUTPUT_FILE" 2>/dev/null || true
            OUTPUT_FILE="$FINAL_OUTPUT_FILE"
        else
            OUTPUT_FILE="$ORIGINAL_FINAL_OUTPUT"
        fi
    else
        OUTPUT_FILE="$ORIGINAL_FINAL_OUTPUT"
    fi
fi

### 根据 ROOT_DEVICE_NAME 补齐文件名前缀（兼容旧产物命名：未带 V3/V3Pro 前缀的情况）
DEVICE_PREFIX="$(resolve_root_device_name)"
if [ -n "$DEVICE_PREFIX" ] && [ -n "${OUTPUT_FILE:-}" ] && [ -f "$OUTPUT_FILE" ]; then
    FINAL_DIR="$(dirname "$OUTPUT_FILE")"
    FINAL_BASE="$(basename "$OUTPUT_FILE")"
    case "$FINAL_BASE" in
        "${DEVICE_PREFIX}"_*) : ;;
        *)
            # 若原文件名已包含一个前缀段（形如 XXX_YYYYMMDD_...），则替换该前缀段为 ROOT_DEVICE_NAME
            # 否则退化为直接在文件名前追加 ROOT_DEVICE_NAME_
            if [[ "$FINAL_BASE" == *_* ]]; then
                FINAL_REST="${FINAL_BASE#*_}"
                RENAMED_OUT="$FINAL_DIR/${DEVICE_PREFIX}_${FINAL_REST}"
            else
                RENAMED_OUT="$FINAL_DIR/${DEVICE_PREFIX}_${FINAL_BASE}"
            fi
            # 若目标已存在，避免覆盖；保持原名
            if [ ! -f "$RENAMED_OUT" ]; then
                mv "$OUTPUT_FILE" "$RENAMED_OUT" 2>/dev/null || true
                if [ -f "$RENAMED_OUT" ]; then
                    OUTPUT_FILE="$RENAMED_OUT"
                fi
            fi
            ;;
    esac
fi

### 显示结果
if [ $PACK_EXIT_CODE -eq 0 ]; then
    echo ""
    echo "=================================================="
    echo "✅ 打包成功！"
    echo "=================================================="
    echo "资源包位置: $OUTPUT_FILE"
    if [ -f "$OUTPUT_FILE" ]; then
        SIZE=$(ls -lh "$OUTPUT_FILE" | awk '{print $5}')
        echo "文件大小: $SIZE"
    fi
    echo ""
    if [ -n "$RESOURCE_DIR" ]; then
        echo "💡 提示:"
        echo "  - 资源目录: $(basename "$RESOURCE_DIR")"
        echo "  - 查看配置: cat $CONFIG_FILE"
        echo "  - 修改资源: vim $CONFIG_FILE"
        echo "  - 输出目录: $OUTPUT_DIR"
    elif [ -n "$VERSION" ]; then
        echo "💡 提示:"
        echo "  - 版本: $VERSION"
        echo "  - 查看配置: cat $CONFIG_FILE"
        echo "  - 修改资源: vim $CONFIG_FILE"
        echo "  - 输出目录: $BIN_DIR/$VERSION"
    else
        echo "💡 提示:"
        echo "  - 查看配置: cat $CONFIG_FILE"
        echo "  - 修改资源: vim $CONFIG_FILE"
    fi
else
    echo ""
    echo "❌ 打包失败！请检查错误信息"
fi
return $PACK_EXIT_CODE
}

if [ "$PACK_SH_SOURCED" -eq 1 ]; then
    (pack_main "$@")
    return $?
else
    pack_main "$@"
    exit $?
fi

