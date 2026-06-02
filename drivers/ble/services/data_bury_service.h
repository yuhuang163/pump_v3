#ifndef __DATA_BURY_SERVICE_H__
#define __DATA_BURY_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Momcozy 智能泌乳专项-数据埋点服务 (协议 v1.0.2)
 * Service: 9F6B1A20-3C4D-4E5F-A601-7B8C9D0E1122
 * Write:   9F6B1A21-3C4D-4E5F-A601-7B8C9D0E1122  (Write Without Response)
 * Notify:  9F6B1A22-3C4D-4E5F-A601-7B8C9D0E1122
 ******************************************************************************/
#include "bsp_common.h"

/* 128-bit UUID, little-endian byte order (同 GATT 栈约定) */
#define ROOT_DATA_BURY_SRV_UUID_128 \
    0x22, 0x11, 0x0e, 0x9d, 0x8c, 0x7b, 0x01, 0xa6, 0x5f, 0x4e, 0x4d, 0x3c, 0x20, 0x1a, 0x6b, 0x9f

#define ROOT_DATA_BURY_SRV_WRITE_CHAR_UUID_128 \
    0x22, 0x11, 0x0e, 0x9d, 0x8c, 0x7b, 0x01, 0xa6, 0x5f, 0x4e, 0x4d, 0x3c, 0x21, 0x1a, 0x6b, 0x9f

#define ROOT_DATA_BURY_SRV_NOTIFY_CHAR_UUID_128 \
    0x22, 0x11, 0x0e, 0x9d, 0x8c, 0x7b, 0x01, 0xa6, 0x5f, 0x4e, 0x4d, 0x3c, 0x22, 0x1a, 0x6b, 0x9f

void ble_data_bury_service_init(void);
bool ble_data_bury_service_notify_status_get(void);
int ble_data_bury_service_notify_data_send(uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
