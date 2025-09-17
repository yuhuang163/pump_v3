/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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
#include "resource_conf.h"

#ifdef GPU_TST_WITH_NV12_RAWFILE
#define TO_STRING_A(s)                  # s
#define TO_STRING(s)                    TO_STRING_A(s)

.globl __start_flash_nv12_raw
.globl __end_flash_nv12_raw
.section .rodata, "a", %progbits
.balign 4

__start_flash_nv12_raw:
	.incbin TO_STRING(nv12.bin)
__end_flash_nv12_raw:

#endif