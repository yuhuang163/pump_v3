/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_config.h" // IWYU pragma: keep
#include "cmsis_os_api.h"
#include "hal_spi.h" // IWYU pragma: keep
#include "hal_iomux.h" // IWYU pragma: keep
#include "hal_sysfreq.h" // IWYU pragma: keep
#include "bsp_lcd_spi.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_LCD_TE_IRQ                   (1)
#define IS_SUPPORT_LCD_SPI_DMA                  (1)
#if IS_SUPPORT_LCD_SPI_DMA
#define IS_SUPPORT_LCD_SPI_ASYN_SEND            (1)
#else
#define IS_SUPPORT_LCD_SPI_ASYN_SEND            (0)
#endif
#define IS_SUPPORT_LCD_SPI_SOFT_CS              (1)
#define IS_SUPPORT_LCD_DEBUG                    (0)

#define DMA_MAX_SIZE                            (3072)
#define LCD_WRITE_CMD(X)                        GPIO_PIN_CTRL(USR_LCD_DC_PIN, X ? 1 : 0) 
#define LCD_WRITE_DATA(X)                       GPIO_PIN_CTRL(USR_LCD_DC_PIN, X ? 1 : 0) 
#if IS_SUPPORT_LCD_SPI_SOFT_CS
#define LCD_WRITE_CS(X)                         GPIO_PIN_CTRL(USR_LCD_CS_PIN, X ? 1 : 0) 
#else
#define LCD_WRITE_CS(X)                         
#endif
#define LCD_SPI_CLK_MHZ                         (24)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_LCD_SPI_ASYN_SEND
static os_message_id lcd_spi_send_msg_id = NULL;
#endif
static bool lcd_spi_ready = false;

#if IS_SUPPORT_LCD_TE_IRQ
static os_message_id lcd_te_irq_msg_id = NULL;
static bool te_test_pin1_status = false;
static bool te_test_pin2_status = false;
static bool te_test_pin3_status = false;
#endif

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

static void lcd_spi_send_callback(int error)
{
#if IS_SUPPORT_LCD_SPI_ASYN_SEND
    uint8_t msg = 0;
    MSG_QUEUE_SEND(lcd_spi_send_msg_id, &msg, 0);
    lcd_te_test_pin_toggle(2);
#endif
    if (error != 0) {
        LOGE("lcd spi send callback error = %d", error);
    }
}

static int lcd_spi_gpio_init(void) 
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)USR_LCD_DC_PIN, 
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(USR_LCD_DC_PIN, HAL_GPIO_DIR_OUT, 1);

#if IS_SUPPORT_LCD_SPI_SOFT_CS
    gpio_map.pin = (enum HAL_IOMUX_PIN_T)USR_LCD_CS_PIN;
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(USR_LCD_CS_PIN, HAL_GPIO_DIR_OUT, 1);
#endif
    return BSP_OK;
}

static int lcd_spi_write_command(uint8_t cmd)
{
    int ret = 0;
    LCD_WRITE_CS(0);
    LCD_WRITE_CMD(0);
#if IS_SUPPORT_LCD_SPI_DMA
    uint8_t msg = 0;
    ret = hal_spilcd_dma_send(&cmd, 1, lcd_spi_send_callback);
    if (ret != BSP_OK) {
        LOGE("write command failed, ret = %d", ret);
        goto ERROR_EXIT;
    }
    MSG_QUEUE_RECV(lcd_spi_send_msg_id, &msg, 100);
#else
    ret = hal_spilcd_send(&cmd, 1);
    if (ret != BSP_OK) {
        LOGE("write command failed, ret = %d", ret);
        goto ERROR_EXIT;
    }
#endif

ERROR_EXIT:
    LCD_WRITE_CMD(1);
    LCD_WRITE_CS(1);

    return BSP_OK;
}

static int lcd_spi_write_data(const uint8_t *p_data, uint32_t data_size)
{
    int ret = 0;
    uint8_t msg = 0;
    uint32_t offset = 0;
    uint32_t remaining = data_size;

    LCD_WRITE_CS(0);
    LCD_WRITE_DATA(1);
    while (remaining > 0) {
        uint32_t packet_size = (remaining > DMA_MAX_SIZE) ? DMA_MAX_SIZE : remaining;
    #if IS_SUPPORT_LCD_SPI_DMA
        ret = hal_spilcd_dma_send(p_data + offset, packet_size, lcd_spi_send_callback);
        if (ret != BSP_OK) {
            LOGE("lcd spi write data failed, ret = %d", ret);
            goto ERROR_EXIT;
        }
        // BSP_LOG("lcd spi write data", "packet_size = %d, remaining = %d", packet_size, remaining);
        MSG_QUEUE_RECV(lcd_spi_send_msg_id, &msg, 1000);
    #else
        ret = hal_spilcd_send(p_data + offset, packet_size);
        if (ret != BSP_OK) {
            LOGE("lcd spi write data failed, ret = %d", ret);
            goto ERROR_EXIT;
        }
    #endif
        offset += packet_size;
        remaining -= packet_size;
    }
    // BSP_LOG("lcd spi write data", "end, data = %02X %02X", p_data[data_size - 1], p_data[data_size - 2]);

ERROR_EXIT:
    LCD_WRITE_CS(1);
    return BSP_OK;
}

static int lcd_spi_init(void)
{
    int ret = 0;
    struct HAL_SPI_CFG_T spi_cfg = {
        .clk_delay_half = false,
        .clk_polarity = false,
        .slave = false,
    #if IS_SUPPORT_LCD_SPI_DMA
        .dma_rx = true,
        .dma_tx = true,
    #else
        .dma_rx = false,
        .dma_tx = false,
    #endif
        .rx_sep_line = false,
        .ext_dss_rw = false,
        .skip_en = false,
        .rx_big_endian = false,
        .tx_big_endian = false,
        .cs = 0,
        .rx_bits = 8,
        .tx_bits = 8,
        .rx_frame_bits = 0,
        .rate = LCD_SPI_CLK_MHZ * 1000 * 1000,
        .samp_delay = 0,
    };
    hal_iomux_set_spilcd();
    ret = hal_spilcd_open(&spi_cfg);
    if (ret != BSP_OK) {
        LOGE("lcd spi open failed, ret = %d", ret);
        return BSP_ERROR;
    }
    return BSP_OK;
}

static int lcd_spi_deinit(void)
{
    int ret = 0;
    ret = hal_spilcd_close(0);
    if (ret != BSP_OK) {
        LOGE("lcd spi close failed, ret = %d", ret);
        return BSP_ERROR;
    }
    return BSP_OK;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_lcd_spi_init(void)
{
    int ret = 0;
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M); /// 系统频率影响SPI输出
#if IS_SUPPORT_LCD_TE_IRQ
    CREATE_MSG_TYPE_QUEUE(&lcd_te_irq_msg_id, 1, 1);
    if (lcd_te_irq_msg_id == NULL) {
        LOGE("lcd spi msg id create failed");
        return BSP_ERROR;
    }
    lcd_te_irq_init();
    lcd_te_irq_control(true);
#endif
#if IS_SUPPORT_LCD_SPI_ASYN_SEND
    CREATE_MSG_TYPE_QUEUE(&lcd_spi_send_msg_id, LCD_SPI_MESQ_NUM, 1);
    if (lcd_spi_send_msg_id == NULL) {
        LOGE("lcd spi msg id create failed");
        return BSP_ERROR;
    }
#endif
    ret = lcd_spi_init();
    if (ret != BSP_OK) {
        LOGE("lcd spi init failed, ret = %d", ret);
        return BSP_ERROR;
    }

    ret = lcd_spi_gpio_init();
    if (ret != BSP_OK) {
        LOGE("lcd spi gpio init failed, ret = %d", ret);
        return BSP_ERROR;
    }

    LOG_PBUS("lcd spi init ok");
    lcd_spi_ready = true;

    return BSP_OK;
}

int bsp_lcd_spi_resume(void)
{
    int ret = 0;
    /// 系统频率影响SPI输出
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    ret = lcd_spi_init();
    if (ret != BSP_OK) {
        LOGE("lcd spi resume failed, ret = %d", ret);
        return BSP_ERROR;
    }
    lcd_spi_ready = true;

    return BSP_OK;
}

int bsp_lcd_spi_suspend(void)
{
    int ret = 0;
    lcd_spi_ready = false;
#if IS_SUPPORT_LCD_SPI_ASYN_SEND
    uint8_t msg = 0;
    MSG_QUEUE_RECV(lcd_spi_send_msg_id, &msg, 100); /// 等待数据发送完毕 
#else
    msleep(10);
#endif
    ret = lcd_spi_deinit();
    if (ret != BSP_OK) {
        LOGE("lcd spi deinit failed, ret = %d", ret);
        return BSP_ERROR;
    }
    LOG_PBUS("lcd spi deinit ok");
    
    return BSP_OK; 
}

int bsp_lcd_spi_write_cmd_param(uint8_t cmd, const uint8_t *p_data, uint32_t data_size)
{
#if IS_SUPPORT_LCD_DEBUG
    uint32_t start_time = get_timestamp();
#endif
    if (!lcd_spi_ready) {
        LOGE("lcd spi is not ready");
        return BSP_ERROR;
    }
    
    if (lcd_spi_write_command(cmd) != BSP_OK) {
        return BSP_ERROR;
    }
    
    if (p_data == NULL || data_size == 0) {
        return BSP_OK;
    }

    if (lcd_spi_write_data(p_data, data_size) != BSP_OK) {
        return BSP_ERROR;
    }

#if IS_SUPPORT_LCD_DEBUG
    uint32_t end_time = get_timestamp();
    LOG_PBUS("write param time = %dms, data_size = %d", get_timestamp_ms(start_time), data_size);
#endif

    return BSP_OK;
}

int bsp_lcd_spi_write_img_param(uint8_t cmd, const uint8_t *p_data, uint32_t data_size)
{
    uint8_t msg = 0;
#if IS_SUPPORT_LCD_DEBUG
    uint32_t start_time = get_timestamp();
#endif
    if (!lcd_spi_ready) {
        LOGE("lcd spi is not ready");
        return BSP_ERROR;
    }
    /// 等待TE中断
    MSG_QUEUE_RESET(lcd_te_irq_msg_id);
    lcd_te_test_pin_toggle(2);
    MSG_QUEUE_RECV(lcd_te_irq_msg_id, &msg, 40);
    lcd_te_test_pin_toggle(3);
    if (lcd_spi_write_command(cmd) != BSP_OK) {
        return BSP_ERROR;
    }
    
    if (p_data == NULL || data_size == 0) {
        return BSP_OK;
    }

    if (lcd_spi_write_data(p_data, data_size) != BSP_OK) {
        return BSP_ERROR;
    }
#if IS_SUPPORT_LCD_DEBUG
    uint32_t end_time = get_timestamp();
    LOG_PBUS("write param time = %dms, data_size = %d", get_timestamp_ms(start_time), data_size);
#endif

    return BSP_OK;
}