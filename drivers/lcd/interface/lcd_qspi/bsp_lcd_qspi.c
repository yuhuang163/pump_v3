/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_lcdc_qspi.h" // IWYU pragma: keep
#include "hal_lcdc.h" // IWYU pragma: keep
#include "hal_timer.h" // IWYU pragma: keep
#include "hal_psc.h" // IWYU pragma: keep
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_config.h" // IWYU pragma: keep
#include "bsp_lcd_qspi.h" // IWYU pragma: keep
#include "bsp_lcd_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_LCD_TE_IRQ                    (1)
#define LCDC_RGB565_FORMAT                       (11)
#define IS_SUPPORT_LCD_PSRAM_DRIVER              (1)
#define LCD_PIXEL_SIZE                           (LCD_H_RES * LCD_V_RES)
#define LCD_IMG_SIZE                             (LCD_PIXEL_SIZE * sizeof(uint16_t))

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static bool lcd_qspi_ready = false;
#if IS_SUPPORT_LCD_TE_IRQ
static os_message_id lcd_te_irq_msg_id = NULL;
static bool te_test_pin1_status = false;
static bool te_test_pin2_status = false;
static bool te_test_pin3_status = false;
#endif
static os_message_id lcd_framedone_irq_msg_id = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_LCD_TE_IRQ
void lcd_te_test_pin_toggle(uint8_t pin)
{
    if (pin == 1) {
        te_test_pin1_status = !te_test_pin1_status;
        GPIO_PIN_CTRL(HAL_GPIO_PIN_P1_4, te_test_pin1_status ? 1 : 0);
    } else if (pin == 2) {
        te_test_pin2_status = !te_test_pin2_status;
        GPIO_PIN_CTRL(HAL_GPIO_PIN_P1_5, te_test_pin2_status ? 1 : 0);
    } else if (pin == 3) {
        te_test_pin3_status = !te_test_pin3_status;
        GPIO_PIN_CTRL(HAL_GPIO_PIN_P1_6, te_test_pin3_status ? 1 : 0);
    }
}

static void lcd_te_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    uint8_t msg = 0;
    lcd_te_test_pin_toggle(1);
    MSG_QUEUE_SEND(lcd_te_irq_msg_id, &msg, 0);
}

static void lcd_te_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable, 
        0, 
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE, 
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING,
        lcd_te_irq_callback
    };
    hal_gpio_setup_irq(USR_LCD_TE_PIN, &gpio_irq_cfg);
}

static void lcd_te_irq_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)USR_LCD_TE_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(USR_LCD_TE_PIN,  HAL_GPIO_DIR_IN,  1);
    gpio_map.pin = (enum HAL_IOMUX_PIN_T)HAL_GPIO_PIN_P1_4;
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_4,  HAL_GPIO_DIR_OUT,  1);
    gpio_map.pin = (enum HAL_IOMUX_PIN_T)HAL_GPIO_PIN_P1_5;
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_5,  HAL_GPIO_DIR_OUT,  1);
    gpio_map.pin = (enum HAL_IOMUX_PIN_T)HAL_GPIO_PIN_P1_6;
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_6,  HAL_GPIO_DIR_OUT,  1);
}
#endif

static void lcdc_isr(void)
{
    uint8_t msg = 0;
    uint32_t irq_stat = hal_lcdc_get_irqstat();
    if (hal_lcdc_irq_is_framedone(irq_stat)) {
        hal_lcdc_framedone_irqstat_clr();
        hal_lcdc_qspi_cs_sel(false);
        LOG_PBUS("framedone irq");
        MSG_QUEUE_SEND(lcd_framedone_irq_msg_id, &msg, 0);
    }
}

static int lcd_qspi_init(void)
{
    hal_psc_display_enable(HAL_PSC_DISPLAY_MOD_QSPI);
    hal_lcdc_qspi_initialize(LCDC_QSPI_RGB565, LCDC_QSPI_4LINE);
    return BSP_OK;
}

static int lcd_display_controller_init(int width, int height)
{
    int irq = hal_lcdc_irqn();
    hal_lcdc_disp_pn_reset(); /// 复位显示控制器
    /* Layer SPU */
    hal_lcdc_lsize(LCDC_LAYER_SPU, width - 1, height - 1); /// 设置层SPU的大小
    hal_lcdc_lcolor(LCDC_LAYER_SPU, 0); /// 设置层SPU的颜色(默认黑色)
    hal_lcdcl_enable(LCDC_LAYER_SPU, true); /// 使能层SPU
    /* Layer Foreground (0) */
    hal_lcdc_lsize(LCDC_LAYER_LFORE, width, height); /// 设置层Foreground的大小
    hal_lcdc_lpitch(LCDC_LAYER_LFORE, width * sizeof(uint16_t)); /// 设置层Foreground的步长
    hal_lcdc_lzoom_set(LCDC_LAYER_LFORE, width, height); /// 设置层Foreground的缩放比例
    hal_lcdc_sponscreen_set(LCDC_LAYER_LFORE, 1, 1); /// 设置层Foreground的显示模式
    hal_lcdc_lpfc_fmt(LCDC_LAYER_LFORE, LCDC_RGB565_FORMAT); /// 设置层Foreground的格式
    hal_lcdc_lpfc_ltransp(LCDC_LAYER_LFORE, 0, 0); /// 设置层Foreground的透明度
    hal_lcdc_select_top(LCDC_LAYER_LFORE); /// 选择层Foreground为顶层

    hal_lcdc_smpn_enable(); /// 使能层Foreground
    hal_lcdc_smpn_mode_set(5); /// 设置层Foreground的模式

    hal_lcdc_panelpath_dmatrans_trigger_enable(LCDC_LAYER_LFORE, true); /// 使能层Foreground的DMA传输触发
    hal_lcdc_set_sync_line_pixs(5); /// 设置同步线的像素数
    hal_lcdc_set_sync_lines(0); /// 设置同步线的行数
    hal_lcdc_smpn_vsync_delaymode_set(1); /// 设置层Foreground的VSYNC延迟模式
    hal_lcdc_disp_extra_delay(0); /// 设置显示额外的延迟
    hal_lcdc_sl_dmaburst_length_set(0x0f); /// 设置层Foreground的DMA突发长度
    hal_lcdc_ol_dmaburst_length_set(8); /// 设置层Foreground的DMA突发长度

    /* Configure dither */
    hal_lcdc_dither_s4x8(true); /// 使能4x8抖动
    hal_lcdc_dither(false, 1, 0, 0, 0); /// 设置抖动参数

    hal_lcdc_disp_avsync_trigger_disable(true); /// 禁用AVSYNC触发
    hal_lcdc_disp_svsync_trigger_disable(true); /// 禁用VSYNC触发

    hal_lcdcl_enable(LCDC_LAYER_LFORE, true); /// 使能层Foreground
    hal_lcdcl_enable(LCDC_LAYER_LBACK, false); /// 禁用层Background

    /* lcdc framedone irq */
    hal_lcdc_irq_disable();
    NVIC_SetVector((IRQn_Type)irq, (uint32_t) lcdc_isr);
    NVIC_SetPriority((IRQn_Type)irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ((IRQn_Type)irq);
    NVIC_EnableIRQ((IRQn_Type)irq);

    hal_lcdc_gen_frame_enable(true);
    hal_lcdc_framedone_irq_enable(true);
    return BSP_OK;
}

static int lcd_qspi_deinit(void)
{
    int irq = hal_lcdc_irqn();
    hal_lcdc_framedone_irq_enable(false);
    hal_lcdc_gen_frame_enable(false);
    hal_lcdc_irq_disable();
    NVIC_ClearPendingIRQ((IRQn_Type)irq);
    NVIC_DisableIRQ((IRQn_Type)irq);
    hal_cmu_lcdc_clock_disable();

    return BSP_OK;
}

static void lcd_qspi_sleep(void)
{
    hal_lcdc_qspi_sleep();
    hal_lcdc_sleep();
}

static void lcd_qspi_wakeup(void)
{
    hal_lcdc_wakeup();
    hal_lcdc_qspi_wakeup();
}

static void lcd_reset_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)USR_LCD_RST_PIN, 
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(USR_LCD_RST_PIN, HAL_GPIO_DIR_OUT, 0);
}

static void lcd_device_reset(void)
{
    GPIO_PIN_CTRL(USR_LCD_RST_PIN, 1);
    msleep(5);
    GPIO_PIN_CTRL(USR_LCD_RST_PIN, 0);
    msleep(20);
    GPIO_PIN_CTRL(USR_LCD_RST_PIN, 1);
    msleep(10);
}

static int bsp_lcd_color_set(uint16_t color)
{
    return BSP_OK;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_lcd_qspi_init(void)
{
    int ret = 0;
#if IS_SUPPORT_LCD_TE_IRQ
    CREATE_MSG_TYPE_QUEUE(&lcd_te_irq_msg_id, 1, 1);
    if (lcd_te_irq_msg_id == NULL) {
        LOGE("lcd qspi msg id create failed");
        return BSP_ERROR;
    }
    lcd_te_irq_init();
    lcd_te_irq_control(true);
#endif
    CREATE_MSG_TYPE_QUEUE(&lcd_framedone_irq_msg_id, 1, 1);
    if (lcd_framedone_irq_msg_id == NULL) {
        LOGE("lcd qspi framedone msg id create failed");
        return BSP_ERROR;
    }
    ret = lcd_qspi_init();
    if (ret != BSP_OK) {
        LOGE("lcd qspi init failed, ret = %d", ret);
        return BSP_ERROR;
    }

    ret = lcd_display_controller_init(LCD_H_RES, LCD_V_RES);
    if (ret != BSP_OK) {
        LOGE("lcd qspi display controller init failed, ret = %d", ret);
        return BSP_ERROR;
    }
    lcd_qspi_ready = true;
    LOG_PBUS("lcd qspi init success");
    return BSP_OK;
}

int bsp_lcd_qspi_resume(void)
{
    return BSP_OK;
}

int bsp_lcd_qspi_suspend(void)
{
    return BSP_OK; 
}

int bsp_lcd_qspi_write_cmd_param(uint8_t cmd, const uint8_t *p_data, uint32_t data_size)
{
    if (!lcd_qspi_ready) {
        LOGE("lcd qspi is not ready");
        return BSP_ERROR;
    }
    // hal_lcdc_qspi_set_command_freq(96);
    hal_lcdc_qspi_send_command_list(cmd, data_size, (uint8_t *)p_data);
    return BSP_OK;
}

int bsp_lcd_qspi_write_img_param(uint8_t cmd, const uint8_t *p_data, uint32_t data_size)
{
    uint8_t msg = 0;
#if IS_SUPPORT_LCD_DEBUG
    uint32_t start_time = get_timestamp();
#endif
    if (!lcd_qspi_ready) {
        LOGE("lcd qspi is not ready");
        return BSP_ERROR;
    }
    // lcd_qspi_wakeup();
    memset((uint8_t *)p_data, 0x55, 100);
    BSP_LOG_HEX("img12", p_data, 100);
    hal_lcdc_lstartaddr(0, (uint32_t)p_data);
    // MSG_QUEUE_RESET(lcd_te_irq_msg_id);
    // lcd_te_test_pin_toggle(2);
    // MSG_QUEUE_RECV(lcd_te_irq_msg_id, &msg, 40);
    // lcd_te_test_pin_toggle(3);
    hal_lcdc_qspi_gen_start();
    MSG_QUEUE_RECV(lcd_framedone_irq_msg_id, &msg, 100);
    // lcd_qspi_sleep();
    msleep(1000);
#if IS_SUPPORT_LCD_DEBUG
    uint32_t end_time = get_timestamp();
    LOG_PBUS("write param time = %dms, data_size = %d", get_timestamp_ms(start_time), data_size);
#endif
    return BSP_OK;
}

int bsp_lcd_init(void) 
{
    int ret = 0;
    lcd_device_reset();
    ret = bsp_lcd_qspi_init();
    if (ret != BSP_OK) {
        LOGE("lcd interface init failed");
        return ret;
    }
    msleep(10);
    ret = lcd_chip_init();
    if (ret != BSP_OK) {
        LOGE("lcd chip init failed");
        return ret;
    }
    bsp_lcd_color_set(0x0000); /// 清屏
    return BSP_OK;
}

int bsp_lcd_uart_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    return BSP_OK;
}

int bsp_lcd_refresh(const void *buf, uint32_t n_bytes)
{
    if (!buf || n_bytes < (sizeof(uint32_t) * 6)) {
        LOG_PBUS("invalid args: buf:%p, n_bytes:%d", buf, n_bytes);
        return -1;
    }
    uint32_t *tmp = (uint32_t *)buf;
    uint16_t x1 = (uint16_t)tmp[0];
    uint16_t y1 = (uint16_t)tmp[1];
    uint16_t x2 = (uint16_t)tmp[2];
    uint16_t y2 = (uint16_t)tmp[3];
    uint16_t *color = (uint16_t *)tmp[4];

    if (color == NULL) {
        LOGE("color is NULL");
        return BSP_ERR_POINTER_NULL;
    }

    if (!lcd_qspi_ready) {
        LOGE("lcd no ready");
        return BSP_NOT_READY;
    } 
    return lcd_draw_bitmap(x1, y1, x2, y2, color);
}

