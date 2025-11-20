/*
 *  HMAC_DRBG implementation (NIST SP 800-90)
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
 *  The NIST SP 800-90A DRBGs are described in the following publication.
 *  http://csrc.nist.gov/publications/nistpubs/800-90A/SP800-90A.pdf
 *  References below are based on rev. 1 (January 2012).
 */

#include "hal_location.h"
#include "common.h"

#if defined(MBEDTLS_HMAC_DRBG_C)

#include "mbedtls/hmac_drbg.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/error.h"

#include <string.h>

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif /* MBEDTLS_PLATFORM_C */

#if !defined(MBEDTLS_SHA1_C)
/* Dummy checkup routine */
int mbedtls_hmac_drbg_test( int verbose )
{
    (void) verbose;
    return( 0 );
}
#else

#define OUTPUT_LEN  80

/* From a NIST PR=true test vector */
static const unsigned char entropy_pr[] = {
    0xa0, 0xc9, 0xab, 0x58, 0xf1, 0xe2, 0xe5, 0xa4, 0xde, 0x3e, 0xbd, 0x4f,
    0xf7, 0x3e, 0x9c, 0x5b, 0x64, 0xef, 0xd8, 0xca, 0x02, 0x8c, 0xf8, 0x11,
    0x48, 0xa5, 0x84, 0xfe, 0x69, 0xab, 0x5a, 0xee, 0x42, 0xaa, 0x4d, 0x42,
    0x17, 0x60, 0x99, 0xd4, 0x5e, 0x13, 0x97, 0xdc, 0x40, 0x4d, 0x86, 0xa3,
    0x7b, 0xf5, 0x59, 0x54, 0x75, 0x69, 0x51, 0xe4 };
static const unsigned char result_pr[OUTPUT_LEN] = {
    0x9a, 0x00, 0xa2, 0xd0, 0x0e, 0xd5, 0x9b, 0xfe, 0x31, 0xec, 0xb1, 0x39,
    0x9b, 0x60, 0x81, 0x48, 0xd1, 0x96, 0x9d, 0x25, 0x0d, 0x3c, 0x1e, 0x94,
    0x10, 0x10, 0x98, 0x12, 0x93, 0x25, 0xca, 0xb8, 0xfc, 0xcc, 0x2d, 0x54,
    0x73, 0x19, 0x70, 0xc0, 0x10, 0x7a, 0xa4, 0x89, 0x25, 0x19, 0x95, 0x5e,
    0x4b, 0xc6, 0x00, 0x1d, 0x7f, 0x4e, 0x6a, 0x2b, 0xf8, 0xa3, 0x01, 0xab,
    0x46, 0x05, 0x5c, 0x09, 0xa6, 0x71, 0x88, 0xf1, 0xa7, 0x40, 0xee, 0xf3,
    0xe1, 0x5c, 0x02, 0x9b, 0x44, 0xaf, 0x03, 0x44 };

/* From a NIST PR=false test vector */
static const unsigned char entropy_nopr[] = {
    0x79, 0x34, 0x9b, 0xbf, 0x7c, 0xdd, 0xa5, 0x79, 0x95, 0x57, 0x86, 0x66,
    0x21, 0xc9, 0x13, 0x83, 0x11, 0x46, 0x73, 0x3a, 0xbf, 0x8c, 0x35, 0xc8,
    0xc7, 0x21, 0x5b, 0x5b, 0x96, 0xc4, 0x8e, 0x9b, 0x33, 0x8c, 0x74, 0xe3,
    0xe9, 0x9d, 0xfe, 0xdf };
static const unsigned char result_nopr[OUTPUT_LEN] = {
    0xc6, 0xa1, 0x6a, 0xb8, 0xd4, 0x20, 0x70, 0x6f, 0x0f, 0x34, 0xab, 0x7f,
    0xec, 0x5a, 0xdc, 0xa9, 0xd8, 0xca, 0x3a, 0x13, 0x3e, 0x15, 0x9c, 0xa6,
    0xac, 0x43, 0xc6, 0xf8, 0xa2, 0xbe, 0x22, 0x83, 0x4a, 0x4c, 0x0a, 0x0a,
    0xff, 0xb1, 0x0d, 0x71, 0x94, 0xf1, 0xc1, 0xa5, 0xcf, 0x73, 0x22, 0xec,
    0x1a, 0xe0, 0x96, 0x4e, 0xd4, 0xbf, 0x12, 0x27, 0x46, 0xe0, 0x87, 0xfd,
    0xb5, 0xb3, 0xe9, 0x1b, 0x34, 0x93, 0xd5, 0xbb, 0x98, 0xfa, 0xed, 0x49,
    0xe8, 0x5f, 0x13, 0x0f, 0xc8, 0xa4, 0x59, 0xb7 };

/* "Entropy" from buffer */
static size_t test_offset;
static int hmac_drbg_test_entropy( void *data,
                                        unsigned char *buf, size_t len )
{
    const unsigned char *p = data;
    memcpy( buf, p + test_offset, len );
    test_offset += len;
    return( 0 );
}

#define CHK( c )    if( (c) != 0 )                          \
                    {                                       \
                        if( verbose != 0 )                  \
                            mbedtls_printf( "  HMAC_DRBG-TEST: failed\n" );  \
                        return( 1 );                        \
                    }

static PSRAM_BSS_LOC unsigned char test_buf[OUTPUT_LEN];

/*
 * Checkup routine for HMAC_DRBG with SHA-1
 */
int mbedtls_hmac_drbg_test( int verbose )
{
    mbedtls_hmac_drbg_context ctx;
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type( MBEDTLS_MD_SHA1 );

    mbedtls_hmac_drbg_init( &ctx );

    /*
     * PR = True
     */
    if( verbose != 0 )
        mbedtls_printf( "  HMAC_DRBG-TEST (PR = True): start\n" );

    test_offset = 0;
    CHK( mbedtls_hmac_drbg_seed( &ctx, md_info,
                         hmac_drbg_test_entropy, (void *) entropy_pr,
                         NULL, 0 ) );
    mbedtls_hmac_drbg_set_prediction_resistance( &ctx, MBEDTLS_HMAC_DRBG_PR_ON );
    CHK( mbedtls_hmac_drbg_random( &ctx, test_buf, OUTPUT_LEN ) );
    CHK( mbedtls_hmac_drbg_random( &ctx, test_buf, OUTPUT_LEN ) );
    CHK( memcmp( test_buf, result_pr, OUTPUT_LEN ) );
    mbedtls_hmac_drbg_free( &ctx );

    mbedtls_hmac_drbg_free( &ctx );

    if( verbose != 0 )
        mbedtls_printf( "  HMAC_DRBG-TEST (PR = True): passed\n" );

    /*
     * PR = False
     */
    if( verbose != 0 )
        mbedtls_printf( "  HMAC_DRBG-TEST (PR = False): start\n" );

    mbedtls_hmac_drbg_init( &ctx );

    test_offset = 0;
    CHK( mbedtls_hmac_drbg_seed( &ctx, md_info,
                         hmac_drbg_test_entropy, (void *) entropy_nopr,
                         NULL, 0 ) );
    CHK( mbedtls_hmac_drbg_reseed( &ctx, NULL, 0 ) );
    CHK( mbedtls_hmac_drbg_random( &ctx, test_buf, OUTPUT_LEN ) );
    CHK( mbedtls_hmac_drbg_random( &ctx, test_buf, OUTPUT_LEN ) );
    CHK( memcmp( test_buf, result_nopr, OUTPUT_LEN ) );
    mbedtls_hmac_drbg_free( &ctx );

    mbedtls_hmac_drbg_free( &ctx );

    if( verbose != 0 )
        mbedtls_printf( "  HMAC_DRBG-TEST (PR = False): passed\n" );

    return( 0 );
}
#endif /* MBEDTLS_SHA1_C */

#endif /* MBEDTLS_HMAC_DRBG_C */
