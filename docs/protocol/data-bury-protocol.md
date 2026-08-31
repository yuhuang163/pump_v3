# 数据埋点协议（0x5A）

设备数据透传上报协议，分「链路层」与「应用层」两层。实现位于 `overlay/protocol/app_data_bury_protocol/`，服务端接入在 `overlay/servers/data_bury_protocol_server_client/`。

## 链路层

```text
+--------+-----------+-----------+------------+-----------+-----------+
|  SOF   |  Length   |  Control  |  [FSN]     |  Payload  |  CRC16    |
|  1B    |  2B       |  1B       |  [1B]      |  N B      |  2B       |
+--------+-----------+-----------+------------+-----------+-----------+
```

| 字段 | 大小 | 说明 |
|------|------|------|
| `sof` | 1B | `0x5A` |
| `length` | 2B | 帧长度 |
| `control` | 1B | Bit2=ACK，Bit1-0=FSN 分帧标志 |
| `fsn` | 1B | 分帧序号（仅 FSN != COMPLETE 时存在） |
| `payload` | N B | 应用层数据 |
| `crc16` | 2B | CRC16-CCITT-FALSE |

Control 字段（`app_data_bury_protocol_defs.h`）：

| 宏 | 值 | 说明 |
|----|----|------|
| `CTRL_FSN_COMPLETE` | 0x00 | 单帧完成 |
| `CTRL_FSN_START` | 0x01 | 分帧起始 |
| `CTRL_FSN_MID` | 0x02 | 分帧中间 |
| `CTRL_FSN_END` | 0x03 | 分帧结束 |
| `CTRL_ACK` | 0x04 | ACK 标志 |

常量：`MFS_DEFAULT = 128`，`APP_PAYLOAD_MAX = 247`。

## 应用层

```text
Svc(1B) | Cmd(1B) | [TLV]...
  TLV: Type(1B) | Length(1B) | Value[...]
```

### 服务 ID

| 宏 | 值 | 说明 |
|----|----|------|
| `SVC_DEVICE_MGMT` | 0x01 | 设备管理（下行） |
| `SVC_DATA` | 0x07 | 数据上报（上行） |

### 设备管理服务（0x01）命令

| Cmd | 值 | 说明 |
|-----|----|------|
| `CONN_PARAM_NEGOTIATE` | 0x01 | 连接参数协商 |
| `SET_DEVICE_TIME` | 0x02 | 设置设备时间 |

连接参数协商 TLV Type：`prot_version=0x01`、`max_frame_size=0x02`、`mtu=0x03`、`tx_interval=0x04`、`arq_mode=0x05`、`ack_timeout_ms=0x06`、`max_retransmit=0x07`、`random_value=0x08`。协商结果（如 MFS 默认 128，范围 20~247）存 `s_negotiated_mfs`。

### 数据服务（0x07）命令（EVT_001~019）

`APP_BURY_CMD_*`，从 `0x01` 起编号：

| Cmd | 值 | 事件 |
|-----|----|------|
| PUMP_STARTUP / SHUTDOWN | 0x01/0x02 | 泵启动/关机 |
| BT_CONNECT_EVENT | 0x03 | 蓝牙连接 |
| PUMP_SESSION_START / END | 0x04/0x05 | 泵奶会话 |
| PUMP_MANUAL_ADJUST | 0x06 | 手动调节 |
| PUMP_USER_EVENT | 0x07 | 用户事件 |
| MILK_EJECTION | 0x08 | 泌乳 |
| BAT_CHARGE_START / END | 0x09/0x0A | 充电 |
| EXCEPTION | 0x0B | 异常 |
| FIRMWARE_UPDATE | 0x0C | 固件升级 |
| OVERFLOW | 0x0D | 溢出 |
| MILK_VOLUME | 0x0E | 奶量 |
| HEAT_SESSION_START / END | 0x0F/0x10 | 加热会话 |
| MASSAGE_SESSION_START / END | 0x11/0x12 | 按摩会话 |
| WEAR_DETECTION | 0x13 | 佩戴检测 |

通用错误码（wire 编码 `0x001000xx`）：`SUCCESS=0x00100000`、`UNKNOWN=0x00100001`、`SVC=0x00100002`、`CMD=0x00100003`、`PARAM=0x00100007`。

## 上行 / 下行链路

**上行**：业务 `DATA_BURY_EVENT_TRIGGER` → 事件采集 packed → `data_bury_publish` → 延时消息池 → bury 线程：在线（BLE 连 + notify 使能）走 `data_bury_server_data_publish` → `data_bury_tlv_packed_to_wire`（packed LE → 联机 TLV BE）→ 0x5A 组帧；离线写 PSRAM 环 + Flash 快照。

**下行**：App 写 Data Bury 特征 → `ble_data_bury_srv_recv_data` → `DEVICE_BLE_DATA_BURY_MSG_CHANNEL` → `data_bury_protocol_server_client` 组包（自动回 ACK：ACK 位被置时回 `TLV type=0x7F`）→ 完整帧 → `data_bury_server_protocol_on_frame` 按 svc 分发。

## 注意事项

- 未连接返回 `-3` 时 `data_bury_hook_publish_event` 专门回落写离线环，其余错误直接丢弃。
- 泵奶工作期间不 flush 离线环（300ms 后重排），避免打断实时数据。
- MFS 分帧接口存在但当前走单帧，依赖 BLE 层 MTU 分包。
