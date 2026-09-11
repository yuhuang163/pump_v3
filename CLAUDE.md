# CLAUDE.md

本文件为 Claude Code（claude.ai/code）在本仓库工作提供指引。

## 构建系统

构建的权威文档见 [`README.md`](README.md)。修改构建开关、功能裁剪、板级配置或 SDK 设置前，先读它。

本项目是面向吸奶泵/母婴穿戴设备的模块化固件框架，基于 BES best1306p（Cortex-M33）MCU，采用分层架构：

- `overlay/` 产品定制与集成
- `trunk/` 框架核心与公共业务逻辑
- `drivers/` 板级设备驱动（BSP）；opcode/类型的 HAL 定义在 `trunk/root_source/hal/`
- `library/` 算法/通用库
- `boards/` 编译期配置与板级定义
- `project/` 构建产物输出

### 常用命令

```bash
# 首次初始化 BES SDK 子仓库（主仓锁定的 commit）
git submodule update --init --recursive trunk/platform_chip/bes

# 跟随 .gitmodules 的 develop 拉取远端最新（会改主仓 gitlink，需另行提交）
git submodule update --remote --recursive trunk/platform_chip/bes

# 交互式构建
./build_helper.sh

# 非交互式固件构建
./build_helper.sh --non-interactive build-firmware

# 生成编译数据库（供 IDE/静态分析）
./build_helper.sh --non-interactive bear

# 直接 make 构建（固件；目标 ble_only_lt，芯片 best1306p）
make firmware -j16

# 直接 make 构建 Bootloader（目标 ota_boot_v2）
make bootloader -j16

# 清理构建产物
make clean
```

产物通常在 `project/bin/firmware/` 和 `project/bin/bootloader/`；中间产物在 `project/build/`。

## 架构

### 分层栈（自上而下）

```text
overlay/         产品定制、钩子、业务配置、UI、BLE 服务、产测
  ├─ framework_hooks/
  ├─ breast_pump/
  ├─ product_tests/
  ├─ ui/
  ├─ servers/
  └─ windows/
trunk/           框架核心（管理器、模块、协议、消息、状态机、编排）
  ├─ include/
  ├─ root_source/
  ├─ external/
  ├─ platform_chip/
  └─ simulator/
library/         算法与通用库
drivers/         板级设备驱动（BSP）
boards/          板级与产品配置聚合
```

### 板级与配置文件

| 文件 | 用途 |
|------|------|
| `boards/product_info.h` | 产品元数据、固件类型、版本标识 |
| `boards/project_config.h` | 线程栈深、消息队列深度、运行期可调参数 |
| `boards/project_log_filter.h` | 各模块日志过滤 |
| `boards/project_partitions.h` | Flash 分区布局与存储映射 |
| `boards/project_support_overlay.h` | 模块/设备/功能开关（`IS_SUPPORT_MODULE_*`、`IS_SUPPORT_DEVICE_*`） |

### 关键架构模式

**模块系统**：`trunk/` 内是通用框架代码；产品代码在 `overlay/`。模块通过框架生命周期注册并初始化，产品级钩子经 `framework_hooks` 注入。

**框架 vs overlay**：产品特定行为必须实现在 `overlay/` 入口；`trunk/` 保持通用、跨产品复用。

**钩子注入**：业务逻辑应通过框架钩子注册接入，而不是对框架核心文件做隐晦改动。接入功能时沿用现有钩子模式。

**状态机驱动**：吸奶泵逻辑与模式切换以状态机为核心。转移逻辑集中管理，不要把状态变更散落到无关模块。

**广播协调**：领域事件与状态变化应尽量走框架级广播通道，而不是模块间直接调用。

**运行时协调模型**：两条异步通道承载全部跨模块工作——`broadcast`（位掩码发布/订阅，用于低频状态变化）与 `handler_post`（定时器驱动的延迟任务）。二者都汇合到窗口管理（WM）线程；LVGL 渲染跑在独立的 `ui_hdr_thd` 线程。模块遵循两阶段生命周期：`register_module()` 建好所有模块 context，然后 `init_module()` 先对所有执行 `perpare_init`，再对所有执行 `post_init`。

**协处理器边界**：处理协处理器或 UART 相关板级逻辑时，通过上层管理器接口路由，不要在业务模块里直接操作底层寄存器。

### 修改代码时的关键规则

- 保持依赖方向为 `overlay -> trunk -> drivers -> platform_chip`。
- 产品特定逻辑放在 `overlay/`，不要放进 `trunk/`。
- 不要在不相关模块间做跨域直接调用；使用现有管理器、广播或钩子路径。
- 新增产品配置放在 `boards/`，功能开关放在对应配置文件里，不要硬编码到实现里。
- 协议或接口变更时，在同一变更集内更新 `docs/protocol/` 下的对应文档。
- 优先做小而可逆的改动，板级特定逻辑尽量归到 overlay 配置下。

### 文档索引

项目文档索引在 [`docs/README.md`](docs/README.md)。修改跨域行为、广播契约、吸奶泵状态机或产测接口前，先读 `docs/architecture/` 下的架构文档；协议变更前读 `docs/protocol/` 下的协议文档。

## Git 工作流

- 任何代码改动优先使用特性分支。
- 未经明确授权，不要直接推送到受保护分支。
- 子仓库 `trunk/platform_chip/bes` 是嵌套仓库，其更新必须当作独立的依赖变更处理。
- 推送前确认远端是目标仓库、分支正确。
- 未经用户明确要求，不要执行 `git commit` 或 `git push`。

## 测试

本仓库是嵌入式固件项目，没有主机侧单元测试作为主工作流。测试通常通过以下方式进行：

- 实际硬件验证
- `overlay/product_tests/` 下的产品测试
- 工厂或自检流程
- 使用 `./build_helper.sh` 和 `make` 做构建验证

常用检查：

```bash
./build_helper.sh --non-interactive build-firmware
./build_helper.sh --non-interactive bear
```

## 提交与改动规范

- 改动保持聚焦，只对应单一架构关注点。
- 尽可能把协议/接口变更与业务逻辑变更分开。
- 新文档要反映真实的运行时契约，而不只是实现细节。
- 架构文档要与代码改动同步，尤其是涉及吸奶泵行为、UI 关系、广播用法或系统状态转换的改动。

## 仓库特定说明

本仓库是面向穿戴式吸奶泵固件的产品平台，基于 BES MCU 与分层分支逻辑构建。设计优先：

- 通过 `overlay/` 做产品定制
- 通过 `trunk/` 提供可复用平台逻辑
- 通过 `boards/` 做板级控制
- 通过 `docs/` 内文档使行为易于发现

有疑问时，优先以具体代码路径和架构文档为准，而非临时修复。
