
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_1306P_T0_SYMBOL_H__
#define  __BT_CONTROLLER_1306P_T0_SYMBOL_H__

/*****   Macro Definition of Relative Offset of Variables   *****/

// ld_acl_env_tag/rssi_record
#define OFF_RR_IN_LAET_LD_1306P_T0                                    0XE6
// ld_acl_env_tag/last_sync_bit_off
#define OFF_LSBO_IN_LAET_LD_1306P_T0                                  0XCE
// ld_acl_env_tag/t_poll
#define OFF_TP_IN_LAET_LD_1306P_T0                                    0XF0
// ld_acl_env_tag/last_sync_clk_off
#define OFF_LSCO_IN_LAET_LD_1306P_T0                                  0XB8
// ld_acl_env_tag/rxbit_1us
#define OFF_R1_IN_LAET_LD_1306P_T0                                    0XCA
// bt_util_buf_env_tag/acl_rx_free
#define OFF_ARF_IN_BUBET_BT_1306P_T0                                  0X14
// bt_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BT_1306P_T0                                  0X28
// ble_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BLE_1306P_T0                                 0X28
// dbg_bt_sche_setting_cmd/music_playing_link
#define OFF_MPL_IN_DBSSC_DBG_1306P_T0                                 0XA
// dbg_bt_sche_setting_cmd/acl_slot_in_ibrt_mode
#define OFF_ASIIM_IN_DBSSC_DBG_1306P_T0                               0XC
// dbg_bt_sche_setting_cmd/acl_interv_in_ibrt_normal_mode
#define OFF_AIIINM_IN_DBSSC_DBG_1306P_T0                              0X2
// dbg_bt_common_setting_cmd/trace_level
#define OFF_TL_IN_DBCSC_DBG_1306P_T0                                  0X8
// ld_bes_bt_env_tag/tws_device_env/link_id
#define OFF_LI_IN_LBBET_LD_1306P_T0                                   0X12
// dbg_bt_ibrt_setting_cmd/accept_remote_enter_sniff
#define OFF_ARES_IN_DBISC_DBG_1306P_T0                                0XF
// dbg_bt_ibrt_setting_cmd/ibrt_second_sco_decision
#define OFF_ISSD_IN_DBISC_DBG_1306P_T0                                0XE
// lc_sco_env_tag/p_link_params/link_id
#define OFF_LI_IN_LSET_LC_1306P_T0                                    0X1E
// lc_sco_nego_params_tag/air_params_received/m2s_pkt_type
#define OFF_MPT_IN_LSNPT_LC_1306P_T0                                  0X47
// lc_sco_nego_params_tag/air_params_received/air_mode
#define OFF_AM_IN_LSNPT_LC_1306P_T0                                   0X4E
// dbg_bt_hw_feat_setting_cmd/bt_sync_swagc_en
#define OFF_BSSE_IN_DBHFSC_DBG_1306P_T0                               0XF
// dbg_bt_hw_feat_setting_cmd/le_sync_swagc_en
#define OFF_LSSE_IN_DBHFSC_DBG_1306P_T0                               0X10
// rwip_env_tag/lp_clk
#define OFF_LC_IN_RET_RWIP_1306P_T0                                   0X39C
// rwip_env_tag/last_bt_samp_time/hs
#define OFF_H_IN_RET_RWIP_0_1306P_T0                                  0X28
// rwip_env_tag/last_bt_samp_time/hus
#define OFF_H_IN_RET_RWIP_1_1306P_T0                                  0X2C
// rwip_env_tag/last_us_samp_time
#define OFF_LUST_IN_RET_RWIP_1306P_T0                                 0X34
// lld_cis_env/grp_hdl
#define OFF_GH_IN_LCE_LLD_1306P_T0                                    0X1A
// lld_cig_env/anchor_hs
#define OFF_AH_IN_LCE_LLD_0_1306P_T0                                  0X80
// lld_cig_env/anchor_hus
#define OFF_AH_IN_LCE_LLD_1_1306P_T0                                  0X84
// lld_cig_env/iso_interval
#define OFF_II_IN_LCE_LLD_1306P_T0                                    0X92
// lld_bis_env/grp_hdl
#define OFF_GH_IN_LBE_LLD_1306P_T0                                    0X1C
// lld_big_env/anchor_hs
#define OFF_AH_IN_LBE_LLD_0_1306P_T0                                  0XE8
// lld_big_env/anchor_hus
#define OFF_AH_IN_LBE_LLD_1_1306P_T0                                  0XEC
// lld_big_env/iso_interval
#define OFF_II_IN_LBE_LLD_1306P_T0                                    0XEE
// dbg_bt_common_setting_t2/bt_master_sleep_en
#define OFF_BMSE_IN_DBCST_DBG_1306P_T0                                0X2F
// dbg_bt_common_setting_t2/bt_master_tx_silence_en
#define OFF_BMTSE_IN_DBCST_DBG_1306P_T0                               0X30


#define HCI_FC_ENV_1306P_T0_ADDR                                      0XC0006A40
#define LD_ACL_ENV_1306P_T0_ADDR                                      0XC00053DC
#define BT_UTIL_BUF_ENV_1306P_T0_ADDR                                 0XC0004BDC
#define BLE_UTIL_BUF_ENV_1306P_T0_ADDR                                0XC0005880
#define LD_BES_BT_ENV_1306P_T0_ADDR                                   0XC00057DC
#define DBG_STATE_1306P_T0_ADDR                                       0XC0006C58
#define LC_STATE_1306P_T0_ADDR                                        0XC00052F4
#define LD_SCO_ENV_1306P_T0_ADDR                                      0XC00056B4
#define RX_MONITOR_1306P_T0_ADDR                                      0XC0007678
#define LC_ENV_1306P_T0_ADDR                                          0XC000508C
#define LM_NB_SYNC_ACTIVE_1306P_T0_ADDR                               0XC0005078
#define LM_ENV_1306P_T0_ADDR                                          0XC0004E44
#define HCI_ENV_1306P_T0_ADDR                                         0XC0006958
#define LC_SCO_ENV_1306P_T0_ADDR                                      0XC0005068
#define LLM_ENV_1306P_T0_ADDR                                         0XC0005ABC
#define LD_ENV_1306P_T0_ADDR                                          0XC0005364
#define RWIP_ENV_1306P_T0_ADDR                                        0XC0007714
#define BLE_RX_MONITOR_1306P_T0_ADDR                                  0XC00075D8
#define LLC_ENV_1306P_T0_ADDR                                         0XC0006100
#define RWIP_RF_1306P_T0_ADDR                                         0XC00076B0
#define LD_ACL_METRICS_1306P_T0_ADDR                                  0XC00053EC
#define RF_RX_HWGAIN_TBL_1306P_T0_ADDR                                0XC00049A8
#define RF_HWAGC_RSSI_CORRECT_TBL_1306P_T0_ADDR                       0XC00048B8
#define RF_RX_GAIN_FIXED_TBL_1306P_T0_ADDR                            0XC0007510
#define HCI_DBG_EBQ_TEST_MODE_1306P_T0_ADDR                           0XC0006C74
#define DBG_BT_COMMON_SETTING_1306P_T0_ADDR                           0XC0006B04
#define DBG_BT_SCHE_SETTING_1306P_T0_ADDR                             0XC0006BB0
#define DBG_BT_IBRT_SETTING_1306P_T0_ADDR                             0XC0006B90
#define DBG_BT_HW_FEAT_SETTING_1306P_T0_ADDR                          0XC0006B6C
#define HCI_DBG_SET_SW_RSSI_1306P_T0_ADDR                             0XC0006CA4
#define RWIP_PROG_DELAY_1306P_T0_ADDR                                 0XC00076AC
#define DATA_BACKUP_CNT_1306P_T0_ADDR                                 0XC0004070
#define DATA_BACKUP_ADDR_PTR_1306P_T0_ADDR                            0XC0004074
#define DATA_BACKUP_VAL_PTR_1306P_T0_ADDR                             0XC0004078
#define SCH_MULTI_IBRT_ADJUST_ENV_1306P_T0_ADDR                       0XC00068B8
#define RF_HWAGC_RSSI_CORRECT_TBL_1306P_T0_ADDR                       0XC00048B8
#define RF_RX_GAIN_THS_TBL_LE_1306P_T0_ADDR                           0XC0004968
#define RF_RX_GAIN_THS_TBL_LE_2M_1306P_T0_ADDR                        0XC0004988
#define RF_RPL_TX_PW_CONV_TBL_1306P_T0_ADDR                           0XC00048C8
#define REPLACE_MOBILE_ADDR_1306P_T0_ADDR                             0XC00052FC
#define REPLACE_ADDR_VALID_1306P_T0_ADDR                              0XC00052F8
#define PCM_NEED_START_FLAG_1306P_T0_ADDR                             0XC00056D0
#define RT_SLEEP_FLAG_CLEAR_1306P_T0_ADDR                             0XC00080BC
#define RF_RX_GAIN_THS_TBL_BT_3M_1306P_T0_ADDR                        0XC000751C
#define LD_IBRT_ENV_1306P_T0_ADDR                                     0XC00057F0
#define LLM_LOCAL_LE_FEATS_1306P_T0_ADDR                              0XC000477C
#define RF_RX_GAIN_THS_TBL_BT_1306P_T0_ADDR                           0XC0004908
#define DBG_BT_COMMON_SETTING_T2_1306P_T0_ADDR                        0XC0006B2C
#define LLD_CON_ENV_1306P_T0_ADDR                                     0XC00063A8
#define LLD_ISO_ENV_1306P_T0_ADDR                                     0XC00063C8
#define ECC_RX_MONITOR_1306P_T0_ADDR                                  0XC000760C
#define __STACKLIMIT_1306P_T0_ADDR                                    0XC0008220
#define PER_MONITOR_PARAMS_1306P_T0_ADDR                              0XC0004774
#define SENS2BT_EN_1306P_T0_ADDR                                      0XC0004050
#define RF_RX_GAIN_THS_TBL_ECC_1306P_T0_ADDR                          0XC0004948
#define LLD_BIS_ENV_1306P_T0_ADDR                                     0XC0006594
#define LLD_BIG_ENV_1306P_T0_ADDR                                     0XC000658C
#define LLD_CIS_ENV_1306P_T0_ADDR                                     0XC000656C
#define LLD_CIG_ENV_1306P_T0_ADDR                                     0XC0006564
#define RX_MONITOR_3M_RXGAIN_1306P_T0_ADDR                            0XC0004A01
#define INTERSYS_MSG_TX_MODE_1306P_T0_ADDR                            0XC0004030

#endif