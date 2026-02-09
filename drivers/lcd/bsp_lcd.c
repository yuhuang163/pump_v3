/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_pwm.h" // IWYU pragma: keep
#include "bsp_lcd_internal.h"
#include "bsp_config.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
#define LCD_POWER_ON               1
#define LCD_POWER_OFF              0
#define IS_SUPPORT_LCD_BL          1
#define IS_SUPPORT_LCD_POWER       0
#else
#define LCD_POWER_ON     1
#define LCD_POWER_OFF    0
#define IS_SUPPORT_LCD_BL          0
#define IS_SUPPORT_LCD_POWER       1
#endif

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;
static bool lcd_device_is_ready = false;
static void (*s_lcd_ready_callback)(int type, int status);

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void lcd_power_init(void)
{
#if IS_SUPPORT_LCD_POWER
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)USR_LCD_POWER_PIN, 
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(USR_LCD_POWER_PIN, HAL_GPIO_DIR_OUT, LCD_POWER_OFF);
#endif
}

static void lcd_power_control(bool enable)
{
#if IS_SUPPORT_LCD_POWER
    GPIO_PIN_CTRL(USR_LCD_POWER_PIN, enable ? LCD_POWER_ON : LCD_POWER_OFF);
    LOG_PBUS("lcd_power_control: %d", enable);
#endif
}

static void bsp_add_lcd_ready_callback(void (*callback)(int type, int status))
{
    s_lcd_ready_callback = callback;
}

static int lcd_dev_open(struct rt_device_node *dev)
{
    int ret = 0;
    lcd_power_init();
    bsp_lcd_backlight_init();
    lcd_power_control(true);
    msleep(10);
    ret = bsp_lcd_init();
    if (ret != BSP_OK) {
        LOGE("lcd init failed");
        return ret;
    }
    
    if (s_lcd_ready_callback) {
        s_lcd_ready_callback(0, true);
    }
    return BSP_OK;
}

static int lcd_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int lcd_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return bsp_lcd_refresh(buf, n_bytes);
}

static int lcd_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    if (op_code == OP_CODE_PANEL_ADD_READY_CB) {
        bsp_add_lcd_ready_callback((void (*)(int, int))args);
        return 0;
    }
#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1)   
    return bsp_lcd_uart_dev_ioctl(dev, op_code, args);
#else
    if (op_code == OP_CODE_PANEL_SET_BL) {
        uint32_t *tmp = (uint32_t *)args;
        if (tmp) {
            bsp_lcd_backlight_control((uint8_t)(tmp[0]));
        }
        return 0;
    }
    return BSP_OK;
#endif
}

static int lcd_dev_suspend(struct rt_device_node *dev)
{
    lcd_device_is_ready = false;
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
    bsp_lcd_backlight_suspend();
    lcd_power_control(false);
    bsp_lcd_deinit();
#endif
    if (s_lcd_ready_callback) {
        s_lcd_ready_callback(0, false);
    }
    return 0;
}

static int lcd_dev_resume(struct rt_device_node *dev)
{
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
    lcd_power_control(true);
    bsp_lcd_backlight_resume();
    msleep(10);
    bsp_lcd_init();
#endif
    lcd_device_is_ready = true;
    if (s_lcd_ready_callback) {
        s_lcd_ready_callback(0, true);
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_lcd(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));

    node->name = "lcd";
    node->id = ID_DEVICE_DISPLAY_PANEL;
    node->open = lcd_dev_open;
    node->close = lcd_dev_close;
    node->suspend = lcd_dev_suspend;
    node->resume = lcd_dev_resume;
    node->write = lcd_dev_write;
    node->ioctl = lcd_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}