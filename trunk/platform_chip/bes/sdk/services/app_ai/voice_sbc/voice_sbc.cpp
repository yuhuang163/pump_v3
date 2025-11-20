#ifndef VOICE_DATAPATH
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
#include "voice_sbc.h"
#include "hal_trace.h"

static VOICE_SBC_CONFIG_T	voice_sbc_config =
{
    VOICE_SBC_CHANNEL_COUNT     ,
    VOICE_SBC_CHANNEL_MODE 	    ,
    VOICE_SBC_BIT_POOL	 	    ,
    VOICE_SBC_SIZE_PER_SAMPLE   ,
    VOICE_SBC_SAMPLE_RATE	    ,
    VOICE_SBC_NUM_BLOCKS	    ,
    VOICE_SBC_NUM_SUB_BANDS	    ,
    VOICE_SBC_MSBC_FLAG		    ,
    VOICE_SBC_ALLOC_METHOD	    ,
};

static int voice_sbc_init_encoder(void);

static sbc_encoder_t voice_sbc_encoder;
static sbc_decoder_t voice_sbc_decoder;
static uint32_t voice_sbc_frame_len = 0;

static int voice_sbc_init_encoder(void)
{
    sbc_stream_info_t info;
    info.flags =  voice_sbc_config.mSbcFlag;
    info.num_channels = voice_sbc_config.channelCnt;
    info.channel_mode = voice_sbc_config.channelMode;
    info.bit_pool     = voice_sbc_config.bitPool;
    info.sample_rate  = voice_sbc_config.sampleRate;
    info.alloc_method = voice_sbc_config.allocMethod;
    info.num_blocks   = voice_sbc_config.numBlocks;
    info.num_subbands = voice_sbc_config.numSubBands;
    sbc_encoder_open(&voice_sbc_encoder, info);
    voice_sbc_frame_len = sbc_get_frame_len(info);
    TRACE(1,"frame len is %d", voice_sbc_frame_len);
    return 0;
}

static int voice_sbc_init_decoder(void)
{
    sbc_decoder_open(&voice_sbc_decoder);
    return 0;
}

uint32_t voice_sbc_get_frame_len(void)
{
    return voice_sbc_frame_len;
}

uint32_t voice_sbc_encode(uint8_t *input, uint32_t inputBytes, uint32_t* purchasedBytes, uint8_t *output, uint8_t isReset)
{
	if (isReset)
	{
		voice_sbc_init_encoder();
	}

    pcm_frame_t pcm_data;
    uint16_t outputSbcBytes, bytes_encoded;
    sbc_stream_info_t info;
    sbc_encoder_get_stream_info(&voice_sbc_encoder, &info);
    if (info.flags == SBC_FLAGS_MSBC)
    {
        uint16_t outputOffset = 0;
        uint16_t oneChunkConsumed;
        bytes_encoded = 0;

        for (uint32_t index = 0;index < inputBytes;index += (uint32_t)VOICE_SBC_PCM_DATA_SIZE_PER_FRAME)
        {
            pcm_data.pcm_data = (int16_t *)(input+index);
            pcm_data.buffer_size = VOICE_SBC_PCM_DATA_SIZE_PER_FRAME;
            pcm_data.valid_size = VOICE_SBC_PCM_DATA_SIZE_PER_FRAME;
            sbc_frame_t sbc_data;
            sbc_data.sbc_data = &output[outputOffset];
            sbc_data.buffer_size = 0XFFFF;
            sbc_data.valid_size = 0;
            //output[outputOffset++] = 0xAD;
            //output[outputOffset++] = 0x02;
            sbc_encoder_process_frame(&voice_sbc_encoder, &pcm_data, &sbc_data);
            outputOffset += sbc_data.valid_size;
            //output[outputOffset++] = 0x00;
            oneChunkConsumed = VOICE_SBC_PCM_DATA_SIZE_PER_FRAME - pcm_data.valid_size;
            bytes_encoded += oneChunkConsumed;
        }

        outputSbcBytes = outputOffset;
    }
    else
    {
        pcm_data.pcm_data = (int16_t *)input;
        pcm_data.buffer_size = inputBytes;
        pcm_data.valid_size = inputBytes;
        sbc_frame_t sbc_data;
        sbc_data.sbc_data = output;
        sbc_data.buffer_size = 0XFFFF;
        sbc_data.valid_size = 0;
        sbc_encoder_process_frame(&voice_sbc_encoder, &pcm_data, &sbc_data);
        outputSbcBytes = sbc_data.valid_size;
        bytes_encoded = inputBytes - pcm_data.valid_size;
    }

    TRACE(2,"Encode %d bytes PCM data into %d bytes SBC data.", inputBytes, outputSbcBytes);
    TRACE(1,"Consumed PCM data %d bytes", bytes_encoded);

    *purchasedBytes = bytes_encoded;

    return outputSbcBytes;
}

uint32_t voice_sbc_decode(uint8_t* pcm_buffer, CQueue* encodedDataQueue, uint32_t expectedOutputSize, uint8_t isReset)
{
    if (isReset)
    {
        voice_sbc_init_decoder();
    }
    int r = 0;
    unsigned char *e1 = NULL, *e2 = NULL;
    unsigned int len1 = 0, len2 = 0;
    uint32_t sbcDataBytesToDecode;
    unsigned int pcm_offset = 0;
    uint16_t byte_decode;
    int8_t ret;
    pcm_frame_t pcm_data;

get_again:    
    pcm_data.pcm_data = (int16_t *)(pcm_buffer+pcm_offset);
    pcm_data.buffer_size = expectedOutputSize-pcm_offset;
    pcm_data.valid_size = 0;

    if (LengthOfCQueue(encodedDataQueue) > VOICE_SBC_ENCODED_DATA_SIZE_PER_FRAME)
    {
        sbcDataBytesToDecode = VOICE_SBC_ENCODED_DATA_SIZE_PER_FRAME;
    }
    else
    {
        sbcDataBytesToDecode = LengthOfCQueue(encodedDataQueue);
    }

    len1 = len2 = 0;
    r = PeekCQueue(encodedDataQueue, sbcDataBytesToDecode, &e1, &len1, &e2, &len2);
    if ((r == CQ_ERR) || (0 == len1)) {
        goto exit;
    }  

    sbc_frame_t sbc_data;
    sbc_data.sbc_data = e1;
    sbc_data.valid_size = len1;
    ret = sbc_decoder_process_frame(&voice_sbc_decoder, &sbc_data, &pcm_data);
    byte_decode = len1 - sbc_data.valid_size;
    TRACE(4,"len1 %d byte_decode %d expectedOutputSize %d pcm_offset %d",
        len1, byte_decode, expectedOutputSize, pcm_offset);
    TRACE(1,"voice_PcmDecData.dataLen %d", pcm_data.valid_size);

    DeCQueue(encodedDataQueue, 0, byte_decode);

    pcm_offset += pcm_data.valid_size;

    if (expectedOutputSize == pcm_offset)
    {
        goto exit;
    }

    if ((ret == SBC_CONTINUE) || (ret == SBC_OK))  {
        goto get_again;
    }

exit:
    TRACE(1,"Get pcm data size %d", pcm_offset);
    return pcm_offset;
}

int voice_sbc_init(VOICE_SBC_CONFIG_T* pConfig)
{
    voice_sbc_config = *pConfig;
    return 0;
}

#endif
