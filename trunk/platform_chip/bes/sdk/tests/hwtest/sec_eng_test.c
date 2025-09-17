/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#include "plat_addr_map.h"

#ifdef SEC_ENG_BASE

#include "hwtest.h"
#include "hal_location.h"
#include "hal_sec_eng.h"
#include "hal_gpio.h"
#include "hexdump.h"
#include "string.h"
#include "stdlib.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif

#define TIME_MEASURE_ENABLE
//#define TIME_MEASURE_GPIO
//#define SEC_MASS_DATA_TEST_MODE

#ifdef SEC_MASS_DATA_TEST_MODE
    //#define AES_128_MODE
    //#define AES_192_MODE
    //#define AES_256_MODE  //default

    //#define HASH_MD5_MODE
    //#define HASH_SHA1_MODE
    //#define HASH_SHA256_MODE
    //#define HASH_SHA512_MODE  //default

    //#define HMAC_SHA256_MODE
    //#define HMAC_SHA384_MODE
    //#define HMAC_SHA512_MODE  //default

    #define CALC_RATE_TIME_BYTE                 64
    #define MASS_DATA_BUF_SIZE                  (384 * 1024)

#if (defined(AES_128_MODE) && defined(AES_192_MODE) && defined(AES_256_MODE)) || \
    (defined(AES_128_MODE) && defined(AES_192_MODE)) || \
    (defined(AES_128_MODE) && defined(AES_256_MODE)) || \
    (defined(AES_192_MODE) && defined(AES_256_MODE))
#error "AES_128_MODE/AES_192_MODE/AES_256_MODE, the 3 macros can only open one of them"
#endif

#if (defined(HASH_MD5_MODE) && defined(HASH_SHA1_MODE) && defined(HASH_SHA256_MODE) && defined(HASH_SHA512_MODE)) || \
    (defined(HASH_MD5_MODE) && defined(HASH_SHA1_MODE)) || \
    (defined(HASH_MD5_MODE) && defined(HASH_SHA256_MODE)) || \
    (defined(HASH_MD5_MODE) && defined(HASH_SHA512_MODE)) || \
    (defined(HASH_SHA1_MODE) && defined(HASH_SHA256_MODE)) || \
    (defined(HASH_SHA1_MODE) && defined(HASH_SHA512_MODE)) || \
    (defined(HASH_SHA256_MODE) && defined(HASH_SHA512_MODE))
#error "HASH_MD5_MODE/HASH_SHA1_MODE/HASH_SHA256_MODE/HASH_SHA512_MODE, the 4 macros can only open one of them"
#endif

#if (defined(HMAC_SHA256_MODE) && defined(HMAC_SHA384_MODE) && defined(HMAC_SHA512_MODE)) || \
    (defined(HMAC_SHA256_MODE) && defined(HMAC_SHA384_MODE)) || \
    (defined(HMAC_SHA256_MODE) && defined(HMAC_SHA512_MODE)) || \
    (defined(HMAC_SHA384_MODE) && defined(HMAC_SHA512_MODE))
#error "HMAC_SHA256_MODE/HMAC_SHA384_MODE/HMAC_SHA512_MODE, the 3 macros can only open one of them"
#endif
#endif

#ifdef DEFAULT_BSS_IN_PSRAM
    #define SEC_ENG_BUF_LOC                     SYNC_FLAGS_PSRAM_LOC
#else
    #define SEC_ENG_BUF_LOC                     SYNC_FLAGS_LOC
#endif

#ifdef TIME_MEASURE_ENABLE
    volatile uint8_t flag_complete = 0;
#endif

//tool:http://www.ip33.com/crypto/aes.html
//-----------------aes ecb mode
static const uint8_t aes128_ecb_key[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};

static const uint8_t aes192_ecb_key[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
};

static const uint8_t aes256_ecb_key[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
};

#ifdef SEC_MASS_DATA_TEST_MODE
SEC_ENG_BUF_LOC static uint8_t aes_ecb_plain_data[MASS_DATA_BUF_SIZE];
SEC_ENG_BUF_LOC static uint8_t aes128_ecb_cipher_data[MASS_DATA_BUF_SIZE];
SEC_ENG_BUF_LOC static uint8_t aes192_ecb_cipher_data[MASS_DATA_BUF_SIZE];
SEC_ENG_BUF_LOC static uint8_t aes256_ecb_cipher_data[MASS_DATA_BUF_SIZE];
#else
static const uint8_t aes_ecb_plain_data[] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
};

static const uint8_t aes128_ecb_cipher_data[] = {
    0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30, 0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a,
};

static const uint8_t aes192_ecb_cipher_data[] = {
    0xdd, 0xa9, 0x7c, 0xa4, 0x86, 0x4c, 0xdf, 0xe0, 0x6e, 0xaf, 0x70, 0xa0, 0xec, 0x0d, 0x71, 0x91,
};

static const uint8_t aes256_ecb_cipher_data[] = {
    0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf, 0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89,
};
#endif

//-----------------aes cbc mode
#ifdef SEC_MASS_DATA_TEST_MODE
SEC_ENG_BUF_LOC static uint8_t aes_cbc_plain_data[MASS_DATA_BUF_SIZE];
#else
static const uint8_t aes_cbc_plain_data[] = {
    0x3c, 0x88, 0x8b, 0xbb, 0xb1, 0xa8, 0xeb, 0x9f, 0x3e, 0x9b, 0x87, 0xac, 0xaa, 0xd9, 0x86, 0xc4,
    0x66, 0xe2, 0xf7, 0x07, 0x1c, 0x83, 0x08, 0x3b, 0x8a, 0x55, 0x79, 0x71, 0x91, 0x88, 0x50, 0xe5,
};
#endif

static const uint8_t aes128_cbc_key[] = {
    0x42, 0x78, 0xb8, 0x40, 0xfb, 0x44, 0xaa, 0xa7, 0x57, 0xc1, 0xbf, 0x04, 0xac, 0xbe, 0x1a, 0x3e,
};

static const uint8_t aes128_iv[] = {
    0x57, 0xf0, 0x2a, 0x5c, 0x53, 0x39, 0xda, 0xeb, 0x0a, 0x29, 0x08, 0xa0, 0x6a, 0xc6, 0x39, 0x3f,
};

#ifdef SEC_MASS_DATA_TEST_MODE
SEC_ENG_BUF_LOC static uint8_t aes128_cbc_cipher_data[MASS_DATA_BUF_SIZE];
#else
static const uint8_t aes128_cbc_cipher_data[] = {
    0x47, 0x9c, 0x89, 0xec, 0x14, 0xbc, 0x98, 0x99, 0x4e, 0x62, 0xb2, 0xc7, 0x05, 0xb5, 0x01, 0x4e,
    0x17, 0x5b, 0xd7, 0x83, 0x2e, 0x7e, 0x60, 0xa1, 0xe9, 0x2a, 0xac, 0x56, 0x8a, 0x86, 0x1e, 0xb7,
};
#endif

static const uint8_t aes192_cbc_key[] = {
    0x42, 0x78, 0xb8, 0x40, 0xfb, 0x44, 0xaa, 0xa7, 0x57, 0xc1, 0xbf, 0x04, 0xac, 0xbe, 0x1a, 0x3e,
    0x11, 0x23, 0xfe, 0x84, 0xde, 0x45, 0xcd, 0x68,
};

static const uint8_t aes192_iv[] = {
    0x57, 0xf0, 0x2a, 0x5c, 0x53, 0x39, 0xda, 0xeb, 0x0a, 0x29, 0x08, 0xa0, 0x6a, 0xc6, 0x39, 0x3f,
};

#ifdef SEC_MASS_DATA_TEST_MODE
SEC_ENG_BUF_LOC static uint8_t aes192_cbc_cipher_data[MASS_DATA_BUF_SIZE];
#else
static const uint8_t aes192_cbc_cipher_data[] = {
    0xab, 0x27, 0x2b, 0x5b, 0x4f, 0x81, 0x22, 0xf8, 0xdf, 0xf1, 0x7c, 0xb7, 0x8d, 0x5d, 0xe6, 0x81,
    0xb7, 0xed, 0x50, 0x49, 0x60, 0x40, 0x20, 0xa5, 0x93, 0x62, 0xf9, 0x20, 0xb1, 0x89, 0x3d, 0x23,
};
#endif

static const uint8_t aes256_cbc_key[] = {
    0x42, 0x78, 0xb8, 0x40, 0xfb, 0x44, 0xaa, 0xa7, 0x57, 0xc1, 0xbf, 0x04, 0xac, 0xbe, 0x1a, 0x3e,
    0xe2, 0xee, 0xcc, 0x28, 0xeb, 0x27, 0x48, 0x08, 0x5f, 0xd6, 0x2d, 0x84, 0x66, 0x33, 0x28, 0xab,
};

static const uint8_t aes256_iv[] = {
    0x57, 0xf0, 0x2a, 0x5c, 0x53, 0x39, 0xda, 0xeb, 0x0a, 0x29, 0x08, 0xa0, 0x6a, 0xc6, 0x39, 0x3f,
};

#ifdef SEC_MASS_DATA_TEST_MODE
SEC_ENG_BUF_LOC static uint8_t aes256_cbc_cipher_data[MASS_DATA_BUF_SIZE];
#else
static const uint8_t aes256_cbc_cipher_data[] = {
    0xab, 0x07, 0x46, 0x8f, 0x3b, 0xb6, 0x8a, 0x67, 0x57, 0x2c, 0xa7, 0x96, 0x6e, 0xaf, 0x60, 0x2c,
    0x77, 0x68, 0xf1, 0x00, 0x8f, 0xef, 0x07, 0xac, 0xf9, 0xcd, 0xe3, 0x3a, 0x2c, 0xbf, 0x6e, 0x74,
};
#endif

#ifdef AES_LARGE_DATA
extern uint8_t __aes_plain_data_start[];
extern uint8_t __aes_plain_data_end[];
extern uint8_t __aes_cipher_data_start[];
extern uint8_t __aes_cipher_data_end[];

static const uint8_t aes_key_large[] = {
    0xE1, 0x0A, 0xDC, 0x39, 0x49, 0xBA, 0x59, 0xAB, 0xBE, 0x56, 0xE0, 0x57, 0xF2, 0x0F, 0x88, 0x3E,
};

static const uint8_t aes_iv_large[] = {
    0x65, 0xB4, 0xAD, 0x27, 0x0B, 0x3B, 0x98, 0x09, 0x8D, 0x25, 0x6A, 0xB3, 0x2F, 0x5B, 0x8F, 0xBA,
};

#define OUT_BUF_SIZE                        0x10000
#else
#define OUT_BUF_SIZE                        0x1000
#endif

#ifdef SEC_MASS_DATA_TEST_MODE
#undef  OUT_BUF_SIZE
#define OUT_BUF_SIZE                        MASS_DATA_BUF_SIZE
#endif

SEC_ENG_BUF_LOC
static uint8_t out_buf[OUT_BUF_SIZE];
static uint8_t aes_ctx_buf[64];

static void sec_eng_done(void *buf, uint32_t len, enum HAL_SE_DONE_ERR_T err)
{
#ifdef TIME_MEASURE_ENABLE
    flag_complete = 1;
#else
    TR_INFO(0, "%s: buf=0x%08X len=%u err=%d", __func__, (uint32_t)buf, len, err);

    if (buf && len && len <= 64) {
        TR_INFO(0, "%s: Start of dump", __func__);
        dump_buffer(buf, len);
        TR_INFO(0, "%s: End of dump", __func__);
    }
#endif
}

#ifdef SEC_MASS_DATA_TEST_MODE
static void rand_generator(uint8_t *dest, uint32_t size)
{
    while (size--) {
        *dest++ = (uint8_t)rand();
    }
}
#endif

POSSIBLY_UNUSED static int aes_ecb_test(void)
{
    struct HAL_SE_AES_CFG_T cfg;
    enum HAL_SE_RET_T ret;
#ifdef TIME_MEASURE_ENABLE
    uint32_t time_start, time;
#endif

#ifdef SEC_MASS_DATA_TEST_MODE
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());
    rand_generator(aes_ecb_plain_data, MASS_DATA_BUF_SIZE);
#endif

    struct ECB_TEST_CFG {
        const uint8_t *key_data;
        uint32_t key_len;
        const uint8_t *cipher_data;
        uint32_t cipher_len;
    };
    struct ECB_TEST_CFG test_cfg[] = {
        {
            .key_data = aes128_ecb_key,
            .key_len = sizeof(aes128_ecb_key),
            .cipher_data = aes128_ecb_cipher_data,
            .cipher_len = sizeof(aes128_ecb_cipher_data),
        },
        {
            .key_data = aes192_ecb_key,
            .key_len = sizeof(aes192_ecb_key),
            .cipher_data = aes192_ecb_cipher_data,
            .cipher_len = sizeof(aes192_ecb_cipher_data),
        },
        {
            .key_data = aes256_ecb_key,
            .key_len = sizeof(aes256_ecb_key),
            .cipher_data = aes256_ecb_cipher_data,
            .cipher_len = sizeof(aes256_ecb_cipher_data),
        },
    };

    TR_INFO(0, " ");
    TR_INFO(0, "%s: Start", __func__);
    TR_INFO(0, " ");

#ifdef SEC_MASS_DATA_TEST_MODE
    while (1)
#if defined(AES_128_MODE)
    for (int i = 0; i < 1; i++) {
#elif defined(AES_192_MODE)
    for (int i = 1; i < 2; i++) {
#else
    for (int i = 2; i < 3; i++) {
#endif
#else   //SEC_MASS_DATA_TEST_MODE
    for (int i = 0; i < ARRAY_SIZE(test_cfg); i++) {
#endif
        memset(&cfg, 0, sizeof(cfg));
        cfg.done_hdlr = sec_eng_done;
        cfg.mode = HAL_SE_AES_ECB;
        cfg.key = (uint8_t *)test_cfg[i].key_data;
        cfg.key_len = test_cfg[i].key_len;
        cfg.in = (uint8_t *)&aes_ecb_plain_data[0];
        cfg.in_len = sizeof(aes_ecb_plain_data);
        cfg.out = (uint8_t *)&out_buf[0];
        cfg.out_len = cfg.in_len;

#ifdef TIME_MEASURE_ENABLE
        flag_complete = 0;
        time_start = hal_fast_sys_timer_get();
#endif
        ret = hal_se_aes_encrypt(&cfg);
        ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_encrypt failed: %d", __func__, ret);
#ifdef TIME_MEASURE_ENABLE
        while (!flag_complete);
        time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
#endif
        while (hal_se_aes_busy()) {
            osDelay(1);
        }
#ifndef SEC_MASS_DATA_TEST_MODE
        TR_INFO(0, "Cipher Text:");
        dump_buffer(&out_buf[0], cfg.out_len);
        ret = memcmp(&out_buf[0], test_cfg[i].cipher_data, test_cfg[i].cipher_len);
#else
        memcpy((void *)test_cfg[i].cipher_data, &out_buf[0], test_cfg[i].cipher_len);
        ret = 0;
#endif
        if (ret) {
            TR_INFO(0, "%s: *** AES%u ECB Encrypt Test Failed ***", __func__, test_cfg[i].key_len * 8);
            return 1;
        } else {
#ifdef TIME_MEASURE_ENABLE
#ifdef SEC_MASS_DATA_TEST_MODE
            TR_INFO(0, "===time cost:%uus/%ubytes, %uus/%ubytes", time, cfg.in_len, time / (cfg.in_len / CALC_RATE_TIME_BYTE), CALC_RATE_TIME_BYTE);
#else
            TR_INFO(0, "===time cost:%uus/%ubytes", time, cfg.in_len);
#endif
#endif
            TR_INFO(0, "%s: === AES%u ECB Encrypt Test Passed, plain data size:%ubytes ===", __func__, test_cfg[i].key_len * 8, cfg.in_len);
        }
        TR_INFO(0, " ");

        cfg.in = (uint8_t *)test_cfg[i].cipher_data;
        cfg.in_len = test_cfg[i].cipher_len;
        cfg.out_len = cfg.in_len;

#ifdef TIME_MEASURE_ENABLE
        flag_complete = 0;
        time_start = hal_fast_sys_timer_get();
#endif
        ret = hal_se_aes_decrypt(&cfg);
        ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_decrypt failed: %d", __func__, ret);
#ifdef TIME_MEASURE_ENABLE
        while (!flag_complete);
        time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
#endif
        while (hal_se_aes_busy()) {
            osDelay(1);
        }
#ifndef SEC_MASS_DATA_TEST_MODE
        TR_INFO(0, "Plain Text:");
        dump_buffer(&out_buf[0], cfg.out_len);
#endif
        ret = memcmp(&out_buf[0], &aes_ecb_plain_data[0], sizeof(aes_ecb_plain_data));
        if (ret) {
            TR_INFO(0, "%s: *** AES%u ECB Decrypt Test Failed ***", __func__, test_cfg[i].key_len * 8);
            return 1;
        } else {
#ifdef TIME_MEASURE_ENABLE
#ifdef SEC_MASS_DATA_TEST_MODE
            TR_INFO(0, "===time cost:%uus/%ubytes, %uus/%ubytes", time, cfg.in_len, time / (cfg.in_len / CALC_RATE_TIME_BYTE), CALC_RATE_TIME_BYTE);
#else
            TR_INFO(0, "===time cost:%uus/%ubytes", time, cfg.in_len);
#endif
#endif
            TR_INFO(0, "%s: === AES%u ECB Decrypt Test Passed ===", __func__, test_cfg[i].key_len * 8);
        }
        TR_INFO(0, " ");
    }

#ifndef SIMU
    TRACE_FLUSH();
#endif

    return 0;
}

POSSIBLY_UNUSED static int aes_cbc_test(void)
{
    struct HAL_SE_AES_CFG_T cfg;
    enum HAL_SE_RET_T ret;
#ifdef TIME_MEASURE_ENABLE
    uint32_t time_start, time;

#ifdef TIME_MEASURE_GPIO
    #define TRIGGER_GPIO_ENC    HAL_GPIO_PIN_P3_0
    #define TRIGGER_GPIO_DEC    HAL_GPIO_PIN_P3_4
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
        {TRIGGER_GPIO_ENC, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {TRIGGER_GPIO_DEC, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(TRIGGER_GPIO_ENC, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(TRIGGER_GPIO_DEC, HAL_GPIO_DIR_OUT, 1);
#endif
#endif

#ifdef SEC_MASS_DATA_TEST_MODE
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());
    rand_generator(aes_cbc_plain_data, MASS_DATA_BUF_SIZE);
#endif

    struct CBC_TEST_CFG {
        const uint8_t *key_data;
        uint32_t key_len;
        const uint8_t *cipher_data;
        uint32_t cipher_len;
        const uint8_t *iv_data;
    };
    struct CBC_TEST_CFG test_cfg[] = {
        {
            .key_data = aes128_cbc_key,
            .key_len = sizeof(aes128_cbc_key),
            .cipher_data = aes128_cbc_cipher_data,
            .cipher_len = sizeof(aes128_cbc_cipher_data),
            .iv_data = aes128_iv,
        },
        {
            .key_data = aes192_cbc_key,
            .key_len = sizeof(aes192_cbc_key),
            .cipher_data = aes192_cbc_cipher_data,
            .cipher_len = sizeof(aes192_cbc_cipher_data),
            .iv_data = aes192_iv,
        },
        {
            .key_data = aes256_cbc_key,
            .key_len = sizeof(aes256_cbc_key),
            .cipher_data = aes256_cbc_cipher_data,
            .cipher_len = sizeof(aes256_cbc_cipher_data),
            .iv_data = aes256_iv,
        },
    };

    TR_INFO(0, " ");
    TR_INFO(0, "%s: Start", __func__);
    TR_INFO(0, " ");

#ifdef SEC_MASS_DATA_TEST_MODE
    while (1)
#if defined(AES_128_MODE)
    for (int i = 0; i < 1; i++) {
#elif defined(AES_192_MODE)
    for (int i = 1; i < 2; i++) {
#else
    for (int i = 2; i < 3; i++) {
#endif
#else   //SEC_MASS_DATA_TEST_MODE
    for (int i = 0; i < ARRAY_SIZE(test_cfg); i++) {
#endif
        memset(&cfg, 0, sizeof(cfg));
        cfg.done_hdlr = sec_eng_done;
        cfg.mode = HAL_SE_AES_CBC;
        cfg.key = (uint8_t *)test_cfg[i].key_data;
        cfg.key_len = test_cfg[i].key_len;
        cfg.in = (uint8_t *)&aes_cbc_plain_data[0];
        cfg.in_len = sizeof(aes_cbc_plain_data);
        cfg.out = (uint8_t *)&out_buf[0];
        cfg.out_len = cfg.in_len;
        if (cfg.mode != HAL_SE_AES_ECB) {
            cfg.iv = (uint8_t *)test_cfg[i].iv_data;
        }

#ifdef TIME_MEASURE_ENABLE
        flag_complete = 0;
        time_start = hal_fast_sys_timer_get();
#ifdef TIME_MEASURE_GPIO
        hal_gpio_pin_clr(TRIGGER_GPIO_ENC);
#endif
#endif
        ret = hal_se_aes_encrypt(&cfg);
        ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_encrypt failed: %d", __func__, ret);
#ifdef TIME_MEASURE_ENABLE
        while (!flag_complete);
#ifdef TIME_MEASURE_GPIO
        hal_gpio_pin_set(TRIGGER_GPIO_ENC);
#endif
        time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
#endif
        while (hal_se_aes_busy()) {
            osDelay(1);
        }
#ifndef SEC_MASS_DATA_TEST_MODE
        TR_INFO(0, "Cipher Text:");
        dump_buffer(&out_buf[0], cfg.out_len);
        ret = memcmp(&out_buf[0], test_cfg[i].cipher_data, test_cfg[i].cipher_len);
#else
        memcpy((void *)test_cfg[i].cipher_data, &out_buf[0], test_cfg[i].cipher_len);
        ret = 0;
#endif
        if (ret) {
            TR_INFO(0, "%s: *** AES%u CBC Encrypt Test Failed ***", __func__, test_cfg[i].key_len * 8);
            return 1;
        } else {
#ifdef TIME_MEASURE_ENABLE
#ifdef SEC_MASS_DATA_TEST_MODE
            TR_INFO(0, "===time cost:%uus/%ubytes, %uus/%ubytes", time, cfg.in_len, time / (cfg.in_len / CALC_RATE_TIME_BYTE), CALC_RATE_TIME_BYTE);
#else
            TR_INFO(0, "===time cost:%uus/%ubytes", time, cfg.in_len);
#endif
#endif
            TR_INFO(0, "%s: === AES%u CBC Encrypt Test Passed, plain data size:%ubytes ===", __func__, test_cfg[i].key_len * 8, cfg.in_len);
        }
        TR_INFO(0, " ");

        cfg.in = (uint8_t *)test_cfg[i].cipher_data;
        cfg.in_len = test_cfg[i].cipher_len;
        cfg.out_len = cfg.in_len;

#ifdef TIME_MEASURE_ENABLE
        flag_complete = 0;
        time_start = hal_fast_sys_timer_get();
#ifdef TIME_MEASURE_GPIO
        hal_gpio_pin_clr(TRIGGER_GPIO_DEC);
#endif
#endif
        ret = hal_se_aes_decrypt(&cfg);
        ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_decrypt failed: %d", __func__, ret);
#ifdef TIME_MEASURE_ENABLE
        while (!flag_complete);
#ifdef TIME_MEASURE_GPIO
        hal_gpio_pin_set(TRIGGER_GPIO_DEC);
#endif
        time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
#endif
        while (hal_se_aes_busy()) {
            osDelay(1);
        }
#ifndef SEC_MASS_DATA_TEST_MODE
        TR_INFO(0, "Plain Text:");
        dump_buffer(&out_buf[0], cfg.out_len);
#endif
        ret = memcmp(&out_buf[0], &aes_cbc_plain_data[0], sizeof(aes_cbc_plain_data));
        if (ret) {
            TR_INFO(0, "%s: *** AES%u CBC Decrypt Test Failed ***", __func__, test_cfg[i].key_len * 8);
            return 1;
        } else {
#ifdef TIME_MEASURE_ENABLE
#ifdef SEC_MASS_DATA_TEST_MODE
            TR_INFO(0, "===time cost:%uus/%ubytes, %uus/%ubytes", time, cfg.in_len, time / (cfg.in_len / CALC_RATE_TIME_BYTE), CALC_RATE_TIME_BYTE);
#else
            TR_INFO(0, "===time cost:%uus/%ubytes", time, cfg.in_len);
#endif
#endif
            TR_INFO(0, "%s: === AES%u CBC Decrypt Test Passed ===", __func__, test_cfg[i].key_len * 8);
        }
        TR_INFO(0, " ");
    }

#ifdef AES_LARGE_DATA
    // Generate Test File:
    // (CAUTION: The size of aes_plain_data must be aligned with AES block size, 16 bytes or 32 bytes)
    // ln -s xxx tests/hwtest/aes_plain_data
    // openssl enc -aes-128-cbc -nosalt -k 123456 -in tests/hwtest/aes_plain_data -out tests/hwtest/aes_cipher_data

    memset(&cfg, 0, sizeof(cfg));
    cfg.done_hdlr = sec_eng_done;
    cfg.mode = HAL_SE_AES_CBC;
    cfg.key = (uint8_t *)&aes_key_large[0];
    cfg.key_len = sizeof(aes_key_large);
    cfg.in = (uint8_t *)&__aes_plain_data_start[0];
    cfg.in_len = __aes_plain_data_end - __aes_plain_data_start;
    cfg.out = (uint8_t *)&out_buf[0];
    cfg.out_len = cfg.in_len;
    if (cfg.mode != HAL_SE_AES_ECB) {
        cfg.iv = (uint8_t *)&aes_iv_large[0];
    }

    ret = hal_se_aes_encrypt(&cfg);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_encrypt failed: %d", __func__, ret);
    while (hal_se_aes_busy()) {
        osDelay(1);
    }

    TR_INFO(0, "Cipher Text:");
    dump_buffer(&out_buf[0], 32);

    ret = memcmp(&out_buf[0], &__aes_cipher_data_start[0], cfg.out_len);
    if (ret) {
        TR_INFO(0, "%s: *** AES Large Encrypt Test Failed ***", __func__);
    } else {
        TR_INFO(0, "%s: === AES Large Encrypt Test Passed ===", __func__);
    }

    if (ret) {
        for (int i = 0; i < cfg.out_len; i++) {
            if (out_buf[i] != __aes_cipher_data_start[i]) {
                TR_INFO(0, "Error pos: %u", i);
                TR_INFO(0, "Encrypt:");
                dump_buffer(&out_buf[i], 16);
                TR_INFO(0, "Cipher:");
                dump_buffer(&__aes_cipher_data_start[i], 16);
                break;
            }
        }
        return 1;
    }
    TR_INFO(0, " ");

    cfg.in = (uint8_t *)&__aes_cipher_data_start[0];
    cfg.in_len = __aes_plain_data_end - __aes_plain_data_start;
    cfg.out_len = cfg.in_len;

    ret = hal_se_aes_decrypt(&cfg);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_decrypt failed: %d", __func__, ret);
    while (hal_se_aes_busy()) {
        osDelay(1);
    }

    TR_INFO(0, "Plain Text:");
    dump_buffer(&out_buf[0], 32);

    ret = memcmp(&out_buf[0], &__aes_plain_data_start[0], cfg.out_len);
    if (ret) {
        TR_INFO(0, "%s: *** AES Large Decrypt Test Failed ***", __func__);
    } else {
        TR_INFO(0, "%s: === AES Large Decrypt Test Passed ===", __func__);
    }

    if (ret) {
        for (int i = 0; i < cfg.out_len; i++) {
            if (out_buf[i] != __aes_plain_data_start[i]) {
                TR_INFO(0, "Error pos: %u", i);
                TR_INFO(0, "Decrypt:");
                dump_buffer(&out_buf[i], 16);
                TR_INFO(0, "Plain:");
                dump_buffer(&__aes_plain_data_start[i], 16);
                break;
            }
        }
        return 1;
    }
    TR_INFO(0, " ");
#endif

    return 0;
}

POSSIBLY_UNUSED static int aes_cbc_ctx_test(void)
{
    struct HAL_SE_AES_CFG_T cfg;
    enum HAL_SE_RET_T ret;
    const uint32_t part1_len = 16;
    uint8_t *cmp_buf = (uint8_t *)&out_buf[0] + sizeof(out_buf) / 2;

    TR_INFO(0, " ");
    TR_INFO(0, "%s: Start", __func__);
    TR_INFO(0, " ");

    ASSERT(sizeof(aes_cbc_plain_data) * 2 <= sizeof(out_buf), "%s: out buf too small", __func__);

    // Setup baseline encryption
    memset(&cfg, 0, sizeof(cfg));
    cfg.done_hdlr = sec_eng_done;
    cfg.mode = HAL_SE_AES_CBC;
    cfg.key = (uint8_t *)&aes192_ecb_key[0];
    cfg.key_len = sizeof(aes192_ecb_key);
    cfg.in = (uint8_t *)&aes_cbc_plain_data[0];
    cfg.in_len = sizeof(aes_cbc_plain_data);
    cfg.out = (uint8_t *)cmp_buf;
    cfg.out_len = cfg.in_len;
    cfg.iv = (uint8_t *)&aes128_iv[0];

    ret = hal_se_aes_encrypt(&cfg);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_encrypt failed: %d", __func__, ret);
    while (hal_se_aes_busy()) {
        osDelay(1);
    }

    // Start segment encryption
    memset(&cfg, 0, sizeof(cfg));
    cfg.done_hdlr = sec_eng_done;
    cfg.mode = HAL_SE_AES_CBC;
    cfg.key = (uint8_t *)&aes192_ecb_key[0];
    cfg.key_len = sizeof(aes192_ecb_key);
    cfg.in = (uint8_t *)&aes_cbc_plain_data[0];
    cfg.in_len = part1_len;
    cfg.out = (uint8_t *)&out_buf[0];
    cfg.out_len = cfg.in_len;
    cfg.iv = (uint8_t *)&aes128_iv[0];
    cfg.seg_oper = HAL_SE_SEG_INIT;
    cfg.seg_ctx_buf = aes_ctx_buf;
    cfg.seg_ctx_len = sizeof(aes_ctx_buf);

    ret = hal_se_aes_encrypt(&cfg);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_encrypt failed: %d", __func__, ret);
    while (hal_se_aes_busy()) {
        osDelay(1);
    }

    memset(&cfg, 0, sizeof(cfg));
    cfg.done_hdlr = sec_eng_done;
    cfg.mode = HAL_SE_AES_CBC;
    //cfg.key = (uint8_t *)&aes192_ecb_key[0];
    cfg.key_len = sizeof(aes192_ecb_key);
    cfg.in = (uint8_t *)&aes_cbc_plain_data[0] + part1_len;
    cfg.in_len = sizeof(aes_cbc_plain_data) - part1_len;
    cfg.out = (uint8_t *)&out_buf[part1_len];
    cfg.out_len = cfg.in_len;
    cfg.seg_oper = HAL_SE_SEG_TERM;
    cfg.seg_ctx_buf = aes_ctx_buf;
    cfg.seg_ctx_len = sizeof(aes_ctx_buf);

    ret = hal_se_aes_encrypt(&cfg);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_encrypt failed: %d", __func__, ret);
    while (hal_se_aes_busy()) {
        osDelay(1);
    }

    TR_INFO(TR_ATTR_IMM, "Cipher Text:");
    dump_buffer(&out_buf[0], sizeof(aes_cbc_plain_data));
    TR_INFO(TR_ATTR_IMM, "Cipher Baseline Text:");
    dump_buffer(cmp_buf, sizeof(aes_cbc_plain_data));

    ret = memcmp(&out_buf[0], cmp_buf, sizeof(aes_cbc_plain_data));
    if (ret) {
        TR_INFO(0, "%s: *** AES192 CBC CTX Encrypt Test Failed ***", __func__);
        return 1;
    } else {
        TR_INFO(0, "%s: === AES192 CBC CTX Encrypt Test Passed ===", __func__);
    }
    TR_INFO(TR_ATTR_IMM, " ");

    // Setup baseline decryption
    memset(&cfg, 0, sizeof(cfg));
    cfg.done_hdlr = sec_eng_done;
    cfg.mode = HAL_SE_AES_CBC;
    cfg.key = (uint8_t *)&aes192_ecb_key[0];
    cfg.key_len = sizeof(aes192_ecb_key);
    cfg.in = (uint8_t *)&aes128_cbc_cipher_data[0];
    cfg.in_len = sizeof(aes128_cbc_cipher_data);
    cfg.out = (uint8_t *)cmp_buf;
    cfg.out_len = cfg.in_len;
    cfg.iv = (uint8_t *)&aes128_iv[0];

    ret = hal_se_aes_decrypt(&cfg);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_decrypt failed: %d", __func__, ret);
    while (hal_se_aes_busy()) {
        osDelay(1);
    }

    // Start segment decryption
    memset(&cfg, 0, sizeof(cfg));
    cfg.done_hdlr = sec_eng_done;
    cfg.mode = HAL_SE_AES_CBC;
    cfg.key = (uint8_t *)&aes192_ecb_key[0];
    cfg.key_len = sizeof(aes192_ecb_key);
    cfg.in = (uint8_t *)&aes128_cbc_cipher_data[0];
    cfg.in_len = part1_len;
    cfg.out = (uint8_t *)&out_buf[0];
    cfg.out_len = cfg.in_len;
    cfg.iv = (uint8_t *)&aes128_iv[0];
    cfg.seg_oper = HAL_SE_SEG_INIT;
    cfg.seg_ctx_buf = aes_ctx_buf;
    cfg.seg_ctx_len = sizeof(aes_ctx_buf);

    ret = hal_se_aes_decrypt(&cfg);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_decrypt failed: %d", __func__, ret);
    while (hal_se_aes_busy()) {
        osDelay(1);
    }

    memset(&cfg, 0, sizeof(cfg));
    cfg.done_hdlr = sec_eng_done;
    cfg.mode = HAL_SE_AES_CBC;
    //cfg.key = (uint8_t *)&aes192_ecb_key[0];
    cfg.key_len = sizeof(aes192_ecb_key);
    cfg.in = (uint8_t *)&aes128_cbc_cipher_data[0] + part1_len;
    cfg.in_len = sizeof(aes128_cbc_cipher_data) - part1_len;
    cfg.out = (uint8_t *)&out_buf[part1_len];
    cfg.out_len = cfg.in_len;
    cfg.seg_oper = HAL_SE_SEG_TERM;
    cfg.seg_ctx_buf = aes_ctx_buf;
    cfg.seg_ctx_len = sizeof(aes_ctx_buf);

    ret = hal_se_aes_decrypt(&cfg);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_aes_decrypt failed: %d", __func__, ret);
    while (hal_se_aes_busy()) {
        osDelay(1);
    }

    TR_INFO(TR_ATTR_IMM, "Plain Text:");
    dump_buffer(&out_buf[0], sizeof(aes128_cbc_cipher_data));
    TR_INFO(TR_ATTR_IMM, "Plain Baseline Text:");
    dump_buffer(cmp_buf, sizeof(aes128_cbc_cipher_data));

    ret = memcmp(&out_buf[0], cmp_buf, sizeof(aes128_cbc_cipher_data));
    if (ret) {
        TR_INFO(0, "%s: *** AES192 CBC CTX Decrypt Test Failed ***", __func__);
        return 1;
    } else {
        TR_INFO(0, "%s: === AES192 CBC CTX Decrypt Test Passed ===", __func__);
    }
    TR_INFO(TR_ATTR_IMM, " ");

    return 0;
}

#ifdef SEC_ENG_HAS_HASH
//tool:http://www.ip33.com/hash.html
static const uint8_t hash_msg_9[]  = "123456789";
static const uint8_t hash_msg_16[] = "1234567890123456";
static const uint8_t hash_msg_32[] = "12345678901234567890123456789012";
static const uint8_t hash_msg_64[] = "1234567890123456789012345678901234567890123456789012345678901234";
static const uint8_t hash_msg_75[] = "123456789012345678901234567890123456789012345678901234567890123456789012345";

static const uint8_t md5_dgst_9[] = {
    0x25, 0xf9, 0xe7, 0x94, 0x32, 0x3b, 0x45, 0x38, 0x85, 0xf5, 0x18, 0x1f, 0x1b, 0x62, 0x4d, 0x0b,
};
static const uint8_t md5_dgst_16[] = {
    0xab, 0xea, 0xc0, 0x7d, 0x3c, 0x28, 0xc1, 0xbe, 0xf9, 0xe7, 0x30, 0x00, 0x2c, 0x75, 0x3e, 0xd4,
};
static const uint8_t md5_dgst_32[] = {
    0x76, 0x71, 0x79, 0xc7, 0xa2, 0xbf, 0xf1, 0x96, 0x51, 0xce, 0x97, 0xd2, 0x94, 0xc3, 0x0c, 0xfb,
};
static const uint8_t md5_dgst_64[] = {
    0xeb, 0x6c, 0x41, 0x79, 0xc0, 0xa7, 0xc8, 0x2c, 0xc2, 0x82, 0x8c, 0x1e, 0x63, 0x38, 0xe1, 0x65,
};
static const uint8_t md5_dgst_75[] = {
    0x83, 0xad, 0x2d, 0x47, 0xc7, 0xe7, 0x72, 0xed, 0x01, 0x41, 0x79, 0xb5, 0x96, 0x16, 0x18, 0x1d,
};

static const uint8_t sha1_dgst_9[] = {
    0xf7, 0xc3, 0xbc, 0x1d, 0x80, 0x8e, 0x04, 0x73, 0x2a, 0xdf, 0x67, 0x99, 0x65, 0xcc, 0xc3, 0x4c, 0xa7, 0xae, 0x34, 0x41,
};
static const uint8_t sha1_dgst_16[] = {
    0xde, 0xed, 0x2a, 0x88, 0xe7, 0x3d, 0xcc, 0xaa, 0x30, 0xa9, 0xe6, 0xe2, 0x96, 0xf6, 0x2b, 0xe2, 0x38, 0xbe, 0x4a, 0xde,
};
static const uint8_t sha1_dgst_32[] = {
    0x93, 0x7d, 0xcb, 0x68, 0xf7, 0x30, 0x8e, 0xec, 0xdb, 0x70, 0x2e, 0xf1, 0x5f, 0xaa, 0x02, 0x73, 0x5f, 0xc3, 0xcc, 0x61,
};
static const uint8_t sha1_dgst_64[] = {
    0xc7, 0x14, 0x90, 0xfc, 0x24, 0xaa, 0x3d, 0x19, 0xe1, 0x12, 0x82, 0xda, 0x77, 0x03, 0x2d, 0xd9, 0xcd, 0xb3, 0x31, 0x03,
};
static const uint8_t sha1_dgst_75[] = {
    0x6a, 0x6d, 0x81, 0xe1, 0x23, 0xb9, 0xd7, 0x96, 0x30, 0x8e, 0xeb, 0x69, 0xc1, 0xcb, 0x46, 0xe3, 0xb2, 0x59, 0x08, 0x63,
};

static const uint8_t sha224_dgst_9[] = {
    0x9b, 0x3e, 0x61, 0xbf, 0x29, 0xf1, 0x7c, 0x75, 0x57, 0x2f, 0xae, 0x2e, 0x86, 0xe1, 0x78, 0x09,
    0xa4, 0x51, 0x3d, 0x07, 0xc8, 0xa1, 0x81, 0x52, 0xac, 0xf3, 0x45, 0x21,
};
static const uint8_t sha224_dgst_16[] = {
    0xca, 0x16, 0x19, 0x7b, 0xe1, 0xee, 0x05, 0xb6, 0x9d, 0x23, 0xb1, 0xa2, 0x1d, 0xfb, 0xea, 0xc6,
    0xfe, 0xd2, 0xc4, 0xa5, 0x7c, 0x7a, 0xb5, 0x40, 0xd7, 0x1c, 0x81, 0xcb,
};
static const uint8_t sha224_dgst_32[] = {
    0x3f, 0xa2, 0x82, 0x0c, 0x79, 0xb0, 0x91, 0xf2, 0xed, 0x6e, 0x0f, 0x81, 0x22, 0x24, 0x3e, 0xee,
    0xc4, 0x76, 0xe5, 0x9d, 0xad, 0xd7, 0x8d, 0x16, 0x26, 0x6c, 0xe5, 0x35,
};
static const uint8_t sha224_dgst_64[] = {
    0x3b, 0x30, 0xd9, 0x0a, 0xba, 0x8a, 0xc4, 0x07, 0x50, 0x2c, 0xac, 0xf4, 0x5b, 0xe7, 0x8b, 0x8b,
    0xe3, 0x1f, 0x8a, 0xde, 0x1d, 0x31, 0x4f, 0x5b, 0x81, 0x0c, 0x83, 0x0e,
};
static const uint8_t sha224_dgst_75[] = {
    0x12, 0x7f, 0xb4, 0x5b, 0x9a, 0x3b, 0xdd, 0x55, 0x61, 0xf5, 0xe8, 0xe9, 0x25, 0x9b, 0xc1, 0x42,
    0xf9, 0xeb, 0x74, 0xb6, 0xdb, 0xc8, 0x0c, 0x20, 0xb3, 0x9c, 0xb6, 0x84,
};

static const uint8_t sha256_dgst_9[] = {
    0x15, 0xe2, 0xb0, 0xd3, 0xc3, 0x38, 0x91, 0xeb, 0xb0, 0xf1, 0xef, 0x60, 0x9e, 0xc4, 0x19, 0x42,
    0x0c, 0x20, 0xe3, 0x20, 0xce, 0x94, 0xc6, 0x5f, 0xbc, 0x8c, 0x33, 0x12, 0x44, 0x8e, 0xb2, 0x25,
};
static const uint8_t sha256_dgst_16[] = {
    0x7a, 0x51, 0xd0, 0x64, 0xa1, 0xa2, 0x16, 0xa6, 0x92, 0xf7, 0x53, 0xfc, 0xda, 0xb2, 0x76, 0xe4,
    0xff, 0x20, 0x1a, 0x01, 0xd8, 0xb6, 0x6f, 0x56, 0xd5, 0x0d, 0x4d, 0x71, 0x9f, 0xd0, 0xdc, 0x87,
};
static const uint8_t sha256_dgst_32[] = {
    0xe1, 0xb8, 0x5b, 0x27, 0xd6, 0xbc, 0xb0, 0x58, 0x46, 0xc1, 0x8e, 0x6a, 0x48, 0xf1, 0x18, 0xe8,
    0x9f, 0x0c, 0x05, 0x87, 0x14, 0x0d, 0xe9, 0xfb, 0x33, 0x59, 0xf8, 0x37, 0x0d, 0x0d, 0xba, 0x08,
};
static const uint8_t sha256_dgst_64[] = {
    0x67, 0x64, 0x91, 0x96, 0x5e, 0xd3, 0xec, 0x50, 0xcb, 0x7a, 0x63, 0xee, 0x96, 0x31, 0x54, 0x80,
    0xa9, 0x5c, 0x54, 0x42, 0x6b, 0x0b, 0x72, 0xbc, 0xa8, 0xa0, 0xd4, 0xad, 0x12, 0x85, 0xad, 0x55,
};
static const uint8_t sha256_dgst_75[] = {
    0x13, 0x73, 0x5f, 0x38, 0x1b, 0xa0, 0xa2, 0xe3, 0xc3, 0xe4, 0x64, 0x93, 0x2a, 0xf0, 0xff, 0x67,
    0x32, 0x25, 0x42, 0x9e, 0xef, 0xd0, 0xfc, 0x05, 0x19, 0xef, 0xbf, 0xbd, 0x37, 0xec, 0x04, 0x49,
};

static const uint8_t sha384_dgst_9[] = {
    0xeb, 0x45, 0x5d, 0x56, 0xd2, 0xc1, 0xa6, 0x9d, 0xe6, 0x4e, 0x83, 0x20, 0x11, 0xf3, 0x39, 0x3d,
    0x45, 0xf3, 0xfa, 0x31, 0xd6, 0x84, 0x2f, 0x21, 0xaf, 0x92, 0xd2, 0xfe, 0x46, 0x9c, 0x49, 0x9d,
    0xa5, 0xe3, 0x17, 0x98, 0x47, 0x33, 0x4a, 0x18, 0x47, 0x9c, 0x8d, 0x1d, 0xed, 0xea, 0x1b, 0xe3,
};
static const uint8_t sha384_dgst_16[] = {
    0x9f, 0x66, 0x5a, 0xcf, 0x54, 0x05, 0xd4, 0x86, 0x48, 0x08, 0x17, 0xc4, 0xdf, 0xa9, 0xa8, 0x2a,
    0x89, 0x4b, 0x6e, 0x07, 0x7d, 0x21, 0x71, 0x61, 0x77, 0x41, 0x37, 0x25, 0x87, 0x38, 0x58, 0x3a,
    0x21, 0x29, 0xb2, 0xdd, 0xa2, 0x25, 0x71, 0x1c, 0x0b, 0x72, 0xcf, 0x9a, 0x00, 0x05, 0x89, 0x79,
};
static const uint8_t sha384_dgst_32[] = {
    0x62, 0xf1, 0x40, 0xc5, 0x6c, 0x96, 0x60, 0x44, 0xf1, 0x06, 0xab, 0xe5, 0x01, 0x5b, 0xb4, 0xd4,
    0xe8, 0xe1, 0x13, 0x80, 0x50, 0x3c, 0xde, 0x20, 0x23, 0x60, 0x7d, 0xca, 0xce, 0xf5, 0x25, 0xb5,
    0x50, 0xb5, 0x20, 0x8d, 0xa2, 0x44, 0x40, 0xd5, 0xb5, 0x64, 0x12, 0x82, 0x2c, 0x6f, 0x59, 0x8d,
};
static const uint8_t sha384_dgst_64[] = {
    0xa4, 0x34, 0xe9, 0xa9, 0x3f, 0xcf, 0x10, 0x8f, 0x43, 0x27, 0x71, 0x8f, 0xd6, 0x8a, 0xba, 0x44,
    0x10, 0x93, 0xcb, 0x4f, 0x4e, 0x7d, 0x36, 0x1c, 0x2f, 0xc5, 0x10, 0xe7, 0x99, 0x84, 0x84, 0x1a,
    0x87, 0xbe, 0x90, 0x6b, 0x90, 0x87, 0xf2, 0xeb, 0xce, 0xb7, 0x47, 0xb1, 0x3a, 0x62, 0xac, 0x78,
};
static const uint8_t sha384_dgst_75[] = {
    0x8d, 0xa4, 0x11, 0x02, 0x77, 0x99, 0x8c, 0xa3, 0xb7, 0xb3, 0x3e, 0x3f, 0x65, 0xa0, 0xf1, 0xfb,
    0x88, 0x05, 0x97, 0x12, 0x5c, 0xd2, 0x0a, 0xdc, 0x72, 0xfe, 0x2b, 0x0e, 0x72, 0x29, 0x5b, 0x02,
    0xd8, 0xb9, 0x8c, 0xa0, 0xd5, 0x3d, 0x1b, 0x89, 0x44, 0x6e, 0x6b, 0x9a, 0xb2, 0xdd, 0xdc, 0xd7,
};

static const uint8_t sha512_dgst_9[] = {
    0xd9, 0xe6, 0x76, 0x2d, 0xd1, 0xc8, 0xea, 0xf6, 0xd6, 0x1b, 0x3c, 0x61, 0x92, 0xfc, 0x40, 0x8d,
    0x4d, 0x6d, 0x5f, 0x11, 0x76, 0xd0, 0xc2, 0x91, 0x69, 0xbc, 0x24, 0xe7, 0x1c, 0x3f, 0x27, 0x4a,
    0xd2, 0x7f, 0xcd, 0x58, 0x11, 0xb3, 0x13, 0xd6, 0x81, 0xf7, 0xe5, 0x5e, 0xc0, 0x2d, 0x73, 0xd4,
    0x99, 0xc9, 0x54, 0x55, 0xb6, 0xb5, 0xbb, 0x50, 0x3a, 0xcf, 0x57, 0x4f, 0xba, 0x8f, 0xfe, 0x85,
};
static const uint8_t sha512_dgst_16[] = {
    0x1f, 0x52, 0xed, 0x51, 0x58, 0x71, 0xc9, 0x13, 0x16, 0x43, 0x98, 0xec, 0x24, 0xc4, 0x70, 0x88,
    0xcd, 0xf9, 0x57, 0xe8, 0x1a, 0xf2, 0x8c, 0x89, 0x9a, 0x8a, 0x01, 0x95, 0xd3, 0x62, 0x0e, 0x08,
    0x39, 0x68, 0xa6, 0xd4, 0xd8, 0x6c, 0xb8, 0xf9, 0xbd, 0x7f, 0x90, 0x9b, 0x23, 0xf7, 0x5a, 0x1c,
    0x04, 0x4e, 0xc8, 0xe6, 0x75, 0xc6, 0xef, 0xbc, 0xb0, 0xe4, 0xbf, 0x0e, 0xb4, 0x45, 0x52, 0x5d,
};
static const uint8_t sha512_dgst_32[] = {
    0xf5, 0x10, 0x66, 0x60, 0xc3, 0xdd, 0xa2, 0x3f, 0x16, 0xda, 0xa9, 0xac, 0x5b, 0x81, 0x1b, 0x96,
    0x30, 0x77, 0xf5, 0xbc, 0x0a, 0xf8, 0x9f, 0x85, 0x80, 0x4f, 0x0d, 0xe8, 0xe4, 0x24, 0xf0, 0x10,
    0xf9, 0x8d, 0x66, 0xf3, 0x94, 0x42, 0x50, 0x6f, 0xf9, 0x47, 0xfd, 0x91, 0x1f, 0x18, 0xc7, 0xa7,
    0xa5, 0xda, 0x63, 0x9a, 0x63, 0xe8, 0xd3, 0xb4, 0xe2, 0x33, 0xf7, 0x41, 0x43, 0xd9, 0x51, 0xc1,
};
static const uint8_t sha512_dgst_64[] = {
    0xc9, 0x67, 0x98, 0xc0, 0x78, 0x1b, 0x89, 0xbb, 0xf0, 0x1d, 0xe9, 0x9f, 0x1a, 0xba, 0x95, 0x0c,
    0xe8, 0x38, 0x84, 0xef, 0x54, 0x51, 0x71, 0x20, 0xd1, 0x58, 0x6a, 0x59, 0x6b, 0xf0, 0x04, 0xf4,
    0x2a, 0xaa, 0xf2, 0x59, 0xda, 0x15, 0xf5, 0xb1, 0x33, 0x25, 0x64, 0x60, 0x9c, 0xa2, 0x70, 0x27,
    0x3c, 0xc3, 0x27, 0x14, 0x67, 0xf9, 0x81, 0xde, 0xed, 0x44, 0x95, 0xb7, 0x8c, 0xde, 0xd9, 0x3c,
};
static const uint8_t sha512_dgst_75[] = {
    0x4b, 0x13, 0x69, 0xcf, 0x13, 0x4d, 0x1b, 0xf8, 0x18, 0x09, 0xde, 0xe3, 0x01, 0xaf, 0x5d, 0x5e,
    0x30, 0x14, 0x7c, 0x63, 0xbc, 0x64, 0x80, 0x3c, 0xba, 0xb4, 0x6a, 0x71, 0x00, 0x16, 0xf9, 0x85,
    0xea, 0x56, 0x11, 0x4a, 0xf1, 0xb6, 0xba, 0x0e, 0xc8, 0x37, 0x3c, 0xda, 0xe4, 0xd0, 0x5d, 0x4d,
    0xe0, 0xc7, 0x80, 0x19, 0x82, 0xa1, 0xf4, 0xc8, 0xfc, 0x0d, 0xb5, 0x9a, 0x5b, 0x76, 0x62, 0x95,
};

struct HASH_MSG_T {
    const uint8_t *msg;
    uint32_t len;
};

struct HASH_DGST_T {
    enum HAL_SE_HASH_MODE_T mode;
    const char *name;
    const uint8_t **dgst_array;
};

static const struct HASH_MSG_T hash_msg_all[] = {
    { hash_msg_9,  sizeof(hash_msg_9)  - 1, },
    { hash_msg_16, sizeof(hash_msg_16) - 1, },
    { hash_msg_32, sizeof(hash_msg_32) - 1, },
    { hash_msg_64, sizeof(hash_msg_64) - 1, },
    { hash_msg_75, sizeof(hash_msg_75) - 1, },
};

static const uint8_t *md5_dgst_all[] = {
    md5_dgst_9, md5_dgst_16, md5_dgst_32, md5_dgst_64, md5_dgst_75,
};
static const uint8_t *sha1_dgst_all[] = {
    sha1_dgst_9, sha1_dgst_16, sha1_dgst_32, sha1_dgst_64, sha1_dgst_75,
};
static const uint8_t *sha224_dgst_all[] = {
    sha224_dgst_9, sha224_dgst_16, sha224_dgst_32, sha224_dgst_64, sha224_dgst_75,
};
static const uint8_t *sha256_dgst_all[] = {
    sha256_dgst_9, sha256_dgst_16, sha256_dgst_32, sha256_dgst_64, sha256_dgst_75,
};
static const uint8_t *sha384_dgst_all[] = {
    sha384_dgst_9, sha384_dgst_16, sha384_dgst_32, sha384_dgst_64, sha384_dgst_75,
};
static const uint8_t *sha512_dgst_all[] = {
    sha512_dgst_9, sha512_dgst_16, sha512_dgst_32, sha512_dgst_64, sha512_dgst_75,
};

#define HASH_TYPE_AND_NAME(n)               HAL_SE_HASH_ ## n, #n

static const struct HASH_DGST_T hash_dgst_all[] = {
    { HASH_TYPE_AND_NAME(MD5),      md5_dgst_all,       },
    { HASH_TYPE_AND_NAME(SHA1),     sha1_dgst_all,      },
    { HASH_TYPE_AND_NAME(SHA224),   sha224_dgst_all,    },
    { HASH_TYPE_AND_NAME(SHA256),   sha256_dgst_all,    },
    { HASH_TYPE_AND_NAME(SHA384),   sha384_dgst_all,    },
    { HASH_TYPE_AND_NAME(SHA512),   sha512_dgst_all,    },
};

extern uint8_t __hash_msg_data_start[];
extern uint8_t __hash_msg_data_end[];
extern uint8_t __hash_digest_data_start[];
extern uint8_t __hash_digest_data_end[];

POSSIBLY_UNUSED
static int hash_test(void)
{
    enum HAL_SE_RET_T ret;
    struct HAL_SE_HASH_CFG_T cfg;
    uint32_t digest[16];
    uint32_t len;
    int i;
    int j;
#ifdef TIME_MEASURE_ENABLE
    uint32_t time_start, time[2];
#endif

    TR_INFO(0, " ");
    TR_INFO(0, "%s: Start", __func__);
    TR_INFO(0, " ");

#ifdef SEC_MASS_DATA_TEST_MODE
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());

#if defined(HASH_MD5_MODE)
    i = 0;
#elif defined(HASH_SHA1_MODE)
    i = 1;
#elif defined(HASH_SHA256_MODE)
    i = 3;
#else
    i = 5;
#endif
    while (1) {
        memset(&cfg, 0, sizeof(cfg));
        cfg.done_hdlr = sec_eng_done;
        cfg.in = out_buf;
        cfg.in_len = MASS_DATA_BUF_SIZE;
        rand_generator(out_buf, MASS_DATA_BUF_SIZE);

#ifdef TIME_MEASURE_ENABLE
        flag_complete = 0;
        time_start = hal_fast_sys_timer_get();
#endif
        ret = hal_se_hash(hash_dgst_all[i].mode, &cfg);
        ASSERT(ret == HAL_SE_OK, "%s: hal_se_hash failed: %s ret=%d", __func__, hash_dgst_all[i].name, ret);
#ifdef TIME_MEASURE_ENABLE
        while (!flag_complete);
        time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
#endif
        while (hal_se_hash_busy()) {
            osDelay(1);
        }
        TR_INFO(0, "===time cost:%uus, %uus/%ubytes", time[0], time[0] / (cfg.in_len / CALC_RATE_TIME_BYTE), CALC_RATE_TIME_BYTE);
        TR_INFO(0, "%s: === HASH Test Passed (%s msg_len=%u) ===", __func__, hash_dgst_all[i].name, cfg.in_len);
    }
#endif

    for (i = 0; i < ARRAY_SIZE(hash_dgst_all); i++) {
        for (j = 0; j < ARRAY_SIZE(hash_msg_all); j++) {
            memset(&cfg, 0, sizeof(cfg));
            cfg.done_hdlr = sec_eng_done;
            cfg.in = hash_msg_all[j].msg;
            cfg.in_len = hash_msg_all[j].len;

#ifdef TIME_MEASURE_ENABLE
            flag_complete = 0;
            time_start = hal_fast_sys_timer_get();
#endif
            ret = hal_se_hash(hash_dgst_all[i].mode, &cfg);
            ASSERT(ret == HAL_SE_OK, "%s: hal_se_hash failed: %s ret=%d", __func__, hash_dgst_all[i].name, ret);
#ifdef TIME_MEASURE_ENABLE
            while (!flag_complete);
            time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
#endif
            while (hal_se_hash_busy()) {
                osDelay(1);
            }

#ifdef TIME_MEASURE_ENABLE
            time_start = hal_fast_sys_timer_get();
#endif
            ret = hal_se_hash_get_digest(&digest[0], sizeof(digest), &len);
            ASSERT(ret == HAL_SE_OK, "%s: hal_se_hash_get_digest failed: %s ret=%d", __func__, hash_dgst_all[i].name, ret);
#ifdef TIME_MEASURE_ENABLE
            time[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
#endif
            TR_INFO(0, "%s: Get digest: %s len=%u", __func__, hash_dgst_all[i].name, len);
            dump_buffer(digest, len);

            ret = memcmp(digest, hash_dgst_all[i].dgst_array[j], len);
            if (ret) {
                TR_INFO(0, "%s: *** HASH Test Failed (%s/%u msg_len=%u) ***", __func__, hash_dgst_all[i].name, len, hash_msg_all[j].len);
            } else {
#ifdef TIME_MEASURE_ENABLE
                TR_INFO(0, "===time cost:calc=%uus,get dgst=%uus", time[0], time[1]);
#endif
                TR_INFO(0, "%s: === HASH Test Passed (%s/%u msg_len=%u) ===", __func__, hash_dgst_all[i].name, len, hash_msg_all[j].len);
            }

            if (ret) {
                TR_INFO(0, "%s: Result:", __func__);
                dump_buffer(digest, len);
                TR_INFO(0, "%s: Verified Hash:", __func__);
                dump_buffer(hash_dgst_all[i].dgst_array[j], len);
                goto _exit;
            }

            TR_INFO(0, " ");
            NORM_LOG_FLUSH();
        }
    }

#ifdef HASH_LARGE_DATA
    // Generate Test File:
    // ln -s xxx tests/hwtest/hash_msg_data
    // openssl dgst -sha256 -binary -out tests/hwtest/hash_digest_data tests/hwtest/hash_msg_data

    const uint32_t max_seg_len = 64;
    uint32_t cur_len = 0;
    uint32_t large_msg_len = __hash_msg_data_end - __hash_msg_data_start;
    uint32_t large_dgst_len = __hash_digest_data_end - __hash_digest_data_start;

    ret = hal_se_hash_init(HAL_SE_HASH_SHA256);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_hash_init failed: ret=%d", __func__, ret);
    while (hal_se_hash_busy()) {
        osDelay(1);
    }

    while (cur_len < large_msg_len) {
        cfg.in = __hash_msg_data_start + cur_len;
        cfg.in_len = large_msg_len - cur_len;
        if (cfg.in_len > max_seg_len) {
            cfg.in_len = max_seg_len;
            ret = hal_se_hash_update(&cfg);
            ASSERT(ret == HAL_SE_OK, "%s: hal_se_hash_update failed: ret=%d", __func__, ret);
        } else {
            ret = hal_se_hash_final(&cfg, large_msg_len);
            ASSERT(ret == HAL_SE_OK, "%s: hal_se_hash_final failed: ret=%d", __func__, ret);
        }
        cur_len += cfg.in_len;
        while (hal_se_hash_busy()) {
            osDelay(1);
            NORM_LOG_FLUSH();
        }
    }

    ret = hal_se_hash_get_digest(&digest[0], sizeof(digest), &len);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_hash_get_digest failed: ret=%d", __func__, ret);
    ASSERT(len == large_dgst_len, "%s: Bad hash len: len=%u large_dgst_len=%u", __func__, len, large_dgst_len);

    TR_INFO(0, "%s: Get digest: len=%u", __func__, len);
    dump_buffer(digest, len);

    ret = memcmp(digest, __hash_digest_data_start, len);
    if (ret) {
        TR_INFO(0, "%s: *** HASH Large Test Failed (msg_len=%u) ***", __func__, large_msg_len);
    } else {
        TR_INFO(0, "%s: === HASH Large Test Passed (msg_len=%u) ===", __func__, large_msg_len);
    }

    if (ret) {
        TR_INFO(0, "%s: Result:", __func__);
        dump_buffer(digest, len);
        TR_INFO(0, "%s: Verified Hash:", __func__);
        dump_buffer(__hash_digest_data_start, len);
        goto _exit;
    }
#endif

_exit:
    if (ret) {
        TR_INFO(0, "%s: *** HASH Test Failed ***", __func__);
    } else {
        TR_INFO(0, "%s: === HASH Test Passed ===", __func__);
    }
    TR_INFO(0, " ");

    return ret;
}

static const uint8_t hmac_key_prf1[] = {
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b,
};
static const uint8_t hmac_data_prf1[] = {
    0x48, 0x69, 0x20, 0x54, 0x68, 0x65, 0x72, 0x65,
};
static const uint8_t hmac_sha256_prf1[] = {
    0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53, 0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
    0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7, 0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7,
};
static const uint8_t hmac_sha384_prf1[] = {
    0xaf, 0xd0, 0x39, 0x44, 0xd8, 0x48, 0x95, 0x62, 0x6b, 0x08, 0x25, 0xf4, 0xab, 0x46, 0x90, 0x7f,
    0x15, 0xf9, 0xda, 0xdb, 0xe4, 0x10, 0x1e, 0xc6, 0x82, 0xaa, 0x03, 0x4c, 0x7c, 0xeb, 0xc5, 0x9c,
    0xfa, 0xea, 0x9e, 0xa9, 0x07, 0x6e, 0xde, 0x7f, 0x4a, 0xf1, 0x52, 0xe8, 0xb2, 0xfa, 0x9c, 0xb6,
};
static const uint8_t hmac_sha512_prf1[] = {
    0x87, 0xaa, 0x7c, 0xde, 0xa5, 0xef, 0x61, 0x9d, 0x4f, 0xf0, 0xb4, 0x24, 0x1a, 0x1d, 0x6c, 0xb0,
    0x23, 0x79, 0xf4, 0xe2, 0xce, 0x4e, 0xc2, 0x78, 0x7a, 0xd0, 0xb3, 0x05, 0x45, 0xe1, 0x7c, 0xde,
    0xda, 0xa8, 0x33, 0xb7, 0xd6, 0xb8, 0xa7, 0x02, 0x03, 0x8b, 0x27, 0x4e, 0xae, 0xa3, 0xf4, 0xe4,
    0xbe, 0x9d, 0x91, 0x4e, 0xeb, 0x61, 0xf1, 0x70, 0x2e, 0x69, 0x6c, 0x20, 0x3a, 0x12, 0x68, 0x54,
};

static const uint8_t hmac_key_prf2[] = {
    0x4a, 0x65, 0x66, 0x65,
};
static const uint8_t hmac_data_prf2[] = {
    0x77, 0x68, 0x61, 0x74, 0x20, 0x64, 0x6f, 0x20, 0x79, 0x61, 0x20, 0x77, 0x61, 0x6e, 0x74, 0x20,
    0x66, 0x6f, 0x72, 0x20, 0x6e, 0x6f, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x3f,
};
static const uint8_t hmac_sha256_prf2[] = {
    0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e, 0x6a, 0x04, 0x24, 0x26, 0x08, 0x95, 0x75, 0xc7,
    0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27, 0x39, 0x83, 0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43,
};
static const uint8_t hmac_sha384_prf2[] = {
    0xaf, 0x45, 0xd2, 0xe3, 0x76, 0x48, 0x40, 0x31, 0x61, 0x7f, 0x78, 0xd2, 0xb5, 0x8a, 0x6b, 0x1b,
    0x9c, 0x7e, 0xf4, 0x64, 0xf5, 0xa0, 0x1b, 0x47, 0xe4, 0x2e, 0xc3, 0x73, 0x63, 0x22, 0x44, 0x5e,
    0x8e, 0x22, 0x40, 0xca, 0x5e, 0x69, 0xe2, 0xc7, 0x8b, 0x32, 0x39, 0xec, 0xfa, 0xb2, 0x16, 0x49,
};
static const uint8_t hmac_sha512_prf2[] = {
    0x16, 0x4b, 0x7a, 0x7b, 0xfc, 0xf8, 0x19, 0xe2, 0xe3, 0x95, 0xfb, 0xe7, 0x3b, 0x56, 0xe0, 0xa3,
    0x87, 0xbd, 0x64, 0x22, 0x2e, 0x83, 0x1f, 0xd6, 0x10, 0x27, 0x0c, 0xd7, 0xea, 0x25, 0x05, 0x54,
    0x97, 0x58, 0xbf, 0x75, 0xc0, 0x5a, 0x99, 0x4a, 0x6d, 0x03, 0x4f, 0x65, 0xf8, 0xf0, 0xe6, 0xfd,
    0xca, 0xea, 0xb1, 0xa3, 0x4d, 0x4a, 0x6b, 0x4b, 0x63, 0x6e, 0x07, 0x0a, 0x38, 0xbc, 0xe7, 0x37,
};

static const uint8_t hmac_key_prf3[] = {
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa,
};
static const uint8_t hmac_data_prf3[] = {
    0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
    0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
    0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
    0xdd, 0xdd,
};
static const uint8_t hmac_sha256_prf3[] = {
    0x77, 0x3e, 0xa9, 0x1e, 0x36, 0x80, 0x0e, 0x46, 0x85, 0x4d, 0xb8, 0xeb, 0xd0, 0x91, 0x81, 0xa7,
    0x29, 0x59, 0x09, 0x8b, 0x3e, 0xf8, 0xc1, 0x22, 0xd9, 0x63, 0x55, 0x14, 0xce, 0xd5, 0x65, 0xfe,
};
static const uint8_t hmac_sha384_prf3[] = {
    0x88, 0x06, 0x26, 0x08, 0xd3, 0xe6, 0xad, 0x8a, 0x0a, 0xa2, 0xac, 0xe0, 0x14, 0xc8, 0xa8, 0x6f,
    0x0a, 0xa6, 0x35, 0xd9, 0x47, 0xac, 0x9f, 0xeb, 0xe8, 0x3e, 0xf4, 0xe5, 0x59, 0x66, 0x14, 0x4b,
    0x2a, 0x5a, 0xb3, 0x9d, 0xc1, 0x38, 0x14, 0xb9, 0x4e, 0x3a, 0xb6, 0xe1, 0x01, 0xa3, 0x4f, 0x27,
};
static const uint8_t hmac_sha512_prf3[] = {
    0xfa, 0x73, 0xb0, 0x08, 0x9d, 0x56, 0xa2, 0x84, 0xef, 0xb0, 0xf0, 0x75, 0x6c, 0x89, 0x0b, 0xe9,
    0xb1, 0xb5, 0xdb, 0xdd, 0x8e, 0xe8, 0x1a, 0x36, 0x55, 0xf8, 0x3e, 0x33, 0xb2, 0x27, 0x9d, 0x39,
    0xbf, 0x3e, 0x84, 0x82, 0x79, 0xa7, 0x22, 0xc8, 0x06, 0xb4, 0x85, 0xa4, 0x7e, 0x67, 0xc8, 0x07,
    0xb9, 0x46, 0xa3, 0x37, 0xbe, 0xe8, 0x94, 0x26, 0x74, 0x27, 0x88, 0x59, 0xe1, 0x32, 0x92, 0xfb,
};

static const uint8_t hmac_key_prf4[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
};
static const uint8_t hmac_data_prf4[] = {
    0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
    0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
    0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
    0xcd, 0xcd,
};
static const uint8_t hmac_sha256_prf4[] = {
    0x82, 0x55, 0x8a, 0x38, 0x9a, 0x44, 0x3c, 0x0e, 0xa4, 0xcc, 0x81, 0x98, 0x99, 0xf2, 0x08, 0x3a,
    0x85, 0xf0, 0xfa, 0xa3, 0xe5, 0x78, 0xf8, 0x07, 0x7a, 0x2e, 0x3f, 0xf4, 0x67, 0x29, 0x66, 0x5b,
};
static const uint8_t hmac_sha384_prf4[] = {
    0x3e, 0x8a, 0x69, 0xb7, 0x78, 0x3c, 0x25, 0x85, 0x19, 0x33, 0xab, 0x62, 0x90, 0xaf, 0x6c, 0xa7,
    0x7a, 0x99, 0x81, 0x48, 0x08, 0x50, 0x00, 0x9c, 0xc5, 0x57, 0x7c, 0x6e, 0x1f, 0x57, 0x3b, 0x4e,
    0x68, 0x01, 0xdd, 0x23, 0xc4, 0xa7, 0xd6, 0x79, 0xcc, 0xf8, 0xa3, 0x86, 0xc6, 0x74, 0xcf, 0xfb,
};
static const uint8_t hmac_sha512_prf4[] = {
    0xb0, 0xba, 0x46, 0x56, 0x37, 0x45, 0x8c, 0x69, 0x90, 0xe5, 0xa8, 0xc5, 0xf6, 0x1d, 0x4a, 0xf7,
    0xe5, 0x76, 0xd9, 0x7f, 0xf9, 0x4b, 0x87, 0x2d, 0xe7, 0x6f, 0x80, 0x50, 0x36, 0x1e, 0xe3, 0xdb,
    0xa9, 0x1c, 0xa5, 0xc1, 0x1a, 0xa2, 0x5e, 0xb4, 0xd6, 0x79, 0x27, 0x5c, 0xc5, 0x78, 0x80, 0x63,
    0xa5, 0xf1, 0x97, 0x41, 0x12, 0x0c, 0x4f, 0x2d, 0xe2, 0xad, 0xeb, 0xeb, 0x10, 0xa2, 0x98, 0xdd,
};

static const uint8_t *hmac_sha256_all[] = {
    hmac_sha256_prf1, hmac_sha256_prf2, hmac_sha256_prf3, hmac_sha256_prf4,
};
static const uint8_t *hmac_sha384_all[] = {
    hmac_sha384_prf1, hmac_sha384_prf2, hmac_sha384_prf3, hmac_sha384_prf4,
};
static const uint8_t *hmac_sha512_all[] = {
    hmac_sha512_prf1, hmac_sha512_prf2, hmac_sha512_prf3, hmac_sha512_prf4,
};

struct HMAC_KEY_T {
    const uint8_t *key;
    uint32_t len;
};

static const struct HMAC_KEY_T hmac_key_all[] = {
    { hmac_key_prf1, sizeof(hmac_key_prf1), },
    { hmac_key_prf2, sizeof(hmac_key_prf2), },
    { hmac_key_prf3, sizeof(hmac_key_prf3), },
    { hmac_key_prf4, sizeof(hmac_key_prf4), },
};

static const struct HASH_MSG_T hmac_msg_all[] = {
    { hmac_data_prf1, sizeof(hmac_data_prf1), },
    { hmac_data_prf2, sizeof(hmac_data_prf2), },
    { hmac_data_prf3, sizeof(hmac_data_prf3), },
    { hmac_data_prf4, sizeof(hmac_data_prf4), },
};

static const struct HASH_DGST_T hmac_dgst_all[] = {
    { HASH_TYPE_AND_NAME(SHA256),   hmac_sha256_all, },
    { HASH_TYPE_AND_NAME(SHA384),   hmac_sha384_all, },
    { HASH_TYPE_AND_NAME(SHA512),   hmac_sha512_all, },
};

extern uint8_t __hmac_digest_data_start[];
extern uint8_t __hmac_digest_data_end[];

POSSIBLY_UNUSED
static int hmac_test(void)
{
    enum HAL_SE_RET_T ret;
    struct HAL_SE_HASH_CFG_T cfg;
    uint32_t digest[16];
    uint32_t len;
    int i;
    int j;
#ifdef TIME_MEASURE_ENABLE
    uint32_t time_start, time[2];
#endif

    TR_INFO(0, " ");
    TR_INFO(0, "%s: Start", __func__);
    TR_INFO(0, " ");

#ifdef SEC_MASS_DATA_TEST_MODE
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());

#if defined(HMAC_SHA256_MODE)
    i = 0;
    j = 0;
#elif defined(HMAC_SHA384_MODE)
    i = 1;
    j = 1;
#else
    i = 2;
    j = 2;
#endif
    while (1) {
        memset(&cfg, 0, sizeof(cfg));
        cfg.done_hdlr = sec_eng_done;
        cfg.in = out_buf;
        cfg.in_len = MASS_DATA_BUF_SIZE;
        rand_generator(out_buf, MASS_DATA_BUF_SIZE);

#ifdef TIME_MEASURE_ENABLE
        flag_complete = 0;
        time_start = hal_fast_sys_timer_get();
#endif
        ret = hal_se_hmac(hmac_dgst_all[i].mode, hmac_key_all[j].key, hmac_key_all[j].len, &cfg);
        ASSERT(ret == HAL_SE_OK, "%s: hal_se_hmac failed: %s ret=%d", __func__, hmac_dgst_all[i].name, ret);
#ifdef TIME_MEASURE_ENABLE
        while (!flag_complete);
        time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
#endif
        while (hal_se_hash_busy()) {
            osDelay(1);
        }
        TR_INFO(0, "===time cost:%uus, %uus/%ubytes", time[0], time[0] / (cfg.in_len / CALC_RATE_TIME_BYTE), CALC_RATE_TIME_BYTE);
        TR_INFO(0, "%s: === HMAC Test Passed (%s msg_len=%u) ===", __func__, hmac_dgst_all[i].name, cfg.in_len);
    }
#endif

    for (i = 0; i < ARRAY_SIZE(hmac_dgst_all); i++) {
        for (j = 0; j < ARRAY_SIZE(hmac_msg_all); j++) {
            memset(&cfg, 0, sizeof(cfg));
            cfg.done_hdlr = sec_eng_done;
            cfg.in = hmac_msg_all[j].msg;
            cfg.in_len = hmac_msg_all[j].len;

#ifdef TIME_MEASURE_ENABLE
            flag_complete = 0;
            time_start = hal_fast_sys_timer_get();
#endif
            ret = hal_se_hmac(hmac_dgst_all[i].mode, hmac_key_all[j].key, hmac_key_all[j].len, &cfg);
            ASSERT(ret == HAL_SE_OK, "%s: hal_se_hmac failed: %s ret=%d", __func__, hmac_dgst_all[i].name, ret);
#ifdef TIME_MEASURE_ENABLE
            while (!flag_complete);
            time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
#endif
            while (hal_se_hmac_busy()) {
                osDelay(1);
            }

#ifdef TIME_MEASURE_ENABLE
            time_start = hal_fast_sys_timer_get();
#endif
            ret = hal_se_hmac_get_digest(&digest[0], sizeof(digest), &len);
            ASSERT(ret == HAL_SE_OK, "%s: hal_se_hmac_get_digest failed: %s ret=%d", __func__, hmac_dgst_all[i].name, ret);
#ifdef TIME_MEASURE_ENABLE
            time[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
#endif
            TR_INFO(0, "%s: Get digest: %s len=%u", __func__, hmac_dgst_all[i].name, len);
            dump_buffer(digest, len);

            ret = memcmp(digest, hmac_dgst_all[i].dgst_array[j], len);
            if (ret) {
                TR_INFO(0, "%s: *** HMAC Test Failed (%s/%u msg_len=%u) ***", __func__, hmac_dgst_all[i].name, len, hmac_msg_all[j].len);
            } else {
#ifdef TIME_MEASURE_ENABLE
                TR_INFO(0, "===time cost:calc=%uus,get dgst=%uus", time[0], time[1]);
#endif
                TR_INFO(0, "%s: === HMAC Test Passed (%s/%u msg_len=%u) ===", __func__, hmac_dgst_all[i].name, len, hmac_msg_all[j].len);
            }

            if (ret) {
                TR_INFO(0, "%s: Result:", __func__);
                dump_buffer(digest, len);
                TR_INFO(0, "%s: Verified Hmac:", __func__);
                dump_buffer(hmac_dgst_all[i].dgst_array[j], len);
                goto _exit;
            }

            TR_INFO(0, " ");
            NORM_LOG_FLUSH();
        }
    }

#ifdef HMAC_LARGE_DATA
    // Generate Test File:
    // [ ! -f tests/hwtest/hmac_digest_data ] && ln -s xxx tests/hwtest/hash_msg_data
    // openssl dgst -hmac 123456 -binary -out tests/hwtest/hmac_digest_data tests/hwtest/hash_msg_data

    const uint32_t max_seg_len = 64;
    uint32_t cur_len = 0;
    uint32_t large_msg_len = __hash_msg_data_end - __hash_msg_data_start;
    uint32_t large_dgst_len = __hmac_digest_data_end - __hmac_digest_data_start;

    static const uint8_t large_key[] = "123456";
    const uint32_t large_key_len = sizeof(large_key) - 1;

    ret = hal_se_hmac_init(HAL_SE_HASH_SHA256, large_key, large_key_len);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_hmac_init failed: ret=%d", __func__, ret);
    while (hal_se_hmac_busy()) {
        osDelay(1);
    }

    while (cur_len < large_msg_len) {
        cfg.in = __hash_msg_data_start + cur_len;
        cfg.in_len = large_msg_len - cur_len;
        if (cfg.in_len > max_seg_len) {
            cfg.in_len = max_seg_len;
            ret = hal_se_hmac_update(&cfg);
            ASSERT(ret == HAL_SE_OK, "%s: hal_se_hmac_update failed: ret=%d", __func__, ret);
        } else {
            ret = hal_se_hmac_final(&cfg, large_msg_len);
            ASSERT(ret == HAL_SE_OK, "%s: hal_se_hmac_final failed: ret=%d", __func__, ret);
        }
        cur_len += cfg.in_len;
        while (hal_se_hmac_busy()) {
            osDelay(1);
            NORM_LOG_FLUSH();
        }
    }

    ret = hal_se_hmac_get_digest(&digest[0], sizeof(digest), &len);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_hmac_get_digest failed: ret=%d", __func__, ret);
    ASSERT(len == large_dgst_len, "%s: Bad hash len: len=%u large_dgst_len=%u", __func__, len, large_dgst_len);

    TR_INFO(0, "%s: Get digest: len=%u", __func__, len);
    dump_buffer(digest, len);

    ret = memcmp(digest, __hmac_digest_data_start, len);
    if (ret) {
        TR_INFO(0, "%s: *** HMAC Large Test Failed (msg_len=%u) ***", __func__, large_msg_len);
    } else {
        TR_INFO(0, "%s: === HMAC Large Test Passed (msg_len=%u) ===", __func__, large_msg_len);
    }

    if (ret) {
        TR_INFO(0, "%s: Result:", __func__);
        dump_buffer(digest, len);
        TR_INFO(0, "%s: Verified HMAC:", __func__);
        dump_buffer(__hmac_digest_data_start, len);
        goto _exit;
    }
#endif

_exit:
    if (ret) {
        TR_INFO(0, "%s: *** HMAC Test Failed ***", __func__);
    } else {
        TR_INFO(0, "%s: === HMAC Test Passed ===", __func__);
    }
    TR_INFO(0, " ");
    return ret;
}

SEC_ENG_BUF_LOC
static unsigned char hash_ctx_msg[128];
static unsigned char hash_ctx_buf[4][64];
static unsigned char ctx_good_digest[4][64];
static unsigned char ctx_test_digest[4][64];

int hash_ctx_test(void)
{
    struct HAL_SE_HASH_CFG_T cfg;
    uint32_t len;
    int ret;
    unsigned char *buf = hash_ctx_msg;
    int i;
    const int cnt = 10;
    enum HAL_SE_HASH_MODE_T mode_list[] = {
        HAL_SE_HASH_SHA224, HAL_SE_HASH_SHA256, HAL_SE_HASH_SHA384, HAL_SE_HASH_SHA512,
    };
    int k;

    TR_INFO(0, " ");
    TR_INFO(0, "%s: Start", __func__);
    TR_INFO(0, " ");

    memset(buf, 'a', 64);
    memset(buf + 64, 0, 64);

    for (k = 0; k < ARRAY_SIZE(mode_list); k++) {
        ret = hal_se_hash_init(mode_list[k]);
        ASSERT(ret == HAL_SE_OK, "%s:%d: hal_se_hash_init failed: ret=%d", __func__, __LINE__, ret);
        while (hal_se_hash_busy()) {
            osDelay(1);
        }
        if (mode_list[k] == HAL_SE_HASH_SHA384 || mode_list[k] == HAL_SE_HASH_SHA512) {
            len = 128;
        } else {
            len = 64;
        }
        for (i = 0; i < cnt; i++) {
            memset(&cfg, 0, sizeof(cfg));
            cfg.in = buf;
            cfg.in_len = len;
            ret = hal_se_hash_update(&cfg);
            ASSERT(ret == HAL_SE_OK, "%s:%d: hal_se_hash_update failed: ret=%d", __func__, __LINE__, ret);
            while (hal_se_hash_busy()) {
                osDelay(1);
            }
        }

        memset(&cfg, 0, sizeof(cfg));
        cfg.in = buf;
        cfg.in_len = len;
        ret = hal_se_hash_final(&cfg, len * (cnt + 1));
        ASSERT(ret == HAL_SE_OK, "%s:%d: hal_se_hash_final failed: ret=%d", __func__, __LINE__, ret);
        while (hal_se_hash_busy()) {
            osDelay(1);
        }
        ret = hal_se_hash_get_digest(ctx_good_digest[k], sizeof(ctx_good_digest[k]), &len);
        ASSERT(ret == HAL_SE_OK, "%s:%d: hal_se_hash_get_digest failed: ret=%d", __func__, __LINE__, ret);
        TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "HASH SHA%u digest:", hal_se_hash_mode_digest_len(mode_list[k]) * 8);
        for (i = 0; i < len; i += 8) {
            DUMP8(" %02X", ctx_good_digest[k] + i, 8);
        }
    }

    for (i = 0; i < cnt; i++) {
        for (k = 0; k < ARRAY_SIZE(mode_list); k++) {
            ret = hal_se_hash_init(mode_list[k]);
            ASSERT(ret == HAL_SE_OK, "%s:%d: hal_se_hash_init failed: ret=%d", __func__, __LINE__, ret);
            while (hal_se_hash_busy()) {
                osDelay(1);
            }
            if (mode_list[k] == HAL_SE_HASH_SHA384 || mode_list[k] == HAL_SE_HASH_SHA512) {
                len = 128;
            } else {
                len = 64;
            }
            memset(&cfg, 0, sizeof(cfg));
            cfg.in = buf;
            cfg.in_len = len;
            cfg.seg_oper = (i == 0) ? HAL_SE_SEG_INIT : HAL_SE_SEG_CONT;
            cfg.seg_ctx_buf = hash_ctx_buf[k];
            cfg.seg_ctx_len = sizeof(hash_ctx_buf[k]);
            ret = hal_se_hash_update(&cfg);
            ASSERT(ret == HAL_SE_OK, "%s:%d: hal_se_hash_update failed: ret=%d", __func__, __LINE__, ret);
            while (hal_se_hash_busy()) {
                osDelay(1);
            }
        }
    }

    for (k = 0; k < ARRAY_SIZE(mode_list); k++) {
        ret = hal_se_hash_init(mode_list[k]);
        ASSERT(ret == HAL_SE_OK, "%s:%d: hal_se_hash_init failed: ret=%d", __func__, __LINE__, ret);
        while (hal_se_hash_busy()) {
            osDelay(1);
        }
        if (mode_list[k] == HAL_SE_HASH_SHA384 || mode_list[k] == HAL_SE_HASH_SHA512) {
            len = 128;
        } else {
            len = 64;
        }
        memset(&cfg, 0, sizeof(cfg));
        cfg.in = buf;
        cfg.in_len = len;
        cfg.seg_oper = HAL_SE_SEG_TERM;
        cfg.seg_ctx_buf = hash_ctx_buf[k];
        cfg.seg_ctx_len = sizeof(hash_ctx_buf[k]);
        ret = hal_se_hash_final(&cfg, len * (cnt + 1));
        ASSERT(ret == HAL_SE_OK, "%s:%d: hal_se_hash_update failed: ret=%d", __func__, __LINE__, ret);
        while (hal_se_hash_busy()) {
            osDelay(1);
        }
        ret = hal_se_hash_get_digest(ctx_test_digest[k], sizeof(ctx_test_digest[k]), &len);
        ASSERT(ret == HAL_SE_OK, "%s:%d: hal_se_hash_get_digest failed: ret=%d", __func__, __LINE__, ret);
        TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "HASH SHA%u digest:", hal_se_hash_mode_digest_len(mode_list[k]) * 8);
        for (i = 0; i < len; i += 8) {
            DUMP8(" %02X", ctx_test_digest[k] + i, 8);
        }
    }

    ret = 0;
    for (k = 0; k < ARRAY_SIZE(mode_list); k++) {
        len = hal_se_hash_mode_digest_len(mode_list[k]);
        if (memcmp(ctx_test_digest[k], ctx_good_digest[k], len)) {
            TRACE(0, "HASH SHA%u: Result is bad", hal_se_hash_mode_digest_len(mode_list[k]) * 8);
            ret = 1;
        } else {
            TRACE(0, "HASH SHA%u: Result is good", hal_se_hash_mode_digest_len(mode_list[k]) * 8);
        }
    }

    return ret;
}
#endif // SEC_ENG_HAS_HASH

int se_read_bn(uint32_t *N, uint32_t nwords, const char *s)
{
    uint32_t i;
    uint32_t j;
    uint32_t len;
    uint8_t k;

    if (s == NULL) {
        return 1;
    }
    if (s[0] == '-') {
        // Not support negative value yet
        return 2;
    }
    len = strlen(s);
    if (len > nwords * 8) {
        return 3;
    }
    for (i = 0; i < nwords; i++) {
        N[i] = 0;
    }
    for (i = len, j = 0; i > 0; i--, j++) {
        k = s[i - 1];
        if ('0' <= k && k <= '9') {
            k -= '0';
        } else if ('a' <= k && k <= 'f') {
            k = k - 'a' + 0xA;
        } else if ('A' <= k && k <= 'F') {
            k = k - 'A' + 0xA;
        } else {
            return 4;
        }
        N[j / 8] |= k << ((j % 8) * 4);
    }
    return 0;
}

void se_print_bn(const char *name, const uint32_t *a, uint32_t nwords)
{
    int non_zero = false;

    TRACE(0, "%s=", name);
    while (nwords--) {
        if (non_zero || a[nwords]) {
            TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_LF, "%08X", a[nwords]);
            non_zero = true;
        }
    }
    if (non_zero) {
        TRACE_OUTPUT_LINEFEED();
    } else {
        TRACE(TR_ATTR_NO_TS, "0");
    }
}

int se_zero_bn(uint32_t *N, uint32_t nwords)
{
    uint32_t i;

    for (i = 0; i < nwords; i++) {
        N[i] = 0;
    }
    return 0;
}

int se_cmp_bn(const uint32_t *a, const uint32_t *b, uint32_t nwords)
{
    while (nwords--) {
        if (a[nwords] > b[nwords])
            return 1;
        if (a[nwords] < b[nwords])
            return -1;
    }
    return 0;
}

#if defined(SEC_ENG_HAS_ZMODP) && !defined(SEC_ENG_ROM_ONLY)
#define ZMODP_BITS                          2048

static uint32_t zmodp_X[ZMODP_BITS / 8 / sizeof(uint32_t)];
static uint32_t zmodp_Y[ZMODP_BITS / 8 / sizeof(uint32_t)];
static uint32_t zmodp_Z[ZMODP_BITS / 8 / sizeof(uint32_t)];
static uint32_t zmodp_M[ZMODP_BITS / 8 / sizeof(uint32_t)];
static uint32_t zmodp_X1[ZMODP_BITS / 8 / sizeof(uint32_t)];
static uint32_t zmodp_EXP[ZMODP_BITS / 8 / sizeof(uint32_t)];
static uint32_t zmodp_CMP[ZMODP_BITS / 8 / sizeof(uint32_t)];

int zmodp_test(void)
{
    int ret;
    uint32_t start_time;
    uint32_t end_time;

    TR_INFO(0, " ");
    TR_INFO(0, "%s: Start", __func__);
    TR_INFO(0, " ");

    ret = hal_se_zmodp_init(true);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_zmodp_init failed: %d", __func__, ret);

    TR_INFO(0, "ZMODP Pre Compute Test");

    se_read_bn(zmodp_M, ARRAY_SIZE(zmodp_M),
                "A1CE6BC1FB3FCA85F4360F6DEC6D743D"
                "3DA0C568026F74F922F78F20E0D270BE"
                "FCB12F24E533E8395D144CD7E621AA98"
                "85B3382083BCD1AE19B017BC01BF84AE"
                "5F66DA8BA4D88005CCD8AC444A1FE6A5"
                "988D4EF8B39635A105211A6B31CC9DB1"
                "A523F1F62C6051E9280C36A45BA51399"
                "0ED8225448D6CC830185B93F7F6340A6"
                "39B83D65E72C5225101EDD52EBFA9223"
                "077764D6A034DC4B68E5CE4D42BD874A"
                "6F2E4BA5DC9AB7AAE3C5294D08D60D72"
                "69FCC11D269A96EBA73148DB68FC8905"
                "1F755D9C9FCB2360D0D119C638E50432"
                "3CE6672AEBE95288FA13E42C5BD6FFE9"
                "4DD5ECFBE08ADECE36FC17CBFB24D8D4"
                "AF83680870ACB40CBA0EED84E66EDC99");
    se_zero_bn(zmodp_Z, ARRAY_SIZE(zmodp_Z));

    //se_print_bn("M", zmodp_M, ARRAY_SIZE(zmodp_M));

    start_time = hal_fast_sys_timer_get();

    hal_se_zmodp_pre_comput(zmodp_M, zmodp_Z, ZMODP_BITS, ZMODP_BITS);

    end_time = hal_fast_sys_timer_get();

    //se_print_bn("Z", zmodp_Z, ARRAY_SIZE(zmodp_Z));

    se_read_bn(zmodp_CMP, ARRAY_SIZE(zmodp_CMP),
                "27c5494024cbd901fdc5432599c2addc"
                "e724eb4e081281b27fea99a0e4a16a9a"
                "392916ad2947b9440bcd2ae484710f91"
                "bb38c7133f1c721cbb52161a94b158ac"
                "adde3391a709ce87794430113125690b"
                "a566fc674af4199669a71db1300a85a4"
                "62ca1d6a618d3af91f4db91c953e2cf6"
                "d51d42b3f39f1feea770273f926d27ab"
                "44e6d6dbbdb7cda5a4349a69892eb345"
                "513f667da509fc0a07fe537f8f40e6b4"
                "c19a67b964f3e33fcdd141d38b4cad26"
                "02b0e3efe7f92fcd08663ceaee61310f"
                "a0ead5caa4485f87698a67e3bfdb869b"
                "4619a96d449be52f9cad5eaf36ef4c0d"
                "d87f32734808764e6469b3ecd701ff99"
                "c1bbef9d8d58aff02903b8c0442875e7");
    ret = se_cmp_bn(zmodp_Z, zmodp_CMP, ARRAY_SIZE(zmodp_CMP));
    if (ret) {
        TR_INFO(0, "ZMODP Pre Compute failed: %d", ret);
        return ret;
    }
    TR_INFO(0, "ZMODP Pre Compute success: %u us", FAST_TICKS_TO_US(end_time - start_time));

    TR_INFO(0, "ZMODP Montgomery Mul Test");

    se_read_bn(zmodp_X, ARRAY_SIZE(zmodp_X),
                "1"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000");
    se_read_bn(zmodp_Y, ARRAY_SIZE(zmodp_Y),
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
                "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
                "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
                "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                "99999999999999999999999999999999"
                "88888888888888888888888888888888"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000"
                "00000000000000000000000000000000");
    se_read_bn(zmodp_M, ARRAY_SIZE(zmodp_M),
                "A1CE6BC1FB3FCA85F4360F6DEC6D743D"
                "3DA0C568026F74F922F78F20E0D270BE"
                "FCB12F24E533E8395D144CD7E621AA98"
                "85B3382083BCD1AE19B017BC01BF84AE"
                "5F66DA8BA4D88005CCD8AC444A1FE6A5"
                "988D4EF8B39635A105211A6B31CC9DB1"
                "A523F1F62C6051E9280C36A45BA51399"
                "0ED8225448D6CC830185B93F7F6340A6"
                "39B83D65E72C5225101EDD52EBFA9223"
                "077764D6A034DC4B68E5CE4D42BD874A"
                "6F2E4BA5DC9AB7AAE3C5294D08D60D72"
                "69FCC11D269A96EBA73148DB68FC8905"
                "1F755D9C9FCB2360D0D119C638E50432"
                "3CE6672AEBE95288FA13E42C5BD6FFE9"
                "4DD5ECFBE08ADECE36FC17CBFB24D8D4"
                "AF83680870ACB40CBA0EED84E66EDC99");
    se_zero_bn(zmodp_Z, ARRAY_SIZE(zmodp_Z));

    start_time = hal_fast_sys_timer_get();

    hal_se_zmodp_montgomery_mul(zmodp_M, zmodp_X, zmodp_Y, zmodp_Z, ZMODP_BITS, ZMODP_BITS);

    end_time = hal_fast_sys_timer_get();

    se_read_bn(zmodp_CMP, ARRAY_SIZE(zmodp_CMP),
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
                "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
                "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
                "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                "99999999999999999999999999999999"
                "88888888888888888888888888888888");
    ret = se_cmp_bn(zmodp_Z, zmodp_CMP, ARRAY_SIZE(zmodp_CMP));
    if (ret) {
        TR_INFO(0, "ZMODP Montgomery Mul failed: %d", ret);
        return ret;
    }
    TR_INFO(0, "ZMODP Montgomery Mul success: %u us", FAST_TICKS_TO_US(end_time - start_time));

    TR_INFO(0, "ZMODP Modular Mul Test");

    se_read_bn(zmodp_X, ARRAY_SIZE(zmodp_X),
                "00000000000000000000000000000000"
                "11111111111111111111111111111111"
                "22222222222222222222222222222222"
                "33333333333333333333333333333333"
                "44444444444444444444444444444444"
                "55555555555555555555555555555555"
                "66666666666666666666666666666666"
                "77777777777777777777777777777777");
    se_read_bn(zmodp_Y, ARRAY_SIZE(zmodp_Y),
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
                "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
                "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
                "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                "99999999999999999999999999999999"
                "88888888888888888888888888888888");
    se_read_bn(zmodp_M, ARRAY_SIZE(zmodp_M),
                "A1CE6BC1FB3FCA85F4360F6DEC6D743D"
                "3DA0C568026F74F922F78F20E0D270BE"
                "FCB12F24E533E8395D144CD7E621AA98"
                "85B3382083BCD1AE19B017BC01BF84AE"
                "5F66DA8BA4D88005CCD8AC444A1FE6A5"
                "988D4EF8B39635A105211A6B31CC9DB1"
                "A523F1F62C6051E9280C36A45BA51399"
                "0ED8225448D6CC830185B93F7F6340A6"
                "39B83D65E72C5225101EDD52EBFA9223"
                "077764D6A034DC4B68E5CE4D42BD874A"
                "6F2E4BA5DC9AB7AAE3C5294D08D60D72"
                "69FCC11D269A96EBA73148DB68FC8905"
                "1F755D9C9FCB2360D0D119C638E50432"
                "3CE6672AEBE95288FA13E42C5BD6FFE9"
                "4DD5ECFBE08ADECE36FC17CBFB24D8D4"
                "AF83680870ACB40CBA0EED84E66EDC99");
    se_zero_bn(zmodp_Z, ARRAY_SIZE(zmodp_Z));
    se_zero_bn(zmodp_X1, ARRAY_SIZE(zmodp_X1));

    start_time = hal_fast_sys_timer_get();

    hal_se_zmodp_pre_comput(zmodp_M, zmodp_X1, ZMODP_BITS, ZMODP_BITS);
    /* `num_coefs` must be 2048, or answer is incorrect! */
    hal_se_zmodp_modular_mul(zmodp_M, zmodp_X, zmodp_Y, zmodp_X1, zmodp_Z, ZMODP_BITS, ZMODP_BITS);

    end_time = hal_fast_sys_timer_get();

    se_read_bn(zmodp_CMP, ARRAY_SIZE(zmodp_CMP),
                "11111111111111111111111111111111"
                "20fedcba987654320fedcba987654321"
                "0da740da740da740da740da740da740d"
                "b4e81b4e81b4e81b4e81b4e81b4e81b4"
                "f49f49f49f49f49f49f49f49f49f49f4"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "b4e81b4e81b4e81b4e81b4e81b4e81b4"
                "68acf13579be02468acf13579be02468"
                "a4fa4fa4fa4fa4fa4fa4fa4fa4fa4fa4"
                "f37c048d159e26af37c048d159e26af3"
                "7654320fedcba987654320fedcba9876"
                "4fa4fa4fa4fa4fa4fa4fa4fa4fa4fa4f"
                "a1907f6e5d4c3b2a1907f6e5d4c3b2a1"
                "8e38e38e38e38e38e38e38e38e38e38e"
                "37c048d159e26af37c048d159e26af38");
    ret = se_cmp_bn(zmodp_Z, zmodp_CMP, ARRAY_SIZE(zmodp_CMP));
    if (ret) {
        TR_INFO(0, "ZMODP Modular Mul failed: %d", ret);
        return ret;
    }
    TR_INFO(0, "ZMODP Modular Mul success: %u us", FAST_TICKS_TO_US(end_time - start_time));

    TR_INFO(0, "ZMODP Modular Exp Test");

    se_read_bn(zmodp_M, ARRAY_SIZE(zmodp_M),
                "A1CE6BC1FB3FCA85F4360F6DEC6D743D"
                "3DA0C568026F74F922F78F20E0D270BE"
                "FCB12F24E533E8395D144CD7E621AA98"
                "85B3382083BCD1AE19B017BC01BF84AE"
                "5F66DA8BA4D88005CCD8AC444A1FE6A5"
                "988D4EF8B39635A105211A6B31CC9DB1"
                "A523F1F62C6051E9280C36A45BA51399"
                "0ED8225448D6CC830185B93F7F6340A6"
                "39B83D65E72C5225101EDD52EBFA9223"
                "077764D6A034DC4B68E5CE4D42BD874A"
                "6F2E4BA5DC9AB7AAE3C5294D08D60D72"
                "69FCC11D269A96EBA73148DB68FC8905"
                "1F755D9C9FCB2360D0D119C638E50432"
                "3CE6672AEBE95288FA13E42C5BD6FFE9"
                "4DD5ECFBE08ADECE36FC17CBFB24D8D4"
                "AF83680870ACB40CBA0EED84E66EDC99");
    se_read_bn(zmodp_X, ARRAY_SIZE(zmodp_X),
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
                "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
                "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
                "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                "99999999999999999999999999999999"
                "88888888888888888888888888888888");
    se_read_bn(zmodp_EXP, ARRAY_SIZE(zmodp_EXP), "10001");
    se_zero_bn(zmodp_Z, ARRAY_SIZE(zmodp_Z));
    se_zero_bn(zmodp_X1, ARRAY_SIZE(zmodp_X1));

    start_time = hal_fast_sys_timer_get();

    hal_se_zmodp_pre_comput(zmodp_M, zmodp_X1, ZMODP_BITS, ZMODP_BITS);
    /* `num_coefs` must be 2048, or answer is incorrect! */
    hal_se_zmodp_modular_exp(zmodp_M, zmodp_X, zmodp_X1, zmodp_EXP, zmodp_Z, ZMODP_BITS, ZMODP_BITS);

    end_time = hal_fast_sys_timer_get();

    se_read_bn(zmodp_CMP, ARRAY_SIZE(zmodp_CMP),
            "0e99b405100ebd84c975e5f0511de202"
            "6a1a350d650bf10173051eb8c868e2a5"
            "f2e07d791a2e87b3435fb0366ec2f436"
            "33c44596bd08accd990d4ef3f2e79c28"
            "7c3a0d846f8898527ef865de4dfb48ae"
            "a51cc33db4dda01896e39dfcba47ce21"
            "de5d906229eddbeec2be91514a49380a"
            "db683a523a16a1b101f95bb72458b8eb"
            "7929739f3ec57924ed0054ed6e39ba8f"
            "995516f3467cb2243b9558cb462cb1c4"
            "0322a53ece3ea7e89cad1f9284e5460b"
            "3b7b554f9e01b73336805318871997d9"
            "8e3061b2b3139e48c8b2c95a8c8664cb"
            "2d4ad8a3806ea79b7d17d626fba95095"
            "478356a9b6fee4e1ac58209ca14aca7a"
            "74d1064dd4e8a05c2471fa4e69a33315");
    ret = se_cmp_bn(zmodp_Z, zmodp_CMP, ARRAY_SIZE(zmodp_CMP));
    if (ret) {
        TR_INFO(0, "ZMODP Modular Exp failed: %d", ret);
        return ret;
    }
    TR_INFO(0, "ZMODP Modular Exp success: %u us", FAST_TICKS_TO_US(end_time - start_time));

    TR_INFO(0, "ZMODP Modular Inv 1024 Test");

    se_read_bn(zmodp_M, ARRAY_SIZE(zmodp_M),
                "F36993E3A4F890C0ADF100C2F25E37A8"
                "944D5482CA84F68EFCDB2BB26BCDD61F"
                "346019F3E4D7815FDF010369C006CF01"
                "6EA7178ED514F268F12A239B18AA4732"
                "39A650B50BBA4CDC1ACB44FDE196BC0D"
                "D7A23BF2DAE85F3071448C1C6D1F314E"
                "E2D94FA8F626F4670CB558705A34C5AE"
                "D3DF9C1CD1C72415567B5ADDBE5BDA2D");
    se_read_bn(zmodp_X, ARRAY_SIZE(zmodp_X),
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
                "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
                "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
                "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                "99999999999999999999999999999999"
                "88888888888888888888888888888888");
    se_zero_bn(zmodp_Z, ARRAY_SIZE(zmodp_Z));
    se_zero_bn(zmodp_Y, ARRAY_SIZE(zmodp_Y));

    start_time = hal_fast_sys_timer_get();

    hal_se_zmodp_pre_comput(zmodp_M, zmodp_Y, 1024, 1024);
    /* M must be a prime */
    hal_se_zmodp_modular_inv(zmodp_M, zmodp_X, zmodp_Y, zmodp_Z, 1024, 1024);

    end_time = hal_fast_sys_timer_get();

    se_read_bn(zmodp_CMP, ARRAY_SIZE(zmodp_CMP),
                "cf604a7ac38211cae4776efe6fcea090"
                "163c5bb18ff5582d124e19229b2bab29"
                "386eec44ea9743e2b95d9953c966efbc"
                "c103b00e5a3b1ace14a5a32ba7882c40"
                "f5a6239f5c33cec4c054edb0b959dfcc"
                "ba2cecc7f579fc0be86a85a5485cc7e7"
                "c55627cbfd3927c8994ac21aafe47d01"
                "fe962eb481c47c8806d277af9b14639b");
    ret = se_cmp_bn(zmodp_Z, zmodp_CMP, ARRAY_SIZE(zmodp_CMP));
    if (ret) {
        TR_INFO(0, "ZMODP Modular Inv 1024 failed: %d", ret);
        return ret;
    }
    TR_INFO(0, "ZMODP Modular Inv 1024 success: %u us", FAST_TICKS_TO_US(end_time - start_time));

    TR_INFO(0, "ZMODP Modular Inv 2048 Test");

    se_read_bn(zmodp_M, ARRAY_SIZE(zmodp_M),
                "dcf465a7c6e637e2c6e8a01e03a76d44baba7b8ecec1e563a3cd4442c449bb72"
                "4af1e3ddba4af2ba8cc0620500234cb12defc345db5cf368a7e62d4a2e432e3f"
                "962f219997f63f7950b21ec2442a7560221dc9a4ba0844668ba684330c6f86ce"
                "c5db3061164a40ae9ac308fab0cb4a0bd5e3062b16db2a1dd9037abc11af381d"
                "ace7c003694a718c2b4dbb6b7ff0044f5de8e22526c52a49cfc1d872281a2047"
                "aed11fec9a79cb68fbd44bf97cd27a4285d2198b22d23378e5e543047aaf7c8b"
                "61a90906d30ec25bfb01478b1b7228956ae42000cff98dc8fe353f3c206930f4"
                "ce758f974e239e06a3e589cc15772d5c17e65e8c9059d8999f96ac19f1d6523d");
    se_read_bn(zmodp_X, ARRAY_SIZE(zmodp_X),
                "ea966bd09ffc725be216507149161b4e8161d11d1c8a8d2033c9b3007327948f"
                "d92742277c3ccbcfd064f038e93dae525a5898baaf368906d2ba118b2714a907"
                "c58f735f694de33093be764e331b51af3f22174367fff62daa0e8023dd405200"
                "54b117f3c2f96a91f33b29a8216e7e8b60927549c8da28fe262da71f07a3ef22"
                "ea13cfb5c8ec06f2fc3ca4caf221ef918436a80e9d00e7135794214b18f97522"
                "c90f79c6d372022ebae51852af0123d9450c51523ebc27a4877d708ed24dcb50"
                "b2d211ecb14e5a8c4c030d1dee6f70d");
    se_zero_bn(zmodp_Z, ARRAY_SIZE(zmodp_Z));
    se_zero_bn(zmodp_Y, ARRAY_SIZE(zmodp_Y));

    start_time = hal_fast_sys_timer_get();

    hal_se_zmodp_pre_comput(zmodp_M, zmodp_Y, ZMODP_BITS, ZMODP_BITS);
    /* M must be a prime */
    hal_se_zmodp_modular_inv(zmodp_M, zmodp_X, zmodp_Y, zmodp_Z, ZMODP_BITS, ZMODP_BITS);

    end_time = hal_fast_sys_timer_get();

    se_read_bn(zmodp_CMP, ARRAY_SIZE(zmodp_CMP),
                "1bc73f9bdb233179b65e6a1e5053541235fbe280e4dd96a944f76c830f28a74f"
                "ebe509ff771d39ab811565988439624366f8fb071f170881610ce81cf23378db"
                "965641dbeb1c26602f09908112183bfee5184cc2b4326a0db3d78e0ec5099d97"
                "81520f009a292068fe1e3a59734ed6e90f4d9b3c3e784eb3c76fd83dde7eaf55"
                "110aca58e01dba01b262db0484b8eb87c8ee7922a7028144d414cd3337dcfc9f"
                "2d2f864d1aa2b4776d183138dbabb3781b84fbb57bedbab7862a9ad0efefd6c0"
                "06b4e22a4bc187a9037dbb4a92ae2685178ddbafbe31678ea749496f68fd91f4"
                "217004f14ef6f18928236c6cb90873b52b4d9828e1511a36b6f8181f467db57c");
    ret = se_cmp_bn(zmodp_Z, zmodp_CMP, ARRAY_SIZE(zmodp_CMP));
    if (ret) {
        TR_INFO(0, "ZMODP Modular Inv 2048 failed: %d", ret);
        return ret;
    }
    TR_INFO(0, "ZMODP Modular Inv 2048 success: %u us", FAST_TICKS_TO_US(end_time - start_time));

    return 0;
}
#endif // SEC_ENG_HAS_ZMODP && !SEC_ENG_ROM_ONLY

#if defined(SEC_ENG_HAS_ECP) && !defined(SEC_ENG_ROM_ONLY)
#define ECP_BITS                            521
#define ECP_NWORDS                          ECP_GET_WCNT(ECP_BITS)

#define ECP_GET_WCNT(bits)                  ((((bits) + 7) / 8 + 3) / 4)

static uint32_t ecp_A[ECP_NWORDS];
static uint32_t ecp_B[ECP_NWORDS];
static uint32_t ecp_X1[ECP_NWORDS];
static uint32_t ecp_Y1[ECP_NWORDS];
static uint32_t ecp_X2[ECP_NWORDS];
static uint32_t ecp_Y2[ECP_NWORDS];
static uint32_t ecp_RX[ECP_NWORDS];
static uint32_t ecp_RY[ECP_NWORDS];
static uint32_t ecp_CMPX[ECP_NWORDS];
static uint32_t ecp_CMPY[ECP_NWORDS];
static struct HAL_SE_ECP_POINT_T ecp_ab = { ecp_A, ecp_B, };
static struct HAL_SE_ECP_POINT_T ecp_xy1 = { ecp_X1, ecp_Y1, };
static struct HAL_SE_ECP_POINT_T ecp_xy2 = { ecp_X2, ecp_Y2, };
static struct HAL_SE_ECP_POINT_T ecp_rxy = { ecp_RX, ecp_RY, };

int ecp_test(void)
{
    int ret;
    uint32_t start_time;
    uint32_t end_time;

    TR_INFO(0, " ");
    TR_INFO(0, "%s: Start", __func__);
    TR_INFO(0, " ");

    ret = hal_se_ecp_init();
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_ecp_init failed: %d", __func__, ret);

    TR_INFO(0, "ECP P224 ADD Test");

    se_read_bn(ecp_A, ARRAY_SIZE(ecp_A),
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFE");
    se_read_bn(ecp_B, ARRAY_SIZE(ecp_B),
                "B4050A850C04B3ABF54132565044B0B7D7BFD8BA270B39432355FFB4");
    se_read_bn(ecp_X1, ARRAY_SIZE(ecp_X1),
                "B70E0CBD6BB4BF7F321390B94A03C1D356C21122343280D6115C1D21");
    se_read_bn(ecp_Y1, ARRAY_SIZE(ecp_Y1),
                "BD376388B5F723FB4C22DFE6CD4375A05A07476444D5819985007E34");
    se_read_bn(ecp_X2, ARRAY_SIZE(ecp_X2),
                "AF065CDB099785D908DB24A30D7B018DA6EF38B444DB4A2EA6F724DD");
    se_read_bn(ecp_Y2, ARRAY_SIZE(ecp_Y2),
                "9E1EF269E926181392A604DDD75C2B0E96E3397BC0FC38152808C227");
    se_zero_bn(ecp_RX, ARRAY_SIZE(ecp_RX));
    se_zero_bn(ecp_RY, ARRAY_SIZE(ecp_RY));

    start_time = hal_fast_sys_timer_get();

    ret = hal_se_ecp_calc(HAL_SE_ECP_P224, HAL_SE_ECP_OP_ADD, &ecp_ab, &ecp_xy1, &ecp_xy2, &ecp_rxy);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_ecp_calc failed: %d", __func__, ret);

    end_time = hal_fast_sys_timer_get();

    se_read_bn(ecp_CMPX, ARRAY_SIZE(ecp_CMPX),
                "37e60a0ec5c56b43f077d57150fac5f46c819386c1e6d6abac7477c9");
    se_read_bn(ecp_CMPY, ARRAY_SIZE(ecp_CMPY),
                "ecc1062f1d98276361ba84990c96934be0ae59013082b9ef90347102");
    ret = se_cmp_bn(ecp_RX, ecp_CMPX, ARRAY_SIZE(ecp_CMPX));
    ret |= se_cmp_bn(ecp_RY, ecp_CMPY, ARRAY_SIZE(ecp_CMPY));
    if (ret) {
        TR_INFO(0, "ECP P224 ADD failed: %d", ret);
        return ret;
    }
    TR_INFO(0, "ECP P224 ADD success: %u us", FAST_TICKS_TO_US(end_time - start_time));

    // TODO: Add more ECP tests here

    return 0;
}
#endif

void sec_eng_test(void)
{
    int ret = 0;
    uint32_t time_start, time;

    TRACE(0, "  ");
    TRACE(0, "%s: Start", __func__);

    time_start = hal_fast_sys_timer_get();
    ret = hal_se_open();
    time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    ASSERT(ret == HAL_SE_OK, "%s: hal_se_open failed: %d", __func__, ret);
    TRACE(0, "hal_se_open() execution time: %u us", time);

    ret = aes_ecb_test();
    if (ret) {
        goto _exit;
    }

    ret = aes_cbc_test();
    if (ret) {
        goto _exit;
    }

    ret = aes_cbc_ctx_test();
    if (ret) {
        goto _exit;
    }

#ifdef SEC_ENG_HAS_HASH
    ret = hash_test();
    if (ret) {
        goto _exit;
    }

    ret = hmac_test();
    if (ret) {
        goto _exit;
    }

    ret = hash_ctx_test();
    if (ret) {
        goto _exit;
    }
#endif

#if defined(SEC_ENG_HAS_ZMODP) && !defined(SEC_ENG_ROM_ONLY)
    ret = zmodp_test();
    if (ret) {
        goto _exit;
    }
#endif

#if defined(SEC_ENG_HAS_ECP) && !defined(SEC_ENG_ROM_ONLY)
    ret = ecp_test();
    if (ret) {
        goto _exit;
    }
#endif

_exit:
    time_start = hal_fast_sys_timer_get();
    hal_se_close();
    time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    TRACE(0, "  ");
    TRACE(0, "hal_se_close() execution time: %u us", time);

    TRACE(0, "  ");
    TRACE(0, "%s: End", __func__);
    TRACE(0, "%s", ret ? "Test error" : "All tests passed");
}

#endif

