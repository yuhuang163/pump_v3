#!/bin/bash
# V2 PNG 转 BAJ 便捷脚本

cd "$(dirname "${BASH_SOURCE[0]}")"

# 使用 Docker 运行转换
./docker_run.sh ./png2baj_fixed.sh /workspace/V2/png /workspace/V2/baj
