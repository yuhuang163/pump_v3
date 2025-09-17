#include "cmsis_os.h"
#include "hal_trace.h"
#include "app_audio.h"
#include "app_through_put.h"
#include "bluetooth_bt_api.h"
#if defined(NEW_NV_RECORD_ENABLED)
#include "nvrecord_bt.h"
#endif
#include "app_ai_if.h"
#include "app_ai_tws.h"
#include "app_ai_manager_api.h"
#include "app_ai_if_config.h"
#include "app_ai_if_thirdparty.h"
#include "app_ai_if_custom_ui.h"

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#ifdef __AI_VOICE__
#ifdef __IAG_BLE_INCLUDE__
#include "app_ai_ble.h"
#endif
#include "ai_manager.h"
#include "ai_control.h"
#include "ai_thread.h"
#include "app_ai_voice.h"
#include "bluetooth_bt_api.h"
#include "ai_spp.h"
#endif

#ifdef BISTO_ENABLED
#include "gsound_custom.h"
#include "gsound_custom_bt.h"
#endif

#ifdef IBRT
#include "app_ibrt_middleware.h"
#include "app_tws_ibrt.h"
#endif

#define CASE_S(s) case s:return "["#s"]";
#define CASE_D()  default:return "[INVALID]";

AI_CAPTURE_BUFFER_T app_ai_if_capture_buf;

const char *ai_spec_type2str(AI_SPEC_TYPE_E ai_spec)
{
    switch(ai_spec) {
        CASE_S(AI_SPEC_INIT)
        CASE_S(AI_SPEC_GSOUND)
        CASE_S(AI_SPEC_AMA)
        CASE_S(AI_SPEC_BES)
        CASE_S(AI_SPEC_BAIDU)
        CASE_S(AI_SPEC_TENCENT)
        CASE_S(AI_SPEC_ALI)
        CASE_S(AI_SPEC_COMMON)
        CASE_D()
    }
}

bool ai_if_is_ai_stream_mic_open(void)
{
    bool mic_open = false;

#ifdef __AI_VOICE__
    mic_open = app_ai_voice_is_mic_open();
#endif

    return mic_open;
}

uint8_t app_ai_if_get_ble_connection_index(void)
{
    uint8_t ble_connection_index = 0xFF;

#if defined(__AI_VOICE__) && defined(__IAG_BLE_INCLUDE__)
    ble_connection_index = app_ai_ble_get_connection_index(ai_manager_get_foreground_ai_conidx());
#endif

    return ble_connection_index;
}

static void app_ai_if_set_ai_spec(uint32_t *ai_spec, uint32_t set_spec)
{
    if(*ai_spec == AI_SPEC_INIT)
    {
        *ai_spec = set_spec;
    }
    else
    {
        *ai_spec = (*ai_spec << 8) | set_spec;
        TRACE(2, "%s, AI SPEC = %04x", __func__, *ai_spec);
    }
}

uint32_t app_ai_if_get_ai_spec(void)
{
    uint32_t ai_spec = AI_SPEC_INIT;

    app_ai_if_set_ai_spec(&ai_spec, AI_SPEC_INIT);//Avoid not using this function in some cases, resulting in compilation failure

#ifdef __AMA_VOICE__
    app_ai_if_set_ai_spec(&ai_spec, AI_SPEC_AMA);
#endif
#ifdef __DMA_VOICE__
    app_ai_if_set_ai_spec(&ai_spec, AI_SPEC_BAIDU);
#endif
#ifdef __SMART_VOICE__
    app_ai_if_set_ai_spec(&ai_spec, AI_SPEC_BES);
#endif
#ifdef __TENCENT_VOICE__
    app_ai_if_set_ai_spec(&ai_spec, AI_SPEC_TENCENT);
#endif
#ifdef __GMA_VOICE__
    app_ai_if_set_ai_spec(&ai_spec, AI_SPEC_ALI);
#endif
#ifdef __CUSTOMIZE_VOICE__
    app_ai_if_set_ai_spec(&ai_spec, AI_SPEC_COMMON);
#endif
#ifdef DUAL_MIC_RECORDING
    app_ai_if_set_ai_spec(&ai_spec, AI_SPEC_RECORDING);
#endif
    //TRACE(2, "%s, AI SPEC = %04x", __func__, ai_spec);
    return ai_spec;
}

#define     AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE           (0x00)
static uint32_t ai_capture_mempool_user_addr[AI_CAPTURE_MEMPOOL_USER_MAX_COUNT] = {
    [AI_CAPTURE_MEMPOOL_USER_INVALID]                       = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_AUDIOFLINGER]                  = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_PCM_CACHE]                     = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_HOTWORD_ENGINE]                = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_VAD]                           = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_ANC_ASSIST]                    = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_ENCODE_STORAGE]                = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_ENCODE_ONE_FRAME]              = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_ENCODE_STEREO_M_STORAGE]       = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_ENCODE_STEREO_S_STORAGE]       = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_ENCODE_STEREO_PROCESS_HEAP]    = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_ENCODE_OPUS_HEAP]              = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_ENCODE_OPUS_STACK]             = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_ANC_ASSIST_RESAMPLE]           = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
    [AI_CAPTURE_MEMPOOL_USER_HOTWORD_MODELFILE]             = AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE,
};

static POSSIBLY_UNUSED void app_ai_if_mempool_init(void)
{
    uint8_t *buf = NULL;
    app_capture_audio_mempool_get_buff(&buf, APP_CAPTURE_AUDIO_BUFFER_SIZE);

    memset((uint8_t *)buf, 0, APP_CAPTURE_AUDIO_BUFFER_SIZE);

    app_ai_if_capture_buf.buff = buf;
    app_ai_if_capture_buf.buff_size_total = APP_CAPTURE_AUDIO_BUFFER_SIZE;
    app_ai_if_capture_buf.buff_size_used = 0;
    app_ai_if_capture_buf.buff_size_free = APP_CAPTURE_AUDIO_BUFFER_SIZE;

    memset(ai_capture_mempool_user_addr, AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE, sizeof(ai_capture_mempool_user_addr));
    TRACE(3, "%s buf %p size %d", __func__, buf, APP_CAPTURE_AUDIO_BUFFER_SIZE);
}

void app_ai_if_mempool_deinit(void)
{
    TRACE(2, "%s size %d", __func__, app_ai_if_capture_buf.buff_size_total);

    memset((uint8_t *)app_ai_if_capture_buf.buff, 0, app_ai_if_capture_buf.buff_size_total);

    app_ai_if_capture_buf.buff_size_used = 0;
    app_ai_if_capture_buf.buff_size_free = app_ai_if_capture_buf.buff_size_total;
    memset(ai_capture_mempool_user_addr, AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE, sizeof(ai_capture_mempool_user_addr));
}

void app_ai_if_mempool_get_buff(uint8_t **buff, uint32_t size, uint8_t user_index)
{
    uint32_t buff_size_free;

    buff_size_free = app_ai_if_capture_buf.buff_size_free;

    if (user_index >= AI_CAPTURE_MEMPOOL_USER_MAX_COUNT)
    {
        ASSERT(0, "Invalid user request to ai mempool");
    }
    if (ai_capture_mempool_user_addr[user_index] != AI_CAPTURE_MEMPOOL_ADDR_INVALID_VALUE)
    {
        *buff = (uint8_t *)ai_capture_mempool_user_addr[user_index];
        TRACE(2, "user(%d) has allocated mem:%p", user_index, *buff);
        return;
    }
    if (size % 4){
        size = size + (4 - size % 4);
    }

    TRACE(3,"%s free %d to allocate %d", __func__, buff_size_free, size);

    ASSERT(size <= buff_size_free, "[%s] size = %d > free size = %d", __func__, size, buff_size_free);

    *buff = app_ai_if_capture_buf.buff + app_ai_if_capture_buf.buff_size_used;

    app_ai_if_capture_buf.buff_size_used += size;
    app_ai_if_capture_buf.buff_size_free -= size;

    ai_capture_mempool_user_addr[user_index] = (uint32_t)(*buff);

    TRACE(5, "AI allocate %d, now used %d left %d user(%d) addr:0x%08x", size,
          app_ai_if_capture_buf.buff_size_used,
          app_ai_if_capture_buf.buff_size_free, user_index, ai_capture_mempool_user_addr[user_index]);
}

void app_ai_if_mobile_connect_handle(void *addr)
{
    POSSIBLY_UNUSED bt_bdaddr_t * _addr = (bt_bdaddr_t *)addr;

#ifdef BISTO_ENABLED
    gsound_custom_bt_link_connected_handler(_addr->address);
#endif

#ifdef BT_DIP_SUPPORT
    POSSIBLY_UNUSED MOBILE_CONN_TYPE_E type = MOBILE_CONNECT_IDLE;
    bt_dip_pnp_info_t pnp_info;
    pnp_info.vend_id = 0;

    bool isExsit = nv_record_get_pnp_info(_addr, &pnp_info);
    TRACE(3, "%s vend_id 0x%x vend_id_source 0x%x", __func__, 
        pnp_info.vend_id, pnp_info.vend_id_source);
    if (isExsit)
    {
        type = bes_bt_dip_check_is_ios_device(pnp_info.vend_id, pnp_info.vend_id_source)?MOBILE_CONNECT_IOS:MOBILE_CONNECT_ANDROID;
#ifdef __AI_VOICE__
        app_ai_mobile_connect_handle(type, _addr->address);
#endif
#ifdef BISTO_ENABLED
        gsound_mobile_type_get_callback(type);
#endif
    }
#endif

#ifdef __IAG_BLE_INCLUDE__
    bes_ble_gap_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
#endif
}

void app_ai_if_hfp_connected_handler(uint8_t device_id)
{
#ifdef __AI_VOICE__
    uint8_t ai_index = 0;

    for (uint8_t ai_connect_index = 0; ai_connect_index < AI_CONNECT_NUM_MAX; ai_connect_index++)
    {
        ai_index = app_ai_get_ai_index_from_connect_index(ai_connect_index);

        if(ai_index)
        {
            AI_connect_info *ai_info = app_ai_get_connect_info(ai_connect_index);
            if(ai_info) {
                uint8_t dest_id = app_ai_get_dest_id(ai_info);
                ai_function_handle(CALLBACK_STOP_SPEECH, NULL, 0, ai_index, dest_id);
            }
        }
    }
#endif
}

void app_ai_voice_stay_active(uint8_t aiIndex)
{
#ifdef __AI_VOICE__
#ifdef IBRT
        ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        if (app_tws_ibrt_tws_link_connected() && \
            (p_ibrt_ctrl->nv_role == IBRT_MASTER) && \
            p_ibrt_ctrl->tws_conhandle != BT_INVALID_CONN_HANDLE)
        {
            bes_bt_me_acl_stop_sniff(p_ibrt_ctrl->tws_conhandle);
        }

        bt_acl_state_t acl_state = bes_bt_me_acl_get_state(app_ai_get_device_id_from_index(aiIndex));
        if (acl_state.acl_is_connected)
        {
            app_ibrt_middleware_prevent_sniff_set(acl_state.remote.address, AI_VOICE_RECORD);
        }

#else
        bes_bt_me_set_stay_active_mode(true, BT_ACTIVE_MODE_KEEP_USER_AI_VOICE_STREAM, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
#endif
#endif
}

void app_ai_voice_resume_sleep(uint8_t aiIndex)
{
#ifdef __AI_VOICE__
#ifdef IBRT
    bt_acl_state_t acl_state = bes_bt_me_acl_get_state(app_ai_get_device_id_from_index(aiIndex));
    if (acl_state.acl_is_connected)
    {
        app_ibrt_middleware_prevent_sniff_clear(acl_state.remote.address, AI_VOICE_RECORD);
    }

#else
    bes_bt_me_set_stay_active_mode(false, BT_ACTIVE_MODE_KEEP_USER_AI_VOICE_STREAM, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
#endif
#endif
}

void app_ai_key_event_handle(void *param1, uint32_t param2)
{
}

void app_ai_init(void)
{
#ifdef __AI_VOICE__
    app_ai_manager_init();

#ifdef BISTO_ENABLED
    gsound_custom_init(true);
#endif
#ifdef __AMA_VOICE__
    ai_open_specific_ai(AI_SPEC_AMA);
#endif
#ifdef __DMA_VOICE__
    ai_open_specific_ai(AI_SPEC_BAIDU);
#endif
#ifdef __SMART_VOICE__
    ai_open_specific_ai(AI_SPEC_BES);
#endif
#ifdef __TENCENT_VOICE__
    ai_open_specific_ai(AI_SPEC_TENCENT);
#endif
#ifdef __GMA_VOICE__
    ai_open_specific_ai(AI_SPEC_ALI);
#endif
#ifdef __CUSTOMIZE_VOICE__
    ai_open_specific_ai(AI_SPEC_COMMON);
#endif
#ifdef DUAL_MIC_RECORDING
    ai_open_specific_ai(AI_SPEC_RECORDING);
#endif
#ifdef __BIXBY_VOICE__
    ai_open_specific_ai(AI_SPEC_BIXBY);
#endif

    app_ai_tws_init();
    app_ai_if_custom_init();

#ifdef __THROUGH_PUT__
    app_throughput_test_init();
#endif

    app_ai_voice_thread_init();

    app_capture_audio_mempool_init();

#ifdef __THIRDPARTY
    app_ai_if_thirdparty_init();
#endif

    app_ai_if_mempool_init();
#endif
}

uint8_t app_ai_get_codec_type(uint8_t spec)
{
    /// init as invalid value
    uint8_t codec = 0;

    if (0)
    {
    }
#ifdef __AMA_VOICE__
    else if (AI_SPEC_AMA == spec)
    {
        codec = VOC_ENCODE_OPUS;
    }
#endif
#ifdef __DMA_VOICE__
    else if (AI_SPEC_BAIDU == spec)
    {
        codec = VOC_ENCODE_OPUS;
    }
#endif
#ifdef __SMART_VOICE__
    else if (AI_SPEC_BES == spec)
    {
        codec = VOC_ENCODE_OPUS;
    }
#endif
#ifdef __TENCENT_VOICE__
    else if (AI_SPEC_TENCENT == spec)
    {
        codec = VOC_ENCODE_OPUS;
    }
#endif
#ifdef __GMA_VOICE__
    else if (AI_SPEC_ALI == spec)
    {
        codec = VOC_ENCODE_OPUS;
    }
#endif
#ifdef __CUSTOMIZE_VOICE__
    else if (AI_SPEC_COMMON == spec)
    {
        codec = VOC_ENCODE_OPUS;
    }
#endif
#ifdef DUAL_MIC_RECORDING
    else if (AI_SPEC_RECORDING == spec)
    {
#ifdef RECORDING_USE_OPUS
    codec = VOC_ENCODE_OPUS;
#else
    codec = VOC_ENCODE_SCALABLE;
#endif
    }
#endif
#ifdef __BIXBY_VOICE__
    else if (AI_SPEC_BIXBY == spec)
    {
        codec = VOC_ENCODE_OPUS;
    }
#endif

    return codec;
}
