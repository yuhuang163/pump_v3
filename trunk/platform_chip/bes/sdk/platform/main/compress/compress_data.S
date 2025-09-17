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

#define TO_STRING_A(s)                  # s
#define TO_STRING(s)                    TO_STRING_A(s)

#ifdef COMPRESS_SRAM_CODE

.globl __lzma_mcu_sram_bin_start
.globl __lzma_mcu_sram_bin_end
.section .mcu_lzma_code, "a", %progbits
.balign 4

__lzma_mcu_sram_bin_start:
    .incbin TO_STRING(COMPRESSED_BIN_NAME_SRAM)
__lzma_mcu_sram_bin_end:


.globl __lzma_mcu_fsram_bin_start
.globl __lzma_mcu_fsram_bin_end
.section .mcu_lzma_code, "a", %progbits
.balign 4

__lzma_mcu_fsram_bin_start:
    .incbin TO_STRING(COMPRESSED_BIN_NAME_FSRAM)
__lzma_mcu_fsram_bin_end:


.globl __lzma_mcu_cptext_bin_start
.globl __lzma_mcu_cptext_bin_end
.section .mcu_lzma_code, "a", %progbits
.balign 4

__lzma_mcu_cptext_bin_start:
    .incbin TO_STRING(COMPRESSED_BIN_NAME_CPSRAM)
__lzma_mcu_cptext_bin_end:

.globl __lzma_mcu_cpdata_bin_start
.globl __lzma_mcu_cpdata_bin_end
.section .mcu_lzma_code, "a", %progbits
.balign 4

__lzma_mcu_cpdata_bin_start:
    .incbin TO_STRING(COMPRESSED_BIN_NAME_CPSRAM_DATA)
__lzma_mcu_cpdata_bin_end:
#endif


#ifdef COMPRESS_PSRAM_CODE

.globl __lzma_mcu_psram_bin_start
.globl __lzma_mcu_psram_bin_end
.section .mcu_lzma_code, "a", %progbits
.balign 4

__lzma_mcu_psram_bin_start:
    .incbin TO_STRING(COMPRESSED_BIN_NAME_PSRAM)
__lzma_mcu_psram_bin_end:

#endif
