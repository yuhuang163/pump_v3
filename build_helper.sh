#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="$PROJECT_DIR/project/bin"
SHARE_DIR="/mnt/hgfs/ubuntu_share/v3_firmware"
MAKE_ARGS=("T=ble_only_lt" "-j16")

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
	local kv_out
	if ! kv_out=$(make -C "$PROJECT_DIR" "${MAKE_ARGS[@]}" print-vars 2>/dev/null); then
		TARGET_NAME="best1306p"
		OUTPUT_DIR="$PROJECT_DIR/project/bin"
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
	TARGET_NAME=${parsed_t:-best1306p}
	OUTPUT_DIR=${parsed_bin:-"$PROJECT_DIR/project/bin"}
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

do_rebuild() {
	local t0=$(date +%s)
	echo "开始重新编译（清理 + 编译）..."
	do_clean
	do_build
	echo "重新编译完成，总用时: $(duration_since "$t0")"
}

do_build_best1306p() {
	local t0=$(date +%s)
	echo "开始编译 best1306p 目标板..."
	make -C "$PROJECT_DIR" T=best1306p -j16
	echo "best1306p 编译完成，用时: $(duration_since "$t0")"
}

do_bear_best1306p() {
	local t0=$(date +%s)
	echo "开始使用 bear 编译（生成编译数据库）..."
	bear -- make -C "$PROJECT_DIR" "${MAKE_ARGS[@]}"
	echo "bear 编译完成，用时: $(duration_since "$t0")"
}

do_bear_build() {
	local t0=$(date +%s)
	echo "开始使用 bear 编译（生成编译数据库）..."
	bear -- make -C "$PROJECT_DIR" "${MAKE_ARGS[@]}"
	echo "bear 编译完成，用时: $(duration_since "$t0")"
}

show_menu() {
	resolve_make_vars
	cat <<EOF
================ 一键编译工具 ================
1) 编译
2) 清理
3) 重新编译（清理后编译）
4) 仅拷贝编译产物到共享目录
5) bear -- make（生成编译数据库）
6) 编译（不拷贝到共享目录）
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
			build)
				do_build && copy_artifacts ;;
			clean)
				do_clean ;;
			rebuild)
				do_rebuild && copy_artifacts ;;
			best1306p|bear)
				do_bear_build && copy_artifacts ;;
			build-only)
				do_build ;;
			copy)
				copy_artifacts ;;
			*)
				echo "用法: $0 --non-interactive [build|clean|rebuild|bear|best1306p|build-only|copy]" ; exit 2 ;;
		esac
		return 0
	fi

			while true; do
		show_menu
		read -rp "请选择功能 [0-6]: " choice
		case "$choice" in
			1)
				do_build
				copy_artifacts
				;;
			2)
				do_clean
				;;
			3)
				do_rebuild
				copy_artifacts
				;;
			4)
				copy_artifacts
				;;
			5)
				do_bear_build
				copy_artifacts
				;;
			6)
				do_build
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


