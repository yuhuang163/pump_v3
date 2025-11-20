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
#ifndef MBEDTLS_PORTS_CONFIG_H
#define MBEDTLS_PORTS_CONFIG_H

#define MBEDTLS_AES_ALT
#define MBEDTLS_MD5_C
#define MBEDTLS_MD5_ALT
#define MBEDTLS_SHA1_ALT
#define MBEDTLS_SHA256_ALT
#define MBEDTLS_SHA512_C
#define MBEDTLS_SHA512_ALT
#define MBEDTLS_SHA384_C

#define MBEDTLS_GCM_C
#define MBEDTLS_AES_C
#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CIPHER_MODE_OFB
#define MBEDTLS_CIPHER_MODE_CFB
#define MBEDTLS_CIPHER_MODE_CTR
#define MBEDTLS_CIPHER_MODE_XTS

#define MBEDTLS_RSA_C
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_OID_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_GENPRIME

#define MBEDTLS_CCM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_MD_C
//#define MBEDTLS_NET_C

#define MBEDTLS_SHA1_C
#define MBEDTLS_SHA224_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_TLS_C

#define MBEDTLS_BASE64_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_CRL_PARSE_C

#define MBEDTLS_CMAC_C
#define MBEDTLS_NIST_KW_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_PKCS5_C

#define MBEDTLS_AES_ROM_TABLES

#define MBEDTLS_PSK_MAX_LEN    16 /* 128-bits keys are generally enough */

#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_NO_PLATFORM_ENTROPY
//#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_ENTROPY_MAX_SOURCES 2

#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_HANDSHAKE_WITH_CERT_ENABLED
#define MBEDTLS_SSL_CIPHERSUITES                        \
        MBEDTLS_TLS_PSK_WITH_AES_256_CCM_8,             \
        MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8

#define MBEDTLS_SSL_IN_CONTENT_LEN              1024
#define MBEDTLS_SSL_OUT_CONTENT_LEN             1024

//#define MBEDTLS_THREADING_ALT
//#define MBEDTLS_THREADING_C

#endif /* MBEDTLS_PORTS_CONFIG_H */

