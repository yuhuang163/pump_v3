# 电池充放电 SOC 策略

> 基于 `bsp_battery.c` / `bsp_battery_internal.h` / `bsp_charge.c` / `battery_manager.c` / `storage_hook.c`  
> 充电器芯片、PD、温度策略见 [CHARGE_STRATEGY.md](charge/CHARGE_STRATEGY.md)  
> 更新日期：2026-08-14

---

## 1. 目标

内部维护一个**估算电压** `battery.voltage`，再查开路电压表得到显示电量。采样 ADC 会随负载、充电端电压抖动，估算电压用来把显示电量做平滑，并满足：

| 原则 | 做法 |
|------|------|
| 放电电量不回跳 | 估算电压只降不升；负载撤掉后不把内阻压降补回去 |
| 充电不虚高 | 大电流用安时积分 + 浮压补偿；小电流用裸 ADC 慢跟，避免满电锁在 ADC−200mV |
| 大偏差快跟、小偏差慢跟 | 查表按偏差分档回拉；充电最上档按超出量封顶，其余档固定 mV/拍 |
| 充满少跳变 | 硬件报满后先去抖约 10s，期间快拉靠近 ADC，再进 FULL 对齐 |

轮询约 **100ms**（`< 95ms` 的调度直接返回）。

---

## 2. 状态分流

入口：`battery_voltage_poll()` → `get_charge_state()`

| 状态 | 条件 | SOC 处理 |
|------|------|----------|
| `CHARGE_STATE_ERROR` | 适配器不匹配 | 按未充电处理 |
| `CHARGE_STATE_CHARGING` | 插入且芯片在充；硬件满电但去抖未满 100 拍也仍走这里 | `charging_voltage_handler()` |
| `CHARGE_STATE_FULL` | `charge_full && plug_in && is_charging` 且去抖 ≥ 100 | `charge_full_voltage_handler()` |
| `CHARGE_STATE_UNCHARGE` | 其他 | `uncharge_voltage_handler()` |

去抖：`get_charge_full_debounce()` 在充电处理里累加。`BATT_CHARGE_FULL_DEBOUNCE = 100`（约 10s），`BATT_CHARGE_FULL_DEBOUNCE_IGNORE = 20`（前 2s 不快拉）。

```
采样 ADC
  ├─ CHARGING ─► 充满快拉 / 电流积分 / 查表 ─► 估算电压上调
  ├─ FULL     ─► 估算电压对齐 ADC，更新动态满电电压
  └─ UNCHARGE ─► 满负载表 / 非满负载表 ─► 估算电压下调
         └─ battery_percent_handler(估算电压) → 显示电量
```

---

## 3. 估算电压与电量

### 3.1 两个电压

| 量 | 含义 |
|----|------|
| ADC `voltage` | 充电芯片 VBAT，含负载压降 / 充电端浮压 |
| `battery.voltage` | SOC 用的内部估算，目标接近开路电压 |

`vref` 为小数残差：命中补偿时累加本拍 `vref_temp`，再取整数 `step` 写入估算电压，小数留到下一拍。`vref_flag == 0`（`offset ≤ 0` 或本拍无需补偿）时清掉残差。累加后若 `vref > offset`，夹到 `offset`，避免一拍越过 ADC。

### 3.2 电量映射 `sc_batt_tables`

三串，满电基准 `BATT_FULL_VOLTAGE = 12400mV`，上限 `BATT_VOLTAGE_MAX = 12680mV`。末段上限用动态满电电压 `s_batt_full_voltage`（充满时按实际 ADC 上修）。

| 电量 | 电压区间 (mV) |
|------|----------------|
| 0% | 8500–8999 |
| 1% | 9000–9499 |
| 5% | 9500–10018 |
| 10% | 10019–10289 |
| 20% | 10290–10609 |
| 30% | 10610–10843 |
| 40% | 10844–11141 |
| 50% | 11142–11379 |
| 60% | 11380–11617 |
| 70% | 11618–11937 |
| 80% | 11938–12056 |
| 90%–100% | 12057–满电电压 |

段内按电压线性插值。健康系数见 **3.3**。

### 3.3 健康系数

`battery.health` 范围 **90～100**，电压表按 `health/100` 缩放；电流积分的有效容量也是 `3200mAh × health/100`。宏 `IS_SUPPORT_BATTERY_HEALTH`（`project_support_overlay.h`，默认 0）：

| 宏 | 行为 |
|----|------|
| 1 | 充电超时 / 充满时 `battery_health_set()` 写入新系数；值变化才回调上层 |
| 0 | `battery.health` 固定 100，校准只打日志，不回调、不落库 |

RTC 里已有合法值时，休眠唤醒 `battery_health_update()` 不再重置为 100。

**持久化**（宏为 1 时才走通）：

```
校准改 health
  └─ battery_health_set() 仅在值变化时回调
       └─ battery_manager：OP_CODE_BAT_ADD_HEALTH_CHANGE_CALLBACK
            └─ put_store_ram_val(STORE_ID_SYS_BATTERY_HEALTH)
                 └─ sys_state_info_t.battery_health → 周期/关机 sync 进 database

上电 storage_hook（与电量 PERCENT_UPDATE 同时）
  └─ 从 database 读 battery_health（非法值当 100）
       └─ OP_CODE_BAT_HEALTH_SET → battery_health_set()
            └─ 此时尚未注册回调，不会把刚恢复的值再写回去
```

`battery_manager` 只注册回调，不负责上电回灌。

---

## 4. 放电策略

`offset = 估算电压 − ADC`。只在 `offset > 0` 时下调估算电压，**从不往回加**。

### 4.1 满负载 / 非满负载

`is_full_load_running()` 为真走满负载表（电机大电流，滤纹波、慢跟压降）；否则走非满负载表。命中最高档后 `break`，`.rate` 为**每拍回拉 mV**。

结算：

```
if (vref_flag == 0)  vref = 0
else                 vref += uncharge_vref_calculate(vref_temp)
                     若 vref > offset，夹到 offset
估算电压 -= (int)vref
```

夹到 `offset` 后，单拍不会把估算电压拉到 ADC 以下。仍不往回加，负载撤掉后电量不回跳。

低电压加速 `uncharge_vref_calculate()`（看 ADC）：

| ADC | 满负载乘数 | 非满负载乘数 |
|-----|------------|--------------|
| < 9.0V | 1.25 | 1.15 |
| < 9.5V | 1.15 | 1.05 |
| < 10.0V | 1.05 | 1.01 |
| ≥ 10.0V | 1.0 | 1.0 |

下表时间为 **≥10V、无加速**（最慢）。公式：`档宽 / 步长 / 10`（100ms/拍）。最上档无上界。

### 4.2 非满负载（6 档）

| 偏差 | 步长 (mV/拍) | 本档最久 | 从本档顶收到 0 |
|------|--------------|----------|----------------|
| >400mV | 1 | 每 100mV 10s | — |
| 300–400 | 0.5 | 20s | 6min 27s |
| 200–300 | 0.25 | 40s | 6min 7s |
| 100–200 | 0.15 | 67s | 5min 27s |
| 40–100 | 0.1 | 60s | 4min 20s |
| 0–40 | 0.02 | 3min 20s | 3min 20s |

从 400mV 收到 0 约 **6.4 分钟**。

### 4.3 满负载（6 档）

| 偏差 | 步长 (mV/拍) | 本档最久 | 从本档顶收到 0 |
|------|--------------|----------|----------------|
| >700mV | 0.8 | 每 100mV 12.5s | — |
| 400–700 | 0.3 | 100s | 13min 8s |
| 200–400 | 0.2 | 100s | 11min 28s |
| 150–200 | 0.1 | 50s | 9min 48s |
| 40–150 | 0.08 | 2min 18s | 8min 58s |
| 0–40 | 0.01 | 6min 40s | 6min 40s |

从 700mV 收到 0 约 **13.1 分钟**。长时间满负载会把内阻压降吃进显示电量，停机后不回抬（避免来回跳）。

拔充电器时清 `s_charing_voltage_max`，并把充电曲线百分比同步成当前电量。

---

## 5. 充电策略

`offset = ADC − 估算电压`（正值表示估算偏低，需要上调）。  
`lag_offset = 浮压补偿后的 ADC − 估算电压`，只给**电流积分加速**用。

浮压补偿 `battery_charging_compensation()`（大电流充电端虚高）：

| 采样电压 | 减去 |
|----------|------|
| >12.0V | 200mV |
| >11.0V | 460mV |
| >9.0V | 600mV |

切勿在小电流查表里用这张表：CV/涓流内阻压降很小，再减 200mV 会让快满电一直差一截。

### 5.1 进入充电

- 估算电压为 0：初值 = 浮压补偿后的 ADC  
- 从放电切过来（`s_charing_voltage_max == 0`）：用当前电量反推开路电压，避免从负载压降往上猛追

### 5.2 三条路径（优先级从高到低）

各路径只填 `vref_temp` / `vref_flag`，统一在 `CALC_EXIT` 结算：

```
if (vref_flag == 0)  vref = 0
else                 vref += charging_vref_calculate(vref_temp, 估算电压)
                     若 vref > offset，夹到 offset
估算电压 += (int)vref
```

`charging_vref_calculate()` 按**估算电压**加减速度，避免充电端虚高提前触发：

| 估算电压 | 乘数 |
|----------|------|
| < 9.0V | 2.0 |
| < 9.5V | 1.1 |
| < 10.0V | 1.02 |
| > 12.0V | 0.95 |
| 其他 | 1.0 |

**① 充满快拉**（去抖 21–99 且 `offset > 0`）

```
vref_temp = offset × debounce / 80
vref_flag = 1
```

硬件已报满、状态仍是 CHARGING 的约 8s 窗口，把估算电压拉向裸 ADC。`offset ≤ 0` 时不快拉，避免电量回落。超过 `offset` 的部分在结算里夹掉。

**② 电流积分**（`ibat > 100mA` 且 `offset > 0`）

按有效容量 `3200mAh × health/100`、电流、间隔累加 `s_charging_curve_percent`，再反查目标电压。健康越低，同样电流电量爬得越快。

内部再分三档（先判偏差，再判曲线）：

- `offset > 460mV`：强制回拉，`vref_temp = (offset − 460) / 10`，下限 `0.2`（350–460 档步长），不封顶  
- 曲线已到 100%：`vref_temp = CHARGE_FULL_100MS_RATE × ibat × dt / 100`  
- 曲线未到 100%：目标电压更高则 `vref_temp = 差值`；否则 flag 保持 0，清残差  

估算相对补偿电压滞后过大（`ibat × 0.18Ω + 300mV`）时，曲线增量 ×20，单拍最多 +0.025%。health=100 时 1A 约 31%/h。

**③ 查表**（涓流、电流读失败等）

用裸 `offset`，6 档命中最高档 `break`。小电流时内阻压降已很小，必须跟裸 ADC，才能收到满电。

最上档（`i == 0`，`offset > 460mV`）不是固定步长：

```
vref_temp = min(offset − 460, THREE_CHARGING_FORCE_UPDATE_RATE)   // 当前 rate = 20
```

刚过 460mV 时步长约 1mV，偏差再大封顶 20mV/拍。其余档仍是固定 `.rate`。

| 偏差 | 步长 (mV/拍) | 本档最久 | 从本档顶收到 0 |
|------|--------------|----------|----------------|
| >460mV | min(offset−460, 20) | 每 100mV 约 0.5s（封顶） | — |
| 350–460 | 0.2 | 55s | 约 1h 17min |
| 250–350 | 0.015 | 11min 7s | 约 1h 16min |
| 150–250 | 0.01 | 16min 40s | 约 1h 5min |
| 50–150 | 0.008 | 20min 50s | 约 48min 37s |
| 0–50 | 0.003 | 27min 47s | 27min 47s |

从 460mV 纯查表收到 0 仍约 **1 小时 17 分**（最上档只处理 460 以上那截）。硬件报满后的快拉/FULL 一般不会让最后 50mV 爬满 28 分钟。

---

## 6. 充满

`charge_full_voltage_handler()`：

- 估算电压低于 ADC 则直接对齐（可能跳一截电量，去抖快拉是为了减小这一跳）  
- `voltage > 12400` 时上修 `s_batt_full_voltage`（封顶 12680）  
- 充电曲线百分比置 100  

健康校准：充电超时 6h 或充满时按最高电压算系数，夹在 90～100。`IS_SUPPORT_BATTERY_HEALTH` 为 1 时写入 `battery.health` 并回调落库；为 0 时只打日志。详见 3.3。

---

## 7. 调参

宏在 `bsp_battery_internal.h`。`.rate` **一律是每拍 mV**，不是除数、也不是 `offset × 系数`。

| 想要的效果 | 改什么 |
|------------|--------|
| 大偏差跟更快 | 加大该档 `.rate` 或降低阈值让强制档更早命中 |
| 充电大偏差回拉更快 | 加大 `THREE_CHARGING_FORCE_UPDATE_RATE`（最上档封顶，当前 20） |
| 小偏差少抖 | 减小末档 `.rate` |
| 充电涓流收得更快 | 加大 `THREE_CHARGING_DEBOUNCE_RATE_4/5` |
| 满负载电量掉太快 | 减小 `THREE_UNCHARGE_DEBOUNCE_RATE_5～10` |
| 充满对齐更早/更晚 | `BATT_CHARGE_FULL_DEBOUNCE`（拍数 × 100ms） |
| 低电压充电加速 | `charging_vref_calculate()` 里 `<9.0V` 的 2.0 |
| 快满电减速 | `charging_vref_calculate()` 里 `>12.0V` 的 0.95 |

时间估算：`秒 = 档宽(mV) / 步长 / 10`。
