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
#ifndef __SHANHAI_MBEDTLS_TEST__

void mbedtls_shanhai_init(void);

int shanhai_aes_test(int verbose);

int shanhai_sha1_test_1(int verbose);

int shanhai_sha1_test_2(int verbose);

int shanhai_sha256_test_1(int verbose);

int shanhai_sha256_test_2(int verbose);

int shanhai_sha512_test_1(int verbose);

int shanhai_sha512_test_2(int verbose);

int shanhai_md5_test(int verbose);

int shanhai_hmac_test(int verbose);

int shanhai_ccm_gcm_test(int verbose);

int shanhai_bignum_test(int verbose);

int shanhai_trng_test(int verbose);

int shanhai_rsa_test(int verbose);

int shanhai_ecdh_test(int verbose);

int shanhai_ecdsa_test(int verbose);

#endif
