/*
 *  Implementation of NIST SP 800-38F key wrapping, supporting KW and KWP modes
 *  only
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
/*
 * Definition of Key Wrapping:
 * https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-38F.pdf
 * RFC 3394 "Advanced Encryption Standard (AES) Key Wrap Algorithm"
 * RFC 5649 "Advanced Encryption Standard (AES) Key Wrap with Padding Algorithm"
 *
 * Note: RFC 3394 defines different methodology for intermediate operations for
 * the wrapping and unwrapping operation than the definition in NIST SP 800-38F.
 */

#include "common.h"

#if defined(MBEDTLS_NIST_KW_C)

#include "mbedtls/nist_kw.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/error.h"
#include "mbedtls/constant_time.h"

#include <stdint.h>
#include <string.h>

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif /* MBEDTLS_PLATFORM_C */

#define KW_TESTS 3

/*
 * Test vectors taken from NIST
 * https://csrc.nist.gov/Projects/Cryptographic-Algorithm-Validation-Program/CAVP-TESTING-BLOCK-CIPHER-MODES#KW
 */
static const unsigned int key_len[KW_TESTS] = { 16, 24, 32 };

static const unsigned char kw_key[KW_TESTS][32] = {
    { 0x75, 0x75, 0xda, 0x3a, 0x93, 0x60, 0x7c, 0xc2,
      0xbf, 0xd8, 0xce, 0xc7, 0xaa, 0xdf, 0xd9, 0xa6 },
    { 0x2d, 0x85, 0x26, 0x08, 0x1d, 0x02, 0xfb, 0x5b,
      0x85, 0xf6, 0x9a, 0xc2, 0x86, 0xec, 0xd5, 0x7d,
      0x40, 0xdf, 0x5d, 0xf3, 0x49, 0x47, 0x44, 0xd3 },
    { 0x11, 0x2a, 0xd4, 0x1b, 0x48, 0x56, 0xc7, 0x25,
      0x4a, 0x98, 0x48, 0xd3, 0x0f, 0xdd, 0x78, 0x33,
      0x5b, 0x03, 0x9a, 0x48, 0xa8, 0x96, 0x2c, 0x4d,
      0x1c, 0xb7, 0x8e, 0xab, 0xd5, 0xda, 0xd7, 0x88 }
};

static const unsigned char kw_msg[KW_TESTS][40] = {
    { 0x42, 0x13, 0x6d, 0x3c, 0x38, 0x4a, 0x3e, 0xea,
      0xc9, 0x5a, 0x06, 0x6f, 0xd2, 0x8f, 0xed, 0x3f },
    { 0x95, 0xc1, 0x1b, 0xf5, 0x35, 0x3a, 0xfe, 0xdb,
      0x98, 0xfd, 0xd6, 0xc8, 0xca, 0x6f, 0xdb, 0x6d,
      0xa5, 0x4b, 0x74, 0xb4, 0x99, 0x0f, 0xdc, 0x45,
      0xc0, 0x9d, 0x15, 0x8f, 0x51, 0xce, 0x62, 0x9d,
      0xe2, 0xaf, 0x26, 0xe3, 0x25, 0x0e, 0x6b, 0x4c },
    { 0x1b, 0x20, 0xbf, 0x19, 0x90, 0xb0, 0x65, 0xd7,
      0x98, 0xe1, 0xb3, 0x22, 0x64, 0xad, 0x50, 0xa8,
      0x74, 0x74, 0x92, 0xba, 0x09, 0xa0, 0x4d, 0xd1 }
};

static const size_t kw_msg_len[KW_TESTS] = { 16, 40, 24 };
static const size_t kw_out_len[KW_TESTS] = { 24, 48, 32 };
static const unsigned char kw_res[KW_TESTS][48] = {
    { 0x03, 0x1f, 0x6b, 0xd7, 0xe6, 0x1e, 0x64, 0x3d,
      0xf6, 0x85, 0x94, 0x81, 0x6f, 0x64, 0xca, 0xa3,
      0xf5, 0x6f, 0xab, 0xea, 0x25, 0x48, 0xf5, 0xfb },
    { 0x44, 0x3c, 0x6f, 0x15, 0x09, 0x83, 0x71, 0x91,
      0x3e, 0x5c, 0x81, 0x4c, 0xa1, 0xa0, 0x42, 0xec,
      0x68, 0x2f, 0x7b, 0x13, 0x6d, 0x24, 0x3a, 0x4d,
      0x6c, 0x42, 0x6f, 0xc6, 0x97, 0x15, 0x63, 0xe8,
      0xa1, 0x4a, 0x55, 0x8e, 0x09, 0x64, 0x16, 0x19,
      0xbf, 0x03, 0xfc, 0xaf, 0x90, 0xb1, 0xfc, 0x2d },
    { 0xba, 0x8a, 0x25, 0x9a, 0x47, 0x1b, 0x78, 0x7d,
      0xd5, 0xd5, 0x40, 0xec, 0x25, 0xd4, 0x3d, 0x87,
      0x20, 0x0f, 0xda, 0xdc, 0x6d, 0x1f, 0x05, 0xd9,
      0x16, 0x58, 0x4f, 0xa9, 0xf6, 0xcb, 0xf5, 0x12 }
};

static const unsigned char kwp_key[KW_TESTS][32] = {
    { 0x78, 0x65, 0xe2, 0x0f, 0x3c, 0x21, 0x65, 0x9a,
      0xb4, 0x69, 0x0b, 0x62, 0x9c, 0xdf, 0x3c, 0xc4 },
    { 0xf5, 0xf8, 0x96, 0xa3, 0xbd, 0x2f, 0x4a, 0x98,
      0x23, 0xef, 0x16, 0x2b, 0x00, 0xb8, 0x05, 0xd7,
      0xde, 0x1e, 0xa4, 0x66, 0x26, 0x96, 0xa2, 0x58 },
    { 0x95, 0xda, 0x27, 0x00, 0xca, 0x6f, 0xd9, 0xa5,
      0x25, 0x54, 0xee, 0x2a, 0x8d, 0xf1, 0x38, 0x6f,
      0x5b, 0x94, 0xa1, 0xa6, 0x0e, 0xd8, 0xa4, 0xae,
      0xf6, 0x0a, 0x8d, 0x61, 0xab, 0x5f, 0x22, 0x5a }
};

static const unsigned char kwp_msg[KW_TESTS][31] = {
    { 0xbd, 0x68, 0x43, 0xd4, 0x20, 0x37, 0x8d, 0xc8,
      0x96 },
    { 0x6c, 0xcd, 0xd5, 0x85, 0x18, 0x40, 0x97, 0xeb,
      0xd5, 0xc3, 0xaf, 0x3e, 0x47, 0xd0, 0x2c, 0x19,
      0x14, 0x7b, 0x4d, 0x99, 0x5f, 0x96, 0x43, 0x66,
      0x91, 0x56, 0x75, 0x8c, 0x13, 0x16, 0x8f },
    { 0xd1 }
};
static const size_t kwp_msg_len[KW_TESTS] = { 9, 31, 1 };

static const unsigned char kwp_res[KW_TESTS][48] = {
    { 0x41, 0xec, 0xa9, 0x56, 0xd4, 0xaa, 0x04, 0x7e,
      0xb5, 0xcf, 0x4e, 0xfe, 0x65, 0x96, 0x61, 0xe7,
      0x4d, 0xb6, 0xf8, 0xc5, 0x64, 0xe2, 0x35, 0x00 },
    { 0x4e, 0x9b, 0xc2, 0xbc, 0xbc, 0x6c, 0x1e, 0x13,
      0xd3, 0x35, 0xbc, 0xc0, 0xf7, 0x73, 0x6a, 0x88,
      0xfa, 0x87, 0x53, 0x66, 0x15, 0xbb, 0x8e, 0x63,
      0x8b, 0xcc, 0x81, 0x66, 0x84, 0x68, 0x17, 0x90,
      0x67, 0xcf, 0xa9, 0x8a, 0x9d, 0x0e, 0x33, 0x26 },
    { 0x06, 0xba, 0x7a, 0xe6, 0xf3, 0x24, 0x8c, 0xfd,
      0xcf, 0x26, 0x75, 0x07, 0xfa, 0x00, 0x1b, 0xc4  }
};
static const size_t kwp_out_len[KW_TESTS] = { 24, 40, 16 };

int mbedtls_nist_kw_test( int verbose )
{
    mbedtls_nist_kw_context ctx;
    unsigned char out[48];
    size_t olen;
    int i;
    int ret = 0;
    mbedtls_nist_kw_init( &ctx );

    for( i = 0; i < KW_TESTS; i++ )
    {
        if( verbose != 0 )
            mbedtls_printf( "  NIST_KW-TEST-AES-%u: start\n", (unsigned int) key_len[i] * 8 );

        ret = mbedtls_nist_kw_setkey( &ctx, MBEDTLS_CIPHER_ID_AES,
                                      kw_key[i], key_len[i] * 8, 1 );
        if( ret != 0 )
        {
            if( verbose != 0 )
                mbedtls_printf( "  NIST_KW-TEST-AES-%u: failed\n", (unsigned int) key_len[i] * 8 );

            goto end;
        }

        ret = mbedtls_nist_kw_wrap( &ctx, MBEDTLS_KW_MODE_KW, kw_msg[i],
                                    kw_msg_len[i], out, &olen, sizeof( out ) );
        if( ret != 0 || kw_out_len[i] != olen ||
            memcmp( out, kw_res[i], kw_out_len[i] ) != 0 )
        {
            if( verbose != 0 )
                mbedtls_printf( "  NIST_KW-TEST-AES-%u: failed\n", (unsigned int) key_len[i] * 8 );

            ret = 1;
            goto end;
        }

        if( ( ret = mbedtls_nist_kw_setkey( &ctx, MBEDTLS_CIPHER_ID_AES,
                                            kw_key[i], key_len[i] * 8, 0 ) )
              != 0 )
        {
            if( verbose != 0 )
                mbedtls_printf( "  NIST_KW-TEST-AES-%u: failed\n", (unsigned int) key_len[i] * 8 );

            goto end;
        }

        ret = mbedtls_nist_kw_unwrap( &ctx, MBEDTLS_KW_MODE_KW,
                                      out, olen, out, &olen, sizeof( out ) );

        if( ret != 0 || olen != kw_msg_len[i] ||
            memcmp( out, kw_msg[i], kw_msg_len[i] ) != 0 )
        {
            if( verbose != 0 )
                mbedtls_printf( "  NIST_KW-TEST-AES-%u: failed\n", (unsigned int) key_len[i] * 8 );

            ret = 1;
            goto end;
        }

        if( verbose != 0 )
            mbedtls_printf( "  NIST_KW-TEST-AES-%u: passed\n", (unsigned int) key_len[i] * 8 );
    }

    for( i = 0; i < KW_TESTS; i++ )
    {
        olen = sizeof( out );
        if( verbose != 0 )
            mbedtls_printf( "  NIST_KWP-TEST-AES-%u: start\n", (unsigned int) key_len[i] * 8 );

        ret = mbedtls_nist_kw_setkey( &ctx, MBEDTLS_CIPHER_ID_AES, kwp_key[i],
                                      key_len[i] * 8, 1 );
        if( ret  != 0 )
        {
            if( verbose != 0 )
                mbedtls_printf( "  NIST_KWP-TEST-AES-%u: failed\n", (unsigned int) key_len[i] * 8 );

            goto end;
        }
        ret = mbedtls_nist_kw_wrap( &ctx, MBEDTLS_KW_MODE_KWP, kwp_msg[i],
                                    kwp_msg_len[i], out, &olen, sizeof( out ) );

        if( ret != 0 || kwp_out_len[i] != olen ||
            memcmp( out, kwp_res[i], kwp_out_len[i] ) != 0 )
        {
            if( verbose != 0 )
                mbedtls_printf( "  NIST_KWP-TEST-AES-%u: failed\n", (unsigned int) key_len[i] * 8 );

            ret = 1;
            goto end;
        }

        if( ( ret = mbedtls_nist_kw_setkey( &ctx, MBEDTLS_CIPHER_ID_AES,
                                            kwp_key[i], key_len[i] * 8, 0 ) )
              != 0 )
        {
            if( verbose != 0 )
                mbedtls_printf( "  NIST_KWP-TEST-AES-%u: failed\n", (unsigned int) key_len[i] * 8 );

            goto end;
        }

        ret = mbedtls_nist_kw_unwrap(  &ctx, MBEDTLS_KW_MODE_KWP, out,
                                       olen, out, &olen, sizeof( out ) );

        if( ret != 0 || olen != kwp_msg_len[i] ||
            memcmp( out, kwp_msg[i], kwp_msg_len[i] ) != 0 )
        {
            if( verbose != 0 )
                mbedtls_printf( "  NIST_KWP-TEST-AES-%u: failed\n", (unsigned int) key_len[i] * 8 );

            ret = 1;
            goto end;
        }

        if( verbose != 0 )
            mbedtls_printf( "  NIST_KWP-TEST-AES-%u: passed\n", (unsigned int) key_len[i] * 8 );
    }
end:
    mbedtls_nist_kw_free( &ctx );

    return( ret );
}

#endif /* MBEDTLS_NIST_KW_C */
