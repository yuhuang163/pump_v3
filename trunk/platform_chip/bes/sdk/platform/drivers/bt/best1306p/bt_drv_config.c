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
#include <besbt_string.h>
#include "bt_drv.h"
#include "hal_chipid.h"

#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_internal.h"
#include <string.h>

//CHIP related
#include "bt_drv_1306p_internal.h"
#include "bt_1306p_reg_map.h"
#include "bt_drv_1306p_config.h"
#include CHIP_SPECIFIC_HDR(bt_drv_modem_reg_map)

const int8_t btdrv_rf_env[]=
{
    0x01,0x00,  //rf api
    0x01,   //rf env
    185,     //rf length
    BT_MAX_TX_PWR_IDX,     //txpwr_max
    -1,    ///rssi high thr
    -2,   //rssi low thr
    -100,  //rssi interf thr
    0xf,  //rssi interf gain thr
    2,  //wakeup delay
    BLE_MIN_TX_PWR_IDX, //ble_txpwr_min
    BLE_MAX_TX_PWR_IDX, //ble_txpwr_max
    0xe8,0x3,    //ble agc inv thr
    BTC_HW_AGC_ENABLE_FLAG,
    0xff,//sw gain set
    0xff,    //sw gain set
    -85,//bt_inq_page_iscan_pscan_dbm
    0x7f,//ble_scan_adv_dbm
    BT_MIN_TX_PWR_IDX,    //txpwr_min
    -20,   //le_rssi_high_thr in dbm
    -60,   //le_rssi_low_thr in dbm
    1,  //bt interfere  detector en
    0,  //ble interfere detector en

#ifdef __HW_AGC__
    49,0,0,
    43,0,0,
    37,0,0,
    32,0,0,
    27,0,0,
    22,0,0,
    16,0,0,
    5,0,0,  //rx hwgain tbl ptr hw
#else
    0,0,0,
    3,3,12,
    6,6,28,
    9,9,28,
    12,12,28,
    15,15,28,
    18,18,28,
    21,21,28,
#endif

    0x7f,24,0x7f,
    0x7f,27,0x7f,
    0x7f,30,0x7f,
    0x7f,33,0x7f,
    0x7f,36,0x7f,
    0x7f,39,0x7f,
    0x7f,42,0x7f,  //rx hwgain tbl ptr sw

#ifdef BT_RF_I2V_BYPASS
    51,0,-80,
    36,0,-80,
    33,0,-80,
    27,0,-80,
    23,0,-80,
    18,0,-80,
    13,0,-80,
    4,0,-80,
#else
    52,0,-80,
    47,0,-80,
    39,0,-80,
    37,0,-80,
    34,0,-80,
    29,0,-80,
    22,0,-80,
    14,0,-80,
#endif


    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,  //rx gain tbl ptr

#ifdef BT_RF_I2V_BYPASS
    -83,-83,
    -80,-80,
    -73,-73,
    -69,-69,
    -65,-65,
    -60,-60,
    -52,-52,
    -25,0x7f,
#else
    -92,-92,
    -84,-84,
    -81,-81,
    -76,-76,
    -74,-74,
    -69,-69,
    -62,-62,
    -25,0x7f,
#endif


    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,    //rx gain ths tbl ptr

    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,1,
    0,2,
    0,2,
    0,2,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,    //flpha filter factor ptr
    -23,-20,-17,-14,-11,-8,-5,-2,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,   //tx pw onv tbl ptr
};

const int8_t btdrv_rxgain_gain_ths_3m[] = {
#ifdef BT_RF_I2V_BYPASS
    -74,-74,
    -71,-71,
    -64,-64,
    -60,-60,
    -56,-56,
    -51,-51,
    -43,-43,
    -25,0x7f,
#else
    -83,-83,
    -75,-75,
    -72,-72,
    -67,-67,
    -65,-65,
    -60,-60,
    -53,-53,
    -25,0x7f,
#endif
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,    //rx gain ths tbl ptr
};

const int8_t btdrv_rxgain_ths_tbl_le[0xf * 2] = {
    -92,-92,
    -84,-84,
    -81,-81,
    -76,-76,
    -74,-74,
    -69,-69,
    -62,-62,
    -25,0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f, //ble rx gain ths tbl ptr
};

const int8_t btdrv_rxgain_ths_tbl_le_2m[0xf * 2] = {
    -92,-92,
    -84,-84,
    -81,-81,
    -76,-76,
    -74,-74,
    -69,-69,
    -62,-62,
    -25,0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f, //ble rx gain ths tbl ptr
};

const int8_t btdrv_rxgain_ths_tbl_ecc[0xf * 2] = {
    -92,-92,
    -84,-84,
    -75,-75,
    -70,-70,
    -62,-62,
    -47,-47,
    -37,-37,
    -25,0x7f,

    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,    //ECC rx gain ths tbl ptr
};

//BT txpwr convert
int8_t  btdrv_txpwr_conv_tbl[8] = {
        [0] = -4,
        [1] = 0,
        [2] = 3,
        [3] = 7,
        [4] = 11,
        [5] = 16,
        [6] = 16,
        [7] = 16
};

//ble txpwr convert
int8_t  btdrv_ble_txpwr_conv_tbl[8] = {
        [0] = -4,
        [1] = 0,
        [2] = 3,
        [3] = 7,
        [4] = 11,
        [5] = 16,
        [6] = 16,
        [7] = 16
};

const int8_t btdrv_afh_env[] =
{
    0x02,0x00,   //afh env
    0x00,      //ignore
    33,          //length
    5,   //nb_reass_chnl
    10,  //win_len
    -70,  //rf_rssi_interf_thr
    10,  //per_thres_bad
    20,  //reass_int
    20,   //n_min
    20,   //afh_rep_intv_max
    96,    //ths_min
    2,   //chnl_assess_report_style
    15,  //chnl_assess_diff_thres
    60, // chnl_assess_interfere_per_thres_bad
    9,  //chnl_assess_stat_cnt_max
    -9,  //chnl_assess_stat_cnt_min
    1,2,3,2,1,   //chnl_assess_stat_cnt_inc_mask[5]
    1,2,3,2,1,    //chnl_assess_stat_cnt_dec_mask
    0xd0,0x7,      //chnl_assess_timer
    -48,        //chnl_assess_min_rssi
    0x64,0,   //chnl_assess_nb_pkt
    0x32,0,     //chnl_assess_nb_bad_pkt
    6,    //chnl_reassess_cnt_val
    0x3c,0,     //chnl_assess_interfere_per_thres_bad
};

const uint8_t lpclk_drift_jitter[] =
{
    0xfa,0x00,  //  drift  250ppm
    0x0a,0x00    //jitter  +-10us

};

uint8_t  sleep_param[] =
{
    0,    // sleep_en;
    1,    // exwakeup_en;
    0xd0,0x7,    //  lpo_calib_interval;   lpo calibration interval
    0x32,0,0,0,    // lpo_calib_time;  lpo count lpc times
};

void btdrv_sleep_config(uint8_t sleep_en)
{
    sleep_param[0] = sleep_en;
    btdrv_send_cmd(HCI_DBG_SET_SLEEP_SETTING_CMD_OPCODE,8,sleep_param);
    btdrv_delay(1);
}

const uint8_t  sync_config[] =
{
    1,1,   //sco path config   0:hci  1:pcm
    0,      //sync use max buff length   0:sync data length= packet length 1:sync data length = host sync buff len
    0,        //cvsd bypass     0:cvsd2pcm   1:cvsd transparent
};

//pcm general ctrl
#define PCM_PCMEN_POS            15
#define PCM_LOOPBCK_POS          14
#define PCM_MIXERDSBPOL_POS      11
#define PCM_MIXERMODE_POS        10
#define PCM_STUTTERDSBPOL_POS    9
#define PCM_STUTTERMODE_POS      8
#define PCM_CHSEL_POS            6
#define PCM_MSTSLV_POS           5
#define PCM_PCMIRQEN_POS         4
#define PCM_DATASRC_POS          0


//pcm phy ctrl
#define PCM_LRCHPOL_POS     15
#define PCM_CLKINV_POS      14
#define PCM_IOM_PCM_POS     13
#define PCM_BUSSPEED_LSB    10
#define PCM_SLOTEN_MASK     ((uint32_t)0x00000380)
#define PCM_SLOTEN_LSB      7
#define PCM_WORDSIZE_MASK   ((uint32_t)0x00000060)
#define PCM_WORDSIZE_LSB    5
#define PCM_DOUTCFG_MASK    ((uint32_t)0x00000018)
#define PCM_DOUTCFG_LSB     3
#define PCM_FSYNCSHP_MASK   ((uint32_t)0x00000007)
#define PCM_FSYNCSHP_LSB    0

/// Enumeration of PCM status
enum PCM_STAT
{
    PCM_DISABLE = 0,
    PCM_ENABLE
};

/// Enumeration of PCM channel selection
enum PCM_CHANNEL
{
    PCM_CH_0 = 0,
    PCM_CH_1
};

/// Enumeration of PCM role
enum PCM_MSTSLV
{
    PCM_SLAVE = 0,
    PCM_MASTER
};

/// Enumeration of PCM data source
enum PCM_SRC
{
    PCM_SRC_DPV = 0,
    PCM_SRC_REG
};

/// Enumeration of PCM left/right channel selection versus frame sync polarity
enum PCM_LR_CH_POL
{
    PCM_LR_CH_POL_RIGHT_LEFT = 0,
    PCM_LR_CH_POL_LEFT_RIGHT
};

/// Enumeration of PCM clock inversion
enum PCM_CLK_INV
{
    PCM_CLK_RISING_EDGE = 0,
    PCM_CLK_FALLING_EDGE
};

/// Enumeration of PCM mode selection
enum PCM_MODE
{
    PCM_MODE_PCM = 0,
    PCM_MODE_IOM
};

/// Enumeration of PCM bus speed
enum PCM_BUS_SPEED
{
    PCM_BUS_SPEED_128k = 0,
    PCM_BUS_SPEED_256k,
    PCM_BUS_SPEED_512k,
    PCM_BUS_SPEED_1024k,
    PCM_BUS_SPEED_2048k
};

/// Enumeration of PCM slot enable
enum PCM_SLOT
{
    PCM_SLOT_NONE = 0,
    PCM_SLOT_0,
    PCM_SLOT_0_1,
    PCM_SLOT_0_2,
    PCM_SLOT_0_3
};

/// Enumeration of PCM word size
enum PCM_WORD_SIZE
{
    PCM_8_BITS = 0,
    PCM_13_BITS,
    PCM_14_BITS,
    PCM_16_BITS
};

/// Enumeration of PCM DOUT pad configuration
enum PCM_DOUT_CFG
{
    PCM_OPEN_DRAIN = 0,
    PCM_PUSH_PULL_HZ,
    PCM_PUSH_PULL_0
};

/// Enumeration of PCM FSYNC physical shape
enum PCM_FSYNC
{
    PCM_FSYNC_LF = 0,
    PCM_FSYNC_FR,
    PCM_FSYNC_FF,
    PCM_FSYNC_LONG,
    PCM_FSYNC_LONG_16
};

const uint32_t pcm_setting[] =
{
//pcm_general_ctrl
    (PCM_DISABLE<<PCM_PCMEN_POS) |                      //enable auto
    (PCM_DISABLE << PCM_LOOPBCK_POS)  |                 //LOOPBACK test
    (PCM_DISABLE << PCM_MIXERDSBPOL_POS)  |
    (PCM_DISABLE << PCM_MIXERMODE_POS)  |
    (PCM_DISABLE <<PCM_STUTTERDSBPOL_POS) |
    (PCM_DISABLE <<PCM_STUTTERMODE_POS) |
    (PCM_CH_0<< PCM_CHSEL_POS) |
    (PCM_MASTER<<PCM_MSTSLV_POS) |                      //BT clock
    (PCM_DISABLE << PCM_PCMIRQEN_POS) |
    (PCM_SRC_DPV<<PCM_DATASRC_POS),

//pcm_phy_ctrl
    (PCM_LR_CH_POL_RIGHT_LEFT << PCM_LRCHPOL_POS) |
    (PCM_CLK_FALLING_EDGE << PCM_CLKINV_POS) |
    (PCM_MODE_PCM << PCM_IOM_PCM_POS) |
    (PCM_BUS_SPEED_2048k << PCM_BUSSPEED_LSB) |         //8k sample rate; 2048k = slot_num * sample_rate * bit= 16 * 8k * 16
    (PCM_SLOT_0_1 << PCM_SLOTEN_LSB) |
    (PCM_16_BITS << PCM_WORDSIZE_LSB) |
    (PCM_PUSH_PULL_0 << PCM_DOUTCFG_LSB) |
    (PCM_FSYNC_LF << PCM_FSYNCSHP_LSB),
};

const uint8_t local_feature[] =
{
#if defined(__3M_PACK__)
    0xBF, 0xeE, 0x4D,0xFe,0xdb,0xFd,0x7b,0x87
#else
    0xBF, 0xeE, 0x4D,0xFa,0xdb,0xbd,0x7b,0x87

    //0xBF,0xFE,0x4D,0xFa,0xDB,0xFd,0x73,0x87   // disable simple pairing
#endif
};

const uint8_t local_ex_feature_page2[] =
{
    2,   //page
    0x1f,0x03,0x00,0x00,0x00,0x00,0x00,0x00,   //page 2 feature
};

const uint8_t bt_rf_timing[] =
{
    0x37,// rxpwrupct;
    0x0C,// txpwrdnct;
    0x2A,// txpwrupct;
    0x00,// rxpathdly;
    0x10,// txpathdly;
    0x00,// sync_position;
    0x18,// edr_rxgrd_timeout;
};

// LE 1M:uncoded PHY at 1Mbps
// LE 2M:uncoded PHY at 2Mbps
const uint8_t ble_rf_timing[] =
{
    0x00,  //LE 1M syncposition0
    0x37,  //LE 1M rxpwrup0
    0x0C,  //LE 1M txpwrdn0
    0x2A,  //LE 1M txpwrup0

    0x00,  //LE 2M syncposition1
    0x37,  //LE 2M rxpwrup1
    0x0C,  //LE 2M txpwrdn1
    0x2A,  //LE 2M txpwrup1

    0x00,  //coded PHY at 125kbps and 500kbps syncposition2
    0x41,  //coded PHY at 125kbps and 500kbps rxpwrup2
    0x0C,  //coded PHY at 125kbps txpwrdn2
    0x2A,  //coded PHY at 125kbps txpwrup2

    0x0C,  //coded PHY at 500kbps txpwrdn3
    0x2A,  //coded PHY at 500kbps txpwrup3

    0x00,  //LE 1M rfrxtmda0
    0x06,  //LE 1M rxpathdly0
    0x0a,  //LE 1M txpathdly0

    0x00,  //LE 2M rfrxtmda1
    0x00,  //LE 2M rxpathdly1
    0x08,  //LE 2M txpathdly1

    0x15,  //coded PHY at 125kbps rxflushpathdly2
    0xa0,  //coded PHY at 125kbps rfrxtmda2
    0x14,  //coded PHY at 125kbps rxpathdly2
    0x0b,  //coded PHY at 125kbps txpathdly2

    0x15,  //coded PHY at 500kbps rxflushpathdly3
    0x00,  //coded PHY at 500kbps rfrxtmda3
    0x0c,  //coded PHY at 500kbps txpathdly3

    /*   AoA/AoD Registers */
    0x08,  //rxsampstinst01us
    0x18,  //rxswstinst01us
    0x19,  //txswstinst01us

    0x08,  //rxsampstinst02us
    0x18,  //rxswstinst02us
    0x19,  //txswstinst02us

    0x08,  //rxsampstinst11us
    0x18,  //rxswstinst11us
    0x19,  //txswstinst11us

    0x08,  //rxsampstinst12us
    0x18,  //rxswstinst12us
    0x19,  //txswstinst12us

    0x00,  //rxprimidcntlen
    0x00,  //rxprimantid
    0x00,  //txprimidcntlen
    0x00,  //txprimantid
};

const uint8_t bt_common_setting_1306p[38] =
{
    0x00,0x00, //tports_level
    0xb0,0x02, //comp_id
    0x00,0x08, //max_hdc_adv_dur in slots
    0x40,0x05,//sniff_interval_max
    0x03, //trace_level
    0x01, //trace_output
    0x06, //wesco_nego
    0x01, //esco_retx_after_establish
    0x00, //sco_start_delay
    0x01, //msbc_pcmdout_zero_flag
    0x01, //master_2_poll
    0x01, //pca_disable_in_nosync
    BT54_VERSION, //version_major
    0x07, //version_minor
    0x13, //version_build chip=1306p
    0x00, //address_reset
    0x02, //ibrt_relay_traffic
    0x05, //lmp_to_before_complete
    0x08, //fastpcm_interval
    0x01, //lm_env_reset_local_name
    0xc8, //seq_error_num
    0x0a, //delay_process_lmp_to 10*100 halt slot
    0x01, //ignore_pwr_ctrl_sm_state
    0x01, //iso_host_to_controller_flow
    0x03, //enable_assert
    0x00, //ble_aux_adv_ind_update
    0x10, //page_max_duration_in_a2dp
    0x06, //ble_cis_conn_event_cnt_distance (in connect interval)
    0x0c, //ble_ci_alarm_init_distance (in half slots)
    0x00, //btc_send_name_req
    0x00, //read_name_from_peer
    0x01, //ble_adv_buf_malloc
    0x20, //pscan_gap_slot_in_a2dp
    0x14, //page_gap_slot_in_a2dp
};

const uint8_t bt_common_setting_t2_1306p[66] =
{
    0x64,0x00, //mhdt_instant;
    0xff,0x00, //walkie_talkie_pa_data_size;
    0x40,0x1f, //afh_reporting_interval; // slot unit
    0xf4,0x01, //ecc_no_sync_timeout;
    0x91,0x00, //bw2m_2dh_len;
    0xdc,0x00, //bw2m_3dh_len;
    0x40,0x06, //tws_resync_clk_info;
    0x00,0x00, //ibrt_salve_extra_window;
    0x40,0x01, //twp_a2dp_heavy_interval;
    0x00,     //bw2m_enable;
    0x00,     //bw2m_test_flag;
    0x01,     //change_txtype_mode;
    0x01,     //fa_rx_isr_en;
    -90 ,     //ecc_no_sync_rssi;
    0x0a,     //ecc_no_sync_cnt_thr;
    -60 ,     //antenna_low_thr;
    0x00,     //page_no_prio_inc;
    0x00,     //pscan_no_prio_inc;
    0xff,0xff,0xff,0xff,0xff,  //dbg_evt_filter[HCI_FILTER]
    0x01,     //bt_sync_found_hecerror_check;
    0x05,     //wait_setup_cmp_to;
    0x01,     //ignore_pa_status_for_pa_data;
    0x01,     //reject_setup_sync_cmd;
    0x05,     //(s)wait_tx_empty_to;
    0x01,     //sco_open_rx_isr;
    0x00,     //twp_enable;
    0x00,     //twp_a2dp_heavy_enable;
    0x00,     //ibrt_sync_mobile_clk_enable;
    0x14,     //(us)ibrt_sync_mobile_clk_diff_in_hus;
    0x00,     //ble2bt_isr_en;
    0x00,     //bt2ble_isr_en;
    0x00,     //en_sec_con_base_on_le_aud_sup;
    0x6e,     //tws_coex_with_sniff_prio;
    0x00,     //tws_resync_clk_en;
    0x00,     //bt_master_sleep_en;
    0x00,     //bt_master_tx_silence_en;
    0x00,     //check_host_iso_packet_late;
    0x00,     //compatible_fa_window;
    0x00,     //walkie_talkie_pa_offset_info;
    0x00,     //cis_sco_coex_anchor_tuning;
    0x00,     //ull_cig_enable;
    0x01,     //bool tws_disallow_secure_connect;
    0x00,     //ch_idx_enable; //56
    0x00,     //sleep_status_report_en;
    0x01,     //bool combine_table;
    0x0f,     //tws_sleep_dura_dec;
    0x05,     //normal_sleep_dura_inc;
    0x03,     //vendor_evt_itf;
    0x00,     //mic_data_via_tws;
    0x00,     //high_eff_tx_pwr_cntl;
    0x00,     //multi_cis_sch_plan_en
    0x20,0x00,//multi_cis_iso_interval
};

const uint8_t bt_sche_setting_1306p[24] =
{
    0x9c,0x00, //acl_interv_in_ibrt_sco_mode
    0x68,0x00, //acl_interv_in_ibrt_normal_mode
    0x40,0x06, //acl_switch_to_threshold
    0x60,0x00, //sniff_priority_interv_thd
    0x64,0x00, //unsniff_trans_interval
    0xff, //music_playing_link
    0x08, //default_tpoll
    0x08, //acl_slot_in_ibrt_mode
    0x01, //ble_wrong_packet_lantency
    0x02, //double_pscan_in_sco
    0x06, //reduce_rext_for_sniff_thd
    0x06, //reduce_att_for_sco_thd
    0x01, //reduce_att_space_adjust
    0x01, //bandwidth_check_ignore_retx
    0x02, //ble_slot
    0x02, //sniff_max_frm_time
    0x01, //unsniff_schdule_more
    120,  //tws_acl_prio_in_sco
    144,  //tws_acl_prio_in_normal
};

const uint8_t bt_ibrt_setting_1306p[17] =
{
    0x01, //hci_auto_accept_tws_link_en
    0x6c, //sync_win_size hus
    0x16, //magic_cal_bitoff
    0x3f, //role_switch_packet_br
    0x2b, //role_switch_packet_edr
    0x00, //relay_sam_info_in_start_snoop
    0x02, //slave_rx_traffic_siam
    0x02, //ibrt_lmp_to
    0x00, //fa_use_twslink_table
    0x06, //ibrt_afh_instant_adjust
    0x06, //ibrt_detach_send_instant
    0x03, //ibrt_detach_receive_instant
    0x03, //ibrt_detach_txcfm_instant
    0x01, //ibrt_auto_accept_sco
    0x03, //ibrt_second_sco_decision
    0x01, //accept_remote_enter_sniff
    0x96, //avg_rssi_cnt
};

const uint8_t bt_hw_feat_setting_1306p[36] =
{
    0x55,0x00,0x00,0x00, //fa_to_type
    0x50,0x00,0x00,0x00, //fa_disable_type
    0x05,0x00,//apb_rf_reg_table_num
    0x01, //rxheader_int_en
    0x00, //rxdone_bt_int_en
    0x00, //txdone_bt_int_en
    0x01, //rxsync_bt_int_en
    0x01, //rxsync_ble_int_en
#ifdef __NEW_SWAGC_MODE__
    0x01, //bt_sync_swagc_en
#else
    0x00, //bt_sync_swagc_en
#endif
    0x00, //le_sync_swagc_en
    0x00, //fa_to_en
#ifdef __BES_FA_MODE__
    0x01, //fa_dsb_en
#else
    0x00, //fa_dsb_en
#endif
    0x0F, //fa_to_num
#ifdef __FIX_FA_RX_GAIN___
    0x01, //fa_rxgain
#else
    0xff, //fa_rxgain
#endif
    0xff, //fa_txpwr
    0x00, //rx_noise_chnl_assess_en
    0x14, //rx_noise_thr_good
    0x0a, //rx_noise_thr_bad
    0x00, //snr_chnl_assess_en
    0x14, //snr_good_thr_value
    0x28, //snr_bad_thr_value
    0x00, //new_agc_adjust_dbm
    0x0f, //ble_rssi_noise_thr
    0x01, //ble_rxgain_adjust_once
    0x00, //trig_open_pcm_flag
    0x00, //sco_sw_mute_en
    0x00, //hwspi_bt_en
    0x00, //hwspi_ble_en
    0x00, //iso_use_intersys2
};

const uint8_t bt_txrx_gain_setting[] =
{
    BT_INIT_TX_PWR_IDX,//bt_init_txpwr
    0x01,//bt_inq_rxgain
    0x01,//bt_page_rxgain
    0x06,//bt_page_txpwr
    0x01,//bt_iscan_rxgain
    0xff,//bt_iscan_txpwr
    0x01,//bt_pscan_rxgain
    0xff,//bt_pscan_txpwr
    0x00,//bt_ibrt_rxgain
    0xff,//ble_adv_txpwr
    0x00,//ble_adv_rxgain
    0xff,//ble_bis_txpwr
    0x00,//ble_bis_rxgain
    0xff,//ble_adv_per_txpwr
    0x00,//ble_adv_per_rxgain
    0xff,//ble_testmode_txpwr
    0x00,//ble_testmode_rxgain
    0xff,//ble_con_txpwr
    0x00,//ble_con_rxgain
    0xff,//ble_con_init_txpwr
    0x00,//ble_con_init_rxgain
    0xff,//ble_scan_txpwr
    0x00,//ble_scan_rxgain
};

const uint8_t bt_peer_txpwr_dft_thr[]=
{
    0x64,00,//uint16_t rssi_avg_nb_pkt;
    -1,//rssi_high_thr;
    -2,//rssi_low_thr;
    5,//rssi_below_low_thr;
    50,//unused rssi_interf_thr;
};

const struct rssi_txpower_link_thd tws_link_txpwr_thd =
{
    0x32,//uint16_t rssi_avg_nb_pkt;
    -40,//rssi_high_thr;
    -50,//rssi_low_thr;
    5,//rssi_below_low_thr;
    50,//unused rssi_interf_thr;
};

const struct rssi_txpower_link_thd* btdrv_get_tws_link_txpwr_thd_ptr(void)
{
    return &tws_link_txpwr_thd;
}

const uint8_t bt_sw_rssi_setting[] =
{
     0,  //.sw_rssi_en = false
    80,00,00,00,//.link_agc_thd_mobile = 80,
    100,00,00,00,//.link_agc_thd_mobile_time = 100,
    80,00,00,00,//.link_agc_thd_tws = 80,
    100,00,00,00,//.link_agc_thd_tws_time = 100,
    3,//.rssi_mobile_step = 3,
    3,//.rssi_tws_step = 3,
    -100,//.rssi_min_value_mobile = -100,
    -100,//.rssi_min_value_tws = -100,

    0,//.ble_sw_rssi_en = 0,
    80,00,00,00,//.ble_link_agc_thd = 80,
    100,00,00,00,//.ble_link_agc_thd_time = 100,//(in BT half-slots)
    3,//.ble_rssi_step = 3,
    -100,//.ble_rssidbm_min_value = -100,

    1,//.bt_no_sync_en = 1,
    -90,//.bt_link_no_sync_rssi= -90,
    80,00,//.bt_link_no_snyc_thd = 0x50,
    200,00,//.bt_link_no_sync_timeout = 200,

    1,//.ble_no_sync_en = 1,
    -90,//.ble_link_no_sync_rssi= -90,
    20,00,//.ble_link_no_snyc_thd = 20,
    0x20,0x03,//.ble_link_no_sync_timeout = 800,
};

struct bt_cmd_chip_config_t g_bt_drv_btstack_chip_config = {
    HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE,
    HCI_DBG_SET_SCO_SWITCH_CMD_OPCODE,
};

#ifdef __BESTRX_SUPPORT__
static void btdrv_bestrx_en(void)
{
    struct hci_dbg_set_bestrx_en_cmd param;

    param.enable = true;
    param.mode = 0;

    btdrv_send_cmd(HCI_DBG_SET_BESTRX_EN_CMD_OPCODE, sizeof(struct hci_dbg_set_bestrx_en_cmd),(const uint8_t *)&param);
}

void btdrv_bestrx_rf_timin_config(void)
{
    struct hci_dbg_set_bestrx_rf_setting_cmd param;
    param.offset = BESTRX_OFFSET;
    param.txsetuptime = BESTRX_TXPWRUP_CNT;
    param.rxsetuptime = BESTRX_RXPWRUP_CNT;
    param.rxwin = BESTRX_MASTER_RX_WINSIZE;
    param.ifs = BESTRX_TIFS;

    btdrv_send_cmd(HCI_DBG_SET_BESTRX_RF_SETTING_CMD_OPCODE, sizeof(struct hci_dbg_set_bestrx_rf_setting_cmd),(const uint8_t *)&param);
}

void btdrv_bestrx_config_init(void)
{
    btdrv_bestrx_en();
    btdrv_bestrx_rf_timin_config();
}
#endif

static BTDRV_CFG_TBL_STRUCT  btdrv_cfg_tbl[] = {
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_FUNC_PATCH_CMD_OPCODE, 0, NULL},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_SLEEP_SETTING_CMD_OPCODE,sizeof(sleep_param),sleep_param},
    {BTDRV_CONFIG_ACTIVE,HCI_RD_LOCAL_VER_INFO_CMD_OPCODE, 0, NULL},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_LOCAL_FEATURE_CMD_OPCODE,sizeof(local_feature),local_feature},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_SETTING_CMD_OPCODE,sizeof(bt_common_setting_1306p),bt_common_setting_1306p},
    //{BTDRV_CONFIG_ACTIVE,HCI_DBG_BT_COMMON_SETTING_T2_CMD_OPCODE,sizeof(bt_common_setting_t2_2007p),bt_common_setting_t2_2007p},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_SCHE_SETTING_CMD_OPCODE,sizeof(bt_sche_setting_1306p),bt_sche_setting_1306p},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_IBRT_SETTING_CMD_OPCODE,sizeof(bt_ibrt_setting_1306p),bt_ibrt_setting_1306p},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_HW_FEAT_SETTING_CMD_OPCODE,sizeof(bt_hw_feat_setting_1306p),bt_hw_feat_setting_1306p},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_CUSTOM_PARAM_CMD_OPCODE,189,(uint8_t *)&btdrv_rf_env},
#ifdef _SCO_BTPCM_CHANNEL_
    {BTDRV_CONFIG_INACTIVE,HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE,sizeof(sync_config),(uint8_t *)&sync_config},
    {BTDRV_CONFIG_INACTIVE,HCI_DBG_SET_PCM_SETTING_CMD_OPCODE,sizeof(pcm_setting),(uint8_t *)&pcm_setting},
#endif
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_LOCAL_EX_FEATURE_CMD_OPCODE,sizeof(local_ex_feature_page2),(uint8_t *)&local_ex_feature_page2},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_RF_TIMING_CMD_OPCODE,sizeof(bt_rf_timing),(uint8_t *)&bt_rf_timing},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BLE_RF_TIMING_CMD_OPCODE,sizeof(ble_rf_timing),(uint8_t *)&ble_rf_timing},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_RSSI_TX_POWER_DFT_THR_CMD_OPCODE,sizeof(bt_peer_txpwr_dft_thr),(uint8_t *)&bt_peer_txpwr_dft_thr},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_BLE_TXRX_GAIN_CMD_OPCODE,sizeof(bt_txrx_gain_setting),(uint8_t *)bt_txrx_gain_setting},
};

//factor=N/512
void btdrv_set_bdr_ble_txpower(uint8_t txpwr_idx, uint16_t n)
{
    uint32_t reg_base = MODEM_BLE_TX_POWR_ADDR;
    uint32_t reg;
    if(txpwr_idx > 7)
    {
        BT_DRV_TRACE(1, "%s txpwr idx err:%d\n", __func__, txpwr_idx);
        return;
    }

    if(n > 1023)
    {
        BT_DRV_TRACE(1, "%s n err:%d\n", __func__, n);
        return;
    }

    reg = (txpwr_idx / 3) * 4 + reg_base;

    BTDIGITAL_REG_SET_FIELD(reg, 0x3ff, (txpwr_idx % 3) * 10, n);
}

/*
*   TWOSC: Time to wake-up osc_en before deepsleep_time expiration
*   TWRM  : Time to wake-up radio module(no used)
*   TWEXT : Time to wake-up osc_en on external wake-up request
*/
static void btdrv_hci_init_sleep_wakeup_param(void)
{
   uint16_t twosc_cnt  = LPU_TIMER_US(BT_CMU_OSC_READY_TIMEOUT_US);
   uint16_t wait_26m_cnt = LPU_TIMER_US(BT_CMU_26M_READY_TIMEOUT_US);
   uint16_t twext_cnt = LPU_TIMER_US(BT_CMU_WEXT_READY_TIMEOUT_US);

    struct hci_dbg_set_sleep_para_cmd sleep_wakeup_param;
    sleep_wakeup_param.twrm = twosc_cnt; //no used
    sleep_wakeup_param.twosc = twosc_cnt;
    sleep_wakeup_param.twext = twext_cnt;
    sleep_wakeup_param.rwip_prog_delay = IP_PROG_DELAY_DFT;
    sleep_wakeup_param.clk_corr = 2;
    sleep_wakeup_param.wait_26m_cnt_us = wait_26m_cnt;
    sleep_wakeup_param.cpu_idle_en = 1;
    sleep_wakeup_param.reserved = 0;//no used
    sleep_wakeup_param.poweroff_flag = BTC_LP_MODE;

    btdrv_send_cmd(HCI_DBG_SET_WAKEUP_TIME_CMD_OPCODE, sizeof(struct hci_dbg_set_sleep_para_cmd),(const uint8_t *)&sleep_wakeup_param);
    btdrv_delay(1);
    BT_DRV_TRACE(1, "%s,wait26m cycle=%d,twosc=%d",__func__, wait_26m_cnt, twosc_cnt);
}

void btdrv_hci_set_ble_rpl_tx_pwr_conv_tbl(void)
{
    struct dbg_set_ble_rpl_tx_pwr_conv_tbl_cmd ble_rpl_tx_pwr;
    memcpy(&ble_rpl_tx_pwr.rf_rpl_tx_pw_conv_tbl[0], &btdrv_txpwr_conv_tbl[0], 8);
    memcpy(&ble_rpl_tx_pwr.rf_ble_tx_pw_conv_tbl[0], &btdrv_ble_txpwr_conv_tbl[0], 8);
    btdrv_send_cmd(HCI_DBG_SET_BLE_PRL_TX_PWR_CONV_TBL_CMD_OPCODE, sizeof(struct dbg_set_ble_rpl_tx_pwr_conv_tbl_cmd),(const uint8_t *)&ble_rpl_tx_pwr);
    btdrv_delay(1);
}
#ifdef __BT_FAST_ACK_EN__

void btdrv_fa_syncword_phy_setting(uint8_t syncword_len)
{
    if (syncword_len == FA_SYNCWORD_32BIT)
    {
        //PHY using 32 bit FA
        BTDIGITAL_REG_SET_FIELD(BESMDM_FASTACK_TXEX_ADDR, 1, 29, 1);
        BTDIGITAL_REG_SET_FIELD(BESMDM_FASTACK_TXEX_ADDR, 1, 30, 1);
        besmdm_sync_parameter_4_pack(0x3, 0x3, 0x64, 0x32);
    }
    else if (syncword_len == FA_SYNCWORD_64BIT)
    {
        //PHY using 64 bit FA
        BTDIGITAL_REG_SET_FIELD(BESMDM_FASTACK_TXEX_ADDR, 1, 29, 0);
        BTDIGITAL_REG_SET_FIELD(BESMDM_FASTACK_TXEX_ADDR, 1, 30, 0);
        besmdm_sync_parameter_4_pack(0x3, 0xA, 0xC8, 0x64);
    }
}

void btdrv_fa_config_tx_gain(bool tx_gain_en, uint8_t tx_gain_idx)//false :disable tx gain
{
    if(tx_gain_en == true)
    {
        bt_bes_cntl3_reg_fa_txpwr_en_setf(1);
        bt_bes_cntl3_reg_fatxpwr_setf(tx_gain_idx);
    }
    else
    {
        bt_bes_cntl3_reg_fa_txpwr_en_setf(0);
    }
}

void btdrv_fa_config_rx_gain(bool rx_gain_en, uint8_t rx_gain_idx)//false: disable rx gain
{
    if(rx_gain_en == true)
    {
        bt_bes_cntl3_reg_fa_gain_en_setf(1);
        bt_bes_cntl3_reg_farxgain_setf(rx_gain_idx);
    }
    else
    {
        bt_bes_cntl3_reg_fa_gain_en_setf(0);
    }
}

void btdrv_fa_multi_mode0_enable(bool fa_multi_mode0_en)
{
    if(fa_multi_mode0_en)
    {
        bt_bes_cntl5_multifa_mode_0_setf(1);
        bt_bes_facntl1_reg_fa_rxwinsz_setf(0x66);
    }
    else
    {
        bt_bes_cntl5_multifa_mode_0_setf(0);
    }
}

void btdrv_fa_multi_mode1_enable(bool fa_multi_mode1_en, uint8_t fa_multi_tx_count)
{
    if(fa_multi_mode1_en)
    {
        bt_bes_cntl5_multifa_mode_1_setf(1);
        if(fa_multi_tx_count == 2)
        {
            bt_bes_cntl5_mode_1_fa_times_setf(2);
            bt_bes_facntl1_reg_fa_rxwinsz_setf(0x31);
        }

        if(fa_multi_tx_count == 3)
        {
            bt_bes_cntl5_mode_1_fa_times_setf(3);
            bt_bes_facntl1_reg_fa_rxwinsz_setf(0x5b);
        }
    }
    else
    {
        bt_bes_cntl5_multifa_mode_1_setf(0);
    }
}

void btdrv_fa_margin_timig_setting(uint8_t margin)
{
    bt_bes_cntl2_reg_cnt_pkt_us_setf((margin&0x1f));
    bt_trigreg_reg_cnt_pkt_us_h_setf((margin&0xe0)>>5);
}

void btdrv_fa_basic_config(btdrv_fa_basic_config_t* p_fa_basic_config)
{
    if(p_fa_basic_config != NULL)
    {
        //fa 2M phy
        if(p_fa_basic_config->fa_2m_mode)
        {
            bt_bes_cntl2_reg_fatxpwrupct_setf(FA_BW2M_TXPWRUP_TIMING);
            bt_bes_cntl2_reg_farxpwrupct_setf(FA_BW2M_RXPWRUP_TIMING);
        }
        else
        {
            bt_bes_cntl2_reg_fatxpwrupct_setf(FA_TXPWRUP_TIMING);
            bt_bes_cntl2_reg_farxpwrupct_setf(FA_RXPWRUP_TIMING);
        }
        //fa 2M mode select
        bt_bes_facntl0_reg__2m_fa_mode_setf(p_fa_basic_config->fa_2m_mode);
        //fa phy setting
        btdrv_fa_syncword_phy_setting(p_fa_basic_config->syncword_len);

        //fa TX power gain set
        btdrv_fa_config_tx_gain(p_fa_basic_config->fa_tx_gain_en, p_fa_basic_config->fa_tx_gain_idx);
#ifdef __FIX_FA_RX_GAIN___
        //fix fa rx gain
        btdrv_fa_config_rx_gain(p_fa_basic_config->fa_rx_gain_en, p_fa_basic_config->fa_rx_gain_idx);
#endif
        //fa syncword len mode
        bt_bes_facntl0_reg_fasync_mode_setf(p_fa_basic_config->syncword_len);
        //fa win size
        bt_bes_facntl1_reg_fa_rxwinsz_setf(p_fa_basic_config->fa_rx_winsz);

        //only use E-Fsync for FA module

        //fa multi mode 0
        btdrv_fa_multi_mode0_enable(p_fa_basic_config->fa_multi_mode0_en);
        //fa multi mode 1
        if(p_fa_basic_config->fa_multi_mode1_en)
        {
            btdrv_fa_multi_mode1_enable(true, p_fa_basic_config->fa_multi_tx_count);
        }
        else
        {
            btdrv_fa_multi_mode1_enable(false,p_fa_basic_config->fa_multi_tx_count);
        }
    }

    btdrv_fa_margin_timig_setting(FA_CNT_PKT_US);
}

void btdrv_fast_ack_config(void)
{
    btdrv_fa_basic_config_t fa_config;
    //fast ack config
    fa_config.syncword_len = FA_SYNCWORD_64BIT;
    fa_config.fa_2m_mode = false;

    fa_config.fa_tx_gain_en = true;
    fa_config.fa_tx_gain_idx = FA_FIX_TX_GIAN_IDX;
    fa_config.fa_rx_winsz = FA_RX_WIN_SIZE;
#ifdef __FIX_FA_RX_GAIN___
    fa_config.fa_rx_gain_en = true;
    fa_config.fa_rx_gain_idx = FA_FIX_RX_GIAN_IDX;
#endif
    fa_config.fa_multi_mode0_en = false;
    fa_config.fa_multi_mode1_en = false;
    fa_config.fa_multi_tx_count = FA_MULTI_TX_COUNT;

    //setting
    btdrv_fa_basic_config(&fa_config);
}

void btdrv_ecc_config(void)
{
    btdrv_fast_ack_config();
    //no ECC
}
#endif //__BT_FAST_ACK_EN__

#ifdef __AFH_ASSESS__
void btdrv_afh_monitor_config(void)
{
    bt_bes_enhpcm_cntl_afh_rxgain_setf(AFH_ASSESS_GAIN);
    bt_afh_monitor_win_spacing_setf(1);
    bt_afh_monitor_win_size_setf(60);
}

#endif //__AFH_ASSESS__

static void btdrv_digital_common_config(void)
{
    bt_bes_cntl0_reg_txpwr_index_dr_setf(0);
    bt_bes_cntl0_reg_rxgain_index_dr_setf(0);

#ifdef __AFH_ASSESS__
    btdrv_afh_monitor_config();
#endif
}

static void btdrv_ble_modem_config(void)
{
    BT_DRV_TRACE(1,"%s",__func__);
    //add BLE modem config here
}

static void btdrv_bt_modem_config(void)
{
    BT_DRV_TRACE(1,"%s",__func__);
    //add BT modem config here

    besmdm_bw_2m_rc_if_value_setf(0x1000147B);
    besmdm_bw_2m_rc_step_setf(0x400000);
    besmdm_bw_2m_rc_ch_step_setf(0x6D3A0);
    besmdm_bw_4m_rc_step_setf(0x000000);

    besmdm_rx_adc_clock_rate_setf(0x1);
    besmdm_rx_iq_swap_pack(0x0DA7, 0x1, 0x0, 0x1);
    besmdm_rx_rate_converter_pack(0x1, 0x400000);

    besmdm_tx_iq_swap_en_pack(0x1, 0x0, 0x0);

    besmdm_edr_gfsk_dsg_nom_pack(0x0, 0x1);

    besmdm_tx_startupdel_setf(0x20);
    besmdm_ramp_mode_dn_setf(0x1);

    // AHI htx tracking
    besmdm_ahi_k_1_parameter_setf(0x5);
    besmdm_ahi_htx_tracking_2_pack(0x64, 0xC);
    besmdm_ahi_htx_tracking_3_pack(0xBE, 0xD7);

    // BT psd filter on
    besmdm_psd_filterbypass_bt_setf(0x0);
    besmdm_bt_psd_filter_on_2_pack(0x38D, 0x315);
    besmdm_bt_psd_filter_on_3_pack(0x47, 0x2D9);

    // RXPWR est
    besmdm_rxpwr_k_parameter_setf(0x3);

    // Sync parameters
    besmdm_sync_parameter_1_pack(0x0, 0x0);
    besmdm_sync_parameter_2_pack(0x0, 0xA, 0xC8);
    besmdm_sync_parameter_3_pack(0x3, 0x3, 0x64);

    besmdm_par_th_ble_2m_setf(0x64);
    besmdm_sync_parameter_6_pack(0xA, 0xA);
    besmdm_par_th_bt_setf(0x03219064);
    besmdm_err_sum_max_th_setf(0x9091E);
    besmdm_rx_pwr_th_setf(0x0000);
    besmdm_rxpwr_th_bw_2m_setf(0x0000);
    besmdm_rxpwr_th_bw_4m_setf(0x0000);

    // RC in filter enable
    besmdm_rx_in_filter_en_setf(0x0);

    // IQmis LMS on
    besmdm_iqmis_comp_en_setf(0x1);
    besmdm_iqmis_comp_setf(0x7);
    besmdm_iqmis_lms_on_3_pack(0xC8, 0x32);

    // valid position
    besmdm_osr__12_valib_posi_setf(0x4);

    // EDR_SKY_ON
    besmdm_rx_dpsk_new_mode_en_pack(0x7F, 0x1);
    besmdm_dpsk_k_3_edr_3m_setf(0xC0);

    besmdm_lr_preamble_det_th_setf(0x3001000);

    cmu_pol_clk_adc_setf(1);// pol_clk_adc use failing edge
    besmdm_old_demodulate_pack(0x8, 0x7, 0x1);

    besmdm_osr_12_ble_2m_setf(0x2);
    besmdm_int_dacfifo_bypass_setf(0x0);
#ifdef __HW_AGC__
    besmdm_hw_agc_pwr_lock_pack(0x0, 0x0);
    besmdm_omega_lock_th_bt_setf(0x0);
    besmdm_omega_lock_th_ble_1m_setf(0x0);
    besmdm_omega_lock_th_ble_2m_setf(0x0);
    besmdm_k_ble_2m_setf(0x20);
    besmdm_omega_lock_th_bt_bw_2m_setf(0x0);
    besmdm_omega_lock_th_bt_bw_4m_setf(0x0);
    besmdm_omega_lock_th_ble_4m_setf(0x0);
    besmdm_k_ble_4m_setf(0x20);
    besmdm_hw_agc_k_ant_pack(0x20, 0x20);
    besmdm_hw_agc_cnt_lock_bt_ble1m_pack(0x1F, 0x1441D4);
    besmdm_hw_agc_cnt_lock_ble2m_blelr_pack(0x1F, 0x5E8240);
    besmdm_cnt_lock_th_bt_bw_2m_bw_4m_setf(0x04800330);
    besmdm_cnt_lock_th_bt_ble_4m_setf(0x450);
    besmdm_cnt_lock_th_ant_1m_pt_1_pt_2_setf(0x01E00180);
    besmdm_cnt_lock_th_ant_2m_pt_1_pt_2_setf(0x030002A0);
    besmdm_hwagc_rrc_gain_sel_setf(0x1);
    besmdm_modem_delay_setf(0x27012727);
#endif
    besmdm_rxgfsk_rx_sto_en_setf(0x1);
    besmdm_sync_pulse_sel_setf(0x0);
    besmdm_corr_th_setf(0x3);
    besmdm_rx_startup_delay_pack(0x80, 0x40, 0x9C, 0x4E);
    // tx digital gain
    besmdm_gfsk_dsg_den_setf(0x5);
    besmdm_gfsk_dsg_nom_pack(0x17, 0x17);
    besmdm_dpsk_dsg_den_setf(0x5);
    besmdm_dpsk_dsg_nom_setf(0x17);
    besmdm_max_wait_guard_time_setf(0x18);
    besmdm_dpsk_k_3_edr_3m_2_setf(0x40);
#ifdef BT_RF_I2V_BYPASS
    bt_agc_ble_rx_dig_i_2v_bypass_setf(0x3);
#else
    bt_agc_ble_rx_dig_i_2v_bypass_setf(0x1);
#endif
}


bool btdrv_is_ecc_enable(void)
{
    bool ret = false;

    return ret;
}

void bt_drv_bt_tport_type_config(void)
{
    uint32_t tport_type = 0xb1b1;

#ifdef __BT_DEBUG_TPORTS__
    tport_type = TPORT_TYPE;
#endif
    BTDIGITAL_REG(IP_DIAGCNTL_ADDR) = tport_type;

    BT_DRV_TRACE(1,"BT_DRV: tport type=0x%x",tport_type);
}

void btdrv_config_end(void)
{
#ifdef BT_ACTIVE_OUTPUT
    hal_iomux_set_bt_active_out();
#endif
}

void btdrv_hciprocess(void)
{
    BT_DRV_TRACE(1,"%s", __func__);

    for(uint8_t i=0; i<sizeof(btdrv_cfg_tbl)/sizeof(btdrv_cfg_tbl[0]); i++){
        //BT other config
        if(btdrv_cfg_tbl[i].is_act == BTDRV_CONFIG_ACTIVE){
            btdrv_send_cmd(btdrv_cfg_tbl[i].opcode,btdrv_cfg_tbl[i].parlen,btdrv_cfg_tbl[i].param);
#ifdef NORMAL_TEST_MODE_SWITCH
            btdrv_delay(20);
#else
            btdrv_delay(1);
#endif
        }
    }

    btdrv_hci_init_sleep_wakeup_param();

    btdrv_hci_set_ble_rpl_tx_pwr_conv_tbl();
}

void btdrv_digital_init(void)
{
    BT_DRV_TRACE(1,"%s", __func__);

    btdrv_digital_common_config();

    btdrv_bt_modem_config();

    btdrv_ble_modem_config();
#ifdef __BT_FAST_ACK_EN__
    btdrv_ecc_config();
#endif //__BT_FAST_ACK_EN__

    btdrv_config_end();
}
