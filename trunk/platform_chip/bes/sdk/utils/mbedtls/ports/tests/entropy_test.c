/*
 *  Entropy accumulator implementation
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

#include "hal_location.h"
#include "common.h"

#if defined(MBEDTLS_ENTROPY_C)

#include "mbedtls/entropy.h"
#include "entropy_poll.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/error.h"

#include <string.h>

#if defined(MBEDTLS_FS_IO)
#include <stdio.h>
#endif

#if defined(MBEDTLS_ENTROPY_NV_SEED)
#include "mbedtls/platform.h"
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf     printf
#endif /* MBEDTLS_PLATFORM_C */

/*
 * Dummy source function
 */
static int entropy_dummy_source( void *data, unsigned char *output,
                                 size_t len, size_t *olen )
{
    ((void) data);

    memset( output, 0x2a, len );
    *olen = len;

    return( 0 );
}

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)

static int mbedtls_entropy_source_test_gather( unsigned char *buf, size_t buf_len )
{
    int ret = 0;
    size_t entropy_len = 0;
    size_t olen = 0;
    size_t attempts = buf_len;

    while( attempts > 0 && entropy_len < buf_len )
    {
        if( ( ret = mbedtls_hardware_poll( NULL, buf + entropy_len,
            buf_len - entropy_len, &olen ) ) != 0 )
            return( ret );

        entropy_len += olen;
        attempts--;
    }

    if( entropy_len < buf_len )
    {
        ret = 1;
    }

    return( ret );
}


static int mbedtls_entropy_source_test_check_bits( const unsigned char *buf,
                                                        size_t buf_len )
{
    unsigned char set= 0xFF;
    unsigned char unset = 0x00;
    size_t i;

    for( i = 0; i < buf_len; i++ )
    {
        set &= buf[i];
        unset |= buf[i];
    }

    return( set == 0xFF || unset == 0x00 );
}

/*
 * A test to ensure hat the entropy sources are functioning correctly
 * and there is no obvious failure. The test performs the following checks:
 *  - The entropy source is not providing only 0s (all bits unset) or 1s (all
 *    bits set).
 *  - The entropy source is not providing values in a pattern. Because the
 *    hardware could be providing data in an arbitrary length, this check polls
 *    the hardware entropy source twice and compares the result to ensure they
 *    are not equal.
 *  - The error code returned by the entropy source is not an error.
 */
int mbedtls_entropy_source_test( int verbose )
{
    int ret = 0;
    unsigned char buf0[2 * sizeof( unsigned long long int )];
    unsigned char buf1[2 * sizeof( unsigned long long int )];

    if( verbose != 0 )
        mbedtls_printf( "  ENTROPY_BIAS-TEST: start\n" );

    memset( buf0, 0x00, sizeof( buf0 ) );
    memset( buf1, 0x00, sizeof( buf1 ) );

    if( ( ret = mbedtls_entropy_source_test_gather( buf0, sizeof( buf0 ) ) ) != 0 )
        goto cleanup;
    if( ( ret = mbedtls_entropy_source_test_gather( buf1, sizeof( buf1 ) ) ) != 0 )
        goto cleanup;

    /* Make sure that the returned values are not all 0 or 1 */
    if( ( ret = mbedtls_entropy_source_test_check_bits( buf0, sizeof( buf0 ) ) ) != 0 )
        goto cleanup;
    if( ( ret = mbedtls_entropy_source_test_check_bits( buf1, sizeof( buf1 ) ) ) != 0 )
        goto cleanup;

    /* Make sure that the entropy source is not returning values in a
     * pattern */
    ret = memcmp( buf0, buf1, sizeof( buf0 ) ) == 0;

cleanup:
    if( verbose != 0 )
    {
        if( ret != 0 )
            mbedtls_printf( "  ENTROPY_BIAS-TEST: failed\n" );
        else
            mbedtls_printf( "  ENTROPY_BIAS-TEST: passed\n" );
    }

    return( ret != 0 );
}

#endif /* MBEDTLS_ENTROPY_HARDWARE_ALT */

static PSRAM_BSS_LOC unsigned char test_buf[MBEDTLS_ENTROPY_BLOCK_SIZE];
static PSRAM_BSS_LOC unsigned char test_acc[MBEDTLS_ENTROPY_BLOCK_SIZE];

/*
 * The actual entropy quality is hard to test, but we can at least
 * test that the functions don't cause errors and write the correct
 * amount of data to buffers.
 */
int mbedtls_entropy_test( int verbose )
{
    int ret = 1;
    mbedtls_entropy_context ctx;
    size_t i, j;

    if( verbose != 0 )
        mbedtls_printf( "  ENTROPY-TEST: start\n" );

    mbedtls_entropy_init( &ctx );

    /* First do a gather to make sure we have default sources */
    if( ( ret = mbedtls_entropy_gather( &ctx ) ) != 0 )
        goto cleanup;

    ret = mbedtls_entropy_add_source( &ctx, entropy_dummy_source, NULL, 16,
                                      MBEDTLS_ENTROPY_SOURCE_WEAK );
    if( ret != 0 )
        goto cleanup;

    if( ( ret = mbedtls_entropy_update_manual( &ctx, test_buf, sizeof test_buf ) ) != 0 )
        goto cleanup;

    /*
     * To test that mbedtls_entropy_func writes correct number of bytes:
     * - use the whole buffer and rely on ASan to detect overruns
     * - collect entropy 8 times and OR the result in an accumulator:
     *   any byte should then be 0 with probably 2^(-64), so requiring
     *   each of the 32 or 64 bytes to be non-zero has a false failure rate
     *   of at most 2^(-58) which is acceptable.
     */
    for( i = 0; i < 8; i++ )
    {
        if( ( ret = mbedtls_entropy_func( &ctx, test_buf, sizeof( test_buf ) ) ) != 0 )
            goto cleanup;

        for( j = 0; j < sizeof( test_buf ); j++ )
            test_acc[j] |= test_buf[j];
    }

    for( j = 0; j < sizeof( test_buf ); j++ )
    {
        if( test_acc[j] == 0 )
        {
            ret = 1;
            goto cleanup;
        }
    }

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)
    if( ( ret = mbedtls_entropy_source_test( 0 ) ) != 0 )
        goto cleanup;
#endif

cleanup:
    mbedtls_entropy_free( &ctx );

    if( verbose != 0 )
    {
        if( ret != 0 )
            mbedtls_printf( "  ENTROPY-TEST: failed\n" );
        else
            mbedtls_printf( "  ENTROPY-TEST: passed\n" );
    }

    return( ret != 0 );
}

#endif /* MBEDTLS_ENTROPY_C */
