/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_config.h" // IWYU pragma: keep
#include "bsp_lcd_internal.h" // IWYU pragma: keep
#include "hal_iomux.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static bool lcd_qspi_ready = false;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if LV_USE_LOG && !LV_LOG_PRINTF
static void print_cb(lv_log_level_t level, const char *buf)
{
    static const char level_tag[] = "TIDEWU";
    char prefix[8];
    int prefix_len = snprintf(prefix, sizeof(prefix), "[%c] ",
                              (level < (int)sizeof(level_tag) - 1) ? level_tag[level] : '?');
    if (prefix_len > 0) {
        hal_trace_output((const unsigned char *)prefix, (unsigned int)prefix_len);
    }
    hal_trace_output((const unsigned char *)buf, strlen(buf));
    hal_trace_output((const unsigned char *)"\r\n", 2);
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_LVGL
int bsp_lcd_init(void) 
{
    lv_init();
#if LV_USE_LOG && !LV_LOG_PRINTF
    lv_log_register_print_cb(print_cb);
#endif
    lv_tick_set_cb(get_timestamp);
    lv_delay_set_cb(msleep);
    lv_hal_disp_init();
#if 0
    hal_iomux_set_analog_i2c(); /// 调试接口
    // while (1) {
    // }
#endif
    return BSP_OK;
}

int bsp_lcd_deinit(void) 
{
    lv_deinit();
    return BSP_OK;
}
#else
int bsp_lcd_init(void) 
{
    return BSP_OK;
}

int bsp_lcd_deinit(void) 
{
    return BSP_OK;
}
#endif

int bsp_lcd_uart_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    return BSP_OK;
}

int bsp_lcd_refresh(const void *buf, uint32_t n_bytes)
{
    return BSP_OK;
}

