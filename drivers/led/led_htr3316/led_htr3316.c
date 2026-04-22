/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" 
#include "bsp_internal.h"
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "bsp_led_internal.h"
#include "hal_led.h"

/*
* DEFINES
*****************************************************************************************
*/
#define LED_TOTAL_SIZE                      (6)
#define LED_THREAD_POLL_INTERVAL            (10)
#define HTR3316_RETRY_COUNT                 (3)
#define HTR3316_DEV_ADDR                    (0x5B)
#define LED_ABNORMAL_MAX_COUNT              (10)
#define LED_LUX_MAX                         (100)

#define HTR3316_REG_OUTPUT_P1               (0x03)
#define HTR3316_REG_INOUT_P1                (0x05)
#define HTR3316_REG_P1_MODE                 (0x13)
#define HTR3316_P1_6_BIT                    (1U << 6)

typedef struct {
    const uint8_t *data;
    uint8_t size;
} led_address_data_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint16_t s_led_abnormal_count = 0;
static bool s_led_is_ready = false;
static uint8_t led_time_is_running = 0;
static led_status_t led_status[LED_TOTAL_SIZE];
static thread_context_t s_led_device_thd;
static os_timer_id s_led_timer_id;
static os_mutex_id s_led_mutex_id = NULL;
/* 记录各路上次已下发亮度，避免重复写同值导致的额外 I2C 负载 */
static uint8_t s_led_hw_level_cache[LED_TOTAL_SIZE];
/* 定时器tick累计，避免队列满导致的节拍丢失 */
static volatile uint16_t s_led_tick_pending = 0;
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
/** 当前已下发的自适应占空比（渐变中）；与 rate 相乘前用于 level */
static uint8_t s_key_adaptive_duty = LED_LEVEL_MAX;
/** 光感算法目标占空比，由 LED 线程每周期向 s_key_adaptive_duty 步进 */
static uint8_t s_key_adaptive_duty_target = LED_LEVEL_MAX;
#endif
static const uint8_t led1_address[] = {
    0x26, 0x27, 0x28   /// PUMP_START_BL
};

static const uint8_t led2_address[] = {
    0x2B, 0x20  /// PUMP_MODE_BL
};

static const uint8_t led3_address[] = {
    0x21, 0x22, 0x23   /// PUMP_FREQ_SET_BL
};

static const uint8_t led4_address[] = {
    0x2C, 0x2D        /// PUMP_BREASTFEEDING_BL
};

static const uint8_t led5_address[] = {
    0x24, 0x25  /// PUMP_LEFT_CONTROL_BL
};

static const uint8_t led6_address[] = {
    0x29, 0x2A  /// PUMP_RIGHT_CONTROL_BL
};

static const led_address_data_t led_address_data[LED_TOTAL_SIZE] = {
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3)
    {led2_address, sizeof(led2_address)},/// PUMP_START_BL
    {led5_address, sizeof(led5_address)},/// PUMP_MODE_BL
    {led6_address, sizeof(led6_address)},/// PUMP_FREQ_SET_BL
    {led1_address, sizeof(led1_address)},/// PUMP_BREASTFEEDING_BL
    {led3_address, sizeof(led3_address)},/// PUMP_LEFT_CONTROL_BL
    {led4_address, sizeof(led4_address)},/// PUMP_RIGHT_CONTROL_BL
#elif (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
    {led3_address, sizeof(led3_address)},/// PUMP_START_BL
    {led5_address, sizeof(led5_address)},/// PUMP_MODE_BL
    {led6_address, sizeof(led6_address)},/// PUMP_FREQ_SET_BL
    {led1_address, sizeof(led1_address)},/// PUMP_BREASTFEEDING_BL
    {led2_address, sizeof(led2_address)},/// PUMP_LEFT_CONTROL_BL
    {led4_address, sizeof(led4_address)},/// PUMP_RIGHT_CONTROL_BL
#else
    {led1_address, sizeof(led1_address)},/// PUMP_START_BL
    {led2_address, sizeof(led2_address)},/// PUMP_MODE_BL
    {led3_address, sizeof(led3_address)},/// PUMP_FREQ_SET_BL
    {led4_address, sizeof(led4_address)},/// PUMP_BREASTFEEDING_BL
    {led5_address, sizeof(led5_address)},/// PUMP_LEFT_CONTROL_BL
    {led6_address, sizeof(led6_address)},/// PUMP_RIGHT_CONTROL_BL
#endif
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int led_extension_write_nbyte(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;

    if (s_led_is_ready == 0) {
        LOGE("led extension is not ready");
        return BSP_NOT_READY;
    }

#if LED_I2C_ID == TOUCH_KEY_I2C_ID
    if (touch_key_is_upgrading()) {
        BSP_LOG("write", "touch key is upgrading");
        return BSP_OK;
    }
#endif

    i2c_master_data_t i2c_data = {
        .dev_address = HTR3316_DEV_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    for (uint8_t i = 0; i < HTR3316_RETRY_COUNT; i++) {
        ret = bsp_i2c_write_byte(LED_I2C_ID, &i2c_data);
        if (ret == BSP_OK) {
            s_led_abnormal_count = 0;
            return BSP_OK;
        }
        msleep(10);
    }

    s_led_abnormal_count++;
    if (s_led_abnormal_count > LED_ABNORMAL_MAX_COUNT) {
        s_led_is_ready = 0;
    }

    return BSP_ERROR;
}

static int led_extension_read_nbyte(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;    
    
    if (s_led_is_ready == 0) {
        LOGE("led extension is not ready");
        return BSP_NOT_READY;
    }

#if LED_I2C_ID == TOUCH_KEY_I2C_ID
    if (touch_key_is_upgrading()) {
        BSP_LOG("read", "touch key is upgrading");
        return BSP_OK;
    }
#endif

    i2c_master_data_t i2c_data = {
        .dev_address = HTR3316_DEV_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    for (uint8_t i = 0; i < HTR3316_RETRY_COUNT; i++) {
        ret = bsp_i2c_read_byte(LED_I2C_ID, &i2c_data);
        if (ret == BSP_OK) {
            s_led_abnormal_count = 0;
            return BSP_OK;
        }
        msleep(10);
    }

    s_led_abnormal_count++;
    if (s_led_abnormal_count > LED_ABNORMAL_MAX_COUNT) {
        s_led_is_ready = 0;
    }

    return BSP_ERROR;
}

static int led_extension_write_nbyte_no_retry(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;

    i2c_master_data_t i2c_data = {
        .dev_address = HTR3316_DEV_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    ret = bsp_i2c_write_byte(LED_I2C_ID, &i2c_data);
    return ret;
}


static int led_extension_read_nbyte_no_retry(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;    
    
    i2c_master_data_t i2c_data = {
        .dev_address = HTR3316_DEV_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    ret = bsp_i2c_read_byte(LED_I2C_ID, &i2c_data);
    return ret;
}

#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
static void led_extension_level_set(uint8_t index, uint8_t level);

static uint8_t key_light_apply_adaptive_level(uint8_t level)
{
    return (uint8_t)(((uint16_t)level * (uint16_t)s_key_adaptive_duty) / 100u);
}

static void led_htr3316_refresh_output(uint8_t index)
{
    if (index >= LED_TOTAL_SIZE) {
        return;
    }
    switch (led_status[index].mode) {
    case LED_MODE_LEVEL:
        led_extension_level_set(index, led_status[index].un.luminous.level);
        break;
    case LED_MODE_ONOFF:
        led_extension_level_set(index,
            led_status[index].un.onoff.status == LED_STATUS_ON ? LED_LUX_MAX : 0);
        break;
    case LED_MODE_BLINK: {
        led_blink_t *led = &led_status[index].un.blink;
        uint8_t lv = (led->status == LED_STATUS_ON) ? LED_LUX_MAX : 0;
        led_extension_level_set(index, lv);
        break;
    }
    default:
        break;
    }
}

void bsp_led_key_adaptive_duty_set(uint8_t duty)
{
    if (duty > LED_LEVEL_MAX) {
        duty = LED_LEVEL_MAX;
    }
    if (s_led_mutex_id) {
        MUTEX_LOCK(s_led_mutex_id);
        s_key_adaptive_duty_target = duty;
        MUTEX_UNLOCK(s_led_mutex_id);
    } else {
        s_key_adaptive_duty_target = duty;
        s_key_adaptive_duty = duty;
    }
}

void bsp_led_key_adaptive_duty_snap(uint8_t duty)
{
    if (duty > LED_LEVEL_MAX) {
        duty = LED_LEVEL_MAX;
    }
    if (s_led_mutex_id == NULL) {
        s_key_adaptive_duty_target = duty;
        s_key_adaptive_duty = duty;
        return;
    }
    MUTEX_LOCK(s_led_mutex_id);
    s_key_adaptive_duty_target = duty;
    s_key_adaptive_duty = duty;
    for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
        led_htr3316_refresh_output(i);
    }
    MUTEX_UNLOCK(s_led_mutex_id);
}

static void key_adaptive_ramp_step_locked(void)
{
    if (s_key_adaptive_duty == s_key_adaptive_duty_target) {
        return;
    }
    if (s_key_adaptive_duty < s_key_adaptive_duty_target) {
        s_key_adaptive_duty++;
    } else {
        s_key_adaptive_duty--;
    }
    for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
        /* BLINK 由 led_blink_handle 驱动节拍，adaptive 渐变阶段不主动刷新 */
        if (led_status[i].mode == LED_MODE_BLINK) {
            continue;
        }
        /*
         * 仅对“亮态”灯做自适应刷新：
         * - ONOFF: 仅 ON 刷新
         * - LEVEL: 仅 level>0 刷新
         * OFF 态不需要随 duty 变化反复写 0，避免无效 I2C 写干扰闪烁节拍。
         */
        if (led_status[i].mode == LED_MODE_ONOFF &&
            led_status[i].un.onoff.status == LED_STATUS_OFF) {
            continue;
        }
        if (led_status[i].mode == LED_MODE_LEVEL &&
            led_status[i].un.luminous.level == 0) {
            continue;
        }
        led_htr3316_refresh_output(i);
    }
}
#endif /* IS_SUPPORT_KEY_LIGHT_ADAPTIVE */

static void led_extension_level_set(uint8_t index, uint8_t level)
{
    if (index >= LED_TOTAL_SIZE) {
        LOGE("index is out of range: %d", index);
        return;
    }

    uint8_t scaled = level;
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
    scaled = key_light_apply_adaptive_level(level);
#endif
    uint8_t new_level = (uint16_t)scaled * led_status[index].rate / 100;
    if (new_level > LED_LEVEL_MAX) {
        new_level = LED_LEVEL_MAX;
    }
    if (s_led_hw_level_cache[index] == new_level) {
        return;
    }

    for (uint8_t i = 0; i < led_address_data[index].size; i++) {
        led_extension_write_nbyte(led_address_data[index].data[i], &new_level, 1);
    }
    s_led_hw_level_cache[index] = new_level;
}

static void led_blink_handle(uint8_t index, uint16_t delta_ms)
{
    if (led_status[index].mode != LED_MODE_BLINK) {
        return;
    }

    led_blink_t *led = &led_status[index].un.blink;
    if (led->times_count >= led->times && led->times != 0) {
        led_status[index].mode = LED_MODE_ONOFF;
        led_status[index].un.onoff.status = led->finish_status;
        if (led->finish_status != led->status) {
            led_extension_level_set(index, 
                led->finish_status == LED_STATUS_ON ? LED_LUX_MAX : 0);
        }
        return;
    }

    if (led->period == 0) {
        return;
    }
    uint32_t acc = (uint32_t)led->period_count + (uint32_t)delta_ms;
    uint32_t toggles = acc / led->period;
    led->period_count = (uint16_t)(acc % led->period);
    if (toggles > 0) {
        uint8_t old_status = led->status;
        uint32_t on_increase = 0;
        if (old_status == LED_STATUS_OFF) {
            on_increase = (toggles + 1u) / 2u;
        } else {
            on_increase = toggles / 2u;
        }
        if (on_increase > 0) {
            uint16_t new_times = (uint16_t)led->times_count + (uint16_t)on_increase;
            led->times_count = (new_times > 0xFFu) ? 0xFFu : (uint8_t)new_times;
        }
        if ((toggles & 0x1u) != 0u) {
            led->status = old_status > 0 ? 0 : 1;
        }
        led_extension_level_set(index,
            led->status == LED_STATUS_ON ? LED_LUX_MAX : 0);
    }
}

static void led_thread_timer_handle(void *arg)
{
    uint8_t msg = 0;
    if (s_led_tick_pending < 0xFFFFu) {
        s_led_tick_pending++;
    }
    MSG_QUEUE_SEND(s_led_device_thd.msg_id, (void *)&msg, 0);
}

static void led_device_thread(void *arg)
{
    uint8_t msg = 0;
    uint16_t ticks = 0;
    CREATE_MSG_TYPE_QUEUE(&s_led_device_thd.msg_id, 1, 1);
    CREATE_TIMER(&s_led_timer_id, "led", NULL, OS_TIMER_PERIODIC, led_thread_timer_handle);
    START_TIMER(s_led_timer_id, LED_THREAD_POLL_INTERVAL);
    while (1) {
        uint16_t delta_ms = 0;
        MSG_QUEUE_RECV(s_led_device_thd.msg_id, (void *)&msg, OS_WAIT_FOREVER);
        ticks = s_led_tick_pending;
        s_led_tick_pending = 0;
        if (ticks == 0) {
            continue;
        }
        if (ticks > (0xFFFFu / LED_THREAD_POLL_INTERVAL)) {
            ticks = (uint16_t)(0xFFFFu / LED_THREAD_POLL_INTERVAL);
        }
        delta_ms = (uint16_t)(ticks * LED_THREAD_POLL_INTERVAL);
        MUTEX_LOCK(s_led_mutex_id);
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_blink_handle(i, delta_ms);
        }
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
        key_adaptive_ramp_step_locked();
#endif
        MUTEX_UNLOCK(s_led_mutex_id);
    }
}

static void led_onoff_set(uint8_t index, led_onoff_t onoff)
{
    if (index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_status[i].mode = LED_MODE_ONOFF;
            led_status[i].un.onoff.status = onoff.status;
            led_extension_level_set(i, onoff.status == LED_STATUS_ON ? LED_LUX_MAX : 0);
        }
        return;
    }
    led_status[index].mode = LED_MODE_ONOFF; 
    led_status[index].un.onoff.status = onoff.status;
    led_extension_level_set(index, onoff.status == LED_STATUS_ON ? LED_LUX_MAX : 0);
}

static void led_level_set(uint8_t index, led_luminous_t luminous)
{
    uint8_t level = luminous.level > LED_LEVEL_MAX ? LED_LEVEL_MAX : luminous.level;
    if (index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_status[i].mode = LED_MODE_LEVEL;
            led_status[i].un.luminous.level = level;
            led_extension_level_set(i, level);
        }
        return;
    }
    led_status[index].mode = LED_MODE_LEVEL; 
    led_status[index].un.luminous.level = level;
    led_extension_level_set(index, level);
}

void bsp_led_key_backlight_uniform_level(uint8_t level)
{
    uint8_t lv = level > LED_LEVEL_MAX ? LED_LEVEL_MAX : level;

    if (s_led_mutex_id == NULL) {
        return;
    }
    MUTEX_LOCK(s_led_mutex_id);
    for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
        /*
         * set_key_backlight 是“统一按键背光”入口：
         * - BLINK: 不覆盖模式，按当前相位立即刷新输出
         * - 非 BLINK: 统一切到 LEVEL 并更新亮度
         */
        if (led_status[i].mode == LED_MODE_BLINK) {
            led_blink_t *blink = &led_status[i].un.blink;
            uint8_t blink_level = (blink->status == LED_STATUS_ON) ? lv : 0;
            led_extension_level_set(i, blink_level);
            continue;
        }
        led_status[i].mode = LED_MODE_LEVEL;
        led_status[i].un.luminous.level = lv;
        led_extension_level_set(i, lv);
    }
    MUTEX_UNLOCK(s_led_mutex_id);
}

static void led_blink_set(uint8_t index, led_blink_t blink)
{
    MUTEX_LOCK(s_led_mutex_id);
    if (index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            memcpy(&led_status[i].un.blink, &blink, sizeof(led_blink_t));
            led_extension_level_set(i, 0);
            led_status[i].un.blink.period_count = 0;
            led_status[i].un.blink.status = LED_STATUS_OFF;
            led_status[i].un.blink.times_count = 0;
            led_status[i].mode = LED_MODE_BLINK;
        }
    } else { 
        memcpy(&led_status[index].un.blink, &blink, sizeof(led_blink_t));
        led_extension_level_set(index, 0);
        led_status[index].un.blink.period_count = 0;
        led_status[index].un.blink.status = LED_STATUS_OFF;
        led_status[index].un.blink.times_count = 0;
        led_status[index].mode = LED_MODE_BLINK;
    }
    MUTEX_UNLOCK(s_led_mutex_id);
}

static void led_htr3316_chip_id_get(uint8_t *chip_id)
{
    int ret = 0;
    for (uint8_t i = 0; i < HTR3316_RETRY_COUNT; i++) {
        ret = led_extension_read_nbyte_no_retry(0x10, chip_id, 1);
        if (ret == BSP_OK) {
            break;
        }
    }
    LOG_PBUS("led extension chip id: %d", *chip_id);
}

static void led_htr3316_init(void)
{
    int ret = 0;
    uint8_t wdata[2] = {0};
    ret = led_extension_write_nbyte(0x12, wdata, 1);
    if (ret != BSP_OK) {
        LOGE("led P0 mode set failed");
        return;
    }

    wdata[0] = 0;
    ret = led_extension_write_nbyte(0x13, wdata, 1);
    if (ret != BSP_OK) {
        LOGE("led P1 mode set failed");
        return;
    }

    wdata[0] = 2; /// 0:Imax, 1:Imax * 3/4, 2:Imax/2 , 3:Imax/4
    ret = led_extension_write_nbyte(0x11, wdata, 1);
    if (ret != BSP_OK) {
        LOGE("led global current set failed");
        return;
    }
    LOG_PBUS("led extension init done");
}

static void led_htr3316_reset(void)
{
    int ret = 0;
    uint8_t reg_value = 0;   
    for (uint8_t i = 0; i < HTR3316_RETRY_COUNT; i++) {
        ret = led_extension_read_nbyte_no_retry(0x7F, &reg_value, 1);
        if (ret == BSP_OK) {
            break;
        }
        msleep(10);
    }
    msleep(10);
}

#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
static int led_htr3316_p16_output_set(bool high_level)
{
    uint8_t reg_val = 0;
    /* Step0: force P1.6 into GPIO mode via P1_Mode (bit6 = 1). */
    if (led_extension_read_nbyte_no_retry(HTR3316_REG_P1_MODE, &reg_val, 1) != BSP_OK) {
        return BSP_ERROR;
    }
    reg_val |= HTR3316_P1_6_BIT;
    //LOG_PBUS("led_htr3316_p16_mode_set: %x", reg_val);
    if (led_extension_write_nbyte_no_retry(HTR3316_REG_P1_MODE, &reg_val, 1) != BSP_OK) {
        return BSP_ERROR;
    }

    /* Step1: force P1.6 into output direction via In/Out_P1 (bit6 = 0). */
    if (led_extension_read_nbyte_no_retry(HTR3316_REG_INOUT_P1, &reg_val, 1) != BSP_OK) {
        return BSP_ERROR;
    }

    reg_val &= (uint8_t)(~HTR3316_P1_6_BIT);
    if (led_extension_write_nbyte_no_retry(HTR3316_REG_INOUT_P1, &reg_val, 1) != BSP_OK) {
        return BSP_ERROR;
    }

    /* Step2: set P1.6 output level via Output_P1 register (0x03). */
    if (led_extension_read_nbyte_no_retry(HTR3316_REG_OUTPUT_P1, &reg_val, 1) != BSP_OK) {
        return BSP_ERROR;
    }

    if (high_level) {
        reg_val |= HTR3316_P1_6_BIT;
    } else {
        reg_val &= (uint8_t)(~HTR3316_P1_6_BIT);
    }

    if (led_extension_write_nbyte_no_retry(HTR3316_REG_OUTPUT_P1, &reg_val, 1) != BSP_OK) {
        return BSP_ERROR;
    }
    return BSP_OK;
}
#endif

static void led_htr3316_level_rate_update(uint8_t index, uint8_t rate)
{
    (void)rate;
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
    led_htr3316_refresh_output(index);
#else
    if (led_status[index].mode == LED_MODE_BLINK) {
        return;
    }
    if (led_status[index].mode == LED_MODE_LEVEL) {
        led_extension_level_set(index, led_status[index].un.luminous.level);
    } else if (led_status[index].mode == LED_MODE_ONOFF) {
        led_extension_level_set(index, led_status[index].un.onoff.status == LED_STATUS_ON ? LED_LUX_MAX : 0);
    }
#endif
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_led_init(void)
{
    uint8_t chip_id = 0;
    if (s_led_mutex_id == NULL) {
        MUTEX_INIT(&s_led_mutex_id, led_mutex);
    }
    bsp_i2c_init(LED_I2C_ID);
    msleep(10);
    led_htr3316_reset();
    led_htr3316_chip_id_get(&chip_id);
    if (chip_id != 0x23) {
        LOGE("chip id error");
        return BSP_ERROR;
    }
    s_led_is_ready = true;
    s_led_abnormal_count = 0;
    led_htr3316_init();
    #if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
        led_htr3316_p16_output_set(false);
        msleep(10);
        led_htr3316_p16_output_set(true);
    #endif
    for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
        led_status[i].rate = 100;
        s_led_hw_level_cache[i] = 0xFF;
    }
    RUN_NEW_THREAD(&s_led_device_thd, "led", led_device_thread,
         RT_PRIO_LED_DEVICE_THREAD, (LED_DEVICE_THREAD_STACKSIZE));
    return BSP_OK;
}

void bsp_led_resume(void)
{
    uint8_t chip_id = 0;
    led_htr3316_reset();
    led_htr3316_chip_id_get(&chip_id);
    if (chip_id != 0x23) {
        LOGE("chip id error");
        return;
    }
    s_led_is_ready = true;
    s_led_abnormal_count = 0;
    led_htr3316_init();
    #if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
        led_htr3316_p16_output_set(false);
        msleep(10);
        led_htr3316_p16_output_set(true);
    #endif
    for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
        s_led_hw_level_cache[i] = 0xFF;
    }
    START_TIMER(s_led_timer_id, LED_THREAD_POLL_INTERVAL);
}

void bsp_led_suspend(void)
{
    s_led_is_ready = false;
    s_led_abnormal_count = 0;
    STOP_TIMER(s_led_timer_id);
    led_htr3316_reset();
}

int bsp_led_status_set(led_status_msg_t *msg)
{
    EXCEPTION_CHECK_INT(msg);
    if (msg->led_index >= LED_TOTAL_SIZE && msg->led_index != PUMP_LED_ALL_ID) {
        LOGE("index is out of range: %d", msg->led_index);
        return BSP_ERROR;
    }

    switch (msg->led_status.mode) {
        case LED_MODE_ONOFF:
            led_onoff_set(msg->led_index, msg->led_status.un.onoff);
            break;

        case LED_MODE_BLINK:
            led_blink_set(msg->led_index, msg->led_status.un.blink);
            break;

        case LED_MODE_LEVEL:
            led_level_set(msg->led_index, msg->led_status.un.luminous);
            break;

        case LED_MODE_BREATH:
            LOGE("LED breathing is not supported");
        break;
    }

    return BSP_OK;
}

int bsp_led_level_rate_update(led_level_rate_msg_t *msg)
{
    EXCEPTION_CHECK_INT(msg);

    if (msg->led_index >= LED_TOTAL_SIZE && msg->led_index != PUMP_LED_ALL_ID) {
        LOGE("index is out of range: %d", msg->led_index);
        return BSP_ERROR;
    }

    if (msg->rate > LED_LEVEL_MAX || msg->rate < 10) {
        LOGE("rate is out of range: %d", msg->rate);
        return BSP_ERROR;
    }

    if (msg->led_index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_status[i].rate = msg->rate;
            led_htr3316_level_rate_update(i, msg->rate);
        }
    } else {
        led_status[msg->led_index].rate = msg->rate;
        led_htr3316_level_rate_update(msg->led_index, msg->rate);
    }
 
    return BSP_OK;
}

int bsp_led_is_ready(void *args)
{
    uint8_t *led_is_ready_status = (uint8_t *)args;
    *led_is_ready_status = s_led_is_ready;
    return BSP_OK;
}

void bsp_touch_rst_pin_output_set(uint8_t high_level)
{
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
    led_htr3316_p16_output_set(high_level > 0 ? true : false);
#endif
}
