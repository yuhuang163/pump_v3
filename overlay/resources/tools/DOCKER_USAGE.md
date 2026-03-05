# Docker 容器使用指南

## 📦 快速开始

### 1. 安装 Docker

```bash
cd overlay/resources/tools
./install_docker.sh
```

安装完成后，**需要重新登录**或执行：
```bash
newgrp docker
```

### 2. 验证安装

```bash
docker --version
docker run hello-world
```

### 3. 使用转换工具

```bash
cd overlay/resources/tools

# PNG 转 BAJ (固定参数，最简单)
./docker_run.sh ./png2baj_fixed.sh /workspace/your_images/

# PNG 转 BAJ (可配置参数)
./docker_run.sh ./png2baj_resize.sh a8 1 99 422 /workspace/your_images/ 352

# PNG 转 TXT
./docker_run.sh ./png_to_txt.sh /workspace/your_images/ -o /workspace/output/

# 进入容器交互模式
./docker_run.sh bash
```

---

## 🔧 详细说明

### Docker 镜像

- **基础镜像**: Ubuntu 22.04 (包含 GLIBC 2.35)
- **已安装工具**: ImageMagick, Python3
- **工作目录**: `/workspace` (自动挂载当前目录)

### 容器特点

- ✅ 包含 GLIBC 2.35，支持 png2baj 和 jpgconv
- ✅ 自动挂载当前目录到容器
- ✅ 执行完命令后自动清理容器
- ✅ 支持所有转换脚本

---

## 📝 使用示例

### 示例1: 批量转换 PNG 到 BAJ

```bash
cd overlay/resources/tools

# 准备图片目录
mkdir -p test_images
cp /path/to/your/*.png test_images/

# 使用固定参数转换
./docker_run.sh ./png2baj_fixed.sh /workspace/test_images/

# 结果在 out_test_images/ 目录
ls -lh out_test_images/*.baj
```

### 示例2: 高质量转换（a8 + YUV444）

```bash
./docker_run.sh ./png2baj_resize.sh a8 1 99 444 /workspace/test_images/ 336
```

### 示例3: PNG 直接转 TXT

```bash
./docker_run.sh ./png_to_txt.sh /workspace/test_images/ -o /workspace/txt_output/
```

### 示例4: 进入容器调试

```bash
# 进入容器
./docker_run.sh bash

# 在容器内执行命令
cd /workspace
./png2baj_fixed.sh test_images/
exit
```

---

## 🛠️ 故障排除

### 问题1: "Docker 未运行"

```bash
sudo systemctl start docker
sudo systemctl enable docker
```

### 问题2: "权限被拒绝"

```bash
# 将用户添加到 docker 组
sudo usermod -aG docker $USER
newgrp docker
```

### 问题3: "镜像构建失败"

```bash
# 手动构建镜像
cd overlay/resources/tools
docker build -t image-converter .
```

### 问题4: "命令找不到"

确保在 `overlay/resources/tools` 目录下运行脚本，或者使用绝对路径。

---

## 💡 提示

1. **首次使用**: 会自动构建 Docker 镜像（需要几分钟）
2. **后续使用**: 直接运行，无需重新构建
3. **路径说明**: 容器内路径使用 `/workspace/`，对应宿主机的 `tools/` 目录
4. **性能**: Docker 容器性能损失很小（<5%）

---

## 📚 相关文档

- [转换脚本说明文档.md](./转换脚本说明文档.md)
- [UPGRADE_GLIBC_GUIDE.md](./UPGRADE_GLIBC_GUIDE.md)

---

**最后更新**: 2025-01-16
