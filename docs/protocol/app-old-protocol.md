# 旧版 App 协议（0xAA55）

生产主通道协议，供存量 App 使用。实现位于 `overlay/protocol/app_old_commication_protocol/`，命令分发在 `overlay/servers/app_old_protocol_server/app_protocol_handler.c`。

## 帧结构

```text
+---------+--------+--------+--------+----------+-----------+
|  SOF    |  CT    |  CID   |  CAL   |   CAB    | Checksum  |
|  2B     |  1B    |  1B    |  1B    |  0~N B   |  1B       |
+---------+--------+--------+--------+----------+-----------+
```

| 字段 | 大小 | 说明 |
|------|------|------|
| `sof` | 2B | 帧头，固定 `0xAA55` |
| `ct` | 1B | 指令类型（见下） |
| `cid` | 1B | 命令 ID |
| `cal` | 1B | Payload 长度（字节数） |
| `cab` | N B | Payload 数据 |
| `checksum` | 1B | CRC8 校验，覆盖 Header + Payload |

常量（`app_old_protocol_parser.h`）：
- `APP_OLD_COMM_MAX_PACKET_LEN = 256`（MTU=128）
- `APP_OLD_COMM_HEARTBEAT_INTERVAL = 30000`（30s 心跳）
- `APP_OLD_COMM_FRAME_TIMEOUT_MS = 400`（组包超时）
- `BLE_PAYLOAD_CRYPTION = 1`（**加密代码当前被 `#if 0` 禁用**，仅留日志）

## 指令类型 `ct`

| 枚举 | 值 | 说明 |
|------|----|------|
| `CT_REQ` | 0x00 | 请求 |
| `CT_ACK` | 0x01 | 应答 |
| `CT_NACK` | 0x02 | 否定应答 |
| `CT_NOTIFY` | 0x03 | 主动上报 |
| `CT_DEVICE` | 0x04 | 硬件控制上报 |

## 命令 ID `cid`（节选）

| CID | 说明 |
|-----|------|
| `0x00` | 心跳 |
| `0xA0~0xA8` | SN/MAC/自定义信息/软硬件版本读取写入 |
| `0xAD` | 满奶检测状态 |
| `0xAF` | 老化模式测试 |
| `0xB0` | 获取吸奶器状态 |
| `0xB1` | 获取绑定状态 |
| `0xB2` | 设置泵运行参数 |
| `0xB3/B4` | 使用记录数量/内容 |
| `0xB5` | 获取自定义模式 ID |
| `0xBA/BB` | 埋点数量/数据 |
| `0xBD` | 旅行锁控制 |
| `0xC0` | 功能控制（CTRL_DEV） |
| `0xC1` | 设置绑定 |
| `0xC2` | APP 灯光控制 |
| `0xC3` | 设备状态控制（屏幕/夜灯亮度） |
| `0xC4/C5` | 自定义模式设置 / 参数下载 |
| `0xCF` | 重置吸奶器状态 |
| `0xD0/D1/D2` | 主动上报：状态 / 绑定 / 强制断开 |
| `0xE9/EA` | 清除自定义模式 / 设置双泵档位频次 |
| `0xEB` | V3 设备异常状态上报 |
| `0xEC` | 获取设备异常信息 |
| `0xF0/F1` | 获取设备信息 / SN |
| `0xF2` | 设置 RTC |
| `0xF7` | 读取三元组 |
| `0xF8~FA` | 写入 productID / deviceID / 密钥 |
| `0xFC` | 系统控制（开关机/重启/OTA） |
| `0xFD` | 获取加密信息 |

完整枚举见 `app_old_protocol_parser.h` 的 `app_old_comm_cmd_id_e`。

## 组包 / 解析接口

```c
// 分片数据组装完整帧（回调式）
int app_old_protocol_assemble_packet(uint8_t *data, uint16_t len,
                                     app_old_protocol_frame_callback_t callback);
// 反序列化帧
int app_old_protocol_frame_deserialize(uint8_t *frame_buf, uint16_t len,
                                       old_protocol_frame_node_t *frame);
// 组帧发送
int app_old_protocol_frame_builder_send(app_old_commcmd_type_e cmdtype,
                                        app_old_comm_cmd_id_e cid,
                                        uint8_t data_len, uint8_t *data);
```

## 下行 / 上行链路

**下行**：App 写 Master GATT RX 特征 → `master_service.c` → `ble_service_data_recv` → trunk `ble_data_callback` → `BLE_MSG_MASTER_RECIVE_DATA` → ble 线程 → `ble_master_srv_recv_data` → `dispatch_ble_client(MASTER)` → `master_server_client` → `app_old_protocol_assemble_packet` → 完整帧 → `app_old_comm_handle_request_cmd(cid, cal, cab)` 按 CID switch 分发到业务 handler。

**上行**：业务状态变化 → `dispatch_servers_event_wrapper` → `servers_thd` → `app_old_protocol_server_data_publish` → `app_old_protocol_frame_builder_send` → `BLE_MSG_MASTER_SEND_DATA` → notify。

## 注意事项

- 命令处理入口先查 `check_if_fault_warning()`，设备异常时直接上报 abnormal 并返回。
- OTA 期间 `app_old_comm_poll_post_task_handler` 暂停周期轮询上报。
- 关机前补报 `SYSTEM_STATUS_RESET`（D0），用去重标志防双报。
- 单帧上报上限 128B。
