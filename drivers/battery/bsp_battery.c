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
static const capacity_data_t sc_batt_tables[] = {
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
};

static const battery_para_t sc_charging_tables[] = {
    {0, THREE_CHARGING_FORCE_UPDATE_THRESHOLD}, 
    {THREE_CHARGING_DEBOUNCE_RATE_1, THREE_CHARGING_UPDATE_THRESHOLD_1}, 
    {THREE_CHARGING_DEBOUNCE_RATE_2, THREE_CHARGING_UPDATE_THRESHOLD_2}, 
    {THREE_CHARGING_DEBOUNCE_RATE_3, THREE_CHARGING_UPDATE_THRESHOLD_3}, 
};

static const battery_para_t sc_charge_tables[] = {
    {0, THREE_UNCHARGE_FORCE_UPDATE_THRESHOLD}, 
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
    battery.health = BATT_HEALTH_COEFF_MAX;
    LOG_PBUS("battery health:%d", battery.health);
}

static void battery_health_set(uint8_t coeff)
{
    
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

    if (battery.health > BATT_HEALTH_COEFF_MIN && battery.health < BATT_HEALTH_COEFF_MAX) {
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

    cmp_val[0] = (sc_batt_tables[table_size - 1].vol_max * health) / BATT_HEALTH_COEFF_MAX;
    if (current_voltage >= cmp_val[0]) {
        current_voltage = cmp_val[0];
        battery.percent = 100;
        return;
    }

    for (i = 0; i < table_size; i++) { // 逐段遍历电压-电量映射表，定位当前电压落在哪个区间
        cmp_val[0] = (sc_batt_tables[i].vol_min * health) / BATT_HEALTH_COEFF_MAX; // 计算该段在“当前健康系数”下的最小电压阈值
        cmp_val[1] = (sc_batt_tables[i].vol_max * health) / BATT_HEALTH_COEFF_MAX; // 计算该段在“当前健康系数”下的最大电压阈值
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
            if (percent_offset > 1) { // 仅当该段电量跨度大于 1% 时才做线性插值，避免无意义抖动
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
    if (battery.health > BATT_HEALTH_COEFF_MIN && battery.health < BATT_HEALTH_COEFF_MAX) {
        health = battery.health;
    } else {
        health = BATT_HEALTH_COEFF_MAX;
    }

    if (percent <= sc_batt_tables[0].percent) {
        return ((double)sc_batt_tables[0].vol_min * health) / BATT_HEALTH_COEFF_MAX;
    }

    if (percent >= 100) {
        return ((double)sc_batt_tables[table_size - 1].vol_max * health) / BATT_HEALTH_COEFF_MAX;
    }

    for (uint8_t i = 0; i < table_size; i++) {
        double percent_min = ((double)sc_batt_tables[i].percent * health) / BATT_HEALTH_COEFF_MAX;
        double percent_max = ((i + 1) < table_size) ? ((double)sc_batt_tables[i + 1].percent * health) / BATT_HEALTH_COEFF_MAX : 100;

        if (percent >= percent_min && percent <= percent_max) {
            uint16_t vol_min = ((double)sc_batt_tables[i].vol_min * health) / BATT_HEALTH_COEFF_MAX;
            uint16_t vol_max = ((double)sc_batt_tables[i].vol_max * health) / BATT_HEALTH_COEFF_MAX;
            double percent_range = percent_max - percent_min;
            double voltage = vol_min;
            if (percent_range > 0) {
                voltage += ((percent - percent_min) * (vol_max - vol_min)) / percent_range;
            }
            return voltage;
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
    double charge_percent = ((double)ibat_ma * polling_interval * 100) / (BATT_MAX_CAPACITY_MAH * battery.health / BATT_HEALTH_COEFF_MAX);
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

static void uncharge_voltage_handler(uint16_t voltage)
{
    static RTC_DATA_ATTR double vref = 0;
    static uint8_t comp_count= 0;
    uint8_t vref_flag = 0;
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
        goto CALC_EXIT;
    }
    
    /// 获取偏差值
    offset = battery.voltage - voltage;

    /// 过滤旋律抖动
    if (is_full_load_running()) {
        for (uint8_t i = 0; i < LENGTH_OF_ARRAY(sc_full_load_tables); i++) {
            if (offset > sc_full_load_tables[i].dropout_voltage) {
                vref += (i == 0) ? (offset / sc_full_load_tables[i].rate) : sc_full_load_tables[i].rate;
                vref_flag = 1;
                break;
            }
        }
        goto CALC_EXIT;
    }

    /// 大偏差补偿：在“非满负载”放电场景下，按分段阈值对估算电压做回拉，避免显示电压长期高于真实采样
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(sc_charge_tables); i++) { // 依次遍历放电补偿分段表，寻找当前偏差命中的第一段
        if (offset > sc_charge_tables[i].dropout_voltage) { // 当“估算值-采样值”超过该段门限时，触发该段补偿策略
            if (i == 0) { // 第一段：偏差极大，先走防抖强制校正，不叠加 vref 渐进补偿
                comp_count++; // 连续大偏差计数 +1，用于去抖判定（防止瞬态尖峰误触发）
                if (comp_count > BATT_FORCE_UPDATE_DEBOUNCE) { // 大偏差持续到达去抖阈值后，执行一次强制回拉
                    vref = offset / 10.f;
                    comp_count = 0; // 触发后清计数，重新开始下一轮去抖
                    LOG_PBUS("uncharge compensation offset:%d", offset); // 记录强制补偿触发时的偏差值，便于参数调优
                } else {
                    vref = 0; // 清掉渐进补偿残差，避免与强制校正叠加导致跳变
                }
            } else{ // 非第一段：偏差较温和，采用 vref 累积 + step 离散更新的平滑策略
                comp_count = 0; // 进入平滑段时清除强制补偿计数，避免跨段误触发
                vref += (i == 1) ? sc_charge_tables[i].rate : (offset / sc_charge_tables[i].rate); // 第二段按固定增量，其后按比例增量，控制收敛速度
            }
            vref_flag = 1; // 标记“本轮命中补偿段”，后续结算阶段不清空 vref
            break; // 分段表按优先级匹配到首段后即退出，避免多段叠加
        }
    }

CALC_EXIT:
    if (vref_flag == 0) { // 本轮未命中任何补偿段，说明偏差不大或无需调整
        vref = 0; // 清除浮点残差，避免历史累积引入无效微调
        comp_count = 0; // 同步清除强制补偿计数，避免旧计数污染后续判定
    } 

    step = (int16_t) vref; // 将浮点补偿量离散为本轮可执行的整数步进（mV 级）
    if (step) { // 仅当存在有效整数步进时更新，0 步进则保持当前估算值
        battery.voltage = battery.voltage - step; // 在放电补偿路径中按 step 下调估算电压（回拉到采样侧）
        vref = vref - step; // 保留小数残差到下一轮继续累计，降低量化误差
    }
    
    if (log_timer > BATT_LOG_INTERVAL || abs(step) > 5) {
        BSP_LOG("uncharge", "type:%d, health:%d%%, percent:%d%%, batt:%dmv, adc:%dmv, offset:%d, step:%d, vref:%04f", 
                battery.type, battery.health, battery.percent, battery.voltage, 
                voltage, offset, step, vref);
    }
    battery_percent_handler(battery.voltage);
    s_charging_curve_percent = battery.percent;
}

static void charging_voltage_handler(uint16_t voltage, uint32_t polling_interval)
{
    static double vref = 0;
    uint8_t vref_flag = 0;
    int16_t step = 0;
    int16_t offset = 0;
    uint8_t health_coeff = 0, table_size = 0, debounce = 0;

    /// 充电状态下重启电量补偿
    if (battery.voltage == 0) {
        battery.voltage = battery_charging_compensation(voltage);
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
        health_coeff = ((s_batt_voltage_max * 100) / sc_batt_tables[table_size - 1].vol_max) - 1;
        if ((battery.percent < 100) || (health_coeff > (battery.health + 1))) {
            battery_health_set(health_coeff);
        }
        LOG_PBUS("charging cali, health:%d%%, %d%%, percent:%d%%", health_coeff, battery.health,
                  battery.percent);
    }

    offset = voltage - battery.voltage; // 当前采样电压与内部电池电压估计值的差值（正值表示需要上调）

    /// 充满快速拉升：在接近“充满判定”阶段，按去抖计数线性放大补偿，尽快把显示电压拉近真实值
    debounce = get_charge_full_debounce(); // 获取“充满”状态去抖计数，计数越大说明越接近稳定充满
    if (debounce > 20 && debounce < BATT_CHARGE_FULL_DEBOUNCE) { // 仅在快速拉升窗口内生效，避免刚充电和完全充满时突变
        vref = (offset * debounce) / 1.0 / (BATT_CHARGE_FULL_DEBOUNCE -  20); // 依据去抖进度计算本轮浮点补偿量（除 1.0 保持浮点计算）
        BSP_LOG("charge full", "%f, %d", vref, offset);
        goto CALC_EXIT; // 快速拉升优先级最高，直接进入统一结算逻辑
    }
    
    uint16_t ibat_ma = 0;
    bsp_charge_get_ibat_ma(&ibat_ma);
    if (ibat_ma > 200 && offset > 0) {
        double charge_voltage = charging_current_curve_voltage_get(ibat_ma, polling_interval, offset);
        if (s_charging_curve_percent >= 100) {
            vref += offset / 1000.f;
        } else {
            if (charge_voltage > battery.voltage) {
                vref = charge_voltage - battery.voltage; 
            } 
        }
        BSP_LOG("charging", "%f/%f/%f, %d ,%d", vref, s_charging_curve_percent, charge_voltage, battery.voltage, offset);
        goto CALC_EXIT;
    }

    /// 充电偏差计算：按分段表逐级叠加补偿，让电压变化更平滑（避免一次性跳变）
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(sc_charging_tables); i++) { // 依次遍历每个补偿分段阈值
        if (offset > sc_charging_tables[i].dropout_voltage) { // 当前偏差超过该段门限，说明该段补偿应生效
            if (i == 0) { // 第一段：主补偿段，按偏差一半快速跟随
                vref = offset / 100.f; // 先给一个较大基准补偿，快速收敛
                LOG_PBUS("charging compensation offset:%d", offset); // 记录进入补偿的偏差值，便于调参
            } else { // 后续段：继续叠加细调，控制收敛速度和稳定性
                vref += (i == 1) ? (offset / sc_charging_tables[i].rate) : sc_charging_tables[i].rate; // 第二段按比例补偿，其它段按固定步进补偿
            }
            vref_flag = 1; // 标记“本轮至少有一个分段命中”，后续不清零 vref
        }
    }

    if (vref_flag == 0) { // 没有命中任何补偿段，说明偏差很小或无需调整
        vref = 0; // 清空浮点残差，防止历史残量造成无意义抖动
    } 

CALC_EXIT:
    step = (int16_t)vref; // 取整数步长作为本轮真正写入的电压修正量
    if (step) { // 仅在有有效整数步进时更新，避免无效写入
        battery.voltage = battery.voltage + step; // 应用本轮补偿到内部电压估计
        vref = vref - step; // 保留小数残差到下轮累计，减少量化误差
    }

    /// 测试日志
    if (log_timer > BATT_LOG_INTERVAL || abs(step) > 5) {
        BSP_LOG("charging", "type:%d, health:%d%%, percent:%d%%, batt:%dmv, adc:%dmv, offset:%d, step:%d, vref:%04f, max:%dmv, %dmv, time:%ld", 
                battery.type, battery.health, battery.percent, battery.voltage,
                voltage, offset, step, vref, s_charing_voltage_max, s_batt_voltage_max, s_charing_time_out_count);
    }
    battery_percent_handler(battery.voltage);
}

static void charge_full_voltage_handler(uint16_t voltage)
{
    uint8_t health_coeff = 0, table_size = 0;
    if (battery.voltage < voltage) {
        battery.voltage = voltage;
        table_size = LENGTH_OF_ARRAY(sc_batt_tables);
        health_coeff = ((voltage * 100) / sc_batt_tables[table_size - 1].vol_max) - 1;
        if ((health_coeff != battery.health)) {
            battery_health_set(health_coeff);
        }
    }

    if (log_timer > 20) {
        BSP_LOG("charge full", "type:%d, health:%d%%, percent:%d%%, batt:%dmv, adc:%dmv", 
                battery.type, battery.health, battery.percent, battery.voltage, voltage);
    }
    battery_percent_handler(battery.voltage);
    s_charging_curve_percent = 100;
}

static int battery_voltage_poll(void)
{
    uint16_t batt_voltage =0; 
    static uint16_t voltage = 0; 
    int ret = BSP_OK;
    
    if (s_battery_is_ready == false) {
        return BSP_ERROR;
    }

    uint32_t poll_interval = get_timestamp_ms(s_batt_poll_time);
    
    /// 避免过快调度
    if (poll_interval < 80) {
        /// LOGE("battery poll error:%ld", poll_interval);
        return BSP_ERROR;
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
    if (buf) {
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
            battery_health_set((uint8_t)tmp[0]);
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
        case OP_CODE_BAT_GET_CHARGE_STATUS:
            return get_charge_state();
        case OP_CODE_BAT_GET_CHARGE_TEMPERATURE:
            return get_charging_temperature();
        case OP_CODE_BAT_IS_READY:
            return bsp_charge_is_ready();
        case OP_CODE_BAT_PD_IS_MISMATCH:
#if IS_SUPPORT_PD_CHARGE
            return bsp_charge_pd_is_mismatch();
#else
            return 0;
#endif
        case OP_CODE_BAT_CHARGE_CURRENT_ADJUST:
            tmp = (uint32_t *)args;
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
        case OP_CODE_BAT_SET_CHARGE_CURRENT:
            tmp = (uint32_t *)args;
            /// 强制充电
            charge_event_internal_cb(CHARGE_EVENT_PLUG_IN);
            return 0;
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
