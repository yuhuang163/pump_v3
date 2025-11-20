#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="$PROJECT_DIR/project/bin"
SHARE_DIR="/mnt/hgfs/ubuntu_share/v3_firmware"
MAKE_ARGS=("T=ble_only_lt" "-j16")

# 若构建阶段已明确目标产物信息，则在拷贝阶段优先使用
FORCED_OUTPUT_DIR=""
FORCED_TARGET_NAME=""

die() {
	echo "错误: $*"
	exit 1
}

duration_since() {
	local start_ts=$1
	local end_ts=$(date +%s)
	local dt=$((end_ts - start_ts))
	printf "%ds" "$dt"
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

	# 解析 ROOT_PRODUCT_ID 数值
	local root_id_token
	root_id_token=$(grep -E '^#define[[:space:]]+ROOT_PRODUCT_ID[[:space:]]+' "$info_file" | awk '{print $3}' | tail -n1 || true)
	local product_id="0"
	case "$root_id_token" in
		PRODUCT_INFO_V3PRO_VALUE|1) product_id="1" ;;
		PRODUCT_INFO_V3_VALUE|0|*) product_id="0" ;;
	esac

	# 设备名（未做复杂预处理，仅处理 PRO -> Pro）
	local device_name
	if [[ "$product_id" == "1" ]]; then
		device_name="V3PRO"
	else
		device_name="V3"
	fi
	local device_title
	device_title=$(echo "$device_name" | sed -E 's/PRO$/Pro/')

	# 类型字母（ROOT_FIRMWARE_TYPE）
	local type_char
	type_char=$(grep -E "^#define[[:space:]]+ROOT_FIRMWARE_TYPE[[:space:]]+'" "$info_file" | sed -E "s/.*'(.+)'.*/\1/" | tail -n1)
	[[ -n "${type_char:-}" ]] || type_char="P"

	# 资源版本（主/次）
	local major minor
	major=$(grep -E '^#define[[:space:]]+ROOT_RES_MAJOR_VERSION[[:space:]]+\(' "$info_file" | sed -E 's/.*\(([0-9]+)\).*/\1/' | tail -n1)
	minor=$(grep -E '^#define[[:space:]]+ROOT_RES_MINOR_VERSION[[:space:]]+\(' "$info_file" | sed -E 's/.*\(([0-9]+)\).*/\1/' | tail -n1)
	major=$(printf "%02d" "${major:-0}")
	minor=$(printf "%02d" "${minor:-0}")

	# 日期与 git 短哈希
	local today
	today=$(date +%Y%m%d)
	local git_hash
	git_hash=$(git -C "$PROJECT_DIR" rev-parse --short HEAD 2>/dev/null || echo unknown)

	# 原始产物路径
	local app_in="$PROJECT_DIR/project/bin/firmware/ble_only_lt.bin"
	local boot_in="$PROJECT_DIR/project/bin/bootloader/ota_boot_v2.bin"
	local ota_in="$PROJECT_DIR/project/bin/firmware/ble_only_lt_ota.bin"

	# 目标文件名（文件用点分 01.01；目录用下划线 01_01）
	local version_dot="${major}.${minor}"
	local version_us="${major}_${minor}"
	local prefix_file="${device_title}_${today}_${type_char}${version_dot}_${git_hash}"
	local prefix_dir="${device_title}_${type_char}${version_us}"

	local target_dir="$SHARE_DIR/$prefix_dir"
	mkdir -p "$target_dir" || die "无法创建版本目录: $target_dir"

	# 拷贝并重命名（存在则覆盖）
	local copied_any=false
	if [[ -s "$app_in" ]]; then
		cp -av "$app_in" "$target_dir/${prefix_file}_APP.bin"
		echo "APP 已归档: $target_dir/${prefix_file}_APP.bin"
		copied_any=true
	else
		echo "未找到 APP 产物: $app_in"
	fi
	if [[ -s "$boot_in" ]]; then
		cp -av "$boot_in" "$target_dir/${prefix_file}_BOOT.bin"
		echo "BOOT 已归档: $target_dir/${prefix_file}_BOOT.bin"
		copied_any=true
	else
		echo "未找到 Bootloader 产物: $boot_in"
	fi
	if [[ -s "$ota_in" ]]; then
		cp -av "$ota_in" "$target_dir/${prefix_file}_OTA.bin"
		echo "OTA 已归档: $target_dir/${prefix_file}_OTA.bin"
		copied_any=true
	else
		echo "未找到 OTA 产物: $ota_in"
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
	8) 重命名并归档产物（APP/BOOT/OTA）
	9) 编译 固件+Bootloader（不拷贝）
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
				do_build_all
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


