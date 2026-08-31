# BLE / 服务器 / 协议管理

## 1. 模块定位

| 模块 | 目录 | 职责 |
|------|------|------|
| BLE 管理 | `trunk/root_source/ble/` + `drivers/ble/` | GATT 服务注册、连接状态、广播、事件线程化分发 |
| 服务器管理 | `trunk/root_source/servers/` + `overlay/servers/` | 消息枢纽：状态变化 → 发布事件 → 路由到 server client |
| 协议管理 | `trunk/root_source/protocol/` + `overlay/protocol/` | 三套协议编解码入口聚合 |
| 数据埋点 | `trunk/root_source/data_bury/` + `overlay/data_bury/` | 事件采集 → 缓存 → 上报 |

## 2. BLE 服务注册

- `drivers/ble/services/*.c` 定义 4 个 GATT 服务（128 位 UUID + RX/TX/CCCD），`ble_custom_service_init()`（bsp_ble.c）依次 init。
- `ble_service_id_e`：MASTER=0 / FACTORY / REMOTE / DATA_BURY。
- RX 事件 → `ble_service_data_recv(srv_id, data, len)` → 单一 `ble_data_callback`（trunk 注册）。

overlay 侧 4 个服务是把 BLE 消息转成业务 channel 的转换层：
- `master_service` → `DEVICE_BLE_MASTER_MSG_CHANNEL`（旧协议）
- `factory_service` → `#` 走 shell，否则 `DEVICE_BLE_FAC_MSG_CHANNEL`（新协议）
- `remot_service` → `DEVICE_BLE_REMOTE_MSG_CHANNEL`（OTA）
- `data_bury_service` → `DEVICE_BLE_DATA_BURY_MSG_CHANNEL`（埋点）

## 3. 连接状态机

- `CONN_OPENED/MTU/UPDATE` → `BLE_MSG_PERIPH_CONNECT_SUC`；`CONN_CLOSED` → 断开（清 notify_enabled[]）。
- 连接成功：记 connect_idx、触发埋点 `BLE_CONNECTED`、发 `ID_BROADCAST_BLE_STATE_CHANGE`、UI 弹窗。
- 断开：触发埋点、重新开广播、主动调 `OP_CODE_BLE_DISCONNECT`。
- 绑定状态存 `STORE_ID_SYS_BIND_STATUS`，未完成绑定断开回 `BLE_BIND_STATE_INIT`。

## 4. 服务器管理

| server client | 订阅 channel | 协议 |
|---------------|--------------|------|
| master_server_client | MASTER | 旧 App 协议 |
| factory_server_client | FACTORY（+ UART） | 新 comm 协议 |
| ota_server_client | REMOTE | 新 comm 协议（OTA，带组包） |
| data_bury_protocol_server_client | DATA_BURY | 埋点协议 |
| common_server/ | 被 factory/ota 调用 | 新协议子服务实现 |

注册：`servers_hook.c` 在 init 时逐个 `*_server_client_init()`，每个 `add_ble_evt_lisenser(channel, cb)` 挂到 BLE `client_list`；RX 时 `dispatch_ble_client(channel)` 遍历分发。反向走 `dispatch_servers_event_wrapper → servers_thd → *_server_data_publish`。

## 5. 数据埋点链路

```text
业务 trigger → 采集 packed → data_bury_publish → 延时消息池
 → bury 线程：在线（BLE 连 + notify）→ data_bury_server_data_publish → TLV 打包 → 0x5A 帧
            离线 → 写 PSRAM 环 + Flash 快照
 → BLE notify（MTU 分包）
```

公共头填充：`use_session_id`（mac + boot_seq）、`evt_timestamp`（UTC）、`pump_session_id`（会话激活时分配）。泵奶会话最短 120s 才有效。

## 6. 注意事项

- 二分查找表（ble 消息表 / comm 订阅 / 埋点发布）均要求按 id 升序。
- 每次 BLE 收发至少 malloc 256B + 入队（`BLE_MSG_MALLOC_MIN_SIZE`）。
- factory server 缺组包，大帧截断；ota server 才用 assemble。
- V3 广播无调速（OP_CODE_BLE_ADV_SPEED_SET 被注释）。
- notify 使能检查不对称：master/data_bury 上报 trunk，factory/remot 没有（notify_enabled[] 盲区）。
- 断连主动再调一次 DISCONNECT；旧协议加密 `#if 0` 禁用。
- `network_manager.c` 是 0 字节占位；`servers_manager` 的 app_server_data_publish 空实现。
