# 新版 comm 协议（0x5CC5）

层次化（多 Service + 多 TLV）协议，用于产测（Factory channel）与 OTA（Remote channel）。实现位于 `overlay/protocol/common_protocl/`，子服务实现位于 `overlay/servers/common_server/`。

## 帧结构

```text
+---------+--------+-------------+----------------+---------------------+-----------+
|  SOF    |  Seq   |  FrameType  |  PayloadLen    |  Payload[Service]... |  CRC16    |
|  2B     |  1B    |  1B         |  2B            |  N B                |  2B       |
+---------+--------+-------------+----------------+---------------------+-----------+
```

| 字段 | 大小 | 说明 |
|------|------|------|
| `sof` | 2B | `0x5CC5` |
| `seq` | 1B | 帧序列号 |
| `frame_type` | 1B | 帧类型（见下） |
| `payload_length` | 2B | Payload 长度（不含 Header/Footer） |
| `payload` | N B | 一个或多个 Service |
| `checksum` | 2B | CRC16，覆盖 Header + Payload |

Service 与 TLV 结构：

```text
Service: SvrID(2B) | SvrLength(2B) | [TLV]...
TLV:     Type(2B)  | Length(2B)    | Value[...]
```

## 常量

| 宏 | 值 | 说明 |
|----|----|------|
| `COMM_PROTOCOL_HEADER_SIZE` | 6 | SOF+Seq+FrameType+Len |
| `COMM_PROTOCOL_FOOTER_SIZE` | 2 | CRC16 |
| `COMM_PROTOCOL_SERVICE_HEADER_SIZE` | 4 | SvrID+Len |
| `COMM_PROTOCOL_TLV_HEADER_SIZE` | 4 | Type+Len |
| `COMM_PROTOCOL_MAX_SERVICE_NUM` | 16 | 单帧最大 Service 数 |
| `COMM_PROTOCOL_MAX_TLV_NUM` | 32 | 单 Service 最大 TLV 数 |
| `COMM_PROTOCOL_MAX_TLV_VALUE_SIZE` | 256 | 单 TLV Value 上限 |
| `COMM_PROTOCOL_MAX_PAYLOAD_SIZE` | `OTA_BLOCK_SIZE(1024)+64` | 最大 Payload |

## 帧类型

```c
typedef enum {
    COMM_PROTOCOL_FRAME_TYPE_REQUEST = 0,
    COMM_PROTOCOL_FRAME_TYPE_RESPONSE,
    COMM_PROTOCOL_FRAME_TYPE_NOTIFY,
    COMM_PROTOCOL_FRAME_TYPE_WRITE_NO_RSP,
} comm_protocol_frame_type_e;
```

## 服务 ID

```c
typedef enum {
    COMM_PROTOCOL_SERVICE_ID_NONE = 0,
    COMM_PROTOCOL_ALGO_SERVICE = 1,
    COMM_PROTOCOL_SENSORS_SERVICE = 2,
    COMM_PROTOCOL_BREAST_PUMP_SERVICE = 3,
    COMM_PROTOCOL_IOT_SERVICE = 4,
    COMM_PROTOCOL_SYSTEM_CONFIG_SERVICE = 5,
    COMM_PROTOCOL_SYSTEM_STATE_SERVICE = 6,
    COMM_PROTOCOL_TESTS_SERVICE = 7,
    COMM_PROTOCOL_TRACKING_DATA_SERVICE = 8,
    COMM_PROTOCOL_OTA_SERVICE = 9,
} comm_protocol_service_id_e;
```

## 组包 / 解析

- `comm_protocol_builder.c`：构建 request/response/notify/write_no_rsp 四类帧，`comm_protocol_builder_finalize` 收尾后回调发送。
- `comm_protocol_parser.c`：环形缓冲组包 `comm_protocol_assemble_packet`（SOF 同步 / CRC 校验 / 超时 1000ms / 单次最多 256 步防脏数据卡死）。
- `common_service.c`：按 service_id 二分查订阅函数；各子服务按 TLV type 二分查 `protocol_service_tlvs_list[]` 执行。
- request 帧自动走 `need_response` 分支组装响应帧。
- TLV 反序列化是零拷贝（`tlvs` 指向入参 buffer），调用方须保证生命周期。

## 错误码

`PROTOCOL_SERVICE_ERROR_CODE_TLV_ID = 0xF000` 为响应帧内的错误码 TLV；`comm_protocol_error_code_t` 定义 `INVALID_FRAME_* / INVALID_CRC / INVALID_SERVICE_ID / INVALID_TLV_*` 等。

## 接入的 server

- `factory_server_client`：订阅 Factory channel，产测上位机 / UART 双路，响应回 `BLE_MSG_FAC_SEND_DATA` 并镜像到 UART。
- `ota_server_client`：订阅 Remote channel，带组包（`assemble`），处理 OTA 子服务。
- `common_server/` 下按服务拆分的协议处理函数：algo / sensors / breast_pump / iot / sys_config / sys_state / tests / tracking_data / ota。

## 注意事项

- 订阅/发布表均要求按 service_id / tlv type 升序（二分查找）。
- `factory_server_client` 的 `on_factory_server_event_cb` 直接 `comm_protocol_subscribe_handler`（**缺少组包功能**），产测大帧会被截断；ota server 才用 `assemble` 组包。
