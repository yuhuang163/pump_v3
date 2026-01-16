# PNG图片转TXT数据文件工具 - 使用说明

## 工具简介

`png_to_txt.py` 是一个将PNG图片文件转换为十六进制格式文本文件的工具，主要用于嵌入式系统。转换后的TXT文件包含PNG图片的二进制数据，格式为C数组（每行16字节，末尾带逗号）。

---

## 使用方法

### 1. 基本使用（输出到PNG文件所在目录）

最简单的用法，转换后的TXT文件会保存在PNG文件所在的目录：

```bash
python3 scripts/png_to_txt.py overlay/resources/img/home_1.png
```

**输出结果：**
- 输出文件：`overlay/resources/img/home_1_360x360.txt`
- 文件名自动包含分辨率信息（360x360）

---

### 2. 指定输出目录

使用 `-o` 或 `--output-dir` 参数指定输出目录：

```bash
python3 scripts/png_to_txt.py overlay/resources/img/home_1.png -o overlay/resources/img_bin
```

**输出结果：**
- 输出文件：`overlay/resources/img_bin/home_1_360x360.txt`

---

### 3. 指定输出文件名

使用 `-n` 或 `--name` 参数指定输出文件名：

```bash
python3 scripts/png_to_txt.py overlay/resources/img/home_1.png -o overlay/resources/img_bin -n home_1_data.txt
```

**输出结果：**
- 输出文件：`overlay/resources/img_bin/home_1_data.txt`

---

### 4. 不在文件名中包含分辨率信息

使用 `--no-resolution` 参数，输出文件名将不包含分辨率信息：

```bash
python3 scripts/png_to_txt.py overlay/resources/img/home_1.png -o overlay/resources/img_bin --no-resolution
```

**输出结果：**
- 输出文件：`overlay/resources/img_bin/home_1.txt`（不包含分辨率）

---

### 5. 一键批量转换（推荐）

直接指定目录路径，自动转换目录下所有PNG文件：

```bash
python3 scripts/png_to_txt.py overlay/resources/img -o overlay/resources/img_bin
```

**输出结果：**
- 自动查找 `img` 目录下的所有 `.png` 文件
- 批量转换并输出到 `img_bin` 目录
- 显示转换进度和统计信息

**示例输出：**
```
找到 4 个PNG文件，开始批量转换...
------------------------------------------------------------
成功: 已转换 47027 字节 (分辨率: 296x296)
输出文件: overlay/resources/img_bin/18_296x296.txt

成功: 已转换 2375 字节 (分辨率: 360x360)
输出文件: overlay/resources/img_bin/charge_1_360x360.txt
...
------------------------------------------------------------
批量转换完成: 成功 4 个, 失败 0 个
```

---

### 6. 递归批量转换（包含子目录）

使用 `-r` 或 `--recursive` 参数，可以递归处理子目录下的所有PNG文件：

```bash
python3 scripts/png_to_txt.py overlay/resources/img -o overlay/resources/img_bin -r
```

**说明：**
- 会递归查找所有子目录中的PNG文件
- 如果指定了输出目录，会保持原有的目录结构

---

## 输出格式说明

转换后的TXT文件格式为十六进制数组，每行16字节，末尾带逗号，例如：

```
0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
0x00,0x00,0x01,0x68,0x00,0x00,0x01,0x68,0x08,0x06,0x00,0x00,0x00,0x28,0xcd,0xde,
...
```

这种格式可以直接用于C/C++代码中作为数组数据。

---

## 参数说明

| 参数 | 简写 | 说明 | 默认值 | 适用模式 |
|------|------|------|--------|---------|
| `input_path` | - | 输入的PNG图片文件路径或目录路径（必需） | - | 全部 |
| `--output-dir` | `-o` | 输出目录 | PNG文件所在目录 | 全部 |
| `--name` | `-n` | 输出文件名（仅单文件模式） | PNG文件名_宽度x高度.txt | 单文件 |
| `--no-resolution` | - | 不在文件名中包含分辨率信息 | 默认包含分辨率 | 全部 |
| `--recursive` | `-r` | 递归处理子目录（仅目录模式） | 不递归 | 目录 |

---

## 输出信息说明

脚本执行成功后会显示：

```
成功: 已转换 34890 字节 (分辨率: 360x360)
输出文件: overlay/resources/img_bin/home_1_360x360.txt
```

- **已转换字节数**：PNG文件的原始大小
- **分辨率**：图片的宽度x高度（从PNG文件头解析）
- **输出文件路径**：生成的TXT文件完整路径

---

## 注意事项

- 脚本路径：`scripts/png_to_txt.py`
- 需要Python 3.6+
- 只支持PNG格式图片文件
- 输出目录不存在时会自动创建
- 如果输出文件已存在，会被覆盖
- 分辨率信息从PNG文件头自动解析，如果解析失败则文件名不包含分辨率

---

## 常见使用场景

### 场景1：一键批量转换目录下所有图片（最常用）

```bash
python3 scripts/png_to_txt.py overlay/resources/img -o overlay/resources/img_bin
```

### 场景2：转换单个图片到指定目录

```bash
python3 scripts/png_to_txt.py overlay/resources/img/home_1.png -o overlay/resources/img_bin
```

### 场景3：转换并自定义文件名

```bash
python3 scripts/png_to_txt.py overlay/resources/img/home_1.png -o overlay/resources/img_bin -n home_1_bin.txt
```

### 场景4：批量转换但不包含分辨率信息

```bash
python3 scripts/png_to_txt.py overlay/resources/img -o overlay/resources/img_bin --no-resolution
```

### 场景5：递归转换所有子目录中的图片

```bash
python3 scripts/png_to_txt.py overlay/resources/img -o overlay/resources/img_bin -r
```

