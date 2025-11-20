#include <stdio.h>
#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "app_utils.h"
#include "app_audio.h"
#include "audio_trigger_checker.h"
#include "a2dp_decoder.h"
#include "bt_drv.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "audioflinger.h"
#include "audio_trigger_common.h"
#include "audio_trigger_a2dp.h"
#include "app_bt_audio.h"
#include "app_audio_trace.h"

#if defined(IBRT)
#include "app_tws_ibrt.h"
#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_audio_analysis.h"
#include "app_tws_ibrt_audio_sync.h"
#include "audio_trigger_ibrt.h"
#include "app_ibrt_a2dp.h"
#endif

#if (A2DP_DECODER_VER < 2)
extern void a2dp_audio_set_mtu_limit(uint8_t mut);
#endif

#ifdef ADVANCE_FILL_ENABLED
extern bool is_doing_pre_fill;
#endif

uint32_t tg_acl_trigger_time = 0;
uint32_t tg_acl_trigger_start_time = 0;
uint32_t tg_acl_trigger_init_time = 0;

#ifdef A2DP_DECODER_CROSS_CORE
extern void a2dp_decoder_bth_set_first_run(A2DP_AUDIO_STARTFRAME_INFO_T headframe);
#endif

void app_bt_stream_set_trigger_time(uint32_t trigger_time_us)
{
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    uint32_t curr_ticks = 0;
    uint32_t dma_buffer_delay_us = 0;
    uint32_t tg_acl_trigger_offset_time = 0;
    POSSIBLY_UNUSED uint8_t device_id = bes_bt_me_select_device(BT_SELECT_CURR_A2DP_DEVICE);
    POSSIBLY_UNUSED bt_acl_state_t acl_state = bes_bt_me_acl_get_state(device_id);

    if (trigger_time_us){
#if defined(IBRT)
        uint16_t conhandle = BT_INVALID_CONN_HANDLE;
        if (app_tws_ibrt_mobile_link_connected(&acl_state.remote)){
            conhandle = app_tws_ibrt_get_mobile_connhandle(&acl_state.remote);

            curr_ticks = bt_syn_get_curr_ticks(conhandle);
        }else if (app_tws_ibrt_slave_ibrt_link_connected(&acl_state.remote)){
            conhandle = app_tws_ibrt_get_ibrt_handle(&acl_state.remote);
            curr_ticks = bt_syn_get_curr_ticks(conhandle);
        }else{
            return;
        }
#else
        uint16_t conhandle = acl_state.acl_conn_hdl;
        curr_ticks = bt_syn_get_curr_ticks(conhandle);
#endif
        app_sysfreq_req(APP_SYSFREQ_USER_TRIGGER, APP_SYSFREQ_52M);
        af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false);
        if (NULL == stream_cfg)
        {
            TRACE(1,"%s Fail!!!", __func__);
            return;
        }

        btdrv_syn_trigger_codec_en(0);
        btdrv_syn_clr_trigger(0);

        btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);

        dma_buffer_delay_us = app_bt_stream_get_dma_buffer_delay_us();
        dma_buffer_delay_us /= 2;
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][SETTIME] %d-%d-%d dma_sz:%d dly:%d", stream_cfg->sample_rate, stream_cfg->channel_num, stream_cfg->bits,
                                                                                   stream_cfg->data_size, dma_buffer_delay_us);

        tg_acl_trigger_offset_time = US_TO_BTCLKS(trigger_time_us-dma_buffer_delay_us);
        tg_acl_trigger_time = curr_ticks + tg_acl_trigger_offset_time;
        tg_acl_trigger_start_time = curr_ticks;
#if defined(IBRT)
        bt_syn_set_tg_ticks(tg_acl_trigger_time, conhandle, BT_TRIG_SLAVE_ROLE,0,false);
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][SETTIME] %d->%d trig_dly:%d aud_dly:%dus",
                                            curr_ticks, tg_acl_trigger_time, trigger_time_us-dma_buffer_delay_us, trigger_time_us+dma_buffer_delay_us);
#else
        bt_syn_set_tg_ticks(tg_acl_trigger_time, conhandle, BT_TRIG_SLAVE_ROLE,0,false);
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][SETTIME] %d->%d trig_dly:%d aud_dly:%dus",
                                    curr_ticks, tg_acl_trigger_time, trigger_time_us-dma_buffer_delay_us, trigger_time_us+dma_buffer_delay_us);
#endif

        btdrv_syn_trigger_codec_en(1);
        app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_WAIT);
    }else{
        tg_acl_trigger_time = 0;
        tg_acl_trigger_start_time = 0;
        btdrv_syn_trigger_codec_en(0);
        btdrv_syn_clr_trigger(0);
        bt_syn_cancel_tg_ticks(0);
        app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_NULL);
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][SETTIME] trigger clear");
        app_sysfreq_req(APP_SYSFREQ_USER_TRIGGER, APP_SYSFREQ_32K);
    }
}

void app_bt_stream_trigger_result(uint8_t device_id)
{
    POSSIBLY_UNUSED uint32_t curr_ticks = 0;

    if(tg_acl_trigger_time){
#if defined(IBRT)
        POSSIBLY_UNUSED bt_acl_state_t acl_state = bes_bt_me_acl_get_state(device_id);
        if (app_tws_ibrt_mobile_link_connected(&acl_state.remote)){
            curr_ticks = bt_syn_get_curr_ticks(app_tws_ibrt_get_mobile_connhandle(&acl_state.remote));
            bt_syn_trig_checker(app_tws_ibrt_get_mobile_connhandle(&acl_state.remote));
        }else if (app_tws_ibrt_slave_ibrt_link_connected(&acl_state.remote)){
            curr_ticks = bt_syn_get_curr_ticks(app_tws_ibrt_get_ibrt_handle(&acl_state.remote));
            bt_syn_trig_checker(app_tws_ibrt_get_ibrt_handle(&acl_state.remote));
        }else{
            TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][RESULT] mobile_link:%d %04x ibrt_link:%d %04x", \
                                       app_tws_ibrt_mobile_link_connected(&acl_state.remote), \
                                       app_tws_ibrt_get_mobile_connhandle(&acl_state.remote), \
                                       app_tws_ibrt_slave_ibrt_link_connected(&acl_state.remote), \
                                       app_tws_ibrt_get_ibrt_handle(&acl_state.remote));
        }
#else
        curr_ticks = btdrv_syn_get_curr_ticks();
#endif
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][RESULT] trig:%d curr:%d tg:%d start:%d", (curr_ticks - (uint32_t)US_TO_BTCLKS(app_bt_stream_get_dma_buffer_delay_us()/2)),
                                                                         curr_ticks, tg_acl_trigger_time, tg_acl_trigger_start_time);
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][RESULT] tg_trig_diff:%d trig_diff:%d", (uint32_t)BTCLKS_TO_US(curr_ticks-tg_acl_trigger_time),
                                              (uint32_t)BTCLKS_TO_US(curr_ticks-tg_acl_trigger_start_time));
        app_bt_stream_set_trigger_time(0);
        app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_OK);
    #if defined(IBRT)
        A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
        a2dp_audio_decoder_headframe_info_get(&headframe_info);
#ifdef ADVANCE_FILL_ENABLED
#ifdef A2DP_DECODER_CROSS_CORE
        if (is_doing_pre_fill)
        {
            A2DP_AUDIO_STARTFRAME_INFO_T start_info;
            a2dp_audio_start_seq_get(&start_info);
            start_info.sequenceNumber += 2;
            a2dp_decoder_bth_set_first_run(start_info);
        }
#endif
#endif
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][RESULT] synchronize_ok :%d", headframe_info.sequenceNumber);
    #endif
    }
}


void app_bt_stream_trigger_init(void)
{
#if defined(IBRT)
    POSSIBLY_UNUSED uint8_t device_id = bes_bt_me_select_device(BT_SELECT_CURR_A2DP_DEVICE);
    POSSIBLY_UNUSED bt_acl_state_t acl_state = bes_bt_me_acl_get_state(device_id);
    if (app_tws_ibrt_mobile_link_connected(&acl_state.remote)){
        tg_acl_trigger_init_time = bt_syn_get_curr_ticks(app_tws_ibrt_get_mobile_connhandle(&acl_state.remote));
    }else if (app_tws_ibrt_slave_ibrt_link_connected(&acl_state.remote)){
        tg_acl_trigger_init_time = bt_syn_get_curr_ticks(app_tws_ibrt_get_ibrt_handle(&acl_state.remote));
    }else{
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][INIT] mobile_link:%d %04x ibrt_link:%d %04x", \
                                    app_tws_ibrt_mobile_link_connected(&acl_state.remote), \
                                    app_tws_ibrt_get_mobile_connhandle(&acl_state.remote), \
                                    app_tws_ibrt_slave_ibrt_link_connected(&acl_state.remote), \
                                    app_tws_ibrt_get_ibrt_handle(&acl_state.remote));
   }
#else
    tg_acl_trigger_init_time = btdrv_syn_get_curr_ticks();
#endif
    app_bt_stream_set_trigger_time(0);
#ifdef PLAYBACK_USE_I2S
    af_i2s_sync_config(AUD_STREAM_ID_0,AUD_STREAM_PLAYBACK, AF_I2S_SYNC_TYPE_BT, false);
    af_i2s_sync_config(AUD_STREAM_ID_0,AUD_STREAM_PLAYBACK, AF_I2S_SYNC_TYPE_BT, true);
#else
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);
#endif
    app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_INIT);
}

void app_bt_stream_trigger_deinit(void)
{
    app_bt_stream_set_trigger_time(0);
}

void app_bt_stream_trigger_start(uint8_t device_id, int8_t offset)
{
    float tg_trigger_time;
    POSSIBLY_UNUSED uint32_t curr_ticks;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false);
    if (NULL == stream_cfg)
    {
        TRACE(1,"%s Fail!!!", __func__);
        return;
    }

#if defined(IBRT)
    POSSIBLY_UNUSED bt_acl_state_t acl_state = bes_bt_me_acl_get_state(device_id);
    if (app_tws_ibrt_mobile_link_connected(&acl_state.remote))
    {
        curr_ticks = bt_syn_get_curr_ticks(app_tws_ibrt_get_mobile_connhandle(&acl_state.remote));
    }
    else if (app_tws_ibrt_slave_ibrt_link_connected(&acl_state.remote))
    {
        curr_ticks = bt_syn_get_curr_ticks(app_tws_ibrt_get_ibrt_handle(&acl_state.remote));
    }
    else
    {
        return;
    }
#else
    curr_ticks = btdrv_syn_get_curr_ticks();
#endif

    TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][START] init(%d)-->set_trig(%d) %dus",
        tg_acl_trigger_init_time, curr_ticks, (uint32_t)BTCLKS_TO_US(curr_ticks-tg_acl_trigger_init_time));
#if defined(A2DP_SCALABLE_ON)
    if(bes_bt_a2dp_get_curr_codec_type() == BT_A2DP_CODEC_TYPE_NON_A2DP){
        if(stream_cfg->sample_rate > AUD_SAMPRATE_48000){
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_US * a2dp_audio_latency_factor_get();

            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_BASE;
#if (A2DP_DECODER_VER < 2)
            a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_MTU);
#endif
        }else{
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_US * a2dp_audio_latency_factor_get();
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_BASE;
#if (A2DP_DECODER_VER < 2)
            a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_MTU);
#endif
        }
    }else
#endif
#if defined(A2DP_AAC_ON)
    if(bes_bt_a2dp_get_curr_codec_type() == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC){
        tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_AAC_US * a2dp_audio_latency_factor_get();
        tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE;
#if (A2DP_DECODER_VER < 2)
        a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU);
#endif
    }else
#endif
#if defined(A2DP_LHDC_ON)
    if(bes_bt_a2dp_get_curr_codec_type() == BT_A2DP_CODEC_TYPE_NON_A2DP){
        if (bes_aud_bt->a2dp_lhdc_config_llc_get()){
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_US * a2dp_audio_latency_factor_get();
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_BASE;
#if (A2DP_DECODER_VER < 2)
            a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU);
#endif
        }else if(stream_cfg->sample_rate > AUD_SAMPRATE_48000){
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_US * a2dp_audio_latency_factor_get();
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE;
#if (A2DP_DECODER_VER < 2)
            a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU);
#endif
        }else{
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_US * a2dp_audio_latency_factor_get();
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE;
#if (A2DP_DECODER_VER < 2)
            a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU);
#endif
        }
    }else
#endif
#if defined(A2DP_LDAC_ON)
    if(bes_bt_a2dp_get_curr_codec_type() == BT_A2DP_CODEC_TYPE_NON_A2DP){
        int32_t frame_mtu = A2DP_PLAYER_PLAYBACK_DELAY_LDAC_FRAME_MTU;
#if (A2DP_DECODER_VER == 2)
        A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
        if (!a2dp_audio_lastframe_info_get(&lastframe_info)){
            frame_mtu = lastframe_info.totalSubSequenceNumber;
        }
#endif
        tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LDAC_US * a2dp_audio_latency_factor_get();
        tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LDAC_BASE*frame_mtu;
#if (A2DP_DECODER_VER < 2)
        a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LDAC_MTU);
#endif
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][START] [%d,%d,%d]", A2DP_PLAYER_PLAYBACK_DELAY_LDAC_MTU,A2DP_PLAYER_PLAYBACK_DELAY_LDAC_BASE,A2DP_PLAYER_PLAYBACK_DELAY_LDAC_US);

    }else
#endif
#if defined(A2DP_LC3_ON)
    if(bes_bt_a2dp_get_curr_codec_type() == BT_A2DP_CODEC_TYPE_NON_A2DP){

        if(stream_cfg->sample_rate > AUD_SAMPRATE_48000){
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LC3_HIRES_US * a2dp_audio_latency_factor_get();
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LC3_HIRES_BASE;
#if (A2DP_DECODER_VER < 2)
            a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LC3_HIRES_MTU);
#endif
        }else{
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LC3_BASERES_US * a2dp_audio_latency_factor_get();
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_BASE;
#if (A2DP_DECODER_VER < 2)
            a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_MTU);
#endif
        }
    }else
#endif
    {
        int32_t frame_mtu = A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU;
#if (A2DP_DECODER_VER == 2)
        A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
        if (!a2dp_audio_lastframe_info_get(&lastframe_info)){
            frame_mtu = lastframe_info.totalSubSequenceNumber;
        }
#endif
        tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_SBC_US * a2dp_audio_latency_factor_get();

        tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE*frame_mtu;
#if (A2DP_DECODER_VER < 2)
        a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU);
#endif
    }
    app_bt_stream_set_trigger_time((uint32_t)tg_trigger_time);
}

bool app_bt_stream_trigger_onprocess(void)
{
    if (app_bt_stream_trigger_stauts_get() == BT_STREAM_TRIGGER_STATUS_INIT){
        return true;
    }else{
        return false;
    }
}


