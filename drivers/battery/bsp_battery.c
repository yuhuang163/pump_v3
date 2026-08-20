/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#if IS_SUPPORT_PD_CHARGE
#include "bsp_charge_pd.h"
#endif
#include "bsp_battery_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
#define IS_SUPPORT_BATTERY_POLL           (1)
#else
#define IS_SUPPORT_BATTERY_POLL           (0)
#endif

#ifndef RTC_DATA_ATTR
#define RTC_DATA_ATTR
#endif

#define BATT_LOG_INTERVAL                 50

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_BATTERY_POLL

#if 0
static const capacity_data_t sc_batt_tables[] = {
    {0,  2400, 3149}, /// 2.4-3.149V
    {1,  3150, 3299}, /// 3.15-3.299V
    {2,  3300, 3399}, /// 3.30-3.399V
    {10, 3400, 3491}, /// 3.400-3.491V
    {20, 3492, 3533}, /// 3.492-3.533V
    {30, 3534, 3570}, /// 3.534-3.570V
    {40, 3571, 3614}, /// 3.571-3.614V
    {50, 3615, 3678}, /// 3.615-3.678V
    {60, 3679, 3771}, /// 3.679-3.771V
    {70, 3772, 3873}, /// 3.772-3.873V
    {80, 3874, 3976}, /// 3.874-3.976V
    {90, 3977, 4160}, /// 3.977-4.16V
};

static const battery_para_t sc_charging_tables[] = {
    {0, SG_CHARGING_FORCE_UPDATE_THRESHOLD}, 
    {SG_CHARGING_DEBOUNCE_RATE_1, SG_CHARGING_UPDATE_THRESHOLD_1}, 
    {SG_CHARGING_DEBOUNCE_RATE_2, SG_CHARGING_UPDATE_THRESHOLD_2}, 
    {SG_CHARGING_DEBOUNCE_RATE_3, SG_CHARGING_UPDATE_THRESHOLD_3}, 
};

static const battery_para_t sc_charge_tables[] = {
    {0, SG_UNCHARGE_FORCE_UPDATE_THRESHOLD}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_0, SG_UNCHARGE_UPDATE_THRESHOLD_0}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_1, SG_UNCHARGE_UPDATE_THRESHOLD_1}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_2, SG_UNCHARGE_UPDATE_THRESHOLD_2}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_3, SG_UNCHARGE_UPDATE_THRESHOLD_3}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_4, SG_UNCHARGE_UPDATE_THRESHOLD_4}, 
};

static const battery_para_t sc_full_load_tables[] = {
    {SG_UNCHARGE_DEBOUNCE_RATE_5, SG_UNCHARGE_UPDATE_THRESHOLD_5}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_6, SG_UNCHARGE_UPDATE_THRESHOLD_6}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_7, SG_UNCHARGE_UPDATE_THRESHOLD_7}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_8, SG_UNCHARGE_UPDATE_THRESHOLD_8}, 
};

static const battery_float_voltage_t sc_charging_float_tables[] = {
    {SG_FLOAT_VOLTAGE_RATE_0, SG_FLOAT_VOLTAGE_THRESHOLD_1}, 
    {SG_FLOAT_VOLTAGE_RATE_1, SG_FLOAT_VOLTAGE_THRESHOLD_2}, 
    {SG_FLOAT_VOLTAGE_RATE_2, SG_FLOAT_VOLTAGE_THRESHOLD_3}, 
};
#else


/// 电池原厂曲线
static const capacity_data_t sc_batt_tables[] = {
#if BATT_CURVE_VERSION == BATT_CURVE_VERSION_1
    {0,  8500, 8999}, 
    {1,  9000, 9499}, 
    {5,  9500, 10088}, 
    {10, 10089, 10378}, 
    {20, 10379, 10609}, 
    {30, 10610, 10843}, 
    {40, 10844, 11141}, 
    {50, 11142, 11379}, 
    {60, 11380, 11617}, 
    {70, 11618, 11937}, 
    {80, 11938, 12019}, 
    {90, 12020, BATT_FULL_VOLTAGE}, 
#elif BATT_CURVE_VERSION == BATT_CURVE_VERSION_2
    {0,  8500, 8999},
    {1,  9000, 9499},
    {5,  9500, 10079},  ///    576
    {10, 10080, 10436}, ///    356
    {20, 10437, 10808}, ///    371
    {30, 10809, 11000}, ///    191
    {40, 11001, 11240}, ///    239
    {50, 11241, 11524}, ///    283
    {60, 11525, 11769}, ///    244
    {70, 11770, 12045}, ///    275
    {80, 12046, 12230}, ///    184
    {90, 12231, BATT_FULL_VOLTAGE}, /// 充电16 分钟
#elif BATT_CURVE_VERSION == BATT_CURVE_VERSION_3
    {0,  8500,  8999},
    {1,  9000,  9975},
    {5,  9976,  10289}, ///    314
    {10, 10290, 10553}, ///    263
    {20, 10554, 10770}, ///    217
    {30, 10771, 10953}, ///    182
    {40, 10954, 11180}, ///    226
    {50, 11181, 11441}, ///    260
    {60, 11442, 11672}, ///    230
    {70, 11673, 11901}, ///    229
    {80, 11902, 12132}, ///    230
    {90, 12133, BATT_FULL_VOLTAGE}, /// 267
#elif BATT_CURVE_VERSION == BATT_CURVE_VERSION_4
    {0,  8500,  8999},
    {1,  9000,  9266},
    {5,  9267,  9799},  ///    576
    {10, 9800,  10369}, ///    570
    {20, 10370, 10671}, ///    302
    {30, 10672, 10865}, ///    194
    {40, 10866, 11083}, ///    217
    {50, 11084, 11357}, ///    274
    {60, 11358, 11633}, ///    276
    {70, 11634, 11869}, ///    235
    {80, 11870, 12121}, ///    251
    {90, 12122, BATT_FULL_VOLTAGE}, /// 278
#endif
};

static const battery_para_t sc_charging_tables[] = {
    {THREE_CHARGING_FORCE_UPDATE_RATE, THREE_CHARGING_FORCE_UPDATE_THRESHOLD}, 
    {THREE_CHARGING_DEBOUNCE_RATE_1, THREE_CHARGING_UPDATE_THRESHOLD_1}, 
    {THREE_CHARGING_DEBOUNCE_RATE_2, THREE_CHARGING_UPDATE_THRESHOLD_2}, 
    {THREE_CHARGING_DEBOUNCE_RATE_3, THREE_CHARGING_UPDATE_THRESHOLD_3}, 
    {THREE_CHARGING_DEBOUNCE_RATE_4, THREE_CHARGING_UPDATE_THRESHOLD_4}, 
    {THREE_CHARGING_DEBOUNCE_RATE_5, THREE_CHARGING_UPDATE_THRESHOLD_5}, 
};

static const battery_para_t sc_charge_tables[] = {
    {THREE_UNCHARGE_FORCE_UPDATE_RATE, THREE_UNCHARGE_FORCE_UPDATE_THRESHOLD}, 
    {THREE_UNCHARGE_DEBOUNCE_RATE_0, THREE_UNCHARGE_UPDATE_THRESHOLD_0}, 
    {THREE_UNCHARGE_DEBOUNCE_RATE_1, THREE_UNCHARGE_UPDATE_THRESHOLD_1}, 
    {THREE_UNCHARGE_DEBOUNCE_RATE_2, THREE_UNCHARGE_UPDATE_THRESHOLD_2}, 
    {THREE_UNCHARGE_DEBOUNCE_RATE_3, THREE_UNCHARGE_UPDATE_THRESHOLD_3}, 
    {THREE_UNCHARGE_DEBOUNCE_RATE_4, THREE_UNCHARGE_UPDATE_THRESHOLD_4}, 
};

static const battery_para_t sc_full_load_tables[] = {
    {THREE_UNCHARGE_DEBOUNCE_RATE_5, THREE_UNCHARGE_UPDATE_THRESHOLD_5}, 
    {THREE_UNCHARGE_DEBOUNCE_RATE_6, THREE_UNCHARGE_UPDATE_THRESHOLD_6}, 
    {THREE_UNCHARGE_DEBOUNCE_RATE_7, THREE_UNCHARGE_UPDATE_THRESHOLD_7}, 
    {THREE_UNCHARGE_DEBOUNCE_RATE_8, THREE_UNCHARGE_UPDATE_THRESHOLD_8}, 
    {THREE_UNCHARGE_DEBOUNCE_RATE_9, THREE_UNCHARGE_UPDATE_THRESHOLD_9}, 
    {THREE_UNCHARGE_DEBOUNCE_RATE_10, THREE_UNCHARGE_UPDATE_THRESHOLD_10}, 
};

static const battery_float_voltage_t sc_charging_float_tables[] = {
    {THREE_FLOAT_VOLTAGE_RATE_0, THREE_FLOAT_VOLTAGE_THRESHOLD_1}, 
    {THREE_FLOAT_VOLTAGE_RATE_1, THREE_FLOAT_VOLTAGE_THRESHOLD_2}, 
    {THREE_FLOAT_VOLTAGE_RATE_2, THREE_FLOAT_VOLTAGE_THRESHOLD_3}, 
};
#endif

static uint8_t  log_timer = 0;
static uint8_t  s_battery_is_ready = 0;
static uint16_t s_charing_voltage_max = 0;
static uint16_t s_batt_voltage_max = 0;
static uint32_t s_charing_time_out_count= 0;
static uint32_t s_batt_poll_time = 0;
static RTC_DATA_ATTR double s_charging_curve_percent = 0;
static RTC_DATA_ATTR battery_status_t battery;
static uint16_t s_batt_full_voltage = BATT_FULL_VOLTAGE;   /// 动态满电电压，初始 BATT_FULL_VOLTAGE，充满时更新，下限 12400 上限 BATT_VOLTAGE_MAX
static battery_health_cb s_battery_health_callback = NULL;
#endif
static rt_device_node_t *node = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_BATTERY_POLL
static void battery_health_update(void) 
{
#if IS_SUPPORT_BATTERY_HEALTH
    if (battery.health < BATT_HEALTH_COEFF_MIN || battery.health > BATT_HEALTH_COEFF_MAX) {
        battery.health = BATT_HEALTH_COEFF_MAX;
    }
#else
    battery.health = BATT_HEALTH_COEFF_MAX;
#endif
    LOG_PBUS("battery health:%d", battery.health);
}

static void add_battery_health_callback(battery_health_cb cb)
{
    s_battery_health_callback = cb;
}

static void battery_health_set(uint16_t coeff)
{
#if IS_SUPPORT_BATTERY_HEALTH
    if (coeff < BATT_HEALTH_COEFF_MIN) {
        coeff = BATT_HEALTH_COEFF_MIN;
    } else if (coeff > BATT_HEALTH_COEFF_MAX) {
        coeff = BATT_HEALTH_COEFF_MAX;
    }

    if (battery.health != (uint8_t)coeff) {
        battery.health = (uint8_t)coeff;
        if (s_battery_health_callback) {
            s_battery_health_callback(battery.health);
        }
    }
#endif
    LOG_PBUS("battery health set:%d/%d", battery.health, coeff);
}

static int battery_pencent_get(void)
{
    return s_battery_is_ready ? battery.percent : 100;
}

static int battery_voltage_get(void)
{
    return s_battery_is_ready ? battery.voltage : BATT_FULL_VOLTAGE;
}

static void battery_percent_handler(uint16_t current_voltage)
{
    uint8_t i = 0, table_size = 0, battery_percent = 0, health = 0;
    uint16_t percent_offset = 0, curr_offset = 0, cmp_val[2] = {0};
    table_size = LENGTH_OF_ARRAY(sc_batt_tables);

    if (battery.health >= BATT_HEALTH_COEFF_MIN && battery.health <= BATT_HEALTH_COEFF_MAX) {
        health = battery.health;
    } else {
        health = BATT_HEALTH_COEFF_MAX;
    }

    cmp_val[0] = (sc_batt_tables[0].vol_min * health) / BATT_HEALTH_COEFF_MAX;
    if (current_voltage < cmp_val[0]) {
        current_voltage = cmp_val[0];
        battery.percent = 0;
        return;
    }

    cmp_val[0] = (s_batt_full_voltage * health) / BATT_HEALTH_COEFF_MAX;
    if (current_voltage >= cmp_val[0]) {
        current_voltage = cmp_val[0];
        battery.percent = 100;
        return;
    }

    for (i = 0; i < table_size; i++) { // 逐段遍历电压-电量映射表，定位当前电压落在哪个区间
        cmp_val[0] = (sc_batt_tables[i].vol_min * health) / BATT_HEALTH_COEFF_MAX; // 计算该段在“当前健康系数”下的最小电压阈值
        cmp_val[1] = ((i == (table_size - 1) ? s_batt_full_voltage : sc_batt_tables[i].vol_max) * health) / BATT_HEALTH_COEFF_MAX; // 计算该段在“当前健康系数”下的最大电压阈值（末段用动态满电电压）
        if (current_voltage >= cmp_val[0] && current_voltage <= cmp_val[1]) { // 命中区间：当前电压在该段上下限之间
            /// 取粗略百分比
            battery_percent = sc_batt_tables[i].percent; // 先取该段起始电量作为粗粒度基值
            /// 取电压范围差值
            if ((i + 1) < table_size) { // 非最后一段：细化范围取“下一段电量 - 当前段电量”
                percent_offset = sc_batt_tables[i + 1].percent - sc_batt_tables[i].percent; // 当前段可细化的电量跨度
            } else { // 最后一段：上限按 100% 封顶
                percent_offset = 100 - sc_batt_tables[i].percent; // 末段可细化跨度 = 100% 到当前段起点
            }
            /// 取细化百分比值，（当前电压-最小值）/（最大值-最小值） * 范围百分比
            if (percent_offset > 1 && cmp_val[1] > cmp_val[0]) { // 仅当该段电量跨度大于 1% 时才做线性插值，避免无意义抖动
                curr_offset = (current_voltage - cmp_val[0]) * percent_offset / (cmp_val[1] - cmp_val[0]); // 在区间内按电压占比换算细分电量偏移
                if (curr_offset <= percent_offset) {             // 防御性保护：确保插值偏移不超过该段允许的最大电量跨度
                    battery.percent = battery_percent + curr_offset; // 粗值 + 细化偏移，得到最终百分比
                }
            } else { // 电量跨度过小（0/1）时不细化，直接使用粗略电量
                battery.percent = battery_percent; // 避免因整数除法导致跳变或噪声
            }
            break; // 已找到匹配区间并完成计算，退出遍历
        }
    }
    
    if (i == table_size) {
        LOGE("voltage table lookup error");     
    }
}

static double battery_voltage_by_percent_get(double percent)
{
    uint8_t table_size = LENGTH_OF_ARRAY(sc_batt_tables);
    uint8_t health = BATT_HEALTH_COEFF_MAX;
    if (battery.health >= BATT_HEALTH_COEFF_MIN && battery.health <= BATT_HEALTH_COEFF_MAX) {
        health = battery.health;
    } else {
        health = BATT_HEALTH_COEFF_MAX;
    }

    if (percent <= sc_batt_tables[0].percent) {
        return ((double)sc_batt_tables[0].vol_min * health) / BATT_HEALTH_COEFF_MAX;
    }

    if (percent >= 100) {
        return ((double)s_batt_full_voltage * health) / BATT_HEALTH_COEFF_MAX;
    }

    for (uint8_t i = 0; i < table_size; i++) {
        /// 电量轴不随 health 缩放，与 battery_percent_handler 只缩放电压阈值保持互逆
        double percent_min = (double)sc_batt_tables[i].percent;
        double percent_max = ((i + 1) < table_size) ?
            (double)sc_batt_tables[i + 1].percent : 100.0;
        uint8_t last = ((i + 1) >= table_size);

        /// 分段右端点归入下一段，使表节点电量对应本段 vol_min
        if (percent >= percent_min && (last || percent < percent_max)) {
            double vol_min = ((double)sc_batt_tables[i].vol_min * health) / BATT_HEALTH_COEFF_MAX;
            double vol_max = ((double)(last ? s_batt_full_voltage : sc_batt_tables[i].vol_max) * health) /
                BATT_HEALTH_COEFF_MAX;
            double percent_range = percent_max - percent_min;
            /// 与正向查表一致：电量跨度 ≤1 不插值，整段都回到 vol_min
            if (percent_range <= 1.0 || vol_max <= vol_min) {
                return vol_min;
            }
            return vol_min + ((percent - percent_min) * (vol_max - vol_min)) / percent_range;
        }
    }

    LOGE("percent table lookup error");
    return battery.voltage;
}

static uint16_t battery_charging_compensation(uint16_t voltage)
{
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(sc_charging_float_tables); i++) {
        if (voltage > sc_charging_float_tables[i].threshold) {
            return voltage - sc_charging_float_tables[i].voltage;
        }
    }
    return voltage;
}

static double charging_current_curve_voltage_get(uint16_t ibat_ma, uint32_t polling_interval, int16_t offset)
{
    uint8_t health = (battery.health >= BATT_HEALTH_COEFF_MIN && battery.health <= BATT_HEALTH_COEFF_MAX) ?
        battery.health : BATT_HEALTH_COEFF_MAX;
    double charge_percent = ((double)ibat_ma * polling_interval * 100) / (BATT_MAX_CAPACITY_MAH * health / BATT_HEALTH_COEFF_MAX);
    if (s_charging_curve_percent < battery.percent || s_charging_curve_percent > 100) {
        s_charging_curve_percent = battery.percent;
    }

    uint16_t force_update_threshold = (ibat_ma * BATT_CHARGE_MAX_RESISTANCE) + CHARGE_FORCE_UPDATE_THRESHOLD_OFFSET; 
    if (offset > force_update_threshold) {
        charge_percent = charge_percent * 20;
        s_charging_curve_percent += charge_percent > 0.025 ? 0.025 : charge_percent; 
    } else {
        s_charging_curve_percent += charge_percent;
    }

    if (s_charging_curve_percent >= 100) {
        s_charging_curve_percent = 100;
    }
    return battery_voltage_by_percent_get(s_charging_curve_percent);
}

/// 放电电压补偿计算（按 ADC 电压，避免放电端虚低滞后减速）
static double uncharge_vref_calculate(double vref_temp, uint16_t voltage, 
                int16_t offset, bool is_full_load)
{
    if (voltage < 8000) {
        return offset;
    }

    if (voltage < 8500) {
        return (offset / 10.f);
    }
    
    if (voltage < 9000) {
        return is_full_load ? (vref_temp * 5) : (vref_temp * 2);
    }

    if (voltage < 9500) {
        return is_full_load ? (vref_temp * 1.6) : (vref_temp * 1.2);
    }

    if (voltage < 10000) {
        return is_full_load ? (vref_temp * 1.05) : (vref_temp * 1.01);
    }
    return vref_temp;
}

static void uncharge_voltage_handler(uint16_t voltage)
{
    static RTC_DATA_ATTR double vref = 0;
    double vref_temp = 0;
    uint8_t vref_flag = 0, uncharge_type = 0;
    uint8_t is_full_load = 0;
    int16_t step = 0;
    int16_t offset = 0;

    /// 充电退出策略
    if (s_charing_voltage_max > 0) {
        s_charing_voltage_max = 0;
        s_charing_time_out_count = 0;
    }

    /// 赋初值
    if (battery.voltage == 0) {
        battery.voltage = voltage;
        uncharge_type |= (1 << 0);
        goto CALC_EXIT;
    }
    
    /// 获取偏差值
    offset = battery.voltage - voltage;

    /// 虚电快速补偿
    if (battery.voltage > s_batt_full_voltage && voltage > s_batt_full_voltage && 
        s_batt_full_voltage >= BATT_FULL_VOLTAGE) {
        vref_temp = offset / 10.f;
        vref_flag = 1;
        uncharge_type |= (1 << 1);
        goto CALC_EXIT;
    }

    /// 获取负载状态
    is_full_load = is_full_load_running();

    /// 满负载补偿
    if (is_full_load) {
        for (uint8_t i = 0; i < LENGTH_OF_ARRAY(sc_full_load_tables); i++) {
            if (offset > sc_full_load_tables[i].dropout_voltage) {
                vref_temp = sc_full_load_tables[i].rate;
                vref_flag = 1;
                break;
            }
        }
        uncharge_type |= (1 << 2);
        goto CALC_EXIT;
    }

    /// 非满负载补偿
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(sc_charge_tables); i++) { 
        if (offset > sc_charge_tables[i].dropout_voltage) {
            vref_temp = sc_charge_tables[i].rate;
            vref_flag = 1;
            uncharge_type |= (1 << 3);
            break;
        }
    }

CALC_EXIT:
    if (vref_flag == 0) { 
        vref = 0; 
    } else {
        vref += uncharge_vref_calculate(vref_temp, voltage, offset, is_full_load);
        if (vref > offset) {
            vref = offset;
        }
    }

    step = (int16_t) vref; 
    if (step) { 
        battery.voltage = battery.voltage - step; 
        vref = vref - step;
    }
    
    if (log_timer > BATT_LOG_INTERVAL || abs(step) > 0) {
        BSP_LOG("uncharge battery", "type:%d/%d, health:%d%%, percent:%d%%, batt:%dmv, adc:%dmv, offset:%d, step:%d, vref:%04f(%04f), load:%d", 
                battery.type, uncharge_type, battery.health, battery.percent, battery.voltage, 
                voltage, offset, step, vref, vref_temp, is_full_load);
    }
    battery_percent_handler(battery.voltage);
    s_charging_curve_percent = battery.percent;
}

/// 充电电压补偿计算（按估算电压，不用 ADC，避免充电端虚高提前加速/减速）
static double charging_vref_calculate(double vref_temp, uint16_t voltage)
{
    if (voltage < 9200) {
        return  (vref_temp * 0.88); // 1.2
    }

    if (voltage < 9500) {
        return  vref_temp * 0.95; // 1.1
    }

    if (voltage < 10000) {
        return vref_temp;
    }

    if (voltage < 11000) {
        return vref_temp;
    }

    if (voltage < 12000) {
        return vref_temp;
    }

    if (voltage < BATT_FULL_VOLTAGE) {
        return (vref_temp * 0.98);
    }

    return (vref_temp * 0.90); 
} 

static void charging_voltage_handler(uint16_t voltage, uint32_t polling_interval)
{
    static double vref = 0;
    double vref_temp = 0;
    uint8_t vref_flag = 0;
    int16_t step = 0;
    int16_t offset = 0;
    int16_t lag_offset = 0;
    uint16_t health_coeff = 0;
    uint8_t table_size = 0, debounce = 0, charge_type = 0;

    /// 充电状态下重启电量补偿
    if (battery.voltage == 0) {
        battery.voltage = battery_charging_compensation(voltage);
    } else if (s_charing_voltage_max == 0) {
        /// 从放电切换到充电：重置估算电压到当前电量对应的开路电压，
        /// 并同步充电曲线百分比基准，避免从放电低电压过度追赶导致电量虚高
        battery.voltage = battery_voltage_by_percent_get(battery.percent);
        s_charging_curve_percent = battery.percent;
    }

    /// 获取充电电压最大值（测试发现异常充电时，高电压只能维持较短时间，不能拿来计算健康状态）
    if (s_charing_voltage_max < voltage) {
        s_charing_voltage_max = voltage;
    }
    
    /// 获取电池电压最大值
    if (s_batt_voltage_max < battery.voltage) {
        s_batt_voltage_max = battery.voltage;
    }

    /// 监测充电超时, 生成电池健康状态(支持健康逆向调整)
    s_charing_time_out_count++;
    if (s_charing_time_out_count > BATT_CHARGING_TIME_OUT) {
        s_charing_time_out_count = 0;
        table_size = LENGTH_OF_ARRAY(sc_batt_tables);
        health_coeff = ((s_batt_voltage_max * 100) / sc_batt_tables[table_size - 1].vol_max);
        if (health_coeff > 0) {
            health_coeff--;
        }
        if ((battery.percent < 100) || (health_coeff > (battery.health + 1))) {
            battery_health_set(health_coeff);
        }
        LOG_PBUS("charging cali, health:%d%%, %d%%, percent:%d%%", health_coeff, battery.health,
                  battery.percent);
    }
    
    /// 充电偏差计算
    offset = voltage - battery.voltage; 
    /// 充电浮压补偿
    lag_offset = (int16_t)battery_charging_compensation(voltage) - (int16_t)battery.voltage;
    uint16_t ibat_ma = 0;
    bsp_charge_get_ibat_ma(&ibat_ma);
    /// 充满快速拉升：在接近“充满判定”阶段，按去抖计数线性放大补偿，尽快把显示电压拉近真实值
    debounce = get_charge_full_debounce(); 
    if (offset > 0 && debounce > BATT_CHARGE_FULL_DEBOUNCE_IGNORE && debounce < BATT_CHARGE_FULL_DEBOUNCE) { 
        vref_temp = ((double)offset * debounce) / (BATT_CHARGE_FULL_DEBOUNCE - BATT_CHARGE_FULL_DEBOUNCE_IGNORE); 
        vref_flag = 1;
        charge_type |= (1 << 0);
        BSP_LOG("charge full", "%f, %d", vref_temp, offset);
        goto CALC_EXIT; // 快速拉升优先级最高，直接进入统一结算逻辑
    }

    if (ibat_ma > 100 && offset > 0) {
        /// 用浮压补偿后的开路电压偏差（估算滞后）判断加速追赶，排除充电端电压虚高的干扰
        double charge_voltage = charging_current_curve_voltage_get(ibat_ma, polling_interval, lag_offset);
        if (offset > THREE_CHARGING_FORCE_UPDATE_THRESHOLD) {
            vref_temp = (offset - THREE_CHARGING_FORCE_UPDATE_THRESHOLD) / 10.f;
            if (vref_temp < THREE_CHARGING_DEBOUNCE_RATE_1) {
                vref_temp = THREE_CHARGING_DEBOUNCE_RATE_1;
            }
            vref_flag = 1;
        } else if (s_charging_curve_percent >= 100) {
            vref_temp = (CHARGE_FULL_100MS_RATE * ibat_ma * polling_interval) / 100.0;
            vref_flag = 1;
        } else {
            if (charge_voltage > battery.voltage) {
                vref_temp = charge_voltage - battery.voltage; 
                vref_flag = 1;
            }
        }
        charge_type |= (1 << 1);
        goto CALC_EXIT;
    } 

    /// 充电偏差计算：小电流时内阻压降已很小，用裸采样偏差回拉，避免浮压补偿把满电锁在 ADC-200mV
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(sc_charging_tables); i++) {
        if (offset > sc_charging_tables[i].dropout_voltage) {
            if (i == 0) {
                uint16_t voltage_diff = offset - sc_charging_tables[0].dropout_voltage;
                vref_temp = voltage_diff > sc_charging_tables[0].rate ? sc_charging_tables[0].rate : voltage_diff;
            } else {
                vref_temp = sc_charging_tables[i].rate;
            }
            vref_flag = 1;
            charge_type |= (1 << 2);
            break;
        }
    }

CALC_EXIT:
    if (vref_flag == 0 || battery.voltage >= BATT_VOLTAGE_MAX) { 
        vref = 0; 
    } else {
        vref += charging_vref_calculate(vref_temp, voltage);
        if (vref > offset) {
            vref = offset;
        }
    }
    step = (int16_t)vref; 
    if (step) { 
        battery.voltage = battery.voltage + step; 
        vref = vref - step; 
    }
    /// 测试日志
    if (log_timer > BATT_LOG_INTERVAL || abs(step) > 0) {
        BSP_LOG("charging battery", "type:%d/%d, health:%d%%, percent:%d%%, batt:%dmv, adc:%dmv, offset:%d, step:%d, vref:%04f, max:%dmv, %dmv, time:%ld", 
                battery.type, charge_type, battery.health, battery.percent, battery.voltage,
                voltage, offset, step, vref, s_charing_voltage_max, 
                s_batt_voltage_max, s_charing_time_out_count);
    }
    battery_percent_handler(battery.voltage);
}

static void charge_full_voltage_handler(uint16_t voltage)
{
    uint16_t health_coeff = 0;
    uint8_t table_size = 0;

    /// 动态更新满电电压：充满时按实际电压更新，下限 BATT_FULL_VOLTAGE，上限 BATT_VOLTAGE_MAX
    uint16_t ibat_ma = 0;
    bsp_charge_get_ibat_ma(&ibat_ma);
    if (voltage > BATT_FULL_VOLTAGE && ibat_ma == 0) {
        uint16_t new_full_voltage = (voltage > BATT_VOLTAGE_MAX) ? BATT_VOLTAGE_MAX : voltage;
        if (new_full_voltage > s_batt_full_voltage) {
            s_batt_full_voltage = new_full_voltage;
        }
    }

    if (battery.voltage < voltage && ibat_ma == 0) {
        battery.voltage = voltage;
        table_size = LENGTH_OF_ARRAY(sc_batt_tables);
        health_coeff = ((voltage * 100) / sc_batt_tables[table_size - 1].vol_max);
        if (health_coeff > 0) {
            health_coeff--; 
        }

        if ((health_coeff != battery.health)) {
            battery_health_set(health_coeff);
        }
    }

    if (log_timer > 20) {
        BSP_LOG("charge full battery", "type:%d, health:%d%%, percent:%d%%, batt:%dmv, adc:%dmv", 
                battery.type, battery.health, battery.percent, battery.voltage, voltage);
    }
    battery_percent_handler(battery.voltage);
    s_charging_curve_percent = 100;
}

static void battery_percent_update(uint8_t percent)
{
    if (percent > 100) {
        LOGE("percent error:%d", percent);
        return;
    }

    /// 显示分辨率只有 1%，电量相同则保留当前估算电压，避免同百分比内反查改压
    if (percent == battery.percent) {
        BSP_LOG("battery", "same percent, skip");
        return;
    }

    uint8_t offset_percent = percent > battery.percent ? 
        (percent - battery.percent) : (battery.percent - percent);
    if (offset_percent > 10) {
        BSP_LOG("percent update", "offset:%d > 10, skip, percent:%d%% -> %d%%", 
            offset_percent, battery.percent, percent);
        return;
    }

    uint8_t last_percent = battery.percent;
    uint16_t last_voltage = battery.voltage;
    battery.percent = percent;
    battery.voltage = (uint16_t)(battery_voltage_by_percent_get(percent) + 0.5);
    s_charging_curve_percent = percent;
    BSP_LOG("percent update", "percent:%d%% -> %d%%, voltage:%dmv -> %dmv",
        last_percent, battery.percent, last_voltage, battery.voltage);
}

static int battery_voltage_poll(void)
{
    uint16_t batt_voltage = 0; 
    static uint16_t voltage = 0; 
    int ret = BSP_OK;
    
    if (s_battery_is_ready == false) {
        return BSP_ERROR;
    }

    uint32_t poll_interval = get_timestamp_ms(s_batt_poll_time);
    /// 避免过快调度
    if (poll_interval < 95) {
        return BSP_OK;
    }

    if (poll_interval > 1000) {
        LOGW("warning: poll interval:%ldms > 1000ms", poll_interval);
        s_batt_poll_time = get_timestamp();
        poll_interval = 100;
    }
    s_batt_poll_time = get_timestamp();
    log_timer++;
#if IS_SUPPORT_BATTERY_DIRECT_SAMPLING
    ret = battery_adc_single_conversion(&voltage);
    if ( ret != BSP_OK) {
        // LOGE("get batt voltage failed");
        return ret;
    }
#else
    voltage = get_charge_battery_voltage();
#endif
    battery.charge = get_charge_state();
    battery.temperature = get_charging_temperature();
    if (battery.charge == CHARGE_STATE_CHARGING) {
        batt_voltage = voltage;
        charging_voltage_handler(batt_voltage, poll_interval);
    } else if (battery.charge == CHARGE_STATE_FULL) {
        batt_voltage = voltage;
        charge_full_voltage_handler(batt_voltage);
    } else {
        batt_voltage = voltage;
        uncharge_voltage_handler(batt_voltage);
    }

    if (log_timer > BATT_LOG_INTERVAL) {
        log_timer = 0;
    }
   
    return BSP_OK;
}
#endif

static int battery_dev_open(struct rt_device_node *dev)
{
    bsp_charge_init();
#if IS_SUPPORT_BATTERY_POLL
#if IS_SUPPORT_BATTERY_DIRECT_SAMPLING
    battery_adc_init();
#endif
    battery_health_update();
    s_battery_is_ready = true;
    LOG_PBUS("battery init ok, health:%d%%, percent: %d%% voltage: %d",
        battery.health, battery.percent, battery.voltage); 
    s_batt_poll_time = 0;   
    battery_voltage_poll();
#endif
    if (node) {
        node->status = DEVICE_BUS_INIT;
    }
    return BSP_OK;
}

static int battery_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int battery_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
#if IS_SUPPORT_BATTERY_POLL
    int val = battery_pencent_get();
    // static uint8_t last_val = 0;
    // static uint32_t last_time = 0;
    // if (get_timestamp_ms(last_time) > 1000 && last_val < 100) {
    //     last_time = get_timestamp();
    //     last_val++;
    // }

    // if (val > (last_val + 6)) {
    //     val = val - last_val;
    // } else {
    //     val = 6;
    // }
    // val = 4;
#else
    int val = 100;
#endif
    if (buf && n_bytes >= sizeof(val)) {
        memcpy(buf, &val, sizeof(int));
    }
    return val;
}

static int battery_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return 0;
}

static int battery_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    uint32_t *tmp = args;
    switch (op_code) {
#if IS_SUPPORT_BATTERY_POLL
        case OP_CODE_BAT_GET_VOLTAGE:
            return battery_voltage_get();
        case OP_CODE_BAT_POLL_VOLTAGE:
            return battery_voltage_poll();
        case OP_CODE_BAT_HEALTH_SET:
            tmp = (uint32_t *)args;
            if (tmp == NULL) {
                return -1;
            }
            battery_health_set((uint16_t)tmp[0]);
            return 0;
#else
        case OP_CODE_BAT_GET_VOLTAGE:
            return BATT_FULL_VOLTAGE;
        case OP_CODE_BAT_POLL_VOLTAGE:
            return 0;
        case OP_CODE_BAT_HEALTH_SET:
            return 0;
#endif
        case OP_CODE_BAT_VBUS_IS_PLUG_IN:
            return vbus_is_plug_in();
        case OP_CODE_BAT_ADD_CHARGE_EVENT_CALLBACK:
            add_charge_event_callback((charge_event_cb)args);
            return 0;
#if IS_SUPPORT_BATTERY_POLL
        case OP_CODE_BAT_ADD_HEALTH_CHANGE_CALLBACK:
            add_battery_health_callback((battery_health_cb)args);
            return 0;
#else
        case OP_CODE_BAT_ADD_HEALTH_CHANGE_CALLBACK:
            return 0;
#endif
        case OP_CODE_BAT_GET_CHARGE_STATUS:
            return get_charge_state();
        case OP_CODE_BAT_GET_CHARGE_TEMPERATURE:
            return get_charging_temperature();
        case OP_CODE_BAT_IS_READY:
            return bsp_charge_is_ready();
        case OP_CODE_BAT_CHARGE_CURRENT_ADJUST:
            tmp = (uint32_t *)args;
            if (tmp == NULL) {
                return -1;
            }
            charge_current_adjust((uint16_t)tmp[0]);
            return 0;
        case OP_CODE_BAT_GET_CHARGE_IBAT_MA: {
            uint16_t *out_ma = (uint16_t *)args;
            if (out_ma == NULL) {
                return -1;
            }
            int r = bsp_charge_get_ibat_ma(out_ma);
            return (r == BSP_OK) ? 0 : -1;
        }
        case OP_CODE_BAT_CHARGE_FORCE_OPEN:
            /// 强制充电
            charge_event_internal_cb(CHARGE_EVENT_FORCE_PLUG_IN);
            return 0;
#if IS_SUPPORT_PD_CHARGE
        case OP_CODE_BAT_PD_IS_READY:
            return bsp_charge_pd_is_ready();
#endif
#if IS_SUPPORT_BATTERY_POLL
        case OP_CODE_BAT_PERCENT_UPDATE:
            tmp = (uint32_t *)args;
            if (tmp == NULL) {
                return -1;
            }
            battery_percent_update((uint8_t)tmp[0]);
            return 0;
#else
        case OP_CODE_BAT_PERCENT_UPDATE:
            return 0;
#endif
        default:
            LOGE("bat err op_code: %lx", op_code);
            return -1;
    }
}

static int battery_dev_suspend(struct rt_device_node *dev)
{
#if IS_SUPPORT_BATTERY_POLL
#if IS_SUPPORT_BATTERY_DIRECT_SAMPLING
    battery_adc_suspend();
#endif
#endif
    bsp_charge_suspend();
    if (node) {
        node->status = DEVICE_BUS_SUSPEND;
    }
    return BSP_OK;
}

static int battery_dev_resume(struct rt_device_node *dev)
{
    bsp_charge_resume();
#if IS_SUPPORT_BATTERY_POLL
#if IS_SUPPORT_BATTERY_DIRECT_SAMPLING
    battery_adc_resume();
#endif
    battery_health_update();
    s_batt_poll_time = 0;
#endif
    if (node) {
        node->status = DEVICE_BUS_RESUME;
    }
    return BSP_OK; 
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void *register_device_for_battery(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "battery";
    node->id = ID_DEVICE_BATTERY;
    node->open = battery_dev_open;
    node->close = battery_dev_close;
    node->write = battery_dev_write;
    node->suspend = battery_dev_suspend;
    node->resume = battery_dev_resume;
    node->read = battery_dev_read;
    node->ioctl = battery_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
