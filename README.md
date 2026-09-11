# Smart Wearable Firmware Framework (v3)

基于模块化与分层设计的智能穿戴固件工程框架，面向吸奶泵/母婴穿戴设备。

## 目录结构

```
FIRMWARE_V3/
├── GNUmakefile                      ← 构建入口（目标/芯片/配置）
├── build_helper.sh                  ← 构建辅助脚本（交互/非交互模式）
├── scripts/                         ← 开发辅助脚本
│   ├── map_section_sizes.py         ← ELF 段大小分析
│   ├── ram_usage_analyzer.py        ← RAM 使用分析
│   ├── psram_usage_analyzer.py      ← PSRAM 使用分析
│   └── *.md                         ← 脚本使用文档
├── tools/                           ← 开发/调试工具
│   └── ble_factory_tool/            ← BLE 工厂测试工具
├── docs/                            ← 项目文档与参考手册
├── compile_commands.json            ← 编译数据库（IDE/静态分析用）
├── configs/                         ← 产品特定的 Makefile 配置
│   ├── V3_2710IMC_GNUmakefile      ← V3_2710IMC 产品配置
│   └── V3_2710IMP_GNUmakefile      ← V3_2710IMP 产品配置
├── library/                         ← 算法库与通用库
│   └── root_algo/                   ← 根算法库（融合算法/穿戴检测等）
├── boards/                          ← 工程级项目/产品配置入口（头文件集中）
│   ├── product_info.h               ← 产品信息与型号定义
│   ├── project_config.h             ← 编译/功能开关/宏定义（顶层聚合）
│   ├── project_log_filter.h         ← 模块化日志过滤策略
│   ├── project_partitions.h         ← 分区/存储布局声明
│   └── project_support_overlay.h    ← 叠加配置（覆盖默认项）
│
├── drivers/                         ← 板级与设备驱动（弱 RTOS 依赖）
│   ├── adc/                         ← 模数转换（ADC）
│   ├── air_pressure/                ← 气压传感器/气压计
│   ├── audio/                       ← 音频编解码/接口驱动
│   ├── base/                        ← 基础硬件抽象/通用寄存器封装
│   ├── battery/                     ← 电池/电量计/充电芯片
│   ├── ble/                         ← 低功耗蓝牙控制器相关适配
│   ├── common/                      ← 驱动通用工具/适配层
│   ├── database/                    ← 简易 KV/参数存储
│   ├── fatfs/                       ← FATFS 文件系统适配
│   ├── flash/                       ← 外部/内部 Flash 访问
│   ├── force_sensor/                ← 力传感器/压力传感器
│   ├── i2c/                         ← 通用 I2C 总线驱动/适配
│   ├── imu/                         ← 惯性传感器（加计/陀螺/融合）
│   ├── key/                         ← 物理按键/触摸按键扫描
│   ├── lcd/                         ← 显示屏/显示控制器
│   ├── led/                         ← 指示灯/背光控制
│   ├── light_sensor/                ← 光传感器驱动
│   ├── motor/                       ← 振动马达/线性马达驱动
│   ├── uart/                        ← 串口/UART 驱动
│   └── wifi/                        ← WiFi 驱动支持
│
├── overlay/                         ← 资源与钩子（工程可重写/定制，版本化管理）
│   ├── adaptive_dimming/            ← 自适应调光策略
│   ├── algo/                        ← 算法处理实现（采集/核心/后处理）
│   ├── ble/                         ← BLE 服务端实现
│   │   ├── data_bury_service/       ← 数据埋点 BLE 服务
│   │   ├── factory_service/         ← 工厂测试 BLE 服务
│   │   ├── master_service/          ← 主设备 BLE 服务
│   │   └── remot_service/           ← 远程 BLE 服务
│   ├── breast_pump/                 ← 吸奶泵业务配置（模式/参数表）
│   │   ├── mode/                    ← 工作模式定义（7 种标准模式）
│   │   └── table/                   ← 参数表配置
│   ├── data_bury/                   ← 数据埋点事件处理
│   ├── font/                        ← 字体与语言包（打包工具 + font_v3）
│   │   ├── packaging_tools/         ← 语言包 pack.sh / preview.sh
│   │   └── font_v3/                 ← 公共 Latin 字库 + 分语言 bin/font_img
│   ├── framework_hooks/             ← 框架钩子（任务/事件/日志等注入点）
│   │   ├── abnormal/                ← 异常检测钩子
│   │   ├── airplane_mode/           ← 飞行模式钩子
│   │   ├── algo/                    ← 算法模块钩子
│   │   ├── ble/                     ← 蓝牙模块钩子
│   │   ├── breast_pump/             ← 吸奶泵业务钩子
│   │   ├── data_bury/               ← 数据埋点钩子
│   │   ├── input_key/               ← 输入按键钩子
│   │   ├── led/                     ← 指示灯/背光钩子
│   │   ├── motor/                   ← 马达控制钩子
│   │   ├── ota/                     ← OTA 升级钩子
│   │   ├── power/                   ← 电源管理钩子
│   │   ├── product_tests/           ← 产测与自检钩子
│   │   ├── sensor/                  ← 传感器处理钩子
│   │   ├── servers/                 ← 服务器框架钩子
│   │   ├── resource/                ← 资源初始化钩子（语言包 probe）
│   │   ├── storage/                 ← 存储配置钩子
│   │   └── ui/                      ← UI 初始化与事件钩子
│   ├── input_key/                   ← 输入按键码定义与映射
│   ├── night_light/                 ← 小夜灯策略控制
│   ├── product_tests/               ← 产品测试（工厂测试/硬件自检/通信）
│   ├── protocol/                    ← 协议处理（应用协议/通用协议）
│   ├── resources/                   ← 图片/主题二进制资源（版本化）
│   │   ├── v3_res_v2~v7/            ← V3 产品资源（v2-v7 版本）
│   │   └── v3pro_res_v2~v7/        ← V3 Pro 产品资源（v2-v7 版本）
│   ├── sensor/                      ← 传感器配置与校准参数
│   │   ├── air_pressure/            ← 气压传感器配置
│   │   ├── force_sensor/            ← 力传感器配置
│   │   ├── imu/                     ← IMU 传感器配置
│   │   └── light_sensor/            ← 环境光传感器配置
│   ├── servers/                     ← 服务器实现
│   │   ├── app_old_protocol_server/ ← 应用层旧版协议服务器
│   │   ├── common_server/           ← 通用服务器
│   │   ├── data_bury_protocol_server_client/ ← 数据埋点协议服务器客户端
│   │   ├── data_bury_server/        ← 数据埋点服务器
│   │   ├── factory_server_client/   ← 工厂测试服务器客户端
│   │   ├── master_server_client/    ← 主服务器客户端
│   │   └── ota_server_client/       ← OTA 服务器客户端
│   ├── storage/                     ← 存储分区配置与持久化定义
│   │   ├── breast_pump/             ← 吸奶泵业务存储配置
│   │   ├── factory/                 ← 工厂校准数据存储
│   │   ├── resource/                ← 资源存储配置
│   │   └── system/                  ← 系统配置存储
│   ├── ui/                          ← UI 实现（版本化、多产品）
│   │   ├── lang_pack/               ← 外部语言包加载/安装（v3 / v3pro 共用）
│   │   ├── v3_ui_v2~v7/             ← V3 产品 UI（v7 含 lang/ 文案与坐标源）
│   │   └── v3pro_ui_v2~v7/         ← V3 Pro 产品 UI（v7 含 lang/）
│   └── windows/                     ← UI 窗口/页面资源（版本化）
│       ├── windows_v1~v6/           ← 窗口定义 v1-v6（每版含完整的窗口集）
│       │   ├── airplane_mode/       ← 飞行模式窗口
│       │   ├── battery/             ← 电池状态/电量展示窗口
│       │   ├── breast_pump/         ← 吸奶泵业务相关窗口
│       │   ├── charge/              ← 充电过程/满电提示窗口
│       │   ├── factory_reset/       ← 恢复出厂设置窗口
│       │   ├── guide/               ← 引导/教学流程窗口
│       │   ├── home/                ← 主页/主屏窗口
│       │   ├── logo/                ← 开机 Logo/品牌展示
│       │   ├── ota/                 ← OTA 升级流程窗口
│       │   ├── popup_windows/       ← 通用弹窗集合（提示/确认/错误等）
│       │   ├── power_off/           ← 关机页面窗口
│       │   ├── product_test_window/ ← 产测窗口
│       │   ├── setting/             ← 设置/选项类窗口
│       │   ├── system_abnormal/     ← 系统异常窗口
│       │   └── theme_guide/         ← 主题引导窗口
│       │   └── windows_map_define.c/h ← 窗口注册与映射表
│
├── project/                         ← 构建产物与构建系统文件
│
└── trunk/                           ← 框架主干（平台与核心业务）
    ├── include/                     ← 对外 API 头文件（管理器/服务接口，共 39 个）
    │   ├── abnormal_manager_api.h   ← 异常检测管理接口
    │   ├── airplane_mode_api.h      ← 飞行模式管理接口
    │   ├── algorithm_manager_api.h  ← 算法框架管理接口
    │   ├── audio_manager_api.h      ← 音频管理接口
    │   ├── battery_manager_api.h    ← 电池管理接口
    │   ├── ble_manager_api.h        ← BLE 管理接口
    │   ├── breast_pump_manager_api.h ← 吸奶泵管理接口
    │   ├── broadcast_manager_api.h  ← 广播管理接口
    │   ├── data_bury_manager_api.h  ← 数据埋点管理接口
    │   ├── handler_post.h           ← 任务/消息派发接口
    │   ├── hang_detect.h            ← 卡死检测/线程看门狗
    │   ├── key_input_manager_api.h  ← 输入系统管理接口
    │   ├── led_manager_api.h        ← LED 管理接口
    │   ├── misc_manager_api.h       ← 杂项能力管理接口
    │   ├── motor_manager_api.h      ← 马达管理接口
    │   ├── ota_manager_api.h        ← OTA 升级管理接口
    │   ├── peripheral_bus_api.h     ← 外设总线接口
    │   ├── power_manager_api.h      ← 电源管理接口
    │   ├── project_default_features.h ← 默认功能开关
    │   ├── project_default_parameter.h ← 默认参数定义
    │   ├── protocol_manager_api.h   ← 协议管理接口
    │   ├── resource_manager_api.h   ← 资源管理接口
    │   ├── root_common.h            ← 通用接口集合
    │   ├── root_common_api.h        ← 通用接口 API
    │   ├── root_common_base.h       ← 公共基类/宏/类型
    │   ├── root_common_config.h     ← 统一配置/线程栈与优先级
    │   ├── root_common_register.h   ← 模块注册/框架入口
    │   ├── root_list_api.h          ← 链表管理接口
    │   ├── root_log_define.h        ← 日志系统定义
    │   ├── root_message.h           ← 异步消息实体/统一消息封装
    │   ├── root_storage_base.h      ← 存储分区/魔数基础定义
    │   ├── sensor_manager_api.h     ← 传感器管理接口
    │   ├── servers_manager_api.h    ← 服务器管理接口
    │   ├── storage_manager_api.h    ← 存储管理接口
    │   ├── tests_manager_api.h      ← 测试模块接口
    │   ├── thread_api.h             ← 线程接口抽象
    │   ├── time_system_api.h        ← 时间系统接口
    │   ├── ui_manager_api.h         ← UI 管理接口
    │   └── windows_manager_api.h    ← 窗口管理接口
    │
    ├── root_source/                 ← 核心子系统实现（面向上层的服务，共 25 个模块）
    │   ├── abnormal/                ← 异常检测与管理
    │   ├── airplane_mode/           ← 飞行模式
    │   ├── algorithm/               ← 算法框架/调度
    │   ├── audio/                   ← 音频子系统
    │   ├── base/                    ← 基础设施（初始化/错误码/工具）
    │   ├── battery/                 ← 电池管理子系统
    │   ├── ble/                     ← BLE 主干（Host/Profile/Manager）
    │   ├── breast_pump/             ← 吸奶泵业务模块（模式/控制/状态管理）
    │   ├── data_bury/               ← 数据埋点管理（采集/上传/存储）
    │   ├── hal/                     ← 面向上层的硬件抽象
    │   ├── key_input/               ← 输入系统（按键/触摸）
    │   ├── led/                     ← LED/背光控制子系统
    │   ├── misc/                    ← 杂项能力（时间/随机数/配置）
    │   ├── motor/                   ← 马达子系统（含策略引擎）
    │   ├── network/                 ← 网络与协议
    │   ├── ota/                     ← 升级子系统
    │   ├── pm/                      ← 电源管理/低功耗策略
    │   ├── protocol/                ← 应用/产测/私有协议
    │   ├── resource/                ← 资源管理/解压/缓存
    │   ├── sensor/                  ← 传感器封装（IMU/FSensor/气压汇聚）
    │   ├── servers/                 ← 服务器管理子系统
    │   ├── storage/                 ← 存储抽象（KV/FS/分区）
    │   ├── tests/                   ← 自检/产测/集成测试模块
    │   ├── ui/                      ← UI 管理子系统（界面控制/交互）
    │   ├── utils/                   ← 通用工具集
    │   └── windows/                 ← UI 窗口框架/页面管理
    │
    ├── platform_chip/               ← 芯片平台相关适配层
    │   └── bes/                     ← BES 平台移植/驱动对接
    │       ├── os_base/             ← 操作系统基础抽象
    │       └── sdk/                 ← BES SDK 适配（Git 子仓库）
    │
    ├── external/                    ← 外部组件/第三方库（8 个）
    │   ├── base64/                  ← Base64 编解码库
    │   ├── cJSON/                   ← JSON 解析库
    │   ├── crypto/                  ← 加解密/哈希/安全库
    │   ├── littlefs/                ← 嵌入式文件系统（轻量级 FS）
    │   ├── lvgl/                    ← 图形界面库（LVGL）
    │   ├── md5/                     ← MD5 哈希算法库
    │   ├── quicklz/                 ← 资源压缩/解压库
    │   └── scan_key/                ← 按键扫描库
    │
    ├── simulator/                   ← 仿真/PC 侧开发调试（如有）
    ├── doc/                         ← 文档/设计说明/接口约定
    └── tools/                       ← 开发/发布工具脚本
```

## 核心功能特性

### 1. 外设总线系统 (Peripheral Bus)
- **文件**: `trunk/include/peripheral_bus_api.h`
- **功能**: 统一的外设设备管理接口，支持设备注册、状态管理、操作封装
- **特性**:
  - 设备 ID 枚举管理
  - 设备状态跟踪（初始化/挂起/恢复）
  - 标准化的设备操作接口（open/close/read/write/ioctl）
  - 宏封装简化设备操作

### 2. 增强日志系统
- **文件**: `trunk/include/root_log_define.h`、`boards/project_log_filter.h`
- **功能**: 模块化日志过滤和彩色输出支持
- **特性**:
  - 多级日志级别（ERROR/WARN/INFO/DEBUG/VERBOSE）
  - 模块化日志过滤（支持 24 个功能模块独立过滤）
  - 彩色日志输出支持
  - 时间戳和线程 ID 支持

### 3. 任务/消息派发（Handler Post）
- **文件**: `trunk/include/handler_post.h`
- **功能**: 轻量级任务/消息派发与异步执行，降低线程耦合
- **特性**:
  - 支持延时消息投递
  - 消息队列管理
  - 回调函数封装

### 4. 卡死检测（Hang Detect）
- **文件**: `trunk/include/hang_detect.h`
- **功能**: 线程卡死/超时检测，支持独立监控线程
- **特性**:
  - 独立看门狗线程
  - 可配置超时阈值
  - 自动复位或告警

### 5. 统一配置基座
- **文件**: `trunk/include/root_common_config.h`
- **功能**: 统一线程优先级、栈深与关键业务配置项
- **特性**:
  - 集中管理所有线程配置
  - 统一优先级定义
  - 便于平台移植和调优

### 6. 项目默认功能与参数
- **文件**: `trunk/include/project_default_features.h`
- **文件**: `trunk/include/project_default_parameter.h`
- **功能**: 汇总项目级默认开关与参数，便于产品化覆盖
- **特性**:
  - 功能开关集中管理
  - 参数默认值定义
  - 支持 `boards/project_config.h` 覆盖

### 7. 链表管理接口
- **文件**: `trunk/include/root_list_api.h`
- **功能**: 通用链表数据结构管理接口
- **特性**:
  - 双向链表操作
  - 线程安全版本
  - 统一接口封装

### 8. 广播管理接口
- **文件**: `trunk/include/broadcast_manager_api.h`
- **功能**: 系统级广播消息管理
- **特性**:
  - 事件订阅机制
  - 跨模块消息分发
  - 支持优先级过滤

## 业务管理器（Manager Layer）

### 9. UI 管理系统
- **文件**: `trunk/include/ui_manager_api.h`
- **实现**: `trunk/root_source/ui/`
- **配置**: `overlay/ui/`（版本化：v3_ui_v2~v7、v3pro_ui_v2~v7）、`overlay/ui/lang_pack/`
- **功能**: UI 界面整体管理与控制
- **特性**:
  - UI 任务调度
  - 界面刷新控制
  - 触摸事件分发
  - 与 LVGL 集成
  - 多产品多版本 UI 支持
  - 文案/控件坐标/字库运行时查表（`ui_str` / `ui_widget` / `ui_font`），优先外部语言包，无包时回退固件 default

### 10. 马达管理系统
- **文件**: `trunk/include/motor_manager_api.h`
- **实现**: `trunk/root_source/motor/`
- **配置**: `overlay/framework_hooks/motor/`
- **功能**: 马达/泵体控制核心模块，为吸奶泵提供底层驱动能力
- **特性**:
  - 马达策略管理（支持复杂的时序控制）
  - PWM 波形生成与控制
  - 压力/频率闭环调节
  - 多级策略嵌套（定时策略 + 子策略）
  - 实时参数调整
  - 马达状态监控与保护

### 11. 吸奶泵管理系统
- **文件**: `trunk/include/breast_pump_manager_api.h`
- **实现**: `trunk/root_source/breast_pump/`
- **配置**: `overlay/breast_pump/`
- **功能**: 吸奶泵业务核心管理模块
- **特性**:
  - 多种工作模式支持（共 7 种模式）
  - 压力/频率闭环控制
  - 状态机管理
  - 参数表配置化（支持定时策略与子策略组合）
  - 历史记录管理
  - 动态模式参数更新（支持自定义模式在线调整）

### 12. 输入系统管理接口
- **文件**: `trunk/include/key_input_manager_api.h`
- **实现**: `trunk/root_source/key_input/`
- **配置**: `overlay/input_key/`
- **功能**: 抽象按键/触摸输入事件，统一上报与去抖
- **特性**:
  - 按键事件统一管理
  - 支持短按/长按/连击
  - 可配置按键码映射
  - 触摸事件支持

### 13. 传感器管理接口
- **文件**: `trunk/include/sensor_manager_api.h`
- **实现**: `trunk/root_source/sensor/`
- **配置**: `overlay/sensor/`
- **功能**: 统一多类传感器的注册、轮询/中断与数据分发
- **特性**:
  - IMU 传感器（加速度/陀螺仪）
  - 力传感器/压力传感器
  - 气压传感器
  - 环境光传感器（支持自适应调光和小夜灯策略）
  - 传感器校准与滤波
  - 数据融合算法接入

### 14. 算法框架管理
- **文件**: `trunk/include/algorithm_manager_api.h`
- **实现**: `trunk/root_source/algorithm/`
- **配置**: `overlay/algo/`
- **功能**: 算法框架调度与管理
- **特性**:
  - 算法任务调度
  - 数据流管理
  - 算法链式处理

### 15. 测试模块（Tests）
- **文件**: `trunk/include/tests_manager_api.h`
- **实现**: `trunk/root_source/tests/`
- **配置**: `overlay/product_tests/`
- **功能**: 提供固件自检、产测钩子与集成用例的统一入口
- **特性**:
  - 产测命令处理
  - 自检流程
  - 测试用例框架

### 16. 电池管理系统
- **文件**: `trunk/include/battery_manager_api.h`
- **实现**: `trunk/root_source/battery/`
- **功能**: 电池状态监控、电量管理、充电控制
- **特性**:
  - 电池电量检测
  - 充电状态管理
  - 低电量保护

### 17. BLE 管理系统
- **文件**: `trunk/include/ble_manager_api.h`
- **实现**: `trunk/root_source/ble/`
- **配置**: `overlay/ble/`、`overlay/framework_hooks/ble/`
- **功能**: 蓝牙低功耗通信管理
- **特性**:
  - BLE 服务注册与管理（数据埋点/工厂/主设备/远程服务）
  - 连接状态管理
  - 广播与扫描控制

### 18. 服务器管理系统
- **文件**: `trunk/include/servers_manager_api.h`
- **实现**: `trunk/root_source/servers/`
- **配置**: `overlay/servers/`
- **功能**: 统一管理各类服务器
- **特性**:
  - 服务器注册与管理（通用/工厂/主设备/OTA/数据埋点/旧版协议）
  - 消息路由与分发
  - 协议适配层

### 19. 资源管理系统
- **文件**: `trunk/include/resource_manager_api.h`
- **实现**: `trunk/root_source/resource/`
- **配置**: `overlay/resources/`（版本化：V3 res v2-v7、V3 Pro res v2-v7）
- **功能**: 统一管理图片主题包的加载、解压与缓存（与语言包分区分离）
- **特性**:
  - 资源分包管理（Base/Custom）
  - 版本化资源发布
  - 图片缓存与解压
  - 资源初始化钩子中触发语言包 `probe`（`overlay/framework_hooks/resource/`）

### 19a. 外部语言包（FDS schema 2）
- **加载器**: `overlay/ui/lang_pack/`（v3 / v3pro 共用）
- **文案/坐标源**: `overlay/ui/<v3|v3pro>_ui_v7/lang/<zh|en>/`（`strings.h`、`layout.h`）
- **字库与打包**: `overlay/font/font_v3/`、`overlay/font/packaging_tools/`
- **开关**: `IS_SUPPORT_LANG_PACK`（`boards/project_support_overlay.h`；`make firmware IS_SUPPORT_LANG_PACK=0` 可关闭加载器与语言包 OTA）
- **功能**: 文案、控件坐标、语言相关字库、图片文字存外部 Flash `FDS_EXT`（littlefs，约 1.5MB @ `0xE00000`），可独立烧录/OTA，固件不内置 `lang_pack.bin`
- **特性**:
  - 开机有 `RDY1` 则按清单加载各 FDS key；无 ready 时走编译期 default 表
  - 公共 Aeonik Latin / 数字字库编进固件，不进语言包；中文包带 Noto 五档字号，英文包主要是页面表 + `font_img`
  - 整包容器 `*_PACK.bin` 供 OTA（`TYPE=0x04`），`*_LFS.bin` 供产线烧录；安装后拆成字号/页面文案/页面组件等多 key
  - 只改已有宏的字符串或坐标时，一般只需重打包并烧语言包，不必重编固件；新增宏需跑 gen 并重编（新 ID）
- **文档**: [`docs/architecture/ui/lang-pack-fds.md`](docs/architecture/ui/lang-pack-fds.md)、[`docs/architecture/ui/lang-edit.md`](docs/architecture/ui/lang-edit.md)、[`overlay/font/README.md`](overlay/font/README.md)

### 20. 协议管理系统
- **文件**: `trunk/include/protocol_manager_api.h`
- **实现**: `trunk/root_source/protocol/`
- **配置**: `overlay/protocol/`
- **功能**: 统一封装应用协议、通用协议与产品协议入口

### 21. 电源管理系统
- **文件**: `trunk/include/power_manager_api.h`
- **实现**: `trunk/root_source/pm/`
- **配置**: `overlay/framework_hooks/power/`
- **功能**: 负责电源状态切换、低功耗控制与系统级电源事件处理
- **特性**:
  - 插电开机引导策略
  - 适配器匹配检测
  - 低功耗模式切换

### 22. LED 管理系统
- **文件**: `trunk/include/led_manager_api.h`
- **实现**: `trunk/root_source/led/`
- **配置**: `overlay/framework_hooks/led/`
- **功能**: 统一管理指示灯、背光与灯效相关控制逻辑

### 23. OTA 升级管理系统
- **文件**: `trunk/include/ota_manager_api.h`
- **实现**: `trunk/root_source/ota/`
- **配置**: `overlay/framework_hooks/ota/`、`overlay/servers/ota_server_client/`
- **功能**: 固件空中升级管理
- **特性**:
  - 压缩 OTA 包支持
  - 升级状态管理
  - 断点续传
  - 语言包 OTA 复用 Service 9 原子 TLV，`TYPE=0x04`，进度存独立 `STORE_ID_LANG_PACK_OTA_INFO`；包体为 schema 2 容器，写入 `FDS_EXT` 后拆 key

### 24. 异常检测管理系统 (Abnormal Manager)
- **文件**: `trunk/include/abnormal_manager_api.h`
- **实现**: `trunk/root_source/abnormal/`
- **配置**: `overlay/framework_hooks/abnormal/`
- **功能**: 系统异常检测与处理
- **特性**:
  - 硬件异常监控
  - 运行时异常捕获
  - 异常恢复策略

### 25. 数据埋点管理系统 (Data Bury Manager)
- **文件**: `trunk/include/data_bury_manager_api.h`
- **实现**: `trunk/root_source/data_bury/`
- **配置**: `overlay/data_bury/`、`overlay/framework_hooks/data_bury/`
- **功能**: 用户行为与设备数据采集、上报
- **特性**:
  - 事件采集与缓存
  - 协议适配与打包
  - 数据上传管理
  - BLE 透传服务支持

## 吸奶泵工作模式详解

吸奶泵管理系统是业务层模块，负责工作模式、状态管理、历史记录等高层业务逻辑。它依赖于马达管理系统提供的底层驱动能力来实现实际的泵体控制。

**系统分层关系**:
```
吸奶泵管理系统 (breast_pump_manager)
    ↓ 调用
马达管理系统 (motor_manager)
    ↓ 控制
马达驱动 (drivers/motor)
    ↓ 输出
硬件泵体
```

### 模式架构设计
吸奶泵系统基于策略模式设计，每种工作模式由以下层次结构组成：
- **总策略管理器** (`motor_policy_manager_t`): 管理整个模式的生命周期
- **定时策略节点** (`motor_timed_policy_node_t`): 定义阶段性的时间分段
- **子策略节点** (`motor_policy_node_t`): 具体的马达控制参数（压力、频率、持续时间等）

策略配置通过 `overlay/breast_pump/table/` 中的参数表定义，在运行时由马达管理系统执行。

### 七种标准工作模式

#### 1. 刺激模式 (Stimulation Mode)
- **模式 ID**: `BREAST_PUMP_STIMULATION_MODE`
- **用途**: 模拟婴儿初始吸吮动作，快速频率刺激乳腺，促进泌乳反射
- **特点**: 高频率、低压力、短周期
- **工作时长**: 标准时长（可配置）
- **适用场景**: 吸乳前的预备阶段，帮助乳汁快速流出

#### 2. 吸乳模式 (Pump Mode)
- **模式 ID**: `BREAST_PUMP_LACTATION_MODE`
- **用途**: 标准的吸乳工作模式，模拟婴儿深度吸吮
- **特点**: 中等频率、可调压力、长周期
- **工作时长**: 标准时长（可配置）
- **适用场景**: 主要吸乳阶段，适合常规使用

#### 3. 混合模式 (Mix Mode)
- **模式 ID**: `BREAST_PUMP_MIX_MODE`
- **用途**: 结合刺激和吸乳两种模式的优势
- **特点**: 先快速刺激后深度吸乳，自动切换
- **工作时长**: 标准时长（可配置）
- **适用场景**: 一键完成刺激到吸乳的完整流程

#### 4. 开奶模式 (Lactation Start Mode)
- **模式 ID**: `BREAST_PUMP_LACTATION_START_MODE`
- **用途**: 专为产后初期（产后 1-3 天）设计，帮助建立泌乳
- **特点**: 温和压力、特殊节奏、延长工作时间
- **工作时长**: 延长时长（通常 20 分钟，由 `LACTATION_START_MODE_WORK_TIME` 定义）
- **适用场景**: 产后开奶期，需要更长时间和特殊节奏刺激乳腺

#### 5. 个性化模式 (Custom Mode)
- **模式 ID**: `BREAST_PUMP_CUSTOM_MODE`
- **用途**: 用户可根据自身情况自定义参数
- **特点**:
  - 支持动态参数更新（通过 `custom_mode_parameter_update()`）
  - 可调整压力、频率、持续时间等所有参数
  - 工作时长根据配置的子策略动态计算
- **工作时长**: 动态计算（所有子策略持续时间总和）
- **适用场景**: 特殊需求用户，或通过 APP 学习用户习惯后自动生成

#### 6. PP 追奶模式 (PP Milk Track Mode)
- **模式 ID**: `BREAST_PUMP_PP_MILK_TRACK_MODE`
- **用途**: 专为追奶场景设计，增加泌乳量
- **特点**: 特殊的频率和压力组合，优化追奶效果
- **工作时长**: 标准时长（可配置）
- **适用场景**: 奶量不足的妈妈，需要增加泌乳量

#### 7. 2+8 奶量提升模式 (Milk Boost Mode)
- **模式 ID**: `BREAST_PUMP_MILK_BOOST_MODE`
- **用途**: 通过科学的时间分配提升泌乳量
- **特点**: 2 分钟刺激 + 8 分钟吸乳的黄金组合
- **工作时长**: 固定 10 分钟（由 `MILK_BOOST_MODE_WORK_TIME` 定义）
- **适用场景**: 追求高效吸乳，时间有限的场景

### 模式参数配置
- **配置文件位置**: `overlay/breast_pump/table/pump_mode_default_config.h`
- **参数类型**:
  - `motor_policy_cfg_t`: 单个策略配置（压力、频率、持续时间、循环次数等）
  - 支持 `MOTOR_TIME_POLICY` 类型（定时策略）和子策略的嵌套组合
- **配置示例**: 每种模式有对应的默认参数数组（如 `stimulation_default[]`、`pump_default[]` 等）

### 模式管理接口
```c
// 获取模式参数
const motor_policy_cfg_t *get_breast_pump_mode_para(int mode);

// 获取模式工作时长（秒）
int breast_pump_mode_work_time_get(int mode);

// 模式策略初始化
motor_policy_manager_t *breast_pump_mode_parameter_init(
    const motor_policy_cfg_t *cfg,
    uint8_t cfg_num,
    char *name
);

// 模式策略释放
int breast_pump_mode_parameter_deinit(motor_policy_manager_t *obj);

// 自定义模式参数更新（仅限 Custom Mode）
int custom_mode_parameter_update(motor_policy_cfg_t *cfg, uint8_t cfg_num);
```

### 模式扩展指南
要添加新的工作模式，需要：
1. 在模式枚举中添加新的模式 ID
2. 在 `overlay/breast_pump/table/` 中定义参数表
3. 在 `overlay/breast_pump/mode/` 中创建模式文件（参考现有模式）
4. 在 `breast_pump_mode.c` 的 `get_breast_pump_mode_para()` 中添加分支
5. 在 `breast_pump_mode_work_time_get()` 中设置工作时长
6. 在业务层注册新模式

## Overlay 配置系统详解

`overlay/` 目录是框架的配置层，采用**版本化**策略管理 UI 与资源，不同产品/版本可通过覆盖该目录实现差异化配置。

### Overlay 目录结构与用途

#### 1. `overlay/ui/` - UI 实现（版本化）
- **结构**: `v3_ui_v2` ~ `v3_ui_v7`（V3 产品）、`v3pro_ui_v2` ~ `v3pro_ui_v7`（V3 Pro 产品）
- **公共加载器**: `overlay/ui/lang_pack/`（外部语言包 probe / 安装 / 运行时查表）
- **每个版本包含**:
  - 业务页面实现：`pump_home/`、`home/`、`charge/`、`power_off/`、`ota/`、`guide/`、`setting/` 等
  - 弹窗与提示：`page_popup/`、`top_popup/`、`popup_windows/`
  - 系统页面：`logo/`、`airplane_mode/`、`factory_reset/`、`system_abnormal/`、`low_power/`
  - UI 特性：`home_night_light/`（小夜灯）、`home_idle/`（空闲态）、`charge_adapter/`（适配器检测）
  - 控件组件：`progress_arc/`（进度弧）、`snapshot/`（快照）、`wave/`（波形动画）
  - `ui_img_define.h` / `ui_page_define.h` - 图片与页面定义
  - v7 `lang/`：`zh|en/strings.h`、`layout.h` 为源宏；`gen_ui_lang_tables.py` 生成 ID 表；页面用 `ui_str` / `ui_widget` / `ui_layout_i16`，不要手改生成头

#### 2. `overlay/windows/` - UI 窗口资源（版本化）
- **结构**: `windows_v1` ~ `windows_v6`
- **每个版本包含**: `airplane_mode/`、`battery/`、`breast_pump/`、`charge/`、`factory_reset/`、`guide/`、`home/`、`logo/`、`ota/`、`popup_windows/`、`power_off/`、`product_test_window/`、`setting/`、`system_abnormal/`、`theme_guide/`
- **窗口注册**: 通过 `windows_map_define.c/h` 管理窗口映射与路由

#### 3. `overlay/resources/` - 图片主题资源（版本化）
- **结构**: `v3_res_v2` ~ `v3_res_v7`（V3 产品）、`v3pro_res_v2` ~ `v3pro_res_v7`（V3 Pro 产品）
- **每个版本包含**:
  - `base/` - 基础资源包（baj 图片 + png 源文件）
  - `custom/` - 定制资源包
  - `resource_id_define.h` / `resource_info.h` - 资源 ID 与信息定义
- **说明**: 图片主题包与语言包分开；语言文案/字库走 `overlay/font/` + `FDS_EXT`

#### 3a. `overlay/font/` - 字体与语言包
- **packaging_tools/**: `pack.sh` 产出 `*_LFS.bin`（烧录）与 `*_PACK.bin`（OTA）；`preview.sh --open` 圆屏预览
- **font_v3/common/**: Aeonik 19/22/26/29/33/56/69，编进固件
- **font_v3/lang/<zh|en>/**: 语言相关 LVGL `.bin` 字库与 `font_img`
- **产物**: `overlay/font/bin/font_v3/<lang>/`；烧 LFS 到外部 Flash `0xE00000`，长度 `0x180000`
- **依赖**: `python3 -m pip install --user littlefs-python`（或 `./preview.sh --install`）

#### 4. `overlay/breast_pump/` - 吸奶泵业务配置
- **mode/**: 工作模式定义，支持 7 种标准模式
- **table/**: 参数表配置（压力曲线、频率表、时间配置、定时策略等）

#### 5. `overlay/framework_hooks/` - 框架钩子（17 个模块）
- `abnormal/` - 异常检测钩子
- `airplane_mode/` - 飞行模式状态切换钩子
- `algo/` - 算法模块初始化与回调钩子
- `ble/` - 蓝牙模块初始化与事件钩子
- `breast_pump/` - 吸奶泵业务钩子（模式切换、状态回调、韵律提醒等）
- `data_bury/` - 数据埋点事件采集钩子
- `input_key/` - 按键事件处理钩子
- `led/` - 指示灯与背光控制钩子
- `motor/` - 马达控制钩子（PWM 调整、反馈处理等）
- `ota/` - OTA 升级生命周期钩子
- `power/` - 电源状态迁移与低功耗策略钩子（适配器匹配、插电引导等）
- `product_tests/` - 产测与自检流程钩子
- `sensor/` - 传感器数据处理钩子
- `servers/` - 服务器生命周期与路由钩子
- `resource/` - 资源初始化钩子（`on_lang_res_init` → `ui_lang_pack_probe`）
- `storage/` - 存储初始化与配置钩子
- `ui/` - UI 初始化、事件转发与页面联动钩子

#### 6. `overlay/ble/` - BLE 服务端实现
- `data_bury_service/` - 数据埋点 BLE 透传服务
- `factory_service/` - 工厂测试 BLE 服务
- `master_service/` - 主设备通信 BLE 服务
- `remot_service/` - 远程控制 BLE 服务

#### 7. `overlay/data_bury/` - 数据埋点事件处理
- **用途**: 定义埋点事件类型、采集规则、上传策略

#### 8. `overlay/night_light/` - 小夜灯策略
- **用途**: 小夜灯亮度、触发条件、时长等策略控制

#### 9. `overlay/adaptive_dimming/` - 自适应调光
- **用途**: 根据环境光自动调整屏幕亮度

#### 10. `overlay/input_key/` - 输入按键配置
- **用途**: 按键码定义与映射

#### 11. `overlay/sensor/` - 传感器配置
- `air_pressure/`、`force_sensor/`、`imu/`、`light_sensor/`
- **用途**: 不同批次或型号的传感器校准参数

#### 12. `overlay/storage/` - 存储分区配置
- `breast_pump/`、`factory/`、`resource/`、`system/`
- **用途**: 定义各类数据的存储结构和分区布局；语言包 OTA 断点为独立 persist（`STORE_ID_LANG_PACK_OTA_INFO`），不要复用图片资源 OTA store

#### 13. `overlay/algo/` - 算法处理实现
- `root_algo_acq_proc.c/h`、`root_algo_core.c/h`、`root_algo_post_proc.c/h`
- **用途**: 产品特定的算法处理实现，与 `library/root_algo/` 配合

#### 14. `overlay/protocol/` - 协议处理
- `app_old_commication_protocol/`、`common_protocl/`
- **用途**: 产品特定的协议实现

#### 15. `overlay/servers/` - 服务器实现
- `app_old_protocol_server/`、`common_server/`、`data_bury_protocol_server_client/`、`data_bury_server/`、`factory_server_client/`、`master_server_client/`、`ota_server_client/`
- **用途**: 产品特定的服务器实现，由 `trunk/root_source/servers/` 统一管理

#### 16. `overlay/product_tests/` - 产品测试
- **用途**: 工厂测试、硬件自检、通信测试

### Overlay 使用原则

1. **框架主干不变**: `trunk/` 目录中的代码应保持通用性，不包含产品特定逻辑
2. **版本化资源管理**: UI、窗口、资源均按版本号组织（v1-v7），支持多版本共存与独立发布
3. **配置优先覆盖**: `overlay/` 中的配置会覆盖框架默认配置
4. **钩子注入逻辑**: 通过 `framework_hooks/` 注入产品特定的业务逻辑
5. **资源分离管理**: UI 图片主题、字体、语言包与代码分离；语言包可独立烧录/OTA，不必每次改文案都发整包固件
6. **多产品支持**: 不同产品（V3、V3 Pro）使用各自的 UI 和资源版本，实现一套框架支持多产品线

## 驱动支持

| 驱动 | 目录 | 功能 |
|------|------|------|
| ADC | `drivers/adc/` | 模拟量采样，面向传感器读取与校准 |
| 气压传感器 | `drivers/air_pressure/` | 气压/海拔传感器适配与读数 |
| 音频 | `drivers/audio/` | 音频编解码/接口驱动 |
| 基础抽象 | `drivers/base/` | 基础硬件抽象/通用寄存器封装 |
| 电池 | `drivers/battery/` | 电池/电量计/充电芯片 |
| BLE | `drivers/ble/` | 低功耗蓝牙控制器适配 |
| 通用工具 | `drivers/common/` | 驱动通用工具/适配层 |
| 数据库 | `drivers/database/` | 简易 KV/参数存储 |
| FATFS | `drivers/fatfs/` | FATFS 文件系统适配 |
| Flash | `drivers/flash/` | 外部/内部 Flash 访问 |
| 力传感器 | `drivers/force_sensor/` | 力传感器/压力传感器 |
| I2C | `drivers/i2c/` | 通用 I2C 总线驱动/适配 |
| IMU | `drivers/imu/` | 惯性传感器（加速度计/陀螺仪/融合） |
| 按键 | `drivers/key/` | 物理按键/触摸按键扫描 |
| LCD | `drivers/lcd/` | 显示屏/显示控制器 |
| LED | `drivers/led/` | 指示灯/背光控制 |
| 光传感器 | `drivers/light_sensor/` | 环境光传感器驱动，支持自适应调光 |
| 马达 | `drivers/motor/` | 振动马达/线性马达驱动 |
| UART | `drivers/uart/` | 串口/UART 驱动 |
| WiFi | `drivers/wifi/` | WiFi 驱动支持 |

## 外部库集成

| 库 | 目录 | 功能 |
|----|------|------|
| LVGL | `trunk/external/lvgl/` | 轻量级嵌入式图形界面库 |
| LittleFS | `trunk/external/littlefs/` | 嵌入式轻量级文件系统 |
| cJSON | `trunk/external/cJSON/` | 轻量级 JSON 解析与生成 |
| Crypto | `trunk/external/crypto/` | 加解密/哈希/安全库 |
| MD5 | `trunk/external/md5/` | MD5 哈希算法 |
| Base64 | `trunk/external/base64/` | Base64 编解码 |
| QuickLZ | `trunk/external/quicklz/` | 快速资源压缩/解压 |
| Scan Key | `trunk/external/scan_key/` | 按键扫描库 |

## 算法库

### 根算法库（Root Algo）
- **目录**: `library/root_algo/`
- **功能**: 核心算法实现库
- **特性**:
  - 算法融合处理（`root_algo_fusion.c`）
  - 穿戴检测（`wear_detecttion/`）
  - 环形缓冲区（`ring_buffer/`）
  - 算法处理钩子（`overlay/algo/`）：采集处理、核心处理、后处理

## 开发辅助工具

### 构建辅助脚本 (build_helper.sh)
- **位置**: `build_helper.sh`
- **模式**: 交互模式 / 非交互模式（CI 适用）
- **子命令**: `build-firmware`、`build-only`、`clean-firmware`、`build-bootloader`、`copy`、`bear`、`rebuild-firmware` 等
- **功能**: 一键编译/清理、生成 OTA 包、拷贝产物、归档命名

### Python 分析脚本
- **目录**: `scripts/`
- **工具列表**:
  - `map_section_sizes.py` - ELF 文件段大小分析
  - `ram_usage_analyzer.py` - RAM 使用量分析（含文档 `RAM_ANALYZER_USAGE.md`）
  - `psram_usage_analyzer.py` - PSRAM 使用量分析（含文档 `PSRAM_ANALYZER_USAGE.md`）
  - `PNG_TO_TXT_USAGE.md` - PNG 转 TXT 工具使用文档

### BLE 工厂工具
- **目录**: `tools/ble_factory_tool/`
- **功能**: BLE 工厂测试与校准工具

## 体系结构（逻辑分层）

- **上层调用下层**，唯一与 RTOS 直接交互的应在平台/OS 抽象处（若集成 RTOS）
- **UI/业务 → 管理器/服务 → 平台抽象/驱动 → 芯片平台/硬件**
- 通过 `overlay/` 实现产品差异化与资源替换，避免改动 `trunk/` 主干代码
- 版本化策略：同产品不同迭代版本（v1-v7）可并行存在于 overlay，便于回滚与多版本维护

## 开发规范

### API 设计原则
- API 面向 `trunk/include/` 对外暴露，内部实现放入 `trunk/root_source/`
- 芯片相关改动位于 `trunk/platform_chip/`，避免渗透到上层逻辑
- 资源与产品差异通过 `overlay/` 覆盖，引入统一初始化钩子
- 构建仅修改根目录 `GNUmakefile` 与 `boards/*.h` 聚合配置
- 模块命名统一使用 `*_manager_api.h` 形式对外暴露接口

### 依赖约束
- `trunk/include` 被上层直接引用；实现仅在 `root_source/`
- `drivers` 不依赖上层业务；仅向上提供稳定接口
- `platform_chip` 仅被 `drivers` 与 `root_source` 适配层使用
- `overlay` 可覆盖资源与钩子，但不应直接依赖底层私有实现细节
- `library` 独立于框架，提供通用算法能力

## 构建说明

### 仓库初始化与 SDK 子仓库更新
- 本工程依赖 Git 子仓库 `trunk/platform_chip/bes`，首次克隆后请先初始化同步，再开始编译。

```bash
# 首次克隆（推荐）
git clone --recurse-submodules <repo-url>

# 已克隆但未初始化子仓库（检出主仓锁定的 SDK commit，不是分支最新）
git submodule update --init --recursive

# 当主仓更新了 SDK 指针后，同步到主仓锁定版本
git submodule sync --recursive
git submodule update --init --recursive trunk/platform_chip/bes

# 主动拉取 .gitmodules 中 branch（当前为 develop）的远端最新提交
git submodule update --remote --recursive trunk/platform_chip/bes
git add trunk/platform_chip/bes
```

- `git submodule update --init --recursive` 只跟随主仓 gitlink 记录的 commit SHA，**不会**自动切到 `develop` 最新。
- `.gitmodules` 里 `branch = develop` 只在带 `--remote` 时生效：`git submodule update --remote` 会把子仓更新到 `origin/develop` 尖端。
- 升级 SDK 后请检查差异，并把主仓中的子仓库指针一并提交，否则其他人执行不带 `--remote` 的 update 仍会回到旧 SHA。

### 构建入口
- 在根目录直接使用 `GNUmakefile` 或 `build_helper.sh` 进行编译与打包
- 产品特定配置位于 `configs/` 目录（如 `V3_2710IMC_GNUmakefile`、`V3_2710IMP_GNUmakefile`）
- 产物默认输出到 `project/bin/firmware/`，中间文件位于 `project/build/`

### 编译命令
```bash
# 编译特定目标
make T=best1306p -j16

# 使用产品特定配置编译
make -f configs/V3_2710IMP_GNUmakefile T=best1306p -j16

# 清理构建
make clean

# 关闭外部语言包（对比固件体积 / 仅用固件 default 表）
make firmware IS_SUPPORT_LANG_PACK=0 -j16

# 查看帮助
make help
```

### 语言包打包与预览
源宏只改 `overlay/ui/<v3|v3pro>_ui_v7/lang/<zh|en>/` 的 `strings.h` / `layout.h`（en/zh 宏名必须相同；新宏追加在文件末尾）。不要手改 `ui_str_id.h`、`ui_widget_id.h`、`ui_layout_id.h`、`ui_lang_bind.h`、`ui_lang_default.c`。

```bash
# 新增宏后生成 ID 表（只改正文/坐标不必跑）
python3 overlay/ui/v3_ui_v7/lang/tools/gen_ui_lang_tables.py

# 打包（按 GNUmakefile 产品族，默认中文）
cd overlay/font/packaging_tools
./pack.sh
./pack.sh --ui v3pro -l zh
./preview.sh --open
```

对照表：`overlay/ui/<ui>_ui_v7/lang/ui_lang_id_map.html`。详细约定见 [`docs/architecture/ui/lang-edit.md`](docs/architecture/ui/lang-edit.md)。

### build_helper.sh 使用与功能

- **位置**: `build_helper.sh`
- **作用**: 一键编译/清理、生成编译数据库（bear）、生成 OTA、拷贝到共享目录、按产品信息重命名归档。

#### 交互模式（推荐日常使用）
- **启动**: `./build_helper.sh`
- **菜单项**（数字对应脚本内功能）:
  - 1: 编译固件（目标: `firmware`），并拷贝产物
  - 2: 清理固件构建目录
  - 3: bear -- make（生成 `compile_commands.json`，会先清理固件构建目录）
  - 4: 编译 Bootloader（目标: `bootloader`），并拷贝产物
  - 5: 清理 Bootloader 构建目录
  - 6: 生成压缩 OTA（`*_ota.bin`），并尝试拷贝
  - 7: 仅拷贝当前可用产物到共享目录（固件/Bootloader/OTA）
  - 8: 依据 `boards/product_info.h` 重命名并归档 APP/BOOT/OTA

#### 非交互模式（适合 CI 或脚本集成）
- **用法**: `./build_helper.sh --non-interactive <subcmd>`
- **子命令**:
  - `build-firmware`: 编译固件并拷贝产物
  - `build-only`: 仅编译固件（不拷贝）
  - `rebuild-firmware`: 先清理后编译固件，并拷贝产物
  - `clean-firmware`: 清理固件构建目录
  - `build-bootloader`: 编译 Bootloader 并拷贝产物
  - `build-bootloader-only`: 仅编译 Bootloader（不拷贝）
  - `clean-bootloader`: 清理 Bootloader 构建目录
  - `copy`: 仅拷贝当前产物
  - `bear`: 使用 bear 生成编译数据库

示例:
```bash
# 交互式
./build_helper.sh

# 非交互：编译固件并拷贝
./build_helper.sh --non-interactive build-firmware

# 非交互：仅生成编译数据库
./build_helper.sh --non-interactive bear
```

#### 产物与共享目录
- **默认共享目录**: `/mnt/hgfs/ubuntu_share/v3_firmware`（脚本内变量 `SHARE_DIR`）。
- **拷贝内容**: `.bin/.elf/.map`（不拷贝 `.lst`）。
- **固件输出目录**: `project/bin/firmware/`；**Bootloader 输出目录**: `project/bin/bootloader/`。
- **目标前缀**: 固件 `ble_only_lt`，Bootloader `ota_copy`（由内部逻辑与 Make 变量解析得到）。

#### OTA 生成
- 从 `project/bin/firmware/ble_only_lt.bin` 生成压缩 OTA：`ble_only_lt_ota.bin`。
- 依赖脚本: `trunk/platform_chip/bes/sdk/tools/build_compressed_ota.sh`。

#### 归档与命名规则
- 读取 `boards/product_info.h`：`ROOT_PRODUCT_ID`、`ROOT_FIRMWARE_TYPE`、`ROOT_RES_MAJOR_VERSION/ROOT_RES_MINOR_VERSION` 与 git 短哈希。
- 生成目录与文件名模式：
  - 目录: `${DEVICE}_${TYPE}${MAJOR}_${MINOR}`（示例: `V3_P01_01`）
  - 文件: `${DEVICE}_${DATE}_${TYPE}${MAJOR.MINOR}_${GIT}_{APP|BOOT|OTA}.bin`

备注：若需修改共享目录或默认目标等，可直接调整脚本顶部的 `SHARE_DIR` 与 `MAKE_ARGS`。

## 版本与命名

- 版本号遵循 `${PRODUCT}_V${MAJOR}.${MINOR}` 格式
- 外部 API 使用 `*_manager_api.h` 统一命名
- UI 与图片资源采用版本化目录管理（v1-v7），支持多代产品并行维护
- 语言包 schema major = 2；与图片主题 OTA 分区、store 键分离
- 新增接口遵循现有命名规范，保持一致性

---

**注意**: 仅允许向下依赖（上层只能依赖同级或更底层模块），禁止跨层倒置依赖。
