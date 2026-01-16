/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#ifdef VGLITE_DUMP_PNG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vg_lite.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "heap_api.h"

#include "lodepng.h"

vg_lite_error_t vg_lite_save_png(vg_lite_buffer_t *src, uint8_t *png_data, uint32_t *png_size)
{
    uint8_t *out;
    if (lodepng_encode_memory(&out, png_size, src->memory, src->width, src->height, LCT_RGBA, 8) != 0) {
        TRACE(0, "dump to png failed");
        return VG_LITE_INVALID_ARGUMENT;
    }

    memcpy(png_data, out, *buf_size);
    extern void lodepng_free(void *);
    lodepng_free(out);
    TRACE(0, "dump to png success");
    /* Success. */
    return VG_LITE_SUCCESS;
}
#endif
