#ifndef __BLE_TESTS_H__
#define __BLE_TESTS_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h>

/*
* GLOBAL FUNCTION DECLARATION
******************************************************************************
*/
void ble_test_enter_signalingtest_mode(void);
void ble_test_enter_nosignalingtest_mode(void);
int ble_test_xtal_calib_proc_start(void);
unsigned char ble_test_xtal_calib_set(uint32_t fcap);
unsigned char ble_test_xtal_calib_get(uint32_t *fcap);
unsigned char ble_test_xtal_calib_end(void);
unsigned char ble_test_get_rssi_value(int32_t *rssi);

#ifdef __cplusplus
}
#endif

#endif