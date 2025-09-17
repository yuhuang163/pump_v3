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
#ifndef __MEDIA_CALLBACK_TESTS_H__
#define __MEDIA_CALLBACK_TESTS_H__

#include "plat_types.h"

void prompt_stop_all_test(char* cmdParam, uint32_t cmdParam_len);

void media_PlayAudio_test(char* cmdParam, uint32_t cmdParam_len);

void media_PlayAudio_locally_test(char* cmdParam, uint32_t cmdParam_len);

void media_PlayAudio_remotely_test(char* cmdParam, uint32_t cmdParam_len);

void media_PlayAudio_standalone_test(char* cmdParam, uint32_t cmdParam_len);

void media_PlayAudio_standalone_locally_test(char* cmdParam, uint32_t cmdParam_len);

void media_PlayAudio_standalone_remotely_test(char* cmdParam, uint32_t cmdParam_len);

void media_PlayAudio_continuous_test(char* cmdParam, uint32_t cmdParam_len);

void app_prompt_finish_callback_register_test(char* cmdParam, uint32_t cmdParam_len);

#ifdef IBRT
void app_ibrt_send_volumeup_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_send_volumedown_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_send_set_absvolume_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_adjust_local_volumeup_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_adjust_local_volumedown_test(char* cmdParam, uint32_t cmdParam_len);
#endif

#endif /* #ifndef __MEDIA_CALLBACK_TESTS_H__ */
