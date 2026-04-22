/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_config.h"
#include "bsp_common.h"
#include "bsp_internal.h"
#include "bsp_i2c.h"
#include "bsp_hyn.h"
#include "bsp_key_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HYN_TOUCH_KEY_DEBUG            (0) /// 触摸按键调试开关
#define HYN_KEY_IDLE_COUNT             (3)

#if HYN_TOUCH_USE_POLL
#define HYN_POLL_PERIOD_MS             (20)  /// 轮询周期，IRQ 不触发时靠轮询保证触摸有响应
#endif

#ifndef HYN_IRQ_USE_RISING_EDGE
#define HYN_IRQ_USE_RISING_EDGE        (0) /// 是否使用上升沿触发
#endif

typedef enum {
    TOUCH_KEY_THREAD_MSG_INTERRUPT = 0,  /// 中断消息
    TOUCH_KEY_THREAD_MSG_MAX,
} touch_key_thread_msg_e;

typedef enum {
    TOUCH_OTA_THREAD_MSG_UPDATE_START = 0,   /// 启动升级
    TOUCH_OTA_THREAD_MSG_MAX,
} touch_ota_thread_msg_e;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint16_t s_touch_abnormal_count = 0;
#if HYN_TOUCH_USE_POLL
static uint16_t s_last_poll_key_state = 0;
static os_timer_id s_poll_timer_id = NULL;
#endif
#if HYN_ENABLE_DIFF_SIGNAL_DUMP
static os_timer_id s_diff_signal_timer_id = NULL;
#endif
static bool s_touch_is_ready = false;
static uint8_t s_touch_fw_version = 0;
static thread_context_t s_key_device_thd = {0};
static thread_context_t s_touch_ota_thd = {0};
static uint8_t s_touch_key[HYN_TOUCH_KEY_NUM] = {0};
static uint16_t s_key_state = 0;
static uint16_t s_key_last_state = 0;
static bool s_is_upgrading = false;
static key_ota_event_cb_func s_key_ota_event_cb = NULL;
/// 0x22 寄存器位定义: 与硬件/丝印一致
static const uint16_t s_key_mask[HYN_TOUCH_KEY_NUM] = {
    0x02,  /// 启动键
    0x20,  /// 模式键
    0x08,  /// 频段键
    0x10,  /// 追奶键
    0x01,  /// 左按键
    0x04,  /// 右按键
};
 
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int hyn_i2c_write(uint8_t reg_addr, const uint8_t *buf, uint16_t len)
{
    i2c_master_data_t data = {
        .dev_address = HYN_TOUCH_I2C_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = (uint8_t *)buf,
        .len = len,
    };
    return bsp_i2c_write_byte(TOUCH_KEY_I2C_ID, &data);
}

static int hyn_i2c_write_with_retry(uint8_t reg_addr, const uint8_t *buf, uint16_t len, uint8_t retry_count)
{
    for (uint8_t i = 0; i < retry_count; i++) {
        if (hyn_i2c_write(reg_addr, buf, len) == BSP_OK) {
            return BSP_OK;
        }
        msleep(10);
    }
    return BSP_ERROR;
}

static int hyn_i2c_read(uint8_t reg_addr, uint8_t *buf, uint16_t len)
{
    i2c_master_data_t data = {
        .dev_address = HYN_TOUCH_I2C_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = buf,
        .len = len,
    };
    return bsp_i2c_read_byte(TOUCH_KEY_I2C_ID, &data);
}

static int hyn_i2c_read_with_retry(uint8_t reg_addr, uint8_t *buf, uint16_t len, uint8_t retry_count)
{
    for (uint8_t i = 0; i < retry_count; i++) {
        if (hyn_i2c_read(reg_addr, buf, len) == BSP_OK) {
            return BSP_OK;
        }
        msleep(10);
    }
    return BSP_ERROR;
}

static int hyn_i2c_write_and_check(uint8_t reg_addr, const uint8_t *buf, uint16_t len)
{
    int ret = hyn_i2c_write(reg_addr, buf, len);
    if (ret != BSP_OK) {
        return BSP_ERROR;
    }
    msleep(2);
    uint8_t read_buf[4];
    if (len > sizeof(read_buf)) {
        return BSP_OK;
    }
    ret = hyn_i2c_read(reg_addr, read_buf, len);
    if (ret != BSP_OK) {
        return BSP_ERROR;
    }
    for (uint16_t i = 0; i < len; i++) {
        if (read_buf[i] != buf[i]) {
            return BSP_ERROR;
        }
    }
    return BSP_OK;
}

static int hyn_init_csk14t(void)
{
    uint8_t data_buff[4] = {0};
    int retry;

    for (retry = 0; retry < 10; retry++) {
        LOG_PBUS("HYN CSK14T init retry %d", retry);
        bsp_touch_key_reset();
        msleep(50 + retry * 2);

        data_buff[0] = 0x3A;
        if (hyn_i2c_write_and_check(0xAF, data_buff, 1) != BSP_OK) {
            continue;
        }
        data_buff[0] = 0x00;
        if (hyn_i2c_write_and_check(0x1F, data_buff, 1) != BSP_OK) {
            continue;
        }
        msleep(20);

        data_buff[0] = 0x01;
        if (hyn_i2c_write_and_check(0x17, data_buff, 1) != BSP_OK) {
            continue;
        }
        data_buff[0] = 0x08;
        if (hyn_i2c_write_and_check(0x12, data_buff, 1) != BSP_OK) {
            continue;
        }
        data_buff[0] = 0xFF;
        data_buff[1] = 0x3F;
        if (hyn_i2c_write_and_check(0x14, data_buff, 2) != BSP_OK) {
            continue;
        }
        data_buff[0] = 0x1E;
        if (hyn_i2c_write_and_check(0x1A, data_buff, 1) != BSP_OK) {
            continue;
        }
        data_buff[0] = 0x64;
        if (hyn_i2c_write_and_check(0x1E, data_buff, 1) != BSP_OK) {
            continue;
        }
        data_buff[0] = 0xFF;
        data_buff[1] = 0x3F;
        if (hyn_i2c_write_and_check(0xB0, data_buff, 2) != BSP_OK) {
            continue;
        }
        data_buff[0] = 0x68;
        if (hyn_i2c_write_and_check(0xEE, data_buff, 1) != BSP_OK) {
            continue;
        }
        data_buff[0] = 0x20;
        if (hyn_i2c_write_and_check(0x2F, data_buff, 1) != BSP_OK) {
            continue;
        }
        data_buff[0] = 0x01;
        data_buff[1] = 0x68;
        if (hyn_i2c_write_and_check(0x5E, data_buff, 2) != BSP_OK) {
            continue;
        }
        break;
    }

    if (retry >= 10) {
        LOGE("HYN CSK14T init fail");
        return BSP_ERROR;
    }

    for (retry = 0; retry < 10; retry++) {
        data_buff[0] = 0x41;
        if (hyn_i2c_write_and_check(0x1F, data_buff, 1) == BSP_OK) {
            LOG_PBUS("HYN CSK14T init OK");
            return BSP_OK;
        }
    }
    return BSP_ERROR;
}

static uint8_t hyn_read_fw_version(void)
{
    uint8_t read_buff[1] = {0};

    if (hyn_i2c_read(0xFF, read_buff, 1) != BSP_OK) {
        read_buff[0] = 0;
        LOGE("HYN reg 0xFF ERROR!!!");
    }
    s_touch_fw_version = read_buff[0];
    LOG_PBUS("HYN reg 0xFF(read_fw): 0x%02x", (unsigned)read_buff[0]);
    return s_touch_fw_version;
}

static void hyn_irq_callback(enum HAL_GPIO_PIN_T pin)
{
#if HYN_TOUCH_KEY_DEBUG
    BSP_LOG_PASS("HYN irq pin=%d\n", (int)pin);
#endif
    if (pin == TOUCH_INT_PIN && s_key_device_thd.msg_id) {
        uint8_t msg = TOUCH_KEY_THREAD_MSG_INTERRUPT;
        MSG_QUEUE_SEND(s_key_device_thd.msg_id, &msg, 0);
        bsp_irq_msg_send(INT_NUM_TOUCH_KEY);
    }
} 

static void hyn_irq_control(bool enable)
{
    /* INT 脚为约 1ms 低脉冲，边沿触发易漏检；用电平触发在低电平期间可靠触发 */
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable,
        0,
        HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE,
#if HYN_IRQ_USE_RISING_EDGE
        HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
#else
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING,
#endif
        hyn_irq_callback,
    };
    hal_gpio_setup_irq(TOUCH_INT_PIN, &gpio_irq_cfg);
}
 
#if HYN_TOUCH_USE_POLL
/** 轮询定时器回调：IRQ 未触发时通过定期读 0x22 驱动按键，保证触摸有响应 */
static void hyn_poll_timer_cb(const void *arg)
{
    uint8_t read_buff[2] = {0};
    (void)arg;

    if (s_is_upgrading) {
        return;
    }

    if (hyn_i2c_read(0x22, read_buff, 2) != BSP_OK) {
        return;
    }
    s_key_state = (uint16_t)(read_buff[1] << 8) | read_buff[0];
    if (s_key_state != s_last_poll_key_state) {
        s_last_poll_key_state = s_key_state;
        for (uint8_t i = 0; i < HYN_TOUCH_KEY_NUM; i++) {
            s_touch_key[i] = (s_key_state & s_key_mask[i]) ? 1 : 0;
        }
        LOGE("HYN poll 0x22=0x%04x", (unsigned)s_key_state);
        key_irq_event_callback(-1);
    }
}
#endif

#if HYN_ENABLE_DIFF_SIGNAL_DUMP
/** 订制品(CST812T/CST820 等): 0xC8 读各路信号量，每键 2 字节大端 */
static void hyn_diff_signal_dump_once(void)
{
    uint8_t sig_buff[HYN_TOUCH_KEY_NUM * 2];

    if (hyn_i2c_read(0xC8, sig_buff, (uint16_t)(HYN_TOUCH_KEY_NUM * 2)) != BSP_OK) {
        return;
    }
    LOG_PBUS("HYN diff 0xC8: %05u %05u %05u %05u %05u %05u",
            (unsigned)(((uint16_t)sig_buff[0] << 8) | sig_buff[1]),
            (unsigned)(((uint16_t)sig_buff[2] << 8) | sig_buff[3]),
            (unsigned)(((uint16_t)sig_buff[4] << 8) | sig_buff[5]),
            (unsigned)(((uint16_t)sig_buff[6] << 8) | sig_buff[7]),
            (unsigned)(((uint16_t)sig_buff[8] << 8) | sig_buff[9]),
            (unsigned)(((uint16_t)sig_buff[10] << 8) | sig_buff[11]));
}

static void hyn_diff_signal_timer_cb(const void *arg)
{
    (void)arg;
    if (!s_touch_is_ready || s_is_upgrading) {
        return;
    }
    hyn_diff_signal_dump_once();
}
#endif

static void hyn_irq_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)TOUCH_INT_PIN,
        HAL_IOMUX_FUNC_AS_GPIO,
        HAL_IOMUX_PIN_VOLTAGE_VIO,
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(TOUCH_INT_PIN, HAL_GPIO_DIR_IN, 1);
}

static void hyn_rst_pin_init(void)
{
#if IS_SUPPORT_TOUCH_KEY_RESET_PIN
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)HYN_TOUCH_RST_PIN,
        HAL_IOMUX_FUNC_AS_GPIO,
        HAL_IOMUX_PIN_VOLTAGE_VIO,
        HAL_IOMUX_PIN_NOPULL,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(HYN_TOUCH_RST_PIN, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set(HYN_TOUCH_RST_PIN);
#endif
}

static void hyn_touch_key_scan(void)
{
    uint8_t read_buff[5] = {0};
    uint8_t clear_irq;
    uint16_t key_idle_count = 0;
    while (1) {
        memset(read_buff, 0, sizeof(read_buff));
        hyn_i2c_read_with_retry(0x1F, read_buff, sizeof(read_buff), 3);
        if ((read_buff[0] & 0x01) != 0x01) {
            msleep(10);
            BSP_LOG_PASS("no key irq\n");
            continue;
        }
        clear_irq = read_buff[0] & (~0x02);
        s_key_state = (uint16_t)(read_buff[4] << 8) | read_buff[3];
        hyn_i2c_write_with_retry(0x1F, &clear_irq, 1, 3);
        for (uint8_t i = 0; i < HYN_TOUCH_KEY_NUM; i++) {
            s_touch_key[i] = (s_key_state & s_key_mask[i]) ? 1 : 0;
        }

        if (s_key_state != s_key_last_state) {
            s_key_last_state = s_key_state;
#if HYN_TOUCH_KEY_DEBUG
            BSP_LOG_PASS("%lu:%04x->%04x\n", get_timestamp(), 
                (unsigned)s_key_last_state, (unsigned)s_key_state);
#endif
        }

        if (s_key_state == 0) {
            key_idle_count++;
            if (key_idle_count > HYN_KEY_IDLE_COUNT) {
                break;
            }
        } else {
            key_idle_count = 0;
        }
        msleep(10);
    }
}

static void key_device_thread(void *arg)
{
    uint8_t msg = 0;
    (void)arg;
    while (1) {
        MSG_QUEUE_RECV(s_key_device_thd.msg_id, &msg, OS_WAIT_FOREVER);
        if (s_is_upgrading == true) {
            continue;
        }
        if (msg == TOUCH_KEY_THREAD_MSG_INTERRUPT) {
            hyn_touch_key_scan();
        }
    }
}

#if IS_SUPPORT_TOUCH_IC_OTA
static bool hyn_should_start_upgrade(void)
{
    hyn_read_fw_version();
#if (BSP_TOUCH_IC_FW_OTA_AUTO == 1)
    {
        return true;
    }
#else
    return touch_firmware_is_lower_than_image(s_touch_fw_version);
#endif
}

static void touch_ota_thread(void *arg)
{
    uint8_t msg = 0;
    (void)arg;
    while (1) {
        MSG_QUEUE_RECV(s_touch_ota_thd.msg_id, &msg, OS_WAIT_FOREVER);
        if (msg != TOUCH_OTA_THREAD_MSG_UPDATE_START || s_is_upgrading == true) {
            continue;
        }

        if (!hyn_should_start_upgrade()) {
            LOG_PBUS("HYN skip OTA: ic=0x%02x target=0x%02x",
                (unsigned)s_touch_fw_version, (unsigned)touch_firmware_target_version());
#if (BSP_TOUCH_IC_FW_OTA_AUTO == 1)
            msleep(HYN_TOUCH_OTA_STRESS_INTERVAL_MS);
            msg = TOUCH_OTA_THREAD_MSG_UPDATE_START;
            MSG_QUEUE_SEND(s_touch_ota_thd.msg_id, &msg, 0);
#endif
            continue;
        }

        s_is_upgrading = true;
#if HYN_TOUCH_USE_POLL
        if (s_poll_timer_id) {
            STOP_TIMER(s_poll_timer_id);
        }
#endif
#if HYN_ENABLE_DIFF_SIGNAL_DUMP
        if (s_diff_signal_timer_id) {
            STOP_TIMER(s_diff_signal_timer_id);
        }
#endif
        hyn_irq_control(false);
        bsp_touch_key_ota_event_cb(KEY_OTA_EVENT_UPDATE_START, 0);
#if (BSP_TOUCH_IC_FW_OTA_AUTO == 1)
        uint8_t ret = touch_fw_update_handler_force(true);
#else
        uint8_t ret = touch_fw_update_handler_force(false);
#endif
        if (ret != 0) {
            msleep(1000);
            bsp_touch_key_ota_event_cb(KEY_OTA_EVENT_UPDATE_FAIL, 0);
        } else {
            bsp_touch_key_ota_event_cb(KEY_OTA_EVENT_UPDATE_SUCCESS, 100);
        }
        msleep(60);
        hyn_read_fw_version();
        s_is_upgrading = false;
        hyn_irq_control(true);
#if HYN_TOUCH_USE_POLL
        if (s_poll_timer_id) {
            s_last_poll_key_state = 0;
            START_TIMER(s_poll_timer_id, HYN_POLL_PERIOD_MS);
        }
#endif
#if HYN_ENABLE_DIFF_SIGNAL_DUMP
        if (s_diff_signal_timer_id) {
            START_TIMER(s_diff_signal_timer_id, HYN_DIFF_SIGNAL_PERIOD_MS);
        }
#endif
#if (BSP_TOUCH_IC_FW_OTA_AUTO == 1)
        /* 自动化压测模式：每轮按真实流程检查后升级，固定间隔循环 */
        msleep(HYN_TOUCH_OTA_STRESS_INTERVAL_MS);
        msg = TOUCH_OTA_THREAD_MSG_UPDATE_START;
        MSG_QUEUE_SEND(s_touch_ota_thd.msg_id, &msg, 0);
#endif
    }
}
#endif

static int hyn_is_ready(void)
{
    uint8_t read_buf[2] = {0};
    for (uint8_t i = 0; i < 5; i++) {
        if (hyn_i2c_read(0x22, read_buf, 2) == BSP_OK) {
            return BSP_OK;
        }
        msleep(5);
    }
    return BSP_NOT_READY;
}

static void power_key_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    (void)pin;
    bsp_irq_msg_send(INT_NUM_KEY_1);
}

static void power_key_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable,
        0,
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING,
        power_key_irq_callback,
    };
    hal_gpio_setup_irq(POWER_KEY_PIN, &gpio_irq_cfg);
}

static void power_key_irq_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)POWER_KEY_PIN,
        HAL_IOMUX_FUNC_AS_GPIO,
        HAL_IOMUX_PIN_VOLTAGE_VIO,
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(POWER_KEY_PIN, HAL_GPIO_DIR_IN, 1);
}

 /** s_key_mask[i] 为单 bit 掩码：返回该 bit 在 0..15 中的位置号（与 0x22 状态位、0xC8 通道序号一致） */
 static uint8_t hyn_key_mask_bit_pos(uint16_t mask)
 {
     if (mask == 0) {
         return 0;
     }
     uint8_t n = 0;
     uint16_t m = mask;
     while ((m & 1U) == 0U) {
         m >>= 1;
         n++;
     }
     return n;
 }

int bsp_key_init(void)
{
    power_key_irq_init();
    power_key_irq_control(true);

#if IS_SUPPORT_DEVICE_TOUCH_KEY
    /* 唯一标识：确认链接的是 HYN 驱动而非 SC09B */
    LOG_PBUS("[HYN] touch key bsp_key_init start");
    hyn_rst_pin_init();
    hyn_irq_init();
    bsp_i2c_init(TOUCH_KEY_I2C_ID);

#if IS_SUPPORT_TOUCH_IC_OTA && BSP_TOUCH_IC_FW_OTA_AT_BOOT
// if (hyn_read_fw_version() != TOUCH_IC_FW_VERSION)
{
    LOG_PBUS("touch IC FW OTA at boot (BSP_TOUCH_IC_FW_OTA_AT_BOOT)");
    touch_fw_update_handler();
    msleep(60);
    hyn_read_fw_version();
}
#else
    /* 先复位芯片，再执行 CSK14T 初始化；不在未 init 前做 hyn_is_ready() 检测（芯片尚未响应会超时） */
    bsp_touch_key_reset();
    msleep(60);
#if IS_SUPPORT_TOUCH_IC_OTA
    hyn_read_fw_version();
#endif

#if 0
    if (hyn_init_csk14t() != BSP_OK) {
        LOGE("HYN touch key init failed");
        return BSP_ERROR;
    }
#endif

#endif
    s_touch_is_ready = true;
    s_touch_abnormal_count = 0;

    CREATE_MSG_TYPE_QUEUE(&s_key_device_thd.msg_id, 1, 1);
    RUN_NEW_THREAD(&s_key_device_thd, "key", key_device_thread,
                RT_PRIO_KEY_DEVICE_THREAD, (KEY_DEVICE_THREAD_STACKSIZE));
#if IS_SUPPORT_TOUCH_IC_OTA
    CREATE_MSG_TYPE_QUEUE(&s_touch_ota_thd.msg_id, 2, 1);
    RUN_NEW_THREAD(&s_touch_ota_thd, "hyn_ota", touch_ota_thread,
                RT_PRIO_KEY_DEVICE_THREAD, (KEY_DEVICE_THREAD_STACKSIZE));
#endif
    hyn_irq_control(true);
#if HYN_TOUCH_USE_POLL
    /* 启动轮询定时器：INT 脚未触发时仍能通过轮询 0x22 得到按键 */
    CREATE_TIMER(&s_poll_timer_id, "hyn_poll", NULL, OS_TIMER_PERIODIC, hyn_poll_timer_cb);
    if (s_poll_timer_id) {
        START_TIMER(s_poll_timer_id, HYN_POLL_PERIOD_MS);
    }
#endif
#if HYN_ENABLE_DIFF_SIGNAL_DUMP
    CREATE_TIMER(&s_diff_signal_timer_id, "hyn_diff", NULL, OS_TIMER_PERIODIC, hyn_diff_signal_timer_cb);
    if (s_diff_signal_timer_id) {
        START_TIMER(s_diff_signal_timer_id, HYN_DIFF_SIGNAL_PERIOD_MS);
    }
    /* 周期定时器首帧往往要等一个周期；初始化完成后立即打一行，再每 HYN_DIFF_SIGNAL_PERIOD_MS 循环 */
    hyn_diff_signal_dump_once();
#endif
    LOG_PBUS("HYN key init success");
#endif
    return BSP_OK;
}

int bsp_key_deinit(void)
{
#if IS_SUPPORT_DEVICE_TOUCH_KEY
    hyn_irq_control(false);

#if HYN_TOUCH_USE_POLL
    if (s_poll_timer_id) {
        STOP_TIMER(s_poll_timer_id);
        s_poll_timer_id = NULL;
    }
#endif

#if HYN_ENABLE_DIFF_SIGNAL_DUMP
    if (s_diff_signal_timer_id) {
        STOP_TIMER(s_diff_signal_timer_id);
        s_diff_signal_timer_id = NULL;
    }
#endif

#endif
    bsp_i2c_deinit(TOUCH_KEY_I2C_ID);
    s_touch_is_ready = false;
    s_touch_abnormal_count = 0;
    return BSP_OK;
}

int bsp_read_key_value(uint8_t key_index)
{
    if (key_index > HYN_TOUCH_KEY_NUM) {
        return 0;
    }
    if (key_index == 0) {
        return hal_gpio_pin_get_val(POWER_KEY_PIN) == 0 ? 1 : 0;
    }
    return s_touch_key[key_index - 1];
}

int bsp_read_all_key_value(void)
{
    int key_value = 0;
    for (uint8_t i = 1; i <= HYN_TOUCH_KEY_NUM; i++) {
        key_value |= s_touch_key[HYN_TOUCH_KEY_NUM - i];
        key_value <<= 1;
    }
    key_value |= hal_gpio_pin_get_val(POWER_KEY_PIN) == 0 ? 1 : 0;
    return key_value;
}

int bsp_key_sensitivity_set(uint8_t key_index, uint8_t sensitivity)
{
    (void)key_index;
    (void)sensitivity;
    /// 海栎创全局灵敏度在 init 中已配置(0x1E=0x64)，单通道可扩展
    return BSP_OK;
}

int bsp_touch_is_ready(void *args)
{
    uint8_t *touch_is_ready_status = (uint8_t *)args;
    *touch_is_ready_status = s_touch_is_ready;
    return BSP_OK;
}

void bsp_touch_key_reset(void)
{
#if IS_SUPPORT_TOUCH_KEY_RESET_PIN
    hal_gpio_pin_clr(HYN_TOUCH_RST_PIN);
    msleep(10);
    hal_gpio_pin_set(HYN_TOUCH_RST_PIN);
#else
    bsp_touch_rst_pin_output_set(0);
    msleep(10);
    bsp_touch_rst_pin_output_set(1);
#endif
}

int bsp_add_touch_key_ota_event_cb(void *args)
{
    s_key_ota_event_cb = (key_ota_event_cb_func)args;
    if (s_key_ota_event_cb == NULL) {
        return BSP_ERROR;
    }
    return BSP_OK;
}

void bsp_touch_key_ota_event_cb(key_ota_event_e event, uint8_t progress)
{
    if (s_key_ota_event_cb == NULL) {
        return;
    }
    s_key_ota_event_cb(event, progress);
}

int bsp_start_touch_key_update_fw(void)
{
#if IS_SUPPORT_TOUCH_IC_OTA
#if (BSP_TOUCH_IC_FW_OTA_AUTO == 0)
    if (touch_firmware_is_lower_than_image(s_touch_fw_version) == false) {
        return 0;
    }
#endif
    if (s_is_upgrading == true || s_touch_ota_thd.msg_id == NULL) {
        return 0;
    }
    uint8_t msg = TOUCH_OTA_THREAD_MSG_UPDATE_START;
    MSG_QUEUE_SEND(s_touch_ota_thd.msg_id, &msg, 0);
#endif
    return 0;
}

int bsp_get_touch_key_fw_version(void)
{
    return s_touch_fw_version;
} 

bool bsp_touch_key_is_upgrading(void)
{
    return s_is_upgrading;
}

int bsp_touch_signal_read(void *args)
{
   if (args == NULL) {
       return BSP_ERROR;
   }
    if (s_is_upgrading) {
        return BSP_NOT_READY;
    }
    uint8_t sig_buff[HYN_TOUCH_KEY_NUM * 2];

    if (hyn_i2c_read(0xC8, sig_buff, (uint16_t)(HYN_TOUCH_KEY_NUM * 2)) != BSP_OK) {
        return BSP_ERROR;
    }
     /// 按 s_key_mask 表顺序（与 s_touch_key 下标一致），从 0xC8 缓冲中取对应「置 1 位号」通道的 2 字节数据写入 args
    uint8_t *out = (uint8_t *)args;
    for (uint8_t i = 0; i < HYN_TOUCH_KEY_NUM; i++) {
        unsigned ch = (unsigned)hyn_key_mask_bit_pos(s_key_mask[i]);
        out[i * 2] = sig_buff[ch * 2];
        out[i * 2 + 1] = sig_buff[ch * 2 + 1];
    }
    return BSP_OK;
}