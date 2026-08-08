# 力度爬升策略说明

## 1. 概述

用户设定力度突然升高时，不立刻拉满，而是按泵奶周期逐步抬升，避免体感突兀。

- 开关：`IS_SUPPORT_CLIMB_POLICY`
- 实现：`trunk/root_source/motor/breast_pump_policy/pump_climb_policy.c`
- 调用：`pump_policy_info_reset()` 在算出 `period_ms`（`total_time`）后调用 `pump_climb_policy_update()`
- 状态：每个泵独立维护 `climb[pump_id]`
- 日志：`setting:实际档(目标档)/pwm/speed`，例如 `setting:4(15)/3600/0`

## 2. 自动调档初始化（重要）

### 2.1 启停 / 手动切模式：从 0 爬升

`pump_climb_policy_mark_auto()` **必须在** `pump_policy_info_reset()` 之前调用。

| 调用点 | 位置 |
|--------|------|
| 切模式 | `change_motor_mode()` |
| 启停/切模式准备 | `breast_pump_start_prepare()`（泄压后立刻 `policy_info_reset`） |
| 继续播放 | `breast_pump_policy_prepare()` CONTINUE 分支 |

```
mark_auto()
  → negative_pressure_release()   // 只做泄压，不再内部 reset
  → pump_policy_info_reset()      // 此处才推进爬升、生成首拍参数
```

注意：`negative_pressure_release()` **不再**调用 `pump_policy_info_reset()`。

### 2.2 韵律自动切步：按宏选择缓冲策略

韵律（多时间策略）步进切换**没有暂停过程**，走 `pump_time_policy.c` 的 `timed_policy->next`。  
此处调用 `pump_climb_policy_mark_rhythm_switch()`，**不要**再用 `mark_auto()`（否则会从 0 重爬）。

宏（`motor_policy.h`）：

| 宏值 | 含义 | 行为 |
|------|------|------|
| `PUMP_CLIMB_RHYTHM_SWITCH_NONE` (0) | 方案1：不缓冲 | 下一拍直接到新目标 |
| `PUMP_CLIMB_RHYTHM_SWITCH_FROM_1` (1) | 方案2：从 1 开始 | 等同清零后从低档爬 |
| `PUMP_CLIMB_RHYTHM_SWITCH_FROM_CUR` (2) | 方案3：从当前档 | 保留实际档；升档爬升，降档直落 |
| `PUMP_CLIMB_RHYTHM_SWITCH_FROM_CUR_STIM_DOWN` (3) | 方案4：当前档 + 刺激降档缓冲 | 同方案3；吸乳高档→刺激时先钳到 5，再向下缓冲 |

当前默认：

```c
#define PUMP_CLIMB_RHYTHM_SWITCH_POLICY  (PUMP_CLIMB_RHYTHM_SWITCH_FROM_CUR_STIM_DOWN)
```

方案3 / 方案4 升档示例：

| 上一模式实际档 | 下一模式目标 | 行为 |
|----------------|--------------|------|
| 刺激 4 | 吸乳 5 | 从 4 爬到 5（不会 1→2→3→4→5） |
| 吸乳 3 | 刺激 5 | 从 3 开始缓冲（如 3→4→5） |

方案3 降档：一律直落（如吸乳 3 → 刺激 2，直接到 2）。

方案4 降档（`allow_down=1`）：

| 上一模式实际档 | 下一模式目标 | 行为 |
|----------------|--------------|------|
| 吸乳 15 | 刺激 1 | 起点钳到刺激上限 5，再向下缓冲 `5→4→3→2→1` |
| 吸乳 15 | 刺激 5 | 钳到 5，已到目标，直接 5 |
| 吸乳 3 | 刺激 2 | 小幅降档（差值 ≤ 2），直接到 2 |
| 吸乳 8 | 刺激 1 | 钳到 5，再 `5→…→1` |

方案3/4 韵律续爬置 `from_current=1`，升档时**忽略**普通路径的 `START_DIFF=2`。  
方案4 额外置 `allow_down=1`，大跨度降档才走向下缓冲。

## 3. 状态结构

```c
typedef struct {
    uint8_t count;          /// 已爬升步数
    uint8_t target_level;   /// 目标力度
    uint8_t level;          /// 当前实际输出力度
    uint8_t start_level;    /// 本次爬升起始力度
    uint8_t from_current;   /// 1：韵律切档续爬，升档忽略 START_DIFF
    uint8_t allow_down;     /// 1：允许降档缓冲（方案4）
} pump_climb_policy_t;
```

## 4. 关键参数

| 宏 | 值 | 含义 |
|----|----|------|
| `PUMP_CLIMB_MAX_COUNT` | 10 | 最大爬升步数，超限直接落地到目标 |
| `PUMP_CLIMB_TOTAL_TIME_MS` | 6000 | 爬升总时长预算（ms），用于算步进 |
| `PUMP_CLIMB_START_DIFF` | 2 | 启动爬升的最小力度差（仅用于是否启动） |
| `PUMP_CLIMB_MIN_STEP` | 1 | 每步最小增量 |

## 5. 决策流程（当前实现）

```
输入: map->level (目标力度), climb->level (当前实际力度), period_ms (泵奶周期)

1. 目标 == 0
   → 清空爬升状态，输出 0

2. 目标 ≤ 当前（降档或已到目标）
   → 直接落地

3. 新目标且差值 ≤ 2
   → 不启动爬升，直接落地

4. 新目标且差值 > 2
   → 启动爬升：count=0，记录 start_level / target_level

5. count ≥ 10
   → 直接落地到目标

6. 否则按固定步进抬升
   step = max(1, ceil((target - start) * period_ms / 6000))
   new_level = min(当前 + step, 目标)
   count++
```

要点：

- **只爬升、不爬降**：降档立即落地
- **小幅升档（差值 ≤ 2）不爬**：旋钮连加 1～2 档时 `实际==目标`，属预期
- **爬升过程中不再因剩余差 ≤ 2 跳档**：避免 `…→2→4` 这种中途跳目标
- **目标变化会重置 count**：从当前实际力度重新规划步进
- **左右泵独立**：两侧设定不同时，日志会出现 `15(15)` / `0(0)` 这类不对称

## 6. 步进计算（当前策略）

```
step = max(1, ceil((target - start) * period_ms / 6000))
new_level = min(current + step, target)
最多 10 步；达到上限仍未到目标则直接落地
```

按每个泵奶周期步进一次，周期之间可以不连续。步进在本次爬升启动时按总差值算死，过程中不随剩余差重算（除非目标变化重新启动）。

### 6.1 短吸 / 长吸对比

| 模式 | 典型 period | 0→15 的 step | 典型序列 | 耗时 |
|------|-------------|--------------|----------|------|
| 短吸（刺激） | ≈857ms | `ceil(15*857/6000)=3` | `3→6→9→12→15` | ≈ 4～5 秒 |
| 短吸 0→5 | ≈857ms | `ceil(5*857/6000)=1` | `1→2→3→4→5` | ≈ 4 秒 |
| 长吸（吸乳） | ≈1578ms | `ceil(15*1578/6000)=4` | `4→8→12→15` | ≈ 6 秒 |

实机日志（刺激→吸乳 15）：`4(15)→8(15)→12(15)→15(15)`，与上表一致。

---

## 7. 示例（按当前公式）

### 7.1 不启动爬升（差值 ≤ 2）

| 当前 | 目标 | 差值 | 行为 | 输出 |
|------|------|------|------|------|
| 5 | 5 | 0 | 已到目标 | 5 |
| 5 | 6 | 1 | 小幅升档，直接落地 | 6 |
| 5 | 7 | 2 | 边界，直接落地 | 7 |
| 8 | 3 | -5 | 降档，直接落地 | 3 |
| 4 | 0 | -4 | 目标为 0，清空状态 | 0 |

说明：旋钮连续小幅加档时日志常见 `1(1)→3(3)→5(5)→6(6)`，这是直接落地，不是爬升失效。

### 7.2 长吸：0 → 15，period = 1578ms

```
step = ceil(15 * 1578 / 6000) = 4
```

| 周期 | count | 输出 |
|------|-------|------|
| 1 | 1 | **4** |
| 2 | 2 | **8** |
| 3 | 3 | **12** |
| 4 | 4 | **15** |

约 6.3 秒到位。

### 7.3 短吸：0 → 5，period = 857ms

```
step = ceil(5 * 857 / 6000) = 1
```

| 周期 | count | 输出 |
|------|-------|------|
| 1 | 1 | **1** |
| 2 | 2 | **2** |
| 3 | 3 | **3** |
| 4 | 4 | **4** |
| 5 | 5 | **5** |

约 4.3 秒到位，接近每档 +1。

### 7.4 爬升中途改目标

当前实际 5，原目标 12，用户改为 8：

```
差值 = 8 - 5 = 3 > 2 → 重新启动爬升
count=0, start_level=5, target_level=8
period=1578 → step = ceil(3*1578/6000)=1
序列: 6 → 7 → 8
```

### 7.5 爬升中途降档

当前实际 6，目标从 12 改为 4：

```
差值 = 4 - 6 < 0 → 直接落地到 4，重置 count
```

### 7.6 左右泵目标不同

一侧设定 15、一侧设定 0 时：

```
pump0: 4(15)→8(15)→12(15)→15(15)
pump1: 0(0) 保持
```

属左右独立设定，不是爬升串档。

---

## 8. 长吸「尽量每档 +1」——可选改法与后果

当前长吸大跨度（如 0→15）步进为 4，体感偏跳。若希望长吸也尽量 +1，可选方案如下。

### 方案对比（长吸 period≈1578ms，0→15）

| 方案 | 改法要点 | 典型序列 | 耗时 | 优点 | 缺点 |
|------|----------|----------|------|------|------|
| A. 当前策略 | `step=ceil(diff*period/6000)` | `4→8→12→15` | ≈6 秒 | 尽快到目标档 | 步进大，体感跳 |
| B. 强制 step=1 | `step = 1`，取消 6 秒约束 | `1→2→…→15` | ≈**24 秒** | 最平滑 | 切高档后长时间偏弱 |
| C. step=1 + 仍限 10 步 | `step=1`，`count≥10` 直达目标 | `1→…→10`，再跳到 15 | ≈16 秒 | 前段细腻 | 末尾仍跳一截 |
| D. 推荐折中 | `step = max(1, ceil(diff/10))` 或 `min(2, 公式结果)` | `2→4→…→14→15` 或步进≤2 | ≈12～16 秒 | 比 A 细，又不太慢 | 仍非严格每档 +1 |

### 方案 B 代码示意（仅说明，未合入）

```c
// 长吸强制每档 +1（会突破 6 秒预算）
step = PUMP_CLIMB_MIN_STEP;
```

或仅对长吸：

```c
if (period_ms >= 1200) {          // 经验阈值，区分长短吸
    step = PUMP_CLIMB_MIN_STEP;
} else {
    step = pump_climb_get_step_size(total_diff, period_ms);
}
```

### 方案 D 代码示意（推荐折中，未合入）

```c
// 用满最多 10 步，尽量细，同时避免拖到 24 秒
uint8_t step_by_time = pump_climb_get_step_size(total_diff, period_ms);
uint8_t step_by_count = (total_diff + PUMP_CLIMB_MAX_COUNT - 1) / PUMP_CLIMB_MAX_COUNT;
if (step_by_count < PUMP_CLIMB_MIN_STEP) {
    step_by_count = PUMP_CLIMB_MIN_STEP;
}
// 取更细的那个，但至少能在 10 步内爬完
step = step_by_time < step_by_count ? step_by_time : step_by_count;
// 若希望更偏细腻，可改为：step = step_by_count;
```

0→15 时 `ceil(15/10)=2` → 约 `2→4→6→8→10→12→14→15`，约 12.6 秒。

### 选型建议

1. 若产品强调「切到吸乳高档尽快有力」→ 维持 **方案 A（当前）**。
2. 若产品强调「长吸渐变细腻」，可接受十秒级爬升 → 优先 **方案 D**。
3. 仅当明确接受约 20 秒以上弱档过渡时，再用 **方案 B**。

---

## 9. 关键约束小结

1. **只爬升、不爬降**：降档立即落地；普通路径小幅升档（差值 ≤ 2）也立即落地
2. **步进由 6 秒预算与 period 决定**：`step = ceil(总差 * period / 6000)`，最小 1
3. **最多 10 步**：超限直接落到目标
4. **按周期步进**：不依赖连续时钟；泄压路径不得推进爬升
5. **目标变化可中断重来**：重置 count，从当前实际力度重新规划
6. **每泵独立**：左右泵互不影响
7. **启停/手动切模式**：`mark_auto`，从 0 爬升
8. **韵律自动切步**：`mark_rhythm_switch`，由 `PUMP_CLIMB_RHYTHM_SWITCH_POLICY` 选择方案 1/2/3/4
9. **方案4**：吸乳高档→刺激时起点钳到 `BREAST_PUMP_LEVEL_5`，再向下缓冲到目标
10. **mark_* 必须在 policy_info_reset 之前**
