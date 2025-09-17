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
#ifndef __LCDC_DRIVER_H__
#define __LCDC_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_dsi.h"

#if 1
enum DSI_MODE_T {
    DSI_MODE_VIDEO,
    DSI_MODE_CMD,
};
#endif

typedef void (*HAL_DSI_XFER_COMPLETE_CALLBACK_T)(uint8_t layerId, uint8_t channel, uint32_t addr);

void lcdc_hw_init(const struct HAL_DSI_CFG_T *cfg,
    const uint8_t *layer0, const uint8_t *layer1, const uint8_t *layer2);

void lcdc_hw_start(void);

void lcdc_hw_setup_callback(HAL_DSI_XFER_COMPLETE_CALLBACK_T callback);

void lcdc_hw_update_dma_addr(uint8_t layerId, uint8_t channel, uint32_t addr);

void lcdc_hw_gamma_enable(const uint8_t *config_R, const uint8_t *config_G, const uint8_t *config_B);

void lcdc_hw_gamma_disable(void);

void lcdc_hw_itf_init(uint16_t h_res, enum DSI_MODE_T mode, uint8_t lane_number,
	uint32_t dsi_bitclk, uint32_t dsi_pclk);

void lcdc_hw_itf_start(void);

#ifdef __cplusplus
}
#endif

#endif /* __LCDC_DRIVER_H__ */
