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
 
 #define HYN_KEY_RETRY_COUNT             (3)
 #define HYN_KEY_IDLE_COUNT             (3)
 #define HYN_TOUCH_ABNORMAL_MAX_COUNT   (10)
 #if HYN_TOUCH_USE_POLL
 #define HYN_POLL_PERIOD_MS             (20)  /// 轮询周期，IRQ 不触发时靠轮询保证触摸有响应
 #endif
 #define HYN_TOUCH_KEY_DEBUG            (0) /// 触摸按键调试开关
 
 static uint16_t s_touch_abnormal_count = 0;
 #if HYN_TOUCH_USE_POLL
 static uint16_t s_last_poll_key_state = 0;
 static os_timer_id s_poll_timer_id = NULL;
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
 
     /* 先复位芯片，再执行 CSK14T 初始化；不在未 init 前做 hyn_is_ready() 检测（芯片尚未响应会超时） */
     hyn_reset_ic();
     msleep(60);
 
     if (hyn_init_CSK14T() != BSP_OK) {
         LOGE("HYN touch key init failed");
         return BSP_ERROR;
     }
 
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
 