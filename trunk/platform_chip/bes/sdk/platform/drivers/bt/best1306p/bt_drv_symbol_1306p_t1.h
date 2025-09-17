
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_1306P_T1_SYMBOL_H__
#define  __BT_CONTROLLER_1306P_T1_SYMBOL_H__

/*****   Macro Definition of Relative Offset of Variables   *****/

// ld_acl_env_tag/rssi_record
#define OFF_RR_IN_LAET_LD_1306P_T1                                    0XE6
// ld_acl_env_tag/last_sync_bit_off
#define OFF_LSBO_IN_LAET_LD_1306P_T1                                  0XCE
// ld_acl_env_tag/t_poll
#define OFF_TP_IN_LAET_LD_1306P_T1                                    0XF0
// ld_acl_env_tag/last_sync_clk_off
#define OFF_LSCO_IN_LAET_LD_1306P_T1                                  0XB8
// ld_acl_env_tag/rxbit_1us
#define OFF_R1_IN_LAET_LD_1306P_T1                                    0XCA
// bt_util_buf_env_tag/acl_rx_free
#define OFF_ARF_IN_BUBET_BT_1306P_T1                                  0X14
// bt_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BT_1306P_T1                                  0X28
// ble_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BLE_1306P_T1                                 0X28
// dbg_bt_sche_setting_cmd/music_playing_link
#define OFF_MPL_IN_DBSSC_DBG_1306P_T1                                 0XA
// dbg_bt_sche_setting_cmd/acl_slot_in_ibrt_mode
#define OFF_ASIIM_IN_DBSSC_DBG_1306P_T1                               0XC
// dbg_bt_sche_setting_cmd/acl_interv_in_ibrt_normal_mode
#define OFF_AIIINM_IN_DBSSC_DBG_1306P_T1                              0X2
// dbg_bt_common_setting_cmd/trace_level
#define OFF_TL_IN_DBCSC_DBG_1306P_T1                                  0X8
// ld_bes_bt_env_tag/tws_device_env/link_id
#define OFF_LI_IN_LBBET_LD_1306P_T1                                   0X12
// dbg_bt_ibrt_setting_cmd/accept_remote_enter_sniff
#define OFF_ARES_IN_DBISC_DBG_1306P_T1                                0XF
// dbg_bt_ibrt_setting_cmd/ibrt_second_sco_decision
#define OFF_ISSD_IN_DBISC_DBG_1306P_T1                                0XE
// lc_sco_env_tag/p_link_params/link_id
#define OFF_LI_IN_LSET_LC_1306P_T1                                    0X1E
// lc_sco_nego_params_tag/air_params_received/m2s_pkt_type
#define OFF_MPT_IN_LSNPT_LC_1306P_T1                                  0X47
// lc_sco_nego_params_tag/air_params_received/air_mode
#define OFF_AM_IN_LSNPT_LC_1306P_T1                                   0X4E
// dbg_bt_hw_feat_setting_cmd/bt_sync_swagc_en
#define OFF_BSSE_IN_DBHFSC_DBG_1306P_T1                               0XF
// dbg_bt_hw_feat_setting_cmd/le_sync_swagc_en
#define OFF_LSSE_IN_DBHFSC_DBG_1306P_T1                               0X10
// rwip_env_tag/lp_clk
#define OFF_LC_IN_RET_RWIP_1306P_T1                                   0X39C
// rwip_env_tag/last_bt_samp_time/hs
#define OFF_H_IN_RET_RWIP_0_1306P_T1                                  0X28
// rwip_env_tag/last_bt_samp_time/hus
#define OFF_H_IN_RET_RWIP_1_1306P_T1                                  0X2C
// rwip_env_tag/last_us_samp_time
#define OFF_LUST_IN_RET_RWIP_1306P_T1                                 0X34
// lld_cis_env/grp_hdl
#define OFF_GH_IN_LCE_LLD_1306P_T1                                    0X1A
// lld_cig_env/anchor/hs
#define OFF_H_IN_LCE_LLD_0_1306P_T1                                   0X58
// lld_cig_env/anchor/hus
#define OFF_H_IN_LCE_LLD_1_1306P_T1                                   0X5C
// lld_cig_env/iso_interval
#define OFF_II_IN_LCE_LLD_1306P_T1                                    0X9A
// lld_bis_env/grp_hdl
#define OFF_GH_IN_LBE_LLD_1306P_T1                                    0X1C
// lld_big_env/anchor/hs
#define OFF_H_IN_LBE_LLD_0_1306P_T1                                   0XBC
// lld_big_env/anchor/hus
#define OFF_H_IN_LBE_LLD_1_1306P_T1                                   0XC0
// lld_big_env/iso_interval
#define OFF_II_IN_LBE_LLD_1306P_T1                                    0XF4
// dbg_bt_common_setting_t2/bt_master_sleep_en
#define OFF_BMSE_IN_DBCST_DBG_1306P_T1                                0X2F
// dbg_bt_common_setting_t2/bt_master_tx_silence_en
#define OFF_BMTSE_IN_DBCST_DBG_1306P_T1                               0X30


#define HCI_FC_ENV_1306P_T1_ADDR                                      0XC0007098
#define LD_ACL_ENV_1306P_T1_ADDR                                      0XC0005394
#define BT_UTIL_BUF_ENV_1306P_T1_ADDR                                 0XC0004BF4
#define BLE_UTIL_BUF_ENV_1306P_T1_ADDR                                0XC0005998
#define LD_BES_BT_ENV_1306P_T1_ADDR                                   0XC00057B0
#define DBG_STATE_1306P_T1_ADDR                                       0XC00072B6
#define LC_STATE_1306P_T1_ADDR                                        0XC00052B4
#define LD_SCO_ENV_1306P_T1_ADDR                                      0XC000567C
#define RX_MONITOR_1306P_T1_ADDR                                      0XC0007C79
#define LC_ENV_1306P_T1_ADDR                                          0XC000504C
#define LM_NB_SYNC_ACTIVE_1306P_T1_ADDR                               0XC0005038
#define LM_ENV_1306P_T1_ADDR                                          0XC0004E04
#define HCI_ENV_1306P_T1_ADDR                                         0XC0006FA8
#define LC_SCO_ENV_1306P_T1_ADDR                                      0XC0005028
#define LLM_ENV_1306P_T1_ADDR                                         0XC0005BEC
#define LD_ENV_1306P_T1_ADDR                                          0XC000531C
#define RWIP_ENV_1306P_T1_ADDR                                        0XC0007D24
#define BLE_RX_MONITOR_1306P_T1_ADDR                                  0XC0007C11
#define LLC_ENV_1306P_T1_ADDR                                         0XC000648C
#define RWIP_RF_1306P_T1_ADDR                                         0XC0007CB0
#define LD_ACL_METRICS_1306P_T1_ADDR                                  0XC00053A4
#define RF_RX_HWGAIN_TBL_1306P_T1_ADDR                                0XC00049B6
#define RF_HWAGC_RSSI_CORRECT_TBL_1306P_T1_ADDR                       0XC00048D4
#define RF_RX_GAIN_FIXED_TBL_1306P_T1_ADDR                            0XC0007B84
#define HCI_DBG_EBQ_TEST_MODE_1306P_T1_ADDR                           0XC00072D0
#define DBG_BT_COMMON_SETTING_1306P_T1_ADDR                           0XC0007158
#define DBG_BT_SCHE_SETTING_1306P_T1_ADDR                             0XC0007214
#define DBG_BT_IBRT_SETTING_1306P_T1_ADDR                             0XC00071F8
#define DBG_BT_HW_FEAT_SETTING_1306P_T1_ADDR                          0XC00071D4
#define HCI_DBG_SET_SW_RSSI_1306P_T1_ADDR                             0XC0007300
#define RWIP_PROG_DELAY_1306P_T1_ADDR                                 0XC0007CAC
#define SCH_MULTI_IBRT_ADJUST_ENV_1306P_T1_ADDR                       0XC0006F08
#define RF_HWAGC_RSSI_CORRECT_TBL_1306P_T1_ADDR                       0XC00048D4
#define RF_RX_GAIN_THS_TBL_LE_1306P_T1_ADDR                           0XC000497A
#define RF_RX_GAIN_THS_TBL_LE_2M_1306P_T1_ADDR                        0XC0004998
#define RF_RPL_TX_PW_CONV_TBL_1306P_T1_ADDR                           0XC00048E4
#define REPLACE_MOBILE_ADDR_1306P_T1_ADDR                             0XC00052C0
#define REPLACE_ADDR_VALID_1306P_T1_ADDR                              0XC00052BC
#define PCM_NEED_START_FLAG_1306P_T1_ADDR                             0XC0005698
#define RF_RX_GAIN_THS_TBL_BT_3M_1306P_T1_ADDR                        0XC0007B90
#define LD_IBRT_ENV_1306P_T1_ADDR                                     0XC0005910
#define LLM_LOCAL_LE_FEATS_1306P_T1_ADDR                              0XC000477E
#define RF_RX_GAIN_THS_TBL_BT_1306P_T1_ADDR                           0XC0004920
#define DBG_BT_COMMON_SETTING_T2_1306P_T1_ADDR                        0XC000717E
#define LLD_CON_ENV_1306P_T1_ADDR                                     0XC0006864
#define LLD_ISO_ENV_1306P_T1_ADDR                                     0XC0006894
#define ECC_RX_MONITOR_1306P_T1_ADDR                                  0XC0007C35
#define __STACKLIMIT_1306P_T1_ADDR                                    0XC00088C0
#define PER_MONITOR_PARAMS_1306P_T1_ADDR                              0XC0004A6A
#define SENS2BT_EN_1306P_T1_ADDR                                      0XC0004050
#define RF_RX_GAIN_THS_TBL_ECC_1306P_T1_ADDR                          0XC000495C
#define LLD_BIS_ENV_1306P_T1_ADDR                                     0XC0006C40
#define LLD_BIG_ENV_1306P_T1_ADDR                                     0XC0006C30
#define LLD_CIS_ENV_1306P_T1_ADDR                                     0XC0006C00
#define LLD_CIG_ENV_1306P_T1_ADDR                                     0XC0006BF0
#define RX_MONITOR_3M_RXGAIN_1306P_T1_ADDR                            0XC000477D
#define INTERSYS_MSG_TX_MODE_1306P_T1_ADDR                            0XC0004030

#endif