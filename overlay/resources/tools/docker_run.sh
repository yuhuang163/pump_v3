#!/bin/bash
# Docker 容器运行脚本 - 用于运行图片转换工具

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# 挂载整个 resources 目录，以便访问 V2 等子目录
RESOURCES_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
IMAGE_NAME="image-converter"
CONTAINER_NAME="image-converter-$(date +%s)"

# 检查 Docker 是否运行
if ! docker info >/dev/null 2>&1; then
    echo "❌ 错误: Docker 未运行"
    echo "   请先启动 Docker: sudo systemctl start docker"
    exit 1
fi

# 检查镜像是否存在，不存在则构建
if ! docker images | grep -q "$IMAGE_NAME"; then
    echo "构建 Docker 镜像..."
    cd "$SCRIPT_DIR"
    docker build -t "$IMAGE_NAME" .
    if [ $? -ne 0 ]; then
        echo "❌ 镜像构建失败"
        exit 1
    fi
    echo "✅ 镜像构建完成"
fi

# 显示使用说明
if [ $# -eq 0 ]; then
    echo "================================================"
    echo "Docker 容器图片转换工具"
    echo "================================================"
    echo ""
    echo "用法:"
    echo "  $0 <命令> [参数...]"
    echo ""
    echo "示例:"
    echo "  # PNG 转 BAJ (固定参数)"
    echo "  $0 ./png2baj_fixed.sh /workspace/your_images/"
    echo ""
    echo "  # PNG 转 BAJ (可配置)"
    echo "  $0 ./png2baj_resize.sh a8 1 99 422 /workspace/your_images/ 352"
    echo ""
    echo "  # PNG 转 TXT"
    echo "  $0 ./png_to_txt.sh /workspace/your_images/ -o /workspace/output/"
    echo ""
    echo "  # 进入容器交互模式"
    echo "  $0 bash"
    echo ""
    echo "注意:"
    echo "  - 当前目录会自动挂载到容器的 /workspace"
    echo "  - 所有路径都相对于当前目录"
    exit 0
fi

# 运行容器
echo "启动容器并执行命令..."
echo "挂载目录: $RESOURCES_DIR -> /workspace"
docker run --rm -it \
    --name "$CONTAINER_NAME" \
    -v "$RESOURCES_DIR:/workspace" \
    -w /workspace/tools \
    "$IMAGE_NAME" \
    "$@"

exit_code=$?

if [ $exit_code -eq 0 ]; then
    echo ""
    echo "✅ 命令执行成功"
else
    echo ""
    echo "❌ 命令执行失败 (退出码: $exit_code)"
fi

exit $exit_code
