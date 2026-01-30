/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"  // IWYU pragma: keep
#include "gatt_service.h" // IWYU pragma: keep
#include "ble_core_common.h" // IWYU pragma: keep
#include "nvrecord_dev.h"
#include "bt_drv_reg_op.h"
#include "hal_bootmode.h"
#include "apps.h"
/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
/// ======================== 蓝牙测试相关函数封装 ========================

/*
* EXTERNAL FUNCTION DECLARATIONS (蓝牙测试相关)
*****************************************************************************************
*/
// 前向声明，避免头文件依赖问题
// app_enter_signalingtest_mode 和 app_enter_non_signalingtest_mode 已在 apps.h 中声明
extern int app_factorymode_bt_xtalcalib_proc(void);

void ble_test_enter_signalingtest_mode(void)
{
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
    app_enter_signalingtest_mode();
}

void ble_test_enter_nosignalingtest_mode(void)
{
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
    app_enter_non_signalingtest_mode();
}

int ble_test_xtal_calib_proc_start(void)
{
    // 前向声明，函数在 app_factory_bt.cpp 中定义（需要 __FACTORY_MODE_SUPPORT__ 宏）
    extern int app_factorymode_bt_xtalcalib_proc(void);
#ifdef __FACTORY_MODE_SUPPORT__
    //return app_factorymode_bt_xtalcalib_proc();
#else
    // 如果 FACTORY_MODE 未启用，返回错误
    return 0;
#endif
}

unsigned char ble_test_xtal_calib_set(uint32_t fcap)
{
/*   btdrv_vco_test_start(0);
    osDelay(200);
    btdrv_rf_set_xtal_fcap(fcap, 1);//todo
    osDelay(200);*/ 
    nvrec_dev_set_xtal_fcap(fcap);
    return 0;
}

unsigned char ble_test_xtal_calib_get(uint32_t *fcap)
{
    if (fcap == NULL) {
        return 1; // 返回错误：空指针
    }
    
    int ret = nvrec_dev_get_xtal_fcap(fcap);
    osDelay(200);
    
    if (ret != 0) {
        return 1; // 返回错误：获取失败
    }
    
    return 0; // 成功
}

unsigned char ble_test_xtal_calib_end(void)
{
    /*nvrec_dev_set_xtal_fcap(btdrv_rf_get_xtal_fcap());
    osDelay(200);
    btdrv_vco_test_stop();
    osDelay(200);*/
    return 0;
}

unsigned char  ble_test_get_rssi_value(int32_t *rssi)
{
    if (rssi == NULL) {
        return 1; // 返回错误：空指针
    }
    
    rx_agc_t rx_val;
    //bool ret = bt_drv_reg_op_read_rssi_in_dbm(0x80, &rx_val);
    //if (ret) {
    //    *rssi = rx_val.rssi;
    //}
    bool ret = bt_drv_reg_op_read_ble_rssi_in_dbm(0x00, &rx_val);
    if (ret) {
        *rssi = rx_val.rssi;
        return 0; // 成功
    }
    return 1; // 返回错误：获取失败
}
