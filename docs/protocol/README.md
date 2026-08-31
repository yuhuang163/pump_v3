# 协议实现索引

本目录收录工程内所有协议的实现说明。协议代码位于 `overlay/protocol/`（编解码）与 `overlay/servers/`、`overlay/ble/`（服务端/链路接入），OTA 协议核心在 `trunk/platform_chip/bes/sdk/services/ota/`。

## 协议总览

| 协议 | SOF | 承载通道 | 帧结构 | 用途 |
|------|-----|----------|--------|------|
| [旧版 App 协议](app-old-protocol.md) | `0xAA55` | BLE Master 服务 | 扁平命令帧 + CRC8 | 生产主通道，存量 App 兼容 |
| [新版 comm 协议](comm-protocol.md) | `0x5CC5` | BLE Factory / Remote 服务 | 多 Service + 多 TLV + CRC16 | 产测 / OTA 通道 |
| [数据埋点协议](data-bury-protocol.md) | `0x5A` | BLE Data Bury 服务 | Link 层 + 应用层 TLV + CRC16-CCITT | 设备数据透传上报 |
| [OTA 协议](ota-protocol.md) | — | Remote 服务（comm 协议承载） | 命令/应答 + 断点续传 + CRC32 | 固件/资源/触摸键升级 |
| [产测 Shell](factory-shell.md) | 文本 `#cmd` | BLE Factory 服务 | 文本命令 | 产线调试与烧录 |

## 代码位置

```text
overlay/protocol/
├── app_old_commication_protocol/   旧版 App 协议编解码
├── common_protocl/                 新版 comm 协议编解码
└── app_data_bury_protocol/         数据埋点协议编解码

overlay/servers/
├── app_old_protocol_server/        旧协议命令处理总入口（CID 分发）
├── common_server/                  新协议子服务（algo/sensors/pump/sys/ota/tests...）
├── factory_server_client/          产测 server（订阅 Factory channel）
├── ota_server_client/              OTA server（订阅 Remote channel，带组包）
└── data_bury_protocol_server_client/ 埋点协议 server

overlay/ble/
├── master_service/                 Master GATT 服务 → 旧协议 channel
├── factory_service/                Factory GATT 服务 → 产测 channel（含 #shell）
├── remot_service/                  Remote GATT 服务 → OTA channel
└── data_bury_service/              Data Bury GATT 服务 → 埋点 channel
```

## 三套协议的层次对比

```text
旧版 App 协议（扁平）:
  SOF(0xAA55) | CT(1) | CID(1) | CAL(1) | CAB(payload) | CRC8(1)

新版 comm 协议（层次化）:
  SOF(0x5CC5) | Seq(1) | FrameType(1) | Length(2) | [Service]... | CRC16(2)
    Service: SvrID(2) | Len(2) | [TLV]...
      TLV: Type(2) | Len(2) | Value

数据埋点协议（Link + App 两层）:
  Link: SOF(0x5A) | Len(2) | Ctrl(1) | [FSN(1)] | payload | CRC16(2)
    App:  Svc(1) | Cmd(1) | [TLV]...
      TLV: Type(1) | Len(1) | Value
```

## 关键约定

- 三套协议由 `trunk/root_source/protocol/` 的 `protocol_manager` 统一暴露函数指针，业务层不关心底层差异。
- 所有协议下行入口都经 BLE GATT 服务 → `ble_service_data_recv` → channel 分发 → 对应 server client 组包解析。
- 上行动作经 `servers_publish` 统一发布，由各 server 走对应协议组帧发送。
- 二分查找的订阅/发布/消息表均要求按 id 升序排列（见各实现注释），新增条目必须插序。
