#!/bin/bash
###
### 字库打包脚本
### 使用方法: ./pack.sh [选项] [字体目录] [output.bin]  # 详见 ./pack.sh --help
###

PACK_SH_SOURCED=0
if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    PACK_SH_SOURCED=1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FONT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN_DIR="$FONT_DIR/bin"
PACKER="$SCRIPT_DIR/pack_lvgl_fonts.py"
LFS_IMAGE_TOOL="$SCRIPT_DIR/make_font_lfs_image.py"
DEFAULT_OUTPUT_NAME="lv_font_pack.bin"
FONT_PACK_VERSION="${FONT_PACK_VERSION:-100}"
FONT_PACK_DB_ID_BASE="${FONT_PACK_DB_ID_BASE:-590000000}"
FONT_PACK_MAX_PACKAGE_SIZE="${FONT_PACK_MAX_PACKAGE_SIZE:-102400}"
FONT_LFS_BLOCK_SIZE="${FONT_LFS_BLOCK_SIZE:-4096}"
FONT_LFS_BLOCK_COUNT="${FONT_LFS_BLOCK_COUNT:-256}"

show_usage() {
    echo "使用方法:"
    echo "  ./pack.sh [-m] [-d font_dir] [-a] [--version 100] [--single] [V3|V3PRO|V3_font_v1|V3pro_font_v1] [output.bin]"
    echo ""
    echo "选项:"
    echo "  -m, --menu             通过数字菜单选择字库目录"
    echo "  -d, --dir DIR          显式指定字库目录，可使用目录名、相对路径或绝对路径"
    echo "  -a, --all              打包 overlay/font 下所有 *_font_* 字库目录"
    echo "      --version VERSION  输出文件名版本号，默认 100，表示 V01.00"
    echo "      --single           不按字号拆包，生成一个完整字库包"
    echo "  -h, --help             显示帮助信息"
    echo ""
    echo "示例:"
    echo "  ./pack.sh                              # 通过编号交互选择字库目录"
    echo "  ./pack.sh V3                           # 打包 V3_font_v1，默认按字号拆包"
    echo "  ./pack.sh V3PRO                        # 打包 V3pro_font_v1"
    echo "  ./pack.sh -d V3_font_v1                # 显式指定字库目录"
    echo "  ./pack.sh -a                           # 打包全部字库目录并按目录归类"
    echo "  ./pack.sh --single V3_font_v1 custom_font.bin   # 指定完整包输出文件名"
    echo ""
    echo "默认输出:"
    echo "  overlay/font/bin/<字体目录名>/<PREFIX>_<YYYYMMDD>_<VXX.XX>_<MD5>_FONT.bin"
    echo "  注: 该文件是 external littlefs 镜像，烧到 FDS_EXT_START_ADDR 后可直接按具体字体 key 加载。"
}

has_font_bins() {
    local dir="$1"
    compgen -G "$dir/*.bin" >/dev/null
}

list_packable_font_dirs() {
    local dir
    PACKABLE_FONT_DIRS=()
    for dir in "$FONT_DIR"/*; do
        if [ -d "$dir" ] && [ "$(basename "$dir")" != "bin" ] && [ "$(basename "$dir")" != "tools" ] && has_font_bins "$dir"; then
            PACKABLE_FONT_DIRS+=("$dir")
        fi
    done
    return 0
}

select_font_dir_from_menu() {
    local choice=""
    local idx=0

    list_packable_font_dirs
    if [ ${#PACKABLE_FONT_DIRS[@]} -eq 0 ]; then
        echo "❌ 错误: 未找到可打包的字库目录" >&2
        return 1
    fi

    echo "================ 字库目录选择 ================" >&2
    for idx in "${!PACKABLE_FONT_DIRS[@]}"; do
        printf "%d) %s\n" "$((idx + 1))" "$(basename "${PACKABLE_FONT_DIRS[$idx]}")" >&2
    done
    echo "0) 取消" >&2
    echo "---------------------------------------------" >&2

    while true; do
        read -rp "请选择字库目录编号 [0-${#PACKABLE_FONT_DIRS[@]}]: " choice >&2
        if [ "$choice" = "0" ]; then
            return 1
        fi
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le "${#PACKABLE_FONT_DIRS[@]}" ]; then
            printf "%s" "${PACKABLE_FONT_DIRS[$((choice - 1))]}"
            return 0
        fi
        echo "无效选择，请重新输入。" >&2
    done
}

resolve_font_dir_arg() {
    local dir_arg="$1"
    local candidate=""

    case "$dir_arg" in
        V3|v3) candidate="$FONT_DIR/V3_font_v1" ;;
        V3PRO|V3Pro|v3pro) candidate="$FONT_DIR/V3pro_font_v1" ;;
        *) candidate="$dir_arg" ;;
    esac

    if [ -d "$candidate" ]; then
        (cd "$candidate" && pwd)
    elif [ -d "$FONT_DIR/$candidate" ]; then
        (cd "$FONT_DIR/$candidate" && pwd)
    else
        printf "%s" ""
    fi
}

resolve_output_file() {
    local font_root="$1"
    local output_arg="$2"
    local font_name

    font_name="$(basename "$font_root")"
    if [ -n "$output_arg" ]; then
        case "$output_arg" in
            /*) printf "%s" "$output_arg" ;;
            */*) printf "%s" "$output_arg" ;;
            *) printf "%s/%s/%s" "$BIN_DIR" "$font_name" "$output_arg" ;;
        esac
    else
        printf "%s/%s/%s" "$BIN_DIR" "$font_name" "$DEFAULT_OUTPUT_NAME"
    fi
}

resolve_font_name_prefix() {
    local font_root="$1"
    local font_name

    font_name="$(basename "$font_root")"
    case "$font_name" in
        V3pro_*|v3pro_*|V3PRO_*) printf "%s" "V3PRO" ;;
        V3_*|v3_*) printf "%s" "V3" ;;
        *)
            printf "%s" "$font_name" | tr '[:lower:]' '[:upper:]' | sed 's/[^0-9A-Z]/_/g'
            ;;
    esac
}

format_font_pack_version() {
    local version="$1"
    local major
    local minor

    major=$((version / 100))
    minor=$((version % 100))
    printf "V%02d.%02d" "$major" "$minor"
}

make_versioned_output_path() {
    local output_file="$1"
    local font_root="$2"
    local md5_short="$3"
    local font_size="$4"
    local output_dir
    local prefix
    local date_str
    local version_str
    local suffix

    output_dir="$(dirname "$output_file")"
    prefix="$(resolve_font_name_prefix "$font_root")"
    date_str="$(date +%Y%m%d)"
    version_str="$(format_font_pack_version "$FONT_PACK_VERSION")"
    if [ -n "$font_size" ]; then
        suffix="FONT_${font_size}"
    else
        suffix="FONT"
    fi
    printf "%s/%s_%s_%s_%s_%s.bin" "$output_dir" "$prefix" "$date_str" "$version_str" "$md5_short" "$suffix"
}

font_pack_db_id_for_name() {
    local font_name="$1"

    python3 - "$FONT_PACK_DB_ID_BASE" "$font_name" <<'PY'
import binascii
import sys

base = int(sys.argv[1], 0)
name = sys.argv[2].encode("utf-8")
db_id = base + (binascii.crc32(name) % 100000000)
print(f"0x{db_id:08X}")
PY
}

extract_font_size_from_name() {
    local file_name="$1"

    if [[ "$file_name" =~ _([0-9]+)(_|\\.) ]]; then
        printf "%s" "${BASH_REMATCH[1]}"
    else
        printf "%s" ""
    fi
}

list_font_files() {
    local font_root="$1"
    local file_path

    FONT_PACK_FILES=()
    for file_path in "$font_root"/*.bin; do
        [ -f "$file_path" ] || continue
        FONT_PACK_FILES+=("$file_path")
    done
    IFS=$'\n' FONT_PACK_FILES=($(sort <<<"${FONT_PACK_FILES[*]}"))
    unset IFS
}

pack_font_pattern() {
    local font_root="$1"
    local output_file="$2"
    local input_pattern="$3"
    local font_size="$4"
    local db_id="$5"
    local manifest_file
    local pack_log
    local pack_exit_code=0
    local md5_short
    local final_output_file

    if [ -n "$font_size" ]; then
        manifest_file="$(dirname "$output_file")/lv_font_pack_${font_size}_manifest.h"
    else
        manifest_file="$(dirname "$output_file")/lv_font_pack_manifest.h"
    fi
    mkdir -p "$(dirname "$output_file")"

    echo "--------------------------------------------------"
    echo "📦 开始打包字库: $(basename "$font_root")"
    if [ -n "$font_size" ]; then
        echo "字号:     $font_size"
    fi
    echo "源目录:   $font_root"
    echo "匹配规则: $input_pattern"
    echo "输出文件: $output_file"
    echo "配置头:   $manifest_file"
    echo "DB Key:   $db_id"
    echo "文件版本号: $FONT_PACK_VERSION"
    echo "单字体包上限: $FONT_PACK_MAX_PACKAGE_SIZE"

    set +e
    pack_log="$(python3 "$PACKER" \
        --input-dir "$font_root" \
        --output "$output_file" \
        --manifest "$manifest_file" \
        --pattern "$input_pattern" \
        --db-id "$db_id" \
        --max-size "$FONT_PACK_MAX_PACKAGE_SIZE" 2>&1)"
    pack_exit_code=$?
    set -e

    echo "$pack_log"

    if [ $pack_exit_code -ne 0 ]; then
        return $pack_exit_code
    fi

    md5_short="$(md5sum "$output_file" | awk '{print substr($1, 1, 8)}')"
    final_output_file="$(make_versioned_output_path "$output_file" "$font_root" "$md5_short" "$font_size")"
    if [ "$final_output_file" != "$output_file" ]; then
        mv -f "$output_file" "$final_output_file"
    fi

    echo "OUTPUT_FILE_FINAL=$final_output_file"
    PACK_RESULT_OUTPUT="$final_output_file"
}

create_font_lfs_image() {
    local output_file="$1"
    shift
    local args=()
    local font_size
    local db_id
    local package_path

    while [ $# -gt 0 ]; do
        font_size="$1"
        db_id="$2"
        package_path="$3"
        args+=("--entry" "${font_size}:${db_id}:${package_path}")
        shift 3
    done

    python3 "$LFS_IMAGE_TOOL" \
        --output "$output_file" \
        --block-size "$FONT_LFS_BLOCK_SIZE" \
        --block-count "$FONT_LFS_BLOCK_COUNT" \
        "${args[@]}"
}

generate_font_manifest() {
    local output_file="$1"
    shift

    python3 - "$output_file" "$FONT_PACK_MAX_PACKAGE_SIZE" "$@" <<'PY'
import re
import sys
from pathlib import Path

output = Path(sys.argv[1])
max_package_size = int(sys.argv[2], 0)
items = []
args = sys.argv[3:]
if len(args) % 2 != 0:
    raise SystemExit("internal error: expected pairs of font_name/db_id")

def c_identifier(value: str) -> str:
    text = re.sub(r"[^0-9A-Za-z_]", "_", value)
    text = re.sub(r"^lv_font_", "", text, flags=re.IGNORECASE)
    if not text or text[0].isdigit():
        text = "_" + text
    return text.upper()

for i in range(0, len(args), 2):
    items.append((args[i], int(args[i + 1], 0)))

lines = [
    "#ifndef __LV_FONT_PACK_MANIFEST_H__",
    "#define __LV_FONT_PACK_MANIFEST_H__",
    "",
    "#ifdef __cplusplus",
    'extern "C" {',
    "#endif",
    "",
    f"#define LV_FONT_PACK_MAX_PACKAGE_SIZE   ({max_package_size}u)",
    "#define LV_FONT_PACK_MAGIC              (0x5046564Cu)",
    "",
]
for name, db_id in items:
    ident = c_identifier(name)
    lines.append(f"#define FONT_ID_{ident} (0x{db_id:08X}u)")
    lines.append(f'#define FONT_NAME_{ident} "{name}"')
lines.extend([
    "",
    "#ifdef __cplusplus",
    "}",
    "#endif",
    "",
    "#endif",
    "",
])
output.write_text("\n".join(lines), encoding="utf-8")
print(f"FONT_MANIFEST={output}")
PY
}

pack_one_font_dir() {
    local font_root="$1"
    local output_file="$2"

    pack_font_pattern "$font_root" "$output_file" "*.bin" "" "$LV_FONT_PACK_DB_ID"
}

pack_one_font_dir_by_font() {
    local font_root="$1"
    local output_dir="$2"
    local file_path
    local font_name
    local font_size
    local output_file
    local db_id
    local temp_dir
    local combine_args=()
    local manifest_args=()
    local md5_short
    local final_output_file

    list_font_files "$font_root"
    if [ ${#FONT_PACK_FILES[@]} -eq 0 ]; then
        echo "❌ 错误: 未找到可打包字体: $font_root"
        return 1
    fi

    mkdir -p "$output_dir"
    rm -f "$output_dir"/*_FONT*.bin "$output_dir"/lv_font_pack*.bin "$output_dir"/lv_font_pack*_manifest.h
    temp_dir="$(mktemp -d)"
    for file_path in "${FONT_PACK_FILES[@]}"; do
        font_name="$(basename "$file_path" .bin)"
        font_size="$(extract_font_size_from_name "$(basename "$file_path")")"
        output_file="$temp_dir/${font_name}.pack.bin"
        db_id="$(font_pack_db_id_for_name "$font_name")"
        pack_font_pattern "$font_root" "$output_file" "$(basename "$file_path")" "$font_size" "$db_id"
        combine_args+=("${font_size:-0}" "$db_id" "$PACK_RESULT_OUTPUT")
        manifest_args+=("$font_name" "$db_id")
    done

    generate_font_manifest "$output_dir/lv_font_pack_manifest.h" "${manifest_args[@]}"
    generate_font_manifest "$SCRIPT_DIR/lv_font_pack_manifest.h" "${manifest_args[@]}"

    output_file="$output_dir/lv_font_pack.bin"
    create_font_lfs_image "$output_file" "${combine_args[@]}"

    md5_short="$(md5sum "$output_file" | awk '{print substr($1, 1, 8)}')"
    final_output_file="$(make_versioned_output_path "$output_file" "$font_root" "$md5_short" "")"
    if [ "$final_output_file" != "$output_file" ]; then
        mv -f "$output_file" "$final_output_file"
    fi
    rm -rf "$temp_dir"

    echo "OUTPUT_FILE_FINAL=$final_output_file"
    PACK_RESULT_OUTPUT="$final_output_file"
}

pack_main() {
set -e

FONT_DIR_ARG=""
FONT_ROOT=""
OUTPUT_ARG=""
USE_MENU=0
PACK_ALL=0
SPLIT_BY_SIZE=1
GENERATED_OUTPUTS=()
PACK_RESULT_OUTPUT=""
LV_FONT_PACK_DB_ID="0xAFFA5892"

while [ $# -gt 0 ]; do
    case "$1" in
        -m|--menu)
            USE_MENU=1
            shift
            ;;
        -d|--dir|--font-dir)
            if [ $# -lt 2 ]; then
                echo "❌ 错误: $1 需要指定字库目录"
                echo ""
                show_usage
                return 1
            fi
            FONT_DIR_ARG="$2"
            shift 2
            ;;
        -a|--all)
            PACK_ALL=1
            shift
            ;;
        --version|--package-version)
            if [ $# -lt 2 ]; then
                echo "❌ 错误: $1 需要指定版本号"
                echo ""
                show_usage
                return 1
            fi
            FONT_PACK_VERSION="$2"
            shift 2
            ;;
        --single)
            SPLIT_BY_SIZE=0
            shift
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

if [ $PACK_ALL -eq 0 ] && [ -z "$FONT_DIR_ARG" ] && [ $# -gt 0 ]; then
    FONT_DIR_ARG="$1"
    shift
fi

if [ $# -gt 0 ]; then
    OUTPUT_ARG="$1"
    shift
fi

if [ $# -gt 0 ]; then
    echo "❌ 错误: 参数过多"
    echo ""
    show_usage
    return 1
fi

if [ ! -x "$PACKER" ] && [ ! -f "$PACKER" ]; then
    echo "❌ 错误: 找不到打包器: $PACKER"
    return 1
fi

echo "=================================================="
echo "🚀 字库打包工具"
echo "=================================================="

if [ $PACK_ALL -eq 1 ]; then
    list_packable_font_dirs
    if [ ${#PACKABLE_FONT_DIRS[@]} -eq 0 ]; then
        echo "❌ 错误: 未找到可打包的字库目录"
        return 1
    fi
    for FONT_ROOT in "${PACKABLE_FONT_DIRS[@]}"; do
        if [ $SPLIT_BY_SIZE -eq 1 ]; then
            pack_one_font_dir_by_font "$FONT_ROOT" "$BIN_DIR/$(basename "$FONT_ROOT")"
            GENERATED_OUTPUTS+=("$PACK_RESULT_OUTPUT")
        else
            OUTPUT_FILE="$(resolve_output_file "$FONT_ROOT" "")"
            pack_one_font_dir "$FONT_ROOT" "$OUTPUT_FILE"
            GENERATED_OUTPUTS+=("$PACK_RESULT_OUTPUT")
        fi
    done
elif [ $USE_MENU -eq 1 ] || { [ -z "$FONT_DIR_ARG" ] && [ $# -eq 0 ]; }; then
    FONT_ROOT="$(select_font_dir_from_menu)"
    if [ -z "$FONT_ROOT" ]; then
        echo "已取消字库打包。"
        return 0
    fi
    if [ $SPLIT_BY_SIZE -eq 1 ]; then
        pack_one_font_dir_by_font "$FONT_ROOT" "$BIN_DIR/$(basename "$FONT_ROOT")"
        GENERATED_OUTPUTS+=("$PACK_RESULT_OUTPUT")
    else
        OUTPUT_FILE="$(resolve_output_file "$FONT_ROOT" "$OUTPUT_ARG")"
        pack_one_font_dir "$FONT_ROOT" "$OUTPUT_FILE"
        GENERATED_OUTPUTS+=("$PACK_RESULT_OUTPUT")
    fi
else
    FONT_ROOT="$(resolve_font_dir_arg "$FONT_DIR_ARG")"
    if [ -z "$FONT_ROOT" ]; then
        echo "❌ 错误: 字库目录不存在: $FONT_DIR_ARG"
        echo ""
        show_usage
        return 1
    fi
    if ! has_font_bins "$FONT_ROOT"; then
        echo "❌ 错误: 字库目录中没有 .bin 文件: $FONT_ROOT"
        return 1
    fi
    if [ $SPLIT_BY_SIZE -eq 1 ]; then
        pack_one_font_dir_by_font "$FONT_ROOT" "$BIN_DIR/$(basename "$FONT_ROOT")"
        GENERATED_OUTPUTS+=("$PACK_RESULT_OUTPUT")
    else
        OUTPUT_FILE="$(resolve_output_file "$FONT_ROOT" "$OUTPUT_ARG")"
        pack_one_font_dir "$FONT_ROOT" "$OUTPUT_FILE"
        GENERATED_OUTPUTS+=("$PACK_RESULT_OUTPUT")
    fi
fi

echo ""
echo "=================================================="
echo "✅ 字库打包成功！"
echo "=================================================="
echo "生成文件:"
for output_item in "${GENERATED_OUTPUTS[@]}"; do
    if [ -f "$output_item" ]; then
        size="$(ls -lh "$output_item" | awk '{print $5}')"
        echo "  - $output_item ($size)"
    else
        echo "  - $output_item"
    fi
done
echo ""
echo "💡 输出为单个 external littlefs 镜像，内部按具体字体 key 存储。"
}

if [ "$PACK_SH_SOURCED" -eq 1 ]; then
    (pack_main "$@")
    return $?
else
    pack_main "$@"
    exit $?
fi
