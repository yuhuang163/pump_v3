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

#ifndef MBEDTLS_AES_ALT_H
#define MBEDTLS_AES_ALT_H

typedef struct mbedtls_aes_context
{
    unsigned char MBEDTLS_PRIVATE(buf)[256];
}
mbedtls_aes_context;

typedef struct mbedtls_aes_xts_context
{
    mbedtls_aes_context crypt;
    mbedtls_aes_context tweak;
}
mbedtls_aes_xts_context;

#endif /* MBEDTLS_AES_ALT_H */
