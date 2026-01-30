# MD5 Firmware Verifier (GUI)

本目录提供一个 **Windows 环境**可用的固件文件名校验工具，用于验证文件名后缀中的 8 个 hex 校验值是否与文件内容匹配。

## 1. 文件名规则（Expected 值如何提取）

工具只对以下格式的 `.bin` 文件进行“期望值提取”：

- `..._<8hex>_APP.bin`
- `..._<8hex>_BOOT.bin`
- `..._<8hex>_OTA.bin`
- `..._<8hex>_RES.bin`

其中 `<8hex>` 为 **8 个十六进制字符**（4 字节）。

如果文件名不符合上述后缀格式，工具会将该文件标记为 **FAIL**，原因显示为：

- `No 8-hex suffix before _APP/_BOOT/_OTA/_RES`

## 2. MD5 规则与“折叠 4 字节”方式（Actual 值如何计算）

### 2.1 先计算完整 MD5（16 字节）

对文件内容计算 MD5，得到 16 字节数组：

- `md5[0..15]`

### 2.2 将整个 MD5 折叠为 4 字节（XOR folding）

使用 **按字节异或**折叠为 4 字节：

```
out[0] = md5[0]  XOR md5[4]  XOR md5[8]  XOR md5[12]
out[1] = md5[1]  XOR md5[5]  XOR md5[9]  XOR md5[13]
out[2] = md5[2]  XOR md5[6]  XOR md5[10] XOR md5[14]
out[3] = md5[3]  XOR md5[7]  XOR md5[11] XOR md5[15]
```

然后把 `out[0..3]` 按顺序输出为 **8 个小写 hex**（每字节 2 个 hex），得到 `actual`。

### 2.3 判定逻辑

- 从文件名提取到的 `<8hex>` 记为 `expected`
- 由文件内容计算得到的折叠值记为 `actual`
- `expected == actual` → **PASS**
- 否则 → **FAIL**（Reason: `Mismatch`）

## 3. 如何使用（工厂现场推荐）

### 3.1 GUI 启动（无命令行窗口）

双击运行：

- `verify_md5_suffix_gui.bat`

在弹出的窗口中：

1. 点击 **Select folder...** 选择待验证目录
2. 点击 **Verify**
3. 表格中查看每个文件的 **PASS/FAIL**、Expected/Actual、Reason

## 4. 日志与排障

工具会在当前目录输出日志（用于排查“闪退/权限/环境问题”等）：

- `verify_md5_suffix_launcher.log`：启动器（bat/vbs）日志，包含 PowerShell 启动与输出重定向信息
- `verify_md5_suffix.log`：脚本运行日志（GUI 启动、校验开始/结束、异常堆栈等）

## 5. 文件说明

- `verify_md5_suffix.ps1`：主校验脚本（GUI + 计算逻辑）
- `verify_md5_suffix_gui.bat`：工厂双击入口（隐藏启动 PowerShell，避免黑框）
- `verify_md5_suffix_gui.vbs`：辅助隐藏启动并收集输出到 launcher log

