FROM ubuntu:22.04

# 设置非交互模式
ENV DEBIAN_FRONTEND=noninteractive

# 安装必要的工具
RUN apt update && \
    apt install -y \
    imagemagick \
    python3 \
    python3-pip \
    bash \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /workspace

# 保持容器运行
CMD ["/bin/bash"]
