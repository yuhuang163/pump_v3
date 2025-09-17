
/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "hwtest.h"
#include "string.h"

extern int fb_dsi_1s_test(void);
extern int fb_graphic_test(void);
extern int gra_test_main(void);
extern int fb_dsi_60hz_test(void);
extern int fb_dsi_60hz_cpu_sim_test(void);

int run_graphic_test_cases(void)
{
    int ret = 0;

#if defined(GRA_TEST_APP)
    gra_test_main();
    return ret;
#endif

#if defined(FB_DSI_1S_TEST)
    ret = fb_dsi_1s_test();
#elif defined(FB_DSI_60HZ_TEST)
    ret = fb_dsi_60hz_test();
#elif defined(FB_DSI_60HZ_CPU_SIM_TEST)
    ret = fb_dsi_60hz_cpu_sim_test();
#elif defined(FB_GRAPHIC_TEST)
    ret = fb_graphic_test();
#else
    extern int sim_dma2d_test(void);
    extern void lcdc_test_sim(void);
    //sim_dma2d_test();
    //lcdc_test_sim();
#endif
    return ret;
}
