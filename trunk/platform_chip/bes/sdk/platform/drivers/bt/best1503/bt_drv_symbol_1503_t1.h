
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_1503_T1_SYMBOL_H__
#define  __BT_CONTROLLER_1503_T1_SYMBOL_H__

/*****   Macro Definition of Relative Offset of Variables   *****/

// ld_acl_env_tag/rssi_record
#define OFF_RR_IN_LAET_LD_T1                                          0XD6
// ld_acl_env_tag/last_sync_bit_off
#define OFF_LSBO_IN_LAET_LD_T1                                        0XBE
// ld_acl_env_tag/t_poll
#define OFF_TP_IN_LAET_LD_T1                                          0XE0
// ld_acl_env_tag/last_sync_clk_off
#define OFF_LSCO_IN_LAET_LD_T1                                        0XA8
// ld_acl_env_tag/rxbit_1us
#define OFF_R1_IN_LAET_LD_T1                                          0XBA
// bt_util_buf_env_tag/acl_rx_free
#define OFF_ARF_IN_BUBET_BT_T1                                        0X14
// bt_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BT_T1                                        0X28
// ble_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BLE_T1                                       0X28
// dbg_bt_sche_setting_cmd/music_playing_link
#define OFF_MPL_IN_DBSSC_DBG_T1                                       0XA
// dbg_bt_sche_setting_cmd/acl_slot_in_ibrt_mode
#define OFF_ASIIM_IN_DBSSC_DBG_T1                                     0XC
// dbg_bt_sche_setting_cmd/acl_interv_in_ibrt_normal_mode
#define OFF_AIIINM_IN_DBSSC_DBG_T1                                    0X2
// dbg_bt_common_setting_cmd/trace_level
#define OFF_TL_IN_DBCSC_DBG_T1                                        0X8
// ld_bes_bt_env_tag/tws_device_env/link_id
#define OFF_LI_IN_LBBET_LD_T1                                         0X12
// dbg_bt_ibrt_setting_cmd/accept_remote_enter_sniff
#define OFF_ARES_IN_DBISC_DBG_T1                                      0XF
// dbg_bt_ibrt_setting_cmd/ibrt_second_sco_decision
#define OFF_ISSD_IN_DBISC_DBG_T1                                      0XE
// lc_sco_env_tag/p_link_params/link_id
#define OFF_LI_IN_LSET_LC_T1                                          0X1E
// lc_sco_nego_params_tag/air_params_received/m2s_pkt_type
#define OFF_MPT_IN_LSNPT_LC_T1                                        0X47
// lc_sco_nego_params_tag/air_params_received/air_mode
#define OFF_AM_IN_LSNPT_LC_T1                                         0X4E
// dbg_bt_hw_feat_setting_cmd/bt_sync_swagc_en
#define OFF_BSSE_IN_DBHFSC_DBG_T1                                     0XF
// rwip_env_tag/lp_clk
#define OFF_LC_IN_RET_RWIP_T1                                         0X200
// rwip_env_tag/last_bt_samp_time/hs
#define OFF_H_IN_RET_RWIP_0_T1                                        0X1C
// rwip_env_tag/last_bt_samp_time/hus
#define OFF_H_IN_RET_RWIP_1_T1                                        0X20
// rwip_env_tag/last_us_samp_time
#define OFF_LUST_IN_RET_RWIP_T1                                       0X24
// lld_cis_env/grp_hdl
#define OFF_GH_IN_LCE_LLD_T1                                          0X1A
// lld_cig_env/anchor_hs
#define OFF_AH_IN_LCE_LLD_0_T1                                        0X74
// lld_cig_env/anchor_hus
#define OFF_AH_IN_LCE_LLD_1_T1                                        0X78
// lld_cig_env/iso_interval
#define OFF_II_IN_LCE_LLD_T1                                          0X86
// lld_bis_env/grp_hdl
#define OFF_GH_IN_LBE_LLD_T1                                          0X1C
// lld_big_env/anchor_hs
#define OFF_AH_IN_LBE_LLD_0_T1                                        0XD4
// lld_big_env/anchor_hus
#define OFF_AH_IN_LBE_LLD_1_T1                                        0XD8
// lld_big_env/iso_interval
#define OFF_II_IN_LBE_LLD_T1                                          0XDA


#define HCI_FC_ENV_T1_ADDR                                            0XC0009C78
#define LD_ACL_ENV_T1_ADDR                                            0XC0005774
#define BT_UTIL_BUF_ENV_T1_ADDR                                       0XC0004C8C
#define BLE_UTIL_BUF_ENV_T1_ADDR                                      0XC0005B34
#define LD_BES_BT_ENV_T1_ADDR                                         0XC00058EC
#define DBG_STATE_T1_ADDR                                             0XC0009DB0
#define LC_STATE_T1_ADDR                                              0XC0005110
#define LD_SCO_ENV_T1_ADDR                                            0XC0005790
#define RX_MONITOR_T1_ADDR                                            0XC000A530
#define LC_ENV_T1_ADDR                                                0XC00050EC
#define LM_NB_SYNC_ACTIVE_T1_ADDR                                     0XC00050E6
#define LM_ENV_T1_ADDR                                                0XC0004E5C
#define HCI_ENV_T1_ADDR                                               0XC0009B70
#define LC_SCO_ENV_T1_ADDR                                            0XC00050C8
#define LLM_ENV_T1_ADDR                                               0XC0005F44
#define LD_ENV_T1_ADDR                                                0XC00053E8
#define RWIP_ENV_T1_ADDR                                              0XC000A708
#define BLE_RX_MONITOR_T1_ADDR                                        0XC000A4D0
#define LLC_ENV_T1_ADDR                                               0XC0008B98
#define RWIP_RF_T1_ADDR                                               0XC000A698
#define LD_ACL_METRICS_T1_ADDR                                        0XC000542C
#define RF_RX_HWGAIN_TBL_T1_ADDR                                      0XC00049AC
#define RF_HWAGC_RSSI_CORRECT_TBL_T1_ADDR                             0XC00048BC
#define RF_RX_GAIN_FIXED_TBL_T1_ADDR                                  0XC000A48C
#define HCI_DBG_EBQ_TEST_MODE_T1_ADDR                                 0XC0009E7C
#define DBG_BT_COMMON_SETTING_T1_ADDR                                 0XC0009E10
#define DBG_BT_SCHE_SETTING_T1_ADDR                                   0XC0009EAC
#define DBG_BT_IBRT_SETTING_T1_ADDR                                   0XC0009D34
#define DBG_BT_HW_FEAT_SETTING_T1_ADDR                                0XC0009DE8
#define HCI_DBG_SET_SW_RSSI_T1_ADDR                                   0XC0009EC4
#define RWIP_PROG_DELAY_T1_ADDR                                       0XC000A704
#define DATA_BACKUP_CNT_T1_ADDR                                       0XC0004070
#define DATA_BACKUP_ADDR_PTR_T1_ADDR                                  0XC0004074
#define DATA_BACKUP_VAL_PTR_T1_ADDR                                   0XC0004078
#define SCH_MULTI_IBRT_ADJUST_ENV_T1_ADDR                             0XC0009B28
#define RF_HWAGC_RSSI_CORRECT_TBL_T1_ADDR                             0XC00048BC
#define RF_RX_GAIN_THS_TBL_LE_T1_ADDR                                 0XC000496C
#define RF_RX_GAIN_THS_TBL_LE_2M_T1_ADDR                              0XC000498C
#define RF_RPL_TX_PW_CONV_TBL_T1_ADDR                                 0XC00048CC
#define REPLACE_MOBILE_ADDR_T1_ADDR                                   0XC000538C
#define REPLACE_ADDR_VALID_T1_ADDR                                    0XC0004BC6
#define PCM_NEED_START_FLAG_T1_ADDR                                   0XC0004BE8
#define RT_SLEEP_FLAG_CLEAR_T1_ADDR                                   0XC0004C7C
#define RF_RX_GAIN_THS_TBL_BT_3M_T1_ADDR                              0XC000A46C
#define LD_IBRT_ENV_T1_ADDR                                           0XC0005904
#define LLM_LOCAL_LE_FEATS_T1_ADDR                                    0XC0004780
#define RF_RX_GAIN_THS_TBL_BT_T1_ADDR                                 0XC000490C
#define DBG_BT_COMMON_SETTING_T2_T1_ADDR                              0XC0009E40
#define LLD_CON_ENV_T1_ADDR                                           0XC00090B4
#define LLD_ISO_ENV_T1_ADDR                                           0XC0009104
#define ECC_RX_MONITOR_T1_ADDR                                        0XC000A464
#define __STACKLIMIT_T1_ADDR                                          0XC000B018
#define PER_MONITOR_PARAMS_T1_ADDR                                    0XC0004778
#define SENS2BT_EN_T1_ADDR                                            0XC0004050
#define RF_RX_GAIN_THS_TBL_ECC_T1_ADDR                                0XC000494C
#define LLD_BIS_ENV_T1_ADDR                                           0XC0009690
#define LLD_BIG_ENV_T1_ADDR                                           0XC0009670
#define LLD_CIS_ENV_T1_ADDR                                           0XC0009620
#define LLD_CIG_ENV_T1_ADDR                                           0XC0009600
#define RX_MONITOR_3M_RXGAIN_T1_ADDR                                  0XC0004A05
#define CON_LATENCY_APPLY_T1_ADDR                                     0XC00047A0

#endif