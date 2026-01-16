#!/bin/bash
### BAJ文件转TXT数据文件 - Shell包装脚本
### 用法: ./baj_to_txt.sh <输入路径> [选项]

### 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PYTHON_SCRIPT="$SCRIPT_DIR/baj_to_txt.py"

### 检查Python脚本是否存在
if [ ! -f "$PYTHON_SCRIPT" ]; then
    echo "错误: 找不到 baj_to_txt.py 脚本"
    echo "路径: $PYTHON_SCRIPT"
    exit 1
fi

### 检查Python是否安装
if ! command -v python3 &> /dev/null; then
    echo "错误: 未找到 python3，请先安装 Python 3"
    exit 1
fi

### 显示帮助信息
show_help() {
    echo "================================================"
    echo "BAJ文件转TXT数据文件工具"
    echo "================================================"
    echo ""
    echo "用法:"
    echo "  $0 <输入路径> [选项]"
    echo ""
    echo "参数:"
    echo "  输入路径        BAJ文件路径或包含BAJ文件的目录"
    echo ""
    echo "选项:"
    echo "  -o <目录>       指定输出目录（默认：BAJ文件所在目录）"
    echo "  -n <文件名>     指定输出文件名（仅单文件模式有效）"
    echo "  -r              递归处理子目录（仅目录模式有效）"
    echo "  -h, --help      显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  ### 转换单个文件"
    echo "  $0 input.baj"
    echo "  $0 input.baj -o output_dir"
    echo "  $0 input.baj -o output_dir -n output.txt"
    echo ""
    echo "  ### 批量转换目录下所有BAJ文件"
    echo "  $0 out_img_a4_yuv420/"
    echo "  $0 out_img_a4_yuv420/ -o txt_output/"
    echo "  $0 out_img_a4_yuv420/ -o txt_output/ -r"
    echo ""
    echo "输出格式:"
    echo "  - 十六进制格式，每行16字节"
    echo "  - C数组格式，每个字节用逗号分隔"
    echo "  - 适合嵌入式系统直接使用"
    echo "================================================"
}

### 如果没有参数或参数是 -h/--help，显示帮助
if [ $# -eq 0 ] || [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    show_help
    exit 0
fi

### 执行Python脚本，传递所有参数
echo "================================================"
echo "开始转换 BAJ -> TXT"
echo "================================================"
echo ""

python3 "$PYTHON_SCRIPT" "$@"
exit_code=$?

echo ""
if [ $exit_code -eq 0 ]; then
    echo "================================================"
    echo "✓ 转换完成"
    echo "================================================"
else
    echo "================================================"
    echo "✗ 转换失败（错误码: $exit_code）"
    echo "================================================"
fi


