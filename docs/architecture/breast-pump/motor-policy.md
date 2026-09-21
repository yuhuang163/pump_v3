# 吸奶泵与马达策略引擎

吸奶泵业务分两层：`breast_pump_manager`（业务状态机）→ `motor_manager`（播放引擎 + 策略引擎）。参数表驱动，20ms 节拍执行。

## 1. 分层关系

```text
breast_pump_manager（业务状态机 IDLE/START/PAUSE/CONTINUE/FINISH/ERROR）
  ↓ start_motor / stop_motor / change_motor_mode
motor_manager（马达播放线程 + 策略引擎）
  ↓ update_para_to_motor_device → ioctl(OP_CODE_MOTOR_START_PLAY / SET_PARAMETER)
bsp_motor 驱动（bsp_motor.c ioctl 分发）
  ↓
motor_pump / motor_pwm / motor_stall_detection / pump_motor_error_check
  ↓ HAL PWM/GPADC
物理马达、电磁阀、气压采样
```

## 2. 关键文件

| 文件 | 作用 |
|------|------|
| `trunk/root_source/motor/breast_pump_policy/motor_policy.h` | 策略引擎数据结构与枚举 |
| `.../breast_pump_policy_handler.c` | 主控制流：主循环、驱动下发、补偿、补吸 |
| `.../pump_policy.c` / `pump_time_policy.c` | 单次吸力状态机 / 时间策略 |
| `.../pump_climb_policy.c` / `pump_user_policy.c` / `pump_total_policy.c` | 爬升 / 用户参数 / 管理器封装 |
| `trunk/root_source/motor/motor_manager.c` | 播放线程、策略运行入口 |
| `trunk/root_source/breast_pump/breast_pump_manager.c` | 吸奶业务状态机 |
| `overlay/breast_pump/mode/*.c` | 各模式参数表 |
| `overlay/breast_pump/table/pump_parameters_table.h` | 吸力曲线表 / 频率表 |
| `drivers/motor/bsp_motor.c` + `chip/motor_pump/*` | 驱动 ioctl 分发 + 泵/阀/PWM/堵转 |

## 3. 策略数据结构

```c
// 静态配置（表驱动）
typedef struct motor_policy_cfg {
    uint8_t policy_type;   // MOTOR_TIME_POLICY / MOTOR_TIME_SUB_POLICY
    uint8_t duration;      // 秒
    motor_policy_core_t core;  // type: MOTOR_PARAM_TABLE / PASS_THROUGH
} motor_policy_cfg_t;

// 运行时链表
motor_policy_manager_t { timed_policy_count, head }
  → motor_timed_policy_node_t { sub_policy_count, duration_seconds, policy, next }
    → motor_policy_node_t { core, next }   // 子策略链
```

核心动作状态机（`suction_status_e`）：`IDLE → GENERATE → HOLD → RELEASE → SLEEP → UPDATE_POLICY`。

## 4. 模式列表

`overlay/breast_pump/mode/` 下的模式（`get_breast_pump_mode_para` switch 分发）：

| 模式 | 文件 | 说明 |
|------|------|------|
| 刺激 Stimulation | stimulation_mode.c | 高频率低压力，促进泌乳反射 |
| 吸乳 Pump | pump_mode.c | 标准吸乳 |
| 混合 Mix | mix_mode.c | 先刺激后吸乳（含 mix 第 2 组） |
| 开奶 Lactation Start | lactation_start_mode.c | 产后初期，延长工作时长 |
| 自定义 Custom | custom_mode.c | 动态参数更新 |
| 自定义试戴 Custom Trial | custom_trial_mode.c | 试戴自定义 |
| PP 追奶 | pp_milk_track_mode.c | 追奶场景，含休息节拍 |
| 2+8 奶量提升 Milk Boost | milk_boost_mode.c | 2min 刺激 + 8min 吸乳 |
| 常规泵奶 Regular Pumping | regular_pumping_mode.c | 常规泵奶 |
| 追加排空 Emptying Boost | emptying_boost_mode.c | 排空 |
| 放松 Relax | relax_mode.c | 不转（IDLE + level 0） |

模式枚举 `breast_pump_mode_e` 定义在 `overlay/storage/persist_storage.h`，分两组：

- **常规模式**（`NORMAL_MODE_LOWER..UPPER`）：刺激 / 吸乳 / 混合（`PUMP_MODE_NUM` 决定上限含混合与否）。
- **专家/韵律模式**（`EXPERT_MODE_LOWER..UPPER`）：放松、2+8、PP 追奶、个性化、自定义试戴、常规韵律、开奶、追加排空。

枚举按 `PUMP_RHYTHM_MODE_NUM` 有两套变体（==3 与 else），仅影响顺序与默认值（`NORMAL_MODE_DEFAULT` / `EXPERT_MODE_DEFAULT` / `PUMP_MODE_DEFAULT`），模式集合一致。

## 5. 控制流

**启动**：`start_breast_pump_work() → motor->start_motor()` → 播放线程入队 → `breast_pump_start_prepare()`（拷贝策略到双泵、预泄压、reset）→ 启动 20ms 定时器。

**主循环（每 20ms）**：
```text
RECV msg → 累加 policy_running_time_ms → is_can_play_now()
 → pump_policy_run_devices（时间策略推进）
 → pump_policy_update（每泵吸力状态机推进）
 → action != IDLE → update_para_to_motor_device → ioctl 下发
 → 处理 MOTOR_PUMP_LEVEL_ADJUST_MSG（气压补偿）
```

**参数计算** `pump_policy_info_reset()`：解析 mapping → 取用户档位 → 查吸力表 `[level][speed]` → 算 original 参数（pump_level/pumping_time/valve_on_time/hold_time/sleep_time = 60000/freq − 各时间）→ 爬升叠加 → target_pressure。

**时间策略推进**：递减 `time_policy_remaining_time`，为 0 且双泵 `policy_update_lock==0` 时切到下一 timed_policy 节点并 reset。

## 6. 反馈闭环

1. **气压堵转检测**（motor_stall_detection.c）：GPADC 电压 > 阈值 → 回调 → 转 PAUSE + 广播。
2. **占空比堵转检测**（pump_motor_error_check.c）：占空比动态阈值表 + 滑窗（5 次）→ `MOTOR_STATUS_STALL`。两套并存。
3. **算法佩戴补偿**：`pump_algo_compensation_callback`（IS_SUPPORT_PUMP_DUTY_ADJUST）→ `MOTOR_PUMP_LEVEL_ADJUST_MSG` → `pump_duty_adjust_to_motor_device` 修正 PWM（±2000 钳制）。

## 7. 注意事项

- 双堵转检测并存；`pump_motor_error_check.c` 轮询定时器疑似未启动。
- `policy_update_lock` 门控时间策略推进，防打断未完成动作。
- 爬升标记（mark_auto/mark_rhythm_switch）须在 `pump_policy_info_reset` 之前。力度爬升策略的详细说明见 [`pump_climb_policy.md`](../../../trunk/root_source/motor/breast_pump_policy/pump_climb_policy.md)。
- 时间策略静态表 duration 是秒，`breast_pump_mode_work_time_get` 返回值同为秒（fast-test 分支为 1/10 缩放），注意单位。
- 追奶/自定义依赖 `STORE_ID_PUMP_MODE_STEPS_INFO`，`id.dat32==0` 回退默认。
- 双泵各自独立 `info[i]/policy[i]`，频率同步由 `PUMP_FREQ_SYNC_ENABLE` 控制。
