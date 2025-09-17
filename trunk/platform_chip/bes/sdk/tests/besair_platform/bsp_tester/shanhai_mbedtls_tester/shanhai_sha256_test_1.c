#if !defined(MBEDTLS_CONFIG_FILE)
    #include "mbedtls/config.h"
#else
    #include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_CIPHER_C)

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "hexdump.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"

#include "mbedtls/md.h"
#include "mbedtls/debug.h"
#include "mbedtls/cipher.h"

#define mbedtls_printf printf

static const unsigned char sha256_test_buf[3][57] = {
    { "abc" },
    { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" },
    { "" }
};

static const size_t sha256_test_buflen[3] = {
    3, 56, 1000
};

static unsigned char sha256_test_sum[6][32] = {
    /*
     * SHA-224 test vectors
     */
    {
        0x23, 0x09, 0x7D, 0x22, 0x34, 0x05, 0xD8, 0x22,
        0x86, 0x42, 0xA4, 0x77, 0xBD, 0xA2, 0x55, 0xB3,
        0x2A, 0xAD, 0xBC, 0xE4, 0xBD, 0xA0, 0xB3, 0xF7,
        0xE3, 0x6C, 0x9D, 0xA7
    },
    {
        0x75, 0x38, 0x8B, 0x16, 0x51, 0x27, 0x76, 0xCC,
        0x5D, 0xBA, 0x5D, 0xA1, 0xFD, 0x89, 0x01, 0x50,
        0xB0, 0xC6, 0x45, 0x5C, 0xB4, 0xF5, 0x8B, 0x19,
        0x52, 0x52, 0x25, 0x25
    },
    {
        0x20, 0x79, 0x46, 0x55, 0x98, 0x0C, 0x91, 0xD8,
        0xBB, 0xB4, 0xC1, 0xEA, 0x97, 0x61, 0x8A, 0x4B,
        0xF0, 0x3F, 0x42, 0x58, 0x19, 0x48, 0xB2, 0xEE,
        0x4E, 0xE7, 0xAD, 0x67
    },

    /*
     * SHA-256 test vectors
     */
    {
        0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA,
        0x41, 0x41, 0x40, 0xDE, 0x5D, 0xAE, 0x22, 0x23,
        0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C,
        0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD
    },
    {
        0x24, 0x8D, 0x6A, 0x61, 0xD2, 0x06, 0x38, 0xB8,
        0xE5, 0xC0, 0x26, 0x93, 0x0C, 0x3E, 0x60, 0x39,
        0xA3, 0x3C, 0xE4, 0x59, 0x64, 0xFF, 0x21, 0x67,
        0xF6, 0xEC, 0xED, 0xD4, 0x19, 0xDB, 0x06, 0xC1
    },
    {
        0xCD, 0xC7, 0x6E, 0x5C, 0x99, 0x14, 0xFB, 0x92,
        0x81, 0xA1, 0xC7, 0xE2, 0x84, 0xD7, 0x3E, 0x67,
        0xF1, 0x80, 0x9A, 0x48, 0xA4, 0x97, 0x20, 0x0E,
        0x04, 0x6D, 0x39, 0xCC, 0xC7, 0x11, 0x2C, 0xD0
    }
};

static unsigned char buf[1024];
static unsigned char sha256sum[32];

int shanhai_sha256_test_1(int verbose)
{
    int i, j, k, buflen, ret = 0;
    mbedtls_md_type_t md_type;

    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    if (ret)
        goto fail;

    for (i = 0; i < 6; i++) {
        j = i % 3;
        k = i < 3;
        switch (256 - k * 32) {
            case 224:
                md_type = MBEDTLS_MD_SHA224;
                break;
            case 256:
                md_type = MBEDTLS_MD_SHA256;
                break;
            default:
                break;
        }

        mbedtls_md_init(&ctx);
        info = mbedtls_md_info_from_type(md_type);
        ret = mbedtls_md_setup(&ctx, info, 0);

        if (verbose != 0)
            mbedtls_printf("SHA%d-TEST-1 test #%d: start\n", 256 - k * 32,  i + 1);

        if ((ret = mbedtls_md_starts(&ctx)) != 0)
            goto fail;

        if (j == 2) {
            memset(buf, 'a', buflen = 1000);

            for (j = 0; j < 1000; j++) {
                ret = mbedtls_md_update(&ctx, buf, buflen);
            }
        } else {
            ret = mbedtls_md_update(&ctx, sha256_test_buf[j], sha256_test_buflen[j]);
            if (ret != 0)
                goto fail;
        }

        if ((ret = mbedtls_md_finish(&ctx, sha256sum)) != 0)
            goto fail;

        if (memcmp(sha256sum, sha256_test_sum[i], 32 - k * 4) != 0) {
            ret = 1;
            goto fail;
        }

        mbedtls_md_free(&ctx);

        if (verbose != 0)
            mbedtls_printf("SHA%d-TEST-1 test #%d: pass\n", 256 - k * 32, i + 1);
    }

    goto exit;

fail:
    if (verbose != 0)
        mbedtls_printf("SHA%d-TEST-1 test #%d: failed\n", 256 - k * 32, i + 1);

exit:
    return ret;
}

#endif
