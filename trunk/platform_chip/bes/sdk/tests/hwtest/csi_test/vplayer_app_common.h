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
#ifndef __VPLAYER_APP_COMMON_H__
#define __VPLAYER_APP_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

enum VPLAYER_APP_MODE_T {
    VPLAYER_APP_MODE_NONE,
    VPLAYER_APP_MODE_CSI,
    VPLAYER_APP_MODE_DSI,
    VPLAYER_APP_MODE_CSI_DSI_LOOP,
};

#if defined(PSRAMUHS_ENABLE)
#define REMAP_C_ADDR(addr)   PSRAMUHS_NC_TO_C(addr)
#define REMAP_NC_ADDR(addr)  PSRAMUHS_NC_TO_C(addr)
#elif defined(PSRAM_ENABLE)
#define REMAP_C_ADDR(addr)   PSRAM_NC_TO_C(addr)
#define REMAP_NC_ADDR(addr)  PSRAM_NC_TO_C(addr)
#else
#define REMAP_C_ADDR(addr)   (addr)
#define REMAP_NC_ADDR(addr)  (addr)
#endif
#define REMAP_DMA_ADDR(addr) REMAP_NC_ADDR(addr)

#ifndef ALIGNED4
#define ALIGNED4 ALIGNED(4)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VPLAYER_APP_COMMON_H__ */
