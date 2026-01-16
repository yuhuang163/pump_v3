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

#ifndef _GPU_PORT_H
#define _GPU_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief GPU hardware initialize, clock/irq/memory.
 */
void gpu_hal_init(void);

/*
 * @brief GPU hardware disable.
 */
void gpu_hal_exit(void);

/*
 * @brief GPU hardware reset.
 */
void gpu_hal_reset(void);

/*
 * @brief memory alloc from GPU own heap.
 */
void *gpu_hal_malloc(uint32_t size);

/*
 * @brief realloc memory from GPU own heap.
 */
void *gpu_hal_realloc(void *buf, uint32_t size);

/*
 * @brief memory free.
 */
void gpu_hal_free(void *mem);

/*
 * @brief GPU mutex lock
 */
int gpu_hal_os_lock(void);

/*
 * @brief GPU mutex unlock
 */
int gpu_hal_os_unlock(void);

/*
 * @brief Save GPU current thread task pointer.
 */
int gpu_hal_os_set_tls(void *task);

/*
 * @brief Get  GPU current thread task pointer.
 */
void *gpu_hal_os_get_tls(void);

/*
 * @brief sync GPU command buffer.
 */
void gpu_hal_cache_sync(void);

/*
 * @brief wait for GPU irq.
 */
int gpu_hal_wait_done(void);

/*
 * @brief get GPU working frequency in Mhz
 */
uint32_t gpu_hal_get_freqmhz(void);

/*
 * @brief set GPU working frequency in Mhz
 */
void gpu_hal_set_freqmhz(uint32_t freqmhz);

/*
 * @brief GPU enter low power mode.
 */
void gpu_hal_enter_lpwr(void);

/*
 * @brief GPU exit low power mode.
 */
void gpu_hal_exit_lpwr(void);

/*
 * @brief Sync all data cache.
 */
void gpu_hal_cache_sync_all(void);

/*
 * @brief Convert buffer addr for GPU view.
 */
uint32_t gpu_hal_buf_addr(uint32_t addr);

#ifdef __cplusplus
}
#endif

#endif //_GPU_PORT_H
