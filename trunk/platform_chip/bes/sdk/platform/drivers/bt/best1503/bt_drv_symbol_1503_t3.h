
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_1503_T3_SYMBOL_H__
#define  __BT_CONTROLLER_1503_T3_SYMBOL_H__

/*****   Macro Definition of Relative Offset of Variables   *****/

// ld_acl_env_tag/rssi_record
#define OFF_RR_IN_LAET_LD_T3                                          0XEA
// ld_acl_env_tag/last_sync_bit_off
#define OFF_LSBO_IN_LAET_LD_T3                                        0XD2
// ld_acl_env_tag/t_poll
#define OFF_TP_IN_LAET_LD_T3                                          0XF4
// ld_acl_env_tag/last_sync_clk_off
#define OFF_LSCO_IN_LAET_LD_T3                                        0XBC
// ld_acl_env_tag/rxbit_1us
#define OFF_R1_IN_LAET_LD_T3                                          0XCE
// bt_util_buf_env_tag/acl_rx_free
#define OFF_ARF_IN_BUBET_BT_T3                                        0X14
// bt_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BT_T3                                        0X28
// ble_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BLE_T3                                       0X28
// dbg_bt_sche_setting_cmd/music_playing_link
#define OFF_MPL_IN_DBSSC_DBG_T3                                       0XA
// dbg_bt_sche_setting_cmd/acl_slot_in_ibrt_mode
#define OFF_ASIIM_IN_DBSSC_DBG_T3                                     0XC
// dbg_bt_sche_setting_cmd/acl_interv_in_ibrt_normal_mode
#define OFF_AIIINM_IN_DBSSC_DBG_T3                                    0X2
// dbg_bt_common_setting_cmd/trace_level
#define OFF_TL_IN_DBCSC_DBG_T3                                        0X8
// ld_bes_bt_env_tag/tws_device_env/link_id
#define OFF_LI_IN_LBBET_LD_T3                                         0X12
// dbg_bt_ibrt_setting_cmd/accept_remote_enter_sniff
#define OFF_ARES_IN_DBISC_DBG_T3                                      0XF
// dbg_bt_ibrt_setting_cmd/ibrt_second_sco_decision
#define OFF_ISSD_IN_DBISC_DBG_T3                                      0XE
// lc_sco_env_tag/p_link_params/link_id
#define OFF_LI_IN_LSET_LC_T3                                          0X1E
// lc_sco_nego_params_tag/air_params_received/m2s_pkt_type
#define OFF_MPT_IN_LSNPT_LC_T3                                        0X47
// lc_sco_nego_params_tag/air_params_received/air_mode
#define OFF_AM_IN_LSNPT_LC_T3                                         0X4E
// dbg_bt_hw_feat_setting_cmd/bt_sync_swagc_en
#define OFF_BSSE_IN_DBHFSC_DBG_T3                                     0XF
// dbg_bt_hw_feat_setting_cmd/le_sync_swagc_en
#define OFF_LSSE_IN_DBHFSC_DBG_T3                                     0X10
// rwip_env_tag/lp_clk
#define OFF_LC_IN_RET_RWIP_T3                                         0X39C
// rwip_env_tag/last_bt_samp_time/hs
#define OFF_H_IN_RET_RWIP_0_T3                                        0X28
// rwip_env_tag/last_bt_samp_time/hus
#define OFF_H_IN_RET_RWIP_1_T3                                        0X2C
// rwip_env_tag/last_us_samp_time
#define OFF_LUST_IN_RET_RWIP_T3                                       0X34
// lld_cis_env/grp_hdl
#define OFF_GH_IN_LCE_LLD_T3                                          0X1A
// lld_cig_env/anchor/hs
#define OFF_H_IN_LCE_LLD_0_T3                                         0X5C
// lld_cig_env/anchor/hus
#define OFF_H_IN_LCE_LLD_1_T3                                         0X60
// lld_cig_env/iso_interval
#define OFF_II_IN_LCE_LLD_T3                                          0X9E
// lld_bis_env/grp_hdl
#define OFF_GH_IN_LBE_LLD_T3                                          0X20
// lld_big_env/anchor/hs
#define OFF_H_IN_LBE_LLD_0_T3                                         0XC8
// lld_big_env/anchor/hus
#define OFF_H_IN_LBE_LLD_1_T3                                         0XCC
// lld_big_env/iso_interval
#define OFF_II_IN_LBE_LLD_T3                                          0XF4
// dbg_bt_common_setting_t2/bt_master_sleep_en
#define OFF_BMSE_IN_DBCST_DBG_T3                                      0X27
// dbg_bt_common_setting_t2/bt_master_tx_silence_en
#define OFF_BMTSE_IN_DBCST_DBG_T3                                     0X28
// dbg_bt_common_setting_t2/tws_sleep_dura_dec
#define OFF_TSDD_IN_DBCST_DBG_T3                                      0X32
// dbg_bt_common_setting_t2/normal_sleep_dura_inc
#define OFF_NSDI_IN_DBCST_DBG_T3                                      0X33


#define HCI_FC_ENV_T3_ADDR                                            0XC0009D04
#define LD_ACL_ENV_T3_ADDR                                            0XC000521C
#define BT_UTIL_BUF_ENV_T3_ADDR                                       0XC0004C58
#define BLE_UTIL_BUF_ENV_T3_ADDR                                      0XC0005ADC
#define LD_BES_BT_ENV_T3_ADDR                                         0XC00056F8
#define DBG_STATE_T3_ADDR                                             0XC0009FF8
#define LC_STATE_T3_ADDR                                              0XC0005138
#define LD_SCO_ENV_T3_ADDR                                            0XC00055B4
#define RX_MONITOR_T3_ADDR                                            0XC000AA3A
#define LC_ENV_T3_ADDR                                                0XC000509C
#define LM_NB_SYNC_ACTIVE_T3_ADDR                                     0XC0005088
#define LM_ENV_T3_ADDR                                                0XC0004E08
#define HCI_ENV_T3_ADDR                                               0XC0009BDC
#define LC_SCO_ENV_T3_ADDR                                            0XC0005078
#define LLM_ENV_T3_ADDR                                               0XC0005D8C
#define LD_ENV_T3_ADDR                                                0XC00051A4
#define RWIP_ENV_T3_ADDR                                              0XC000AAEC
#define BLE_RX_MONITOR_T3_ADDR                                        0XC000A9B5
#define LLC_ENV_T3_ADDR                                               0XC0008B4C
#define RWIP_RF_T3_ADDR                                               0XC000AA78
#define LD_ACL_METRICS_T3_ADDR                                        0XC0005230
#define RF_RX_HWGAIN_TBL_T3_ADDR                                      0XC0004A23
#define RF_HWAGC_RSSI_CORRECT_TBL_T3_ADDR                             0XC0004941
#define RF_RX_GAIN_FIXED_TBL_T3_ADDR                                  0XC000A928
#define HCI_DBG_EBQ_TEST_MODE_T3_ADDR                                 0XC000A010
#define DBG_BT_COMMON_SETTING_T3_ADDR                                 0XC0009DCC
#define DBG_BT_SCHE_SETTING_T3_ADDR                                   0XC0009E9C
#define DBG_BT_IBRT_SETTING_T3_ADDR                                   0XC0009E80
#define DBG_BT_HW_FEAT_SETTING_T3_ADDR                                0XC0009E58
#define HCI_DBG_SET_SW_RSSI_T3_ADDR                                   0XC000A040
#define RWIP_PROG_DELAY_T3_ADDR                                       0XC000AA74
#define SCH_MULTI_IBRT_ADJUST_ENV_T3_ADDR                             0XC0009B18
#define RF_HWAGC_RSSI_CORRECT_TBL_T3_ADDR                             0XC0004941
#define RF_RX_GAIN_THS_TBL_LE_T3_ADDR                                 0XC00049E7
#define RF_RX_GAIN_THS_TBL_LE_2M_T3_ADDR                              0XC0004A05
#define RF_RPL_TX_PW_CONV_TBL_T3_ADDR                                 0XC0004951
#define REPLACE_MOBILE_ADDR_T3_ADDR                                   0XC0005144
#define REPLACE_ADDR_VALID_T3_ADDR                                    0XC0005140
#define PCM_NEED_START_FLAG_T3_ADDR                                   0XC00055D4
#define RF_RX_GAIN_THS_TBL_BT_3M_T3_ADDR                              0XC000A934
#define LD_IBRT_ENV_T3_ADDR                                           0XC0005880
#define LLM_LOCAL_LE_FEATS_T3_ADDR                                    0XC00047C2
#define RF_RX_GAIN_THS_TBL_BT_T3_ADDR                                 0XC000498D
#define DBG_BT_COMMON_SETTING_T2_T3_ADDR                              0XC0009DF8
#define LLD_CON_ENV_T3_ADDR                                           0XC00090F4
#define LLD_ISO_ENV_T3_ADDR                                           0XC0009144
#define ECC_RX_MONITOR_T3_ADDR                                        0XC000A9E9
#define __STACKLIMIT_T3_ADDR                                          0XC000B690
#define SENS2BT_EN_T3_ADDR                                            0XC0004090
#define RF_RX_GAIN_THS_TBL_ECC_T3_ADDR                                0XC00049C9
#define LLD_BIS_ENV_T3_ADDR                                           0XC0009810
#define LLD_BIG_ENV_T3_ADDR                                           0XC00097F0
#define LLD_CIS_ENV_T3_ADDR                                           0XC00097A0
#define LLD_CIG_ENV_T3_ADDR                                           0XC0009780
#define RX_MONITOR_3M_RXGAIN_T3_ADDR                                  0XC00047C1
#define INTERSYS_MSG_TX_MODE_T3_ADDR                                  0XC0004060
#define RX_RECORD_T3_ADDR                                             0XC00055E2

#endif