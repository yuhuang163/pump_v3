# RAM使用情况分析工具 - 使用说明

## 使用方法

### 1. 基本使用（显示所有统计）

显示完整的RAM使用情况，包括：
- RAM内存区域总览（RAM, RAMX, FRAMX等）
- 按模块统计
- 前50个大文件详情
- 按模块分组的文件详情

```bash
python3 scripts/ram_usage_analyzer.py project/bin/firmware/ble_only_lt.map
```

---

### 2. 只看指定模块的详细文件

当你发现某个模块占用很大，想知道具体是哪些文件时使用：

```bash
python3 scripts/ram_usage_analyzer.py project/bin/firmware/ble_only_lt.map --module=platform/main
```

**常用模块名示例：**
- `platform/main`
- `thirdparty/lvgl`
- `rtos/freertos`
- `services/bt`
- `root_source/ui`

---

### 3. 显示更多文件（调整显示数量）

默认显示前50个大文件，如果想看更多：

```bash
python3 scripts/ram_usage_analyzer.py project/bin/firmware/ble_only_lt.map --top=100
```

或者只看前20个：

```bash
python3 scripts/ram_usage_analyzer.py project/bin/firmware/ble_only_lt.map --top=20
```

---

### 4. 组合使用

同时指定模块和显示数量：

```bash
python3 scripts/ram_usage_analyzer.py project/bin/firmware/ble_only_lt.map --module=rtos/freertos --top=20
```

这样可以只看 `rtos/freertos` 模块的前20个大文件。

---

## 输出说明

脚本会输出以下信息：

1. **RAM内存区域总览** - 各RAM区域的使用情况和百分比（RAM, RAMX, FRAMX等）
2. **RAM模块使用情况统计** - 按模块汇总，从大到小排序
3. **RAM段分类统计** - 按段名分类（.data, .bss, .ram_text等）
4. **RAM文件详细使用情况** - 具体到每个.o文件的使用量
5. **按模块分组的文件详情** - 每个模块内部的文件排名

---

## RAM段说明

脚本会分析以下RAM相关的段：

- `.data` - 已初始化的全局变量和静态变量
- `.bss` - 未初始化的全局变量和静态变量
- `.ram_text` - 放在RAM中执行的代码
- `.sram` - SRAM相关段
- `.ramx` - RAMX相关段

---

## 快速定位内存占用

**步骤1：** 先运行基本命令，找出占用最大的模块
```bash
python3 scripts/ram_usage_analyzer.py project/bin/firmware/ble_only_lt.map
```

**步骤2：** 针对大模块查看详细文件
```bash
python3 scripts/ram_usage_analyzer.py project/bin/firmware/ble_only_lt.map --module=rtos/freertos
```

**步骤3：** 根据输出的文件列表，优化对应的源文件

---

## 与PSRAM分析工具的区别

| 工具 | 分析对象 | 用途 |
|------|---------|------|
| `ram_usage_analyzer.py` | RAM/SRAM | 分析内部RAM使用情况 |
| `psram_usage_analyzer.py` | PSRAM | 分析外部PSRAM使用情况 |

两个工具使用方法相同，只是分析的内存区域不同。

---

## 注意事项

- 脚本路径：`scripts/ram_usage_analyzer.py`
- map文件路径：`project/bin/firmware/ble_only_lt.map`
- 需要Python 3.6+
- 模块名区分大小写
- RAM通常比PSRAM紧张，建议重点关注
