#include "hal_location.h"
#include "plat_types.h"
#include "common.h"

#if defined(MBEDTLS_X509_CRT_PARSE_C)

#include "mbedtls/bignum.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/x509_crl.h"
#include "mbedtls/x509_csr.h"
#include "mbedtls/pem.h"
#include "mbedtls/oid.h"
#include "mbedtls/base64.h"
#include "mbedtls/error.h"
#include "string.h"

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#define mbedtls_snprintf snprintf
#endif /* MBEDTLS_PLATFORM_C */

#if MBEDTLS_X509_MAX_INTERMEDIATE_CA > 19
#error "The value of MBEDTLS_X509_MAX_INTERMEDIATE_C is larger \
than the current threshold 19. To test larger values, please \
adapt the script tests/data_files/dir-max/long.sh."
#endif

/* Test-only profile allowing all digests, PK algorithms, and curves. */
POSSIBLY_UNUSED
static const mbedtls_x509_crt_profile profile_all =
{
    0xFFFFFFFF, /* Any MD        */
    0xFFFFFFFF, /* Any PK alg    */
    0xFFFFFFFF, /* Any curve     */
    1024,
};

/* Profile for backward compatibility. Allows SHA-1, unlike the default
   profile. */
POSSIBLY_UNUSED
static const mbedtls_x509_crt_profile compat_profile =
{
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA1 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_RIPEMD160 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA224 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA256 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA384 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA512 ),
    0xFFFFFFF, /* Any PK alg    */
    0xFFFFFFF, /* Any curve     */
    1024,
};

POSSIBLY_UNUSED
static const mbedtls_x509_crt_profile profile_rsa3072 =
{
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA256 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA384 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA512 ),
    MBEDTLS_X509_ID_FLAG( MBEDTLS_PK_RSA ),
    0,
    3072,
};

POSSIBLY_UNUSED
static const mbedtls_x509_crt_profile profile_sha512 =
{
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA512 ),
    0xFFFFFFF, /* Any PK alg    */
    0xFFFFFFF, /* Any curve     */
    1024,
};

POSSIBLY_UNUSED
static int verify_none( void *data, mbedtls_x509_crt *crt, int certificate_depth, uint32_t *flags )
{
    ((void) data);
    ((void) crt);
    ((void) certificate_depth);
    *flags |= MBEDTLS_X509_BADCERT_OTHER;

    return 0;
}

POSSIBLY_UNUSED
static int verify_all( void *data, mbedtls_x509_crt *crt, int certificate_depth, uint32_t *flags )
{
    ((void) data);
    ((void) crt);
    ((void) certificate_depth);
    *flags = 0;

    return 0;
}

#if defined(MBEDTLS_X509_TRUSTED_CERTIFICATE_CALLBACK)
POSSIBLY_UNUSED
static int ca_callback_fail( void *data, mbedtls_x509_crt const *child, mbedtls_x509_crt **candidates )
{
    ((void) data);
    ((void) child);
    ((void) candidates);

    return -1;
}

POSSIBLY_UNUSED
static int ca_callback( void *data, mbedtls_x509_crt const *child,
                 mbedtls_x509_crt **candidates )
{
    int ret = 0;
    mbedtls_x509_crt *ca = (mbedtls_x509_crt *) data;
    mbedtls_x509_crt *first;

    /* This is a test-only implementation of the CA callback
     * which always returns the entire list of trusted certificates.
     * Production implementations managing a large number of CAs
     * should use an efficient presentation and lookup for the
     * set of trusted certificates (such as a hashtable) and only
     * return those trusted certificates which satisfy basic
     * parental checks, such as the matching of child `Issuer`
     * and parent `Subject` field. */
    ((void) child);

    first = mbedtls_calloc( 1, sizeof( mbedtls_x509_crt ) );
    if( first == NULL )
    {
        ret = -1;
        goto exit;
    }
    mbedtls_x509_crt_init( first );

    if( mbedtls_x509_crt_parse_der( first, ca->raw.p, ca->raw.len ) != 0 )
    {
        ret = -1;
        goto exit;
    }

    while( ca->next != NULL )
    {
        ca = ca->next;
        if( mbedtls_x509_crt_parse_der( first, ca->raw.p, ca->raw.len ) != 0 )
        {
            ret = -1;
            goto exit;
        }
    }

exit:

    if( ret != 0 )
    {
        mbedtls_x509_crt_free( first );
        mbedtls_free( first );
        first = NULL;
    }

    *candidates = first;
    return( ret );
}
#endif /* MBEDTLS_X509_TRUSTED_CERTIFICATE_CALLBACK */

POSSIBLY_UNUSED
static int verify_fatal( void *data, mbedtls_x509_crt *crt, int certificate_depth, uint32_t *flags )
{
    int *levels = (int *) data;

    ((void) crt);
    ((void) certificate_depth);

    /* Simulate a fatal error in the callback */
    if( *levels & ( 1 << certificate_depth ) )
    {
        *flags |= ( 1 << certificate_depth );
        return( -1 - certificate_depth );
    }

    return( 0 );
}

/* strsep() not available on Windows */
POSSIBLY_UNUSED
static char *mystrsep(char **stringp, const char *delim)
{
    const char *p;
    char *ret = *stringp;

    if( *stringp == NULL )
        return( NULL );

    for( ; ; (*stringp)++ )
    {
        if( **stringp == '\0' )
        {
            *stringp = NULL;
            goto done;
        }

        for( p = delim; *p != '\0'; p++ )
            if( **stringp == *p )
            {
                **stringp = '\0';
                (*stringp)++;
                goto done;
            }
    }

done:
    return( ret );
}

typedef struct {
    char buf[512];
    char *p;
} verify_print_context;

POSSIBLY_UNUSED
static void verify_print_init( verify_print_context *ctx )
{
    memset( ctx, 0, sizeof( verify_print_context ) );
    ctx->p = ctx->buf;
}

POSSIBLY_UNUSED
static int verify_print( void *data, mbedtls_x509_crt *crt, int certificate_depth, uint32_t *flags )
{
    int ret;
    verify_print_context *ctx = (verify_print_context *) data;
    char *p = ctx->p;
    size_t n = ctx->buf + sizeof( ctx->buf ) - ctx->p;
    ((void) flags);

    ret = mbedtls_snprintf( p, n, "depth %d - serial ", certificate_depth );
    MBEDTLS_X509_SAFE_SNPRINTF;

    ret = mbedtls_x509_serial_gets( p, n, &crt->serial );
    MBEDTLS_X509_SAFE_SNPRINTF;

    ret = mbedtls_snprintf( p, n, " - subject " );
    MBEDTLS_X509_SAFE_SNPRINTF;

    ret = mbedtls_x509_dn_gets( p, n, &crt->subject );
    MBEDTLS_X509_SAFE_SNPRINTF;

    ret = mbedtls_snprintf( p, n, " - flags 0x%08x\n", *flags );
    MBEDTLS_X509_SAFE_SNPRINTF;

    ctx->p = p;

    return( 0 );
}

POSSIBLY_UNUSED
static int verify_parse_san( mbedtls_x509_subject_alternative_name *san,
                      char **buf, size_t *size )
{
    int ret;
    size_t i;
    char *p = *buf;
    size_t n = *size;

    ret = mbedtls_snprintf( p, n, "type : %d", san->type );
    MBEDTLS_X509_SAFE_SNPRINTF;

    switch( san->type )
    {
       case( MBEDTLS_X509_SAN_OTHER_NAME ):
           ret = mbedtls_snprintf( p, n, "\notherName :");
           MBEDTLS_X509_SAFE_SNPRINTF;

           if( MBEDTLS_OID_CMP( MBEDTLS_OID_ON_HW_MODULE_NAME,
                    &san->san.other_name.value.hardware_module_name.oid ) != 0 )
           {
               ret = mbedtls_snprintf( p, n, " hardware module name :" );
               MBEDTLS_X509_SAFE_SNPRINTF;
               ret = mbedtls_snprintf( p, n, " hardware type : " );
               MBEDTLS_X509_SAFE_SNPRINTF;

               ret = mbedtls_oid_get_numeric_string( p, n,
                          &san->san.other_name.value.hardware_module_name.oid );
               MBEDTLS_X509_SAFE_SNPRINTF;

               ret = mbedtls_snprintf( p, n, ", hardware serial number : " );
               MBEDTLS_X509_SAFE_SNPRINTF;

               if( san->san.other_name.value.hardware_module_name.val.len >= n )
               {
                   *p = '\0';
                   return( MBEDTLS_ERR_X509_BUFFER_TOO_SMALL );
               }

               for( i=0; i < san->san.other_name.value.hardware_module_name.val.len; i++ )
               {
                   *p++ = san->san.other_name.value.hardware_module_name.val.p[i];
               }
               n -= san->san.other_name.value.hardware_module_name.val.len;
            }
        break;/* MBEDTLS_OID_ON_HW_MODULE_NAME */
        case(  MBEDTLS_X509_SAN_DNS_NAME ):
            ret = mbedtls_snprintf( p, n, "\ndNSName : " );
            MBEDTLS_X509_SAFE_SNPRINTF;
            if( san->san.unstructured_name.len >= n )
            {
                *p = '\0';
                return( MBEDTLS_ERR_X509_BUFFER_TOO_SMALL );
            }
            n -= san->san.unstructured_name.len;
            for( i = 0; i < san->san.unstructured_name.len; i++ )
                *p++ = san->san.unstructured_name.p[i];
        break;/* MBEDTLS_X509_SAN_DNS_NAME */

        default:
        /*
         * Should not happen.
         */
        return( -1 );
    }
    ret = mbedtls_snprintf( p, n, "\n" );
    MBEDTLS_X509_SAFE_SNPRINTF;

    *size = n;
    *buf = p;

    return( 0 );
}

POSSIBLY_UNUSED
static int parse_crt_ext_cb( void *p_ctx, mbedtls_x509_crt const *crt, mbedtls_x509_buf const *oid,
                      int critical, const unsigned char *cp, const unsigned char *end )
{
    ( void ) crt;
    ( void ) critical;
    mbedtls_x509_buf *new_oid = (mbedtls_x509_buf *)p_ctx;
    if( oid->tag == MBEDTLS_ASN1_OID &&
        MBEDTLS_OID_CMP( MBEDTLS_OID_CERTIFICATE_POLICIES, oid ) == 0 )
    {
        /* Handle unknown certificate policy */
        int ret, parse_ret = 0;
        size_t len;
        unsigned char **p = (unsigned char **)&cp;

        /* Get main sequence tag */
        ret = mbedtls_asn1_get_tag( p, end, &len,
                                 MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE );
        if( ret != 0 )
            return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_X509_INVALID_EXTENSIONS, ret ) );

        if( *p + len != end )
            return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_X509_INVALID_EXTENSIONS,
                    MBEDTLS_ERR_ASN1_LENGTH_MISMATCH ) );

        /*
         * Cannot be an empty sequence.
         */
        if( len == 0 )
            return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_X509_INVALID_EXTENSIONS,
                    MBEDTLS_ERR_ASN1_LENGTH_MISMATCH ) );

        while( *p < end )
        {
            const unsigned char *policy_end;

            /*
             * Get the policy sequence
             */
            if( ( ret = mbedtls_asn1_get_tag( p, end, &len,
                    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE ) ) != 0 )
                return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_X509_INVALID_EXTENSIONS, ret ) );

            policy_end = *p + len;

            if( ( ret = mbedtls_asn1_get_tag( p, policy_end, &len,
                                              MBEDTLS_ASN1_OID ) ) != 0 )
                return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_X509_INVALID_EXTENSIONS, ret ) );

            /*
             * Recognize exclusively the policy with OID 1
             */
            if( len != 1 || *p[0] != 1 )
                parse_ret = MBEDTLS_ERR_X509_FEATURE_UNAVAILABLE;

            *p += len;

           /*
            * If there is an optional qualifier, then *p < policy_end
            * Check the Qualifier len to verify it doesn't exceed policy_end.
            */
            if( *p < policy_end )
            {
                if( ( ret = mbedtls_asn1_get_tag( p, policy_end, &len,
                         MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE ) ) != 0 )
                    return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_X509_INVALID_EXTENSIONS, ret ) );
                /*
                 * Skip the optional policy qualifiers.
                 */
                *p += len;
            }

            if( *p != policy_end )
                return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_X509_INVALID_EXTENSIONS,
                        MBEDTLS_ERR_ASN1_LENGTH_MISMATCH ) );
        }

        if( *p != end )
            return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_X509_INVALID_EXTENSIONS,
                    MBEDTLS_ERR_ASN1_LENGTH_MISMATCH ) );

        return( parse_ret );
    }
    else if( new_oid != NULL && new_oid->tag == oid->tag && new_oid->len == oid->len &&
             memcmp( new_oid->p, oid->p, oid->len ) == 0 )
        return( 0 );
    else
        return( MBEDTLS_ERROR_ADD( MBEDTLS_ERR_X509_INVALID_EXTENSIONS,
                                   MBEDTLS_ERR_ASN1_UNEXPECTED_TAG ) );
}

static int x509_verify( const char *crt_data, size_t crt_data_size,
                    const char *ca_data, size_t ca_data_size,
                    const char *crl_data, size_t crl_data_size,
                    char *cn_name_str, int result, int flags_result,
                    char *profile_str,
                    char *verify_callback )
{
    int ret = -1;
    mbedtls_x509_crt   crt;
    mbedtls_x509_crt   ca;
    mbedtls_x509_crl    crl;
    uint32_t         flags = 0;
    int (*f_vrfy)(void *, mbedtls_x509_crt *, int, uint32_t *) = NULL;
    char *      cn_name = NULL;
    const mbedtls_x509_crt_profile *profile;

    mbedtls_x509_crt_init( &crt );
    mbedtls_x509_crt_init( &ca );
    mbedtls_x509_crl_init( &crl );

    if( strcmp( cn_name_str, "NULL" ) != 0 )
        cn_name = cn_name_str;

    if( strcmp( profile_str, "" ) == 0 )
        profile = &mbedtls_x509_crt_profile_default;
    else if( strcmp( profile_str, "next" ) == 0 )
        profile = &mbedtls_x509_crt_profile_next;
    else if( strcmp( profile_str, "suite_b" ) == 0 )
        profile = &mbedtls_x509_crt_profile_suiteb;
    else if( strcmp( profile_str, "compat" ) == 0 )
        profile = &compat_profile;
    else if( strcmp( profile_str, "all" ) == 0 )
        profile = &profile_all;
    else
        goto exit;

    if( strcmp( verify_callback, "NULL" ) == 0 )
        f_vrfy = NULL;
    else if( strcmp( verify_callback, "verify_none" ) == 0 )
        f_vrfy = verify_none;
    else if( strcmp( verify_callback, "verify_all" ) == 0 )
        f_vrfy = verify_all;
    else
        goto exit;

    ret = mbedtls_x509_crt_parse( &crt, (const unsigned char *)crt_data, crt_data_size );
    if (ret)
        goto exit;

    ret = mbedtls_x509_crt_parse( &ca, (const unsigned char *)ca_data, ca_data_size );
    if (ret)
        goto exit;

    ret = mbedtls_x509_crl_parse( &crl, (const unsigned char *)crl_data, crl_data_size );
    if (ret)
        goto exit;

    ret = mbedtls_x509_crt_verify_with_profile( &crt, &ca, &crl, profile, cn_name, &flags, f_vrfy, NULL );
    //mbedtls_printf("  ## mbedtls_x509_crt_verify_with_profile() ret = -0x%x flags = %d\n", -ret, flags);
    if (ret != ( result )) {
        ret = -1;
        goto exit;
    }
    if ( flags != (uint32_t)( flags_result ) ) {
        ret = -1;
        goto exit;
    }
    ret = 0;

exit:
    mbedtls_x509_crt_free( &crt );
    mbedtls_x509_crt_free( &ca );
    mbedtls_x509_crl_free( &crl );
    return ret;
}


/****************************************************************************/

#define CERT_MD5_CRT \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDPzCCAiegAwIBAgIBBjANBgkqhkiG9w0BAQQFADA7MQswCQYDVQQGEwJOTDER\n" \
    "MA8GA1UECgwIUG9sYXJTU0wxGTAXBgNVBAMMEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\n" \
    "MDAwMTAxMTIxMjEyWhcNMzAwMTAxMTIxMjEyWjA8MQswCQYDVQQGEwJOTDERMA8G\n" \
    "A1UECgwIUG9sYXJTU0wxGjAYBgNVBAMMEVBvbGFyU1NMIENlcnQgTUQ1MIIBIjAN\n" \
    "BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyHTEzLn5tXnpRdkUYLB9u5Pyax6f\n" \
    "M60Nj4o8VmXl3ETZzGaFB9X4J7BKNdBjngpuG7fa8H6r7gwQk4ZJGDTzqCrSV/Uu\n" \
    "1C93KYRhTYJQj6eVSHD1bk2y1RPD0hrt5kPqQhTrdOrA7R/UV06p86jt0uDBMHEw\n" \
    "MjDV0/YI0FZPRo7yX/k9Z5GIMC5Cst99++UMd//sMcB4j7/Cf8qtbCHWjdmLao5v\n" \
    "4Jv4EFbMs44TFeY0BGbH7vk2DmqV9gmaBmf0ZXH4yqSxJeD+PIs1BGe64E92hfx/\n" \
    "/DZrtenNLQNiTrM9AM+vdqBpVoNq0qjU51Bx5rU2BXcFbXvI5MT9TNUhXwIDAQAB\n" \
    "o00wSzAJBgNVHRMEAjAAMB0GA1UdDgQWBBRxoQBzckAvVHZeM/xSj7zx3WtGITAf\n" \
    "BgNVHSMEGDAWgBS0WuSls97SUva51aaVD+s+vMf9/zANBgkqhkiG9w0BAQQFAAOC\n" \
    "AQEAF4QcMshVtVbYgvvU7f2lWakubbAISM/k+FW/f7u63m0MSSoSFeYflBOC1Wf4\n" \
    "imgDEnWcWTH5V7sxsLNogxfpfTuFUaKfHeQmRhAK4UgqbDEs4dZvgo3wZ/w92G0/\n" \
    "QNntJefnqaFiITXZTn6J8hxeoEq4QbucbWgeY6fTAwXtIv40BvMOSakkuIFAyIvV\n" \
    "90VY1j4vnx0/xv5lIBAxah1HdtXhqtDu/sUfdCtWX5SCcVUwwM3gZ4Q1ZdWQmlvF\n" \
    "737ZG7XaINxsDaI04sJxc7qvuRYhLdCwUPnZL5TGEQJ8jNa/39eEbnkvs7hbTU98\n" \
    "6qG8UAYsSI7aMe1j7DZpkoPL9w==\n" \
    "-----END CERTIFICATE-----"

#define CERT_SERVER1_CRT \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDPzCCAiegAwIBAgIBATANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDER\n" \
    "MA8GA1UECgwIUG9sYXJTU0wxGTAXBgNVBAMMEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\n" \
    "MTkwMjEwMTQ0NDA2WhcNMjkwMjEwMTQ0NDA2WjA8MQswCQYDVQQGEwJOTDERMA8G\n" \
    "A1UECgwIUG9sYXJTU0wxGjAYBgNVBAMMEVBvbGFyU1NMIFNlcnZlciAxMIIBIjAN\n" \
    "BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqQIfPUBq1VVTi/027oJlLhVhXom/\n" \
    "uOhFkNvuiBZS0/FDUEeWEllkh2v9K+BG+XO+3c+S4ZFb7Wagb4kpeUWA0INq1UFD\n" \
    "d185fAkER4KwVzlw7aPsFRkeqDMIR8EFQqn9TMO0390GH00QUUBncxMPQPhtgSVf\n" \
    "CrFTxjB+FTms+Vruf5KepgVb5xOXhbUjktnUJAbVCSWJdQfdphqPPwkZvq1lLGTr\n" \
    "lZvc/kFeF6babFtpzAK6FCwWJJxK3M3Q91Jnc/EtoCP9fvQxyi1wyokLBNsupk9w\n" \
    "bp7OvViJ4lNZnm5akmXiiD8MlBmj3eXonZUT7Snbq3AS3FrKaxerUoJUsQIDAQAB\n" \
    "o00wSzAJBgNVHRMEAjAAMB0GA1UdDgQWBBQfdNY/KcF0dEU7BRIsPai9Q1kCpjAf\n" \
    "BgNVHSMEGDAWgBS0WuSls97SUva51aaVD+s+vMf9/zANBgkqhkiG9w0BAQUFAAOC\n" \
    "AQEAf2k5OiORp60gBNqioC2mIqSXA0CU/qzllt8IvlcMv1V0PAP9f4IEm5mdkERr\n" \
    "UXjnB1Tr3edrsvXLgZ9vEosbFpNMsKfsmBkpjgWG2ui8pdn8cJiws4k4h5fuueSw\n" \
    "Ps1FLK5Tfpi+GJyPqk4ha9Ojp2p9opuA0aIfLuxI+0UzXH4wgrEW/Yydowv959gf\n" \
    "gGSl766CRdUvJbXOeVryFjFTRfLFFNfTvrftZk1dl8tas1nim8xfWet+BZVvq2zY\n" \
    "C7LeCI9nrfuAxfMJTrWFp17y72+hCDk7NEaB2ZLVuAM/ri7LWrr2V2hLFdIAhfC2\n" \
    "nUaulRRpGt/ZTISw6uSIumNoNA==\n" \
    "-----END CERTIFICATE-----"

#define CERT_SHA512_CRT \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDQjCCAiqgAwIBAgIBCzANBgkqhkiG9w0BAQ0FADA7MQswCQYDVQQGEwJOTDER\n" \
    "MA8GA1UECgwIUG9sYXJTU0wxGTAXBgNVBAMMEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\n" \
    "MTkwMjEwMTQ0NDA2WhcNMjkwMjEwMTQ0NDA2WjA/MQswCQYDVQQGEwJOTDERMA8G\n" \
    "A1UECgwIUG9sYXJTU0wxHTAbBgNVBAMMFFBvbGFyU1NMIENlcnQgU0hBNTEyMIIB\n" \
    "IjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqQIfPUBq1VVTi/027oJlLhVh\n" \
    "Xom/uOhFkNvuiBZS0/FDUEeWEllkh2v9K+BG+XO+3c+S4ZFb7Wagb4kpeUWA0INq\n" \
    "1UFDd185fAkER4KwVzlw7aPsFRkeqDMIR8EFQqn9TMO0390GH00QUUBncxMPQPht\n" \
    "gSVfCrFTxjB+FTms+Vruf5KepgVb5xOXhbUjktnUJAbVCSWJdQfdphqPPwkZvq1l\n" \
    "LGTrlZvc/kFeF6babFtpzAK6FCwWJJxK3M3Q91Jnc/EtoCP9fvQxyi1wyokLBNsu\n" \
    "pk9wbp7OvViJ4lNZnm5akmXiiD8MlBmj3eXonZUT7Snbq3AS3FrKaxerUoJUsQID\n" \
    "AQABo00wSzAJBgNVHRMEAjAAMB0GA1UdDgQWBBQfdNY/KcF0dEU7BRIsPai9Q1kC\n" \
    "pjAfBgNVHSMEGDAWgBS0WuSls97SUva51aaVD+s+vMf9/zANBgkqhkiG9w0BAQ0F\n" \
    "AAOCAQEABnuq7gMU6EWqcmEcj2/wiqOFUBeH9ro0tni9JZzaDAKkBMwoeJ3RP/59\n" \
    "wY92UZC/SoWPm0yLK25KTwxJhd645a5ZeRk+yi1SG+oXNgZFS03F1Orat0bM5u94\n" \
    "RtzLRInGzTxhlVS8HPRya2+nEaPT9YNO25vORczPDjtrI82UnysaWiKx1OCPhdP3\n" \
    "ZySAkX/zE1U8Te0+948C0vmg2aTWCSk5zutryFgHH5UojmmWAkBHpX3tIm8JMRG9\n" \
    "tvp6fbIDan0LmSsVK8rq5OPSwAKMso6GF4Iuxou/jP2gI+NutenX26wrffSjlPiW\n" \
    "KksLNj8oL6vIUap28Oh+Gwph02biSQ==\n" \
    "-----END CERTIFICATE-----"

#define CRL_EXPIRED_PEM \
    "-----BEGIN X509 CRL-----\n" \
    "MIIBqzCBlDANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDERMA8GA1UEChMI\n" \
    "UG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EXDTExMDIyMDEwMjQx\n" \
    "OVoXDTExMDIyMDExMjQxOVowKDASAgEBFw0xMTAyMTIxNDQ0MDdaMBICAQMXDTEx\n" \
    "MDIxMjE0NDQwN1owDQYJKoZIhvcNAQEFBQADggEBAKgP1XmCIPbfY1/UO+SVFQir\n" \
    "jArZ94QnQdoan4tJ29d8DmTxJ+z9/KyWNoGeOwc9P/2GQQaZahQOBr0f6lYd67Ct\n" \
    "wFVh/Q2zF8FgRcrQV7u/vJM33Q2yEsQkMGlM7rE5lC972vUKWu/NKq8bN9W/tWxZ\n" \
    "SFbvTXpv024aI0IRudpOCALnIy8SFhVb2/52IN2uR6qrFizDexMEdSckgpHuJzGS\n" \
    "IiANhIMn5LdQYJFjPgBzQU12tDdgzcpxtGhT10y4uQre+UbSjw+iVyml3issw59k\n" \
    "OSmkWFb06LamRC215JAMok3YQO5RnxCR8EjqPcJr+7+O9a1O1++yiaitg4bUjEA=\n" \
    "-----END X509 CRL-----"

#define CRL_FUTURE_PEM \
    "-----BEGIN X509 CRL-----\n" \
    "MIIBgzCCAQoCAQEwCQYHKoZIzj0EATA+MQswCQYDVQQGEwJOTDERMA8GA1UEChMI\n" \
    "UG9sYXJTU0wxHDAaBgNVBAMTE1BvbGFyc3NsIFRlc3QgRUMgQ0EXDTMyMDMxMDEx\n" \
    "MDUxNVoXDTQyMDMwODExMDUxNVowKDASAgEKFw0xMzA5MjQxNjI4MzhaMBICARYX\n" \
    "DTE0MDEyMDEzNDMwNVqgcjBwMG4GA1UdIwRnMGWAFJ1tICRJAT8ry3i1Gbx+JMnb\n" \
    "+zZ8oUKkQDA+MQswCQYDVQQGEwJOTDERMA8GA1UEChMIUG9sYXJTU0wxHDAaBgNV\n" \
    "BAMTE1BvbGFyc3NsIFRlc3QgRUMgQ0GCCQDBQ+J+YkPM6DAJBgcqhkjOPQQBA2gA\n" \
    "MGUCMQCmsvNsOQdbGpmzpeZlKU9lDP6yyWenrI/89swZYogE3cSPob4tOzeYg38i\n" \
    "or91IPgCMD7N/0Qz6Nq2IgBtZORLgsA0ltK+W6AOS+/EIhvGuXV8uguUyYknl4vb\n" \
    "+cE+lWxhCQ==\n" \
    "-----END X509 CRL-----"

#define CRL_PEM \
    "-----BEGIN X509 CRL-----\n" \
    "MIIBqzCBlDANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDERMA8GA1UECgwI\n" \
    "UG9sYXJTU0wxGTAXBgNVBAMMEFBvbGFyU1NMIFRlc3QgQ0EXDTE5MDIwNjE2MzQ0\n" \
    "NloXDTI5MDIwNjE2MzQ0NlowKDASAgEBFw0xMTAyMTIxMjQ0MDdaMBICAQMXDTEx\n" \
    "MDIxMjEyNDQwN1owDQYJKoZIhvcNAQEFBQADggEBAGHkRHGDz2HsoiXzfJDBpkfg\n" \
    "I+yK34O9zoHMyGcDZbF9fj4NVyyNA6XCgY3IgQYe3boA1edO6+8ImjqiZjYK+GWt\n" \
    "4ne7YhapRFAoFbKQY5hgy8mlpSRlNfmRVVCDDKannMac4tQff1LCFHN8msk/uG1b\n" \
    "WHWAsL6dzLVojhbUFY6mZTb6nqjm5YgqcsNsz37n4dyrCDftB99FQdE/aAec2RXP\n" \
    "Jgy9DnY5jMotPqHLZtMyfVNEoivDb7YJA5Vv6NSyiYVTjyWyTHNRsFEXRtHqjpqs\n" \
    "oZdBgLZAAQsUvuVpMbDKQ4FrZjOaOW/xLtKPlh/HNA5p7hNtwIiCAIvp7uQti0w=\n" \
    "-----END X509 CRL-----"

#define TEST_CA_CRT \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDQTCCAimgAwIBAgIBAzANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDER\n" \
    "MA8GA1UECgwIUG9sYXJTU0wxGTAXBgNVBAMMEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\n" \
    "MTkwMjEwMTQ0NDAwWhcNMjkwMjEwMTQ0NDAwWjA7MQswCQYDVQQGEwJOTDERMA8G\n" \
    "A1UECgwIUG9sYXJTU0wxGTAXBgNVBAMMEFBvbGFyU1NMIFRlc3QgQ0EwggEiMA0G\n" \
    "CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDA3zf8F7vglp0/ht6WMn1EpRagzSHx\n" \
    "mdTs6st8GFgIlKXsm8WL3xoemTiZhx57wI053zhdcHgH057Zk+i5clHFzqMwUqny\n" \
    "50BwFMtEonILwuVA+T7lpg6z+exKY8C4KQB0nFc7qKUEkHHxvYPZP9al4jwqj+8n\n" \
    "YMPGn8u67GB9t+aEMr5P+1gmIgNb1LTV+/Xjli5wwOQuvfwu7uJBVcA0Ln0kcmnL\n" \
    "R7EUQIN9Z/SG9jGr8XmksrUuEvmEF/Bibyc+E1ixVA0hmnM3oTDPb5Lc9un8rNsu\n" \
    "KNF+AksjoBXyOGVkCeoMbo4bF6BxyLObyavpw/LPh5aPgAIynplYb6LVAgMBAAGj\n" \
    "UDBOMAwGA1UdEwQFMAMBAf8wHQYDVR0OBBYEFLRa5KWz3tJS9rnVppUP6z68x/3/\n" \
    "MB8GA1UdIwQYMBaAFLRa5KWz3tJS9rnVppUP6z68x/3/MA0GCSqGSIb3DQEBBQUA\n" \
    "A4IBAQB0ZiNRFdia6kskaPnhrqejIRq8YMEGAf2oIPnyZ78xoyERgc35lHGyMtsL\n" \
    "hWicNjP4d/hS9As4j5KA2gdNGi5ETA1X7SowWOGsryivSpMSHVy1+HdfWlsYQOzm\n" \
    "8o+faQNUm8XzPVmttfAVspxeHSxJZ36Oo+QWZ5wZlCIEyjEdLUId+Tm4Bz3B5jRD\n" \
    "zZa/SaqDokq66N2zpbgKKAl3GU2O++fBqP2dSkdQykmTxhLLWRN8FJqhYATyQntZ\n" \
    "0QSi3W9HfSZPnFTcPIXeoiPd2pLlxt1hZu8dws2LTXE63uP6MM4LHvWxiuJaWkP/\n" \
    "mtxyUALj2pQxRitopORFQdn7AOY5\n" \
    "-----END CERTIFICATE-----"


/*
x509_verify:"data_files/cert_md5.crt":"data_files/test-ca.crt":"data_files/crl.pem":"NULL":MBEDTLS_ERR_X509_CERT_VERIFY_FAILED:MBEDTLS_X509_BADCERT_BAD_MD:"compat":"NULL"
x509_verify:"data_files/cert_md5.crt":"data_files/test-ca.crt":"data_files/crl.pem":"NULL":0:0:"all":"NULL"
x509_verify:"data_files/cert_sha512.crt":"data_files/test-ca.crt":"data_files/crl.pem":"NULL":0:0:"compat":"NULL"
x509_verify:"data_files/cert_sha512.crt":"data_files/test-ca.crt":"data_files/crl.pem":"NULL":MBEDTLS_ERR_X509_CERT_VERIFY_FAILED:MBEDTLS_X509_BADCERT_OTHER:"compat":"verify_none"
x509_verify:"data_files/server1.crt":"data_files/test-ca.crt":"data_files/crl_expired.pem":"NULL":MBEDTLS_ERR_X509_CERT_VERIFY_FAILED:MBEDTLS_X509_BADCERT_REVOKED | MBEDTLS_X509_BADCRL_EXPIRED:"compat":"NULL"
*/
int mbedtls_x509_crt_test(int verbose)
{
    int ret;

    ret = x509_verify(CERT_MD5_CRT, strlen(CERT_MD5_CRT)+1,
                      TEST_CA_CRT, strlen(TEST_CA_CRT)+1,
                      CRL_PEM, strlen(CRL_PEM)+1,
                      "NULL", MBEDTLS_ERR_X509_CERT_VERIFY_FAILED, MBEDTLS_X509_BADCERT_BAD_MD,
                      "compat", "NULL");
    if (ret == 0) {
        mbedtls_printf("  X509-CRT-VERIFY-TEST cert md5: passed\n");
    } else {
        mbedtls_printf("  X509-CRT-VERIFY-TEST cert md5: failed, ret=%d\n", ret);
        goto exit;
    }

    ret = x509_verify(CERT_MD5_CRT, strlen(CERT_MD5_CRT)+1,
                      TEST_CA_CRT, strlen(TEST_CA_CRT)+1,
                      CRL_PEM, strlen(CRL_PEM)+1,
                      "NULL", 0, 0, "all", "NULL");
    if (ret == 0) {
        mbedtls_printf("  X509-CRT-VERIFY-TEST cert md5: passed\n");
    } else {
        mbedtls_printf("  X509-CRT-VERIFY-TEST cert md5: failed, ret=%d\n", ret);
        goto exit;
    }

    ret = x509_verify(CERT_SHA512_CRT, strlen(CERT_SHA512_CRT)+1,
                      TEST_CA_CRT, strlen(TEST_CA_CRT)+1,
                      CRL_PEM, strlen(CRL_PEM)+1,
                      "NULL", 0, 0, "compat", "NULL");
    if (ret == 0) {
        mbedtls_printf("  X509-CRT-VERIFY-TEST cert sha512: passed\n");
    } else {
        mbedtls_printf("  X509-CRT-VERIFY-TEST cert sha512: failed, ret=%d\n", ret);
        goto exit;
    }

    ret = x509_verify(CERT_SHA512_CRT, strlen(CERT_SHA512_CRT)+1,
                      TEST_CA_CRT, strlen(TEST_CA_CRT)+1,
                      CRL_PEM, strlen(CRL_PEM)+1,
                      "NULL", MBEDTLS_ERR_X509_CERT_VERIFY_FAILED, MBEDTLS_X509_BADCERT_OTHER,
                      "compat", "verify_none");
    if (ret == 0) {
        mbedtls_printf("  X509-CRT-VERIFY-TEST cert sha512: passed\n");
    } else {
        mbedtls_printf("  X509-CRT-VERIFY-TEST cert sha512: failed, ret=%d\n", ret);
        goto exit;
    }

    ret = x509_verify(CERT_SERVER1_CRT, strlen(CERT_SERVER1_CRT)+1,
                      TEST_CA_CRT, strlen(TEST_CA_CRT)+1,
                      CRL_EXPIRED_PEM, strlen(CRL_EXPIRED_PEM)+1,
                      "NULL", MBEDTLS_ERR_X509_CERT_VERIFY_FAILED,
                      MBEDTLS_X509_BADCERT_REVOKED,
                      "compat", "NULL");
    if (ret == 0) {
        mbedtls_printf("  X509-CRT-VERIFY-TEST cert server1: passed\n");
    } else {
        mbedtls_printf("  X509-CRT-VERIFY-TEST cert server1: failed, ret=%d\n", ret);
        goto exit;
    }

exit:
    return ret;
}



#endif /* MBEDTLS_X509_CRT_PARSE_C */

