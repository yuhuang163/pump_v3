/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __QRCODE_GUI_H__
#define __QRCODE_GUI_H__

#include "stdint.h"
#include "zbar_detector.h"

#ifdef __cplusplus
extern "C" {
#endif

void qr_gui_show_video_player(void);
void qr_gui_show_blank(void);
void qr_gui_show_result(void);

void qr_gui_img_capture_get(uint32_t addr );
void qr_gui_get_result(qrcode_result_t *buf);
void qr_gui_show_data_refresh(uint32_t , uint32_t, uint32_t);


#ifdef __cplusplus
}
#endif
#endif /* __QRCODE_GUI_H__ */
