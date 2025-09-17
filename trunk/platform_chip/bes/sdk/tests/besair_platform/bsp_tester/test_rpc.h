#ifndef __RPC_SBC_CLI_H__
#define __RPC_SBC_CLI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The command name,this must be same with server side
 * NOTE: don't exceed the RPC_CMD_NAME_LEN
 */
#define INIT_DECODER           "init_decoder"
#define DECODE_FRAME           "decode_frame"
#define DECODE_END             "decode_end"

/*
 * paramters used for sbc_frames_decode
 * NOTE: must same between server&client
 */
struct sbc_decord_params {
    uint8_t         *Buff;
    uint16_t         Len;
    short           *PcmData;
    uint16_t         MaxPcmData;
    float*          gains;
};

/*
 * returned value for sbc_frames_decode
 * NOTE: must same between server&client
 */

struct sbc_decode_result {
    uint32_t result;
    uint16_t BytesParsed;
};

int rpc_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __RPC_SBC_CLI_H__ */
