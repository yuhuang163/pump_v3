# 数据转换工具使用说明

本文档介绍 BAJ/PNG 图片格式转换为 TXT 数据文件的工具使用方法。

## 📦 工具列表

| 工具 | 功能 | 输入 | 输出 |
|-----|------|-----|------|
| `baj_to_txt.sh` | BAJ转TXT数据文件 | .baj | .txt |
| `png_to_txt.sh` | PNG转TXT数据文件 | .png | .txt |

## 🚀 快速开始

### BAJ 转 TXT

```bash
### 转换单个文件
./baj_to_txt.sh image.baj

### 批量转换目录
./baj_to_txt.sh out_images/ -o txt_data/

### 递归转换子目录
./baj_to_txt.sh out_images/ -o txt_data/ -r
```

### PNG 转 TXT

```bash
### 转换单个文件
./png_to_txt.sh image.png

### 批量转换目录
./png_to_txt.sh images/ -o txt_data/

### 递归转换子目录
./png_to_txt.sh images/ -o txt_data/ -r
```

## 📝 输出格式

转换后的 TXT 文件是 C 数组格式的十六进制数据：

```c
/// image_360x360.txt 内容示例
0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
0x00,0x00,0x01,0x68,0x00,0x00,0x01,0x68,0x08,0x06,0x00,0x00,0x00,0xc8,0x52,0x7c,
0x64,0x00,0x00,0x00,0x04,0x67,0x41,0x4d,0x41,0x00,0x00,0xb1,0x8f,0x0b,0xfc,0x61,
...
```

## 💡 使用场景

### 场景1：BAJ格式嵌入固件（推荐）

```bash
### 完整流程
### 1. PNG转BAJ（压缩）
./png2baj_fixed.sh images/

### 2. BAJ转TXT（生成C数组）
./baj_to_txt.sh out_images/ -o txt_data/

### 3. 在C代码中使用
/// image.c
const unsigned char image_data[] = {
    /// 从 TXT 文件复制内容
    0x42,0x41,0x4a,0x00,...
};
```

**优点**：
- ✅ 文件最小（压缩后）
- ✅ 硬件JPEG解码加速
- ✅ 适合大量图片

### 场景2：PNG格式直接嵌入（简单）

```bash
### 直接转换
./png_to_txt.sh images/ -o txt_data/

### 在C代码中使用
/// image.c
const unsigned char png_image_data[] = {
    /// 从 TXT 文件复制内容
    0x89,0x50,0x4e,0x47,...
};
```

**优点**：
- ✅ 转换步骤少
- ✅ 保留PNG原格式
- ✅ 适合小图标

## 🔧 在C代码中使用

### 方法1：直接复制到源文件

```c
/// resources/images.c
#include "images.h"

/// 从 TXT 文件复制数据
const unsigned char home_icon_data[] = {
    0x42,0x41,0x4a,0x00,0x60,0x01,0x60,0x01,0x61,0x04,0x00,0x00,
    0x63,0x0d,0x00,0x00,0x6c,0x11,0x00,0x00,0xff,0xd8,0xff,0xdb,
    /// ... 更多数据
};

const unsigned int home_icon_size = sizeof(home_icon_data);
```

### 方法2：通过链接器包含二进制文件（更高效）

```makefile
### Makefile 示例
### 将 .baj 文件转换为 .o 目标文件
%.o: %.baj
	$(OBJCOPY) -I binary -O elf32-littlearm \
		--rename-section .data=.rodata \
		--redefine-sym _binary_$*_baj_start=$*_data_start \
		--redefine-sym _binary_$*_baj_end=$*_data_end \
		$< $@
```

```c
/// 在C代码中引用
extern const unsigned char home_icon_data_start[];
extern const unsigned char home_icon_data_end[];

void load_home_icon(void) {
    const unsigned char *data = home_icon_data_start;
    unsigned int size = home_icon_data_end - home_icon_data_start;
    
    decode_baj_image(data, size);
}
```

## ⚙️ 命令选项

### 共同选项

- `-o <目录>`: 指定输出目录
- `-r`: 递归处理子目录
- `-h, --help`: 显示帮助信息

### PNG转TXT独有选项

- `-n <文件名>`: 指定输出文件名（单文件模式）
- `--no-resolution`: 不在文件名中包含分辨率

## 📊 文件大小对比

| 格式 | 文件大小 | 说明 |
|-----|---------|------|
| **原始PNG** | 126 KB | 未压缩的PNG图片 |
| **BAJ格式** | 33 KB | YUV420压缩后（节省73.8%） |
| **BAJ的TXT** | ~100 KB | 十六进制文本格式（约3倍BAJ大小） |
| **PNG的TXT** | ~380 KB | 十六进制文本格式（约3倍PNG大小） |

**说明**：
- TXT文件仅用于开发阶段查看和手动嵌入
- 生产环境建议直接使用二进制文件（.baj 或 .png）
- 通过链接器包含可以避免TXT文件膨胀问题

## 📖 详细文档

更多信息请参考：
- [转换脚本说明文档.md](./转换脚本说明文档.md) - 完整工具使用指南
- [BAJ格式转换对比报告.md](./BAJ格式转换对比报告.md) - 格式对比测试报告

## ❓ 常见问题

### Q: TXT文件太大怎么办？

A: TXT文件仅用于查看和手动嵌入。生产环境建议：
1. 直接使用二进制文件（.baj/.png）
2. 通过链接器包含到固件中
3. 参考上面的"方法2"示例

### Q: 如何批量处理所有图片？

A: 使用递归选项：
```bash
### 转换整个图片资源目录
./png2baj_fixed.sh images/
./baj_to_txt.sh out_images/ -o txt_data/ -r
```

### Q: 是否需要Python环境？

A: 是的，这些工具基于Python 3编写。需要：
- Python 3.6 或更高版本
- 无需额外的第三方库

---

**工具版本**: v5.0  
**最后更新**: 2025-12-23

