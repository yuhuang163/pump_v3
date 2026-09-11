#!/bin/bash
###
### 语言包打包脚本（文案/坐标 + overlay/font 字库 bin + font_img）
### 使用方法: ./pack.sh [选项]  # 详见 ./pack.sh --help
###

PACK_SH_SOURCED=0
if [ "${BASH_SOURCE[0]}" != "$0" ]; then
    PACK_SH_SOURCED=1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FONT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN_DIR="$FONT_DIR/bin"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"
PACKER="$SCRIPT_DIR/pack_lang.py"
LANG_PACK_FILE_VERSION="${LANG_PACK_FILE_VERSION:-100}"

show_usage() {
    echo "使用方法:"
    echo "  ./pack.sh [-m] [-l zh|en] [--ui v3|v3pro] [V7|v3pro|v3] [zh|en]"
    echo ""
    echo "选项:"
    echo "  -m, --menu             通过数字菜单选择产品与语言"
    echo "  -l, --lang LANG        语言: zh|en，默认 zh"
    echo "      --ui UI            UI: v3|v3pro，默认按 GNUmakefile 产品族"
    echo "      --version VERSION  输出文件名版本号，默认 100，表示 V01.00"
    echo "  -h, --help             显示帮助信息"
    echo ""
    echo "示例:"
    echo "  ./pack.sh                         # 按 GNUmakefile 产品+UI，打包中文语言包"
    echo "  ./pack.sh --menu                  # 交互选择产品与语言"
    echo "  ./pack.sh V7 zh                   # 当前产品族 + UI v7 + 中文"
    echo "  ./pack.sh v3pro zh                # V3Pro / UI v7 / 中文"
    echo "  ./pack.sh --ui v3 -l zh           # V3 中文包（product-id 0x08）"
    echo ""
    echo "字库与图片文字目录:"
    echo "  overlay/font/font_v3/lang/zh/bin/   # 中文 Noto 19/22/26/29/33"
    echo "  overlay/font/font_v3/lang/en/bin/   # 英文 Aeonik 19/22/26/29/33"
    echo "  overlay/font/font_v3/lang/<zh|en>/font_img/"
    echo ""
    echo "默认输出:"
    echo "  overlay/font/bin/font_v3/<v3|v3pro>/<lang>/<PREFIX>_<YYYYMMDD>_VXX.XX_<MD5>_LFS.bin"
    echo "      烧录: 外部 Flash 0xE00000，长度 0x180000（整段 FDS_EXT littlefs）"
    echo "      镜像内含 LANG 清单 + 各页面/字号/图片 key + RDY1"
    echo "  overlay/font/bin/font_v3/<v3|v3pro>/<lang>/<PREFIX>_<YYYYMMDD>_VXX.XX_<MD5>_OTA.bin"
    echo "      给 OTA TYPE=0x04 整包升级"
    echo ""
    echo "预览:"
    echo "  ./preview.sh --install"
    echo "  ./preview.sh --open [LFS|OTA.bin]"
}

resolve_root_product_family_from_makefile() {
    local makefile="$PROJECT_DIR/GNUmakefile"
    if [ ! -f "$makefile" ]; then
        printf "%s" "V3Pro"
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

resolve_root_ui_version() {
    local makefile="$PROJECT_DIR/GNUmakefile"
    if [ ! -f "$makefile" ]; then
        printf "%s" "0x07"
        return 0
    fi
    local token
    token="$(awk '/^ROOT_UI_VERSION[[:space:]]*\?=/{print $3; exit}' "$makefile")"
    case "$token" in
        '$(ROOT_UI_V7)') printf "%s" "0x07"; return 0 ;;
        '$(ROOT_UI_V6)') printf "%s" "0x06"; return 0 ;;
        '$(ROOT_UI_V5)') printf "%s" "0x05"; return 0 ;;
    esac
    if [[ "$token" =~ ^0x ]]; then
        printf "%s" "$token"
        return 0
    fi
    printf "%s" "0x07"
}

ui_from_product_family() {
    case "$1" in
        V3Pro|V3PRO|v3pro) printf "%s" "v3pro" ;;
        *) printf "%s" "v3" ;;
    esac
}

select_from_menu() {
    local family=""
    local lang=""
    local choice=""

    echo "================ 语言包打包 ================" >&2
    echo "1) V3Pro / 中文" >&2
    echo "2) V3Pro / 英文" >&2
    echo "3) V3 / 中文" >&2
    echo "4) V3 / 英文" >&2
    echo "0) 取消" >&2
    echo "---------------------------------------------" >&2
    while true; do
        read -rp "请选择编号 [0-4]: " choice >&2
        case "$choice" in
            0) return 1 ;;
            1) UI_SEL="v3pro"; LANG_SEL="zh"; return 0 ;;
            2) UI_SEL="v3pro"; LANG_SEL="en"; return 0 ;;
            3) UI_SEL="v3"; LANG_SEL="zh"; return 0 ;;
            4) UI_SEL="v3"; LANG_SEL="en"; return 0 ;;
            *) echo "无效选择，请重新输入。" >&2 ;;
        esac
    done
}

pack_one() {
    local ui="$1"
    local lang="$2"
    local product_id="$3"
    local ui_version="$4"
    local pack_log
    local pack_exit_code=0

    echo "--------------------------------------------------"
    echo "📦 开始打包语言包"
    echo "产品 UI:  $ui"
    echo "语言:     $lang"
    echo "product:  $product_id"
    echo "ui_ver:   $ui_version"
    echo "字库根:   $FONT_DIR/font_v3/lang/$lang"
    echo "输出目录: $BIN_DIR/font_v3/$ui/$lang"

    set +e
    pack_log="$(python3 "$PACKER" \
        --ui "$ui" \
        --lang "$lang" \
        --font-root "$FONT_DIR/font_v3" \
        --product-id "$product_id" \
        --ui-version "$ui_version" \
        --file-version "$LANG_PACK_FILE_VERSION" 2>&1)"
    pack_exit_code=$?
    set -e
    echo "$pack_log"
    return $pack_exit_code
}

pack_main() {
set -e

USE_MENU=0
UI_SEL=""
LANG_SEL="zh"
POSITIONAL=()

while [ $# -gt 0 ]; do
    case "$1" in
        -m|--menu)
            USE_MENU=1
            shift
            ;;
        -l|--lang)
            if [ $# -lt 2 ]; then
                echo "❌ 错误: $1 需要指定语言 zh|en"
                show_usage
                return 1
            fi
            LANG_SEL="$2"
            shift 2
            ;;
        --ui)
            if [ $# -lt 2 ]; then
                echo "❌ 错误: $1 需要指定 v3|v3pro"
                show_usage
                return 1
            fi
            UI_SEL="$2"
            shift 2
            ;;
        --version|--file-version)
            if [ $# -lt 2 ]; then
                echo "❌ 错误: $1 需要指定版本号"
                show_usage
                return 1
            fi
            LANG_PACK_FILE_VERSION="$2"
            shift 2
            ;;
        -h|--help)
            show_usage
            return 0
            ;;
        *)
            POSITIONAL+=("$1")
            shift
            ;;
    esac
done

if [ ! -f "$PACKER" ]; then
    echo "❌ 错误: 找不到打包器: $PACKER"
    return 1
fi

if [ "$USE_MENU" -eq 1 ]; then
    if ! select_from_menu; then
        echo "已取消语言包打包。"
        return 0
    fi
else
    for arg in "${POSITIONAL[@]+"${POSITIONAL[@]}"}"; do
        case "$arg" in
            V7|v7)
                UI_VERSION_HINT="0x07"
                ;;
            V3PRO|V3Pro|v3pro)
                UI_SEL="v3pro"
                ;;
            V3|v3)
                UI_SEL="v3"
                ;;
            zh|en)
                LANG_SEL="$arg"
                ;;
            *)
                echo "❌ 错误: 无法识别参数: $arg"
                show_usage
                return 1
                ;;
        esac
    done
    if [ -z "$UI_SEL" ]; then
        UI_SEL="$(ui_from_product_family "$(resolve_root_product_family_from_makefile)")"
    fi
fi

case "$LANG_SEL" in
    zh|en) ;;
    *)
        echo "❌ 错误: 语言必须是 zh 或 en: $LANG_SEL"
        return 1
        ;;
esac
case "$UI_SEL" in
    v3|v3pro) ;;
    *)
        echo "❌ 错误: UI 必须是 v3 或 v3pro: $UI_SEL"
        return 1
        ;;
esac

if [ "$UI_SEL" = "v3pro" ]; then
    PRODUCT_ID="0x09"
else
    PRODUCT_ID="0x08"
fi
UI_VERSION="${UI_VERSION_HINT:-$(resolve_root_ui_version)}"

echo "=================================================="
echo "🚀 语言包打包工具"
echo "=================================================="
echo "产品族: $(resolve_root_product_family_from_makefile)"
echo "UI:     $UI_SEL"
echo "语言:   $LANG_SEL"
echo "文件版本号: $LANG_PACK_FILE_VERSION"
echo ""

if ! pack_one "$UI_SEL" "$LANG_SEL" "$PRODUCT_ID" "$UI_VERSION"; then
    echo "❌ 语言包打包失败"
    return 1
fi

echo ""
echo "=================================================="
echo "✅ 语言包打包成功！"
echo "=================================================="
echo "产物目录: $BIN_DIR/font_v3/$UI_SEL/$LANG_SEL"
ls -lh "$BIN_DIR/font_v3/$UI_SEL/$LANG_SEL"/*_LFS.bin 2>/dev/null || true
echo ""
echo "镜像烧录: *_LFS.bin -> 外部 Flash 0xE00000 (1.5MB)"
}

if [ "$PACK_SH_SOURCED" -eq 1 ]; then
    (pack_main "$@")
    return $?
else
    pack_main "$@"
    exit $?
fi
