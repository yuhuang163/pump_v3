/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#ifndef __SEC_HAL_H__
#define __SEC_HAL_H__

#include "hal_sec_eng.h"

enum HAL_SE_RET_T sec_alt_open(void);
enum HAL_SE_RET_T sec_alt_close(void);

enum HAL_SE_RET_T sec_alt_aes_encrypt(struct HAL_SE_AES_CFG_T *cfg);
enum HAL_SE_RET_T sec_alt_aes_decrypt(struct HAL_SE_AES_CFG_T *cfg);

enum HAL_SE_RET_T sec_alt_hash_init(enum HAL_SE_HASH_MODE_T mode);
int sec_alt_hash_busy(void);
enum HAL_SE_RET_T sec_alt_hash_update(struct HAL_SE_HASH_CFG_T *cfg);
enum HAL_SE_RET_T sec_alt_hash_final(struct HAL_SE_HASH_CFG_T *cfg, uint64_t total_in_len);
enum HAL_SE_RET_T sec_alt_hash_get_digest(void *out, uint32_t out_len, uint32_t *real_len);

#endif /* __SEC_HAL_H__ */
