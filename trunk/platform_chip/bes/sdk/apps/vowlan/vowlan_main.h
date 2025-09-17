/***************************************************************************
 *
 * Copyright 2015-2022 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __VOWLAN_MAIN_H__
#define __VOWLAN_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* vowlan status machine */
#define VOW_STATUS_NULL             (0)
#define VOW_STATUS_CONNECTED        (1 << 0)
#define VOW_STATUS_PAIRING          (1 << 1)
#define VOW_STATUS_STREAM_IDLE      (1 << 2)
#define VOW_STATUS_STREAM_TRANSFE   (1 << 3)
#define VOW_STATUS_MASK             (0x0F)

void vow_set_status(uint8_t status);
uint8_t vow_get_status(void);
uint8_t vow_get_old_status(void);
void vow_status_printf(uint8_t old_status, uint8_t status);
void vowlan_app_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* __VOWLAN_MAIN_H__ */
