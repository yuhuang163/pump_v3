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

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_AES_C)

#include <string.h>
#include <stdlib.h>
#include "mbedtls/aes.h"
#include "hal_sec_eng.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "sec_alt.h"

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#define mbedtls_calloc  calloc
#define mbedtls_free    free
#endif

#undef LOG_TAG
#define LOG_TAG "[aes_alt] "

//#define AES_ALT_DEBUG
#ifdef AES_ALT_DEBUG
#define AES_ALT_DBG(str,...) TRACE(0, LOG_TAG"%s:%d "str, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define AES_ALT_DBG(str,...)
#endif
#define AES_ALT_ERR(str,...) TRACE(0, LOG_TAG"%s:%d "str, __FUNCTION__, __LINE__, ##__VA_ARGS__)


#if defined(MBEDTLS_AES_ALT)

#define SEC_DATAMAX_LEN     256
#define AES_CTX_BUF_LEN     64

struct sec_aes_ctx {
    struct HAL_SE_AES_CFG_T cfg;
    uint8_t seg_ctx_buf[AES_CTX_BUF_LEN];
    int mode;
    const uint8_t *in;
    uint8_t *out;
    size_t length;
    int32_t iv_len;
    int32_t iv_off;
    uint8_t iv[16];
    uint8_t key[32];
    int32_t key_bitlen;
};

static int aes_alt_crypt(struct sec_aes_ctx *hw_ctx)
{
    struct HAL_SE_AES_CFG_T *cfg = NULL;
    enum HAL_SE_RET_T ret = HAL_SE_ERR;
    unsigned char iv_update[16];
    uint32_t in_len = 0;
    uint8_t *tmp_in = NULL;
    uint8_t *tmp_out = NULL;

    if (!hw_ctx)
        return -1;

    /* check in/out buffer address alignment */
    if ((uint32_t)(hw_ctx->in) & 0x3) {
        tmp_in = mbedtls_calloc(1, SEC_DATAMAX_LEN+4);
        if (!tmp_in)
            goto exit;
    }
    if ((uint32_t)(hw_ctx->out) & 0x3) {
        tmp_out = mbedtls_calloc(1, SEC_DATAMAX_LEN+4);
        if (!tmp_out)
            goto exit;
    }

    cfg = &(hw_ctx->cfg);
    cfg->done_hdlr = NULL;
    cfg->key = hw_ctx->key;
    cfg->key_len = hw_ctx->key_bitlen/8;
    cfg->iv = hw_ctx->iv;
    cfg->seg_ctx_buf = hw_ctx->seg_ctx_buf;
    cfg->seg_ctx_len = sizeof(hw_ctx->seg_ctx_buf);

    while (in_len < hw_ctx->length) {
        uint8_t *in_buf = NULL;
        uint8_t *out_buf = NULL;

        if ((hw_ctx->length - in_len) >= SEC_DATAMAX_LEN) {
            cfg->seg_oper = (in_len == 0) ? HAL_SE_SEG_INIT : HAL_SE_SEG_CONT;
            cfg->in_len = SEC_DATAMAX_LEN;
        } else {
            cfg->seg_oper = (in_len == 0) ? HAL_SE_SEG_NONE : HAL_SE_SEG_TERM;
            cfg->in_len = hw_ctx->length - in_len;
        }
        if (tmp_in) {
            in_buf = (uint8_t *)((uint32_t)tmp_in & (~3));
            memcpy(in_buf, &hw_ctx->in[in_len], cfg->in_len);
            cfg->in = in_buf;
        } else {
            cfg->in = &hw_ctx->in[in_len];
        }

        if (tmp_out) {
            out_buf = (uint8_t *)((uint32_t)tmp_out & (~3));
            cfg->out = out_buf;
        } else {
            cfg->out = &hw_ctx->out[in_len];
        }
        cfg->out_len = cfg->in_len;

        if (hw_ctx->mode == MBEDTLS_AES_ENCRYPT) {
            ret = sec_alt_aes_encrypt(cfg);
            if (ret != HAL_SE_OK) {
                AES_ALT_ERR("sec_alt_aes_encrypt failed: %d\n", ret);
                break;
            }
            if (cfg->mode == HAL_SE_AES_CBC)
                memcpy((uint8_t *)hw_ctx->iv, (uint8_t *)cfg->out + cfg->out_len - 16, 16);

        } else {
            if (cfg->mode == HAL_SE_AES_CBC)
                memcpy(iv_update, (uint8_t *)cfg->in + cfg->in_len - 16, 16);

            ret = sec_alt_aes_decrypt(cfg);
            if (ret != HAL_SE_OK) {
                AES_ALT_ERR("sec_alt_aes_decrypt failed: %d\n", ret);
                break;
            }

            if (cfg->mode == HAL_SE_AES_CBC)
                memcpy((uint8_t *)hw_ctx->iv, (uint8_t *)iv_update, 16);
        }

        if (out_buf)
            memcpy(&hw_ctx->out[in_len], out_buf, cfg->out_len);

        in_len += cfg->in_len;
    }

exit:
    if (tmp_in)
        mbedtls_free(tmp_in);
    if (tmp_out)
        mbedtls_free(tmp_out);
    return (ret == HAL_SE_OK) ? 0 : -1;
}

void mbedtls_aes_init(mbedtls_aes_context *ctx)
{
    struct sec_aes_ctx *hw_ctx = NULL;

    if (!ctx) {
        AES_ALT_ERR("FAILED, context pointer is null.");
        return;
    }

    sec_alt_open();

    hw_ctx = (struct sec_aes_ctx *)ctx;
    memset(hw_ctx, 0 ,sizeof(struct sec_aes_ctx));
    AES_ALT_DBG("OKAY, context pointer [%p].", ctx);
}

void mbedtls_aes_free(mbedtls_aes_context *ctx)
{
    struct sec_aes_ctx *hw_ctx = (struct sec_aes_ctx *)(ctx);

    if (hw_ctx) {
        memset(hw_ctx, 0 ,sizeof(struct sec_aes_ctx));
        AES_ALT_DBG("OKAY, context pointer [%p].", hw_ctx);
    } else {
        AES_ALT_ERR("FAILED, context pointer is null.");
    }
}

static void aes_alt_set_key(struct sec_aes_ctx *hw_ctx, const uint8_t *key, uint32_t keybits)
{
    if (!hw_ctx)
        return;

    memcpy(hw_ctx->key, key, keybits >> 3);
    hw_ctx->key_bitlen = keybits;
}

/*
 * AES key schedule (encryption)
 */
#if !defined(MBEDTLS_AES_SETKEY_ENC_ALT)
int mbedtls_aes_setkey_enc(mbedtls_aes_context *ctx, const unsigned char *key,
                           unsigned int keybits)
{
    struct sec_aes_ctx *hw_ctx = (struct sec_aes_ctx *)(ctx);

    if (!hw_ctx) {
        AES_ALT_ERR("FAILED, context pointer is null.");
        return -1;
    }

    aes_alt_set_key(hw_ctx, key, keybits);
    AES_ALT_DBG("OKAY, context [%p], key %p, keybits %d", hw_ctx, key, keybits);
    return 0;
}
#endif /* !MBEDTLS_AES_SETKEY_ENC_ALT */

/*
 * AES key schedule (decryption)
 */
#if !defined(MBEDTLS_AES_SETKEY_DEC_ALT)
int mbedtls_aes_setkey_dec(mbedtls_aes_context *ctx, const unsigned char *key,
                           unsigned int keybits)
{
    struct sec_aes_ctx *hw_ctx = (struct sec_aes_ctx *)(ctx);

    if (!hw_ctx) {
        AES_ALT_ERR("FAILED, context pointer is null.");
        return -1;
    }

    aes_alt_set_key(hw_ctx, key, keybits);
    AES_ALT_DBG("OKAY, context [%p], key %p, keybits %d", hw_ctx, key, keybits);
    return 0;
}
#endif /* !MBEDTLS_AES_SETKEY_DEC_ALT */

/*
 * AES-ECB block encryption
 */
#if !defined(MBEDTLS_AES_ENCRYPT_ALT)
int mbedtls_internal_aes_encrypt(mbedtls_aes_context *ctx,
                                 const unsigned char input[16],
                                 unsigned char output[16])
{
    return mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, input, output);
}
#endif /* !MBEDTLS_AES_ENCRYPT_ALT */

/*
 * AES-ECB block decryption
 */
#if !defined(MBEDTLS_AES_DECRYPT_ALT)
int mbedtls_internal_aes_decrypt(mbedtls_aes_context *ctx,
                                 const unsigned char input[16],
                                 unsigned char output[16])
{
    return mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_DECRYPT, input, output);
}
#endif /* !MBEDTLS_AES_DECRYPT_ALT */

/*
 * AES-ECB block encryption/decryption
 */
int mbedtls_aes_crypt_ecb(mbedtls_aes_context *ctx,
                          int mode,
                          const unsigned char input[16],
                          unsigned char output[16])
{
    int ret = 0;
    struct sec_aes_ctx *hw_ctx = NULL;

    if (!ctx) {
        AES_ALT_ERR("FAILED, context pointer is null.");
        return -1;
    }

    hw_ctx = (struct sec_aes_ctx *)(ctx);
    memset(&hw_ctx->cfg, 0, sizeof(struct HAL_SE_AES_CFG_T));
    hw_ctx->cfg.mode = HAL_SE_AES_ECB;
    hw_ctx->mode = mode;
    hw_ctx->in = input;
    hw_ctx->out = output;
    hw_ctx->length = 16;

    AES_ALT_DBG("context:[%p] mode:%d in:%p out:%p",
            ctx, mode, input, output);
    ret = aes_alt_crypt(hw_ctx);
    if (ret != 0)
        AES_ALT_ERR("FAILED, ret = %d", ret);

    return ret;
}

#if defined(MBEDTLS_CIPHER_MODE_CBC)
/*
 * AES-CBC buffer encryption/decryption
 */
int mbedtls_aes_crypt_cbc(mbedtls_aes_context *ctx,
                          int mode,
                          size_t length,
                          unsigned char iv[16],
                          const unsigned char *input,
                          unsigned char *output)
{
    int ret = 0;
    struct sec_aes_ctx *hw_ctx = NULL;

    if (!ctx) {
        AES_ALT_ERR("FAILED, context pointer is null.");
        return -1;
    }

    hw_ctx = (struct sec_aes_ctx *)(ctx);
    memset(&hw_ctx->cfg, 0, sizeof(struct HAL_SE_AES_CFG_T));
    hw_ctx->iv_len = 16;
    memcpy(hw_ctx->iv, iv, hw_ctx->iv_len);
    hw_ctx->cfg.mode = HAL_SE_AES_CBC;
    hw_ctx->mode = mode;
    hw_ctx->in = input;
    hw_ctx->out = output;
    hw_ctx->length = length;

    AES_ALT_DBG("context:[%p] mode:%d len:%d iv:%p in:%p out:%p",
            ctx, mode, length, iv, input, output);
    ret = aes_alt_crypt(hw_ctx);
    if (ret != 0)
        AES_ALT_ERR("FAILED, ret = %d", ret);

    memcpy(iv, hw_ctx->iv, hw_ctx->iv_len);
    return ret;
}
#endif /* MBEDTLS_CIPHER_MODE_CBC */

#if defined(MBEDTLS_CIPHER_MODE_CFB)
/*
 * AES-CFB128 buffer encryption/decryption
 */
int mbedtls_aes_crypt_cfb128(mbedtls_aes_context *ctx,
                       int mode,
                       size_t length,
                       size_t *iv_off,
                       unsigned char iv[16],
                       const unsigned char *input,
                       unsigned char *output)
{
    int ret = 0;
    int c;
    size_t n = *iv_off;

    if (mode == MBEDTLS_AES_DECRYPT) {
        while (length--) {
            if (n == 0) {
                ret = mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, iv, iv);
                if (ret != 0)
                    goto exit;
            }

            c = *input++;
            *output++ = (unsigned char)(c ^ iv[n]);
            iv[n] = (unsigned char) c;

            n = (n + 1) & 0x0F;
        }

    } else {
        while (length--) {
            if (n == 0) {
                ret = mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, iv, iv);
                if (ret != 0)
                    goto exit;
            }

            iv[n] = *output++ = (unsigned char)(iv[n] ^ *input++);

            n = (n + 1) & 0x0F;
        }
    }

    *iv_off = n;

exit:
    return ret;
}

/*
 * AES-CFB8 buffer encryption/decryption
 */
int mbedtls_aes_crypt_cfb8(mbedtls_aes_context *ctx,
                           int mode,
                           size_t length,
                           unsigned char iv[16],
                           const unsigned char *input,
                           unsigned char *output)
{
    int ret = 0;
    unsigned char c;
    unsigned char ov[17];

    while (length--) {
        memcpy(ov, iv, 16);
        ret = mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, iv, iv);
        if (ret != 0)
            goto exit;

        if (mode == MBEDTLS_AES_DECRYPT)
            ov[16] = *input;

        c = *output++ = (unsigned char)(iv[0] ^ *input++);

        if (mode == MBEDTLS_AES_ENCRYPT)
            ov[16] = c;

        memcpy(iv, ov + 1, 16);
    }

exit:
    return ret;
}
#endif /*MBEDTLS_CIPHER_MODE_CFB */

#if defined(MBEDTLS_CIPHER_MODE_OFB)
/*
 * AES-OFB (Output Feedback Mode) buffer encryption/decryption
 */
int mbedtls_aes_crypt_ofb(mbedtls_aes_context *ctx,
                           size_t length,
                           size_t *iv_off,
                           unsigned char iv[16],
                           const unsigned char *input,
                           unsigned char *output)
{
    int ret = 0;
    size_t n;

    if ((ctx == NULL) || (iv_off == NULL) ||
        (iv == NULL) || (input == NULL) || (output == NULL))
        return -1;

    n = *iv_off;

    if (n > 15)
        return -1;

    while (length--) {
        if (n == 0) {
            ret = mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, iv, iv);
            if (ret != 0)
                goto exit;
        }
        *output++ =  *input++ ^ iv[n];

        n = (n + 1) & 0x0F;
    }

    *iv_off = n;

exit:
    return ret;
}
#endif /* MBEDTLS_CIPHER_MODE_OFB */

#if defined(MBEDTLS_CIPHER_MODE_CTR)
/*
 * AES-CTR buffer encryption/decryption
 */
int mbedtls_aes_crypt_ctr( mbedtls_aes_context *ctx,
                       size_t length,
                       size_t *nc_off,
                       unsigned char nonce_counter[16],
                       unsigned char stream_block[16],
                       const unsigned char *input,
                       unsigned char *output )
{
    int c, i;
    int ret = -1;
    size_t n;

    if (!ctx || !nc_off || !input || !output)
        return -1;

    n = *nc_off;

    if (n > 0x0F)
        return (MBEDTLS_ERR_AES_BAD_INPUT_DATA);

    while (length--) {
        if (n == 0) {
            ret = mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, nonce_counter, stream_block);
            if (ret != 0)
                goto exit;

            for (i = 16; i > 0; i--)
                if (++nonce_counter[i - 1] != 0)
                    break;
        }
        c = *input++;
        *output++ = (unsigned char)(c ^ stream_block[n]);

        n = (n + 1) & 0x0F;
    }

    *nc_off = n;
    ret = 0;

exit:
    return ret;
}
#endif /* MBEDTLS_CIPHER_MODE_CTR */

#if defined(MBEDTLS_CIPHER_MODE_XTS)
void mbedtls_aes_xts_init(mbedtls_aes_xts_context *ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_aes_init(&ctx->crypt);
    mbedtls_aes_init(&ctx->tweak);
}

void mbedtls_aes_xts_free(mbedtls_aes_xts_context *ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_aes_free(&ctx->crypt);
    mbedtls_aes_free(&ctx->tweak);
}

static int mbedtls_aes_xts_decode_keys(const unsigned char *key,
                                        unsigned int keybits,
                                        const unsigned char **key1,
                                        unsigned int *key1bits,
                                        const unsigned char **key2,
                                        unsigned int *key2bits)
{
    const unsigned int half_keybits = keybits / 2;
    const unsigned int half_keybytes = half_keybits / 8;

    switch (keybits) {
        case 256:
            break;
        case 512:
            break;
        default:
            return MBEDTLS_ERR_AES_INVALID_KEY_LENGTH;
    }

    *key1bits = half_keybits;
    *key2bits = half_keybits;
    *key1 = &key[0];
    *key2 = &key[half_keybytes];

    return 0;
}

int mbedtls_aes_xts_setkey_enc(mbedtls_aes_xts_context *ctx,
                                const unsigned char *key,
                                unsigned int keybits)
{
    int ret;
    const unsigned char *key1, *key2;
    unsigned int key1bits, key2bits;

    ret = mbedtls_aes_xts_decode_keys(key, keybits, &key1, &key1bits,
                                       &key2, &key2bits);
    if (ret != 0)
        return ret;

    /* Set the tweak key. Always set tweak key for the encryption mode. */
    ret = mbedtls_aes_setkey_enc(&ctx->tweak, key2, key2bits);
    if (ret != 0)
        return ret;

    /* Set crypt key for encryption. */
    return mbedtls_aes_setkey_enc(&ctx->crypt, key1, key1bits);
}

int mbedtls_aes_xts_setkey_dec(mbedtls_aes_xts_context *ctx,
                                const unsigned char *key,
                                unsigned int keybits)
{
    int ret;
    const unsigned char *key1, *key2;
    unsigned int key1bits, key2bits;

    ret = mbedtls_aes_xts_decode_keys(key, keybits, &key1, &key1bits,
                                       &key2, &key2bits);
    if (ret != 0)
        return ret;

    /* Set the tweak key. Always set tweak key for encryption. */
    ret = mbedtls_aes_setkey_enc(&ctx->tweak, key2, key2bits);
    if (ret != 0)
        return ret;

    /* Set crypt key for decryption. */
    return mbedtls_aes_setkey_dec(&ctx->crypt, key1, key1bits);
}

/* Endianess with 64 bits values */
#ifndef GET_UINT64_LE
#define GET_UINT64_LE(n,b,i)                            \
{                                                       \
    (n) = ((uint64_t) (b)[(i) + 7] << 56)             \
        | ((uint64_t) (b)[(i) + 6] << 48)             \
        | ((uint64_t) (b)[(i) + 5] << 40)             \
        | ((uint64_t) (b)[(i) + 4] << 32)             \
        | ((uint64_t) (b)[(i) + 3] << 24)             \
        | ((uint64_t) (b)[(i) + 2] << 16)             \
        | ((uint64_t) (b)[(i) + 1] <<  8)             \
        | ((uint64_t) (b)[(i)    ]      );            \
}
#endif

#ifndef PUT_UINT64_LE
#define PUT_UINT64_LE(n,b,i)                            \
{                                                       \
    (b)[(i) + 7] = (unsigned char) ((n) >> 56);       \
    (b)[(i) + 6] = (unsigned char) ((n) >> 48);       \
    (b)[(i) + 5] = (unsigned char) ((n) >> 40);       \
    (b)[(i) + 4] = (unsigned char) ((n) >> 32);       \
    (b)[(i) + 3] = (unsigned char) ((n) >> 24);       \
    (b)[(i) + 2] = (unsigned char) ((n) >> 16);       \
    (b)[(i) + 1] = (unsigned char) ((n) >>  8);       \
    (b)[(i)    ] = (unsigned char) ((n)      );       \
}
#endif

typedef unsigned char mbedtls_be128[16];

/*
 * GF(2^128) multiplication function
 *
 * This function multiplies a field element by x in the polynomial field
 * representation. It uses 64-bit word operations to gain speed but compensates
 * for machine endianess and hence works correctly on both big and little
 * endian machines.
 */
static void mbedtls_gf128mul_x_ble(unsigned char r[16],
                                    const unsigned char x[16])
{
    uint64_t a, b, ra, rb;

    GET_UINT64_LE(a, x, 0);
    GET_UINT64_LE(b, x, 8);

    ra = (a << 1)  ^ 0x0087 >> (8 - ((b >> 63) << 3));
    rb = (a >> 63) | (b << 1);

    PUT_UINT64_LE(ra, r, 0);
    PUT_UINT64_LE(rb, r, 8);
}

/*
 * AES-XTS buffer encryption/decryption
 */
int mbedtls_aes_crypt_xts(mbedtls_aes_xts_context *ctx,
                           int mode,
                           size_t length,
                           const unsigned char data_unit[16],
                           const unsigned char *input,
                           unsigned char *output)
{
    int ret;
    size_t blocks = length / 16;
    size_t leftover = length % 16;
    unsigned char tweak[16];
    unsigned char prev_tweak[16];
    unsigned char tmp[16];

    /* Data units must be at least 16 bytes long. */
    if (length < 16)
        return MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH;

    /* NIST SP 800-38E disallows data units larger than 2**20 blocks. */
    if (length > (1 << 20) * 16)
        return MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH;

    /* Compute the tweak. */
    ret = mbedtls_aes_crypt_ecb(&ctx->tweak, MBEDTLS_AES_ENCRYPT,
                                 data_unit, tweak);
    if(ret != 0)
        goto exit;

    while(blocks--) {
        size_t i;

        if(leftover && (mode == MBEDTLS_AES_DECRYPT) && blocks == 0) {
            /* We are on the last block in a decrypt operation that has
             * leftover bytes, so we need to use the next tweak for this block,
             * and this tweak for the lefover bytes. Save the current tweak for
             * the leftovers and then update the current tweak for use on this,
             * the last full block. */
            memcpy(prev_tweak, tweak, sizeof(tweak));
            mbedtls_gf128mul_x_ble(tweak, tweak);
        }

        for (i = 0; i < 16; i++)
            tmp[i] = input[i] ^ tweak[i];

        ret = mbedtls_aes_crypt_ecb(&ctx->crypt, mode, tmp, tmp);
        if (ret != 0)
            goto exit;

        for (i = 0; i < 16; i++)
            output[i] = tmp[i] ^ tweak[i];

        /* Update the tweak for the next block. */
        mbedtls_gf128mul_x_ble(tweak, tweak);

        output += 16;
        input += 16;
    }

    if(leftover) {
        /* If we are on the leftover bytes in a decrypt operation, we need to
         * use the previous tweak for these bytes (as saved in prev_tweak). */
        unsigned char *t = mode == MBEDTLS_AES_DECRYPT ? prev_tweak : tweak;

        /* We are now on the final part of the data unit, which doesn't divide
         * evenly by 16. It's time for ciphertext stealing. */
        size_t i;
        unsigned char *prev_output = output - 16;

        /* Copy ciphertext bytes from the previous block to our output for each
         * byte of cyphertext we won't steal. At the same time, copy the
         * remainder of the input for this final round (since the loop bounds
         * are the same). */
        for(i = 0; i < leftover; i++) {
            output[i] = prev_output[i];
            tmp[i] = input[i] ^ t[i];
        }

        /* Copy ciphertext bytes from the previous block for input in this
         * round. */
        for (; i < 16; i++)
            tmp[i] = prev_output[i] ^ t[i];

        ret = mbedtls_aes_crypt_ecb(&ctx->crypt, mode, tmp, tmp);
        if(ret != 0)
            goto exit;

        /* Write the result back to the previous block, overriding the previous
         * output we copied. */
        for(i = 0; i < 16; i++)
            prev_output[i] = tmp[i] ^ t[i];
    }

    ret = 0;

exit:
    return ret;
}

#endif /* MBEDTLS_CIPHER_MODE_XTS */

#endif /* MBEDTLS_SELF_TEST */
#endif /* MBEDTLS_AES_C */
