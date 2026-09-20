# V3/V3Pro 固件文档索引

## 架构设计（`architecture/`）

### 系统级

| 文档 | 说明 |
|------|------|
| [`architecture/system/design-specification.md`](architecture/system/design-specification.md) | 总体设计说明书：分层架构、模块职责、系统流转、关键模块概览 |
| [`architecture/system/guide.md`](architecture/system/guide.md) | 开发规范、模块接入检查清单、接线和事件流约定 |
| [`architecture/system/startup-and-registration.md`](architecture/system/startup-and-registration.md) | 启动流程、模块/设备注册两段式初始化、overlay 钩子注入 |

### 框架核心

| 文档 | 说明 |
|------|------|
| [`architecture/framework/async-channels.md`](architecture/framework/async-channels.md) | 广播 broadcast + 延迟任务 handler_post 双异步通道、线程忙闲判定、WM 汇合点 |
| [`architecture/framework/base-infrastructure.md`](architecture/framework/base-infrastructure.md) | 消息信封、线程/优先级、时间系统、misc 轮询、HAL、日志过滤 |

### 业务与模块

| 文档 | 说明 |
|------|------|
| [`architecture/breast-pump/motor-policy.md`](architecture/breast-pump/motor-policy.md) | 吸奶泵 + 马达策略引擎：策略数据结构、模式列表、20ms 主循环、反馈闭环 |
| [`architecture/ui/ui-windows-resource.md`](architecture/ui/ui-windows-resource.md) | UI/窗口/资源三层分工、窗口路由、LVGL 集成、资源分包与版本化 |
| [`architecture/ui/lang-pack-fds.md`](architecture/ui/lang-pack-fds.md) | 语言包 FDS 多 key：字号/页面文案/页面组件、公共 Latin、整包安装 |
| [`architecture/ui/windows-v6-business.md`](architecture/ui/windows-v6-business.md) | 窗口层业务实现（windows_v6）：16 窗口入口/生命周期/按键/广播/跳转 |
| [`architecture/ui/ui-v7-pages-business.md`](architecture/ui/ui-v7-pages-business.md) | UI 页面层业务实现（v3_ui_v7 / v3pro_ui_v7）：页面三件套、组件、V3/V3Pro 差异 |
| [`architecture/ble/ble-servers.md`](architecture/ble/ble-servers.md) | BLE 服务注册、连接状态机、服务器管理、数据埋点链路 |
| [`architecture/sensor-algo/sensor-algorithm.md`](architecture/sensor-algo/sensor-algorithm.md) | 传感器调度、算法框架三段式、佩戴检测、自适应调光与夜灯 |
| [`architecture/power/power-battery.md`](architecture/power/power-battery.md) | 电源状态机、电量 SOC、充电策略与 PD 匹配 |
| [`architecture/storage/storage-ota.md`](architecture/storage/storage-ota.md) | 存储抽象（KV/分区/RAM 镜像）、工厂校准、OTA 升级 |
| [`architecture/managers/other-managers.md`](architecture/managers/other-managers.md) | LED/音频/输入/异常/飞行模式/产测/工具/外部库 |
| [`architecture/platform/build-system.md`](architecture/platform/build-system.md) | 构建链路、配置解析、IMC/IMP 差异、平台适配层、分区布局 |

## 协议实现（`protocol/`）

| 文档 | 说明 |
|------|------|
| [`protocol/README.md`](protocol/README.md) | 协议总览与代码位置索引 |
| [`protocol/app-old-protocol.md`](protocol/app-old-protocol.md) | 旧版 App 协议（0xAA55）帧结构、命令 ID、上下行链路 |
| [`protocol/comm-protocol.md`](protocol/comm-protocol.md) | 新版 comm 协议（0x5CC5）多 Service/TLV 帧结构、服务 ID |
| [`protocol/data-bury-protocol.md`](protocol/data-bury-protocol.md) | 数据埋点协议（0x5A）链路层/应用层、设备管理命令 |
| [`protocol/ota-protocol.md`](protocol/ota-protocol.md) | OTA 协议命令/应答、断点续传、CRC32 校验 |
| [`protocol/factory-shell.md`](protocol/factory-shell.md) | 产测 Shell 文本命令（#cmd） |

## 业务代码内嵌文档（随代码维护）

以下文档直接存放在对应业务代码目录下，随代码一起维护，是这些子系统的权威细节说明，改动前应一并查阅：

| 文档 | 说明 |
|------|------|
| [`../drivers/battery/BATTERY_SOC_STRATEGY.md`](../drivers/battery/BATTERY_SOC_STRATEGY.md) | 电池充放电 SOC 估算策略（估算电压、OCV 查表、充满去抖） |
| [`../drivers/battery/charge/CHARGE_STRATEGY.md`](../drivers/battery/charge/CHARGE_STRATEGY.md) | 充电策略（PD 匹配、充电芯片初始化、温度分段电流） |
| [`../trunk/root_source/motor/breast_pump_policy/pump_climb_policy.md`](../trunk/root_source/motor/breast_pump_policy/pump_climb_policy.md) | 力度爬升策略（mark_auto / mark_rhythm_switch、韵律切步缓冲） |
| [`../overlay/framework_hooks/led/灯光策略.md`](../overlay/framework_hooks/led/灯光策略.md) | 灯光策略（三路灯光、自适应调光 ALS、夜灯） |
| [`../overlay/servers/common_server/ota/IMG_RES_OTA_PROTOCOL.md`](../overlay/servers/common_server/ota/IMG_RES_OTA_PROTOCOL.md) | 图片资源 OTA 协议 |
| [`../overlay/resources/README.md`](../overlay/resources/README.md) | 资源打包与转换工具说明 |
| [`../overlay/ui/v3pro_ui_v7/README.md`](../overlay/ui/v3pro_ui_v7/README.md) | UI 页面层（v3/v3pro × v6/v7）页面与组件清单 |
| [`../overlay/font/README.md`](../overlay/font/README.md) | 字库结构说明 |

---

## 关键仓库约定

- `overlay/`：产品定制代码、业务配置、UI 与服务实现。
- `trunk/`：公共框架、模块管理器、状态机、通信抽象、运行时基础设施。
- `drivers/`：板级设备驱动（BSP）；opcode/类型 HAL 定义在 `trunk/root_source/hal/`。
- `boards/`：板级配置、编译开关和分区声明。
- `library/`：算法库和可复用计算逻辑。
- `project/`：构建产物输出。

## 文档使用建议

在做以下修改时，优先查阅对应文档：

- 新增/变更广播事件或订阅者 → `architecture/framework/async-channels.md`
- 修改吸奶泵状态机或模式切换 → `architecture/breast-pump/motor-policy.md`
- 调整 LED / UI / 产测 / 升级流程 → 对应模块文档
- 改动协议帧字段或命令 → `protocol/` 下对应文档
- 改动硬件抽象或 board config → `architecture/platform/build-system.md`
- 接入新的模块或扩展现有模块接口 → `architecture/system/guide.md`

> 先看代码，再看文档；遇到跨模块行为时，优先校对文档定义和接口约定，避免因状态流不一致导致功能回归。
