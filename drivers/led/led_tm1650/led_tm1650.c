/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" 
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "bsp_led_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
#define LED_TOTAL_SIZE               (6)
#define LED_THREAD_POLL_INTERVAL     (10)
#define TM1650_RETRY_COUNT           3

#define TM1650_COMMAND_ADDRESS       0x48
#define TM1650_DISPLAY_ON            0x01
#define TM1650_7BIT_MODE             0x08

#define TM1650_LEVEL_1               0x10
#define TM1650_LEVEL_2               0x20
#define TM1650_LEVEL_3               0x30
#define TM1650_LEVEL_4               0x40
#define TM1650_LEVEL_5               0x50
#define TM1650_LEVEL_6               0x60
#define TM1650_LEVEL_7               0x70
#define TM1650_LEVEL_8               0
#define TM1650_LEVEL_MAX             8

#define TM1650_ADDR_DIG_MAX          4
#define TM1650_ADDR_DIG1             0x68
#define TM1650_ADDR_DIG2             0x6A
#define TM1650_ADDR_DIG3             0x6C
#define TM1650_ADDR_DIG4             0x6E

#define TM1650_K1_CODE               (1 << 0)
#define TM1650_K2_CODE               (1 << 1)
#define TM1650_K3_CODE               (1 << 2)
#define TM1650_K4_CODE               (1 << 3)
#define TM1650_K5_CODE               (1 << 4)
#define TM1650_K6_CODE               (1 << 5)
#define TM1650_K7_CODE               (1 << 6)
#define TM1650_KP_CODE               (1 << 7)

#define LED_ABNORMAL_MAX_COUNT       10
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
static uint8_t led_dig_data[TM1650_ADDR_DIG_MAX] = {0};
static const uint8_t led_dig_addr[TM1650_ADDR_DIG_MAX] = {
    TM1650_ADDR_DIG1,
    TM1650_ADDR_DIG2,
    TM1650_ADDR_DIG3,
    TM1650_ADDR_DIG4,
};

static const uint8_t led_level_code[TM1650_LEVEL_MAX] = {
    TM1650_LEVEL_1,
    TM1650_LEVEL_2,
    TM1650_LEVEL_3,
    TM1650_LEVEL_4,
    TM1650_LEVEL_5,
    TM1650_LEVEL_6,
    TM1650_LEVEL_7,
    TM1650_LEVEL_8,
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int led_extension_write_nbyte(uint8_t dev_address, uint8_t reg_addr)
{
    int ret = 0;

    if (s_led_is_ready == 0) {
        LOGE("led extension is not ready");
        return BSP_NOT_READY;
    }

    i2c_master_data_t i2c_data = {
        .dev_address = dev_address,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = NULL,
        .len = 0,
    };

    for (uint8_t i = 0; i < TM1650_RETRY_COUNT; i++) {
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

static uint8_t led_extension_level_convert_code(uint8_t level)
{
    if (level >= TM1650_LEVEL_MAX) {
        level = TM1650_LEVEL_MAX - 1;
    }
    return led_level_code[level] | TM1650_DISPLAY_ON;
}

static void led_extension_display(void)
{
    for (uint8_t i = 0; i < TM1650_ADDR_DIG_MAX; i++) {
        led_extension_write_nbyte(led_dig_addr[i] >> 1, led_dig_data[i]);
    }
}

static void led_extension_onoff_set(uint8_t index, uint8_t onoff)
{
    switch (index) { 
        case PUMP_START_BL:
            if (onoff) {
                led_dig_data[0] |= TM1650_K1_CODE;
                led_dig_data[1] |= TM1650_K1_CODE;
            } else {
                led_dig_data[0] &= ~TM1650_K1_CODE;
                led_dig_data[1] &= ~TM1650_K1_CODE;
            }
            break;
        case PUMP_MODE_BL:
            if (onoff) {
                led_dig_data[0] |= TM1650_K2_CODE;
                led_dig_data[1] |= TM1650_K2_CODE;
                led_dig_data[2] |= TM1650_K2_CODE;
            } else {
                led_dig_data[0] &= ~TM1650_K2_CODE;
                led_dig_data[1] &= ~TM1650_K2_CODE;
                led_dig_data[2] &= ~TM1650_K2_CODE;
            }
            break;
        case PUMP_FREQ_SET_BL:
            if (onoff) {
                led_dig_data[0] |= TM1650_K3_CODE;
                led_dig_data[1] |= TM1650_K3_CODE;
            } else {
                led_dig_data[0] &= ~TM1650_K3_CODE;
                led_dig_data[1] &= ~TM1650_K3_CODE;
            }
            break;
        case PUMP_BREASTFEEDING_BL:
            if (onoff) {
                led_dig_data[0] |= TM1650_K4_CODE;
                led_dig_data[1] |= TM1650_K4_CODE;
            } else {
                led_dig_data[0] &= ~TM1650_K4_CODE;
                led_dig_data[1] &= ~TM1650_K4_CODE;
            }
            break;
        case PUMP_LEFT_CONTROL_BL:
            if (onoff) {
                led_dig_data[0] |= TM1650_K5_CODE;
                led_dig_data[1] |= TM1650_K5_CODE;
                led_dig_data[2] |= TM1650_K5_CODE;
            } else {
                led_dig_data[0] &= ~TM1650_K5_CODE;
                led_dig_data[1] &= ~TM1650_K5_CODE;
                led_dig_data[2] &= ~TM1650_K5_CODE;
            }
            break;
        case PUMP_RIGHT_CONTROL_BL:
            if (onoff) {
                led_dig_data[0] |= TM1650_K6_CODE;
                led_dig_data[1] |= TM1650_K6_CODE;
            } else {
                led_dig_data[0] &= ~TM1650_K6_CODE;
                led_dig_data[1] &= ~TM1650_K6_CODE;
            }
            break;
    }
    led_extension_display();
}

static void led_blink_handle(uint8_t index)
{
    if (led_status[index].mode != LED_MODE_BLINK) {
        return;
    }
    led_blink_t *led = &led_status[index].un.blink;
    /// 闪烁完成
    if (get_timestamp_ms(led->period_count) >= led->period && led->period != 0) {
        led_status[index].mode = LED_MODE_ONOFF;
        if (led->finish_status != led->status) {
            led_extension_onoff_set(index, led->finish_status != LED_STATUS_ON ? 0 : 1);
        }
        return;
    }
    /// 闪烁计时
    led->period_count += LED_THREAD_POLL_INTERVAL;
    if (led->period_count >= led->period) {
        led->status = led->status > 0 ? 0 : 1;
        led->period_count = 0;
        led_extension_onoff_set(index, led->status);
    }
}

static void led_thread_timer_handle(void *arg)
{
    uint8_t msg = 0;
    MSG_QUEUE_SEND(s_led_device_thd.msg_id, (void *)&msg, 0);
}

static void led_device_thread(void *arg)
{
    uint8_t msg = 0;
    uint8_t reg_addr = led_extension_level_convert_code(0);
    msleep(20);
    LOG_PBUS("led extension init, reg_addr: %d", reg_addr);
    if (led_extension_write_nbyte(TM1650_COMMAND_ADDRESS >> 1, reg_addr) != BSP_OK) {
        LOGE("led extension init failed");
    }
    CREATE_MSG_TYPE_QUEUE(&s_led_device_thd.msg_id, 1, 1);
    CREATE_TIMER(&s_led_timer_id, "led", NULL, OS_TIMER_PERIODIC, led_thread_timer_handle);
    START_TIMER(s_led_timer_id, LED_THREAD_POLL_INTERVAL);
    while (1) {
        MSG_QUEUE_RECV(s_led_device_thd.msg_id, (void *)&msg, OS_WAIT_FOREVER);
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_blink_handle(i);
        }
    }
}

static void led_onoff_set(uint8_t index, led_onoff_t onoff)
{
    if (index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_status[i].mode = LED_MODE_ONOFF; 
            led_extension_onoff_set(i, onoff.status);
        }
        return;
    }
    led_status[index].mode = LED_MODE_ONOFF; 
    led_extension_onoff_set(index, onoff.status);
}

static void led_level_set(uint8_t index, led_luminous_t luminous)
{
    led_onoff_t onoff = {0};
    onoff.status = luminous.level == 0 ? 0 : 1;
    led_onoff_set(index, onoff);
}

static void led_blink_set(uint8_t index, led_blink_t blink)
{
    if (index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_status[i].mode = LED_MODE_BLINK; 
            memcpy(&led_status[i].un.blink, &blink, sizeof(led_blink_t));
            led_extension_onoff_set(i, LED_STATUS_OFF);
            led_status[i].un.blink.period_count = 0;
            led_status[i].un.blink.status = LED_STATUS_OFF;
            led_status[i].un.blink.times_count = 0;
        }
        return;
    }

    led_status[index].mode = LED_MODE_BLINK; 
    memcpy(&led_status[index].un.blink, &blink, sizeof(led_blink_t));
    led_extension_onoff_set(index, LED_STATUS_OFF);
    led_status[index].un.blink.period_count = 0;
    led_status[index].un.blink.status = LED_STATUS_OFF;
    led_status[index].un.blink.times_count = 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_led_init(void)
{
    int ret = 0;
    
    bsp_i2c_init(LED_I2C_ID);
    s_led_is_ready = true;
    s_led_abnormal_count = 0;
    RUN_NEW_THREAD(&s_led_device_thd, "led", led_device_thread, RT_PRIO_LED_DEVICE_THREAD, (LED_DEVICE_THREAD_STACKSIZE));
    return BSP_OK;
}

void bsp_led_resume(void)
{
    s_led_is_ready = true;
    s_led_abnormal_count = 0;
    START_TIMER(s_led_timer_id, LED_THREAD_POLL_INTERVAL);
}

void bsp_led_suspend(void)
{
    s_led_is_ready = false;
    s_led_abnormal_count = 0;
    STOP_TIMER(s_led_timer_id);
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

int bsp_led_is_ready(void *args)
{
    uint8_t *led_is_ready_status = (uint8_t *)args;
    *led_is_ready_status = s_led_is_ready;
    return BSP_OK;
}
