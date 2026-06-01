#!/bin/bash
###
### 资源打包脚本
### 使用方法: ./pack.sh [选项] [版本或目录] [config.json] [output.bin]  # 详见 ./pack.sh --help
###

PACK_SH_SOURCED=0
if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    PACK_SH_SOURCED=1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN_DIR="$PARENT_DIR/bin"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"
CONFIG_NAME_LEGACY="resource_config.json"
CONFIG_NAME_BASE="resource_base_config.json"
CONFIG_NAME_CUSTOM="resource_custom_config.json"

show_usage() {
    echo "使用方法:"
    echo "  ./pack.sh [-m] [-d resource_dir] [-t base|custom|multi|all|auto] [V2|V3|V4|v3_res_v2|v3pro_res_v2|v3_res_v3|v3pro_res_v3|v3_res_v4|v3pro_res_v4] [config.json] [output.bin]"
    echo ""
    echo "选项:"
    echo "  -m, --menu               通过数字菜单选择资源目录"
    echo "  -d, --dir DIR            显式指定资源目录，可使用目录名、相对路径或绝对路径"
    echo "  -t, --type TYPE          打包类型: auto|base|custom|multi|all，默认 auto"
    echo "  -h, --help               显示帮助信息"
    echo ""
    echo "示例:"
    echo "  ./pack.sh                                       # 无参数时通过编号交互选择资源目录，默认 auto"
    echo "  ./pack.sh -t multi V2                           # 打包 multi 资源包"
    echo "  ./pack.sh -t base V2                            # 只打包 base 资源包"
    echo "  ./pack.sh -t custom V2                          # 只打包 custom 资源包"
    echo "  ./pack.sh -t all V2                             # 同时生成 base/custom/multi"
    echo "  ./pack.sh v3pro_res_v2                          # 显式打包 v3pro_res_v2 目录"
    echo "  ./pack.sh V3                                    # 按 GNUmakefile 产品族选择 v3_res_v3 / v3pro_res_v3"
    echo "  ./pack.sh V4                                    # 按 GNUmakefile 产品族选择 v3_res_v4 / v3pro_res_v4"
    echo "  ./pack.sh v3_res_v3                             # 显式打包 v3_res_v3 目录"
    echo "  ./pack.sh v3pro_res_v3                          # 显式打包 v3pro_res_v3 目录"
    echo "  ./pack.sh v3_res_v4                             # 显式打包 v3_res_v4 目录"
    echo "  ./pack.sh v3pro_res_v4                          # 显式打包 v3pro_res_v4 目录"
    echo "  ./pack.sh --menu                                # 通过编号交互选择资源目录"
    echo "  ./pack.sh -d v3pro_res_v2                       # 通过目录选项切换资源目录"
    echo "  ./pack.sh --dir /abs/path/to/v3_res_v2 -t multi # 使用绝对路径指定资源目录"
    echo "  ./pack.sh v3_res_v2 ../v3_res_v2/resource_config.json    # 兼容旧单配置文件方式"
    echo "  ./pack.sh ../v3_res_v2/resource_base_config.json         # 显式指定 base 配置文件"
}

has_resource_config() {
    local dir="$1"
    [ -f "$dir/$CONFIG_NAME_LEGACY" ] || [ -f "$dir/$CONFIG_NAME_BASE" ] || [ -f "$dir/$CONFIG_NAME_CUSTOM" ]
}

list_packable_resource_dirs() {
    local dir
    local count=0
    PACKABLE_RESOURCE_DIRS=()
    for dir in "$PARENT_DIR"/*; do
        if [ -d "$dir" ] && has_resource_config "$dir"; then
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

select_pack_type_from_menu() {
    local resource_root="$1"
    local choice=""
    local default_mode=""
    local available_modes=()
    local idx=0

    if [ -f "$resource_root/$CONFIG_NAME_LEGACY" ]; then
        available_modes+=("legacy")
    fi
    if [ -f "$resource_root/$CONFIG_NAME_BASE" ]; then
        available_modes+=("base")
    fi
    if [ -f "$resource_root/$CONFIG_NAME_CUSTOM" ]; then
        available_modes+=("custom")
    fi
    if [ -f "$resource_root/$CONFIG_NAME_BASE" ] && [ -f "$resource_root/$CONFIG_NAME_CUSTOM" ]; then
        available_modes+=("multi" "all")
    fi

    if [ ${#available_modes[@]} -eq 0 ]; then
        echo "❌ 错误: 当前资源目录没有可用配置文件" >&2
        return 1
    fi

    default_mode="$(resolve_default_pack_mode "$resource_root")"

    echo "================ 打包类型选择 ================" >&2
    for idx in "${!available_modes[@]}"; do
        case "${available_modes[$idx]}" in
            legacy) printf "%d) legacy  (resource_config.json)\n" "$((idx + 1))" >&2 ;;
            base) printf "%d) base    (resource_base_config.json)\n" "$((idx + 1))" >&2 ;;
            custom) printf "%d) custom  (resource_custom_config.json)\n" "$((idx + 1))" >&2 ;;
            multi) printf "%d) multi   (base + custom，按偏移拼接)\n" "$((idx + 1))" >&2 ;;
            all) printf "%d) all     (同时生成 base/custom/multi)\n" "$((idx + 1))" >&2 ;;
        esac
    done
    echo "0) 使用默认: ${default_mode:-auto}" >&2
    echo "q) 取消" >&2
    echo "---------------------------------------------" >&2

    while true; do
        read -rp "请选择打包类型编号: " choice >&2
        if [ "$choice" = "0" ]; then
            printf "%s" "$default_mode"
            return 0
        fi
        if [ "$choice" = "q" ] || [ "$choice" = "Q" ]; then
            return 1
        fi
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le "${#available_modes[@]}" ]; then
            printf "%s" "${available_modes[$((choice - 1))]}"
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
        V2)
            device_name="$(resolve_root_device_name)"
            if [ "$device_name" = "V3Pro" ] && [ -d "$PARENT_DIR/v3pro_res_v2" ]; then
                printf "%s" "$PARENT_DIR/v3pro_res_v2"
            else
                printf "%s" "$PARENT_DIR/v3_res_v2"
            fi
            ;;
        V3)
            device_name="$(resolve_root_device_name)"
            if [ "$device_name" = "V3Pro" ] && [ -d "$PARENT_DIR/v3pro_res_v3" ]; then
                printf "%s" "$PARENT_DIR/v3pro_res_v3"
            else
                printf "%s" "$PARENT_DIR/v3_res_v3"
            fi
            ;;
        V4)
            device_name="$(resolve_root_device_name)"
            if [ "$device_name" = "V3Pro" ] && [ -d "$PARENT_DIR/v3pro_res_v4" ]; then
                printf "%s" "$PARENT_DIR/v3pro_res_v4"
            else
                printf "%s" "$PARENT_DIR/v3_res_v4"
            fi
            ;;
        v3_res_v2) printf "%s" "$PARENT_DIR/v3_res_v2" ;;
        v3pro_res_v2|v3pro_V2|V3PRO_V2) printf "%s" "$PARENT_DIR/v3pro_res_v2" ;;
        v3_res_v3|V3_V3) printf "%s" "$PARENT_DIR/v3_res_v3" ;;
        v3pro_res_v3|v3pro_V3|V3PRO_V3) printf "%s" "$PARENT_DIR/v3pro_res_v3" ;;
        v3_res_v4|V3_V4) printf "%s" "$PARENT_DIR/v3_res_v4" ;;
        v3pro_res_v4|v3pro_V4|V3PRO_V4) printf "%s" "$PARENT_DIR/v3pro_res_v4" ;;
        *) printf "%s" "$PARENT_DIR/$version" ;;
    esac
}

detect_version_from_path() {
    local path="$1"
    case "$path" in
        */v3_res_v2/*|*/V2/*) printf "%s" "v3_res_v2" ;;
        */v3pro_res_v2/*|*/v3pro_V2/*|*/V3PRO_V2/*) printf "%s" "v3pro_res_v2" ;;
        */v3_res_v3/*|*/V3_V3/*) printf "%s" "v3_res_v3" ;;
        */v3pro_res_v3/*|*/v3pro_V3/*|*/V3PRO_V3/*) printf "%s" "v3pro_res_v3" ;;
        */v3_res_v4/*|*/V3_V4/*) printf "%s" "v3_res_v4" ;;
        */v3pro_res_v4/*|*/v3pro_V4/*|*/V3PRO_V4/*) printf "%s" "v3pro_res_v4" ;;
        *) printf "%s" "" ;;
    esac
}

detect_pack_mode_from_config_path() {
    local config_path="$1"
    case "$(basename "$config_path")" in
        "$CONFIG_NAME_BASE") printf "%s" "base" ;;
        "$CONFIG_NAME_CUSTOM") printf "%s" "custom" ;;
        "$CONFIG_NAME_LEGACY") printf "%s" "legacy" ;;
        *) printf "%s" "legacy" ;;
    esac
}

resolve_default_pack_mode() {
    local resource_root="$1"
    if [ -f "$resource_root/$CONFIG_NAME_BASE" ] && [ -f "$resource_root/$CONFIG_NAME_CUSTOM" ]; then
        printf "%s" "multi"
    elif [ -f "$resource_root/$CONFIG_NAME_BASE" ]; then
        printf "%s" "base"
    elif [ -f "$resource_root/$CONFIG_NAME_CUSTOM" ]; then
        printf "%s" "custom"
    elif [ -f "$resource_root/$CONFIG_NAME_LEGACY" ]; then
        printf "%s" "legacy"
    else
        printf "%s" ""
    fi
}

resolve_config_for_mode() {
    local resource_root="$1"
    local pack_mode="$2"
    case "$pack_mode" in
        legacy) printf "%s" "$resource_root/$CONFIG_NAME_LEGACY" ;;
        base) printf "%s" "$resource_root/$CONFIG_NAME_BASE" ;;
        custom) printf "%s" "$resource_root/$CONFIG_NAME_CUSTOM" ;;
        *) printf "%s" "" ;;
    esac
}

append_output_suffix() {
    local file_path="$1"
    local suffix="$2"
    local dir_name
    local base_name
    if [ -z "$suffix" ]; then
        printf "%s" "$file_path"
        return 0
    fi

    dir_name="$(dirname "$file_path")"
    base_name="$(basename "$file_path")"
    case "$base_name" in
        *"_${suffix}.bin") printf "%s" "$file_path" ;;
        *.bin) printf "%s/%s_%s.bin" "$dir_name" "${base_name%.bin}" "$suffix" ;;
        *) printf "%s/%s_%s" "$dir_name" "$base_name" "$suffix" ;;
    esac
}

normalize_output_dir_mode() {
    local pack_mode="$1"
    case "$pack_mode" in
        multi|all) printf "%s" "all" ;;
        base|custom|legacy) printf "%s" "$pack_mode" ;;
        *) printf "%s" "$pack_mode" ;;
    esac
}

append_output_subdir() {
    local file_path="$1"
    local subdir_name="$2"
    local dir_name
    local base_name

    if [ -z "$subdir_name" ]; then
        printf "%s" "$file_path"
        return 0
    fi

    dir_name="$(dirname "$file_path")"
    base_name="$(basename "$file_path")"
    printf "%s/%s/%s" "$dir_name" "$subdir_name" "$base_name"
}

normalize_output_location() {
    local pack_log="$1"
    local output_template="$2"
    local final_output="$output_template"
    local target_bin_dir
    local original_final_output

    target_bin_dir="$(dirname "$output_template")"
    original_final_output="$(echo "$pack_log" | grep -E '^OUTPUT_FILE_FINAL=' | tail -n 1 | cut -d= -f2-)"

    if [ -n "$original_final_output" ]; then
        if [[ "$original_final_output" != "$target_bin_dir"/* ]]; then
            local final_filename
            final_filename="$(basename "$original_final_output")"
            final_output="$target_bin_dir/$final_filename"
            if [ -f "$original_final_output" ]; then
                mkdir -p "$target_bin_dir"
                mv "$original_final_output" "$final_output" 2>/dev/null || true
            else
                final_output="$original_final_output"
            fi
        else
            final_output="$original_final_output"
        fi
    fi

    printf "%s" "$final_output"
}

apply_device_prefix_if_needed() {
    local file_path="$1"
    local device_prefix
    local root_name
    local fallback_prefix
    local final_dir
    local final_base
    local renamed_out
    local final_rest

    root_name="$(basename "${RESOURCE_ROOT:-}")"
    case "$root_name" in
        v3pro_res_v2|v3pro_res_v3|v3pro_res_v4|V3PRO_V2|V3PRO_V3|V3PRO_V4) device_prefix="V3PRO" ;;
        v3_res_v2|v3_res_v3|v3_res_v4|V2|V3_V3|V3_V4) device_prefix="V3" ;;
        *)
            fallback_prefix="$(resolve_root_device_name)"
            case "$fallback_prefix" in
                V3Pro|V3PRO|v3pro) device_prefix="V3PRO" ;;
                V3|v3) device_prefix="V3" ;;
                *) device_prefix="$fallback_prefix" ;;
            esac
            ;;
    esac

    if [ -z "$device_prefix" ] || [ -z "$file_path" ] || [ ! -f "$file_path" ]; then
        printf "%s" "$file_path"
        return 0
    fi

    final_dir="$(dirname "$file_path")"
    final_base="$(basename "$file_path")"
    case "$final_base" in
        "${device_prefix}"_*) printf "%s" "$file_path" ;;
        *)
            if [[ "$final_base" == *_* ]]; then
                final_rest="${final_base#*_}"
                renamed_out="$final_dir/${device_prefix}_${final_rest}"
            else
                renamed_out="$final_dir/${device_prefix}_${final_base}"
            fi
            if [ ! -f "$renamed_out" ]; then
                mv "$file_path" "$renamed_out" 2>/dev/null || true
                if [ -f "$renamed_out" ]; then
                    file_path="$renamed_out"
                fi
            fi
            printf "%s" "$file_path"
            ;;
    esac
}

apply_pack_suffix_if_needed() {
    local file_path="$1"
    local suffix="$2"
    local suffixed_path
    if [ -z "$suffix" ] || [ -z "$file_path" ] || [ ! -f "$file_path" ]; then
        printf "%s" "$file_path"
        return 0
    fi

    suffixed_path="$(append_output_suffix "$file_path" "$suffix")"
    if [ "$suffixed_path" != "$file_path" ]; then
        mv -f "$file_path" "$suffixed_path" 2>/dev/null || true
        if [ -f "$suffixed_path" ]; then
            file_path="$suffixed_path"
        fi
    fi
    printf "%s" "$file_path"
}

run_pack_resources_for_mode() {
    local pack_mode="$1"
    local config_file="$2"
    local output_template="$3"
    local header_file="$4"
    local base_dir="$5"
    local suffix=""
    local pack_log
    local pack_exit_code=0
    local final_output=""

    case "$pack_mode" in
        base|custom) suffix="$pack_mode" ;;
        *) suffix="" ;;
    esac

    mkdir -p "$(dirname "$output_template")"

    echo "--------------------------------------------------"
    echo "📦 开始打包: $pack_mode"
    echo "配置文件: $config_file"
    echo "输出目录: $(dirname "$output_template")"

    set +e
    pack_log="$(python3 "$SCRIPT_DIR/pack_resources.py" \
        "$config_file" \
        "$output_template" \
        "$header_file" \
        "$base_dir" 2>&1)"
    pack_exit_code=$?
    set -e

    echo "$pack_log"

    if [ $pack_exit_code -ne 0 ]; then
        return $pack_exit_code
    fi

    final_output="$(normalize_output_location "$pack_log" "$output_template")"
    final_output="$(apply_device_prefix_if_needed "$final_output")"
    final_output="$(apply_pack_suffix_if_needed "$final_output" "$suffix")"

    PACK_RESULT_OUTPUT="$final_output"
    return 0
}

resolve_theme_img_res_package_offset() {
    local partitions_file="$PROJECT_DIR/boards/project_partitions.h"
    local compiler=""
    local out=""

    if [ ! -f "$partitions_file" ]; then
        printf "%s" ""
        return 0
    fi

    compiler="$(command -v gcc 2>/dev/null || command -v cc 2>/dev/null || true)"
    if [ -n "$compiler" ]; then
        out="$(
            printf '#include "%s"\nTHEME_IMG_RES_PAKAGE_OFFSET\n' "$partitions_file" \
                | "$compiler" -E -P -xc -I "$PROJECT_DIR/boards" -I "$PROJECT_DIR" - 2>/dev/null \
                | awk 'NF{line=$0} END{print line}' \
                | tr -d '\r'
        )" || true
        out="$(printf "%s" "$out" | tr -d '[:space:]')"
        while [[ "$out" =~ ^\((.*)\)$ ]]; do
            out="${BASH_REMATCH[1]}"
        done
        if [ -n "$out" ] && [ "$out" != "THEME_IMG_RES_PAKAGE_OFFSET" ]; then
            printf "%s" "$out"
            return 0
        fi
    fi

    printf "%s" ""
}

build_multi_output_path() {
    local base_file="$1"
    local output_template="$2"
    local multi_path=""
    if [ -n "$output_template" ]; then
        multi_path="$(append_output_suffix "$output_template" "all")"
        printf "%s" "$multi_path"
        return 0
    fi

    if [ -n "$base_file" ]; then
        case "$base_file" in
            *_base.bin) printf "%s" "${base_file%_base.bin}_all.bin" ;;
            *.bin) printf "%s" "${base_file%.bin}_all.bin" ;;
            *) printf "%s" "${base_file}_all" ;;
        esac
        return 0
    fi

    printf "%s" ""
}

create_multi_package() {
    local base_file="$1"
    local custom_file="$2"
    local output_file="$3"
    local offset_value="$4"

    mkdir -p "$(dirname "$output_file")"

    python3 - "$base_file" "$custom_file" "$output_file" "$offset_value" <<'PY'
import hashlib
import os
import re
import sys

base_file, custom_file, output_file, offset_value = sys.argv[1:5]
offset = int(offset_value, 0)

with open(base_file, "rb") as f:
    base_data = f.read()
with open(custom_file, "rb") as f:
    custom_data = f.read()

base_size = len(base_data)
if base_size > offset:
    raise SystemExit(
        f"base 资源包大小 {base_size} bytes 超过 THEME_IMG_RES_PAKAGE_OFFSET={offset}，无法生成 multi 包"
    )

padding = b"\xFF" * (offset - base_size)
merged_data = base_data + padding + custom_data
merged_md5 = hashlib.md5(merged_data).hexdigest()

base_name = os.path.basename(base_file)
output_dir = os.path.dirname(output_file)
match = re.match(
    r"(?P<prefix>.+)_(?P<date>\d{8})_(?P<ver>V\d{2}\.\d{2})_[0-9a-fA-F]{8}_RES(?:_[A-Za-z0-9]+)?\.bin$",
    base_name,
)
if match:
    final_name = (
        f"{match.group('prefix')}_{match.group('date')}_{match.group('ver')}_"
        f"{merged_md5[:8]}_RES_all.bin"
    )
    final_output_file = os.path.join(output_dir, final_name)
else:
    final_output_file = output_file

with open(final_output_file, "wb") as f:
    f.write(base_data)
    f.write(padding)
    f.write(custom_data)

print(f"MULTI_OUTPUT_FILE={final_output_file}")
print(f"MULTI_BASE_SIZE={base_size}")
print(f"MULTI_PADDING_SIZE={len(padding)}")
print(f"MULTI_CUSTOM_SIZE={len(custom_data)}")
print(f"MULTI_MD5={merged_md5}")
print(f"MULTI_TOTAL_SIZE={os.path.getsize(final_output_file)}")
PY
}

resolve_root_product_family_from_makefile() {
    # 优先按 GNUmakefile 中 ROOT_PRODUCT_ID 的默认配置判断产品族，避免单独预处理
    # product_info.h 时未定义宏被当成 0，导致 V3/V3Pro 误判。
    local makefile="$PROJECT_DIR/GNUmakefile"
    if [ ! -f "$makefile" ]; then
        printf "%s" "V3"
        return 0
    fi

    local v3_id=""
    local v3pro_id=""
    local product_id_token=""
    v3_id="$(awk '/^ROOT_PRODUCT_V3_ID[[:space:]]*\?=/{print $3; exit}' "$makefile")"
    v3pro_id="$(awk '/^ROOT_PRODUCT_V3PRO_ID[[:space:]]*\?=/{print $3; exit}' "$makefile")"
    product_id_token="$(awk '/^ROOT_PRODUCT_ID[[:space:]]*\?=/{print $3; exit}' "$makefile")"

    case "$product_id_token" in
        '$(ROOT_PRODUCT_V3PRO_ID)') product_id_token="$v3pro_id" ;;
        '$(ROOT_PRODUCT_V3_ID)') product_id_token="$v3_id" ;;
    esac

    if [ -n "$v3pro_id" ] && [ "$product_id_token" = "$v3pro_id" ]; then
        printf "%s" "V3Pro"
    else
        printf "%s" "V3"
    fi
}

collect_product_info_cpp_flags() {
    # 从 GNUmakefile 收集与 product_info.h 条件编译一致的 -D 宏
    local makefile="${1:-$PROJECT_DIR/GNUmakefile}"
    if [ ! -f "$makefile" ]; then
        return 0
    fi

    local root_ui_token root_ui_v2 root_ui_v3 root_ui_v4 resolved_ui=""
    local v3_id v3pro_id product_id_token product_id_value=""

    root_ui_token="$(awk '/^ROOT_UI_VERSION[[:space:]]*\?=/{print $3; exit}' "$makefile")"
    root_ui_v2="$(awk '/^ROOT_UI_V2[[:space:]]*\?=/{print $3; exit}' "$makefile")"
    root_ui_v3="$(awk '/^ROOT_UI_V3[[:space:]]*\?=/{print $3; exit}' "$makefile")"
    root_ui_v4="$(awk '/^ROOT_UI_V4[[:space:]]*\?=/{print $3; exit}' "$makefile")"
    v3_id="$(awk '/^ROOT_PRODUCT_V3_ID[[:space:]]*\?=/{print $3; exit}' "$makefile")"
    v3pro_id="$(awk '/^ROOT_PRODUCT_V3PRO_ID[[:space:]]*\?=/{print $3; exit}' "$makefile")"
    product_id_token="$(awk '/^ROOT_PRODUCT_ID[[:space:]]*\?=/{print $3; exit}' "$makefile")"

    case "$product_id_token" in
        '$(ROOT_PRODUCT_V3PRO_ID)') product_id_value="$v3pro_id" ;;
        '$(ROOT_PRODUCT_V3_ID)') product_id_value="$v3_id" ;;
        *) product_id_value="$product_id_token" ;;
    esac

    case "$root_ui_token" in
        '$(ROOT_UI_V2)') resolved_ui="$root_ui_v2" ;;
        '$(ROOT_UI_V3)') resolved_ui="$root_ui_v3" ;;
        '$(ROOT_UI_V4)') resolved_ui="$root_ui_v4" ;;
        *) resolved_ui="$root_ui_token" ;;
    esac

    [ -n "${root_ui_v2:-}" ] && printf '%s\n' "-DROOT_UI_V2=${root_ui_v2}"
    [ -n "${root_ui_v3:-}" ] && printf '%s\n' "-DROOT_UI_V3=${root_ui_v3}"
    [ -n "${root_ui_v4:-}" ] && printf '%s\n' "-DROOT_UI_V4=${root_ui_v4}"
    [ -n "${resolved_ui:-}" ] && printf '%s\n' "-DROOT_UI_VERSION=${resolved_ui}"
    [ -n "${product_id_value:-}" ] && printf '%s\n' "-DROOT_PRODUCT_ID=${product_id_value}"
    [ -n "${v3_id:-}" ] && printf '%s\n' "-DROOT_PRODUCT_V3_ID=${v3_id}"
    [ -n "${v3pro_id:-}" ] && printf '%s\n' "-DROOT_PRODUCT_V3PRO_ID=${v3pro_id}"
}

product_info_preprocess_macro() {
    local macro="$1"
    local info_file="${2:-$PROJECT_DIR/boards/product_info.h}"
    local compiler out cpp_flag
    local -a cpp_flags=()

    if [ ! -f "$info_file" ]; then
        printf "%s" ""
        return 0
    fi

    compiler="$(command -v gcc 2>/dev/null || command -v cc 2>/dev/null || true)"
    if [ -z "$compiler" ]; then
        printf "%s" ""
        return 0
    fi

    while IFS= read -r cpp_flag; do
        [ -n "$cpp_flag" ] && cpp_flags+=("$cpp_flag")
    done < <(collect_product_info_cpp_flags)

    out="$(
        printf '#include "%s"\n%s\n' "$info_file" "$macro" \
            | "$compiler" -E -P -xc "${cpp_flags[@]}" -I "$PROJECT_DIR/boards" -I "$PROJECT_DIR" - 2>/dev/null \
            | awk 'NF{print; exit}' \
            | tr -d '\r'
    )" || true
    printf "%s" "$out"
}

resolve_root_res_path() {
    # 与 product_info.h / GNUmakefile 中 ROOT_UI_VERSION 分支一致
    local makefile="$PROJECT_DIR/GNUmakefile"
    if [ -f "$makefile" ]; then
        local root_ui_token root_ui_v2_value root_ui_v3_value root_ui_v4_value
        root_ui_token="$(awk '/^ROOT_UI_VERSION[[:space:]]*\?=/{print $3; exit}' "$makefile")"
        root_ui_v2_value="$(awk '/^ROOT_UI_V2[[:space:]]*\?=/{print $3; exit}' "$makefile")"
        root_ui_v3_value="$(awk '/^ROOT_UI_V3[[:space:]]*\?=/{print $3; exit}' "$makefile")"
        root_ui_v4_value="$(awk '/^ROOT_UI_V4[[:space:]]*\?=/{print $3; exit}' "$makefile")"

        case "$root_ui_token" in
            '$(ROOT_UI_V2)') printf "%s" "V2"; return 0 ;;
            '$(ROOT_UI_V3)') printf "%s" "V3"; return 0 ;;
            '$(ROOT_UI_V4)') printf "%s" "V4"; return 0 ;;
            "$root_ui_v2_value")
                if [ -n "${root_ui_token:-}" ]; then
                    printf "%s" "V2"
                    return 0
                fi
                ;;
            "$root_ui_v3_value")
                if [ -n "${root_ui_token:-}" ]; then
                    printf "%s" "V3"
                    return 0
                fi
                ;;
            "$root_ui_v4_value")
                if [ -n "${root_ui_token:-}" ]; then
                    printf "%s" "V4"
                    return 0
                fi
                ;;
        esac
    fi

    local out
    out="$(product_info_preprocess_macro "ROOT_RES_PATH")"
    out="${out#\"}"
    out="${out%\"}"
    if [[ "$out" =~ ^V[234]$ ]]; then
        printf "%s" "$out"
        return 0
    fi

    printf "%s" "V4"
}

resolve_root_res_version() {
    # ROOT_RES_VERSION = major*100 + minor，随 UI：V2->102, V3->103, V4->104
    local res_path out
    res_path="$(resolve_root_res_path)"
    case "$res_path" in
        V2) printf "%s" "102"; return 0 ;;
        V3) printf "%s" "103"; return 0 ;;
        V4) printf "%s" "104"; return 0 ;;
    esac

    out="$(product_info_preprocess_macro "ROOT_RES_VERSION")"
    out="${out#(}"
    out="${out%)}"
    if [[ "$out" =~ ^[0-9]+$ ]]; then
        printf "%s" "$out"
        return 0
    fi

    printf "%s" "104"
}

resolve_root_device_name() {
    # 优先按 GNUmakefile 中 ROOT_PRODUCT_ID 默认值判断，避免直接预处理 product_info.h
    # 时把未定义条件宏当成 0，误判为 V3Pro。
    local device_name=""
    device_name="$(resolve_root_product_family_from_makefile)"
    if [ -n "$device_name" ]; then
        printf "%s" "$device_name"
        return 0
    fi

    # 回退：从 boards/product_info.h 解析 ROOT_DEVICE_NAME（带 GNUmakefile 宏展开）
    local out
    out="$(product_info_preprocess_macro "ROOT_DEVICE_NAME")"
    out="${out#\"}"
    out="${out%\"}"
    if [ -n "$out" ] && [ "$out" != "ROOT_DEVICE_NAME" ]; then
        printf "%s" "$out"
        return 0
    fi

    # 回退：按 ROOT_PRODUCT_ID 简单映射
    local info_file="$PROJECT_DIR/boards/product_info.h"
    local root_id_token=""
    root_id_token="$(grep -E '^#define[[:space:]]+ROOT_PRODUCT_ID[[:space:]]+' "$info_file" 2>/dev/null | awk '{print $3}' | tail -n1 || true)"
    case "$root_id_token" in
        ROOT_PRODUCT_V3PRO_ID|1) printf "%s" "V3Pro" ;;
        *) printf "%s" "V3" ;;
    esac
}

resolve_default_resource_version() {
    # 默认资源路径别名（V2/V3/V4），与 GNUmakefile 中 ROOT_UI_VERSION 一致
    resolve_root_res_path
}

pack_main() {
set -e

### 解析参数
VERSION=""
OUTPUT_FILE=""
RESOURCE_DIR=""
RESOURCE_DIR_ARG=""
USE_MENU=0
PACK_TYPE="auto"
EXPLICIT_CONFIG_FILE=""
EXPLICIT_OUTPUT_FILE=""
RESOURCE_ROOT=""
HEADER_FILE=""
OUTPUT_TEMPLATE=""
PACK_RESULT_OUTPUT=""
GENERATED_OUTPUTS=()
PACK_TYPE_SELECTED_FROM_MENU=0

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
        -t|--type)
            if [ $# -lt 2 ]; then
                echo "❌ 错误: $1 需要指定打包类型"
                echo ""
                show_usage
                return 1
            fi
            PACK_TYPE="$2"
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
    if [ "$PACK_TYPE" = "auto" ]; then
        PACK_TYPE="$(select_pack_type_from_menu "$RESOURCE_DIR")"
        if [ -z "$PACK_TYPE" ]; then
            echo "已取消资源打包。"
            return 1
        fi
        PACK_TYPE_SELECTED_FROM_MENU=1
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

case "$PACK_TYPE" in
    auto|base|custom|multi|all) ;;
    *)
        echo "❌ 错误: 不支持的打包类型: $PACK_TYPE"
        echo ""
        show_usage
        return 1
        ;;
esac

# 检查第一个参数是否是版本号或资源目录别名（V2 / V3 / V4 / v3_res_v2 / v3pro_res_v2 / v3_res_v3 / v3pro_res_v3 / v3_res_v4 / v3pro_res_v4）
if [ $# -gt 0 ] && [[ "$1" =~ ^(V2|V3|V4|v3_res_v2|v3pro_res_v2|v3_res_v3|v3pro_res_v3|v3_res_v4|v3pro_res_v4|v3pro_V2|V3PRO_V2|v3pro_V3|V3PRO_V3|V3_V3|v3pro_V4|V3PRO_V4|V3_V4)$ ]]; then
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
    if [ "$PACK_TYPE" = "auto" ]; then
        PACK_TYPE="$(select_pack_type_from_menu "$RESOURCE_DIR")"
        if [ -z "$PACK_TYPE" ]; then
            echo "已取消资源打包。"
            return 0
        fi
        PACK_TYPE_SELECTED_FROM_MENU=1
    fi
fi

if [ $# -gt 0 ]; then
    case "$1" in
        *.json)
            EXPLICIT_CONFIG_FILE="$1"
            shift
            ;;
        *.bin)
            EXPLICIT_OUTPUT_FILE="$1"
            shift
            ;;
        *)
            EXPLICIT_CONFIG_FILE="$1"
            shift
            ;;
    esac
fi

if [ $# -gt 0 ]; then
    EXPLICIT_OUTPUT_FILE="$1"
    shift
fi

if [ $# -gt 0 ]; then
    echo "❌ 错误: 参数过多"
    echo ""
    show_usage
    return 1
fi

if [ -n "$RESOURCE_DIR" ]; then
    RESOURCE_ROOT="$RESOURCE_DIR"
elif [ -n "$VERSION" ]; then
    RESOURCE_ROOT="$(resolve_version_dir "$VERSION")"
elif [ -n "$EXPLICIT_CONFIG_FILE" ]; then
    RESOURCE_ROOT="$(dirname "$EXPLICIT_CONFIG_FILE")"
else
    RESOURCE_ROOT="$PARENT_DIR"
fi

if [ ! -d "$RESOURCE_ROOT" ]; then
    echo "❌ 错误: 资源目录不存在: $RESOURCE_ROOT"
    return 1
fi

HEADER_FILE="$RESOURCE_ROOT/resource_id_define.h"

if [ -n "$EXPLICIT_OUTPUT_FILE" ]; then
    OUTPUT_TEMPLATE="$EXPLICIT_OUTPUT_FILE"
else
    if [ -n "$RESOURCE_DIR" ]; then
        mkdir -p "$BIN_DIR/$(basename "$RESOURCE_DIR")"
        OUTPUT_TEMPLATE="$BIN_DIR/$(basename "$RESOURCE_DIR")/resource_pack.bin"
    elif [ -n "$VERSION" ]; then
        mkdir -p "$BIN_DIR/$VERSION"
        OUTPUT_TEMPLATE="$BIN_DIR/$VERSION/resource_pack.bin"
    elif [ -n "$EXPLICIT_CONFIG_FILE" ] && [ -n "$(detect_version_from_path "$EXPLICIT_CONFIG_FILE")" ]; then
        VERSION="$(detect_version_from_path "$EXPLICIT_CONFIG_FILE")"
        mkdir -p "$BIN_DIR/$VERSION"
        OUTPUT_TEMPLATE="$BIN_DIR/$VERSION/resource_pack.bin"
    else
        OUTPUT_TEMPLATE="$PARENT_DIR/resource_pack.bin"
    fi
fi

if [ -n "$EXPLICIT_CONFIG_FILE" ]; then
    CONFIG_DIR="$(dirname "$EXPLICIT_CONFIG_FILE")"
    if [ ! -d "$CONFIG_DIR" ]; then
        echo "❌ 错误: 配置文件目录不存在: $CONFIG_DIR"
        return 1
    fi
    EXPLICIT_CONFIG_FILE="$(cd "$CONFIG_DIR" && pwd)/$(basename "$EXPLICIT_CONFIG_FILE")"
    if [ ! -f "$EXPLICIT_CONFIG_FILE" ]; then
        echo "❌ 错误: 配置文件不存在: $EXPLICIT_CONFIG_FILE"
        return 1
    fi
    RESOURCE_ROOT="$(cd "$(dirname "$EXPLICIT_CONFIG_FILE")" && pwd)"
fi

if [ -n "$EXPLICIT_CONFIG_FILE" ]; then
    AUTO_PACK_MODE="$(detect_pack_mode_from_config_path "$EXPLICIT_CONFIG_FILE")"
    if [ "$PACK_TYPE" = "auto" ]; then
        RESOLVED_PACK_MODE="$AUTO_PACK_MODE"
    else
        RESOLVED_PACK_MODE="$PACK_TYPE"
    fi
else
    AUTO_PACK_MODE="$(resolve_default_pack_mode "$RESOURCE_ROOT")"
    if [ "$PACK_TYPE" = "auto" ]; then
        RESOLVED_PACK_MODE="$AUTO_PACK_MODE"
    else
        RESOLVED_PACK_MODE="$PACK_TYPE"
    fi
fi

if [ -z "$RESOLVED_PACK_MODE" ]; then
    echo "❌ 错误: 未找到可用的资源配置文件"
    echo "资源目录: $RESOURCE_ROOT"
    return 1
fi

if [ -n "$EXPLICIT_CONFIG_FILE" ] && [[ "$RESOLVED_PACK_MODE" =~ ^(multi|all)$ ]]; then
    echo "❌ 错误: 指定单个配置文件时，不支持 $RESOLVED_PACK_MODE 模式"
    echo "   请改为指定资源目录或版本目录"
    return 1
fi

if [ -n "$EXPLICIT_CONFIG_FILE" ] && [ "$PACK_TYPE" != "auto" ]; then
    if [ "$AUTO_PACK_MODE" != "$RESOLVED_PACK_MODE" ]; then
        echo "❌ 错误: 配置文件 $EXPLICIT_CONFIG_FILE 与打包类型 $PACK_TYPE 不匹配"
        return 1
    fi
fi

if [ -z "$EXPLICIT_OUTPUT_FILE" ]; then
    OUTPUT_TEMPLATE="$(append_output_subdir "$OUTPUT_TEMPLATE" "$(normalize_output_dir_mode "$RESOLVED_PACK_MODE")")"
fi

# 确保输出目录存在
OUTPUT_DIR="$(dirname "$OUTPUT_TEMPLATE")"
if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
fi

### 显示信息
ROOT_RES_PATH_HINT="$(resolve_root_res_path)"
ROOT_RES_VERSION_HINT="$(resolve_root_res_version)"
ROOT_RES_MAJOR_HINT=$((ROOT_RES_VERSION_HINT / 100))
ROOT_RES_MINOR_HINT=$((ROOT_RES_VERSION_HINT % 100))

echo "=================================================="
echo "🚀 资源打包工具"
echo "=================================================="
echo "资源路径: ROOT_RES_PATH=${ROOT_RES_PATH_HINT}"
echo "资源版本: ROOT_RES_VERSION=${ROOT_RES_VERSION_HINT} (V$(printf '%02d' "$ROOT_RES_MAJOR_HINT").$(printf '%02d' "$ROOT_RES_MINOR_HINT"))"
if [ -n "$VERSION" ]; then
    echo "版本参数: $VERSION"
fi
if [ -n "$RESOURCE_DIR" ]; then
    echo "资源目录: $RESOURCE_DIR"
fi
echo "打包类型: $RESOLVED_PACK_MODE"
if [ -n "$EXPLICIT_CONFIG_FILE" ]; then
    echo "显式配置: $EXPLICIT_CONFIG_FILE"
fi
echo "输出目录: $OUTPUT_DIR"
echo "头文件:   $HEADER_FILE"
echo ""

### 检查头文件
if [ ! -f "$HEADER_FILE" ]; then
    echo "⚠️  警告: 头文件不存在: $HEADER_FILE"
    echo "   将使用默认配置或从 resource_id_define.h 查找"
    HEADER_FILE=""
fi

### 执行打包
echo "CRC策略: 打包时按单个资源计算 CRC16 并写入 obj_info"
PACK_EXIT_CODE=0
OUTPUT_FILE=""
PACK_STEP_CODE=0

case "$RESOLVED_PACK_MODE" in
    legacy|base|custom)
        if [ -n "$EXPLICIT_CONFIG_FILE" ]; then
            CONFIG_FILE="$EXPLICIT_CONFIG_FILE"
        else
            CONFIG_FILE="$(resolve_config_for_mode "$RESOURCE_ROOT" "$RESOLVED_PACK_MODE")"
        fi
        if [ ! -f "$CONFIG_FILE" ]; then
            echo "❌ 错误: 配置文件不存在: $CONFIG_FILE"
            return 1
        fi
        if run_pack_resources_for_mode "$RESOLVED_PACK_MODE" "$CONFIG_FILE" "$OUTPUT_TEMPLATE" "$HEADER_FILE" "$RESOURCE_ROOT"; then
            OUTPUT_FILE="$PACK_RESULT_OUTPUT"
            GENERATED_OUTPUTS+=("$OUTPUT_FILE")
        else
            PACK_STEP_CODE=$?
            PACK_EXIT_CODE=$PACK_STEP_CODE
        fi
        ;;
    multi|all)
        BASE_CONFIG_FILE="$(resolve_config_for_mode "$RESOURCE_ROOT" "base")"
        CUSTOM_CONFIG_FILE="$(resolve_config_for_mode "$RESOURCE_ROOT" "custom")"
        if [ ! -f "$BASE_CONFIG_FILE" ] || [ ! -f "$CUSTOM_CONFIG_FILE" ]; then
            echo "❌ 错误: multi/all 模式需要同时存在以下配置文件:"
            echo "   - $BASE_CONFIG_FILE"
            echo "   - $CUSTOM_CONFIG_FILE"
            return 1
        fi

        OFFSET_VALUE="$(resolve_theme_img_res_package_offset)"
        if [ -z "$OFFSET_VALUE" ]; then
            echo "❌ 错误: 无法解析 THEME_IMG_RES_PAKAGE_OFFSET"
            return 1
        fi
        echo "主题资源偏移: THEME_IMG_RES_PAKAGE_OFFSET=$OFFSET_VALUE"

        TEMP_PACK_DIR=""
        BASE_OUTPUT_TEMPLATE="$OUTPUT_TEMPLATE"
        CUSTOM_OUTPUT_TEMPLATE="$OUTPUT_TEMPLATE"
        if [ "$RESOLVED_PACK_MODE" = "multi" ]; then
            TEMP_PACK_DIR="$(mktemp -d)"
            BASE_OUTPUT_TEMPLATE="$TEMP_PACK_DIR/resource_pack.bin"
            CUSTOM_OUTPUT_TEMPLATE="$TEMP_PACK_DIR/resource_pack.bin"
        elif [ -z "$EXPLICIT_OUTPUT_FILE" ]; then
            BASE_OUTPUT_TEMPLATE="$(append_output_subdir "${OUTPUT_TEMPLATE%/*/*}/resource_pack.bin" "base")"
            CUSTOM_OUTPUT_TEMPLATE="$(append_output_subdir "${OUTPUT_TEMPLATE%/*/*}/resource_pack.bin" "custom")"
        fi

        if run_pack_resources_for_mode "base" "$BASE_CONFIG_FILE" "$BASE_OUTPUT_TEMPLATE" "$HEADER_FILE" "$RESOURCE_ROOT"; then
            BASE_PACK_OUTPUT="$PACK_RESULT_OUTPUT"
            if [ "$RESOLVED_PACK_MODE" = "all" ]; then
                GENERATED_OUTPUTS+=("$BASE_PACK_OUTPUT")
            fi
        else
            PACK_STEP_CODE=$?
            PACK_EXIT_CODE=$PACK_STEP_CODE
        fi

        if [ $PACK_EXIT_CODE -eq 0 ]; then
            if run_pack_resources_for_mode "custom" "$CUSTOM_CONFIG_FILE" "$CUSTOM_OUTPUT_TEMPLATE" "$HEADER_FILE" "$RESOURCE_ROOT"; then
                CUSTOM_PACK_OUTPUT="$PACK_RESULT_OUTPUT"
                if [ "$RESOLVED_PACK_MODE" = "all" ]; then
                    GENERATED_OUTPUTS+=("$CUSTOM_PACK_OUTPUT")
                fi
            else
                PACK_STEP_CODE=$?
                PACK_EXIT_CODE=$PACK_STEP_CODE
            fi
        fi

        if [ $PACK_EXIT_CODE -eq 0 ]; then
            MULTI_OUTPUT_FILE="$(build_multi_output_path "$BASE_PACK_OUTPUT" "$OUTPUT_TEMPLATE")"
            echo "--------------------------------------------------"
            echo "📦 开始拼接 multi 资源包"
            echo "base资源包:   $BASE_PACK_OUTPUT"
            echo "custom资源包: $CUSTOM_PACK_OUTPUT"
            echo "multi输出:    $MULTI_OUTPUT_FILE"
            if MULTI_PACK_LOG="$(create_multi_package "$BASE_PACK_OUTPUT" "$CUSTOM_PACK_OUTPUT" "$MULTI_OUTPUT_FILE" "$OFFSET_VALUE" 2>&1)"; then
                echo "$MULTI_PACK_LOG"
                OUTPUT_FILE="$(echo "$MULTI_PACK_LOG" | grep -E '^MULTI_OUTPUT_FILE=' | tail -n 1 | cut -d= -f2-)"
                if [ -z "$OUTPUT_FILE" ]; then
                    OUTPUT_FILE="$MULTI_OUTPUT_FILE"
                fi
                GENERATED_OUTPUTS+=("$OUTPUT_FILE")
            else
                echo "$MULTI_PACK_LOG"
                PACK_EXIT_CODE=$?
            fi
        fi

        if [ -n "$TEMP_PACK_DIR" ] && [ -d "$TEMP_PACK_DIR" ]; then
            rm -rf "$TEMP_PACK_DIR"
        fi
        ;;
esac

### 显示结果
if [ $PACK_EXIT_CODE -eq 0 ]; then
    echo ""
    echo "=================================================="
    echo "✅ 打包成功！"
    echo "=================================================="
    if [ ${#GENERATED_OUTPUTS[@]} -gt 1 ]; then
        echo "生成文件:"
        for output_item in "${GENERATED_OUTPUTS[@]}"; do
            echo "  - $output_item"
        done
    else
        echo "资源包位置: $OUTPUT_FILE"
    fi
    if [ -f "$OUTPUT_FILE" ]; then
        SIZE=$(ls -lh "$OUTPUT_FILE" | awk '{print $5}')
        echo "文件大小: $SIZE"
    fi
    echo ""
    if [ -n "$RESOURCE_DIR" ]; then
        echo "💡 提示:"
        echo "  - 资源目录: $(basename "$RESOURCE_DIR")"
        echo "  - 打包模式: $RESOLVED_PACK_MODE"
        echo "  - 输出目录: $OUTPUT_DIR"
    elif [ -n "$VERSION" ]; then
        echo "💡 提示:"
        echo "  - 版本: $VERSION"
        echo "  - 打包模式: $RESOLVED_PACK_MODE"
        echo "  - 输出目录: $BIN_DIR/$VERSION"
    else
        echo "💡 提示:"
        echo "  - 资源目录: $RESOURCE_ROOT"
        echo "  - 打包模式: $RESOLVED_PACK_MODE"
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

