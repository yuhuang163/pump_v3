# 资源管理目录

## 📁 目录结构

```
overlay/resources/
├── README.md                  # 本文档（目录说明）
│
├── V1/                        # 版本1资源 ⭐
│   ├── resource_config.json   # V1资源配置文件
│   ├── resource_id_define.h   # V1资源ID定义头文件
│   ├── baj/                   # BAJ格式资源文件
│   ├── png/                   # PNG格式资源文件
│   └── baj2txt/               # BAJ转TXT输出目录
│
├── V2/                        # 版本2资源 ⭐
│   ├── resource_config.json   # V2资源配置文件
│   ├── resource_id_define.h   # V2资源ID定义头文件
│   ├── animation/             # 动画资源文件
│   ├── home_baj/              # 首页BAJ格式资源
│   └── home_png/              # 首页PNG格式资源
│
├── bin/                       # 生成的资源包目录
│   └── *.bin                  # 打包后的资源包文件
│
├── packaging_tools/           # 资源打包工具 🔧
│   ├── pack.sh                # 快速打包脚本 ⭐
│   └── pack_resources.py      # 核心打包脚本
│
├── tools/                     # 资源转换工具 🔧
│   ├── png2baj.sh             # PNG转BAJ脚本
│   ├── png_to_txt.py          # PNG转TXT脚本
│   ├── baj_to_txt.py          # BAJ转TXT脚本
│   └── ...                    # 其他转换工具
│
└── reference/                 # 参考文件
    ├── comparison_png/        # PNG对比参考
    └── comparison_png2txt/    # PNG转TXT对比参考
```

## 🚀 快速开始

### 1. 打包资源（按版本）

#### 打包 V1 资源
```bash
cd packaging_tools
./pack.sh ../V1/resource_config.json ../bin/V1_resource_pack.bin ../V1/resource_id_define.h
```

#### 打包 V2 资源
```bash
cd packaging_tools
./pack.sh ../V2/resource_config.json ../bin/V2_resource_pack.bin ../V2/resource_id_define.h
```

### 2. 修改资源配置

编辑对应版本的配置文件：
```bash
# 编辑 V1 配置
vim V1/resource_config.json

# 编辑 V2 配置
vim V2/resource_config.json
```

### 3. 查看打包结果

```bash
ls -lh bin/
```

## 📝 配置文件说明

每个版本都有独立的配置文件：
- **V1/resource_config.json** - 版本1资源配置
- **V2/resource_config.json** - 版本2资源配置

配置文件定义了资源ID到实际图片文件的映射关系。

**示例**：
```json
{
  "meta": {
    "version": "1.0",
    "description": "资源包配置文件 - 映射资源ID到实际图片文件",
    "resource_type": "IMG_RES_TYPE_UI",
    "language": "IMG_RES_LANG_TYPE_ZH",
    "compressed": false
  },
  "resources": [
    {
      "resource_id": "RESOURCE_ID_LOGO",
      "description": "品牌logo",
      "file": "baj/logo_360x360.baj",
      "x_offset": 0,
      "y_offset": 0
    }
  ]
}
```

## 🔧 工具使用

### 打包工具

所有打包工具都在 `packaging_tools/` 目录下：

#### 快速打包（指定版本）
```bash
cd packaging_tools

# 打包 V1
./pack.sh ../V1/resource_config.json ../bin/V1_resource_pack.bin ../V1/resource_id_define.h

# 打包 V2
./pack.sh ../V2/resource_config.json ../bin/V2_resource_pack.bin ../V2/resource_id_define.h
```

#### 使用Python脚本直接打包
```bash
cd packaging_tools
python3 pack_resources.py ../V1/resource_config.json ../bin/V1_resource_pack.bin ../V1/resource_id_define.h
```

### 转换工具

所有转换工具都在 `tools/` 目录下：

#### PNG转BAJ
```bash
cd tools
./png2baj.sh input.png output.baj
```

#### PNG转TXT
```bash
cd tools
python3 png_to_txt.py input.png output.txt
```

#### BAJ转TXT
```bash
cd tools
python3 baj_to_txt.py input.baj output.txt
```

更多工具使用方法请查看 `tools/数据转换工具README.md`

## 📚 文档索引

| 文档 | 说明 | 位置 |
|------|------|------|
| **README.md** | 目录说明（本文档） | `./README.md` |
| **tools/数据转换工具README.md** | 转换工具说明 | `tools/数据转换工具README.md` |
| **tools/转换脚本说明文档.md** | 转换脚本详细说明 | `tools/转换脚本说明文档.md` |
| **tools/使用示例.md** | 工具使用示例 | `tools/使用示例.md` |
| **tools/VGLite-16像素对齐技术文档.md** | VGLite技术文档 | `tools/VGLite-16像素对齐技术文档.md` |

## 💡 工作流程

### V1 资源工作流程
```
1. 准备资源文件（放入 V1/baj/ 或 V1/png/）
   ↓
2. 编辑 V1/resource_config.json
   ↓
3. 运行 packaging_tools/pack.sh 指定V1配置
   ↓
4. 生成 bin/V1_resource_pack.bin
   ↓
5. 集成到固件
```

### V2 资源工作流程
```
1. 准备资源文件（放入 V2/animation/、V2/home_baj/ 或 V2/home_png/）
   ↓
2. 编辑 V2/resource_config.json
   ↓
3. 运行 packaging_tools/pack.sh 指定V2配置
   ↓
4. 生成 bin/V2_resource_pack.bin
   ↓
5. 集成到固件
```

## 🎯 资源文件要求

### 文件命名规范

建议使用包含尺寸信息的命名：
```
name_WIDTHxHEIGHT_xxx.baj
```

**示例**：
- ✅ `logo_360x360_a4_q99_422.baj`
- ✅ `mode1_270x270.baj`
- ⚠️ `image001.baj` (无法自动解析尺寸)

### 文件存放位置

#### V1 版本
- **BAJ格式**: `V1/baj/`
- **PNG格式**: `V1/png/`
- **BAJ转TXT输出**: `V1/baj2txt/`

#### V2 版本
- **动画资源**: `V2/animation/`
- **首页BAJ资源**: `V2/home_baj/`
- **首页PNG资源**: `V2/home_png/`

## 🔍 常见任务

### 添加新资源（V1）

1. 将资源文件放入 `V1/baj/` 或 `V1/png/`
2. 编辑 `V1/resource_config.json`，添加新条目
3. 运行打包脚本：
   ```bash
   cd packaging_tools
   ./pack.sh ../V1/resource_config.json ../bin/V1_resource_pack.bin ../V1/resource_id_define.h
   ```

### 添加新资源（V2）

1. 将资源文件放入对应目录：
   - 动画资源：`V2/animation/`
   - 首页BAJ资源：`V2/home_baj/`
   - 首页PNG资源：`V2/home_png/`
2. 编辑 `V2/resource_config.json`，添加新条目
3. 运行打包脚本：
   ```bash
   cd packaging_tools
   ./pack.sh ../V2/resource_config.json ../bin/V2_resource_pack.bin ../V2/resource_id_define.h
   ```

### 修改现有资源

1. 替换资源文件（保持文件名不变）
2. 运行对应版本的打包脚本重新打包

### 删除资源

1. 从对应版本的 `resource_config.json` 删除对应条目
2. （可选）删除资源文件
3. 运行对应版本的打包脚本重新打包

### 查看资源列表

```bash
# 方式1：查看配置文件
cat V1/resource_config.json
cat V2/resource_config.json

# 方式2：查看资源目录
ls -lh V1/baj/
ls -lh V1/png/
ls -lh V2/animation/
ls -lh V2/home_baj/
ls -lh V2/home_png/
```

## ⚙️ 集成到构建系统

在项目的 Makefile 中添加：

```makefile
# V1 资源打包目标
V1_RESOURCE_PACK := overlay/resources/bin/V1_resource_pack.bin
$(V1_RESOURCE_PACK): overlay/resources/V1/resource_config.json
	@echo "打包 V1 资源..."
	@cd overlay/resources/packaging_tools && \
		./pack.sh ../V1/resource_config.json ../bin/V1_resource_pack.bin ../V1/resource_id_define.h
	@echo "V1 资源打包完成"

# V2 资源打包目标
V2_RESOURCE_PACK := overlay/resources/bin/V2_resource_pack.bin
$(V2_RESOURCE_PACK): overlay/resources/V2/resource_config.json
	@echo "打包 V2 资源..."
	@cd overlay/resources/packaging_tools && \
		./pack.sh ../V2/resource_config.json ../bin/V2_resource_pack.bin ../V2/resource_id_define.h
	@echo "V2 资源打包完成"

# 清理资源包
clean_resources:
	rm -f overlay/resources/bin/*.bin

.PHONY: $(V1_RESOURCE_PACK) $(V2_RESOURCE_PACK) clean_resources
```

## 📊 技术规格

- **资源包格式**: 二进制
- **头部大小**: 固定32KB
- **最大资源数**: 约1636个
- **支持的ID格式**: 宏名称或数字
- **校验方式**: CRC16 + MD5
- **版本**: v2.0

## 🐛 故障排查

### 打包失败

1. 检查配置文件格式是否正确
2. 确认所有资源文件存在
3. 检查资源文件路径是否正确（相对于配置文件所在目录）
4. 查看错误信息

### 文件找不到

- 确保路径相对于配置文件所在目录（V1/ 或 V2/）
- 检查文件名大小写
- 使用 `ls` 命令确认文件存在

### 版本混淆

- 确保使用对应版本的配置文件
- 检查 `resource_id_define.h` 是否与配置文件版本匹配
- 确认资源文件放在正确的版本目录下

### 更多帮助

查看打包脚本帮助：
```bash
cd packaging_tools
./pack.sh --help
```

查看转换工具文档：
```bash
cat tools/数据转换工具README.md
cat tools/转换脚本说明文档.md
```

## 📞 获取帮助

- 查看 `tools/数据转换工具README.md` - 转换工具说明
- 查看 `tools/转换脚本说明文档.md` - 转换脚本详细说明
- 查看 `tools/使用示例.md` - 工具使用示例
- 查看 `tools/VGLite-16像素对齐技术文档.md` - VGLite技术文档

---

**版本**: v2.0  
**最后更新**: 2026-01-07  
**维护**: 资源打包工具团队
