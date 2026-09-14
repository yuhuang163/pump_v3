# 电源管理 / 电池管理

## 1. 电源管理（pm/power_manager.c）

### 状态机

```c
pm_state: PM_SUSPEND_GOING/FINISH, PM_RESUME_GOING/FINISH,
          PM_WILL_POWER_OFF, PM_POWER_OFF
```

- 开机（plug-in 上电）：`system_power_event_cb` 处理 `HDR_ACTION_POWER_ON_DETECT` → POWER_ON（含充电开机 / 引导开机 / 无效开机）。开机有 `POWER_ON_TIMEOUT_MS = 2000-600` 超时检测，电源键 UP 落在窗口内判误触关机或进 guide。
- 适配器匹配：VBUS 已插 && `charge_status == CHARGE_STATE_ERROR` → 禁止休眠并派发 `ID_WINDOW_POWER_OFF`。
- suspend/resume：`on_pm_suspend_route` → 派发 DUMMY 窗口 → `pbus->suspend()` + PMU sleep；resume 逆序。
- 倒计时：`PM_SUSPEND_WORK_PERIOD=1000ms`，`suspend_counter` 与 `PM_IDLE_TIME_S` 比较切 RESUME/IDLE。
- 定时唤醒：`set_period_wake_up` 配置 PMU RTC。
- wakelock：链表带超时，`WAKELOCK_TIMEOUT_FOREVER=-1`，防止误休眠。

### 关键字段

`power_manager_context_t.priv`：`pm_state / suspend_counter / wakeup_reason / wake_lock_list / is_system_power_on / is_goto_sys_standby`。

Wakeup reason 枚举：BOOT / KEY_POWER / KEY_MODE / ENTER_NEW_SCREEN / IMU_IRQ / BLE_REQ / OTA / CHARGE_PLUG_IN/OUT / BACKGROUND_TASK / IOT / FACTORY_RESUME。

## 2. 电池管理（battery/battery_manager.c）

### 电量状态机

```c
battery_state_e: NONE / EMPTY(1%) / LOW_POWER_SHUTDOWN(5%) /
                 FORBID_START(10%) / LOW_POWER_RUN(20%)
```

### SOC 估算（drivers/battery/bsp_battery.c）

- 内部估计电压（RTC_DATA_ATTR 掉电保持），`battery_percent_handler` 通过 `sc_batt_tables` OCV 表查百分比（按 health/100 缩放）。
- 未充电：只降不升；充电：电流曲线积分；充满：反推更新健康度（90~100）。

### 充电策略（drivers/battery/charge/bsp_charge.c）

- 3 串电池，充电 IC I2C 0x75，PD 控制器 I2C 0x23。
- 温度策略状态机：UNKNOW / NORMAL(15~45°C，PD 协议最大电流≤2000mA) / DERATED((0,15) 或 (45,60)°C，600mA) / DISABLE(≤0 或 ≥60°C，GPIO 关充电)。策略用 NTC 滑动平均温度，状态切换约 400ms 去抖；插枪只记电流上限，等去抖后才开充。
- 充电状态 `get_charge_state`：mismatch→ERROR；full→FULL；plug_in→CHARGING；否则 UNCHARGE。FULL 需 100 次轮询去抖。
- PD 匹配（bsp_charge_pd.c）：插入→`charge_pd_power_is_ready`（≥PD2.0）→ `CHARGE_EVENT_PLUG_IN`；版本过低→`ADAPTER_MISMATCH`。
- 一次读 12 字节 ADC 寄存器（VBUS/IBUS/VBAT/IBAT/NTC/status），VBAT/NTC 各 20 点滑动平均。

## 3. 注意事项

- 开机超时检测与按键释放有竞态（UP 落在 timeout 窗口内会误关）。
- 充电/适配器异常强制禁止 PM 休眠。
- 充电温度策略使用 float 滑动平均，无回差，状态切换约 400ms 去抖；DISABLE 期间每轮重申关充。
- I2C 异常计数到阈值置 `s_charge_is_ready=false`，成功时不回退计数。
- `system_power_off` 3 次 ioctl 尝试 + `platform_system_reboot` 兜底。
