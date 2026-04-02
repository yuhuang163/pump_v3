# Smart Wearable Firmware Framework (v3)

基于模块化与分层设计的智能穿戴固件工程框架。

## 目录结构

```
FIRMWARE_V3/
├── GNUmakefile                      ← 构建入口（目标/芯片/配置）
├── build_helper.sh                  ← 构建辅助脚本
├── scripts/                         ← 根目录脚本（构建/发布/工具）
├── docs/                            ← 项目文档与说明
├── compile_commands.json            ← 编译数据库（IDE/静态分析）
├── configs/                         ← 产品特定的Makefile配置
│   ├── V3_2710IMC_GNUmakefile      ← V3_2710IMC产品配置
│   └── V3_2710IMP_GNUmakefile      ← V3_2710IMP产品配置
├── library/                         ← 算法库与通用库
│   └── root_algo/                   ← 根算法库（融合算法/穿戴检测等）
├── boards/                          ← 工程级项目/产品配置入口（头文件集中）
│   ├── product_info.h               ← 产品信息与型号定义
│   ├── project_config.h             ← 编译/功能开关/宏定义（顶层聚合）
│   ├── project_log_filter.h         ← 模块化日志过滤策略
│   ├── project_partitions.h         ← 分区/存储布局声明
│   └── project_support_overlay.h    ← 叠加配置（覆盖默认项）
│
├── drivers/                         ← 板级与设备驱动（弱RTOS依赖）
│   ├── audio/                       ← 音频编解码/接口驱动
│   ├── base/                        ← 基础硬件抽象/通用寄存器封装
│   ├── battery/                     ← 电池/电量计/充电芯片
│   ├── ble/                         ← 低功耗蓝牙控制器相关适配
│   ├── common/                      ← 驱动通用工具/适配层
│   ├── database/                    ← 简易KV/参数存储（如有）
│   ├── fatfs/                       ← FATFS 文件系统适配
│   ├── flash/                       ← 外部/内部Flash访问
│   ├── force_sensor/                ← 力传感器/压力传感器
│   ├── i2c/                         ← 通用I2C总线驱动/适配
│   ├── imu/                         ← 惯性传感器（加计/陀螺/融合）
│   ├── key/                         ← 物理按键/触摸按键扫描
│   ├── lcd/                         ← 显示屏/显示控制器
│   ├── led/                         ← 指示灯/背光控制
│   ├── motor/                       ← 振动马达/线性马达驱动
│   ├── adc/                         ← 模数转换（ADC）
│   ├── air_pressure/                ← 气压传感器/气压计
│   ├── light_sensor/                ← 光传感器驱动
│   ├── uart/                        ← 串口/UART驱动
│   └── wifi/                        ← WiFi驱动支持
│
├── overlay/                         ← 资源与钩子（工程可重写/定制）
│   ├── adaptive_dimming/            ← 自适应调光配置
│   ├── algo/                        ← 算法处理实现（采集/核心/后处理）
│   ├── breast_pump/                 ← 吸奶泵业务配置（模式/参数表）
│   │   ├── mode/                    ← 吸奶泵工作模式配置
│   │   └── table/                   ← 吸奶泵参数表
│   ├── font/                        ← 字体资源
│   ├── framework_hooks/             ← 框架钩子（任务/事件/日志等）
│   │   ├── airplane_mode/           ← 飞行模式钩子
│   │   ├── algo/                    ← 算法模块钩子
│   │   ├── ble/                     ← 蓝牙模块钩子
│   │   ├── breast_pump/             ← 吸奶泵业务钩子
│   │   ├── input_key/               ← 输入按键钩子
│   │   ├── led/                     ← 指示灯/背光钩子
│   │   ├── motor/                   ← 马达控制钩子
│   │   ├── power/                   ← 电源管理钩子
│   │   ├── product_tests/           ← 产测与自检钩子
│   │   ├── sensor/                  ← 传感器处理钩子
│   │   ├── servers/                 ← 服务器框架钩子
│   │   ├── storage/                 ← 存储配置钩子
│   │   └── ui/                      ← UI初始化与事件钩子
│   ├── input_key/                   ← 输入按键码定义与映射
│   ├── product_tests/               ← 产品测试（工厂测试/硬件自检/通信）
│   ├── protocol/                    ← 协议处理（应用协议/通用协议）
│   ├── resources/                   ← 图片/语言包/二进制资源
│   ├── servers/                     ← 服务器实现（应用协议服务器/通用服务器/工厂服务器等）
│   ├── sensor/                      ← 传感器配置与校准参数
│   │   ├── air_pressure/            ← 气压传感器配置
│   │   ├── force_sensor/            ← 力传感器配置
│   │   ├── imu/                     ← IMU传感器配置
│   │   └── light_sensor/            ← 环境光传感器配置
│   ├── storage/                     ← 存储分区配置与持久化定义
│   │   ├── breast_pump/             ← 吸奶泵业务存储配置
│   │   ├── factory/                 ← 工厂校准数据存储
│   │   ├── resource/                ← 资源存储配置
│   │   └── system/                  ← 系统配置存储
│   ├── ui/                          ← UI实现（LCD直接UI/UART UI等）
│   └── windows/                     ← UI窗口/页面资源与布局
│       ├── airplane_mode/           ← 飞行模式窗口
│       ├── battery/                 ← 电池状态/电量展示窗口
│       ├── breast_pump/             ← 吸奶泵业务相关窗口
│       ├── charge/                  ← 充电过程/满电提示窗口
│       ├── guide/                   ← 引导/教学流程窗口
│       ├── home/                    ← 主页/主屏窗口
│       ├── logo/                    ← 开机Logo/品牌展示
│       ├── ota/                     ← OTA升级流程窗口
│       ├── popup_windows/           ← 通用弹窗集合（提示/确认/错误等）
│       ├── product_test_window/     ← 产测窗口
│       ├── setting/                 ← 设置/选项类窗口
│       ├── system/                  ← 系统状态与系统级页面
│       └── dummy/                   ← 占位/示例窗口（可根据产品裁剪）
│
├── project/                         ← 构建产物与构建系统文件
│
└── trunk/                           ← 框架主干（平台与核心业务）
    ├── include/                     ← 对外API头文件（管理器/服务接口）
    │   ├── airplane_mode_api.h      ← 飞行模式管理接口
    │   ├── algorithm_manager_api.h  ← 算法框架管理接口
    │   ├── audio_manager_api.h      ← 音频管理接口
    │   ├── battery_manager_api.h    ← 电池管理接口
    │   ├── ble_manager_api.h        ← BLE管理接口
    │   ├── breast_pump_manager_api.h ← 吸奶泵管理接口
    │   ├── broadcast_manager_api.h  ← 广播管理接口
    │   ├── handler_post.h           ← 任务/消息派发接口
    │   ├── hang_detect.h            ← 卡死检测/线程看门狗
    │   ├── key_input_manager_api.h  ← 输入系统管理接口
    │   ├── led_manager_api.h        ← LED管理接口
    │   ├── misc_manager_api.h       ← 杂项能力管理接口
    │   ├── motor_manager_api.h      ← 马达管理接口
    │   ├── ota_manager_api.h        ← OTA升级管理接口
    │   ├── peripheral_bus_api.h     ← 外设总线接口
    │   ├── power_manager_api.h      ← 电源管理接口
    │   ├── project_default_features.h ← 默认功能开关
    │   ├── project_default_parameter.h ← 默认参数定义
    │   ├── protocol_manager_api.h   ← 协议管理接口
    │   ├── resource_manager_api.h   ← 资源管理接口
    │   ├── root_common.h            ← 通用接口集合
    │   ├── root_common_api.h        ← 通用接口集合（已统一命名）
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
    │   ├── ui_manager_api.h         ← UI管理接口
    │   └── windows_manager_api.h    ← 窗口管理接口
    │
    ├── root_source/                 ← 核心子系统实现（面向上层的服务）
    │   ├── airplane_mode/           ← 飞行模式
    │   ├── algorithm/               ← 算法框架/调度
    │   ├── audio/                   ← 音频子系统
    │   ├── base/                    ← 基础设施（初始化/错误码/工具）
    │   ├── battery/                 ← 电池管理子系统
    │   ├── ble/                     ← BLE主干（Host/Profile/Manager）
    │   ├── breast_pump/             ← 吸奶泵业务模块（模式/控制/状态管理）
    │   ├── hal/                     ← 面向上层的硬件抽象
    │   ├── key_input/               ← 输入系统（按键/触摸）
    │   ├── led/                     ← LED/背光控制子系统
    │   ├── misc/                    ← 杂项能力（时间/随机数/配置）
    │   ├── motor/                   ← 马达子系统
    │   ├── network/                 ← 网络与协议
    │   ├── ota/                     ← 升级子系统
    │   ├── pm/                      ← 电源管理/低功耗策略
    │   ├── protocol/                ← 应用/产测/私有协议
    │   ├── resource/                ← 资源管理/解压/缓存
    │   ├── sensor/                  ← 传感器封装（IMU/FSensor/气压汇聚）
    │   ├── servers/                 ← 服务器管理子系统
    │   ├── storage/                 ← 存储抽象（KV/FS/分区）
    │   ├── tests/                   ← 自检/产测/集成测试模块
    │   ├── ui/                      ← UI管理子系统（界面控制/交互）
    │   ├── utils/                   ← 通用工具集
    │   └── windows/                 ← UI窗口框架/页面管理
    │
    ├── platform_chip/               ← 芯片平台相关适配层
    │   └── bes/                     ← BES 平台移植/驱动对接
    │       ├── os_base/             ← 操作系统基础抽象
    │       └── sdk/                 ← BES SDK适配
    │
    ├── external/                    ← 外部组件/第三方库
    │   ├── base64/                  ← Base64编解码库
    │   ├── cJSON/                   ← JSON解析库
    │   ├── crypto/                  ← 加解密/哈希/安全库
    │   ├── littlefs/                ← 嵌入式文件系统（轻量级FS）
    │   ├── lvgl/                    ← 图形界面库（Light and Versatile Graphics Library）
    │   ├── md5/                     ← MD5哈希算法库
    │   ├── quicklz/                 ← 资源压缩/解压库
    │   └── scan_key/                ← 按键扫描库
    │
    ├── simulator/                   ← 仿真/PC侧开发调试（如有）
    ├── doc/                         ← 文档/设计说明/接口约定
    └── tools/                       ← 开发/发布工具脚本
```

## 核心功能特性

### 1. 外设总线系统 (Peripheral Bus)
- **文件**: `trunk/include/peripheral_bus_api.h`
- **功能**: 统一的外设设备管理接口，支持设备注册、状态管理、操作封装
- **特性**:
  - 设备ID枚举管理
  - 设备状态跟踪（初始化/挂起/恢复）
  - 标准化的设备操作接口（open/close/read/write/ioctl）
  - 宏封装简化设备操作

### 2. 增强日志系统
- **文件**: `trunk/include/root_log_define.h`、`boards/project_log_filter.h`
- **功能**: 模块化日志过滤和彩色输出支持
- **特性**:
  - 多级日志级别（ERROR/WARN/INFO/DEBUG/VERBOSE）
  - 模块化日志过滤（支持24个功能模块独立过滤）
  - 彩色日志输出支持
  - 时间戳和线程ID支持

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
  - 支持`boards/project_config.h`覆盖

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

### 9. UI管理系统
- **文件**: `trunk/include/ui_manager_api.h`
- **实现**: `trunk/root_source/ui/`
- **功能**: UI界面整体管理与控制
- **特性**:
  - UI任务调度
  - 界面刷新控制
  - 触摸事件分发
  - 与LVGL集成

### 10. 马达管理系统
- **文件**: `trunk/include/motor_manager_api.h`
- **实现**: `trunk/root_source/motor/`
- **配置**: `overlay/framework_hooks/motor/`
- **功能**: 马达/泵体控制核心模块，为吸奶泵提供底层驱动能力
- **特性**:
  - 马达策略管理（支持复杂的时序控制）
  - PWM波形生成与控制
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
  - 多种工作模式支持（共7种模式）:
    - **刺激模式** (Stimulation Mode): 快速频率刺激，促进泌乳反射
    - **吸乳模式** (Pump Mode): 标准吸乳工作模式
    - **混合模式** (Mix Mode): 刺激+吸乳组合模式
    - **开奶模式** (Lactation Start Mode): 专为哺乳初期设计
    - **个性化模式** (Custom Mode): 用户可自定义参数的模式
    - **PP追奶模式** (PP Milk Track Mode): 专为追奶场景优化
    - **2+8奶量提升模式** (Milk Boost Mode): 2分钟刺激+8分钟吸乳的增产模式
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
  - IMU传感器（加速度/陀螺仪）
  - 力传感器/压力传感器
  - 气压传感器
  - 传感器校准与滤波
  - 数据融合算法接入

### 14. 算法框架管理
- **文件**: `trunk/include/algorithm_manager_api.h`
- **实现**: `trunk/root_source/algorithm/`
- **功能**: 算法框架调度与管理
- **特性**:
  - 算法任务调度
  - 数据流管理
  - 算法链式处理

### 15. 测试模块（Tests）
- **文件**: `trunk/include/tests_manager_api.h`
- **实现**: `trunk/root_source/tests/`
- **功能**: 提供固件自检、产测钩子与集成用例的统一入口
- **特性**:
  - 产测命令处理
  - 自检流程
  - 测试用例框架

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
- **模式ID**: `BREAST_PUMP_STIMULATION_MODE`
- **用途**: 模拟婴儿初始吸吮动作，快速频率刺激乳腺，促进泌乳反射
- **特点**: 高频率、低压力、短周期
- **工作时长**: 标准时长（可配置）
- **适用场景**: 吸乳前的预备阶段，帮助乳汁快速流出

#### 2. 吸乳模式 (Pump Mode)
- **模式ID**: `BREAST_PUMP_LACTATION_MODE`
- **用途**: 标准的吸乳工作模式，模拟婴儿深度吸吮
- **特点**: 中等频率、可调压力、长周期
- **工作时长**: 标准时长（可配置）
- **适用场景**: 主要吸乳阶段，适合常规使用

#### 3. 混合模式 (Mix Mode)
- **模式ID**: `BREAST_PUMP_MIX_MODE`
- **用途**: 结合刺激和吸乳两种模式的优势
- **特点**: 先快速刺激后深度吸乳，自动切换
- **工作时长**: 标准时长（可配置）
- **适用场景**: 一键完成刺激到吸乳的完整流程

#### 4. 开奶模式 (Lactation Start Mode)
- **模式ID**: `BREAST_PUMP_LACTATION_START_MODE`
- **用途**: 专为产后初期（产后1-3天）设计，帮助建立泌乳
- **特点**: 温和压力、特殊节奏、延长工作时间
- **工作时长**: 延长时长（通常20分钟，由`LACTATION_START_MODE_WORK_TIME`定义）
- **适用场景**: 产后开奶期，需要更长时间和特殊节奏刺激乳腺

#### 5. 个性化模式 (Custom Mode)
- **模式ID**: `BREAST_PUMP_CUSTOM_MODE`
- **用途**: 用户可根据自身情况自定义参数
- **特点**: 
  - 支持动态参数更新（通过`custom_mode_parameter_update()`）
  - 可调整压力、频率、持续时间等所有参数
  - 工作时长根据配置的子策略动态计算
- **工作时长**: 动态计算（所有子策略持续时间总和）
- **适用场景**: 特殊需求用户，或通过APP学习用户习惯后自动生成

#### 6. PP追奶模式 (PP Milk Track Mode)
- **模式ID**: `BREAST_PUMP_PP_MILK_TRACK_MODE`
- **用途**: 专为追奶场景设计，增加泌乳量
- **特点**: 特殊的频率和压力组合，优化追奶效果
- **工作时长**: 标准时长（可配置）
- **适用场景**: 奶量不足的妈妈，需要增加泌乳量

#### 7. 2+8奶量提升模式 (Milk Boost Mode)
- **模式ID**: `BREAST_PUMP_MILK_BOOST_MODE`
- **用途**: 通过科学的时间分配提升泌乳量
- **特点**: 2分钟刺激 + 8分钟吸乳的黄金组合
- **工作时长**: 固定10分钟（由`MILK_BOOST_MODE_WORK_TIME`定义）
- **适用场景**: 追求高效吸乳，时间有限的场景

### 模式参数配置
- **配置文件位置**: `overlay/breast_pump/table/pump_mode_default_config.h`
- **参数类型**: 
  - `motor_policy_cfg_t`: 单个策略配置（压力、频率、持续时间、循环次数等）
  - 支持`MOTOR_TIME_POLICY`类型（定时策略）和子策略的嵌套组合
- **配置示例**: 每种模式有对应的默认参数数组（如`stimulation_default[]`、`pump_default[]`等）

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

// 自定义模式参数更新（仅限Custom Mode）
int custom_mode_parameter_update(motor_policy_cfg_t *cfg, uint8_t cfg_num);
```

### 模式扩展指南
要添加新的工作模式，需要：
1. 在模式枚举中添加新的模式ID
2. 在`overlay/breast_pump/table/`中定义参数表
3. 在`overlay/breast_pump/mode/`中创建模式文件（参考现有模式）
4. 在`breast_pump_mode.c`的`get_breast_pump_mode_para()`中添加分支
5. 在`breast_pump_mode_work_time_get()`中设置工作时长
6. 在业务层注册新模式

## 驱动支持

### 16. 通用I2C总线驱动
- **目录**: `drivers/i2c/`
- **功能**: 统一I2C适配层，支持后续多设备挂载

### 17. FATFS 文件系统适配
- **目录**: `drivers/fatfs/`
- **功能**: 对接 FATFS，提供基础文件系统能力

### 18. WiFi驱动支持
- **目录**: `drivers/wifi/`
- **功能**: 新增WiFi驱动支持，扩展网络连接能力

### 19. 串口驱动（UART）
- **目录**: `drivers/uart/`
- **功能**: 提供 UART 外设访问能力，支持后续协议/日志转储

### 20. ADC 驱动
- **目录**: `drivers/adc/`
- **功能**: 提供模拟量采样能力，面向传感器读取与校准

### 21. 气压传感器驱动
- **目录**: `drivers/air_pressure/`
- **功能**: 气压/海拔等相关传感器适配与读数

### 22. 光传感器驱动
- **目录**: `drivers/light_sensor/`
- **功能**: 环境光传感器驱动，支持自适应调光

## 外部库集成

### 23. LVGL图形库
- **目录**: `trunk/external/lvgl/`
- **功能**: 轻量级嵌入式图形界面库

### 24. LittleFS文件系统
- **目录**: `trunk/external/littlefs/`
- **功能**: 为嵌入式系统设计的轻量级文件系统

### 25. cJSON解析库
- **目录**: `trunk/external/cJSON/`
- **功能**: 轻量级JSON解析与生成

### 26. 加密与哈希库
- **目录**: `trunk/external/crypto/`、`trunk/external/md5/`、`trunk/external/base64/`
- **功能**: 提供加解密、哈希、编码能力

### 27. QuickLZ压缩库
- **目录**: `trunk/external/quicklz/`
- **功能**: 快速资源压缩/解压

## 算法库

### 28. 根算法库（Root Algo）
- **目录**: `library/root_algo/`
- **功能**: 核心算法实现库
- **特性**:
  - 算法融合处理（`root_algo_fusion.c`）
  - 穿戴检测（`wear_detecttion/`）
  - 环形缓冲区（`ring_buffer/`）
  - 算法处理钩子（`overlay/algo/`）：采集处理、核心处理、后处理

## 业务管理器（Manager Layer）扩展

### 29. 电池管理系统
- **文件**: `trunk/include/battery_manager_api.h`
- **实现**: `trunk/root_source/battery/`
- **功能**: 电池状态监控、电量管理、充电控制
- **特性**:
  - 电池电量检测
  - 充电状态管理
  - 低电量保护

### 30. 服务器管理系统
- **文件**: `trunk/include/servers_manager_api.h`
- **实现**: `trunk/root_source/servers/`
- **配置**: `overlay/servers/`
- **功能**: 统一管理各类服务器（应用协议服务器、通用服务器、工厂服务器等）
- **特性**:
  - 服务器注册与管理
  - 消息路由与分发
  - 协议适配层

### 31. 资源管理系统
- **文件**: `trunk/include/resource_manager_api.h`
- **实现**: `trunk/root_source/resource/`
- **配置**: `overlay/resources/`
- **功能**: 统一管理图片、字体、语言包等资源的加载、解压与缓存

### 32. 协议管理系统
- **文件**: `trunk/include/protocol_manager_api.h`
- **实现**: `trunk/root_source/protocol/`
- **配置**: `overlay/protocol/`
- **功能**: 统一封装应用协议、通用协议与产品协议入口

### 33. 电源管理系统
- **文件**: `trunk/include/power_manager_api.h`
- **实现**: `trunk/root_source/pm/`
- **配置**: `overlay/framework_hooks/power/`
- **功能**: 负责电源状态切换、低功耗控制与系统级电源事件处理

### 34. LED管理系统
- **文件**: `trunk/include/led_manager_api.h`
- **实现**: `trunk/root_source/led/`
- **配置**: `overlay/framework_hooks/led/`
- **功能**: 统一管理指示灯、背光与灯效相关控制逻辑

## 体系结构（逻辑分层）

- **上层调用下层**，唯一与RTOS直接交互的应在平台/OS抽象处（若集成RTOS）
- **UI/业务 → 管理器/服务 → 平台抽象/驱动 → 芯片平台/硬件**
- 通过`overlay/`实现产品差异化与资源替换，避免改动`trunk/`主干代码

## Overlay配置系统详解

`overlay/`目录是框架的配置层，用于实现产品差异化定制，不同产品可通过覆盖该目录实现差异化配置。

### Overlay目录结构与用途

#### 1. `overlay/breast_pump/` - 吸奶泵业务配置
- **mode/**: 工作模式定义，支持7种标准模式
  - `stimulation_mode.c` - 刺激模式（快速频率促进泌乳）
  - `pump_mode.c` - 标准吸乳模式
  - `mix_mode.c` - 混合模式（刺激+吸乳）
  - `lactation_start_mode.c` - 开奶模式（初期哺乳优化）
  - `custom_mode.c` - 个性化模式（支持动态参数更新）
  - `pp_milk_track_mode.c` - PP追奶模式
  - `milk_boost_mode.c` - 2+8奶量提升模式
  - `breast_pump_mode.c/h` - 模式管理框架（模式初始化、参数获取、工作时长管理）
- **table/**: 参数表配置（压力曲线、频率表、时间配置、定时策略等）
- **用途**: 
  - 产品可根据不同型号配置不同的工作模式和参数
  - 每种模式由定时策略（Time Policy）和子策略（Sub Policy）组成
  - 支持模式级的工作时长配置（如开奶模式20分钟，奶量提升模式10分钟等）

#### 2. `overlay/framework_hooks/` - 框架钩子
- **airplane_mode/**: 飞行模式状态切换钩子
- **algo/**: 算法模块初始化与回调钩子
- **ble/**: 蓝牙模块初始化与事件钩子
- **breast_pump/**: 吸奶泵业务钩子（模式切换、状态回调等）
- **input_key/**: 按键事件处理钩子
- **led/**: 指示灯与背光控制钩子
- **motor/**: 马达控制钩子（PWM调整、反馈处理等）
- **power/**: 电源状态迁移与低功耗策略钩子
- **product_tests/**: 产测与自检流程钩子
- **sensor/**: 传感器数据处理钩子
- **servers/**: 服务器生命周期与路由钩子
- **storage/**: 存储初始化与配置钩子
- **ui/**: UI初始化、事件转发与页面联动钩子
- **用途**: 在不修改框架核心代码的前提下，注入产品特定逻辑

#### 3. `overlay/input_key/` - 输入按键配置
- **input_key_code.h**: 按键码定义与映射
- **用途**: 不同产品可能有不同的按键布局，通过该文件定义按键功能映射

#### 4. `overlay/sensor/` - 传感器配置
- **air_pressure/**: 气压传感器校准参数与配置
- **force_sensor/**: 力传感器校准参数与配置
- **imu/**: IMU传感器配置（采样率、量程、滤波参数等）
- **light_sensor/**: 环境光传感器阈值与校准配置
- **用途**: 不同批次或型号的传感器可能需要不同的校准参数

#### 5. `overlay/storage/` - 存储分区配置
- **breast_pump/**: 吸奶泵业务数据存储定义（历史记录、用户设置等）
- **factory/**: 工厂校准数据存储（MAC地址、校准系数等）
- **resource/**: 资源文件存储配置
- **system/**: 系统配置存储（设备信息、系统参数等）
- **persist_storage.h**: 持久化存储结构定义
- **用途**: 定义各类数据的存储结构和分区布局

#### 6. `overlay/adaptive_dimming/` - 自适应调光
- **用途**: 根据环境光自动调整屏幕亮度，提升用户体验和节能

#### 7. `overlay/algo/` - 算法处理实现
- **root_algo_acq_proc.c/h**: 算法数据采集与预处理
- **root_algo_core.c/h**: 算法核心处理逻辑
- **root_algo_post_proc.c/h**: 算法后处理与结果输出
- **用途**: 产品特定的算法处理实现，与`library/root_algo/`配合使用

#### 8. `overlay/protocol/` - 协议处理
- **app_old_commication_protocol/**: 应用层旧版通信协议
- **common_protocl/**: 通用协议处理
- **用途**: 产品特定的协议实现，与`overlay/servers/`配合使用

#### 9. `overlay/servers/` - 服务器实现
- **app_old_protocol_server/**: 应用层旧版协议服务器
- **common_server/**: 通用服务器
- **factory_server_client/**: 工厂测试服务器客户端
- **master_server_client/**: 主服务器客户端
- **用途**: 产品特定的服务器实现，由`trunk/root_source/servers/`统一管理

#### 10. `overlay/ui/` - UI实现
- **lcd_direct_ui/**: LCD直接UI实现
- **lcd_direct_ui_v2/**: LCD直接UI V2版本
- **lcd_uart_ui/**: LCD UART UI实现
- **v3pro_lcd_direct_ui_v2/**: V3 Pro产品专用 LCD Direct UI V2实现
- **ui_img_define.h**: UI图片资源定义
- **ui_page_define.h**: UI页面定义
- **用途**: 产品特定的UI实现，支持多种显示方式

#### 11. `overlay/product_tests/` - 产品测试
- **comm/**: 通信测试
- **factory_tests/**: 工厂测试用例
- **hardware_self_tests/**: 硬件自检测试
- **product_tests.h**: 产品测试接口定义
- **用途**: 产品测试与产测功能实现

#### 12. `overlay/font/` - 字体资源
- **用途**: 存放LVGL使用的字体文件，支持多语言显示

#### 13. `overlay/resources/` - 多媒体资源
- **用途**: 存放图片、语言包、音频等二进制资源文件

#### 14. `overlay/windows/` - UI窗口资源
- **airplane_mode/**: 飞行模式窗口
- **battery/**: 电池显示窗口
- **breast_pump/**: 吸奶泵工作与参数窗口
- **charge/**: 充电窗口
- **guide/**: 引导/教学窗口
- **home/**: 主页窗口资源与布局
- **ota/**: OTA升级窗口
- **logo/**: 开机Logo
- **popup_windows/**: 弹窗集合（提示、确认、错误等）
- **product_test_window/**: 产测窗口
- **setting/**: 设置窗口
- **system/**: 系统状态与系统级窗口
- **dummy/**: 示例/占位窗口
- **windows_map_define.c/h**: 窗口映射表与注册
- **用途**: UI页面的资源文件、布局定义与窗口管理

### Overlay使用原则

1. **框架主干不变**: `trunk/`目录中的代码应保持通用性，不包含产品特定逻辑
2. **配置优先覆盖**: `overlay/`中的配置会覆盖框架默认配置
3. **钩子注入逻辑**: 通过`framework_hooks/`注入产品特定的业务逻辑
4. **资源分离管理**: UI资源、字体、图片等与代码分离，便于独立更新
5. **多产品支持**: 不同产品可使用不同的overlay配置，实现一套框架支持多个产品

## 开发规范

### API设计原则
- API面向`trunk/include/`对外暴露，内部实现放入`trunk/root_source/`
- 芯片相关改动位于`trunk/platform_chip/`，避免渗透到上层逻辑
- 资源与产品差异通过`overlay/`覆盖，引入统一初始化钩子
- 构建仅修改根目录`GNUmakefile`与`boards/*.h`聚合配置

### 依赖约束
- `trunk/include` 被上层直接引用；实现仅在`root_source/`
- `drivers` 不依赖上层业务；仅向上提供稳定接口
- `platform_chip` 仅被`drivers`与`root_source`适配层使用
- `overlay` 可覆盖资源与钩子，但不应直接依赖底层私有实现细节

## 构建说明

### 仓库初始化与 SDK 子仓库更新
- 本工程依赖 Git 子仓库 `trunk/platform_chip/bes`，首次克隆后请先初始化同步，再开始编译。

```bash
# 首次克隆（推荐）
git clone --recurse-submodules <repo-url>

# 已克隆但未初始化子仓库
git submodule update --init --recursive

# 当主仓更新了 SDK 指针后，同步到主仓锁定版本
git submodule sync --recursive
git submodule update --init --recursive trunk/platform_chip/bes

# 主动拉取 SDK 子仓库远端最新 master
git submodule update --remote trunk/platform_chip/bes
git add trunk/platform_chip/bes
```

- `git submodule update --init --recursive trunk/platform_chip/bes` 适用于跟随当前主仓记录的 SDK 版本。
- `git submodule update --remote trunk/platform_chip/bes` 适用于需要主动升级 SDK 子仓库版本的场景；执行后请检查差异并提交主仓中的子仓库指针变更。

### 构建入口
- 在根目录直接使用`GNUmakefile`或`build_helper.sh`进行编译与打包
- 产品特定配置位于`configs/`目录（如`V3_2710IMC_GNUmakefile`、`V3_2710IMP_GNUmakefile`）
- 产物默认输出到`project/bin/`，中间文件位于`project/build/`

### 编译命令
```bash
# 编译特定目标
make T=best1306p -j16

# 使用产品特定配置编译
make -f configs/V3_2710IMP_GNUmakefile T=best1306p -j16

# 清理构建
make clean

# 查看帮助
make help
```

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

- 本仓风格对齐`README_REF.md`
- 后续模块命名建议统一使用`*_manager_api.h`形式对外暴露接口
- 新增接口遵循现有命名规范，保持一致性

---

**注意**: 仅允许向下依赖（上层只能依赖同级或更底层模块），禁止跨层倒置依赖。
