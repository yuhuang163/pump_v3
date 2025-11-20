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

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include <string.h>
#include <stdlib.h>
#include "mbedtls/md5.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"
#include "hal_timer.h"
#include "hal_sec_eng.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "sec_alt.h"
#include "sec_lock.h"
#ifdef RTOS
    #include "cmsis_os.h"
#endif

#undef LOG_TAG
#define LOG_TAG "[hash_alt] "

// #define HASH_ALT_DEBUG
#ifdef HASH_ALT_DEBUG
    #define HASH_ALT_DBG(str,...) TRACE(0, LOG_TAG"%s:%d "str, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
    #define HASH_ALT_DBG(str,...)
#endif
#define HASH_ALT_ERR(str,...) TRACE(0, LOG_TAG"%s:%d "str, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define SEC_DATAMAX_LEN         2048 //1024
#define HASH_UINT_LEN           128
#define HASH_CTX_LEN            64
struct sec_hash_ctx {
    struct HAL_SE_HASH_CFG_T cfg;
    uint8_t seg_ctx_buf[HASH_CTX_LEN];
    uint8_t buf[HASH_UINT_LEN];
    uint32_t remain;
    uint32_t total;
    enum HAL_SE_HASH_MODE_T mode;
};

SEC_LOCK_DEC(hash_lock);

static struct sec_hash_ctx *last_hw_ctx = NULL;
static SRAM_BSS_LOC uint8_t data_buf[SEC_DATAMAX_LEN] ALIGNED(4);

POSSIBLY_UNUSED static int hash_alt_open(struct sec_hash_ctx *hw_ctx)
{
    int ret = -1;

    ret = sec_alt_hash_init(hw_ctx->mode);
    if (ret != HAL_SE_OK) {
        HASH_ALT_ERR("sec_alt_hash_init failed: ret=%d", ret);
        goto exit;
    }

    while (sec_alt_hash_busy()) {
#ifdef RTOS
        if (osKernelGetState() == osKernelRunning) {
            osDelay(1);
        } else {
            hal_sys_timer_delay_us(10);
        }
#else
        hal_sys_timer_delay_us(10);
#endif
    }

    last_hw_ctx = hw_ctx;

exit:
    HASH_ALT_DBG("context=%p, mode=%d, ret=%d", hw_ctx, hw_ctx->mode, ret);
    return ret;
}

POSSIBLY_UNUSED static int hash_alt_starts(struct sec_hash_ctx *hw_ctx, enum HAL_SE_HASH_MODE_T mode)
{
    int ret;

    if (!hw_ctx) {
        HASH_ALT_ERR("context is null.");
        return -1;
    }

    memset(hw_ctx, 0, sizeof(struct sec_hash_ctx));
    hw_ctx->mode = mode;

    sec_alt_lock(hash_lock);
    ret = hash_alt_open(hw_ctx);
    sec_alt_unlock(hash_lock);
    return ret;
}

POSSIBLY_UNUSED static int hash_alt_update(struct sec_hash_ctx *hw_ctx, const unsigned char *input, size_t length)
{
    struct HAL_SE_HASH_CFG_T *cfg = NULL;
    enum HAL_SE_RET_T ret = HAL_SE_OK;
    uint32_t max_seg_len = 0;
    uint32_t updated_len = 0;

    if (!hw_ctx) {
        HASH_ALT_ERR("context is null.");
        return -1;
    }

    if (!input || length <= 0) {
        return 0;
    }

    sec_alt_lock(hash_lock);
    if (last_hw_ctx != hw_ctx) {
        hash_alt_open(hw_ctx);
    }

    cfg = &(hw_ctx->cfg);
    max_seg_len = (hw_ctx->mode < HAL_SE_HASH_SHA512) ? 64 : 128;
    while (updated_len < length) {
        uint32_t last_len = length - updated_len + hw_ctx->remain;
        uint32_t in_len = (last_len > SEC_DATAMAX_LEN) ? SEC_DATAMAX_LEN : last_len;

        /* not enough for one segment transfer, save the last data */
        if (in_len < max_seg_len) {
            uint32_t save_len = in_len - hw_ctx->remain;
            memcpy(&hw_ctx->buf[hw_ctx->remain], input + updated_len, save_len);
            hw_ctx->remain += save_len;
            break;
        }

        /* alignment to max_seg_len */
        in_len = (in_len / max_seg_len) * max_seg_len;

        /* prepare the update data */
        if (hw_ctx->remain > 0) {
            memcpy(data_buf, hw_ctx->buf, hw_ctx->remain);
            memcpy(&data_buf[hw_ctx->remain], input + updated_len, in_len - hw_ctx->remain);
            updated_len += (in_len - hw_ctx->remain);
            hw_ctx->remain = 0;
        } else {
            memcpy(data_buf, input + updated_len, in_len);
            updated_len += in_len;
        }

        cfg->in = data_buf;
        cfg->in_len = in_len;
        cfg->seg_oper = (hw_ctx->total == 0) ? HAL_SE_SEG_INIT : HAL_SE_SEG_CONT;
        cfg->seg_ctx_buf = hw_ctx->seg_ctx_buf;
        cfg->seg_ctx_len = sizeof(hw_ctx->seg_ctx_buf);
        ret = sec_alt_hash_update(cfg);
        if (ret != HAL_SE_OK) {
            HASH_ALT_ERR("sec_alt_hash_update failed: ret=%d, context=%p, type=0x%x, \
                         data %p:%d, oper %d, ctx_buf %p:%d", ret, hw_ctx, hw_ctx->mode,
                         cfg->in, cfg->in_len, cfg->seg_oper, cfg->seg_ctx_buf, cfg->seg_ctx_len);
            break;
        }
        hw_ctx->total += in_len;
    }

    sec_alt_unlock(hash_lock);
    HASH_ALT_DBG("context=%p, input=%p, length=%d, ret=%d", hw_ctx, input, length, ret);
    return (ret == HAL_SE_OK) ? 0 : -1;
}

POSSIBLY_UNUSED static int hash_alt_finish(struct sec_hash_ctx *hw_ctx, const unsigned char *output, size_t length)
{
    struct HAL_SE_HASH_CFG_T *cfg = NULL;
    enum HAL_SE_RET_T ret = HAL_SE_OK;
    uint32_t rlen = 0;

    if (!hw_ctx) {
        HASH_ALT_ERR("context is null.");
        return -1;
    }
    if (!output || length <= 0) {
        return 0;
    }

    sec_alt_lock(hash_lock);
    if (last_hw_ctx != hw_ctx) {
        hash_alt_open(hw_ctx);
    }

    cfg = &(hw_ctx->cfg);
    cfg->in = data_buf;
    cfg->in_len = hw_ctx->remain;
    memcpy(data_buf, hw_ctx->buf, cfg->in_len);

    cfg->seg_oper = (hw_ctx->total == 0) ? HAL_SE_SEG_NONE : HAL_SE_SEG_TERM;
    cfg->seg_ctx_buf = (hw_ctx->total == 0) ? NULL : hw_ctx->seg_ctx_buf;
    cfg->seg_ctx_len = (hw_ctx->total == 0) ? 0 : sizeof(hw_ctx->seg_ctx_buf);

    hw_ctx->total += cfg->in_len;
    HASH_ALT_DBG("input=%p, length=%d, total=%d", cfg->in, cfg->in_len, hw_ctx->total);

    ret = sec_alt_hash_final(cfg, hw_ctx->total);
    if (ret != HAL_SE_OK) {
        HASH_ALT_ERR("sec_alt_hash_final failed: ret=%d, context=%p, total=%d",
                     ret, hw_ctx, hw_ctx->total);
        goto exit;
    }

    ret = sec_alt_hash_get_digest((void *)output, length, &rlen);
    if (ret != HAL_SE_OK) {
        HASH_ALT_ERR("sec_alt_hash_get_digest failed: ret=%d, context=%p", ret, hw_ctx);
        goto exit;
    }

    hw_ctx->remain = 0;
    hw_ctx->total = 0;

exit:
    last_hw_ctx = NULL;
    sec_alt_unlock(hash_lock);
    HASH_ALT_DBG("context=%p, output=%p, length=%d, ret=%d", hw_ctx, output, length, ret);
    return (ret == HAL_SE_OK) ? 0 : -1;
}

POSSIBLY_UNUSED static int hash_alt_init(struct sec_hash_ctx *hw_ctx)
{
    if (!hw_ctx) {
        HASH_ALT_ERR("context is null.");
        return -1;
    }

    sec_alt_open();
    if (!hash_lock) {
        sec_alt_lock_init(&hash_lock);
    }

    HASH_ALT_DBG("context=%p", hw_ctx);
    return 0;
}

POSSIBLY_UNUSED static int hash_alt_free(struct sec_hash_ctx *hw_ctx)
{
    if (!hw_ctx) {
        HASH_ALT_ERR("context is null.");
        return -1;
    }

    memset(hw_ctx, 0, sizeof(struct sec_hash_ctx));
    HASH_ALT_DBG("context=%p", hw_ctx);
    return 0;
}

POSSIBLY_UNUSED static int hash_alt_clone(struct sec_hash_ctx *des, const struct sec_hash_ctx *src)
{
    struct sec_hash_ctx *hw_ctx_des = (struct sec_hash_ctx *)des;
    struct sec_hash_ctx *hw_ctx_src = (struct sec_hash_ctx *)src;

    if (!des || !src) {
        return -1;
    }

    memcpy(hw_ctx_des, hw_ctx_src, sizeof(struct sec_hash_ctx));
    HASH_ALT_DBG("des=%p src=%p", des, src);
    return 0;
}

#if defined(MBEDTLS_MD5_C) && defined(MBEDTLS_MD5_ALT)
void mbedtls_md5_init(mbedtls_md5_context *ctx)
{
    hash_alt_init((struct sec_hash_ctx *)ctx);
}

void mbedtls_md5_free(mbedtls_md5_context *ctx)
{
    hash_alt_free((struct sec_hash_ctx *)ctx);
}

void mbedtls_md5_clone(mbedtls_md5_context *dst,
                       const mbedtls_md5_context *src)
{
    hash_alt_clone((struct sec_hash_ctx *)dst, (struct sec_hash_ctx *)src);
}

int mbedtls_md5_starts(mbedtls_md5_context *ctx)
{
    HASH_ALT_DBG("context=%p", ctx);
    return hash_alt_starts((struct sec_hash_ctx *)ctx, HAL_SE_HASH_MD5);
}

int mbedtls_md5_update(mbedtls_md5_context *ctx, const unsigned char *input, size_t ilen)
{
    HASH_ALT_DBG("context=%p len=%d in=%p", ctx, ilen, input);
    return hash_alt_update((struct sec_hash_ctx *)ctx, input, ilen);
}

#if !defined(MBEDTLS_MD5_PROCESS_ALT)
int mbedtls_internal_md5_process(mbedtls_md5_context *ctx,
                                 const unsigned char data[64])
{
    return mbedtls_md5_update(ctx, data, 64);
}
#endif /* !MBEDTLS_MD5_PROCESS_ALT */

int mbedtls_md5_finish(mbedtls_md5_context *ctx, unsigned char output[16])
{
    HASH_ALT_DBG("context=%p out=%p", ctx, output);
    return hash_alt_finish((struct sec_hash_ctx *)ctx, output, 16);
}
#endif

#if defined(MBEDTLS_SHA1_C) && defined(MBEDTLS_SHA1_ALT)
void mbedtls_sha1_init(mbedtls_sha1_context *ctx)
{
    hash_alt_init((struct sec_hash_ctx *)ctx);
}

void mbedtls_sha1_free(mbedtls_sha1_context *ctx)
{
    hash_alt_free((struct sec_hash_ctx *)ctx);
}

void mbedtls_sha1_clone(mbedtls_sha1_context *dst,
                        const mbedtls_sha1_context *src)
{
    hash_alt_clone((struct sec_hash_ctx *)dst, (struct sec_hash_ctx *)src);
}

int mbedtls_sha1_starts(mbedtls_sha1_context *ctx)
{
    HASH_ALT_DBG("context=%p", ctx);
    return hash_alt_starts((struct sec_hash_ctx *)ctx, HAL_SE_HASH_SHA1);
}

int mbedtls_sha1_update(mbedtls_sha1_context *ctx, const unsigned char *input, size_t ilen)
{
    HASH_ALT_DBG("context=%p len=%d in=%p", ctx, ilen, input);
    return hash_alt_update((struct sec_hash_ctx *)ctx, input, ilen);
}

#if !defined(MBEDTLS_SHA1_PROCESS_ALT)
int mbedtls_internal_sha1_process(mbedtls_sha1_context *ctx,
                                  const unsigned char data[64])
{
    return mbedtls_sha1_update(ctx, data, 64);
}
#endif /* !MBEDTLS_SHA1_PROCESS_ALT */

int mbedtls_sha1_finish(mbedtls_sha1_context *ctx, unsigned char output[20])
{
    HASH_ALT_DBG("context=%p out=%p", ctx, output);
    return hash_alt_finish((struct sec_hash_ctx *)ctx, output, 20);
}
#endif

#if defined(MBEDTLS_SHA256_C) && defined(MBEDTLS_SHA256_ALT)
void mbedtls_sha256_init(mbedtls_sha256_context *ctx)
{
    hash_alt_init((struct sec_hash_ctx *)ctx);
}

void mbedtls_sha256_free(mbedtls_sha256_context *ctx)
{
    hash_alt_free((struct sec_hash_ctx *)ctx);
}

void mbedtls_sha256_clone(mbedtls_sha256_context *dst,
                        const mbedtls_sha256_context *src)
{
    hash_alt_clone((struct sec_hash_ctx *)dst, (struct sec_hash_ctx *)src);
}

int mbedtls_sha256_starts(mbedtls_sha256_context *ctx, int is224)
{
    HASH_ALT_DBG("context=%p, is224=%d", ctx, is224);
    return hash_alt_starts((struct sec_hash_ctx *)ctx,
                            is224 ? HAL_SE_HASH_SHA224 : HAL_SE_HASH_SHA256);
}

int mbedtls_sha256_update(mbedtls_sha256_context *ctx,
                            const unsigned char *input, size_t ilen)
{
    HASH_ALT_DBG("context=%p len=%d in=%p", ctx, ilen, input);
    return hash_alt_update((struct sec_hash_ctx *)ctx, input, ilen);
}

#if !defined(MBEDTLS_SHA256_PROCESS_ALT)
int mbedtls_internal_sha256_process(mbedtls_sha256_context *ctx,
                                  const unsigned char data[64])
{
    return mbedtls_sha256_update(ctx, data, 64);
}
#endif /* !MBEDTLS_SHA256_PROCESS_ALT */

int mbedtls_sha256_finish(mbedtls_sha256_context *ctx, unsigned char *output)
{
    HASH_ALT_DBG("context=%p out=%p", ctx, output);
    return hash_alt_finish((struct sec_hash_ctx *)ctx, output, 32);
}
#endif

#if defined(MBEDTLS_SHA512_C) && defined(MBEDTLS_SHA512_ALT)
void mbedtls_sha512_init(mbedtls_sha512_context *ctx)
{
    hash_alt_init((struct sec_hash_ctx *)ctx);
}

void mbedtls_sha512_free(mbedtls_sha512_context *ctx)
{
    hash_alt_free((struct sec_hash_ctx *)ctx);
}

void mbedtls_sha512_clone(mbedtls_sha512_context *dst,
                        const mbedtls_sha512_context *src)
{
    hash_alt_clone((struct sec_hash_ctx *)dst, (struct sec_hash_ctx *)src);
}

int mbedtls_sha512_starts(mbedtls_sha512_context *ctx, int is384)
{
    HASH_ALT_DBG("context=%p", ctx);
    return hash_alt_starts((struct sec_hash_ctx *)ctx,
                            is384 ? HAL_SE_HASH_SHA384 : HAL_SE_HASH_SHA512);
}

int mbedtls_sha512_update(mbedtls_sha512_context *ctx, const unsigned char *input, size_t ilen)
{
    HASH_ALT_DBG("context=%p len=%d in=%p", ctx, ilen, input);
    return hash_alt_update((struct sec_hash_ctx *)ctx, input, ilen);
}

#if !defined(MBEDTLS_SHA512_PROCESS_ALT)
int mbedtls_internal_sha512_process(mbedtls_sha512_context *ctx,
                                  const unsigned char data[128])
{
    return mbedtls_sha512_update(ctx, data, 128);
}
#endif /* !MBEDTLS_SHA512_PROCESS_ALT */

int mbedtls_sha512_finish(mbedtls_sha512_context *ctx, unsigned char *output)
{
    HASH_ALT_DBG("context=%p out=%p", ctx, output);
    return hash_alt_finish((struct sec_hash_ctx *)ctx, output, 64);
}

#endif
