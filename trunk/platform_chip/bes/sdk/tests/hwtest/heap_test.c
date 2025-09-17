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
#ifdef HEAP_TEST

#include <string.h>
#include <stdlib.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "heap_api.h"

#if 1
    #define HEAP_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
    #define HEAP_DUMP32(n, s, ...)   DUMP32(n, s, ##__VA_ARGS__)
#else
    #define HEAP_TRACE(n, s, ...)
    #define HEAP_DUMP32(n, s, ...)
#endif

#if 1
    #define HEAP_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
#else
    #define HEAP_DELAY_MS(ms)
#endif

#define HEAP_BUFF_SIZE (128*1024)
static multi_heap_handle_t multi_heap_handle;
static uint32_t heap_buff[HEAP_BUFF_SIZE / 4];

static void test_heap_init(void *start, size_t size)
{
    multi_heap_handle = heap_register(start, size);

    HEAP_TRACE(1, "%s:%d, multi_heap_handle=0x%x, start=0x%x, size=%d",
               __func__, __LINE__, (uint32_t)multi_heap_handle, (uint32_t)start, (uint32_t)size);
}

/* #define HEAP_TEST_ALIGN */

#ifdef HEAP_TEST_ALIGN
#define HEAP_ALIGN_SZ (64)
#endif

static void *test_heap_malloc(size_t size)
{
    void *ptr = NULL;

    HEAP_TRACE(1, "%s:%d, size=%d", __func__, __LINE__, (uint32_t)size);
#ifdef HEAP_TEST_ALIGN
    ptr = heap_malloc_align(multi_heap_handle, size, HEAP_ALIGN_SZ);
#else
    ptr = heap_malloc(multi_heap_handle, size);
#endif

    ASSERT(ptr != NULL, "%s:%d, malloc failed, size=%d", __func__, __LINE__, (uint32_t)size);
    HEAP_TRACE(1, "malloc done, ptr=0x%x, size=%d", (uint32_t)ptr, (uint32_t)size);

    return ptr;
}

static void test_heap_free(void *ptr)
{
#ifdef HEAP_TEST_ALIGN
    heap_free_align(multi_heap_handle, ptr);
#else
    heap_free(multi_heap_handle, ptr);
#endif

    HEAP_TRACE(1, "%s:%d, ptr=0x%x, free done", __func__, __LINE__, (uint32_t)ptr);
}

static void rand_generator(uint32_t *dest, uint32_t size)
{
    while (size--) {
        *dest++ = (uint32_t)rand();
    }
}

#define HEAP_TEST_CNT   (8/2)
#define HEAP_TEST_SIZE  8192

#ifdef HEAP_GUARD_TEST
static void heap_guard_test(uint32_t *p)
{
    uint32_t len = HEAP_TEST_SIZE;
    multi_heap_enable_tail_guard(multi_heap_handle, p, MEMORY_GUARD_1);
    memset(p, 0x0, len + 10);
}
#endif

void heap_test(void)
{
    uint32_t i;
    uint32_t test_cnt = 0;
    uint32_t fail_cnt = 0;
    uint32_t len = HEAP_TEST_SIZE / sizeof(uint32_t);
    uint32_t *src_addr[HEAP_TEST_CNT];
    uint32_t *dst_addr[HEAP_TEST_CNT];

    HEAP_TRACE(0, "  ");
    HEAP_TRACE(0, "------heap malloc/free test started------");
    HEAP_TRACE(0, "  ");

    test_heap_init((void *)heap_buff, sizeof(heap_buff));
    srand(hal_sys_timer_get());
    while (1) {
        //allocate memory
        for (i = 0; i < HEAP_TEST_CNT; i++) {
            src_addr[i] = (uint32_t *)test_heap_malloc(HEAP_TEST_SIZE);
            dst_addr[i] = (uint32_t *)test_heap_malloc(HEAP_TEST_SIZE);
            memset((uint32_t *)src_addr[i], 0x0, len);
            memset((uint32_t *)dst_addr[i], 0x0, len);
        }

#ifdef HEAP_GUARD_TEST
        heap_guard_test(dst_addr[0]);
#endif

        //show the first address of each memory
        HEAP_TRACE(0, "  ");
        for (i = 0; i < HEAP_TEST_CNT; i++) {
            HEAP_TRACE(4, "src_addr[%d]:0x%x, dst_addr[%d]:0x%x", i, (uint32_t)src_addr[i], i, (uint32_t)dst_addr[i]);
        }
        HEAP_TRACE(0, "  ");

        //fill memory
        for (i = 0; i < HEAP_TEST_CNT; i++) {
            rand_generator(src_addr[i], len);

            HEAP_TRACE(TR_ATTR_NO_LF, "src[%d] data:", i);
            HEAP_DUMP32("%08X ", (uint32_t *)src_addr[i], 8);

            memcpy((uint32_t *)dst_addr[i], (uint32_t *)src_addr[i], len);
            HEAP_TRACE(TR_ATTR_NO_LF, "dst[%d] data:", i);
            HEAP_DUMP32("%08X ", (uint32_t *)dst_addr[i], 8);
        }
        HEAP_TRACE(0, "  ");

        //compare data
        HEAP_TRACE(0, "===============================");
        for (i = 0; i < HEAP_TEST_CNT; i++) {
            if (memcmp((uint32_t *)dst_addr[i], (uint32_t *)src_addr[i], len) == 0) {
                HEAP_TRACE(0, "---src/dst_addr[%d] test success", i);
            } else {
                HEAP_TRACE(0, "***src/dst_addr[%d] test failure", i);
                fail_cnt++;
            }
        }
        HEAP_TRACE(0, "===============================");
        HEAP_TRACE(0, "  ");

        /*if the heap guard was enable, before free the memory, disaable the guard*/
        /* multi_heap_disable_tail_guard(multi_heap_handle, dst_addr[0], MEMORY_GUARD_1); */

        //release memory
        for (i = 0; i < HEAP_TEST_CNT; i++) {
            test_heap_free(src_addr[i]);
            test_heap_free(dst_addr[i]);
        }

        test_cnt++;
        HEAP_TRACE(0, "  ");
        HEAP_TRACE(0, "---------------------------------------------test cnt:%d, fail cnt:%d", test_cnt, fail_cnt);
        HEAP_TRACE(0, "  ");
        HEAP_DELAY_MS(200);
    }
}

#endif

