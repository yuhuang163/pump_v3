/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "bsp_lcd_internal.h"

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
// static void (*s_lcd_ready_callback)(int type, int status);

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int lcd_dev_open(struct rt_device_node *dev)
{
    return bsp_lcd_uart_init();
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
    return 0;
}

static int lcd_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int lcd_dev_resume(struct rt_device_node *dev)
{
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
