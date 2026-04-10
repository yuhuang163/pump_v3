/*
 * 海栎创自容触摸IC驱动 - 按键项目
 * 参考: d:\Root\Git\reference_code\hyn_iic_drive_for_MCU
 * 适配 BES 平台 bsp_i2c 与 bsp_key_internal 接口
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
 #if IS_SUPPORT_TOUCH_IC_OTA
 #include "hyn_firmware.h"
 static uint8_t hyn_current_fw_version = 0;
 #endif

 #define HYN_KEY_RETRY_COUNT             (3)
 #define HYN_KEY_IDLE_COUNT             (3)
 #define HYN_TOUCH_ABNORMAL_MAX_COUNT   (10)
 #if HYN_TOUCH_USE_POLL
 #define HYN_POLL_PERIOD_MS             (20)  /// 轮询周期，IRQ 不触发时靠轮询保证触摸有响应
 #endif
 #define HYN_TOUCH_KEY_DEBUG            (0) /// 触摸按键调试开关

 /*
 * LOCAL VARIABLES
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
 static thread_context_t s_key_device_thd = {0};
 static uint8_t s_touch_key[HYN_TOUCH_KEY_NUM] = {0};
 static uint16_t s_key_state = 0;
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
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
 static int hyn_i2c_write(uint8_t reg_addr, const uint8_t *buf, uint16_t len)
 {
     i2c_master_data_t data = {
         .dev_address = HYN_TOUCH_I2C_ADDR,
         .reg_addr = reg_addr,
         .data = (uint8_t *)buf,
         .len = len,
     };
     return bsp_i2c_write_byte(TOUCH_KEY_I2C_ID, &data);
 }
 
 static int hyn_i2c_read(uint8_t reg_addr, uint8_t *buf, uint16_t len)
 {
     i2c_master_data_t data = {
         .dev_address = HYN_TOUCH_I2C_ADDR,
         .reg_addr = reg_addr,
         .data = buf,
         .len = len,
     };
     return bsp_i2c_read_byte(TOUCH_KEY_I2C_ID, &data);
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
 
 static void hyn_reset_ic(void)
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
 
 static int hyn_init_CSK14T(void)
 {
     uint8_t data_buff[4] = {0};
     int retry;
 
     for (retry = 0; retry < 10; retry++) {
         LOG_PBUS("HYN CSK14T init retry %d", retry);
         hyn_reset_ic();
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
 
#if IS_SUPPORT_TOUCH_IC_OTA
static uint8_t hyn_read_fw_version(void)
{
    uint8_t read_buff[1] = {0};

    if (hyn_i2c_read(0xFF, read_buff, 1) != BSP_OK) {
        read_buff[0] = 0;
        LOGE("HYN reg 0xFF ERROR!!!");
    }
    
    hyn_current_fw_version = read_buff[0];
    LOGE("HYN reg 0xFF(read_fw): 0x%02x", (unsigned)read_buff[0]);
    
    return read_buff[0];
}

uint8_t get_touch_ic_fw_current_version(void)
{
    return hyn_current_fw_version;
}

uint8_t get_touch_ic_fw_target_version(void)
{
    return TOUCH_IC_FW_VERSION;
}
#endif

 static void hyn_irq_callback(enum HAL_GPIO_PIN_T pin)
 {
    #if HYN_TOUCH_KEY_DEBUG
     LOGE("HYN irq pin=%d", (int)pin);
    #endif
     if (pin == TOUCH_INT_PIN && s_key_device_thd.msg_id) {
         uint8_t msg = 0;
         MSG_QUEUE_SEND(s_key_device_thd.msg_id, &msg, 0);
         bsp_irq_msg_send(INT_NUM_TOUCH_KEY);
     }
 }
 
 /* 若触摸无中断打印，可改为 1 尝试上升沿（海栎创部分型号 INT 为高有效） */
 #ifndef HYN_IRQ_USE_RISING_EDGE
 #define HYN_IRQ_USE_RISING_EDGE         (0)
 #endif
 
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
     if (!s_touch_is_ready) {
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
 
 /** 一次快速读键值并更新 s_touch_key[]，供 scan_key 定时器首次回调时能读到有效状态，避免因 key_idle_count 提前停掉定时器 */
 static void hyn_touch_key_read_once(void)
 {
     uint8_t read_buff[2] = {0};
     uint8_t clear_irq;
 
     if (hyn_i2c_read(0x1F, read_buff, 1) != BSP_OK) {
         return;
     }
     if ((read_buff[0] & 0x01) != 0x01) {
         return;
     }
     clear_irq = read_buff[0] & (~0x02);
     if (hyn_i2c_read(0x22, read_buff, 2) != BSP_OK) {
         return;
     }
     (void)hyn_i2c_write(0x1F, &clear_irq, 1);
 
     s_key_state = (uint16_t)(read_buff[1] << 8) | read_buff[0];
     LOGE("HYN reg 0x22(read_once): 0x%04x", (unsigned)s_key_state);
     for (uint8_t i = 0; i < HYN_TOUCH_KEY_NUM; i++) {
         s_touch_key[i] = (s_key_state & s_key_mask[i]) ? 1 : 0;
     }
 }
 
 static void hyn_touch_key_scan(void)
 {
     uint8_t read_buff[2] = {0};
     uint8_t clear_irq;
     uint16_t key_idle_count = 0;
     int error_count = 0;
 
     while (1) {
         if (hyn_i2c_read(0x1F, read_buff, 1) != BSP_OK) {
             error_count++;
             if (error_count > 5) {
                 break;
             }
             msleep(10);
             continue;
         }
         if ((read_buff[0] & 0x01) != 0x01) {
             error_count++;
             if (error_count > 5) {
                 break;
             }
             msleep(10);
             continue;
         }
 
         clear_irq = read_buff[0] & (~0x02);
         if (hyn_i2c_read(0x22, read_buff, 2) != BSP_OK) {
             error_count++;
             if (error_count > 5) {
                 break;
             }
             msleep(10);
             continue;
         }
         if (hyn_i2c_write(0x1F, &clear_irq, 1) != BSP_OK) {
             msleep(10);
             continue;
         }
 
         s_key_state = (uint16_t)(read_buff[1] << 8) | read_buff[0];
         #if HYN_TOUCH_KEY_DEBUG
         LOGE("HYN reg 0x22(scan): 0x%04x", (unsigned)s_key_state);
         #endif
         for (uint8_t i = 0; i < HYN_TOUCH_KEY_NUM; i++) {
             s_touch_key[i] = (s_key_state & s_key_mask[i]) ? 1 : 0;
         }
 
         if (s_key_state == 0) {
             key_idle_count++;
             if (key_idle_count > HYN_KEY_IDLE_COUNT) {
                 break;
             }
         } else {
             key_idle_count = 0;
         }
         error_count = 0;
         msleep(10);
     }
 }
 
 static void key_device_thread(void *arg)
 {
     uint8_t msg = 0;
     (void)arg;
     while (1) {
         MSG_QUEUE_RECV(s_key_device_thd.msg_id, &msg, OS_WAIT_FOREVER);
         /* 先快速更新一次 s_touch_key，让 scan_key 首轮 10ms 定时器能读到有效键值，避免定时器因 key_idle_count 提前停止 */
         hyn_touch_key_read_once();
         hyn_touch_key_scan();
     }
 }
 
 static int hyn_is_ready(void)
 {
     uint8_t read_buf[2] = {0};
     i2c_master_data_t data = {
         .dev_address = HYN_TOUCH_I2C_ADDR,
         .reg_addr = 0x22,
         .data = read_buf,
         .len = 2,
     };
     for (uint8_t i = 0; i < 5; i++) {
         if (bsp_i2c_read_byte(TOUCH_KEY_I2C_ID, &data) == BSP_OK) {
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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
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
    if (hyn_read_fw_version() != TOUCH_IC_FW_VERSION)
    {
        LOG_PBUS("touch IC FW OTA at boot (BSP_TOUCH_IC_FW_OTA_AT_BOOT)");
        touch_fw_update();
        msleep(60);
        hyn_read_fw_version();
    }
#else
     /* 先复位芯片，再执行 CSK14T 初始化；不在未 init 前做 hyn_is_ready() 检测（芯片尚未响应会超时） */
     hyn_reset_ic();
     msleep(60);
#if IS_SUPPORT_TOUCH_IC_OTA
     hyn_read_fw_version();
#endif

    //  if (hyn_init_CSK14T() != BSP_OK) {
    //      LOGE("HYN touch key init failed");
    //      return BSP_ERROR;
    //  }
#endif
     s_touch_is_ready = true;
     s_touch_abnormal_count = 0;
 
     CREATE_MSG_TYPE_QUEUE(&s_key_device_thd.msg_id, 1, 1);
     RUN_NEW_THREAD(&s_key_device_thd, "key", key_device_thread,
                    RT_PRIO_KEY_DEVICE_THREAD, (KEY_DEVICE_THREAD_STACKSIZE));
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

 int bsp_touch_signal_read(void *args)
 {
    if (args == NULL) {
        return BSP_ERROR;
    }
     uint8_t sig_buff[HYN_TOUCH_KEY_NUM * 2];
 
     if (hyn_i2c_read(0xC8, sig_buff, (uint16_t)(HYN_TOUCH_KEY_NUM * 2)) != BSP_OK) {
         return BSP_ERROR;
     }
     uint8_t *signal_data = (uint8_t *)args;
     memcpy(signal_data, sig_buff, (uint16_t)(HYN_TOUCH_KEY_NUM * 2));
     return BSP_OK;
 }

#if IS_SUPPORT_TOUCH_IC_OTA
int touch_ic_fw_update_execute(void)
{
    return (touch_fw_update() == HYN_TRUE) ? 0 : -1;
}

uint8_t TP_HRS_WriteBytes(uint8_t iic_addr, uint16_t reg_addr, uint8_t* buf, uint16_t len, uint8_t reg_num)
{
    /*
     * 16 位寄存器地址：bsp_i2c_write_byte 只带 1 字节 reg，原实现把 [reg 低字节]+payload
     * 塞进调用方 buf 首部。正向拷贝 buf[i]->buf[i+1] 会覆盖未搬移的数据，破坏整块内容；
     * 且 OTA 固件常为 const/Flash，原地写不可接受。改为临时缓冲组装后再写。
     */
    if (reg_num == REG_LEN_2B) {
        uint8_t *tmp = (uint8_t *)u_malloz((uint32_t)len + 1U);
        if (tmp == NULL) {
            return HYN_FAULT;
        }
        tmp[0] = (uint8_t)(reg_addr & 0xFF);
        for (uint16_t i = 0; i < len; i++) {
            tmp[i + 1] = buf[i];
        }
        i2c_master_data_t data = {
            .dev_address = iic_addr,
            .reg_addr = (uint8_t)(reg_addr >> 8),
            .data = tmp,
            .len = (uint16_t)(len + 1),
        };
        int ok = bsp_i2c_write_byte(TOUCH_KEY_I2C_ID, &data);
        u_free(tmp);
        return ((ok == BSP_OK) ? HYN_TRUE : HYN_FAULT);
    }

    i2c_master_data_t data = {
        .dev_address = iic_addr,
        .reg_addr = (uint8_t)reg_addr,
        .data = buf,
        .len = len,
    };
    return ((bsp_i2c_write_byte(TOUCH_KEY_I2C_ID, &data) == BSP_OK) ? HYN_TRUE : HYN_FAULT);
}

static int TP_HRS_ReadBytes(uint8_t iic_addr, uint16_t reg_addr, uint8_t* buf, uint16_t len, uint8_t reg_num)
{
    i2c_master_data_t data = {
        .dev_address = iic_addr,
        .reg_addr = reg_addr,
        .data = buf,
        .len = len,
    };
    return ((bsp_i2c_read_bytes(TOUCH_KEY_I2C_ID, &data, reg_addr) == BSP_OK) ? HYN_TRUE : HYN_FAULT);
}


//uint8_t TP_HRS_WriteBytes(uint8_t iic_addr,uint16_t reg_addr,uint8_t* buff,uint16_t buff_len,uint8_t reg_num);//写数据
//uint8_t TP_HRS_ReadBytes(uint8_t iic_addr,uint16_t reg_addr,uint8_t* buff,uint16_t buff_len,uint8_t reg_num);//读数据
//iic_addr:IIC地址(7位地址)
//reg_addr：寄存器地址
//buff：数据指针
//len：传输的字节总长度
//reg_num ：寄存器长度
//返回值： 1：错误/失败  0：正确/成功

static int hyn_ic_enter_bootmode(void)//使触摸IC进入boot模式
{
	uint8_t retryCnt = 0;
	
	for(retryCnt = 0; retryCnt < 10; retryCnt++){
		uint8_t cmd[3];
		uint8_t read_buff[2];

		hyn_reset_ic();
		
		msleep(5 + retryCnt);//5~20ms 这个延时必须准确
		
		cmd[0] = 0xAB;//0xAB:CST816T/CST816D/CST816S/HYN19X2/CSK0XX		//	 0xAA；CST716    
		if(TP_HRS_WriteBytes(HYN_TOUCH_BOOT_I2C_ADDR, 0xA001, &cmd[0], 1, REG_LEN_2B) == HYN_FAULT){
			continue;
		}

		msleep(1);
		if(TP_HRS_ReadBytes( HYN_TOUCH_BOOT_I2C_ADDR,0XA003,read_buff,1,REG_LEN_2B) == HYN_FAULT){
			continue;
		}

		if (read_buff[0] != 0xC1){//0xC1:CST816T、CST816D、CSK0XT    //   0x55: CST716 、CST816S、CSK0XS 
			continue;
		}
		else{
			return HYN_TRUE;
		}
	}
	if(retryCnt >= 10) return HYN_FAULT;

    return HYN_TRUE;
}

static uint8_t hyn_ic_out_bootmode(void)
{
	uint8_t cmd = 0xEE;
	if(TP_HRS_WriteBytes(HYN_TOUCH_BOOT_I2C_ADDR,0xA006,&cmd,1,REG_LEN_2B) == HYN_FAULT){
		return HYN_FAULT;
	}
	return HYN_TRUE;
}

static uint16_t  hyn_read_checksum(void)//读取IC里面的校验和
{
	union{
		uint16_t sum;
		uint8_t buf[2];
	}checksum;

	uint8_t cmd[3];
	uint8_t retrycnt = 50;
	uint8_t read_01=0;
	uint8_t retry = 1;
	uint8_t result = HYN_TRUE;

	for(retry = 1; retry < 6; retry++)
	{
		if (hyn_ic_enter_bootmode() == HYN_FAULT){
			LOG_PBUS("hyn_ic_enter_bootmode fail!\r\n");
			continue;
		}		
		cmd[0] = 0x00;
		result = TP_HRS_WriteBytes( HYN_TOUCH_BOOT_I2C_ADDR, 0xA003,&cmd[0], 1,REG_LEN_2B);//写数据--开始计算校验和
		if(result == HYN_FAULT)continue;


		msleep(100*retry);//要求100ms以上

		checksum.sum = 0;			
		retrycnt = 50;
		while(retrycnt--){
			result = TP_HRS_ReadBytes( HYN_TOUCH_BOOT_I2C_ADDR, 0xA000,&read_01, 1,REG_LEN_2B);
			if(result == HYN_FAULT)continue;

			if (read_01 == 0x01){ //checksum计算完成
				break;
			}
			msleep(10);
		}
		result = TP_HRS_ReadBytes( HYN_TOUCH_BOOT_I2C_ADDR, 0xA008, checksum.buf, 2,REG_LEN_2B);//读出校验和
		if(result == HYN_FAULT)continue;

		return checksum.sum;
	}	

	if(retry >= 6){
		return HYN_FAULT;
	}
	return checksum.sum;
}

static uint8_t hyn_ic_update(uint16_t startAddr,uint16_t len,uint8_t* src,uint8_t retry)//升级固件
{
	uint16_t sum_len;
	uint8_t cmd[4];
	uint8_t read_bf[2];
	uint8_t retrycnt = 50;
	
	if (hyn_ic_enter_bootmode()){
		return HYN_FAULT;
	}
	sum_len = 0;
    do{
		if (sum_len >= len){
			return HYN_TRUE;
		}
		cmd[0] = startAddr&0xFF;
		cmd[1] = startAddr>>8;

		if(TP_HRS_WriteBytes( HYN_TOUCH_BOOT_I2C_ADDR, 0xA014,&cmd[0],2,REG_LEN_2B) == HYN_FAULT) return HYN_FAULT;
		 
#if 0   // 0xA018 缓存数据起始地址 长度512
			#if 0  //512字节分成4组，每组128字节
			{
				uint8_t i=0;
				for(i=0; i<4; i++){
					if(TP_HRS_WriteBytes( HYN_TOUCH_BOOT_I2C_ADDR, 0xA018+(i*128),src+(i*128),PER_LEN/4,REG_LEN_2B)== HYN_FAULT) return HYN_FAULT;
				}
			}
			#else   
			{		//512字节分成16组，每组32字节
				uint8_t i=0;
				for(i=0; i<16; i++){
					if(TP_HRS_WriteBytes( HYN_TOUCH_BOOT_I2C_ADDR, 0xA018+(i*32),src+(i*32),PER_LEN/16,REG_LEN_2B) == HYN_FAULT) return HYN_FAULT;
				}
			}
			
//			{ //512字节分成128组，每组4字节
//				uint8_t i=0;
//				for(i=0; i<128; i++){
//					if(TP_HRS_WriteBytes( HYN_TOUCH_BOOT_I2C_ADDR, 0xA018+(i*4),src+(i*4),PER_LEN/128,REG_LEN_2B) == HYN_FAULT) return HYN_FAULT;
//				}
//			}
		   #endif 
#else
		if(TP_HRS_WriteBytes( HYN_TOUCH_BOOT_I2C_ADDR, 0xA018,src,PER_LEN,REG_LEN_2B) == HYN_FAULT) return HYN_FAULT;//写入512字节数据
#endif

        	msleep(1);
		cmd[0] = 0xEE;
		if(TP_HRS_WriteBytes( HYN_TOUCH_BOOT_I2C_ADDR, 0xA004,&cmd[0],1,REG_LEN_2B) == HYN_FAULT) return HYN_FAULT;////在执行这个命令之前，必须写够512个字节！！！

        	msleep(100*retry);//这个时间不能省  unit=1ms    >=100
		
		retrycnt = 50;
		while(retrycnt--){
			msleep(5);			
			if(TP_HRS_ReadBytes( HYN_TOUCH_BOOT_I2C_ADDR,0xA005, read_bf, 1,REG_LEN_2B) == HYN_FAULT) continue;						 
			if (read_bf[0] == 0x55){// success
				msleep(1);
				break;
			}
		}
		
		if(retrycnt == 0)return HYN_FAULT;
		
		startAddr += PER_LEN;
		src += PER_LEN;
		sum_len += PER_LEN;
    }while(len);

	cmd[0] = 0x00;
	if(TP_HRS_WriteBytes( HYN_TOUCH_BOOT_I2C_ADDR, 0xA003,&cmd[0],1,REG_LEN_2B) == HYN_FAULT) return HYN_FAULT;
	return HYN_TRUE;
}

/**
 * @param bin  固件镜像（含 6 字节头：addr_lo,addr_hi,len_lo,len_hi,cs_lo,cs_hi + payload）
 * @param bin_size 镜像总长度
 */
static uint8_t touch_fw_update_with_bin(const uint8_t *bin, uint32_t bin_size)
{
	uint16_t IC_checksum = 0;

    LOG_PBUS("touch IC start OTA");
	if (hyn_ic_enter_bootmode() != HYN_TRUE) {
        LOG_PBUS("hynitron enter boot mode fail! \r\n");
		return HYN_FAULT;
	}

    if (bin_size > 10U) {
        uint16_t startAddr = bin[1];
        uint16_t length = bin[3];
        uint16_t checksum = bin[5];
        uint8_t retry = 1;

        startAddr <<= 8;
        startAddr |= bin[0];
        length <<= 8;
        length |= bin[2];
        checksum <<= 8;
        checksum |= bin[4];
        LOG_PBUS("hynitron enter boot mode success.\r\n");
        LOG_PBUS("hex_file_checksum: 0x%02x \r\n", checksum);
        IC_checksum = hyn_read_checksum();
        LOG_PBUS("IC_checksum: 0x%02x \r\n", IC_checksum);

        if ((IC_checksum != checksum) && ((startAddr | length) == 0x3C00)) {
            for (retry = 1; retry < 4; retry++) {
                if (hyn_ic_update(startAddr, length, (uint8_t *)(uintptr_t)(bin + 6), retry) == HYN_FAULT) {
                    continue;
                }
                break;
            }
            if (retry >= 4) {
                hyn_ic_out_bootmode();
                hyn_reset_ic();
                return HYN_FAULT;
            }

            IC_checksum = hyn_read_checksum();
            LOG_PBUS("after update IC_checksum: 0x%02x \r\n", IC_checksum);

            if (IC_checksum != checksum) {
                LOG_PBUS("hynitron update fail! \r\n");
            }
        }
    }

    hyn_ic_out_bootmode();
    hyn_reset_ic();
    LOG_PBUS("touch IC OTA over");

    return HYN_TRUE;
}

uint8_t touch_fw_update(void)
{
#if IS_SUPPORT_TOUCH_IC_OTA
    return touch_fw_update_with_bin(app_bin, (uint32_t)sizeof(app_bin));
#else
    return HYN_FAULT;
#endif
}
#endif
