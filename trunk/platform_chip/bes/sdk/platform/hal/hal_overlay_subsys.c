
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
#include "plat_types.h"
#include "plat_addr_map.h"
#include "hal_overlay.h"
#include "hal_sysfreq.h"
#include "hal_cache.h"
#include "hal_trace.h"
#include "cmsis.h"
#ifdef __ARMCC_VERSION
#include "link_sym_armclang.h"
#endif
#include "hal_overlay_subsys.h"
#include "mpu.h"
#include "system_subsys.h"

//TODO:only support m55 currently, hifi4 will be later
#if !defined(NO_OVERLAY) && defined(M55_ITCM_BASE)

#if defined(CHIP_ROLE_CP) || defined(CHIP_SUBSYS_M55C1) || defined(CHIP_SUBSYS_SENS) || \
        (defined(CHIP_SUBSYS_BTH) ^ defined(BTH_AS_MAIN_MCU))

#define CODE_OVERLAY_MAP_LOC                    __attribute__((section(".code_overlay_map")))

extern uint32_t __overlay_text_start__[];
extern uint32_t __overlay_text_end__[];
extern uint32_t __overlay_data_start__[];
extern uint32_t __overlay_data_end__[];
extern uint32_t __overlay_text0_load_offset__[];
extern uint32_t __overlay_data0_load_offset__[];
extern uint32_t __overlay_text1_load_offset__[];
extern uint32_t __overlay_data1_load_offset__[];
extern uint32_t __overlay_text2_load_offset__[];
extern uint32_t __overlay_data2_load_offset__[];
extern uint32_t __overlay_text3_load_offset__[];
extern uint32_t __overlay_data3_load_offset__[];
extern uint32_t __overlay_text4_load_offset__[];
extern uint32_t __overlay_data4_load_offset__[];
extern uint32_t __overlay_text5_load_offset__[];
extern uint32_t __overlay_data5_load_offset__[];
extern uint32_t __overlay_text6_load_offset__[];
extern uint32_t __overlay_data6_load_offset__[];
extern uint32_t __overlay_text7_load_offset__[];
extern uint32_t __overlay_data7_load_offset__[];
extern uint32_t __overlay_text0_size__[];
extern uint32_t __overlay_data0_size__[];
extern uint32_t __overlay_text1_size__[];
extern uint32_t __overlay_data1_size__[];
extern uint32_t __overlay_text2_size__[];
extern uint32_t __overlay_data2_size__[];
extern uint32_t __overlay_text3_size__[];
extern uint32_t __overlay_data3_size__[];
extern uint32_t __overlay_text4_size__[];
extern uint32_t __overlay_data4_size__[];
extern uint32_t __overlay_text5_size__[];
extern uint32_t __overlay_data5_size__[];
extern uint32_t __overlay_text6_size__[];
extern uint32_t __overlay_data6_size__[];
extern uint32_t __overlay_text7_size__[];
extern uint32_t __overlay_data7_size__[];

CODE_OVERLAY_MAP_LOC
const struct OVERLAY_INFO_T overlay_info = {
    .exec_info = {
        (uint32_t)__overlay_text_start__,
        (uint32_t)__overlay_text_end__,
        (uint32_t)__overlay_data_start__,
        (uint32_t)__overlay_data_end__,
    },

    .overlay_seg = {
        [0] = {
            (uint32_t)__overlay_text0_load_offset__,
            (uint32_t)__overlay_text0_size__,
            (uint32_t)__overlay_data0_load_offset__,
            (uint32_t)__overlay_data0_size__,
        },

        [1] = {
            (uint32_t)__overlay_text1_load_offset__,
            (uint32_t)__overlay_text1_size__,
            (uint32_t)__overlay_data1_load_offset__,
            (uint32_t)__overlay_data1_size__,
        },
        [2] = {
            (uint32_t)__overlay_text2_load_offset__,
            (uint32_t)__overlay_text2_size__,
            (uint32_t)__overlay_data2_load_offset__,
            (uint32_t)__overlay_data2_size__,
        },
        [3] = {
            (uint32_t)__overlay_text3_load_offset__,
            (uint32_t)__overlay_text3_size__,
            (uint32_t)__overlay_data3_load_offset__,
            (uint32_t)__overlay_data3_size__,
        },
        [4] = {
            (uint32_t)__overlay_text4_load_offset__,
            (uint32_t)__overlay_text4_size__,
            (uint32_t)__overlay_data4_load_offset__,
            (uint32_t)__overlay_data4_size__,
        },
        [5] = {
            (uint32_t)__overlay_text5_load_offset__,
            (uint32_t)__overlay_text5_size__,
            (uint32_t)__overlay_data5_load_offset__,
            (uint32_t)__overlay_data5_size__,
        },
        [6] = {
            (uint32_t)__overlay_text6_load_offset__,
            (uint32_t)__overlay_text6_size__,
            (uint32_t)__overlay_data6_load_offset__,
            (uint32_t)__overlay_data6_size__,
        },
        [7] = {
            (uint32_t)__overlay_text7_load_offset__,
            (uint32_t)__overlay_text7_size__,
            (uint32_t)__overlay_data7_load_offset__,
            (uint32_t)__overlay_data7_size__,
        },
    },
};

extern uint32_t __code_overlay_map_start[];
extern uint32_t __code_overlay_map_end[];
extern uint32_t __code_overlay_map_offset[];

SUBSYS_SEGMENT_LOC
const struct SUBSYS_SEGMENT overlay_seg = {
    .seg_name = ".overlay_info",
    .vaddr_start = (uint32_t)__code_overlay_map_start,
    .vaddr_end = (uint32_t)__code_overlay_map_end,
    .laddr_offset = (uint32_t)__code_overlay_map_offset,
};

#else // code for loader

#define CHECK_OVERLAY_SUBSYS_AND_ID(subsys, id) \
    do {\
        if ((subsys < 0) || (subsys >= OVERLAY_SUBSYS_QTY)) { \
            ASSERT(false, "overlay subsys error %d", subsys); \
        } \
        if ((id < 0) || (id >= HAL_OVERLAY_ID_QTY)) { \
            ASSERT(false, "overlay id error %d", id); \
        } \
    } while (0)

static uint32_t *overlay_text_start[OVERLAY_SUBSYS_QTY];

static uint32_t *overlay_data_start[OVERLAY_SUBSYS_QTY];

#define OVERLAY_IS_FREE  0
#define OVERLAY_IS_USED  1

static bool segment_state[OVERLAY_SUBSYS_QTY][HAL_OVERLAY_ID_QTY];

static bool overlay_inited;

static enum HAL_OVERLAY_ID_T cur_overlay_id[OVERLAY_SUBSYS_QTY];

static uint32_t * subsys_text_load_start[OVERLAY_SUBSYS_QTY][HAL_OVERLAY_ID_QTY];

static uint32_t * subsys_text_load_stop[OVERLAY_SUBSYS_QTY][HAL_OVERLAY_ID_QTY];

static uint32_t * subsys_data_load_start[OVERLAY_SUBSYS_QTY][HAL_OVERLAY_ID_QTY];

static uint32_t * subsys_data_load_stop[OVERLAY_SUBSYS_QTY][HAL_OVERLAY_ID_QTY];

enum HAL_OVERLAY_RET_T hal_overlay_subsys_set_text_exec_addr(
                    enum OVERLAY_SUBSYS_T subsys_id, uint32_t text_vma,
                    uint32_t text_size, uint32_t data_vma, uint32_t data_size)
{
    enum HAL_OVERLAY_RET_T ret;

    if (subsys_id >= OVERLAY_SUBSYS_QTY) {
        ASSERT(false, "overlay subsys error %d", subsys_id);
        return HAL_OVERLAY_RET_BAD_SUBSYS;
    }

    ret = HAL_OVERLAY_RET_OK;

    overlay_text_start[subsys_id] = (uint32_t *)text_vma;
    overlay_data_start[subsys_id] = (uint32_t *)data_vma;

    return ret;
}

enum HAL_OVERLAY_RET_T hal_overlay_subsys_set_text_addr(enum OVERLAY_SUBSYS_T subsys_id,
                        enum HAL_OVERLAY_ID_T id, uint32_t start, uint32_t stop)
{
    enum HAL_OVERLAY_RET_T ret;

    CHECK_OVERLAY_SUBSYS_AND_ID(subsys_id, id);
    ret = HAL_OVERLAY_RET_OK;

    subsys_text_load_start[subsys_id][id] = (uint32_t *)start;
    subsys_text_load_stop[subsys_id][id] = (uint32_t *)stop;

    return ret;
}

enum HAL_OVERLAY_RET_T hal_overlay_subsys_set_data_addr(enum OVERLAY_SUBSYS_T subsys_id,
                        enum HAL_OVERLAY_ID_T id, uint32_t start, uint32_t stop)
{
    enum HAL_OVERLAY_RET_T ret;

    CHECK_OVERLAY_SUBSYS_AND_ID(subsys_id, id);
    ret = HAL_OVERLAY_RET_OK;

    subsys_data_load_start[subsys_id][id] = (uint32_t *)start;
    subsys_data_load_stop[subsys_id][id] =  (uint32_t *)stop;

    return ret;
}

/*must called by lock get */
static void invalid_overlay_cache(enum OVERLAY_SUBSYS_T subsys_id, enum HAL_OVERLAY_ID_T id)
{
    return;
}

/*
 * acquire one overlay segment
 */
void hal_overlay_subsys_acquire(enum OVERLAY_SUBSYS_T subsys_id, enum HAL_OVERLAY_ID_T id)
{
    uint32_t lock;

    CHECK_OVERLAY_SUBSYS_AND_ID(subsys_id, id);
    segment_state[subsys_id][id] = OVERLAY_IS_USED;

    lock = int_lock();
    invalid_overlay_cache(subsys_id, id);
    int_unlock(lock);
    return;
}

/*
 * release one overlay segment
 */
void hal_overlay_subsys_release(enum OVERLAY_SUBSYS_T subsys_id, enum HAL_OVERLAY_ID_T id)
{
    CHECK_OVERLAY_SUBSYS_AND_ID(subsys_id, id);
    segment_state[subsys_id][id] = OVERLAY_IS_FREE;
    return;
}

/*
 * check if any overlay segment is used
 */
bool hal_overlay_subsys_is_used(enum OVERLAY_SUBSYS_T subsys_id)
{
    int i;
    uint32_t lock;

    lock = int_lock();
    for (i = 0; i < HAL_OVERLAY_ID_QTY; i++) {
        // if any segment is in use, the overlay is used;
        if (segment_state[subsys_id][i] == OVERLAY_IS_USED) {
            int_unlock(lock);
            return true;
        }
    }
    int_unlock(lock);
    return false;
}

enum HAL_OVERLAY_RET_T hal_overlay_subsys_load(enum OVERLAY_SUBSYS_T subsys_id,
                                                enum HAL_OVERLAY_ID_T id)
{
    enum HAL_OVERLAY_RET_T ret;

    CHECK_OVERLAY_SUBSYS_AND_ID(subsys_id, id);
    ret = HAL_OVERLAY_RET_OK;

    uint32_t lock;
    uint32_t *dst, *src;

    if (hal_overlay_subsys_is_used(subsys_id)) {
        ASSERT(0, "overlay %d-%d is in use", subsys_id, id);
        return HAL_OVERLAY_RET_IN_USE;
    }

    lock = int_lock();
    hal_sysfreq_req(HAL_SYSFREQ_USER_OVERLAY_SUBSYS, HAL_CMU_FREQ_52M);

    if (!overlay_inited) {
        overlay_inited = true;
        for (uint32_t i = 0; i < OVERLAY_SUBSYS_QTY; i++) {
            cur_overlay_id[i] = HAL_OVERLAY_ID_QTY;
        }
    }

    if (cur_overlay_id[subsys_id] == HAL_OVERLAY_ID_QTY) {
        cur_overlay_id[subsys_id] = HAL_OVERLAY_ID_IN_CFG;
    } else if (cur_overlay_id[subsys_id] == HAL_OVERLAY_ID_IN_CFG) {
        ret = HAL_OVERLAY_RET_IN_CFG;
    } else {
        ret = HAL_OVERLAY_RET_IN_USE;
    }

    if (ret != HAL_OVERLAY_RET_OK) {
        goto _exit;
    }

    hal_overlay_subsys_acquire(subsys_id, id);
    mpu_disable();

    dst = overlay_text_start[subsys_id];
    src = subsys_text_load_start[subsys_id][id];

#ifdef SUBSYS_CUSTOMIZED_COPY
    if (subsys_customized_copy(dst, src, (subsys_text_load_stop[subsys_id][id] - src) * 4) == 0) {
        goto _load_text_end;
    }
#endif

    for (; src < subsys_text_load_stop[subsys_id][id]; dst++, src++) {
        *dst = *src;
    }

_load_text_end: POSSIBLY_UNUSED;

    dst = overlay_data_start[subsys_id];
    src = subsys_data_load_start[subsys_id][id];

#ifdef SUBSYS_CUSTOMIZED_COPY
    if (subsys_customized_copy(dst, src, (subsys_data_load_stop[subsys_id][id] - src) * 4) == 0) {
        goto _load_data_end;
    }
#endif

    for (; src < subsys_data_load_stop[subsys_id][id]; dst++, src++) {
        *dst = *src;
    }

_load_data_end: POSSIBLY_UNUSED;

    mpu_enable();
    cur_overlay_id[subsys_id] = id;
    segment_state[subsys_id][id] = OVERLAY_IS_USED;

_exit:
    hal_sysfreq_req(HAL_SYSFREQ_USER_OVERLAY_SUBSYS, HAL_CMU_FREQ_32K);
    int_unlock(lock);

    return ret;
}

enum HAL_OVERLAY_RET_T hal_overlay_subsys_unload(enum OVERLAY_SUBSYS_T subsys_id,
                                                enum HAL_OVERLAY_ID_T id)
{
    enum HAL_OVERLAY_RET_T ret;

    CHECK_OVERLAY_SUBSYS_AND_ID(subsys_id, id);
    ret = HAL_OVERLAY_RET_OK;

    uint32_t lock;

    lock = int_lock();
    if (cur_overlay_id[subsys_id] == id) {
        cur_overlay_id[subsys_id] = HAL_OVERLAY_ID_QTY;
    } else if (cur_overlay_id[subsys_id] == HAL_OVERLAY_ID_IN_CFG) {
        ret = HAL_OVERLAY_RET_IN_CFG;
    } else {
        ret = HAL_OVERLAY_RET_IN_USE;
    }
    hal_overlay_subsys_release(subsys_id, id);
    int_unlock(lock);

    return ret;
}

static const struct OVERLAY_INFO_T *subsys_loader_get_overlay_info(uint32_t addr,
                                                        struct SUBSYS_SEGMENT *seg)
{
    return (const struct OVERLAY_INFO_T *)(addr + seg->laddr_offset);
}

static int subsys_loader_setup_overlay(enum OVERLAY_SUBSYS_T subsys_id,
                                uint32_t addr, struct SUBSYS_SEGMENT *seg,
                                HAL_OVERLAY_CONV_CB conv_cb)
{
    const struct OVERLAY_INFO_T *overlay_info = subsys_loader_get_overlay_info(addr, seg);
    const struct SUBSYS_OVERLAY_EXEC_T *exec_info = &overlay_info->exec_info;
    const struct SUBSYS_OVERLAY_ITEM_T *seg_info = &overlay_info->overlay_seg[0];

    uint32_t overlay_text_sz = exec_info->text_end_addr - exec_info->text_start_addr;
    uint32_t overlay_data_sz = exec_info->data_end_addr - exec_info->data_start_addr;
    uint32_t text_vma = exec_info->text_start_addr;
    uint32_t text_size = overlay_text_sz;

    uint32_t data_vma = exec_info->data_start_addr;
    uint32_t data_size = overlay_data_sz;

    if (conv_cb) {
        text_vma = conv_cb(text_vma);
        data_vma = conv_cb(data_vma);
    }

    hal_overlay_subsys_set_text_exec_addr(subsys_id, text_vma, text_size, data_vma, data_size);

    for (int id = 0; id < SUBSYS_OVERLAY_CNT; id++) {
        uint32_t tload_start, tload_stop;
        uint32_t dload_start, dload_stop;

        tload_start = addr + seg_info->text_offset;
        tload_stop = tload_start + seg_info->text_size;
        hal_overlay_subsys_set_text_addr(subsys_id, id, tload_start, tload_stop);

        dload_start = addr + seg_info->data_offset;
        dload_stop = dload_start + seg_info->data_size;
        hal_overlay_subsys_set_data_addr(subsys_id, id, dload_start, dload_stop);

        seg_info++;
    }

    //for test...
    /* hal_overlay_subsys_load(subsys_id, 1); */

    return 0;
}

int hal_overlay_subsys_init(uint32_t image_addr, enum OVERLAY_SUBSYS_T subsys_id,
                                                SUBSYS_EXEC_ADDR_CONV_CB conv_cb)
{
    struct SUBSYS_SEGMENT *overlay_seg;
    int ret;

    overlay_seg = subsys_loader_get_segment(image_addr, ".overlay_info");
    if (!overlay_seg)
        return 1;

    ret = subsys_loader_setup_overlay(subsys_id, image_addr, overlay_seg, conv_cb);
    if (ret) {
        return 2;
    }

    return 0;
}

#endif // code for loader

#endif // !NO_OVERLAY && M55_ITCM_BASE
