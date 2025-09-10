# Smart Wearable Firmware Framework (v3)

基于模块化与分层设计的智能穿戴固件工程框架。

## 目录结构

```
FIRMWARE_V3/
├── Makefile                         ← 构建入口（目标/芯片/配置）
├── build_helper.sh                  ← 构建辅助脚本
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
│   └── wifi/                        ← WiFi驱动支持（新增）
│
├── overlay/                         ← 资源与钩子（工程可重写/定制）
│   ├── font/                        ← 字体资源
│   ├── framework_hooks/             ← 框架钩子（任务/事件/日志等）
│   ├── motor_config/                ← 马达参数
│   ├── protocol_hooks/              ← 协议处理钩子（产品差异点）
│   ├── resources/                   ← 图片/语言包/二进制资源
│   └── windows/                     ← UI窗口/页面资源与布局
│
├── project/                         ← 构建产物与构建系统文件
│   ├── bin/                         ← 可执行/固件镜像输出
│   │   ├── best1306p.bin            ← 固件二进制文件
│   │   ├── best1306p.elf            ← ELF可执行文件
│   │   └── best1306p.map            ← 内存映射文件
│   └── build/                       ← 中间产物/临时文件
│       ├── extra/                   ← 框架代码编译产物
│       │   └── trunk/               ← 框架主干编译输出
│       ├── lib/                     ← 静态库文件
│       │   └── libextra.a           ← 框架静态库
│       └── sdk/                     ← SDK编译产物
│           ├── best1306p/           ← 目标芯片编译输出
│           │   ├── apps/            ← 应用层编译产物
│           │   ├── bthost/          ← 蓝牙主机栈编译产物
│           │   ├── config/          ← 配置相关编译产物
│           │   ├── multimedia/      ← 多媒体模块编译产物
│           │   ├── platform/        ← 平台相关编译产物
│           │   ├── rtos/            ← 实时操作系统编译产物
│           │   ├── services/        ← 服务层编译产物
│           │   └── utils/           ← 工具库编译产物
│           └── os_base/             ← 操作系统基础编译产物
│               ├── built-in.a       ← 内置库文件
│               └── cmsis_os_log.o   ← CMSIS-OS日志对象文件
│
└── trunk/                           ← 框架主干（平台与核心业务）
    ├── include/                     ← 对外API头文件（管理器/服务接口）
    │   ├── airplane_mode_api.h      ← 飞行模式管理接口
    │   ├── algorithm_api.h          ← 算法框架接口
    │   ├── audio_manager_api.h      ← 音频管理接口
    │   ├── ble_manager_api.h        ← BLE管理接口
    │   ├── broadcast_manager_api.h  ← 广播管理接口
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
    │   └── windows_manager_api.h    ← 窗口管理接口
    │
    ├── root_source/                 ← 核心子系统实现（面向上层的服务）
    │   ├── airplane_mode/           ← 飞行模式
    │   ├── algorithm/               ← 算法框架/调度
    │   ├── audio/                   ← 音频子系统
    │   ├── base/                    ← 基础设施（初始化/错误码/工具）
    │   ├── ble/                     ← BLE主干（Host/Profile/Manager）
    │   ├── breath_pump/             ← 吸奶泵示例模块（演示/占位）
    │   ├── hal/                     ← 面向上层的硬件抽象
    │   ├── input/                   ← 输入系统（按键/触摸）
    │   ├── misc/                    ← 杂项能力（时间/随机数/配置）
    │   ├── network/                 ← 网络与协议
    │   ├── ota/                     ← 升级子系统
    │   ├── pm/                      ← 电源管理/低功耗策略
    │   ├── protocol/                ← 应用/产测/私有协议
    │   ├── resource/                ← 资源管理/解压/缓存
    │   ├── storage/                 ← 存储抽象（KV/FS/分区）
    │   ├── utils/                   ← 通用工具集
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

## 版本与命名

- 本仓风格对齐`README_REF.md`
- 后续模块命名建议统一使用`*_manager_api.h`形式对外暴露接口
- 新增接口遵循现有命名规范，保持一致性

---

**注意**: 仅允许向下依赖（上层只能依赖同级或更底层模块），禁止跨层倒置依赖。
