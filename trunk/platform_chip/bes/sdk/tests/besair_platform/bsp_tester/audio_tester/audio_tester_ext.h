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
#ifndef AUDIO_TESTER_EXT_H
#define AUDIO_TESTER_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#define audio_CMD_HELP_INFO                                                "\n\
        help - an interface to the audio test command manual                \n\
SYNOPSIS                                                                    \n\
        audio,help                                                          \n\
        audio,[command],[state],[channel number],[analog adc gain],         \n\
                     [bit],[spk gain],[mic gain]                            \n\
                command options                                             \n\
                        adda_loop, sin_wave, mute ,pdm ,eq                  \n\
                state options                                               \n\
                        on, off                                             \n\
                channel number options                                      \n\
                        0, 1                                                \n\
                analog adc gain options                                     \n\
                        0 ~ 7                                               \n\
                bit options                                                 \n\
                        16, 24                                              \n\
                spk gain options                                            \n\
                        1 ~ 16                                              \n\
                mic gain options                                            \n\
                        1 ~ 16                                              \n\
EXAMPLES                                                                    \n\
        audio,help                                                          \n\
                get help manual of audio test command                       \n\
        audio,adda_loop,on,1,3,16,16,16                                     \n\
                adda_loop, state on, dac channel 1, adc gain is set to 3,   \n\
                bit is set to 16, spk gain and mic gain are set to 16     \n\n\
"

/**
 * \brief audio test function
 * \param [in]  params  parameters received as strings from uart port
 */
void audio_tester(const char **params);
#ifdef __cplusplus
}
#endif

#endif /* AUDIO_TESTER_EXT_H */