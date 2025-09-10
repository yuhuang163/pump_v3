/*
* INCLUDE FILES
*****************************************************************************************
*/
#include <stdlib.h>
#include "bsp_common.h"
#include "bsp_battery_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
#ifndef RTC_DATA_ATTR
#define RTC_DATA_ATTR
#endif

#define BATT_LOG_INTERVAL                 50
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const capacity_data_t sg_batt_tables[] = {
    {0,  2400, 3149}, ///2.4-3.14V
    {1,  3150, 3299}, ///3.15-3.319V
    {2,  3300, 3399}, ///3.20-3.29V
    {10, 3400, 3491}, ///3.3-3.491V
    {20, 3492, 3533}, ///3.492-3.533V
    {30, 3534, 3570}, ///3.534-3.570V
    {40, 3571, 3614}, ///3.571-3.614V
    {50, 3615, 3678}, ///3.615-3.678V
    {60, 3679, 3771}, ///3.679-3.771V
    {70, 3772, 3873}, ///3.772-3.873V
    {80, 3874, 3976}, ///3.874-3.976V
    {90, 3977, 4160}, ///3.977-4.2V
};

static const battery_para_t sg_charging_tb[] = {
    {0, SG_CHARGING_FORCE_UPDATE_THRESHOLD}, 
    {SG_CHARGING_DEBOUNCE_RATE_1, SG_CHARGING_UPDATE_THRESHOLD_1}, 
    {SG_CHARGING_DEBOUNCE_RATE_2, SG_CHARGING_UPDATE_THRESHOLD_2}, 
    {SG_CHARGING_DEBOUNCE_RATE_3, SG_CHARGING_UPDATE_THRESHOLD_3}, 
};

static const battery_para_t sg_charge_tb[] = {
    {0, SG_UNCHARGE_FORCE_UPDATE_THRESHOLD}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_0, SG_UNCHARGE_UPDATE_THRESHOLD_0}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_1, SG_UNCHARGE_UPDATE_THRESHOLD_1}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_2, SG_UNCHARGE_UPDATE_THRESHOLD_2}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_3, SG_UNCHARGE_UPDATE_THRESHOLD_3}, 
    {SG_UNCHARGE_DEBOUNCE_RATE_4, SG_UNCHARGE_UPDATE_THRESHOLD_4}, 
};

static uint8_t log_timer = 0;
static uint16_t s_charing_voltage_max = 0;
static uint16_t s_batt_voltage_max = 0;
static uint32_t s_charing_time_out_count= 0;
static uint32_t s_batt_poll_time = 0;
static rt_device_node_t *node = NULL;
static RTC_DATA_ATTR battery_status_t battery;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
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
    return battery.percent;
}

static int battery_voltage_get(void)
{
    return battery.voltage;
}

static void battery_percent_handler(uint16_t current_voltage)
{
    uint8_t i = 0, table_size = 0, battery_percent = 0, health = 0;
    uint16_t percent_offset = 0, curr_offset = 0, cmp_val[2] = {0};
    const capacity_data_t *batt_tables = sg_batt_tables;
    table_size = LENGTH_OF_ARRAY(sg_batt_tables);

    if (battery.health > BATT_HEALTH_COEFF_MIN && battery.health < BATT_HEALTH_COEFF_MAX) {
        health = battery.health;
    } else {
        health = BATT_HEALTH_COEFF_MAX;
    }

    cmp_val[0] = (batt_tables[0].vol_min * health) / BATT_HEALTH_COEFF_MAX;
    if (current_voltage < cmp_val[0]) {
        current_voltage = cmp_val[0];
        battery.percent = 0;
        return;
    }

    cmp_val[0] = (batt_tables[table_size - 1].vol_max * health) / BATT_HEALTH_COEFF_MAX;
    if (current_voltage >= cmp_val[0]) {
        current_voltage = cmp_val[0];
        battery.percent = 100;
        return;
    }

    for (i = 0; i < table_size; i++) {
        cmp_val[0] = (batt_tables[i].vol_min * health) / BATT_HEALTH_COEFF_MAX;
        cmp_val[1] = (batt_tables[i].vol_max * health) / BATT_HEALTH_COEFF_MAX;
        if (current_voltage >= cmp_val[0] && current_voltage <= cmp_val[1]) {
            /// 取粗略百分比
            battery_percent = batt_tables[i].percent;
            /// 取电压范围差值
            if ((i + 1) < table_size) {
                percent_offset = batt_tables[i + 1].percent - batt_tables[i].percent; 
            } else {
                percent_offset = 100 - batt_tables[i].percent; 
            }
            /// 取细化百分比值，（当前电压-最小值）/（最大值-最小值） * 范围百分比
            if (percent_offset > 1) {
                curr_offset = (current_voltage - cmp_val[0]) * percent_offset / (cmp_val[1] - cmp_val[0]);
                if (curr_offset <= percent_offset) {             
                    battery.percent = battery_percent + curr_offset;
                }
            } else {
                battery.percent = battery_percent;
            }
            break;
        }
    }
    
    if (i == table_size) {
        LOGE("voltage table lookup error");     
    }
}

static uint16_t battery_charging_compensation(uint16_t voltage)
{
    if (voltage < 3000) {
        return voltage;
    } else if (voltage < 3800) {
        return (voltage - 220);
    } else {
        return (voltage - 150);
    }
}

static void uncharge_voltage_handler(uint16_t voltage)
{
    static RTC_DATA_ATTR double vref = 0;
    static uint8_t comp_count= 0;
    int8_t  step = 0;
    int16_t offset = 0;
    const battery_para_t *charge_tables = sg_charge_tb;

    /// 充电退出策略
    if (s_charing_voltage_max > 0) {
        s_charing_voltage_max = 0;
        s_charing_time_out_count = 0;
    }

    /// 赋初值
    if (battery.voltage == 0) {
        battery.voltage = voltage;
        return;
    }
    
    /// 获取偏差值
    offset = voltage - battery.voltage;

    /// 过滤旋律抖动
    if (bsp_motor_is_running()) {
        if (offset > SG_UNCHARGE_UPDATE_THRESHOLD_5) {
            vref += (offset / SG_UNCHARGE_DEBOUNCE_RATE_5);
        } else if (offset > SG_UNCHARGE_UPDATE_THRESHOLD_6) {
            vref += SG_UNCHARGE_DEBOUNCE_RATE_6;
        } else if (offset > SG_UNCHARGE_UPDATE_THRESHOLD_7) {
            vref += SG_UNCHARGE_DEBOUNCE_RATE_7;
        } else if (offset > SG_UNCHARGE_UPDATE_THRESHOLD_8) {
            vref += SG_UNCHARGE_DEBOUNCE_RATE_8;
        } else {
            vref = 0;
        }
        goto CALC_EXIT;
    }

    /// 大偏差补偿
    if (offset > charge_tables[0].dropout_voltage) {
        vref=0;
        comp_count++; 
        if (comp_count > BATT_FORCE_UPDATE_DEBOUNCE) { 
            battery.voltage = battery.voltage + (offset / 10);
            comp_count = 0;
            LOG_PBUS("charge compensation offset:%d", offset);
        }
    } else if (offset < charge_tables[1].dropout_voltage) {
        comp_count = 0;
        vref += charge_tables[1].rate;
    } else if (offset < charge_tables[2].dropout_voltage) {
        comp_count = 0;
        vref += (offset / charge_tables[2].rate);
    } else if (offset < charge_tables[3].dropout_voltage) {
        comp_count = 0;
        vref += (offset / charge_tables[3].rate);
    } else if (offset < charge_tables[4].dropout_voltage) {
        comp_count = 0;
        vref += (offset / charge_tables[4].rate);
    } else if (offset < charge_tables[5].dropout_voltage) {
        comp_count = 0;
        vref += (offset / charge_tables[5].rate);
    } else {
        comp_count = 0;
        vref = 0;
    }

CALC_EXIT:
    if (vref <= -1) {
        step = (int8_t)vref;
        battery.voltage = battery.voltage + step;
        vref = vref - step;
    }

    if (log_timer > BATT_LOG_INTERVAL || abs(step) > 5) {
        BSP_LOG("uncharge", "type:%d, health:%d%%, percent:%d%%, batt:%dmv, adc:%dmv, offset:%d, step:%d, vref:%04f", 
                battery.type, battery.health, battery.percent, battery.voltage, 
                voltage, offset, step, vref);
    }
    battery_percent_handler(battery.voltage);
}

static void charging_voltage_handler(uint16_t voltage)
{
    static double vref = 0;
    int8_t  step = 0;
    int16_t offset = 0;
    uint8_t health_coeff = 0, table_size = 0, debounce = 0;
    const battery_para_t *charging_tables = sg_charging_tb;

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
        table_size = LENGTH_OF_ARRAY(sg_batt_tables);
        health_coeff = ((s_batt_voltage_max * 100) / sg_batt_tables[table_size - 1].vol_max) - 1;
        if ((battery.percent < 100) || (health_coeff > (battery.health + 1))) {
            battery_health_set(health_coeff);
        }
        LOG_PBUS("charging cali, health:%d%%, %d%%, percent:%d%%", health_coeff, battery.health,
                  battery.percent);
    }

    offset = voltage - battery.voltage;
    if (offset > charging_tables[0].dropout_voltage) {
        vref = 0;
        battery.voltage = battery.voltage + (offset / 2);
        LOG_PBUS("charging compensation offset:%d", offset);
    } else {
        debounce = get_charge_full_debounce();
        if (debounce > 20 && debounce < BATT_CHARGE_FULL_DEBOUNCE) {
            vref = (offset * debounce) / (BATT_CHARGE_FULL_DEBOUNCE -  20);
        } else {
            if (offset > charging_tables[1].dropout_voltage) {
                vref += (offset / charging_tables[1].rate);
            } else if (offset > charging_tables[2].dropout_voltage) {
                vref += charging_tables[2].rate;
            } else if (offset > charging_tables[3].dropout_voltage) {
                vref += charging_tables[3].rate;
            } else {
                vref=0;
            } 
        }
    }

    if (vref >= 1) {
        step = (int8_t)vref;
        battery.voltage = battery.voltage + step;
        vref = vref - step;
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
        table_size = LENGTH_OF_ARRAY(sg_batt_tables);
        health_coeff = ((voltage * 100) / sg_batt_tables[table_size - 1].vol_max) - 1;
        if ((health_coeff != battery.health)) {
            battery_health_set(health_coeff);
        }
    }

    if (log_timer > 20) {
        BSP_LOG("charge full", "type:%d, health:%d%%, percent:%d%%, batt:%dmv, adc:%dmv", 
                battery.type, battery.health, battery.percent, battery.voltage, voltage);
    }
    battery_percent_handler(battery.voltage);
}

void bsp_motor_task_monitor(void);

static int battery_voltage_poll(void)
{
    uint16_t batt_voltage =0; 
    uint16_t voltage = 0, ret = BSP_OK;
    uint32_t poll_interval = get_timestamp() - s_batt_poll_time;
    
    /// 避免过快调度
    if (poll_interval < 80) {
        /// LOGE("battery poll error:%ld", poll_interval);
        return BSP_ERROR;
    }
    bsp_motor_task_monitor();
    
    s_batt_poll_time = get_timestamp();
    log_timer++;
    ret = battery_adc_single_conversion(&voltage);
    if ( ret != BSP_OK) {
        LOGE("get batt voltage failed");
        return ret;
    }

    battery.charge = get_charge_state();
    if (battery.charge == CHARGE_STATE_CHARGING) {
        batt_voltage = voltage;
        charging_voltage_handler(batt_voltage);
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

const battery_status_t *battery_status_get(void)
{
    return &battery;
}

static int battery_dev_open(struct rt_device_node *dev)
{
    battery_adc_init();
    battery_health_update();
    LOG_PBUS("battery init ok, health:%d%%, percent: %d%% voltage: %d",
        battery.health, battery.percent, battery.voltage); 
    s_batt_poll_time = 0;   
    battery_voltage_poll();
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
    int val = battery_pencent_get();

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
        case OP_CODE_BAT_GET_VOLTAGE:
            return battery_voltage_get();

        case OP_CODE_BAT_POLL_VOLTAGE:
            return battery_voltage_poll();

        case OP_CODE_BAT_HEALTH_SET:
            tmp = (uint32_t *)args;
            battery_health_set((uint8_t)tmp[0]);
            return 0;
        case OP_CODE_BAT_CHARGE_IS_PLUG_IN:
            return vbus_is_plug_in();
        case OP_CODE_BAT_CHARGE_ADD_IRQ_CALLBACK:
            add_charge_irq_callback((void (*)(void *))args);
            return 0;
        case OP_CODE_BAT_GET_CHARGE_STATUS:
            return get_charge_state();
        default:
            LOGE("bat err op_code: %lx", op_code);
            return -1;
    }
}

static int battery_dev_suspend(struct rt_device_node *dev)
{
    battery_adc_suspend();
    if (node) {
        node->status = DEVICE_BUS_SUSPEND;
    }
    return BSP_OK;
}

static int battery_dev_resume(struct rt_device_node *dev)
{
    battery_adc_resume();
    battery_health_update();
    s_batt_poll_time = 0;
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
