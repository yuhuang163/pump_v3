/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_config.h" // IWYU pragma: keep
#include "bsp_lcd_internal.h" // IWYU pragma: keep

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
    hal_trace_output((const unsigned char *)buf, strlen(buf));
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_lcd_init(void) 
{
    lv_init();
#if LV_USE_LOG && !LV_LOG_PRINTF
    lv_log_register_print_cb(print_cb);
#endif
    lv_tick_set_cb(get_timestamp);
    lv_delay_set_cb(msleep);
    lv_hal_disp_init();
    return BSP_OK;
}

int bsp_lcd_deinit(void) 
{
    lv_deinit();
    return BSP_OK;
}

int bsp_lcd_uart_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    return BSP_OK;
}

int bsp_lcd_refresh(const void *buf, uint32_t n_bytes)
{
    return BSP_OK;
}

