/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_lcd_internal.h"
#include "bsp_config.h"

/*
* DEFINES
*****************************************************************************************
*/
#define LCD_IMG_SIZE                   (LCD_H_RES * LCD_V_RES * sizeof(uint16_t))
#define LCD_PIXEL_SIZE                 (LCD_H_RES * LCD_V_RES)
#define LCD_SLICE_COUNT                20
#define LCD_SLICE_HEIGHT               (LCD_V_RES / LCD_SLICE_COUNT)
#define LCD_SLICE_PIXELS               (LCD_H_RES * LCD_SLICE_HEIGHT)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;
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
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux = {
        (enum HAL_IOMUX_PIN_T)LCD_POWER_PIN, 
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL,
    };
    hal_iomux_init(&pinmux, 1);
    hal_gpio_pin_set_dir(LCD_POWER_PIN, HAL_GPIO_DIR_OUT, 0);
}

static void lcd_power_control(bool enable)
{
    GPIO_PIN_CTRL(LCD_POWER_PIN, enable ? 1 : 0);
    LOG_PBUS("lcd_power_control: %d", enable);
}

static void bsp_add_lcd_ready_callback(void (*callback)(int type, int status))
{
    s_lcd_ready_callback = callback;
}

static int lcd_dev_open(struct rt_device_node *dev)
{
    lcd_power_init();
    msleep(10);
    lcd_power_control(true);
    msleep(10);
    int ret = bsp_lcd_init();
    if (s_lcd_ready_callback) {
        s_lcd_ready_callback(0, true);
    }
    return ret;
}

static int lcd_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int lcd_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return 0;
}

static int lcd_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    uint32_t *tmp = NULL;
    switch (op_code) {
        case OP_CODE_PANEL_CLEAR:
            tmp = (uint32_t *)args;
            // return bsp_lcd_color_set(NULL, (uint16_t)tmp[0]);
            return 0;
        case OP_CODE_PANEL_SET_BL:
            tmp = (uint32_t *)args;
            lcd_backlight_control(tmp[0]);
            return 0;

        case OP_CODE_PANEL_ADD_READY_CB:
            bsp_add_lcd_ready_callback((void (*)(int, int))args);
            return 0;
        case OP_CODE_PANEL_DATA_SEND:
            tmp = (uint32_t *)args;
            EXCEPTION_CHECK_INT(tmp);
            // LOG_PBUS("lcd data send: %d, %p", tmp[0], tmp[1]);
            return bsp_lcd_data_send((uint8_t)tmp[0], (lcd_tlvs_t *)tmp[1], tmp[2]);
            // return 0;
        default:
            LOGE("lcd err op_code: %lx", op_code);
            return -1;
    }
}

static int lcd_dev_suspend(struct rt_device_node *dev)
{
    if (s_lcd_ready_callback) {
        s_lcd_ready_callback(0, false);
    }
    return 0;
}

static int lcd_dev_resume(struct rt_device_node *dev)
{
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
