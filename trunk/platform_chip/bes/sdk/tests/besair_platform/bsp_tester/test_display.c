/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifdef TEST_DISPLAY
#include "hal_trace.h"
#include "cmsis_os.h"

extern int fb_graphic_test(void);

osThreadId test_display_thread_id = NULL;
static void test_display_thread(const void *arg);
osThreadDef(test_display_thread, osPriorityAboveNormal, 1, 1024*2, "test_display_thread");

int test_display(void)
{
    int ret = 0;

    test_display_thread_id = osThreadCreate(osThread(test_display_thread), NULL);

    return ret;
}

static void test_display_thread(const void *arg)
{
    TRACE(1, "%s", __func__);
    fb_graphic_test();
}

#endif
