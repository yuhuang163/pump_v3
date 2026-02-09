/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_led.h"
#include "bsp_common.h" 
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "bsp_led_internal.h"
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
#if DV_DEVICE_ENABLE == DV1_DEVICE_ENABLE
    {led2_address, sizeof(led2_address)},/// PUMP_START_BL
    {led5_address, sizeof(led5_address)},/// PUMP_MODE_BL
    {led6_address, sizeof(led6_address)},/// PUMP_FREQ_SET_BL
    {led1_address, sizeof(led1_address)},/// PUMP_BREASTFEEDING_BL
    {led3_address, sizeof(led3_address)},/// PUMP_LEFT_CONTROL_BL
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
static int led_extension_write_nbyte(i2c_master_data_t *data)
{
    int ret = 0;

    if (s_led_is_ready == 0) {
        LOGE("led extension is not ready");
        return BSP_NOT_READY;
    }

    for (uint8_t i = 0; i < HTR3316_RETRY_COUNT; i++) {
        ret = bsp_i2c_write_byte(LED_I2C_ID, data);
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

static int led_extension_read_nbyte(i2c_master_data_t *data)
{
    int ret = 0;    
    
    if (s_led_is_ready == 0) {
        LOGE("led extension is not ready");
        return BSP_NOT_READY;
    }

    for (uint8_t i = 0; i < HTR3316_RETRY_COUNT; i++) {
        ret = bsp_i2c_read_byte(LED_I2C_ID, data);
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

static void led_extension_onoff_set(uint8_t index, uint8_t onoff)
{
    if (index >= LED_TOTAL_SIZE) {
        LOGE("index is out of range: %d", index);
        return;
    }

    uint8_t level = onoff == LED_STATUS_ON ? LED_LUX_MAX : 0x00;
    i2c_master_data_t i2c_data = {
        .dev_address = HTR3316_DEV_ADDR,
        .reg_addr = 0,
        .data = &level,
        .len = 1,
    };

    for (uint8_t i = 0; i < led_address_data[index].size; i++) {
        i2c_data.reg_addr = led_address_data[index].data[i];
        led_extension_write_nbyte(&i2c_data);
    }
}

static void led_extension_level_set(uint8_t index, uint8_t level)
{
    if (index >= LED_TOTAL_SIZE) {
        LOGE("index is out of range: %d", index);
        return;
    }

    i2c_master_data_t i2c_data = {
        .dev_address = HTR3316_DEV_ADDR,
        .reg_addr = 0,
        .data = &level,
        .len = 1,
    };
 
    for (uint8_t i = 0; i < led_address_data[index].size; i++) {
        i2c_data.reg_addr = led_address_data[index].data[i];
        led_extension_write_nbyte(&i2c_data);
    }
}

static void led_blink_handle(uint8_t index)
{
    if (led_status[index].mode != LED_MODE_BLINK) {
        return;
    }

    led_blink_t *led = &led_status[index].un.blink;
    if (led->times_count >= led->times && led->times != 0) {
        led_status[index].mode = LED_MODE_ONOFF;
        if (led->finish_status != led->status) {
            led_extension_onoff_set(index, led->finish_status != LED_STATUS_ON ? 0 : 1);
        }
        return;
    }

    led->period_count += LED_THREAD_POLL_INTERVAL;
    if (led->period_count >= led->period) {
        led->status = led->status > 0 ? 0 : 1;
        led->period_count = 0;
        led_extension_onoff_set(index, led->status);
        if (led->status == LED_STATUS_ON) {
            led->times_count++;
        }
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
    CREATE_MSG_TYPE_QUEUE(&s_led_device_thd.msg_id, 1, 1);
    CREATE_TIMER(&s_led_timer_id, "led", NULL, OS_TIMER_PERIODIC, led_thread_timer_handle);
    START_TIMER(s_led_timer_id, LED_THREAD_POLL_INTERVAL);
    while (1) {
        MSG_QUEUE_RECV(s_led_device_thd.msg_id, (void *)&msg, OS_WAIT_FOREVER);
        MUTEX_LOCK(s_led_mutex_id);
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_blink_handle(i);
        }
        MUTEX_UNLOCK(s_led_mutex_id);
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
    led_status[index].un.onoff.status = onoff.status;
    led_extension_onoff_set(index, onoff.status);
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

static void led_blink_set(uint8_t index, led_blink_t blink)
{
    MUTEX_LOCK(s_led_mutex_id);
    if (index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            memcpy(&led_status[i].un.blink, &blink, sizeof(led_blink_t));
            led_extension_onoff_set(i, LED_STATUS_OFF);
            led_status[i].un.blink.period_count = 0;
            led_status[i].un.blink.status = LED_STATUS_OFF;
            led_status[i].un.blink.times_count = 0;
            led_status[i].mode = LED_MODE_BLINK;
        }
    } else { 
        memcpy(&led_status[index].un.blink, &blink, sizeof(led_blink_t));
        led_extension_onoff_set(index, LED_STATUS_OFF);
        led_status[index].un.blink.period_count = 0;
        led_status[index].un.blink.status = LED_STATUS_OFF;
        led_status[index].un.blink.times_count = 0;
        led_status[index].mode = LED_MODE_BLINK;
        MUTEX_UNLOCK(s_led_mutex_id);
    }
    MUTEX_UNLOCK(s_led_mutex_id);
}

static void led_htr3316_chip_id_get(uint8_t *chip_id)
{
    int ret = 0;
    i2c_master_data_t i2c_data = {
        .dev_address = HTR3316_DEV_ADDR,
        .reg_addr = 0x10,
        .data = chip_id,
        .len = 1,
    };

    for (uint8_t i = 0; i < HTR3316_RETRY_COUNT; i++) {
        ret = bsp_i2c_read_byte(LED_I2C_ID, &i2c_data);
        if (ret == BSP_OK) {
            break;
        }
        msleep(10);
    }
    LOG_PBUS("led extension chip id: %d", i2c_data.data[0]);
}

static void led_htr3316_init(void)
{
    int ret = 0;
    uint8_t wdata[2] = {0};
    i2c_master_data_t i2c_data = {
        .dev_address = HTR3316_DEV_ADDR,
        .reg_addr = 0,
        .data = wdata,
        .len = 0,
    };
   
    i2c_data.reg_addr = 0x12;
    i2c_data.data[0] = 0;
    i2c_data.len = 1;
    ret = led_extension_write_nbyte(&i2c_data) ;
    if (ret != BSP_OK) {
        LOGE("led P0 mode set failed");
        return;
    }

    i2c_data.reg_addr = 0x13;
    i2c_data.data[0] = 0;
    i2c_data.len = 1;
    ret = led_extension_write_nbyte(&i2c_data) ;
    if (ret != BSP_OK) {
        LOGE("led P1 mode set failed");
        return;
    }

    i2c_data.reg_addr = 0x11;
    i2c_data.data[0] = 2; /// 0:Imax, 1:Imax * 3/4, 2:Imax/2 , 3:Imax/4
    i2c_data.len = 1;
    ret = led_extension_write_nbyte(&i2c_data) ;
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
    i2c_master_data_t i2c_data = {
        .dev_address = HTR3316_DEV_ADDR,
        .reg_addr = 0x7F,
        .data = &reg_value,
        .len = 1,
    };
   
    for (uint8_t i = 0; i < HTR3316_RETRY_COUNT; i++) {
        ret = bsp_i2c_read_byte(LED_I2C_ID, &i2c_data);
        if (ret == BSP_OK) {
            break;
        }
        msleep(10);
    }
    msleep(10);
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

int bsp_led_is_ready(void *args)
{
    uint8_t *led_is_ready_status = (uint8_t *)args;
    *led_is_ready_status = s_led_is_ready;
    return BSP_OK;
}
