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
#ifndef AUDIO_TESTER_H
#define AUDIO_TESTER_H

typedef struct {
    uint16_t *buf;
    uint32_t len;
    uint32_t cuur_buf_pos;
}audio_test_pcmpatten_t;

enum AUDIO_TEST_CACHE_T {
    AUDIO_TEST_CACHE_CACHEING= 0,
    AUDIO_TEST_CACHE_OK,
    AUDIO_TEST_CACHE_QTY,
};

enum AUDIO_TEST_OPERATION_T {
    AUDIO_TESTER_ADDALOOP= 0,
    AUDIO_TESTER_SINWAVE,
    AUDIO_TESTER_MUTE,
    AUDIO_TESTER_PDM,
    AUDIO_TESTER_EQ,
    AUDIO_TESTER_I2S,
    AUDIO_TESTER_ANC,
};


int addaloop_test_output_pcmpatten(audio_test_pcmpatten_t *pcmpatten, uint8_t *buf, uint32_t len);

#endif /*AUDIO_TESTER_H*/