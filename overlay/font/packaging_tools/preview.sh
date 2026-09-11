#!/bin/bash
###
### 语言包 LFS / PACK 预览
### 使用方法: ./preview.sh [选项] [镜像.bin]
###

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FONT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN_DIR="$FONT_DIR/bin"
PREVIEW_PY="$SCRIPT_DIR/preview_lang_lfs.py"
PYTHON="${PYTHON:-python3}"

show_usage() {
    echo "使用方法:"
    echo "  ./preview.sh [--install] [--open] [--ui v3|v3pro] [--json] [-o HTML] [镜像.bin]"
    echo ""
    echo "选项:"
    echo "  --install              只安装 Python 依赖（littlefs-python），然后退出"
    echo "  --open                 生成 HTML 后用浏览器打开"
    echo "      --ui UI            UI: v3|v3pro，默认 v3pro"
    echo "      --json             打印 JSON，不写 HTML"
    echo "  -o, --output PATH      HTML 输出路径"
    echo "      --no-html          只打印 key 列表"
    echo "  -h, --help             显示帮助"
    echo ""
    echo "不传镜像时，使用 overlay/font/bin/font_v3/ 下最新的 *_LFS.bin。"
    echo ""
    echo "示例:"
    echo "  ./preview.sh --install"
    echo "  ./preview.sh --open"
    echo "  ./preview.sh --open ../bin/font_v3/zh/V3PRO_*_LFS.bin"
}

need_python() {
    if ! command -v "$PYTHON" >/dev/null 2>&1; then
        echo "错误: 找不到 $PYTHON" >&2
        exit 1
    fi
}

install_deps() {
    need_python
    echo "安装 littlefs-python ..."
    "$PYTHON" -m pip install --user littlefs-python
    "$PYTHON" - <<'PY'
from littlefs import LittleFS
print("littlefs-python 可用")
PY
}

ensure_deps() {
    need_python
    if "$PYTHON" -c "from littlefs import LittleFS" >/dev/null 2>&1; then
        return 0
    fi
    echo "未检测到 littlefs-python，开始安装 ..."
    install_deps
}

latest_lfs() {
    local found=""
    if [ -d "$BIN_DIR" ]; then
        found="$(find "$BIN_DIR" -type f -name '*_LFS.bin' -printf '%T@ %p\n' 2>/dev/null | sort -nr | head -n1 | cut -d' ' -f2-)"
    fi
    printf "%s" "$found"
}

DO_INSTALL=0
OPEN=0
IMAGE=""
FORWARD=()

while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help)
            show_usage
            exit 0
            ;;
        --install)
            DO_INSTALL=1
            shift
            ;;
        --open)
            OPEN=1
            shift
            ;;
        --json|--no-html)
            FORWARD+=("$1")
            shift
            ;;
        --ui|-o|--output)
            if [ $# -lt 2 ]; then
                echo "错误: $1 需要参数" >&2
                exit 1
            fi
            FORWARD+=("$1" "$2")
            shift 2
            ;;
        --)
            shift
            FORWARD+=("$@")
            break
            ;;
        -*)
            echo "未知选项: $1" >&2
            show_usage >&2
            exit 1
            ;;
        *)
            if [ -n "$IMAGE" ]; then
                echo "错误: 只能指定一个镜像文件" >&2
                exit 1
            fi
            IMAGE="$1"
            shift
            ;;
    esac
done

if [ "$DO_INSTALL" -eq 1 ]; then
    install_deps
    if [ -z "$IMAGE" ] && [ "$OPEN" -eq 0 ] && [ "${#FORWARD[@]}" -eq 0 ]; then
        exit 0
    fi
else
    ensure_deps
fi

if [ -z "$IMAGE" ]; then
    IMAGE="$(latest_lfs)"
    if [ -z "$IMAGE" ]; then
        echo "错误: 未指定镜像，且 $BIN_DIR 下没有 *_LFS.bin" >&2
        show_usage >&2
        exit 1
    fi
    echo "使用最新镜像: $IMAGE"
fi

if [ ! -f "$IMAGE" ]; then
    echo "错误: 文件不存在: $IMAGE" >&2
    exit 1
fi

cd "$SCRIPT_DIR"
CMD=("$PYTHON" "$PREVIEW_PY" "$IMAGE")
if [ "$OPEN" -eq 1 ]; then
    CMD+=(--open)
fi
if [ "${#FORWARD[@]}" -gt 0 ]; then
    CMD+=("${FORWARD[@]}")
fi
exec "${CMD[@]}"
