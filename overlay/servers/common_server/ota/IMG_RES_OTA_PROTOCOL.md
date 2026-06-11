# 图片资源 OTA 通信协议

> 适用对象：上位机（PC/App）开发人员  
> 固件实现路径：`overlay/servers/common_server/ota/`  
> 传输通道：BLE（`DEVICE_BLE_REMOTE_MSG_CHANNEL`）  
> 协议栈：通用 `comm_protocol` 帧格式 + OTA Service（ID=9）

---

## 1. 概述

本协议用于通过 BLE 向设备下发**主题图片资源包**（Image Resource），写入外部 Flash 的 `THEME_IMG_RES` 分区（最大 **7MB**）。

升级流程分为三阶段：

```
┌──────────┐    ① 开始升级      ┌──────────┐
│  上位机  │ ─────────────────> │  设备端  │
│ (Host)   │ <───────────────── │ (Device) │
└──────────┘    返回断点/块大小  └──────────┘
      │
      │  ② 循环写数据块（可断点续传）
      v
┌──────────┐    ③ 结束升级      ┌──────────┐
│  上位机  │ ─────────────────> │  设备端  │
│          │ <───────────────── │ (CRC校验)│
└──────────┘    返回最终结果     └──────────┘
      │
      │  异常时设备主动推送 Notify
      v
```

**特性：**
- **原子 TLV 设计**：每个字段独立 TLV，动作 TLV（`START`/`END`）Value 长度为 0
- 支持断点续传：进度持久化至 Flash（`STORE_ID_IMG_RES_OTA_INFO`），开始升级时若 `img_id/total_size/version/crc32/recv_size` 与存储记录一致则从上次的 `recv_size` 继续
- 每次写图片数据块大小固定 **4096 字节**（`OTA_BLOCK_SIZE`，见 `project_config.h`），指**资源文件原始数据**长度，不含任何 TLV/帧头开销；最后一块不足时发剩余长度
- 结束时设备对全量数据做 **CRC32** 完整性校验
- 字节序：**小端（Little-Endian）**

> **协议版本 2.0**：与旧版打包式 TLV（v1.x）不兼容，上位机需按 §4 重新组包。

---

## 2. 传输层说明

| 项目 | 说明 |
|------|------|
| 物理链路 | BLE GATT 自定义特征值 |
| 接收通道 | `DEVICE_BLE_REMOTE_MSG_CHANNEL` |
| 应答发送 | `BLE_MSG_FAC_SEND_DATA` |
| 组包 | 上位机发送的原始字节流需按 `comm_protocol` 帧格式组包；BLE MTU 分片由双方底层自动拼接（参考 `comm_protocol_parser.c`） |
| 帧类型 | 请求用 `REQUEST(0)`，应答用 `RESPONSE(1)`，主动通知用 `NOTIFY(2)` |

---

## 3. 通用帧格式（comm_protocol）

### 3.1 帧结构

```
+--------+-----+-----------+-------------+----------+
|  SOF   | Seq | FrameType | PayloadLen  | Payload  | Checksum |
| 2 byte | 1   | 1 byte    | 2 byte      | N byte   | 2 byte   |
+--------+-----+-----------+-------------+----------+----------+
```

| 字段 | 偏移 | 长度 | 值/说明 |
|------|------|------|---------|
| SOF | 0 | 2 | 固定 `0xC5 0x5C`（小端写入为 `0x5CC5`） |
| Seq | 2 | 1 | 序列号，请求与响应保持一致 |
| FrameType | 3 | 1 | `0`=请求，`1`=响应，`2`=通知 |
| PayloadLen | 4 | 2 | Payload 字节数（不含 Header 和 Footer） |
| Payload | 6 | N | 一个或多个 Service 数据 |
| Checksum | 6+N | 2 | CRC16，覆盖 Header + Payload |

**长度约束（`comm_protocol_defs.h`）：**

| 常量 | 值 | 说明 |
|------|-----|------|
| `COMM_PROTOCOL_MAX_PAYLOAD_SIZE` | **4160**（`OTA_BLOCK_SIZE + 64`） | 单帧 Payload 上限（不含 Header/Footer） |
| `COMM_PROTOCOL_FRAME_SIZE_MAX` | **4168**（4160 + 6 + 2） | 单帧总长度上限 |
| `OTA_BLOCK_SIZE` | **4096**（`project_config.h`） | 每块**资源原数据**字节数（写入 Flash 的净荷，**不含** TLV/帧格式开销） |
| 单帧最大 `WRITE_IMG_DATA` Value | **4144** | 4160 − Service(4) − OFFSET TLV(8) − DATA TLV 头(4)；协议层上限，正常仍按 4096 原数据发块 |

**各帧典型长度：**
- 最小 Payload：**8 字节**（`COMM_PROTOCOL_PAYLOAD_SIZE_MIN`，仅含最小 Service）
- 写数据帧（满块 4096B）：Payload **4112** = Service(4) + OFFSET(8) + IMG_DATA(4100)；帧总长 **4120** = 6 + 4112 + 2
- 开始升级帧：Payload **45**；帧总长 **53**
- 结束升级帧：Payload **8**；帧总长 **16**

> **说明：** `OTA_BLOCK_SIZE` 由 `boards/project_config.h` 定义（当前 **4096**），`comm_protocol_defs.h` 中 `MAX_PAYLOAD = OTA_BLOCK_SIZE + 64`。满块 4096B 原数据经 TLV 封装后 Payload **4112** 字节（整帧 **4120** 字节），仍小于 `MAX_PAYLOAD=4160`。固件组包环形缓冲为 `MAX_PAYLOAD × 2`（`comm_protocol_parser.c`）。

### 3.2 CRC16 帧校验（概要）

CRC16 用于**通信帧**完整性校验（非资源文件校验），固件实现见 `trunk/root_source/utils/gernal_utils.c`。

| 项目 | 说明 |
|------|------|
| 函数名 | `crc16_compute()` |
| 初始值 | `0xFFFF` |
| 输入宽度 | 按字节（8 bit）迭代 |
| 输出宽度 | 16 bit |
| 最终 XOR | 无 |
| 计算范围 | 从帧头 SOF 到 Payload 末尾，**不含** Checksum 本身 |
| 附加方式 | 2 字节**小端序**追加到帧尾 |

**计算步骤：**

```
crc = 0xFFFF
for each byte b in [Header + Payload]:
    crc = (crc >> 8) | (crc << 8)       // 高低字节交换
    crc = crc XOR b
    crc = crc XOR (crc & 0xFF) >> 4
    crc = crc XOR (crc << 8) << 4
    crc = crc XOR ((crc & 0xFF) << 4) << 1
// 将 crc 以 Little-Endian 写入帧尾
```

> 详细 C/Python 参考实现与测试向量见 **§11.1**。

### 3.3 Service 结构

```
+--------+------------+------------------+
| SvrID  | SrvLength  | TLV #1 ... TLV #N|
| 2 byte | 2 byte     | 变长             |
+--------+------------+------------------+
```

| 字段 | 说明 |
|------|------|
| SvrID | 服务 ID，OTA 固定为 **9**（`COMM_PROTOCOL_OTA_SERVICE`） |
| SrvLength | 本 Service 内所有 TLV 的总字节数（不含 Service Header） |

### 3.4 TLV 结构

```
+--------+--------+---------+
| Type   | Length | Value   |
| 2 byte | 2 byte | N byte  |
+--------+--------+---------+
```

---

## 4. OTA TLV 定义（原子化）

协议采用**原子 TLV** 设计：每个业务字段独立成一个 TLV；**动作类 TLV** 的 Value 长度为 **0**，在同帧内先收到所需字段 TLV 后，由动作 TLV 触发执行。

同一 Request 帧内多个 TLV **按顺序处理**，字段 TLV 必须排在对应动作 TLV **之前**。

### 4.1 TLV 一览

| TLV ID | 名称 | 方向 | Value 长度 | 说明 |
|--------|------|------|------------|------|
| `0x0001` | IMG_ID | Host → Device | 4 | 资源包 ID（uint32） |
| `0x0002` | VERSION | Host → Device | 4 | 资源版本（uint32） |
| `0x0003` | CRC32 | Host → Device | 4 | 全量文件 CRC32（uint32） |
| `0x0004` | TOTAL_SIZE | Host → Device | 4 | 资源总大小（uint32） |
| `0x0005` | TYPE | Host → Device | 1 | 升级对象类型（uint8） |
| `0x0006` | BLOCK_SIZE | Device → Host | 2 | 推荐块大小（uint16），固定 **4096**；指**原数据**块，非 TLV 长度 |
| `0x0007` | START_UPGRADE_REQUEST | Host → Device | **0** | 触发开始升级 |
| `0x0008` | IMG_OFFSET_ADDR | 双向 | 4 | 图片偏移地址（uint32）；应答为断点续传偏移 |
| `0x0009` | WRITE_IMG_DATA | Host → Device | 1~4096 | 资源**原数据**（Value 净荷）；不含 TLV 头/帧头 |
| `0x000A` | END_UPGRADE_REQUEST | Host → Device | **0** | 触发结束升级 |
| `0x000B` | UPGRADE_RESULT | Device → Host | 2 | 升级结果（uint16，见 §4.4） |
| `0x000C` | UPGRADE_STATUS_NOTIFY | Device → Host | 2 | 异常通知（uint16） |
| `0xF000` | SERVICE_ERROR_CODE | Device → Host | 2 | 通用服务错误码 |

### 4.2 同帧 TLV 组合规则

| 业务 | Host 发送 TLV 顺序（同一帧） | Device 应答 TLV |
|------|------------------------------|-----------------|
| 开始升级 | `IMG_ID` → `VERSION` → `CRC32` → `TOTAL_SIZE` → `TYPE` → `START_UPGRADE_REQUEST` | `IMG_OFFSET_ADDR` + `BLOCK_SIZE` + `F000` |
| 写数据 | `IMG_OFFSET_ADDR` → `WRITE_IMG_DATA` | `F000` |
| 结束升级 | `END_UPGRADE_REQUEST`（可选前置 `IMG_ID` 校验） | `UPGRADE_RESULT` + `F000` |

**会话生命周期：** 设备在内存中维护 `ota_info` 会话缓存，从**开始升级**到**结束升级**之间持续有效；仅在 `END_UPGRADE_REQUEST` 处理完成后清零。因此：
- **开始升级**：`IMG_ID / VERSION / CRC32 / TOTAL_SIZE / TYPE` 与 `START_UPGRADE_REQUEST` 必须在**同一帧**内按序发送
- **写数据**：`IMG_OFFSET_ADDR` 与 `WRITE_IMG_DATA` 必须在**同一帧**内按序发送
- **结束升级**：`IMG_ID` 可省略（沿用开始升级时缓存的值）；`END_UPGRADE_REQUEST` 触发校验并清空会话

### 4.3 资源类型（TYPE 字段）

| 值 | 枚举 | 说明 |
|----|------|------|
| `0x00` | OTA_OBJECT_TYPE_NONE | 无效 |
| `0x01` | OTA_OBJECT_MAIN_FIRMWARE | 主固件（本协议不使用） |
| `0x02` | OTA_OBJECT_IMAGE_RESOURCE | **图片资源（必须使用此值）** |
| `0x03` | OTA_OBJECT_TOUCH_KEY_FIRMWARE | 触摸按键固件（本协议不使用） |

### 4.4 升级状态码（ota_upgrade_status_code_e）

| 值 | 名称 | 说明 |
|----|------|------|
| `0x0000` | SUCCESS | 成功 |
| `0x0001` | FAIL | 通用失败 |
| `0x0002` | CRC_ERROR | CRC 错误 |
| `0x0003` | VERSION_ERROR | 版本错误 |
| `0x0004` | TYPE_ERROR | 类型错误 |
| `0x0005` | BLOCK_SIZE_ERROR | 块大小错误 |
| `0x0006` | DATA_SIZE_ERROR | 数据大小/范围错误（越界、`recv_size ≠ total_size`、跳写留洞导致校验失败等） |
| `0x0007` | DATA_CRC_ERROR | 全量 CRC32 校验失败 |
| `0x0008` | DATA_VERSION_ERROR | 数据版本错误 |
| `0x0009` | DATA_WRITE_ERROR | Flash 写入/擦除失败 |
| `0x000A` | DATA_BLOCK_TIMEOUT | **连续**块接收超时（见 §5.2）；非累计重传次数 |

### 4.5 通用服务错误码（0xF000）

| 值 | 说明 |
|----|------|
| `0x0000` | TLV 处理成功 |
| `0x0001` | TLV 处理失败 |

### 4.6 块数据大小说明（重要）

协议中的**块数据大小**均指**资源包文件中的原始数据字节数**，与通信层封装无关：

| 概念 | 是否含协议开销 | 说明 |
|------|----------------|------|
| `OTA_BLOCK_SIZE`（4096） | **否** | 上位机每次从本地资源文件读取并下发的净荷长度 |
| `WRITE_IMG_DATA` Value | **否** | TLV 的 Value 字段内容，即资源原数据；TLV 头（Type+Length，4B）另计 |
| `TOTAL_SIZE` / `CRC32` / `IMG_OFFSET_ADDR` | **否** | 均针对完整资源原文件及其偏移 |
| `recv_size`（断点进度） | **否** | 已连续覆盖的原数据右边界；`max(recv_size, offset+len)`，见 §5.2 |
| 写数据帧 Payload / 帧总长 | **是** | 含 Service 头、OFFSET TLV、`WRITE_IMG_DATA` TLV 头及原数据 |

**示例（满块一次写入）：**

```
资源原数据：     4096 字节  ← OTA_BLOCK_SIZE / WRITE_IMG_DATA Value 长度
通信封装后：
  IMG_OFFSET_ADDR TLV   8 B  (头 4 + 值 4)
  WRITE_IMG_DATA TLV    4100 B (头 4 + 值 4096)
  OTA Service 头        4 B
  Payload 合计          4112 B
  整帧（含 Header+CRC） 4120 B
```

上位机应按 `BLOCK_SIZE` 从文件切分原数据填入 `WRITE_IMG_DATA` Value，**不要**把 TLV 头或帧头长度计入块大小。

---

## 5. 各指令详细格式

### 5.1 开始升级

**方向：** Host → Device（Request 帧，含多个原子 TLV）

**Host 发送（按顺序）：**

| TLV ID | Value | 说明 |
|--------|-------|------|
| `0x0001` IMG_ID | uint32 小端 | 资源包 ID |
| `0x0002` VERSION | uint32 小端 | 资源版本号 |
| `0x0003` CRC32 | uint32 小端 | 全量文件 CRC32（算法见 §6） |
| `0x0004` TOTAL_SIZE | uint32 小端 | 总大小 `1 ~ 7340032`（7MB） |
| `0x0005` TYPE | uint8 | 固定 `0x02`（图片资源） |
| `0x0007` START_UPGRADE_REQUEST | **空**（Length=0） | 触发开始升级 |

**设备行为：**
- `TYPE != 0x02` 时拒绝，响应 `0xF000 = 0x0001`
- 读取 Flash 中 `img_res_ota_info_t`（`img_id/total_size/version/crc32/recv_size`）
- 若与本次请求元数据**完全一致** → **断点续传**，应答 `IMG_OFFSET_ADDR = recv_size`
- 否则清零进度，从偏移 `0` 重新开始，并擦除状态按新会话初始化
- 开始成功后启动 **10ms 周期定时器**，用于块接收超时监测（见 §5.2）

**断点续传存储（`img_res_ota_info_t`，Flash `STORE_ID_IMG_RES_OTA_INFO`）：**

| 字段 | 类型 | 说明 |
|------|------|------|
| img_id | uint32 | 资源包 ID |
| total_size | uint32 | 资源总大小 |
| version | uint32 | 版本 |
| crc32 | uint32 | 全量 CRC32 |
| recv_size | uint32 | 已成功写入的原数据字节数 |

开始升级时将请求元数据与上述记录 **整结构 memcmp**；一致则 `IMG_OFFSET_ADDR = recv_size`，否则 `recv_size` 清零。每包 Flash 写入成功后按 §5.2 规则更新 `recv_size` 并持久化。

**Device 应答（Response 帧，Seq 与请求相同）：**

| TLV ID | Value | 说明 |
|--------|-------|------|
| `0x0008` IMG_OFFSET_ADDR | uint32 小端 | 建议下次写入偏移（断点位置） |
| `0x0006` BLOCK_SIZE | uint16 小端 | 固定 **4096**（`0x1000`，资源原数据块大小，见 §4.6） |
| `0xF000` | uint16 | `0x0000`=成功 |

---

### 5.2 写图片数据

**方向：** Host → Device（Request 帧）

**Host 发送（按顺序）：**

| TLV ID | Value | 说明 |
|--------|-------|------|
| `0x0008` IMG_OFFSET_ADDR | uint32 小端 | 当前块在文件中的偏移（0, 4096, 8192...） |
| `0x0009` WRITE_IMG_DATA | byte[] | 资源**原数据**净荷，正常 **4096 字节**（不含 TLV 头，见 §4.6） |

**约束（上位机）：**
- `WRITE_IMG_DATA` **Value 长度**固定 **4096 字节**（最后一块为 `total_size % 4096`）；指原数据，非帧/Payload 总长
- `offset + data_len` 不得超过 `total_size`
- **`IMG_OFFSET_ADDR` 必须等于当前断点**（与 START 应答或 Flash 中 `recv_size` 一致），按 `0 → 4096 → 8192 …` 顺序发送，**禁止跳写/乱序**
- `IMG_OFFSET_ADDR` 必须在本帧内排在 `WRITE_IMG_DATA` **之前**

**设备侧 `recv_size` 更新（`img_res_ota.c`，Flash 写入成功后）：**

```
end_offset = offset + data_len
if (end_offset > recv_size)
    recv_size = end_offset
```

| 场景 | 行为 |
|------|------|
| 正常顺序写 | `offset == recv_size`，写完后 `recv_size += data_len` |
| 超时重传同一包 | `offset + len <= recv_size`，Flash 覆盖写，`recv_size` 不变 |
| 跳写/乱序 | 可能写入 Flash 但留下空洞，`END` 时 `recv_size ≠ total_size` 或 CRC 失败 |

**Device 应答：** 仅 `0xF000`

| 0xF000 值 | 说明 |
|-----------|------|
| `0x0000` | 数据已入队，异步写入 Flash |
| `0x0001` | 参数非法或 OTA 未启动 |

> 写入为**异步**操作（`ota_manager` 工作线程执行 Flash 擦写）：`F000=0` 仅表示数据**已入队**，真正擦写结果在回调中判定；失败时推送 `0x000C` Notify（见 §5.4）并终止升级。  
> 成功写入后按上表更新 `recv_size` 并持久化至 `STORE_ID_IMG_RES_OTA_INFO`。  
>
> **连续超时监测**（`img_res_ota.c`）：
>
> | 项目 | 说明 |
> |------|------|
> | 检测周期 | 定时器 **10ms** 轮询 |
> | 单次超时阈值 | 距上次**成功写入 Flash** 超过 **1000ms** 且 `recv_size < total_size` |
> | 计数变量 | `ota_retry_count`，统计**连续**超时次数（非历史累计） |
> | 计数清零 | 任意一包数据**成功写入 Flash** 后 `ota_retry_count = 0` |
> | 终止条件 | **连续 3 次**超时（中间无成功写入） |
>
> 每次触发超时：
> 1. `comm_protocol_assemble_reset()` 清空组包缓冲
> 2. `ota_retry_count++`
> 3. 若 `ota_retry_count < 3`：推送 Notify `0x000C`（`0x000A`），**不终止**升级，上位机从当前断点 `offset` 重发
> 4. 若 `ota_retry_count >= 3`：推送 `0x0001`（FAIL），清空本地 OTA 状态并终止
>
> `recv_size >= total_size` 等待 `END_UPGRADE` 期间不触发块超时。

---

### 5.3 结束升级

**方向：** Host → Device（Request 帧）

**Host 发送（按顺序）：**

| TLV ID | Value | 说明 |
|--------|-------|------|
| `0x0001` IMG_ID（可选） | uint32 小端 | 若发送须置于 `END_UPGRADE_REQUEST` 之前；可省略，沿用会话缓存 |
| `0x000A` END_UPGRADE_REQUEST | **空**（Length=0） | 触发结束升级（使用会话中缓存的 `img_id` 等元数据） |

**设备行为：**
1. 调用 `comm_protocol_assemble_reset()` 清空组包缓冲
2. 停止 OTA 超时定时器，调用 `end_img_res_upgrade(ota_info.img_id)` 校验 `img_id`、数据大小及 CRC32
3. CRC/大小校验失败时除应答 `UPGRADE_RESULT=FAIL` 外，可能已推送 Notify（如 `0x0007` DATA_CRC_ERROR）
4. 应答 `UPGRADE_RESULT` 后**立即清空** TLV 层 `ota_info` 会话缓存；成功时更新 Flash 中 `recv_size = total_size`

**Device 应答：**

| TLV ID | Value | 说明 |
|--------|-------|------|
| `0x000B` UPGRADE_RESULT | uint16 小端 | `0x0000` 成功 / `0x0001` 失败（`OTA_STATUS_CODE_*`） |
| `0xF000` | uint16 | `0x0000`=TLV 处理成功 |

---

### 5.4 升级状态通知 `0x000C`

**方向：** Device → Host（**Notify 帧**，无需应答）

**Service ID：** **9**（`COMM_PROTOCOL_OTA_SERVICE`，与请求/应答一致）

**触发条件：**

| 场景 | error_code | 是否终止升级 |
|------|------------|--------------|
| 连续块接收超时（第 1、2 次） | `0x000A` | 否；成功写入后连续计数清零 |
| 连续块接收超时（第 3 次） | `0x0001` | 是 |
| Flash 擦写失败 | `0x0009` | 是 |
| 写入越界 / 大小不匹配 | `0x0006` | 是 |
| 全量 CRC32 校验失败 | `0x0007` | 是（`END` 阶段） |
| 其它致命错误 | `0x0001` | 是 |

**帧格式：** Notify 帧内可携带**一个或多个** OTA TLV。常见为单条 `0x000C`：

| TLV | Value 长度 | 说明 |
|-----|------------|------|
| `0x000C` UPGRADE_STATUS_NOTIFY | 2 | uint16 小端 `error_code`（见 §4.4） |

固件亦支持通过 `ota_tlvs_notify_args_t` 一次推送多条 TLV（扩展用）；上位机至少需解析 `0x000C`。

---

## 6. CRC32 资源校验（概要）

CRC32 用于**图片资源文件**完整性校验，在 `END_UPGRADE` 阶段由设备对 Flash 中已写入的全量数据计算，并与 `START_UPGRADE` 请求中的 `crc32` 字段比较。

采用标准 **CRC-32/IEEE 802.3**（与 ZIP、`zlib.crc32()`、Linux `crc32` 命令一致）。

| 项目 | 说明 |
|------|------|
| 函数名 | `crc32_compute()`（查表法） |
| 多项式 | `0xEDB88320`（反射形式） |
| 初始值 | `0xFFFFFFFF` |
| 输入宽度 | 按字节（8 bit）迭代 |
| 输出宽度 | 32 bit |
| 最终 XOR | `crc ^ 0xFFFFFFFF` |
| 计算对象 | 完整资源文件二进制内容 |

**计算步骤：**

```
crc = 0xFFFFFFFF
for each byte b in file:
    crc = (crc >> 8) XOR crc32_table[(crc XOR b) & 0xFF]
crc = crc XOR 0xFFFFFFFF
// 与 START_UPGRADE 请求中的 crc32 字段比较
```

> 详细 C/Python 参考实现、查找表说明与测试向量见 **§11.2**。

---

## 7. 交互时序

```
Host                                          Device
  |                                              |
  |--- [REQ] IMG_ID+VERSION+CRC32+TOTAL_SIZE     |
  |         +TYPE+START_UPGRADE(0x0007) -------->|
  |<-- [RSP] OFFSET(0x0008)+BLOCK_SIZE(0x0006) -|
  |                  + F000(0)                    |
  |                                              |
  |--- [REQ] OFFSET(0x0008)+IMG_DATA(0x0009) --->|  (offset=0)
  |<-- [RSP] F000(0) ---------------------------|
  |                                              |
  |--- [REQ] OFFSET+IMG_DATA -------------------->|  (offset=4096)
  |<-- [RSP] F000(0) ---------------------------|
  |                                              |
  |        ... 重复直到所有数据发送完毕 ...        |
  |                                              |
  |--- [REQ] IMG_ID+END_UPGRADE(0x000A) -------->|
  |<-- [RSP] UPGRADE_RESULT(0x000B)+F000(0) ----|
  |                                              |
  |<-- [NTF] Svc9 + 0x000C (error_code) --------|  (异常/超时可重试)
```

**上位机实现建议：**
1. 维护递增的 `seq`（uint8，0~255 循环）
2. 每次 Request 等待对应 Seq 的 Response 后再发下一包（推荐同步模式）
3. 同一帧内按 §4.2 顺序组装多个原子 TLV
4. 开始升级后根据应答中 `IMG_OFFSET_ADDR(0x0008)` 决定从哪继续传
5. 每包写入后检查 `0xF000`；监听 **OTA Service 9** 的 Notify `0x000C`
6. 收到 `0x000A`：**连续超时**提示，从当前 `recv_size`（断点 offset）**原样重发**；允许覆盖已写区域；任意一包写成功后设备清零连续计数；**连续 3 次**超时（中间无成功写入）后设备终止
7. **禁止** `offset` 跳变；必须始终 `offset == recv_size`（可从 START 应答或本地记录的断点获取）
8. 结束升级后检查 `UPGRADE_RESULT(0x000B)` 返回值

---

## 8. 指令实例（十六进制）

以下示例均省略 CRC16 校验和的 2 字节帧尾（需按 §3.2 计算后追加）。

### 8.1 开始升级

**场景：** 下发 1MB 图片资源

| 参数 | 值 |
|------|-----|
| img_id | `0x00000001` |
| total_size | `0x00100000`（1048576） |
| version | `0x00010000` |
| crc32 | `0x89ABCDEF` |
| type | `0x02` |

**Host 请求帧（6 个原子 TLV）：**

```
C5 5C          // SOF
01             // Seq = 1
00             // FrameType = REQUEST
2D 00          // PayloadLen = 45
09 00          // SvrID = 9 (OTA)
29 00          // SrvLength = 41

01 00 04 00    // IMG_ID(0x0001), len=4
01 00 00 00    // img_id = 1

02 00 04 00    // VERSION(0x0002), len=4
00 00 01 00    // version

03 00 04 00    // CRC32(0x0003), len=4
EF CD AB 89    // crc32

04 00 04 00    // TOTAL_SIZE(0x0004), len=4
00 00 10 00    // total_size = 1MB

05 00 01 00    // TYPE(0x0005), len=1
02             // IMAGE_RESOURCE

07 00 00 00    // START_UPGRADE_REQUEST(0x0007), len=0
[CRC16 2B]
```

**Device 应答帧（首次升级，offset=0）：**

```
C5 5C
01             // Seq = 1
01             // RESPONSE
18 00          // PayloadLen = 24
09 00          // SvrID = 9
14 00          // SrvLength = 20

08 00 04 00    // IMG_OFFSET_ADDR(0x0008), len=4
00 00 00 00    // offset = 0

06 00 02 00    // BLOCK_SIZE(0x0006), len=2
00 10          // block_size = 4096 (0x1000)

00 F0 02 00    // F000, len=2
00 00          // success
[CRC16 2B]
```

**Device 应答帧（断点续传，假设已接收 102400 字节）：**

```
...
08 00 04 00
00 90 01 00    // offset = 102400 (0x19000)
06 00 02 00 00 04
00 F0 02 00 00 00
[CRC16 2B]
```

---

### 8.2 写图片数据

**场景：** 向 offset=0 写入 **4096 字节资源原数据**（两个原子 TLV；帧 Payload 4112B，见 §4.6）

**Host 请求帧（节选）：**

```
C5 5C
02             // Seq = 2
00             // REQUEST
10 10          // PayloadLen = 4112 (0x1010)
09 00          // SvrID = 9
0C 10          // SrvLength = 4108 (0x100C)

08 00 04 00    // IMG_OFFSET_ADDR(0x0008), len=4
00 00 00 00    // offset = 0

09 00 00 10    // WRITE_IMG_DATA(0x0009), len=4096 (0x1000)
[data 4096 bytes]
[CRC16 2B]
```

**第二块（offset=4096）：**

```
08 00 04 00 00 00 10 00    // offset = 4096
09 00 00 10 [data 4096 bytes]
[CRC16 2B]
```

**Device 应答帧：**

```
C5 5C
02             // Seq = 2
01             // RESPONSE
09 00          // PayloadLen = 9
09 00          // SvrID = 9
05 00          // SrvLength = 5
00 F0 02 00 00 00
[CRC16 2B]
```

---

### 8.3 结束升级

**Host 请求帧（仅动作 TLV，img_id 沿用会话缓存）：**

```
C5 5C
03             // Seq = 3
00             // REQUEST
08 00          // PayloadLen = 8
09 00          // SvrID = 9
04 00          // SrvLength = 4

0A 00 00 00    // END_UPGRADE_REQUEST(0x000A), len=0
[CRC16 2B]
```

**Device 应答帧（成功）：**

```
C5 5C
03             // Seq = 3
01             // RESPONSE
10 00          // PayloadLen = 16
09 00          // SvrID = 9
0C 00          // SrvLength = 12

0B 00 02 00    // UPGRADE_RESULT(0x000B), len=2
00 00          // result = 0x0000 (SUCCESS)

00 F0 02 00 00 00
[CRC16 2B]
```

**Device 应答帧（失败，如 CRC 不匹配）：**

```
...
0B 00 02 00 01 00    // result = 0x0001 (FAIL)
00 F0 02 00 00 00
[CRC16 2B]
```

---

### 8.4 异常通知（Flash 写入失败）

**Device Notify 帧：**

```
C5 5C
10             // Seq = 16（设备自增）
02             // NOTIFY
09 00          // PayloadLen = 9
09 00          // SvrID = 9 (OTA_SERVICE)
05 00          // SrvLength = 5
0C 00          // UPGRADE_STATUS_NOTIFY(0x000C), len=2
09 00          // error_code = 0x0009 (DATA_WRITE_ERROR)
[CRC16 2B]
```

**连续超时 Notify（`0x000A`，第 1/2 次，升级不终止）：**

```
...
09 00          // SvrID = 9
05 00          // SrvLength = 5
0C 00 02 00
0A 00          // error_code = 0x000A (DATA_BLOCK_TIMEOUT)
[CRC16 2B]
```

---

## 9. 上位机伪代码

```python
import struct
import zlib

BLOCK_SIZE = 4096
COMM_PROTOCOL_MAX_PAYLOAD_SIZE = 4096 + 64   # 4160，与 comm_protocol_defs.h 一致
COMM_PROTOCOL_FRAME_SIZE_MAX = COMM_PROTOCOL_MAX_PAYLOAD_SIZE + 6 + 2  # 4168

# TLV ID
OTA_TLV_IMG_ID = 0x0001
OTA_TLV_VERSION = 0x0002
OTA_TLV_CRC32 = 0x0003
OTA_TLV_TOTAL_SIZE = 0x0004
OTA_TLV_TYPE = 0x0005
OTA_TLV_BLOCK_SIZE = 0x0006
OTA_TLV_START_UPGRADE = 0x0007
OTA_TLV_IMG_OFFSET_ADDR = 0x0008
OTA_TLV_IMG_DATA = 0x0009
OTA_TLV_END_UPGRADE = 0x000A
OTA_TLV_UPGRADE_RESULT = 0x000B

def tlv_u32(tlv_id: int, value: int) -> bytes:
    return struct.pack("<HHI", tlv_id, 4, value)

def tlv_u8(tlv_id: int, value: int) -> bytes:
    return struct.pack("<HHB", tlv_id, 1, value)

def tlv_empty(tlv_id: int) -> bytes:
    return struct.pack("<HH", tlv_id, 0)

def tlv_bytes(tlv_id: int, data: bytes) -> bytes:
    return struct.pack("<HH", tlv_id, len(data)) + data

def upgrade_image_resource(file_path, img_id, version):
    data = read_file(file_path)
    total_size = len(data)
    crc32 = zlib.crc32(data) & 0xFFFFFFFF

    # Step 1: 开始升级（同帧多个原子 TLV）
    start_tlvs = (
        tlv_u32(OTA_TLV_IMG_ID, img_id)
        + tlv_u32(OTA_TLV_VERSION, version)
        + tlv_u32(OTA_TLV_CRC32, crc32)
        + tlv_u32(OTA_TLV_TOTAL_SIZE, total_size)
        + tlv_u8(OTA_TLV_TYPE, 0x02)
        + tlv_empty(OTA_TLV_START_UPGRADE)
    )
    rsp = send_ota_request(seq=1, tlvs=start_tlvs)
    assert get_error_code(rsp) == 0
    offset = get_tlv_u32(rsp, OTA_TLV_IMG_OFFSET_ADDR)
    assert get_tlv_u16(rsp, OTA_TLV_BLOCK_SIZE) == BLOCK_SIZE

    # Step 2: 按 4096 字节原数据逐块传输（chunk 为文件净荷，不含 TLV/帧头）
    seq = 2
    while offset < total_size:
        chunk_len = min(BLOCK_SIZE, total_size - offset)
        write_tlvs = (
            tlv_u32(OTA_TLV_IMG_OFFSET_ADDR, offset)
            + tlv_bytes(OTA_TLV_IMG_DATA, data[offset:offset + chunk_len])
        )
        rsp = send_ota_request(seq=seq, tlvs=write_tlvs)
        assert get_error_code(rsp) == 0
        offset += chunk_len
        seq = (seq + 1) & 0xFF

    # Step 3: 结束升级（img_id 已在会话中，可只发 END_UPGRADE）
    end_tlvs = tlv_empty(OTA_TLV_END_UPGRADE)
    rsp = send_ota_request(seq=seq, tlvs=end_tlvs)
    assert get_error_code(rsp) == 0
    assert get_tlv_u16(rsp, OTA_TLV_UPGRADE_RESULT) == 0x0000
```

---

## 10. 错误处理与重试策略

| 场景 | 建议处理 |
|------|----------|
| Payload 超过 4160 字节 | 上位机组包前校验 `len(payload) <= COMM_PROTOCOL_MAX_PAYLOAD_SIZE`；设备 `comm_protocol_parser.c` 会滑动丢弃非法 `PayloadLen` |
| `offset ≠ recv_size`（跳写） | 设备可能仍写入 Flash 但进度异常；`END` 失败。上位机应始终按断点顺序发块 |
| `0xF000 = 1` | 检查参数、确认 OTA 已正确开始 |
| 收到 `0x000C` + `0x000A` | **连续超时**（第 1 或 2 次）：从当前 `offset` 重发；成功写入一包后连续计数清零 |
| 连续 3 次超时后收到 `0x0001` | 中间无成功写入，设备已终止；需重新 `START`（相同元数据可断点续传） |
| 收到 `0x000C` + `0x0009/0x0006/0x0007` | 记录 error_code，终止；可重新 `START`（相同元数据将断点续传） |
| `UPGRADE_RESULT = 0x0001` | 全量 CRC 或大小校验失败，需重新传输 |
| BLE 断连 | 重连后用相同 `img_id/total_size/version/crc32` 开始升级，设备从 Flash 恢复 `recv_size` |
| `type != 0x02` | 修正 type 字段后重试 |

---

## 11. CRC 参考实现与测试向量

本章提供与固件**完全一致**的 CRC16 / CRC32 计算函数，上位机可直接复制使用。

---

### 11.1 CRC16 详细说明

#### 11.1.1 算法参数

| 参数 | 值 |
|------|-----|
| 算法来源 | 项目自定义（`gernal_utils.c`，非标准 CRC-16-CCITT / Modbus） |
| 初始值 `crc` | `0xFFFF` |
| 反射输入/输出 | 否（通过高低字节交换实现） |
| 结果字节序（帧尾） | Little-Endian |

#### 11.1.2 C 语言参考实现

与固件 `crc16_compute()` 完全一致：

```c
unsigned short crc16_compute(unsigned char const *p_data, unsigned int size,
                             unsigned short const *p_crc)
{
    unsigned short crc = (p_crc == NULL) ? 0xFFFF : *p_crc;
    for (uint32_t i = 0; i < size; i++) {
        crc  = (unsigned char)(crc >> 8) | (crc << 8);
        crc ^= p_data[i];
        crc ^= (unsigned char)(crc & 0xFF) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xFF) << 4) << 1;
    }
    return crc;
}
```

**帧组包时调用方式（与 `comm_protocol_builder_finalize` 一致）：**

```c
uint16_t frame_body_len = COMM_PROTOCOL_HEADER_SIZE + payload_length;
uint16_t checksum = crc16_compute(frame_buffer, frame_body_len, NULL);
frame_buffer[frame_body_len]     = checksum & 0xFF;        // 低字节
frame_buffer[frame_body_len + 1] = (checksum >> 8) & 0xFF; // 高字节
```

**帧解析时校验方式（与 `comm_protocol_frame_deserialize` 一致）：**

```c
uint16_t checksum = crc16_compute(buffer, buffer_size - 2, NULL);
uint16_t received = buffer[buffer_size - 2] | ((uint16_t)buffer[buffer_size - 1] << 8);
if (checksum != received) { /* CRC 错误 */ }
```

#### 11.1.3 Python 参考实现

与 `overlay/protocol/common_protocl/tools/test_comm_protocol_assemble.py` 一致：

```python
def crc16_compute(data: bytes, crc: int = 0xFFFF) -> int:
    """计算 CRC16，与固件 crc16_compute() 完全一致"""
    for b in data:
        crc = ((crc >> 8) & 0xFFFF) | ((crc << 8) & 0xFFFF)
        crc ^= b
        crc ^= (crc & 0xFF) >> 4
        crc ^= ((crc << 8) << 4) & 0xFFFF
        crc ^= (((crc & 0xFF) << 4) << 1) & 0xFFFF
    return crc & 0xFFFF


COMM_PROTOCOL_MAX_PAYLOAD_SIZE = 4096 + 64  # 4160


def build_frame(seq: int, frame_type: int, payload: bytes) -> bytes:
    """组装完整通信帧（含 CRC16）"""
    import struct
    if len(payload) > COMM_PROTOCOL_MAX_PAYLOAD_SIZE:
        raise ValueError(f"payload 超长: {len(payload)} > {COMM_PROTOCOL_MAX_PAYLOAD_SIZE}")
    SOF = 0x5CC5
    header = struct.pack("<HBBH", SOF, seq, frame_type, len(payload))
    body = header + payload
    checksum = crc16_compute(body)
    return body + struct.pack("<H", checksum)


def verify_frame(frame: bytes) -> bool:
    """校验帧 CRC16"""
    if len(frame) < 8:
        return False
    body = frame[:-2]
    expected = crc16_compute(body)
    received = frame[-2] | (frame[-1] << 8)
    return expected == received
```

#### 11.1.4 CRC16 测试向量

| 输入数据（hex） | CRC16 结果 |
|----------------|------------|
| 空（0 字节） | `0xFFFF` |
| ASCII `"123456789"` | `0x29B1` |
| `C5 5C 01 00 00 00 0D 00 09 00 09 00 04 00 04 00 01 00 00 00` | `0x346C` |

#### 11.1.5 CRC16 帧校验实例

以下为一帧「开始升级请求」的 CRC16 计算过程（不含帧尾 Checksum）：

```
待校验数据（34 字节）:
C5 5C 01 00 00 00 0D 00 09 00 09 00 04 00 04 00
01 00 00 00 00 00 10 00 00 00 01 00 EF CD AB 89 02

CRC16 计算结果: 0xEB4A

完整帧（36 字节）:
C5 5C 01 00 00 00 0D 00 09 00 09 00 04 00 04 00
01 00 00 00 00 00 10 00 00 00 01 00 EF CD AB 89 02
4A EB    ← Checksum 小端序
```

---

### 11.2 CRC32 详细说明

#### 11.2.1 算法参数

| 参数 | 值 |
|------|-----|
| 标准名称 | CRC-32 / IEEE 802.3 / PKZip |
| 多项式（正常） | `0x04C11DB7` |
| 多项式（反射） | `0xEDB88320` |
| 初始值 `crc` | `0xFFFFFFFF` |
| 最终 XOR | `0xFFFFFFFF` |
| 反射输入/输出 | 是（查表法内部处理） |
| 结果字节序（协议字段） | Little-Endian（4 字节 uint32） |

#### 11.2.2 C 语言参考实现

**单字节迭代（与固件 `crc32_compute()` 一致）：**

```c
/* crc32_table[256] 定义见 trunk/root_source/utils/gernal_utils.c */

uint32_t crc32_compute(unsigned char const *p_data, unsigned int size,
                       uint32_t const *p_crc)
{
    uint32_t crc = (p_crc == NULL) ? 0xFFFFFFFF : *p_crc;
    for (uint32_t i = 0; i < size; i++) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ p_data[i]) & 0xFF];
    }
    return crc;
}
```

**资源文件完整性校验（与 `img_res_integrity_check()` 一致）：**

```c
uint32_t crc = 0xFFFFFFFF;
while (has_more_data) {
    CRC32_UPDATE(crc, buf, read_len);  /* 等价于 crc = crc32_compute(buf, len, &crc) */
}
crc ^= 0xFFFFFFFF;
if (crc != ota_info.crc32) { /* 校验失败 */ }
```

> `crc32_table[256]` 为标准 IEEE 802.3 查找表，完整 1024 字节表定义在  
> `trunk/root_source/utils/gernal_utils.c`，上位机无需自行推导，使用下方 Python 实现即可。

#### 11.2.3 Python 参考实现

**方式一：推荐，使用标准库（与固件结果一致）**

```python
import zlib

def compute_crc32(data: bytes) -> int:
    """计算资源文件 CRC32，与固件 img_res_integrity_check() 一致"""
    return zlib.crc32(data) & 0xFFFFFFFF
```

**方式二：查表法（与固件 `crc32_compute()` 逻辑一致，便于无 zlib 环境）**

```python
# 标准 CRC-32 IEEE 802.3 查找表（与 gernal_utils.c 相同）
CRC32_TABLE = [
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
]


def crc32_compute(data: bytes, crc: int = 0xFFFFFFFF) -> int:
    """单轮 CRC32 计算（不含最终 XOR）"""
    for b in data:
        crc = (crc >> 8) ^ CRC32_TABLE[(crc ^ b) & 0xFF]
    return crc & 0xFFFFFFFF


def compute_crc32(data: bytes) -> int:
    """资源文件 CRC32（含最终 XOR，与固件校验一致）"""
    return crc32_compute(data) ^ 0xFFFFFFFF
```

**分块计算（模拟设备按 `OTA_BLOCK_SIZE` 分块读取 Flash 校验）：**

```python
def compute_crc32_chunked(data: bytes, chunk_size: int = 4096) -> int:
    crc = 0xFFFFFFFF
    for i in range(0, len(data), chunk_size):
        crc = crc32_compute(data[i:i + chunk_size], crc)
    return crc ^ 0xFFFFFFFF
```

#### 11.2.4 CRC32 测试向量

| 输入数据 | CRC32 结果 |
|----------|------------|
| 空文件（0 字节） | `0x00000000` |
| ASCII `"123456789"` | `0xCBF43926` |
| 单字节 `0x00` | `0xD202EF8D` |
| 单字节 `0xFF` | `0xFF000000` |

#### 11.2.5 CRC32 在协议中的使用

```
1. 上位机读取完整资源文件
2. crc32 = compute_crc32(file_data)
3. 填入 START_UPGRADE_REQUEST 的 crc32 字段（4 字节小端序）
4. 设备写入完成后，以相同算法校验 Flash 数据
5. 校验通过 → END_UPGRADE_RESPONSE = 0x00
   校验失败 → END_UPGRADE_RESPONSE = 0x01，并可能推送 0x0006 Notify (0x0007)
```

**命令行快速验证：**

```bash
# Linux
crc32 /path/to/image_res.bin

# Python
python3 -c "import zlib,sys; d=open(sys.argv[1],'rb').read(); print(hex(zlib.crc32(d)&0xffffffff))" image_res.bin
```

---

## 12. 相关源码索引

| 文件 | 职责 |
|------|------|
| `ota_tlvs_protocol.h` | TLV ID、状态码、数据结构定义 |
| `ota_tlvs_protocol.c` | 原子 TLV 解析、会话缓存、应答组包、Notify 组包（OTA Service 9） |
| `ota_server_protocol.c` | OTA Service TLV 分发 |
| `common_tlvs.c` | `common_tlv_add_u8/u16/u32/empty` 应答辅助函数 |
| `resources/img_res_ota.c` | Flash 擦写、断点续传持久化、**连续超时**监测（1s×3）、CRC 校验 |
| `persist_storage.h` | `img_res_ota_info_t` 断点结构定义 |
| `resource_persist_data.c` | OTA 进度读写 Flash |
| `project_config.h` | **`OTA_BLOCK_SIZE = 4096`**（产品配置） |
| `comm_protocol_defs.h` | 帧/Service/TLV 通用格式；`MAX_PAYLOAD=OTA_BLOCK_SIZE+64`（**4160**） |
| `comm_protocol_parser.c` | BLE 分片组包；`PACKET_ASSEMBLY_BUFFER_SIZE = MAX_PAYLOAD × 2` |
| `gernal_utils.c` | `crc16_compute()` / `crc32_compute()` 实现 |
| `test_comm_protocol_assemble.py` | CRC16 Python 参考与组包测试 |
| `ota_manager_api.h` | `OTA_BLOCK_SIZE` 后备默认 1024（`#ifndef`）；实际以 `project_config.h` 为准 |
| `boards/project_partitions.h` | `THEME_IMG_RES_SIZE = 7MB` |

---

## 13. 修订记录

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.0 | 2026-06-10 | 初始版本，基于当前固件实现整理 |
| 1.1 | 2026-06-10 | 明确每次写数据固定 1024 字节（最后一块除外） |
| 1.2 | 2026-06-10 | CRC32 改为标准算法（增加最终 XOR 0xFFFFFFFF） |
| 1.3 | 2026-06-10 | 补充 CRC16/CRC32 详细计算函数、测试向量与实例 |
| 2.0 | 2026-06-10 | TLV 原子化重构（字段 TLV + 动作 TLV，ID 0x0001~0x000C） |
| 2.1 | 2026-06-10 | 会话改为 END 时重置；`offset_addr` 并入 `ota_info` |
| 2.2 | 2026-06-10 | `UPGRADE_RESULT` 改为 uint16；`0x0008` 统一为 `IMG_OFFSET_ADDR` |
| 2.3 | 2026-06-10 | 块数据 500ms 接收超时监测（`0x000A`） |
| 2.4 | 2026-06-10 | `COMM_PROTOCOL_MAX_PAYLOAD_SIZE` 调整为 **1088**（`1024+64`）；满 1024B 数据块可单帧发送 |
| 2.5 | 2026-06-10 | 明确块数据大小为资源**原数据**长度，不含 TLV/帧格式开销（§4.6） |
| 2.6 | 2026-06-10 | 超时迁至 `img_res_ota.c`（**1s**）；Notify 改为 **OTA Service 9**；断点持久化 Flash；`END` 时 `assemble_reset` |
| 2.7 | 2026-06-10 | 超时策略改为**连续超时**：`ota_retry_count` 仅计连续超时，成功写入清零；连续 3 次才终止 |
| 2.8 | 2026-06-10 | 明确 `recv_size` 更新为 `max(recv_size, offset+len)`；上位机须顺序发块，超时允许同 offset 重传 |
| 2.9 | 2026-06-10 | `OTA_BLOCK_SIZE` 调整为 **4096**（`project_config.h`）；`MAX_PAYLOAD=4160`，满块写帧 **4120** 字节 |
