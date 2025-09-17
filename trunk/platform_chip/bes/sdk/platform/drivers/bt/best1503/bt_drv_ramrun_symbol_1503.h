
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_SYMBOL_H__
#define  __BT_CONTROLLER_SYMBOL_H__

/*****   Macro Definition of Relative Offset of Variables   *****/

// ld_acl_env_tag/rssi_record
#define OFF_RR_IN_LAET_LD                                             0XE6
// ld_acl_env_tag/last_sync_bit_off
#define OFF_LSBO_IN_LAET_LD                                           0XCE
// ld_acl_env_tag/t_poll
#define OFF_TP_IN_LAET_LD                                             0XF0
// ld_acl_env_tag/last_sync_clk_off
#define OFF_LSCO_IN_LAET_LD                                           0XB8
// ld_acl_env_tag/rxbit_1us
#define OFF_R1_IN_LAET_LD                                             0XCA
// bt_util_buf_env_tag/acl_rx_free
#define OFF_ARF_IN_BUBET_BT                                           0X14
// bt_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BT                                           0X28
// ble_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BLE                                          0X28
// dbg_bt_sche_setting_cmd/music_playing_link
#define OFF_MPL_IN_DBSSC_DBG                                          0XA
// dbg_bt_sche_setting_cmd/acl_slot_in_ibrt_mode
#define OFF_ASIIM_IN_DBSSC_DBG                                        0XC
// dbg_bt_sche_setting_cmd/acl_interv_in_ibrt_normal_mode
#define OFF_AIIINM_IN_DBSSC_DBG                                       0X2
// dbg_bt_common_setting_cmd/trace_level
#define OFF_TL_IN_DBCSC_DBG                                           0X8
// ld_bes_bt_env_tag/tws_device_env/link_id
#define OFF_LI_IN_LBBET_LD                                            0X12
// dbg_bt_ibrt_setting_cmd/accept_remote_enter_sniff
#define OFF_ARES_IN_DBISC_DBG                                         0XF
// dbg_bt_ibrt_setting_cmd/ibrt_second_sco_decision
#define OFF_ISSD_IN_DBISC_DBG                                         0XE
// lc_sco_env_tag/p_link_params/link_id
#define OFF_LI_IN_LSET_LC                                             0X1E
// lc_sco_nego_params_tag/air_params_received/m2s_pkt_type
#define OFF_MPT_IN_LSNPT_LC                                           0X47
// lc_sco_nego_params_tag/air_params_received/air_mode
#define OFF_AM_IN_LSNPT_LC                                            0X4E
// dbg_bt_hw_feat_setting_cmd/bt_sync_swagc_en
#define OFF_BSSE_IN_DBHFSC_DBG                                        0XF
// dbg_bt_hw_feat_setting_cmd/le_sync_swagc_en
#define OFF_LSSE_IN_DBHFSC_DBG                                        0X10
// rwip_env_tag/lp_clk
#define OFF_LC_IN_RET_RWIP                                            0X39C
// rwip_env_tag/last_bt_samp_time/hs
#define OFF_H_IN_RET_RWIP_0                                           0X28
// rwip_env_tag/last_bt_samp_time/hus
#define OFF_H_IN_RET_RWIP_1                                           0X2C
// rwip_env_tag/last_us_samp_time
#define OFF_LUST_IN_RET_RWIP                                          0X34
// lld_cis_env/grp_hdl
#define OFF_GH_IN_LCE_LLD                                             0X1A
// lld_cig_env/anchor_hs
#define OFF_AH_IN_LCE_LLD_0                                           0X80
// lld_cig_env/anchor_hus
#define OFF_AH_IN_LCE_LLD_1                                           0X84
// lld_cig_env/iso_interval
#define OFF_II_IN_LCE_LLD                                             0X92
// lld_bis_env/grp_hdl
#define OFF_GH_IN_LBE_LLD                                             0X1C
// lld_big_env/anchor_hs
#define OFF_AH_IN_LBE_LLD_0                                           0XE8
// lld_big_env/anchor_hus
#define OFF_AH_IN_LBE_LLD_1                                           0XEC
// lld_big_env/iso_interval
#define OFF_II_IN_LBE_LLD                                             0XEE
// dbg_bt_common_setting_t2/tws_sleep_dura_dec
#define OFF_TSDD_IN_DBCST_DBG                                         0X3A
// dbg_bt_common_setting_t2/normal_sleep_dura_inc
#define OFF_NSDI_IN_DBCST_DBG                                         0X3B


#define HCI_FC_ENV_ADDR                                               0XC0009CFC
#define LD_ACL_ENV_ADDR                                               0XC000545C
#define BT_UTIL_BUF_ENV_ADDR                                          0XC0004BCC
#define BLE_UTIL_BUF_ENV_ADDR                                         0XC0005B6C
#define LD_BES_BT_ENV_ADDR                                            0XC0005920
#define DBG_STATE_ADDR                                                0XC0009F16
#define LC_STATE_ADDR                                                 0XC0005370
#define LD_SCO_ENV_ADDR                                               0XC00057E0
#define RX_MONITOR_ADDR                                               0XC000A9E2
#define LC_ENV_ADDR                                                   0XC00050D8
#define LM_ENV_ADDR                                                   0XC0004E44
#define HCI_ENV_ADDR                                                  0XC0009BF4
#define LC_SCO_ENV_ADDR                                               0XC00050B4
#define LLM_ENV_ADDR                                                  0XC0005F94
#define LD_ENV_ADDR                                                   0XC00053E4
#define RWIP_ENV_ADDR                                                 0XC000AA84
#define BLE_RX_MONITOR_ADDR                                           0XC000A91C
#define LLC_ENV_ADDR                                                  0XC0008BF4
#define RWIP_RF_ADDR                                                  0XC000AA20
#define LD_ACL_METRICS_ADDR                                           0XC0005470
#define RF_RX_HWGAIN_TBL_ADDR                                         0XC000499B
#define RF_HWAGC_RSSI_CORRECT_TBL_ADDR                                0XC00048B9
#define RF_RX_GAIN_FIXED_TBL_ADDR                                     0XC000A7FC
#define HCI_DBG_EBQ_TEST_MODE_ADDR                                    0XC0009F30
#define DBG_BT_COMMON_SETTING_ADDR                                    0XC0009DC4
#define DBG_BT_SCHE_SETTING_ADDR                                      0XC0009E74
#define DBG_BT_IBRT_SETTING_ADDR                                      0XC0009E58
#define DBG_BT_HW_FEAT_SETTING_ADDR                                   0XC0009E34
#define HCI_DBG_SET_SW_RSSI_ADDR                                      0XC0009F60
#define DATA_BACKUP_CNT_ADDR                                          0XC0004070
#define DATA_BACKUP_ADDR_PTR_ADDR                                     0XC0004074
#define DATA_BACKUP_VAL_PTR_ADDR                                      0XC0004078
#define SCH_MULTI_IBRT_ADJUST_ENV_ADDR                                0XC0009B30
#define RF_HWAGC_RSSI_CORRECT_TBL_ADDR                                0XC00048B9
#define RF_RX_GAIN_THS_TBL_LE_ADDR                                    0XC000495F
#define RF_RX_GAIN_THS_TBL_LE_2M_ADDR                                 0XC000497D
#define RF_RPL_TX_PW_CONV_TBL_ADDR                                    0XC00048C9
#define REPLACE_MOBILE_ADDR_ADDR                                      0XC000537C
#define REPLACE_ADDR_VALID_ADDR                                       0XC0005378
#define PCM_NEED_START_FLAG_ADDR                                      0XC0005800
#define RF_RX_GAIN_THS_TBL_BT_3M_ADDR                                 0XC000A808
#define LD_IBRT_ENV_ADDR                                              0XC0005934
#define LLM_LOCAL_LE_FEATS_ADDR                                       0XC0004776
#define RF_RX_GAIN_THS_TBL_BT_ADDR                                    0XC0004905
#define DBG_BT_COMMON_SETTING_T2_ADDR                                 0XC0009DEA
#define LLD_CON_ENV_ADDR                                              0XC0009138
#define LLD_ISO_ENV_ADDR                                              0XC0009188
#define ECC_RX_MONITOR_ADDR                                           0XC000A968
#define __STACKLIMIT_ADDR                                             0XC000B3A0
#define PER_MONITOR_PARAMS_ADDR                                       0XC0004770
#define MASTER_CON_SUPPORT_LE_AUDIO_ADDR                              0XC00050EC
#define RX_RECORD_ADDR                                                0XC000580E
#define SCH_PROG_DBG_ENV_ADDR                                         0XC0009974
#define RF_RX_GAIN_THS_TBL_ECC_ADDR                                   0XC0004941
#define RX_MONITOR_3M_RXGAIN_ADDR                                     0XC000476D
#define CON_LATENCY_APPLY_ADDR                                        0XC00047A0
#define LLD_PER_ADV_ENV_ADDR                                          0XC0009084
#define MHDT_SWAGC_EN_ADDR                                            0XC000A990
#define RF_RX_GAIN_THS_TBL_MHDT_BT2M_2M_ADDR                          0XC000A826
#define RF_RX_GAIN_THS_TBL_MHDT_BT2M_3M_ADDR                          0XC000A844
#define RF_RX_GAIN_THS_TBL_MHDT_BT4M_2M_ADDR                          0XC000A862
#define LE_CON_SKIP_3_INTERVAL_ADDR                                   0XC0009134
#define LLD_BIS_ENV_ADDR                                              0XC0009714
#define LLD_BIG_ENV_ADDR                                              0XC00096F4
#define LLD_CIS_ENV_ADDR                                              0XC00096A4
#define LLD_CIG_ENV_ADDR                                              0XC0009684
#define RF_BLE_TX_PW_CONV_TBL_ADDR                                    0XC00048AA
#define LLD_ADV_BT2MCU_INTERSYS1_ADDR                                 0XC0008FA0
#define LLD_CON_BT2MCU_INTERSYS1_ADDR                                 0XC0008FA8
#define SENS2BT_EN_ADDR                                               0XC0004050
#define DBG_BT_SCHE_SETTING_COEX_CIS_ADDR                             0XC0009E9A
#define DBG_BT_SCHE_SETTING_COEX_BIS_ADDR                             0XC0009E8C
#define REPLACE_MOBILE_LE_LINK_ID_ADDR                                0XC0004794
#define REPLACE_MOBILE_LE_LINK_ID_VALID_ADDR                          0XC0008C80

//commit a6834cc712258c6afd712d9a76de7bceb063426b
//Author: TianxuWu <tianxuwu@bestechnic.com>
//Date:   Tue Apr 16 11:08:03 2024 +0800
//    [PD]:Fix macro error cause compile fail.
//    
//    Change-Id: I1fdb2b34d20e2639ddf4eacb7695f9b6ecf76694

#ifndef  BT_CONTROLLER_COMMIT_ID
#define  BT_CONTROLLER_COMMIT_ID                            "commit a6834cc712258c6afd712d9a76de7bceb063426b"
#endif
#ifndef  BT_CONTROLLER_COMMIT_AUTHOR
#define  BT_CONTROLLER_COMMIT_AUTHOR                        "Author: TianxuWu <tianxuwu@bestechnic.com>"
#endif
#ifndef  BT_CONTROLLER_COMMIT_DATE
#define  BT_CONTROLLER_COMMIT_DATE                          "Date:   Tue Apr 16 11:08:03 2024 +0800"
#endif
#ifndef  BT_CONTROLLER_COMMIT_MESSAGE
#define  BT_CONTROLLER_COMMIT_MESSAGE                       "[PD]:Fix macro error cause compile fail.  Change-Id: I1fdb2b34d20e2639ddf4eacb7695f9b6ecf76694 "
#endif
#ifndef  BT_CONTROLLER_BUILD_TIME
#define  BT_CONTROLLER_BUILD_TIME                           "2024-04-16 11:11:14"
#endif
#ifndef  BT_CONTROLLER_BUILD_OWNER
#define  BT_CONTROLLER_BUILD_OWNER                          "TianxuWu"
#endif

#endif
