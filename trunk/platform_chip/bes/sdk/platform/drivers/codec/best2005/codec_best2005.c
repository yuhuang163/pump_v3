/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "plat_types.h"
#include "codec_int.h"
#include "hal_codec.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "stdbool.h"
#include "string.h"
#include "analog.h"
#include "tgt_hardware.h"

uint32_t codec_int_stream_open(enum AUD_STREAM_T stream)
{
    return 0;
}

uint32_t codec_int_stream_setup(enum AUD_STREAM_T stream, struct HAL_CODEC_CONFIG_T *cfg)
{
    return 0;
}

void codec_int_stream_mute(enum AUD_STREAM_T stream, bool mute)
{
}

void codec_int_stream_set_chan_vol(enum AUD_STREAM_T stream, enum AUD_CHANNEL_MAP_T ch_map, uint8_t vol)
{
}

void codec_int_stream_restore_chan_vol(enum AUD_STREAM_T stream)
{
}

uint32_t codec_int_stream_start(enum AUD_STREAM_T stream)
{
    return 0;
}

uint32_t codec_int_stream_stop(enum AUD_STREAM_T stream)
{
    return 0;
}

uint32_t codec_int_stream_close(enum AUD_STREAM_T stream)
{
    return 0;
}

uint32_t codec_int_open(void)
{
    return 0;
}

uint32_t codec_int_close(enum CODEC_CLOSE_TYPE_T type)
{
    return 0;
}

