# BleFactoryTool

PC 侧 BLE 调试工具：Factory 原始收发 + 图片资源 OTA 测试。

对应固件：

- Factory：`drivers/ble/services/factory_service.c`
- Remote / OTA：`drivers/ble/services/remot_service.c` + `overlay/servers/ota_server_client/`
- 协议：`overlay/servers/common_server/ota/IMG_RES_OTA_PROTOCOL.md`

## 功能

1. Bleak 扫描附近 BLE 设备
2. **Factory 收发**：连接并使能 Factory TX Notify → Hex/UTF-8 写 RX、看 Notify
3. **图片资源 OTA**：连接 Remote 服务 → 选资源文件 → START/WRITE/END，看进度与流程日志
4. 「退出并断开连接」回到扫描页
5. Windows 可打包为 `BleFactoryTool.exe`（带调试控制台）

## UUID

### Factory（原始收发）

| 角色 | 默认（128-bit） |
|------|-----------------|
| Service | `524f4f54-9000-0080-0010-000000020001` |
| TX Notify | `524f4f54-9000-0080-0010-000000020002` |
| RX Write | `524f4f54-9000-0080-0010-000000020003` |

### Remote（OTA，双向）

| 角色 | 默认（128-bit） |
|------|-----------------|
| Service | `524f4f54-9000-0080-0010-000000030001` |
| TX Notify（设备→PC） | `524f4f54-9000-0080-0010-000000030002` |
| RX Write（PC→设备） | `524f4f54-9000-0080-0010-000000030003` |

支持 16-bit / 128-bit 写法，见 `factory_uuids.py`。

## 源码运行

需要 Python 3.9+，Linux / Windows 均可开发调试。

```bash
cd tools/ble_factory_tool
python -m venv .venv

# Windows
.venv\Scripts\activate
# Linux
source .venv/bin/activate

pip install -r requirements.txt
python main.py
```

### Linux 权限

```bash
sudo usermod -aG bluetooth $USER
# 或
./install_linux_deps.sh
```

### Windows 前置

- Windows 10 1809+ / Windows 11
- 系统蓝牙已开启且有适配器

## Windows 打包 exe

**必须在 Windows 机器上执行**（不可在 Linux 交叉打包）。

```bat
cd tools\ble_factory_tool
build_windows.bat
```

产物：`dist\BleFactoryTool.exe`。

## 使用步骤

### Factory 收发

1. 「开始扫描」→ 选中设备 →「连接并使能 Factory」
2. 发送区填 Hex / UTF-8 →「发送」；接收区看 Notify
3. 「退出并断开连接」

### 图片资源 OTA

1. 扫描并选中设备 →「OTA 测试」（使能 Remote TX Notify）
2. 「浏览」选择资源文件（≤7MB），确认 size / CRC32
3. 按需改 `img_id`（默认 1）、`version`（默认 `0x00010000`）
4. 「启动 OTA」→ 看进度条与流程日志（START / WRITE / END）
5. 「停止」可中断写块；「退出并断开连接」回主页

OTA 通道：**写 Remote RX，收 Remote TX**（与 Factory 页隔离）。

## 目录

```
tools/ble_factory_tool/
├── DESIGN.md
├── README.md
├── requirements.txt
├── factory_uuids.py
├── comm_protocol.py
├── ota_img_res_client.py
├── ble_worker.py
├── gui_app.py
├── main.py
├── ble_factory_tool.spec
└── build_windows.bat
```

## 设计说明

详见 [DESIGN.md](DESIGN.md)。
