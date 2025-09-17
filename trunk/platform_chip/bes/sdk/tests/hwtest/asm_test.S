/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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

#ifdef CHIP_HAS_A7_DSP

#if defined(A7_TEST) || defined(A7_DSP_TEST)

#define TO_STRING_A(s)                  # s
#define TO_STRING(s)                    TO_STRING_A(s)

.globl __dsp_code_start
.globl __dsp_code_end
.section .rodata_dsp_code, "a", %progbits
.balign 4

__dsp_code_start:
	.incbin TO_STRING(DSP_BIN_NAME)
__dsp_code_end:

#endif

#endif


#ifdef AES_LARGE_DATA

.globl __aes_plain_data_start
.globl __aes_plain_data_end
.section .rodata_aes_plain_data, "a", %progbits
.balign 4

__aes_plain_data_start:
    .incbin "aes_plain_data"
__aes_plain_data_end:

.globl __aes_cipher_data_start
.globl __aes_cipher_data_end
.section .rodata_aes_cipher_data, "a", %progbits
.balign 4

__aes_cipher_data_start:
    .incbin "aes_cipher_data"
__aes_cipher_data_end:

#endif

#if defined(HASH_LARGE_DATA) || defined(HMAC_LARGE_DATA)

.globl __hash_msg_data_start
.globl __hash_msg_data_end
.section .rodata_hash_msg_data, "a", %progbits
.balign 4

__hash_msg_data_start:
    .incbin "hash_msg_data"
__hash_msg_data_end:

#ifdef HASH_LARGE_DATA
.globl __hash_digest_data_start
.globl __hash_digest_data_end
.section .rodata_hash_digest_data, "a", %progbits
.balign 4

__hash_digest_data_start:
    .incbin "hash_digest_data"
__hash_digest_data_end:
#endif

#ifdef HMAC_LARGE_DATA
.globl __hmac_digest_data_start
.globl __hmac_digest_data_end
.section .rodata_hmac_digest_data, "a", %progbits
.balign 4

__hmac_digest_data_start:
    .incbin "hmac_digest_data"
__hmac_digest_data_end:
#endif

#endif

