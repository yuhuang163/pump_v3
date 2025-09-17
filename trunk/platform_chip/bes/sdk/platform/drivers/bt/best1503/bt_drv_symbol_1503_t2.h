
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_1503_T2_SYMBOL_H__
#define  __BT_CONTROLLER_1503_T2_SYMBOL_H__

/*****   Macro Definition of Relative Offset of Variables   *****/

// ld_acl_env_tag/rssi_record
#define OFF_RR_IN_LAET_LD_T2                                          0XE6
// ld_acl_env_tag/last_sync_bit_off
#define OFF_LSBO_IN_LAET_LD_T2                                        0XCE
// ld_acl_env_tag/t_poll
#define OFF_TP_IN_LAET_LD_T2                                          0XF0
// ld_acl_env_tag/last_sync_clk_off
#define OFF_LSCO_IN_LAET_LD_T2                                        0XB8
// ld_acl_env_tag/rxbit_1us
#define OFF_R1_IN_LAET_LD_T2                                          0XCA
// bt_util_buf_env_tag/acl_rx_free
#define OFF_ARF_IN_BUBET_BT_T2                                        0X14
// bt_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BT_T2                                        0X28
// ble_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BLE_T2                                       0X28
// dbg_bt_sche_setting_cmd/music_playing_link
#define OFF_MPL_IN_DBSSC_DBG_T2                                       0XA
// dbg_bt_sche_setting_cmd/acl_slot_in_ibrt_mode
#define OFF_ASIIM_IN_DBSSC_DBG_T2                                     0XC
// dbg_bt_sche_setting_cmd/acl_interv_in_ibrt_normal_mode
#define OFF_AIIINM_IN_DBSSC_DBG_T2                                    0X2
// dbg_bt_common_setting_cmd/trace_level
#define OFF_TL_IN_DBCSC_DBG_T2                                        0X8
// ld_bes_bt_env_tag/tws_device_env/link_id
#define OFF_LI_IN_LBBET_LD_T2                                         0X12
// dbg_bt_ibrt_setting_cmd/accept_remote_enter_sniff
#define OFF_ARES_IN_DBISC_DBG_T2                                      0XF
// dbg_bt_ibrt_setting_cmd/ibrt_second_sco_decision
#define OFF_ISSD_IN_DBISC_DBG_T2                                      0XE
// lc_sco_env_tag/p_link_params/link_id
#define OFF_LI_IN_LSET_LC_T2                                          0X1E
// lc_sco_nego_params_tag/air_params_received/m2s_pkt_type
#define OFF_MPT_IN_LSNPT_LC_T2                                        0X47
// lc_sco_nego_params_tag/air_params_received/air_mode
#define OFF_AM_IN_LSNPT_LC_T2                                         0X4E
// dbg_bt_hw_feat_setting_cmd/bt_sync_swagc_en
#define OFF_BSSE_IN_DBHFSC_DBG_T2                                     0XF
// dbg_bt_hw_feat_setting_cmd/le_sync_swagc_en
#define OFF_LSSE_IN_DBHFSC_DBG_T2                                     0X10
// rwip_env_tag/lp_clk
#define OFF_LC_IN_RET_RWIP_T2                                         0X39C
// rwip_env_tag/last_bt_samp_time/hs
#define OFF_H_IN_RET_RWIP_0_T2                                        0X28
// rwip_env_tag/last_bt_samp_time/hus
#define OFF_H_IN_RET_RWIP_1_T2                                        0X2C
// rwip_env_tag/last_us_samp_time
#define OFF_LUST_IN_RET_RWIP_T2                                       0X34
// lld_cis_env/grp_hdl
#define OFF_GH_IN_LCE_LLD_T2                                          0X1A
// lld_cig_env/anchor_hs
#define OFF_AH_IN_LCE_LLD_0_T2                                        0X80
// lld_cig_env/anchor_hus
#define OFF_AH_IN_LCE_LLD_1_T2                                        0X84
// lld_cig_env/iso_interval
#define OFF_II_IN_LCE_LLD_T2                                          0X92
// lld_bis_env/grp_hdl
#define OFF_GH_IN_LBE_LLD_T2                                          0X1C
// lld_big_env/anchor_hs
#define OFF_AH_IN_LBE_LLD_0_T2                                        0XE8
// lld_big_env/anchor_hus
#define OFF_AH_IN_LBE_LLD_1_T2                                        0XEC
// lld_big_env/iso_interval
#define OFF_II_IN_LBE_LLD_T2                                          0XEE
// dbg_bt_common_setting_t2/bt_master_sleep_en
#define OFF_BMSE_IN_DBCST_DBG_T2                                      0X2F
// dbg_bt_common_setting_t2/bt_master_tx_silence_en
#define OFF_BMTSE_IN_DBCST_DBG_T2                                     0X30
// dbg_bt_common_setting_t2/tws_sleep_dura_dec
#define OFF_TSDD_IN_DBCST_DBG_T2                                      0X3A
// dbg_bt_common_setting_t2/normal_sleep_dura_inc
#define OFF_NSDI_IN_DBCST_DBG_T2                                      0X3B


#define HCI_FC_ENV_T2_ADDR                                            0XC0009CDC
#define LD_ACL_ENV_T2_ADDR                                            0XC0005454
#define BT_UTIL_BUF_ENV_T2_ADDR                                       0XC0004BCC
#define BLE_UTIL_BUF_ENV_T2_ADDR                                      0XC0005B5C
#define LD_BES_BT_ENV_T2_ADDR                                         0XC0005910
#define DBG_STATE_T2_ADDR                                             0XC0009EEA
#define LC_STATE_T2_ADDR                                              0XC0005370
#define LD_SCO_ENV_T2_ADDR                                            0XC00057D8
#define RX_MONITOR_T2_ADDR                                            0XC000A9B6
#define LC_ENV_T2_ADDR                                                0XC00050D8
#define LM_NB_SYNC_ACTIVE_T2_ADDR                                     0XC00050C4
#define LM_ENV_T2_ADDR                                                0XC0004E44
#define HCI_ENV_T2_ADDR                                               0XC0009BD4
#define LC_SCO_ENV_T2_ADDR                                            0XC00050B4
#define LLM_ENV_T2_ADDR                                               0XC0005F84
#define LD_ENV_T2_ADDR                                                0XC00053DC
#define RWIP_ENV_T2_ADDR                                              0XC000AA58
#define BLE_RX_MONITOR_T2_ADDR                                        0XC000A8F0
#define LLC_ENV_T2_ADDR                                               0XC0008BE0
#define RWIP_RF_T2_ADDR                                               0XC000A9F4
#define LD_ACL_METRICS_T2_ADDR                                        0XC0005468
#define RF_RX_HWGAIN_TBL_T2_ADDR                                      0XC000499B
#define RF_HWAGC_RSSI_CORRECT_TBL_T2_ADDR                             0XC00048B9
#define RF_RX_GAIN_FIXED_TBL_T2_ADDR                                  0XC000A7D0
#define HCI_DBG_EBQ_TEST_MODE_T2_ADDR                                 0XC0009F04
#define DBG_BT_COMMON_SETTING_T2_ADDR                                 0XC0009DA4
#define DBG_BT_SCHE_SETTING_T2_ADDR                                   0XC0009E48
#define DBG_BT_IBRT_SETTING_T2_ADDR                                   0XC0009E2C
#define DBG_BT_HW_FEAT_SETTING_T2_ADDR                                0XC0009E08
#define HCI_DBG_SET_SW_RSSI_T2_ADDR                                   0XC0009F34
#define RWIP_PROG_DELAY_T2_ADDR                                       0XC000A9F0
#define DATA_BACKUP_CNT_T2_ADDR                                       0XC0004070
#define DATA_BACKUP_ADDR_PTR_T2_ADDR                                  0XC0004074
#define DATA_BACKUP_VAL_PTR_T2_ADDR                                   0XC0004078
#define SCH_MULTI_IBRT_ADJUST_ENV_T2_ADDR                             0XC0009B10
#define RF_HWAGC_RSSI_CORRECT_TBL_T2_ADDR                             0XC00048B9
#define RF_RX_GAIN_THS_TBL_LE_T2_ADDR                                 0XC000495F
#define RF_RX_GAIN_THS_TBL_LE_2M_T2_ADDR                              0XC000497D
#define RF_RPL_TX_PW_CONV_TBL_T2_ADDR                                 0XC00048C9
#define REPLACE_MOBILE_ADDR_T2_ADDR                                   0XC0005376
#define REPLACE_ADDR_VALID_T2_ADDR                                    0XC0005375
#define PCM_NEED_START_FLAG_T2_ADDR                                   0XC00057F8
#define RT_SLEEP_FLAG_CLEAR_T2_ADDR                                   0XC000B40C
#define RF_RX_GAIN_THS_TBL_BT_3M_T2_ADDR                              0XC000A7DC
#define LD_IBRT_ENV_T2_ADDR                                           0XC0005924
#define LLM_LOCAL_LE_FEATS_T2_ADDR                                    0XC000477A
#define RF_RX_GAIN_THS_TBL_BT_T2_ADDR                                 0XC0004905
#define DBG_BT_COMMON_SETTING_T2_T2_ADDR                              0XC0009DCA
#define LLD_CON_ENV_T2_ADDR                                           0XC000911C
#define LLD_ISO_ENV_T2_ADDR                                           0XC000916C
#define ECC_RX_MONITOR_T2_ADDR                                        0XC000A93C
#define __STACKLIMIT_T2_ADDR                                          0XC000B570
#define PER_MONITOR_PARAMS_T2_ADDR                                    0XC0004774
#define SENS2BT_EN_T2_ADDR                                            0XC0004050
#define RF_RX_GAIN_THS_TBL_ECC_T2_ADDR                                0XC0004941
#define LLD_BIS_ENV_T2_ADDR                                           0XC00096F8
#define LLD_BIG_ENV_T2_ADDR                                           0XC00096D8
#define LLD_CIS_ENV_T2_ADDR                                           0XC0009688
#define LLD_CIG_ENV_T2_ADDR                                           0XC0009668
#define RX_MONITOR_3M_RXGAIN_T2_ADDR                                  0XC00049ED
#define INTERSYS_MSG_TX_MODE_T2_ADDR                                  0XC0004030
#define CON_LATENCY_APPLY_T2_ADDR                                     0XC00047A0

#endif