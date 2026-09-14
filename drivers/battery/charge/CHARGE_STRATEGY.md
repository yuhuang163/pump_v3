# 充电策略文档

> 基于 `bsp_charge.c` / `bsp_charge.h` / `bsp_charge_pd.c` / `bsp_battery.c` 代码整理  
> 更新日期：2026-09-14

---

## 1. 概述

充电系统由三层协作完成：

| 层级 | 文件 | 职责 |
|------|------|------|
| PD 适配器层 | `bsp_charge_pd.c` | PD 协商、插拔检测、适配器匹配 |
| 充电芯片层 | `bsp_charge.c` | 充放电控制、ADC 采样、温度策略 |
| 电池管理层 | `bsp_battery.c` | SOC 估算、充电状态上报 |

---

## 2. 硬件架构

| 模块 | 接口 | 作用 |
|------|------|------|
| 充电芯片 | I2C `0x75` | 充放电控制、ADC 采样（VBUS/IBUS/VBAT/IBAT/NTC） |
| PD 控制器 | I2C `0x23` | PD 协商、适配器识别 |
| `CHARGE_POWER_PIN` | GPIO | 充电通路使能（`charge_power_control`） |
| `CHARGE_SLEEP_PIN` | GPIO | 芯片 sleep 控制 |
| `CHARGE_PD_INT_PIN` | GPIO | VBUS 插入检测（低电平 = 插入） |

---

## 3. 芯片初始化参数

初始化入口：`charge_chip_init()`

| 参数 | 宏/值 | 说明 |
|------|-------|------|
| 电池串数 | `CSEL_3_CELL` | 3 串锂电池 |
| 单节电压 | `VCELL_4_2V` | 4.2V/节，满电约 12.6V |
| 内阻补偿 | `IRCOMP_0MOHM` | 0Ω |
| 默认充电电流 | `BATT_INPUT_CURRENT_LIMIT` = **2000mA** | 可被 PD/温度策略覆盖 |
| 降额充电电流 | `TEMP_ABNORMAL_CHARGING_CURRENT` = **600mA** | 温度异常区间使用 |
| 放电电流限制 | `BATT_OUTPUT_CURRENT_LIMIT` = **3000mA** | |
| 输入电流限制 | `CHARGE_INPUT_CURRENT_LIMIT` = **3000mA** | IIN 寄存器 |
| 输入电压限制 | `CHARGE_INPUT_VOLTAGE_LIMIT` = **4200mV** | VINDPM |
| 电流寄存器步进 | `BATT_CURRENT_LIMIT_SCALE` = **50mA** | `IBAT_SCALE_2` |
| ADC 电流步进 | `BATT_ADC_CURRENT_LIMIT_SCALE` = **12.5mA** | |

### 3.1 充电电流寄存器换算

手册公式（`IBAT_SCALE = 1b`，基于 10mΩ 采样电阻）：

```
I_charge = IBAT_CHG[7:0] × 50mA
```

代码实现：

```c
reg_data = current_mA / BATT_CURRENT_LIMIT_SCALE;  // 600mA → reg=0x0C
```

| 目标电流 | 寄存器值 (hex) | 寄存器值 (bin) |
|---------|----------------|----------------|
| 600mA   | 0x0C           | 00001100b      |
| 2000mA  | 0x28           | 00101000b      |
| 3000mA（芯片默认） | 0x3C   | 00111100b      |

---

## 4. 运行线程

```
定时器 (100ms)
    └─ charge_device_thread
         ├─ charge_pd_status_poll()           // PD 插拔检测
         ├─ charge_status_poll()              // 读 ADC + 状态寄存器
         ├─ charge_ntc_temperature_get()      // NTC 电阻 → 温度
         └─ charging_temp_policy_handler()    // 温度策略
```

| 参数 | 值 | 说明 |
|------|-----|------|
| `CHARGE_THREAD_POLL_INTERVAL` | 100ms | 轮询周期 |
| 状态日志间隔 | 2000ms | vbus/ibus/vbat/ibat/ntc/temp |

---

## 5. PD 适配器策略

入口：`charge_pd_status_poll()`（每 100ms 调用）

### 5.1 流程

```
PD_INT 低电平（插入）
    ├─ 首次：初始化 PD I2C，等待下次轮询
    └─ 已初始化：
         ├─ PD ≥ 2.0 且就绪 → CHARGE_EVENT_PLUG_IN
         └─ 未就绪：
              ├─ 去抖 < 10 次 → 继续等待
              └─ 去抖 ≥ 10 次 → CHARGE_EVENT_ADAPTER_MISMATCH

PD_INT 高电平（拔出）
    └─ CHARGE_EVENT_PLUG_OUT + 释放 PD I2C
```

### 5.2 插入成功处理（`CHARGE_EVENT_PLUG_IN`）

1. 读取 PD 协商电流：`charge_pd_current_get()` → `reg × 50mA`
2. 上限钳位：`min(PD电流, BATT_INPUT_CURRENT_LIMIT)`，读失败则按 2000mA
3. 更新 `s_charging_current_max`（只记上限，不直接开充）
4. 刷新 NTC 滑动平均温度，温度状态回到 `UNKNOW`，**不开充**
5. 充电线程连续确认约 300ms 后，按平均温度套用策略再开充或停充

`CHARGE_EVENT_FORCE_PLUG_IN` 同样走温度策略，不会无条件满电流开充。

### 5.3 适配器不匹配

- 标记 `s_charge_adapter_mismatch = true`
- `charge_power_control(false)` 关闭充电
- 上层状态：`CHARGE_STATE_ERROR`

---

## 6. 温度充电策略（核心）

入口：`charging_temp_policy_handler(float temp)`  
使能条件：`IS_SUPPORT_CHARGE_CURRENT_ADJUST = 1`

### 6.1 温度分区

比较使用 float，无回差。端点归属：

- **15°C、45°C**：走协议最大电流（NORMAL）
- **0°C、60°C**：停充（DISABLE）
- **(0, 15) 与 (45, 60)**：最大 600mA（DERATED）

| 温度范围 | 状态枚举 | 充电电流 | 充电使能 |
|---------|---------|---------|---------|
| 15°C ~ 45°C（含端点） | `CHARGE_TEMP_NORMAL` | `s_charging_current_max`（PD 协商，最高 2000mA） | 开启 |
| (0, 15)°C 或 (45, 60)°C | `CHARGE_TEMP_DERATED` | **最大 600mA** | 开启 |
| ≤0°C 或 ≥60°C | `CHARGE_TEMP_DISABLE` | 寄存器不变 | **关闭** |

边界示例：

| 温度 | 判定 |
|------|------|
| 0.0°C | DISABLE（停充） |
| 0.1°C / 14.9°C | DERATED（最大 600mA） |
| 15.0°C / 45.0°C | NORMAL（协议最大电流） |
| 45.3°C / 59.9°C | DERATED（最大 600mA） |
| 60.0°C | DISABLE（停充） |

宏定义（`bsp_charge.h`）：

```c
#define CHARGE_NORMAL_TEMP_MIN         15.0f
#define CHARGE_NORMAL_TEMP_MAX         45.0f
#define CHARGE_DERATED_TEMP_MIN        0.0f
#define CHARGE_DERATED_TEMP_MAX        60.0f
#define TEMP_ABNORMAL_CHARGING_CURRENT 600
```

### 6.2 状态机

```
                    ┌─────────────────────────────────────┐
                    │         CHARGE_TEMP_UNKNOW          │
                    │    (初始化 / resume / 插枪)          │
                    └──────────────┬──────────────────────┘
                                   │
          ┌────────────────────────┼────────────────────────┐
          │                        │                        │
          ▼                        ▼                        ▼
   15~45°C (>300ms)        (0,15) 或 (45,60)°C (>300ms)  ≤0 或 ≥60°C (>300ms)
          │                        │                        │
          ▼                        ▼                        ▼
   CHARGE_TEMP_NORMAL       CHARGE_TEMP_DERATED       CHARGE_TEMP_DISABLE
   ibat = max, 开充          ibat = 600mA, 开充          关充 (GPIO)
```

### 6.3 状态切换动作

| 进入状态 | 动作 |
|---------|------|
| `CHARGE_TEMP_NORMAL` | 进入时写协议最大电流；已在 NORMAL 时：软件设定值不一致，或芯片实测 `ibat` 大于目标，才补写 |
| `CHARGE_TEMP_DERATED` | 进入时写 `min(600mA, 协议最大电流)`；已在 DERATED 时同样仅在设定值不一致或实测超标时补写 |
| `CHARGE_TEMP_DISABLE` | `charge_power_control(false)`，不改电流寄存器；已在 DISABLE 时每轮重申关充 |

**恢复充电前提条件：**

- `s_charge_status.plug_in == true`
- `s_charge_adapter_mismatch == false`

### 6.4 温度平均与去抖

策略输入是 NTC **滑动平均温度** `s_charge_status.temp`（窗口 20 点 × 100ms ≈ 2s），不是瞬时 raw。

状态切换另有去抖：目标状态需连续满足才落地。

| 参数 | 值 | 说明 |
|------|-----|------|
| `CHARGE_TEMP_AVG_WINDOW` | 20 | 温度滑动平均窗口 |
| `CHARGE_TEMP_POLICY_POLL_INTERVAL` | 3 | 连续满足 >3 次才切换 |
| 轮询周期 | 100ms | 策略去抖约 **400ms** |
| 插枪 | 复位为 UNKNOW | 不开充，等平均温度 + 去抖后再套用 |

### 6.5 日志关键字

| 日志 | 含义 |
|------|------|
| `goto normal charging` | 进入正常充电（max 电流） |
| `goto temp derated charging` | 进入降额充电（600mA） |
| `temp abnormal disable charing` | 温度异常，停止充电 |
| `charge_ibat_config: xxxmA` | 写入充电电流寄存器 |
| `charge_power_control: 0/1` | GPIO 关/开充电 |

---

## 7. 插拔事件策略

入口：`charge_event_internal_cb(charge_event_e event)`

| 事件 | 行为 |
|------|------|
| `CHARGE_EVENT_PLUG_IN` | 记录 PD 电流上限 → 刷新平均温度 → 复位 UNKNOW，等去抖后套策略 |
| `CHARGE_EVENT_FORCE_PLUG_IN` | 同上（强制插入也走温度策略） |
| `CHARGE_EVENT_PLUG_OUT` | 关充，清除 plug_in / mismatch / full |
| `CHARGE_EVENT_ADAPTER_MISMATCH` | 关充，标记 mismatch |

### 7.1 充电状态判定

入口：`get_charge_state()`

```
adapter_mismatch && plug_in  → CHARGE_STATE_ERROR
charge_full && plug_in && is_charging && debounce>=100 → CHARGE_STATE_FULL
plug_in && is_charging       → CHARGE_STATE_CHARGING（含硬件已满、去抖未满 100 拍）
其他                         → CHARGE_STATE_UNCHARGE
```

去抖满 100 拍前仍走 `charging_voltage_handler()`，便于 SOC 快拉靠近 ADC。SOC 细节见 [BATTERY_SOC_STRATEGY.md](../BATTERY_SOC_STRATEGY.md)。

### 7.2 满电判定

- 硬件：芯片 `BAT_FULL` 状态位（`charge_status_poll` 读取）
- 软件去抖：`BATT_CHARGE_FULL_DEBOUNCE = 100` 次（上层轮询累计）

---

## 8. 外部电流调节

| 入口 | 函数/接口 | 说明 |
|------|----------|------|
| 直接调节 | `charge_current_adjust(mA)` | 写 IBAT 寄存器，上限 5000mA |
| IOCTL | `OP_CODE_BAT_CHARGE_CURRENT_ADJUST` | 经 `bsp_battery.c` 调用 |
| BLE 工厂命令 | `BLE_CHARGE_CURRENT_ADJUST_SHELL_CMD` | 调试/产测 |
| PD 插入 | `charge_event_internal_cb(PLUG_IN)` | 自动设置 max |

**注意：** 温度策略会在状态切换时覆盖外部设置的电流（NORMAL 写 max，DERATED 写 600mA）。

### 8.1 `charge_ibat_config` 缓存机制

```c
// 写入成功后同步 s_charging_current，避免重复写入跳过寄存器配置
if (ret == BSP_OK) {
    s_charging_current = current;
}
```

---

## 9. 上层电池管理（SOC）

入口：`bsp_battery.c` → `battery_voltage_poll()`。完整策略见 [BATTERY_SOC_STRATEGY.md](../BATTERY_SOC_STRATEGY.md)。

| 充电状态 | SOC 算法分支 |
|---------|-------------|
| `CHARGE_STATE_CHARGING` | `charging_voltage_handler()` |
| `CHARGE_STATE_FULL` | `charge_full_voltage_handler()` |
| 其他 | `uncharge_voltage_handler()` |

低电量保护（非充电状态）：

- 电量 ≤ 各阈值时触发低电/禁止启动等状态，与充电策略独立。

---

## 10. 生命周期

| 接口 | 行为 |
|------|------|
| `bsp_charge_init()` | GPIO/I2C 初始化 → 芯片配置 → 启动 100ms 定时器 + 充电线程 |
| `bsp_charge_suspend()` | 停止定时器，标记 not ready，重置温度状态 |
| `bsp_charge_resume()` | 恢复 ready，温度状态重置为 UNKNOW，重启定时器 |

---

## 11. 典型场景时序

### 11.1 正常 PD 充电

```
1. 插入 PD 适配器
2. PD 协商成功 → PLUG_IN
3. 读取 PD 电流（如 1500mA）→ s_charging_current_max = 1500
4. 记录 PD 电流上限，温度状态复位 UNKNOW，不开充
5. 约 400ms 去抖后：15~45°C → 协议最大电流；边界温区 → 600mA；≤0 或 ≥60 → 关充
6. 电池满 → BAT_FULL → CHARGE_STATE_FULL
```

### 11.2 高温降额 → 停充 → 恢复（正常物理过程）

策略阈值 15 / 45 / 60，无回差；**正常降温路径必须逐级过渡，不会跳过 DERATED**：

```
1. (45, 60)°C → DERATED，charge_ibat_config(600mA)
2. ≥60°C      → DISABLE，charge_power_control(false)，ibat=0
3. (45, 60)°C → DERATED，charge_ibat_config(600mA)，恢复 600mA 充电
4. 15~45°C    → NORMAL，charge_ibat_config(max)，恢复协议最大电流
```

### 11.2.1 日志异常案例（62.5°C → 33.5°C 跳变）

实际日志中出现的 **62.5°C → 33.5°C（约 2s 内）并非正常降温**，而是 NTC 读数异常：

| 时刻 | NTC 电阻 | 温度 | 说明 |
|------|---------|------|------|
| 停充期间 | 2.80 KΩ | 62.5°C | ≥60，DISABLE，ibat=0 |
| 跳变后 | 7.33 KΩ | 33.5°C | 物理上 2s 内不可能完成此降温 |

NTC 查表（`bsp_ntc_map.h`）验证：62°C ↔ 2.842 KΩ，33°C ↔ 7.453 KΩ，读数与温度自洽，但**跳变本身不合理**。

后果：代码将 33.5°C 判为 NORMAL（15~45），误触发 `goto normal charging`。

> **结论：62 和 33 是日志采样值，不是策略配置值；该跳变属于 NTC 采样异常，不应作为正常恢复流程参考。**

### 11.3 适配器不匹配

```
1. 插入非 PD 2.0+ 适配器
2. 去抖 10 次后 → ADAPTER_MISMATCH
3. charge_power_control(false)
4. 上层：CHARGE_STATE_ERROR，UI 显示充电错误
```

---

## 12. 已知局限

| 项目 | 说明 |
|------|------|
| 中断未启用 | `charge_interrupt_config()` 已定义但 init 中未调用，依赖 100ms 轮询 |
| 温度事件未上报 | `CHARGE_EVENT_TEMPERATURE_ERROR` 已定义但未触发，仅 GPIO 关充 |
| DISABLE 不改电流 | 停充时不写寄存器，恢复时依赖 `charge_ibat_config` 重新配置 |
| 采样电阻假设 | 手册基于 10mΩ，若硬件使用其他阻值会有系统性偏差 |
| 电流步进 | 50mA 步进，600mA 可精确设置，非 50 整数倍会向下取整 |
| NTC 读数跳变 | 可能出现 62→33 等不合理跳变，直接触发 NORMAL，缺少变化率校验 |
| 15/45 边界抖动 | 无回差，温度在端点附近波动时可能在 600mA 与协议最大电流间切换（约 300ms 去抖） |
| 插枪无条件开充 | 已修复：插枪走温度策略；DISABLE 期间每轮重申关充 |
| 温度截断 | 已修复：策略使用 float，不再截成 int8_t |

---

## 13. 关键源文件

| 文件 | 说明 |
|------|------|
| `drivers/battery/charge/bsp_charge.c` | 充电芯片驱动、温度策略 |
| `drivers/battery/charge/bsp_charge.h` | 寄存器定义、阈值宏 |
| `drivers/battery/charge_pd/bsp_charge_pd.c` | PD 适配器管理 |
| `drivers/battery/bsp_battery.c` | 电池 SOC、IOCTL 接口 |
| `drivers/battery/bsp_battery_internal.h` | 内部接口、去抖参数 |
| `trunk/root_source/battery/battery_manager.c` | 上层充电状态轮询 |
