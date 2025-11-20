#ifndef AI_IF_H_
#define AI_IF_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "app_ai_if_config.h"

//the ai type now use
typedef enum {
    AI_SPEC_INIT,
    AI_SPEC_GSOUND,
    AI_SPEC_AMA,
    AI_SPEC_BES,
    AI_SPEC_BAIDU,
    AI_SPEC_TENCENT,
    AI_SPEC_ALI,
    AI_SPEC_COMMON,
    AI_SPEC_BIXBY,
    AI_SPEC_RECORDING,
    AI_SPEC_COUNT,
} AI_SPEC_TYPE_E;

typedef struct{
    uint8_t connPathType;
    uint8_t connPathState;
    uint8_t connBtState;
    uint8_t connBdAddr[6];
}AI_CONNECTION_STATE_T;

typedef struct{
    uint8_t *buff;
    uint32_t buff_size_total;
    uint32_t buff_size_used;
    uint32_t buff_size_free;
}AI_CAPTURE_BUFFER_T;

typedef enum {
    AI_CAPTURE_MEMPOOL_USER_INVALID,
    //PCM_AUDIO_FLINGER_BUF_SIZE(_ctl.streamCfg.data_size)
    AI_CAPTURE_MEMPOOL_USER_AUDIOFLINGER,
    //PCM_DATA_CACHE_BUF_SIZE
    AI_CAPTURE_MEMPOOL_USER_PCM_CACHE,
    //HW_ENGINE_BUF_SIZE
    AI_CAPTURE_MEMPOOL_USER_HOTWORD_ENGINE,
    //VAD_BUF_SIZE
    AI_CAPTURE_MEMPOOL_USER_VAD,
    //ANC_ASSIST_BUF_SIZE
    AI_CAPTURE_MEMPOOL_USER_ANC_ASSIST,
    //VOB_ENCODED_DATA_STORAGE_BUF_SIZE.
    //VOB_ENCODED_DATA_STORAGE_BUF_SIZE + ENCODE_DECODE_FRAME_BUF_SIZE < ENCODE_BUF_SIZE
    AI_CAPTURE_MEMPOOL_USER_ENCODE_STORAGE,
    //ENCODE_DECODE_FRAME_BUF_SIZE
    AI_CAPTURE_MEMPOOL_USER_ENCODE_ONE_FRAME,
    //LOCAL_ENCODED_DATA_STORAGE_BUF_SIZE
    //LOCAL_ENCODED_DATA_STORAGE_BUF_SIZE + SLAVE_ENCODED_DATA_STORAGE_BUF_SIZE < ENCODED_DATA_BUF_SIZE
    AI_CAPTURE_MEMPOOL_USER_ENCODE_STEREO_M_STORAGE,
    //LOCAL_ENCODED_DATA_STORAGE_BUF_SIZE
    AI_CAPTURE_MEMPOOL_USER_ENCODE_STEREO_S_STORAGE,
    //BINAURAL_RECORD_PROCESS_BUF_SIZE
    AI_CAPTURE_MEMPOOL_USER_ENCODE_STEREO_PROCESS_HEAP,
    //VOICE_OPUS_HEAP_SIZE
    AI_CAPTURE_MEMPOOL_USER_ENCODE_OPUS_HEAP,
    //VOICE_OPUS_STACK_SIZE
    AI_CAPTURE_MEMPOOL_USER_ENCODE_OPUS_STACK,
    // ANC_ASSIST_RESAMPLE_BUF_SIZE
    AI_CAPTURE_MEMPOOL_USER_ANC_ASSIST_RESAMPLE,
    AI_CAPTURE_MEMPOOL_USER_HOTWORD_MODELFILE,
    AI_CAPTURE_MEMPOOL_USER_MAX_COUNT,
} AI_CAPTURE_MEMPOOL_TYPE_E;


#ifdef __cplusplus
extern "C" {
#endif

const char *ai_spec_type2str(AI_SPEC_TYPE_E ai_spec);

/*---------------------------------------------------------------------------
 *            app_ai_key_event_handle
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    for AI handle key event
 *
 * Parameters:
 *    void *param1
 *    uint32_t param2
 *
 * Return:
 *    void
 */
void app_ai_key_event_handle(void *param1, uint32_t param2);

/*---------------------------------------------------------------------------
 *            app_ai_if_get_ai_spec
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    to get the ai current spec
 *
 * Parameters:
 *    void
 *
 * Return:
 *    uint32_t -- ai spec
 */
uint32_t app_ai_if_get_ai_spec(void);

/*---------------------------------------------------------------------------
 *            app_ai_if_mobile_connect_handle
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handle ai when mobile connect
 *
 * Parameters:
 *    _addr -- the address of connected mobile
 *
 * Return:
 *    void
 */
void app_ai_if_mobile_connect_handle(void *_addr);

/*---------------------------------------------------------------------------
 *            app_ai_if_hfp_connected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handle ai when hfp connect
 *
 * Parameters:
 *    device_id -- the id of connected device
 *
 * Return:
 *    void
 */
    void app_ai_if_hfp_connected_handler(uint8_t device_id);

/*---------------------------------------------------------------------------
 *            app_ai_init
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    init AI
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ai_init(void);

/*---------------------------------------------------------------------------
 *            ai_if_is_ai_stream_mic_open
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    to get whether ai stream is opened
 *
 * Parameters:
 *    void
 *
 * Return:
 *    bool true means ai stream is opened
 */
bool ai_if_is_ai_stream_mic_open(void);

/*---------------------------------------------------------------------------
 *            app_ai_if_get_ble_connection_index
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    to get ai ble connection index
 *
 * Parameters:
 *    void
 *
 * Return:
 *    uint8_t --  ble connection index
 */
uint8_t app_ai_if_get_ble_connection_index(void);


/*---------------------------------------------------------------------------
*            app_ai_if_mempool_deinit
*---------------------------------------------------------------------------
*
*Synopsis:
*    init ai voice capture mempool
*
* Parameters:
*    void
*
* Return:
*    void
*/
void app_ai_if_mempool_deinit(void);

/*---------------------------------------------------------------------------
*            app_ai_if_mempool_get_buff
*---------------------------------------------------------------------------
*
*Synopsis:
*    get buf form ai voice capture mempool
*
* Parameters:
*    buff -- the pointer of buf that get from mempool
*    size -- the size of buf that get from mempool
*
* Return:
*    void
*/
void app_ai_if_mempool_get_buff(uint8_t **buff, uint32_t size, uint8_t user_index);


void app_ai_voice_stay_active(uint8_t aiIndex);

void app_ai_voice_resume_sleep(uint8_t aiIndex);

/**
 * @brief Get code type from AI spec
 * 
 * @param spec          AI type, @see AI_SPEC_TYPE_E
 * @return uint8_t      codec type
 */
uint8_t app_ai_get_codec_type(uint8_t spec);

#ifdef __cplusplus
    }
#endif


#endif //AI_IF_H_

