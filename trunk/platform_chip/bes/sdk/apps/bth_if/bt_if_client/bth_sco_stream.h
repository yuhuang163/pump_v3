/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#ifndef __BTH_SCO_STREAM_H__
#define __BTH_SCO_STREAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SPEECH_CODEC_CAPTURE_CHANNEL_NUM
#define SPEECH_CODEC_CAPTURE_CHANNEL_NUM 1
#endif

#ifndef SPEECH_SCO_FRAME_MS
#define SPEECH_SCO_FRAME_MS 15
#endif

int bt_sco_player_start(uint8_t codec_type);
int bt_sco_player_stop(void);

#ifdef __cplusplus
}
#endif
#endif // end of __BTH_SCO_STREAM_H__