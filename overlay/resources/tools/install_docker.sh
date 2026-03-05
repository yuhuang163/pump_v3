#!/bin/bash
# Docker 安装脚本

echo "================================================"
echo "Docker 安装脚本"
echo "================================================"
echo ""

# 检查是否已安装
if command -v docker >/dev/null 2>&1; then
    echo "✅ Docker 已安装: $(docker --version)"
    exit 0
fi

echo "开始安装 Docker..."
echo ""

# 更新包列表
echo "步骤 1/5: 更新包列表..."
sudo apt update

# 安装必要的依赖
echo "步骤 2/5: 安装依赖包..."
sudo apt install -y \
    ca-certificates \
    curl \
    gnupg \
    lsb-release

# 添加 Docker 官方 GPG 密钥
echo "步骤 3/5: 添加 Docker GPG 密钥..."
sudo mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg

# 添加 Docker 仓库
echo "步骤 4/5: 添加 Docker 仓库..."
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# 安装 Docker
echo "步骤 5/5: 安装 Docker..."
sudo apt update
sudo apt install -y docker-ce docker-ce-cli containerd.io docker-compose-plugin

# 启动 Docker 服务
echo "启动 Docker 服务..."
sudo systemctl start docker
sudo systemctl enable docker

# 将当前用户添加到 docker 组
echo "将用户添加到 docker 组..."
sudo usermod -aG docker $USER

echo ""
echo "================================================"
echo "✅ Docker 安装完成！"
echo "================================================"
echo ""
echo "⚠️  重要：需要重新登录或执行以下命令使组生效："
echo "   newgrp docker"
echo ""
echo "验证安装："
echo "   docker --version"
echo "   docker run hello-world"
echo ""
