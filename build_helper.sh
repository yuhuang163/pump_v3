#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="$PROJECT_DIR/project/bin"
SHARE_DIR="$PROJECT_DIR/../../v3_firmware"
MAKE_ARGS=("T=ble_only_lt" "-j16")

# 若构建阶段已明确目标产物信息，则在拷贝阶段优先使用
FORCED_OUTPUT_DIR=""
FORCED_TARGET_NAME=""

die() {
	echo "错误: $*"
	exit 1
}

md5_fold4_hex_from_file() {
	# 计算文件 MD5（16 字节），将整个 MD5 折叠为 4 字节校验值（8 hex）
	# 折叠规则（按字节 XOR）：
	#   out[i] = md5[i] ^ md5[i+4] ^ md5[i+8] ^ md5[i+12]   (i=0..3)
	# 说明：该规则使用“整个 MD5”生成 4B，便于在 Windows 侧复算并验证。
	local f="$1"
	[[ -s "$f" ]] || die "文件不存在或为空，无法计算 MD5: $f"

	local md5hex
	md5hex=$(md5sum "$f" | awk '{print $1}')
	[[ ${#md5hex} -eq 32 ]] || die "md5sum 输出异常: $md5hex (file=$f)"

	local -a v=()
	for i in {0..15}; do
		local b="${md5hex:$((i * 2)):2}"
		v[$i]=$((16#$b))
	done

	local o0=$((v[0] ^ v[4] ^ v[8] ^ v[12]))
	local o1=$((v[1] ^ v[5] ^ v[9] ^ v[13]))
	local o2=$((v[2] ^ v[6] ^ v[10] ^ v[14]))
	local o3=$((v[3] ^ v[7] ^ v[11] ^ v[15]))

	printf "%02x%02x%02x%02x\n" "$o0" "$o1" "$o2" "$o3"
}

duration_since() {
	local start_ts=$1
	local end_ts=$(date +%s)
	local dt=$((end_ts - start_ts))
	printf "%ds" "$dt"
}

resolve_root_device_name() {
	# 从 boards/product_info.h 解析 ROOT_DEVICE_NAME（优先使用预处理器展开，适配 ROOT_PRODUCT_ID 等间接宏）
	# 输出示例：V3 / V3Pro
	local info_file="${1:-$PROJECT_DIR/boards/product_info.h}"
	if [[ -f "$info_file" ]]; then
		local compiler
		compiler=$(command -v gcc || command -v cc || true)
		if [[ -n "${compiler:-}" ]]; then
			local out
			out=$(
				printf '#include "%s"\nROOT_DEVICE_NAME\n' "$info_file" \
					| "$compiler" -E -P -xc -I "$PROJECT_DIR/boards" -I "$PROJECT_DIR" - 2>/dev/null \
					| awk 'NF{print; exit}' \
					| tr -d '\r'
			) || true
			# 去掉引号（ROOT_DEVICE_NAME 是字符串宏）
			out=${out#\"}
			out=${out%\"}
			if [[ -n "${out:-}" && "$out" != "ROOT_DEVICE_NAME" ]]; then
				printf "%s" "$out"
				return 0
			fi
		fi
	fi

	# 回退：按 ROOT_PRODUCT_ID 简单映射（兼容旧脚本逻辑）
	local root_id_token product_id
	root_id_token=$(grep -E '^#define[[:space:]]+ROOT_PRODUCT_ID[[:space:]]+' "$info_file" | awk '{print $3}' | tail -n1 || true)
	product_id="0"
	case "$root_id_token" in
		ROOT_PRODUCT_V3PRO_ID|1) product_id="1" ;;
		ROOT_PRODUCT_V3_ID|0|*) product_id="0" ;;
	esac
	if [[ "$product_id" == "1" ]]; then
		printf "%s" "V3Pro"
	else
		printf "%s" "V3"
	fi
}

resolve_root_res_path() {
	# 优先根据 GNUmakefile 中的产品/UI 默认配置推导 ROOT_RES_PATH。
	# 之所以不直接单独预处理 product_info.h，是因为缺少 make 侧宏定义时，
	# 条件表达式里的未定义宏会被当成 0，可能把 V1 误判成 V2。
	local makefile="$PROJECT_DIR/GNUmakefile"
	if [[ -f "$makefile" ]]; then
		local product_family ui_v1_token ui_v2_token ui_v3pro_v2_token ui_version_token
		local ui_v1_value ui_v2_value ui_v3pro_v2_value
		product_family=$(resolve_root_product_family_from_makefile "$makefile")

		ui_v1_token='$(V3_UI_V1)'
		ui_v2_token='$(V3_UI_V2)'
		ui_v3pro_v2_token='$(V3PRO_UI_V2)'
		ui_v1_value=$(awk '/^V3_UI_V1[[:space:]]*\?=/{print $3; exit}' "$makefile")
		ui_v2_value=$(awk '/^V3_UI_V2[[:space:]]*\?=/{print $3; exit}' "$makefile")
		ui_v3pro_v2_value=$(awk '/^V3PRO_UI_V2[[:space:]]*\?=/{print $3; exit}' "$makefile")

		if [[ "$product_family" == "V3Pro" ]]; then
			ui_version_token=$(awk '
				/^ifeq[[:space:]]*\(\$\(ROOT_PRODUCT_ID\),[[:space:]]*\$\(ROOT_PRODUCT_V3PRO_ID\)\)/ { in_v3pro=1; next }
				in_v3pro && /^else$/ { exit }
				in_v3pro && /^V3_UI_VERSION[[:space:]]*\?=/ { print $3; exit }
			' "$makefile")
		else
			ui_version_token=$(awk '
				/^ifeq[[:space:]]*\(\$\(ROOT_PRODUCT_ID\),[[:space:]]*\$\(ROOT_PRODUCT_V3PRO_ID\)\)/ { in_block=1; next }
				in_block && /^else$/ { in_else=1; next }
				in_else && /^endif$/ { exit }
				in_else && /^V3_UI_VERSION[[:space:]]*\?=/ { print $3; exit }
			' "$makefile")
		fi

		case "$ui_version_token" in
			'$(V3_UI_V1)') printf "%s" "V1"; return 0 ;;
			'$(V3_UI_V2)'|'$(V3PRO_UI_V2)') printf "%s" "V2"; return 0 ;;
			"$ui_v1_value") [[ -n "${ui_v1_value:-}" ]] && { printf "%s" "V1"; return 0; } ;;
			"$ui_v2_value"|"$ui_v3pro_v2_value")
				if [[ -n "${ui_version_token:-}" ]]; then
					printf "%s" "V2"
					return 0
				fi
				;;
		esac
		if [[ -n "${ui_version_token:-}" ]]; then
			case "$ui_version_token" in
				*V1*) printf "%s" "V1"; return 0 ;;
				*V2*) printf "%s" "V2"; return 0 ;;
			esac
		fi
	fi

	# 回退：通过预处理展开 ROOT_RES_PATH（适配 V3_UI_VERSION 等条件编译）
	# 优先使用编译器预处理，失败时退回 grep 文本解析
	local info_file="${1:-$PROJECT_DIR/boards/product_info.h}"
	if [[ -f "$info_file" ]]; then
		local compiler
		compiler=$(command -v gcc || command -v cc || true)
		if [[ -n "${compiler:-}" ]]; then
			local out
			out=$(
				printf '#include "%s"\nROOT_RES_PATH\n' "$info_file" \
					| "$compiler" -E -P -xc -I "$PROJECT_DIR/boards" -I "$PROJECT_DIR" - 2>/dev/null \
					| awk 'NF{print; exit}' \
					| tr -d '\r'
			) || true
			# 去掉可能的引号
			out=${out#\"}
			out=${out%\"}
			if [[ -n "${out:-}" && "$out" != "ROOT_RES_PATH" ]]; then
				printf "%s" "$out"
				return 0
			fi
		fi
	fi

	# 回退：直接 grep 最近一次定义
	local token
	token=$(grep -E '^#define[[:space:]]+ROOT_RES_PATH[[:space:]]+' "$info_file" | awk '{print $3}' | tail -n1 || true)
	token=${token//\"/}
	if [[ -n "${token:-}" ]]; then
		printf "%s" "$token"
	else
		printf "%s" "V1"
	fi
}

resolve_root_product_family_from_makefile() {
	# 根据 GNUmakefile 中的 ROOT_PRODUCT_ID 默认配置判断当前产品族
	# 输出示例：V3 / V3Pro
	local makefile="${1:-$PROJECT_DIR/GNUmakefile}"
	[[ -f "$makefile" ]] || { printf "%s" "V3"; return 0; }

	local v3_id v3pro_id product_id_token
	v3_id=$(awk '/^ROOT_PRODUCT_V3_ID[[:space:]]*\?=/{print $3; exit}' "$makefile")
	v3pro_id=$(awk '/^ROOT_PRODUCT_V3PRO_ID[[:space:]]*\?=/{print $3; exit}' "$makefile")
	product_id_token=$(awk '/^ROOT_PRODUCT_ID[[:space:]]*\?=/{print $3; exit}' "$makefile")

	case "$product_id_token" in
		'$(ROOT_PRODUCT_V3PRO_ID)') product_id_token="$v3pro_id" ;;
		'$(ROOT_PRODUCT_V3_ID)') product_id_token="$v3_id" ;;
	esac

	if [[ -n "${v3pro_id:-}" && "$product_id_token" == "$v3pro_id" ]]; then
		printf "%s" "V3Pro"
	else
		printf "%s" "V3"
	fi
}

build_resource_bin_dirs() {
	# 根据资源路径与 GNUmakefile 中的产品配置，生成资源包搜索目录候选列表
	# V1 保持不变；V2 在 V3Pro 项目中优先查 bin/V3PRO_V2，再回退 bin/V2
	local res_path="$1"
	local product_family="$2"

	RESOURCE_BIN_DIRS=()
	case "$res_path" in
		V1)
			RESOURCE_BIN_DIRS+=("$PROJECT_DIR/overlay/resources/bin/V1")
			;;
		V2)
			if [[ "$product_family" == "V3Pro" ]]; then
				RESOURCE_BIN_DIRS+=(
					"$PROJECT_DIR/overlay/resources/bin/V3PRO_V2"
					"$PROJECT_DIR/overlay/resources/bin/V2"
				)
			else
				RESOURCE_BIN_DIRS+=("$PROJECT_DIR/overlay/resources/bin/V2")
			fi
			;;
		V3PRO_V2)
			RESOURCE_BIN_DIRS+=(
				"$PROJECT_DIR/overlay/resources/bin/V3PRO_V2"
				"$PROJECT_DIR/overlay/resources/bin/V2"
			)
			;;
		*)
			RESOURCE_BIN_DIRS+=("$PROJECT_DIR/overlay/resources/bin/$res_path")
			;;
	esac
}

resolve_make_vars() {
	# 若在构建阶段已指定，直接采用，避免误判
	if [[ -n "${FORCED_OUTPUT_DIR}" && -n "${FORCED_TARGET_NAME}" ]]; then
		TARGET_NAME="$FORCED_TARGET_NAME"
		OUTPUT_DIR="$FORCED_OUTPUT_DIR"
		return
	fi
	local kv_out
	if ! kv_out=$(make -C "$PROJECT_DIR" "${MAKE_ARGS[@]}" print-vars 2>/dev/null); then
		TARGET_NAME="best1306p"
		OUTPUT_DIR="$PROJECT_DIR/project/bin/firmware"
		return
	fi
	local key value
	local parsed_t="" parsed_bin=""
	while IFS='=' read -r key value; do
		case "$key" in
			T) parsed_t="$value" ;;
			BIN_DIR) parsed_bin="$value" ;;
		esac
	done <<< "$kv_out"
	# 当 T 含斜杠时，仅取 basename 作为产物前缀名
	local t_base
	if [[ -n "${parsed_t:-}" ]]; then
		t_base=$(basename -- "$parsed_t")
	else
		t_base="best1306p"
	fi
	TARGET_NAME="$t_base"
	OUTPUT_DIR=${parsed_bin:-"$PROJECT_DIR/project/bin/firmware"}
}

ensure_share_dir() {
	[[ -d "$SHARE_DIR" ]] || mkdir -p "$SHARE_DIR" || die "无法创建共享目录: $SHARE_DIR"
	if ! (touch "$SHARE_DIR/.write_test" && rm -f "$SHARE_DIR/.write_test"); then
		die "没有共享目录写权限: $SHARE_DIR"
	fi
	echo "共享目录可用: $SHARE_DIR"
}

copy_artifacts() {
	ensure_share_dir
	resolve_make_vars
	local artifacts=("${TARGET_NAME}.bin" "${TARGET_NAME}.elf" "${TARGET_NAME}.map" "${TARGET_NAME}.lst")
	# 去除 .lst 拷贝
	local artifacts=("${TARGET_NAME}.bin" "${TARGET_NAME}.elf" "${TARGET_NAME}.map")
	local found_any=false
	for f in "${artifacts[@]}"; do
		if [[ -s "$OUTPUT_DIR/$f" ]]; then
			found_any=true
			cp -av "$OUTPUT_DIR/$f" "$SHARE_DIR"/
			echo "已拷贝: $f -> $SHARE_DIR/"
		else
			echo "缺少或为空的产物: $OUTPUT_DIR/$f"
		fi
	done
	if [[ "$found_any" == false ]]; then
		die "未发现可拷贝的产物（目录: $OUTPUT_DIR）"
	else
		echo "产物拷贝完成。"
		printf "\n已拷贝产物到: %s\n" "$SHARE_DIR"
	fi
}

# 同时拷贝固件与 Bootloader 产物；任一存在即算成功
copy_all_artifacts() {
	ensure_share_dir
	local any=0
	( FORCED_TARGET_NAME="ble_only_lt" FORCED_OUTPUT_DIR="$PROJECT_DIR/project/bin/firmware" copy_artifacts ) && any=1 || true
	( FORCED_TARGET_NAME="ota_boot_v2" FORCED_OUTPUT_DIR="$PROJECT_DIR/project/bin/bootloader" copy_artifacts ) && any=1 || true
	# 同时尝试拷贝 OTA 产物
	if copy_ota_artifact; then
		any=1
	fi
	if [[ "$any" -eq 0 ]]; then
		die "未发现可拷贝的产物（固件或 Bootloader）"
	fi
}

do_build() {
	local t0=$(date +%s)
	echo "开始编译..."
	make -C "$PROJECT_DIR" "${MAKE_ARGS[@]}"
	echo "编译完成，用时: $(duration_since "$t0")"
}

do_clean() {
	local t0=$(date +%s)
	echo "开始清理..."
	make -C "$PROJECT_DIR" "${MAKE_ARGS[@]}" clean
	echo "清理完成，用时: $(duration_since "$t0")"
}

do_clean_firmware() {
	local t0=$(date +%s)
	echo "开始清理 固件(ble_only_lt)..."
	make -C "$PROJECT_DIR" -j16 T=ble_only_lt \
		O="$PROJECT_DIR/project/build/firmware" \
		BIN_DIR="$PROJECT_DIR/project/bin/firmware" \
		allclean
	echo "固件清理完成，用时: $(duration_since "$t0")"
}

do_clean_bootloader() {
	local t0=$(date +%s)
	echo "开始清理 Bootloader(ota_boot_v2)..."
	make -C "$PROJECT_DIR" -j16 T=ota_boot_v2 \
		O="$PROJECT_DIR/project/build/bootloader" \
		BIN_DIR="$PROJECT_DIR/project/bin/bootloader" \
		allclean
	echo "Bootloader 清理完成，用时: $(duration_since "$t0")"
}

do_rebuild() {
	local t0=$(date +%s)
	echo "开始重新编译（清理 + 编译）..."
	do_clean
	do_build
	echo "重新编译完成，总用时: $(duration_since "$t0")"
}

do_rebuild_firmware() {
	local t0=$(date +%s)
	echo "开始重新编译 固件（清理 + 编译）..."
	do_clean_firmware
	do_build_firmware
	echo "固件重新编译完成，总用时: $(duration_since "$t0")"
}

do_build_firmware() {
	local t0=$(date +%s)
	echo "开始编译 固件(ble_only_lt)..."
	make -C "$PROJECT_DIR" -j16 firmware
	# 记录本次构建的产物目录与产物前缀
	FORCED_TARGET_NAME="ble_only_lt"
	FORCED_OUTPUT_DIR="$PROJECT_DIR/project/bin/firmware"
	echo "固件编译完成，用时: $(duration_since "$t0")"
}

do_build_bootloader() {
	local t0=$(date +%s)
	echo "开始编译 Bootloader(ota_boot_v2)..."
	make -C "$PROJECT_DIR" -j16 bootloader
	# 记录本次构建的产物目录与产物前缀（basename: ota_boot_v2）
	FORCED_TARGET_NAME="ota_boot_v2"
	FORCED_OUTPUT_DIR="$PROJECT_DIR/project/bin/bootloader"
	echo "Bootloader 编译完成，用时: $(duration_since "$t0")"
}

do_build_all() {
	local t0=$(date +%s)
	echo "开始编译 固件+Bootloader（不拷贝）..."
	do_build_firmware
	do_build_bootloader
	echo "固件+Bootloader 编译完成，总用时: $(duration_since "$t0")"
}

do_rebuild_all() {
	local t0=$(date +%s)
	echo "开始重新编译 固件+Bootloader（清理 + 编译，不拷贝）..."
	do_clean_firmware
	do_clean_bootloader
	do_build_all
	echo "固件+Bootloader 重新编译完成，总用时: $(duration_since "$t0")"
}

do_bear_build() {
	local t0=$(date +%s)
	echo "开始使用 bear 编译（生成编译数据库）..."
    # 先清理固件构建目录，确保编译数据库完整且无陈旧目标
    do_clean_firmware
    # 与常规固件构建保持一致，确保 O/BIN_DIR 与编译数据库一致
    bear -- make -C "$PROJECT_DIR" -j16 firmware
	echo "bear 编译完成，用时: $(duration_since "$t0")"
}

# 生成压缩 OTA 固件：输入为应用固件 bin，输出为 *_ota.bin
do_build_ota() {
	local t0=$(date +%s)
	echo "开始生成 OTA 固件(压缩)..."
	local in_bin="$PROJECT_DIR/project/bin/firmware/ble_only_lt.bin"
	local out_ota="$PROJECT_DIR/project/bin/firmware/ble_only_lt_ota.bin"
	local tool="$PROJECT_DIR/trunk/platform_chip/bes/sdk/tools/build_compressed_ota.sh"
	[[ -s "$in_bin" ]] || die "未找到输入固件: $in_bin"
	if [[ ! -f "$tool" ]]; then
		die "未找到 OTA 构建脚本: $tool"
	fi
	bash "$tool" "$in_bin" "$out_ota"
	[[ -s "$out_ota" ]] || die "OTA 生成失败，未产生文件: $out_ota"
	echo "OTA 生成完成，用时: $(duration_since "$t0")"
}

# 拷贝 OTA 产物到共享目录（若存在）
copy_ota_artifact() {
	ensure_share_dir
	local ota_file="$PROJECT_DIR/project/bin/firmware/ble_only_lt_ota.bin"
	if [[ -s "$ota_file" ]]; then
		cp -av "$ota_file" "$SHARE_DIR"/
		echo "已拷贝 OTA: $(basename -- "$ota_file") -> $SHARE_DIR/"
		return 0
	else
		echo "缺少或为空的 OTA 产物: $ota_file"
		return 1
	fi
}

# 解析产品信息并重命名/归档 APP、BOOT、OTA 产物
do_rename_and_archive() {
	ensure_share_dir
	local info_file="$PROJECT_DIR/boards/product_info.h"
	[[ -f "$info_file" ]] || die "未找到产品信息文件: $info_file"

	local product_family device_title
	product_family=$(resolve_root_product_family_from_makefile "$PROJECT_DIR/GNUmakefile")
	device_title="${product_family:-}"
	if [[ -z "${device_title:-}" ]]; then
		device_title=$(resolve_root_device_name "$info_file")
	fi
	[[ -n "${device_title:-}" ]] || device_title="V3"

	# 类型字母（ROOT_FIRMWARE_TYPE）
	local type_char
	type_char=$(grep -E "^#define[[:space:]]+ROOT_FIRMWARE_TYPE[[:space:]]+'" "$info_file" | sed -E "s/.*'(.+)'.*/\1/" | tail -n1)
	[[ -n "${type_char:-}" ]] || type_char="P"

	# 固件版本（主/次）：依赖 ROOT_FIRMWARE_VERSION（如 100 -> 01 / 00）
	local fw_version major minor
	fw_version=$(grep -E '^#define[[:space:]]+ROOT_FIRMWARE_VERSION[[:space:]]+' "$info_file" | awk '{print $3}' | tail -n1 || true)
	[[ -n "${fw_version:-}" ]] || fw_version="0"
	major=$((10#$fw_version / 100))
	minor=$((10#$fw_version % 100))
	major=$(printf "%02d" "$major")
	minor=$(printf "%02d" "$minor")

	# 硬件版本（ROOT_HARDWARE_VERSION）：10 -> H01.0（major=1, minor=0）
	local hw_version hw_major hw_minor hw_major2 hw_tag
	hw_version=$(grep -E '^#define[[:space:]]+ROOT_HARDWARE_VERSION[[:space:]]+' "$info_file" | awk '{print $3}' | tail -n1 || true)
	[[ -n "${hw_version:-}" ]] || hw_version="0"
	hw_major=$((10#$hw_version / 10))
	hw_minor=$((10#$hw_version % 10))
	# 显示规则：硬件主版本不补零（H01.0 -> H1.0；H00.2 -> H0.2）
	hw_tag="H${hw_major}.${hw_minor}"

	# 日期与 git 短哈希
	local today
	today=$(date +%Y%m%d)
	local git_hash
	git_hash=$(git -C "$PROJECT_DIR" rev-parse --short HEAD 2>/dev/null || echo unknown)

	# 原始产物路径
	local app_in="$PROJECT_DIR/project/bin/firmware/ble_only_lt.bin"
	local app_elf="$PROJECT_DIR/project/bin/firmware/ble_only_lt.elf"
	local app_map="$PROJECT_DIR/project/bin/firmware/ble_only_lt.map"
	local boot_in="$PROJECT_DIR/project/bin/bootloader/ota_boot_v2.bin"
	local ota_in="$PROJECT_DIR/project/bin/firmware/ble_only_lt_ota.bin"

	# 目标文件名（文件用点分 01.01；目录用下划线 01_01）
	local version_dot="${major}.${minor}"
	local version_us="${major}_${minor}"
	# 文件名前缀增加 Lute_；目录 ROOT_ 前缀改为 Lute_
	# 在软件版本后追加硬件版本号（用 _ 分隔）：..._P01.04_H01.0_<git>
	local prefix_file="Lute_${device_title}_${today}_${type_char}${version_dot}_${hw_tag}_${git_hash}"
	local prefix_dir="Lute_${device_title}_${type_char}${version_us}"

	local target_dir="$SHARE_DIR/$prefix_dir"
	mkdir -p "$target_dir" || die "无法创建版本目录: $target_dir"

	# 同一版本目录下，先清理旧归档文件（全部 .bin），避免残留多份（日期/哈希不同导致文件名不同）
	rm -f "$target_dir"/*.bin 2>/dev/null || true

	# 根据 ROOT_RES_PATH/ROOT_RES_VERSION 拷贝资源
	# pack.sh 更新后，multi/all 产物默认落在 bin/<版本>/all/ 目录；
	# 这里优先匹配 all 目录下的 *_RES_all.bin，并兼容旧目录里的 *_RES.bin。
	# V1 保持 bin/V1；V2 保持 bin/V2，且在 GNUmakefile 指定为 V3Pro 产品时优先查 bin/V3PRO_V2
	# ROOT_RES_VERSION 定义为 major*100 + minor，例如 100 -> V01.00
	local res_path res_version res_major res_minor res_major2 res_minor2 res_dir res_file res_source_dir
	local res_lookup_dir
	local searched_res_dirs
	res_path=$(resolve_root_res_path "$info_file")
	[[ -n "${res_path:-}" ]] || res_path="V1"

	res_version=$(grep -E '^#define[[:space:]]+ROOT_RES_VERSION[[:space:]]+' "$info_file" \
		| sed -E 's/.*\(([0-9]+)\).*/\1/; t; s/.*[[:space:]]+([0-9]+).*/\1/' \
		| tail -n1 || true)
	[[ -n "${res_version:-}" ]] || res_version="0"

	res_major=$((10#$res_version / 100))
	res_minor=$((10#$res_version % 100))
	res_major2=$(printf "%02d" "$res_major")
	res_minor2=$(printf "%02d" "$res_minor")

	build_resource_bin_dirs "$res_path" "$product_family"
	res_dir="${RESOURCE_BIN_DIRS[0]}"
	res_file=""
	res_source_dir=""
	searched_res_dirs=$(printf "%s " "${RESOURCE_BIN_DIRS[@]}")
	searched_res_dirs=${searched_res_dirs% }
	for res_dir in "${RESOURCE_BIN_DIRS[@]}"; do
		if [[ -d "$res_dir" ]]; then
			for res_lookup_dir in "$res_dir/all" "$res_dir"; do
				[[ -d "$res_lookup_dir" ]] || continue
				# 优先取 all 目录下新的 *_RES_all.bin，找不到时再回退旧的 *_RES.bin。
				shopt -s nullglob
				local candidates=(
					"$res_lookup_dir"/${device_title}_*_V${res_major2}.${res_minor2}_*_RES_all.bin
					"$res_lookup_dir"/${device_title}_*_V${res_major2}.${res_minor2}_*_RES.bin
				)
				shopt -u nullglob
				if ((${#candidates[@]})); then
					res_file=$(ls -1t "${candidates[@]}" 2>/dev/null | awk 'NR==1{print; exit}' || true)
					if [[ -n "${res_file:-}" ]]; then
						res_source_dir="$res_lookup_dir"
						break 2
					fi
				fi
			done
		fi
	done

	# 拷贝并重命名（存在则覆盖）
	local copied_any=false
	if [[ -s "$app_in" ]]; then
		local app_md5
		app_md5=$(md5_fold4_hex_from_file "$app_in")
		cp -av "$app_in" "$target_dir/${prefix_file}_${app_md5}_APP.bin"
		echo "APP 已归档: $target_dir/${prefix_file}_${app_md5}_APP.bin"
		copied_any=true
		# 同时拷贝 ELF 和 MAP 文件到 debug_file 子目录
		local debug_dir="$target_dir/debug_file"
		mkdir -p "$debug_dir" || die "无法创建调试文件目录: $debug_dir"
		if [[ -s "$app_elf" ]]; then
			cp -av "$app_elf" "$debug_dir/${prefix_file}_${app_md5}_APP.elf"
			echo "APP ELF 已归档: $debug_dir/${prefix_file}_${app_md5}_APP.elf"
		else
			echo "未找到 APP ELF 产物: $app_elf"
		fi
		if [[ -s "$app_map" ]]; then
			cp -av "$app_map" "$debug_dir/${prefix_file}_${app_md5}_APP.map"
			echo "APP MAP 已归档: $debug_dir/${prefix_file}_${app_md5}_APP.map"
		else
			echo "未找到 APP MAP 产物: $app_map"
		fi
	else
		echo "未找到 APP 产物: $app_in"
	fi
	if [[ -s "$boot_in" ]]; then
		local boot_md5
		boot_md5=$(md5_fold4_hex_from_file "$boot_in")
		cp -av "$boot_in" "$target_dir/${prefix_file}_${boot_md5}_BOOT.bin"
		echo "BOOT 已归档: $target_dir/${prefix_file}_${boot_md5}_BOOT.bin"
		copied_any=true
	else
		echo "未找到 Bootloader 产物: $boot_in"
	fi
	if [[ -s "$ota_in" ]]; then
		local ota_md5
		ota_md5=$(md5_fold4_hex_from_file "$ota_in")
		cp -av "$ota_in" "$target_dir/${prefix_file}_${ota_md5}_OTA.bin"
		echo "OTA 已归档: $target_dir/${prefix_file}_${ota_md5}_OTA.bin"
		copied_any=true
	else
		echo "未找到 OTA 产物: $ota_in"
	fi

	if [[ -n "${res_file:-}" && -s "$res_file" ]]; then
		local res_md5
		res_md5=$(md5_fold4_hex_from_file "$res_file")
		# RES：仅追加 Lute 前缀 + 硬件版本号，不覆盖资源自身的日期/版本等字段
		local res_base res_out
		res_base=$(basename -- "$res_file")
		if [[ "$res_base" == *_RES.bin ]]; then
			# 例：V3_20260120_V01.01_8e7a2e66_RES.bin -> Lute_V3_20260120_V01.01_H01.0_8e7a2e66_<md5>_RES.bin
			# 规则：硬件版本追加在资源版本号之后（Vxx.xx_Hxx.x）
			local res_nores="${res_base%_RES.bin}"
			if [[ "$res_nores" =~ ^(.*_V[0-9]{2}\.[0-9]{2})_(.*)$ ]]; then
				res_out="Lute_${BASH_REMATCH[1]}_${hw_tag}_${BASH_REMATCH[2]}_${res_md5}_RES.bin"
			else
				# 若未匹配到资源版本号段，则退化为直接在末尾追加
				res_out="Lute_${res_nores}_${hw_tag}_${res_md5}_RES.bin"
			fi
		else
			# 兜底：直接在扩展名前追加
			res_out="Lute_${res_base%.bin}_${hw_tag}_${res_md5}.bin"
		fi
		cp -av "$res_file" "$target_dir/$res_out"
		echo "RES 来源文件: $res_file"
		echo "RES 已归档(${res_path} V${res_major2}.${res_minor2}, source=${res_source_dir}): $target_dir/$res_out"
		copied_any=true
	else
		echo "未找到 RES 资源(${res_path} V${res_major2}.${res_minor2}, product=${product_family})，已搜索: ${searched_res_dirs} (优先 all/*_RES_all.bin，回退 *_RES.bin)"
	fi

	if [[ "$copied_any" == false ]]; then
		die "未找到可归档的产物"
	fi
	echo "产物已归档到: $target_dir"
}

show_menu() {
	resolve_make_vars
	cat <<EOF
	================ 一键编译工具 ================
	1) 编译 固件（并拷贝）
	2) 清理 固件
	3) bear -- make（生成编译数据库）
	4) 编译 Bootloader（并拷贝）
	5) 清理 Bootloader
	6) 生成 OTA（并拷贝）
	7) 仅拷贝产物到共享目录（固件+Bootloader+OTA）
	8) 重命名并归档产物（APP/BOOT/OTA/RES，文件名追加 MD5 折叠 4B/8hex）
	9) 重新编译 固件+Bootloader（清理 + 编译，不拷贝）
0) 退出
---------------------------------------------
目标共享目录: $SHARE_DIR
当前目标板: $TARGET_NAME
编译输出目录: $OUTPUT_DIR
=============================================
EOF
}

main() {
	if [[ ${1-} == "--non-interactive" ]]; then
		# 预留的非交互模式：例如 CI 调用
		case ${2-} in
			build-firmware)
				do_build_firmware && copy_artifacts ;;
			build-bootloader)
				do_build_bootloader && copy_artifacts ;;
			clean-firmware)
				do_clean_firmware ;;
			clean-bootloader)
				do_clean_bootloader ;;
			rebuild-firmware)
				do_rebuild_firmware && copy_artifacts ;;
			build-only)
				do_build_firmware ;;
			copy)
				copy_artifacts ;;
			bear)
				do_bear_build ;;
			build-bootloader-only)
				do_build_bootloader ;;
			build-all)
				do_build_all ;;
			*)
				echo "用法: $0 --non-interactive [build-firmware|build-bootloader|build-bootloader-only|clean-firmware|clean-bootloader|rebuild-firmware|build-only|copy|bear|build-all]" ; exit 2 ;;
		esac
		return 0
	fi

			while true; do
		show_menu
	read -rp "请选择功能 [0-9]: " choice
		case "$choice" in
			1)
				do_build_firmware
				copy_artifacts
				;;
			2)
				do_clean_firmware
				;;
			3)
				do_bear_build
				;;
			4)
				do_build_bootloader
				copy_artifacts
				;;
			5)
				do_clean_bootloader
				;;
			6)
				do_build_ota
				copy_ota_artifact
				;;
			7)
				copy_all_artifacts
				;;
			8)
				do_rename_and_archive
				;;
			9)
				do_rebuild_all
				;;
			0)
				echo "已退出"; exit 0 ;;
			*)
				echo "无效选择，请重新输入。" ;;
		esac
		echo
		read -rp "按回车键返回菜单，或输入 q 退出: " back
		[[ "$back" == "q" || "$back" == "Q" ]] && { echo "已退出"; exit 0; }
	done
}

main "$@"


