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
│   ├── uart/                        ← 串口/UART驱动
│   └── wifi/                        ← WiFi驱动支持（新增）
│
├── overlay/                         ← 资源与钩子（工程可重写/定制）
│   ├── font/                        ← 字体资源
│   ├── framework_hooks/             ← 框架钩子（任务/事件/日志等）
│   ├── motor_config/                ← 马达参数
│   ├── protocol_hooks/              ← 协议处理钩子（产品差异点）
│   ├── resources/                   ← 图片/语言包/二进制资源
│   └── windows/                     ← UI窗口/页面资源与布局
│       ├── home/                    ← 主页/主屏窗口
│       ├── battery/                 ← 电池状态/电量展示窗口
│       ├── charge/                  ← 充电过程/满电提示窗口
│       ├── ota/                     ← OTA升级流程窗口
│       ├── setting/                 ← 设置/选项类窗口
│       ├── breath_pump/             ← 吸奶泵业务相关窗口
│       ├── logo/                    ← 开机Logo/品牌展示
│       ├── pop_windows/             ← 通用弹窗集合（提示/确认/错误等）
│       └── dummy/                   ← 占位/示例窗口（可根据产品裁剪）
│
├── project/                         ← 构建产物与构建系统文件
│
└── trunk/                           ← 框架主干（平台与核心业务）
    ├── include/                     ← 对外API头文件（管理器/服务接口）
    │   ├── airplane_mode_api.h      ← 飞行模式管理接口
    │   ├── algorithm_api.h          ← 算法框架接口
    │   ├── audio_manager_api.h      ← 音频管理接口
    │   ├── ble_manager_api.h        ← BLE管理接口
    │   ├── broadcast_manager_api.h  ← 广播管理接口
    │   ├── root_message.h           ← 异步消息实体/统一消息封装
    │   ├── root_storage_base.h      ← 存储分区/魔数基础定义
    │   ├── handler_post.h           ← 任务/消息派发接口
    │   ├── hang_detect.h            ← 卡死检测/线程看门狗
    │   ├── misc_manager_api.h       ← 杂项能力管理接口
    │   ├── motor_manager_api.h      ← 马达管理接口
    │   ├── ota_manager_api.h        ← OTA升级管理接口
    │   ├── peripheral_bus_api.h     ← 外设总线接口
    │   ├── power_manager_api.h      ← 电源管理接口
    │   ├── time_system_api.h        ← 时间系统接口
    │   ├── thread_api.h             ← 线程接口抽象
    │   ├── resource_manager_api.h   ← 资源管理接口
    │   ├── root_common.h            ← 通用接口集合
    │   ├── root_common_api.h        ← 通用接口集合（已统一命名）
    │   ├── root_common_config.h     ← 统一配置/线程栈与优先级
    │   ├── root_common_register.h   ← 模块注册/框架入口
    │   ├── root_common_base.h       ← 公共基类/宏/类型
    │   ├── root_list_api.h          ← 链表管理接口（新增）
    │   ├── root_log_define.h        ← 日志系统定义
    │   ├── storage_manager_api.h    ← 存储管理接口
    │   ├── key_input_manager_api.h  ← 输入系统管理接口（新增）
    │   ├── sensor_manager_api.h     ← 传感器管理接口（新增）
    │   ├── project_default_features.h ← 默认功能开关（新增）
    │   ├── project_default_parameter.h ← 默认参数定义（新增）
    │   ├── windows_manager_api.h    ← 窗口管理接口
    │   └── tests_manager_api.h      ← 测试模块接口（新增）
    │
    ├── root_source/                 ← 核心子系统实现（面向上层的服务）
    │   ├── airplane_mode/           ← 飞行模式
    │   ├── algorithm/               ← 算法框架/调度
    │   ├── audio/                   ← 音频子系统
    │   ├── base/                    ← 基础设施（初始化/错误码/工具）
    │   ├── ble/                     ← BLE主干（Host/Profile/Manager）
    │   ├── breath_pump/             ← 吸奶泵示例模块（演示/占位）
    │   ├── hal/                     ← 面向上层的硬件抽象
    │   ├── key_input/               ← 输入系统（按键/触摸）
    │   ├── misc/                    ← 杂项能力（时间/随机数/配置）
    │   ├── motor/                   ← 马达子系统
    │   ├── network/                 ← 网络与协议
    │   ├── ota/                     ← 升级子系统
    │   ├── pm/                      ← 电源管理/低功耗策略
    │   ├── protocol/                ← 应用/产测/私有协议
    │   ├── resource/                ← 资源管理/解压/缓存
    │   ├── sensor/                  ← 传感器封装（IMU/FSensor汇聚）
    │   ├── storage/                 ← 存储抽象（KV/FS/分区）
    │   ├── utils/                   ← 通用工具集
    │   ├── tests/                   ← 自检/产测/集成测试模块（新增）
    │   └── windows/                 ← UI窗口框架/页面管理
    │
    ├── platform_chip/               ← 芯片平台相关适配层
    │   └── bes/                     ← BES 平台移植/驱动对接
    │       ├── os_base/             ← 操作系统基础抽象
    │       └── sdk/                 ← BES SDK适配
    │
    ├── external/                    ← 外部组件/第三方库
    │   ├── crypto/                  ← 加解密/哈希/安全库
    │   ├── lvgl/                    ← 图形界面库
    │   ├── quicklz/                 ← 资源压缩/解压
    │   └── scan_key/                ← 按键扫描库
    │
    ├── simulator/                   ← 仿真/PC侧开发调试（如有）
    ├── doc/                         ← 文档/设计说明/接口约定
    └── tools/                       ← 开发/发布工具脚本
```

## 新增功能特性

### 1. 外设总线系统 (Peripheral Bus)
- **文件**: `trunk/include/peripheral_bus_api.h`
- **功能**: 统一的外设设备管理接口，支持设备注册、状态管理、操作封装
- **特性**:
  - 设备ID枚举管理
  - 设备状态跟踪（初始化/挂起/恢复）
  - 标准化的设备操作接口（open/close/read/write/ioctl）
  - 宏封装简化设备操作

### 2. 增强日志系统
- **文件**: `trunk/include/root_log_define.h`
- **功能**: 模块化日志过滤和彩色输出支持
- **特性**:
  - 多级日志级别（ERROR/WARN/INFO/DEBUG/VERBOSE）
  - 模块化日志过滤（支持24个功能模块独立过滤）
  - 彩色日志输出支持
  - 时间戳和线程ID支持

### 3. WiFi驱动支持
- **目录**: `drivers/wifi/`
- **功能**: 新增WiFi驱动支持，扩展网络连接能力

### 4. 广播管理接口
- **文件**: `trunk/include/broadcast_manager_api.h`
- **功能**: 系统级广播消息管理

### 5. 链表管理接口
- **文件**: `trunk/include/root_list_api.h`
- **功能**: 通用链表数据结构管理接口

### 6. 任务/消息派发（Handler Post）
- **文件**: `trunk/include/handler_post.h`
- **功能**: 轻量级任务/消息派发与异步执行，降低线程耦合

### 7. 卡死检测（Hang Detect）
- **文件**: `trunk/include/hang_detect.h`
- **功能**: 线程卡死/超时检测，支持独立监控线程

### 8. 通用I2C总线驱动
- **目录**: `drivers/i2c/`
- **功能**: 统一I2C适配层，支持后续多设备挂载

### 9. FATFS 文件系统适配
- **目录**: `drivers/fatfs/`
- **功能**: 对接 FATFS，提供基础文件系统能力

### 10. 吸奶泵示例模块（Breath Pump）
- **目录**: `trunk/root_source/breath_pump/`
- **功能**: 示例子系统结构与接入方式（占位/演示）

### 11. 统一配置基座
- **文件**: `trunk/include/root_common_config.h`
- **功能**: 统一线程优先级、栈深与关键业务配置项

### 12. 串口驱动（UART）
- **目录**: `drivers/uart/`
- **功能**: 提供 UART 外设访问能力，支持后续协议/日志转储

### 13. ADC 驱动
- **目录**: `drivers/adc/`
- **功能**: 提供模拟量采样能力，面向传感器读取与校准

### 14. 气压传感器驱动
- **目录**: `drivers/air_pressure/`
- **功能**: 气压/海拔等相关传感器适配与读数

### 15. 输入系统管理接口
- **文件**: `trunk/include/key_input_manager_api.h`
- **功能**: 抽象按键/触摸输入事件，统一上报与去抖

### 16. 传感器管理接口
- **文件**: `trunk/include/sensor_manager_api.h`
- **功能**: 统一多类传感器的注册、轮询/中断与数据分发

### 17. 项目默认功能与参数
- **文件**: `trunk/include/project_default_features.h`
- **文件**: `trunk/include/project_default_parameter.h`
- **功能**: 汇总项目级默认开关与参数，便于产品化覆盖

### 18. 测试模块（Tests）
- **目录**: `trunk/root_source/tests/`
- **功能**: 提供固件自检、产测钩子与集成用例的统一入口（`tests_manager`）。

## 体系结构（逻辑分层）

- **上层调用下层**，唯一与RTOS直接交互的应在平台/OS抽象处（若集成RTOS）
- **UI/业务 → 管理器/服务 → 平台抽象/驱动 → 芯片平台/硬件**
- 通过`overlay/`实现产品差异化与资源替换，避免改动`trunk/`主干代码

## 开发规范

### API设计原则
- API面向`trunk/include/`对外暴露，内部实现放入`trunk/root_source/`
- 芯片相关改动位于`trunk/platform_chip/`，避免渗透到上层逻辑
- 资源与产品差异通过`overlay/`覆盖，引入统一初始化钩子
- 构建仅修改根目录`Makefile`与`boards/*.h`聚合配置

### 依赖约束
- `trunk/include` 被上层直接引用；实现仅在`root_source/`
- `drivers` 不依赖上层业务；仅向上提供稳定接口
- `platform_chip` 仅被`drivers`与`root_source`适配层使用
- `overlay` 可覆盖资源与钩子，但不应直接依赖底层私有实现细节

## 构建说明

### 构建入口
- 在根目录直接使用`Makefile`或`build_helper.sh`进行编译与打包
- 产物默认输出到`project/bin/`，中间文件位于`project/build/`

### 编译命令
```bash
# 编译特定目标
make T=best1306p -j16

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
