
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_1503_T0_SYMBOL_H__
#define  __BT_CONTROLLER_1503_T0_SYMBOL_H__

/*****    Macro Definition of Relative Offset of Variables    *****/

// ld_acl_env_tag/rssi_record
#define OFF_RR_IN_LAET_LD_T0                                          0XD6
// ld_acl_env_tag/last_sync_bit_off
#define OFF_LSBO_IN_LAET_LD_T0                                        0XBE
// ld_acl_env_tag/t_poll
#define OFF_TP_IN_LAET_LD_T0                                          0XE0
// ld_acl_env_tag/last_sync_clk_off
#define OFF_LSCO_IN_LAET_LD_T0                                        0XA8
// ld_acl_env_tag/rxbit_1us
#define OFF_R1_IN_LAET_LD_T0                                          0XBA
// bt_util_buf_env_tag/acl_rx_free
#define OFF_ARF_IN_BUBET_BT_T0                                        0X14
// bt_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BT_T0                                        0X28
// ble_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BLE_T0                                       0X28
// dbg_bt_sche_setting_cmd/music_playing_link
#define OFF_MPL_IN_DBSSC_DBG_T0                                       0XA
// dbg_bt_sche_setting_cmd/acl_slot_in_ibrt_mode
#define OFF_ASIIM_IN_DBSSC_DBG_T0                                     0XC
// dbg_bt_sche_setting_cmd/acl_interv_in_ibrt_normal_mode
#define OFF_AIIINM_IN_DBSSC_DBG_T0                                    0X2
// dbg_bt_common_setting_cmd/trace_level
#define OFF_TL_IN_DBCSC_DBG_T0                                        0X8
// ld_bes_bt_env_tag/tws_device_env/link_id
#define OFF_LI_IN_LBBET_LD_T0                                         0X12
// dbg_bt_ibrt_setting_cmd/accept_remote_enter_sniff
#define OFF_ARES_IN_DBISC_DBG_T0                                      0XF
// dbg_bt_ibrt_setting_cmd/ibrt_second_sco_decision
#define OFF_ISSD_IN_DBISC_DBG_T0                                      0XE
// lc_sco_env_tag/p_link_params/link_id
#define OFF_LI_IN_LSET_LC_T0                                          0X1E
// lc_sco_nego_params_tag/air_params_received/m2s_pkt_type
#define OFF_MPT_IN_LSNPT_LC_T0                                        0X47
// lc_sco_nego_params_tag/air_params_received/air_mode
#define OFF_AM_IN_LSNPT_LC_T0                                         0X4E
// dbg_bt_hw_feat_setting_cmd/bt_sync_swagc_en
#define OFF_BSSE_IN_DBHFSC_DBG_T0                                     0XF
// rwip_env_tag/lp_clk
#define OFF_LC_IN_RET_RWIP_T0                                         0X200
// rwip_env_tag/last_bt_samp_time/hs
#define OFF_H_IN_RET_RWIP_0_T0                                        0X1C
// rwip_env_tag/last_bt_samp_time/hus
#define OFF_H_IN_RET_RWIP_1_T0                                        0X20
// rwip_env_tag/last_us_samp_time
#define OFF_LUST_IN_RET_RWIP_T0                                       0X24
// lld_cis_env/grp_hdl
#define OFF_GH_IN_LCE_LLD_T0                                          0X1A
// lld_cig_env/anchor_hs
#define OFF_AH_IN_LCE_LLD_0_T0                                        0X74
// lld_cig_env/anchor_hus
#define OFF_AH_IN_LCE_LLD_1_T0                                        0X78
// lld_cig_env/iso_interval
#define OFF_II_IN_LCE_LLD_T0                                          0X86
// lld_bis_env/grp_hdl
#define OFF_GH_IN_LBE_LLD_T0                                          0X1C
// lld_big_env/anchor_hs
#define OFF_AH_IN_LBE_LLD_0_T0                                        0XD4
// lld_big_env/anchor_hus
#define OFF_AH_IN_LBE_LLD_1_T0                                        0XD8
// lld_big_env/iso_interval
#define OFF_II_IN_LBE_LLD_T0                                          0XDA


#define HCI_FC_ENV_T0_ADDR                                            0XC00083A0
#define LD_ACL_ENV_T0_ADDR                                            0XC0005758
#define BT_UTIL_BUF_ENV_T0_ADDR                                       0XC0004C70
#define BLE_UTIL_BUF_ENV_T0_ADDR                                      0XC0005B18
#define LD_BES_BT_ENV_T0_ADDR                                         0XC00058D0
#define DBG_STATE_T0_ADDR                                             0XC00084F0
#define LC_STATE_T0_ADDR                                              0XC00050F4
#define LD_SCO_ENV_T0_ADDR                                            0XC0005774
#define RX_MONITOR_T0_ADDR                                            0XC0008C7C
#define LC_ENV_T0_ADDR                                                0XC00050D0
#define LM_NB_SYNC_ACTIVE_T0_ADDR                                     0XC00050CA
#define LM_ENV_T0_ADDR                                                0XC0004E40
#define HCI_ENV_T0_ADDR                                               0XC0008298
#define LC_SCO_ENV_T0_ADDR                                            0XC00050AC
#define LLM_ENV_T0_ADDR                                               0XC0005F28
#define LD_ENV_T0_ADDR                                                0XC00053CC
#define RWIP_ENV_T0_ADDR                                              0XC0008E48
#define BLE_RX_MONITOR_T0_ADDR                                        0XC0008BFC
#define LLC_ENV_T0_ADDR                                               0XC0007318
#define RWIP_RF_T0_ADDR                                               0XC0008DE4
#define LD_ACL_METRICS_T0_ADDR                                        0XC0005410
#define RF_RX_HWGAIN_TBL_T0_ADDR                                      0XC000499C
#define RF_HWAGC_RSSI_CORRECT_TBL_T0_ADDR                             0XC00048AC
#define RF_RX_GAIN_FIXED_TBL_T0_ADDR                                  0XC0008BB8
#define HCI_DBG_EBQ_TEST_MODE_T0_ADDR                                 0XC00085B0
#define DBG_BT_COMMON_SETTING_T0_ADDR                                 0XC0008550
#define DBG_BT_SCHE_SETTING_T0_ADDR                                   0XC00085E0
#define DBG_BT_IBRT_SETTING_T0_ADDR                                   0XC0008474
#define DBG_BT_HW_FEAT_SETTING_T0_ADDR                                0XC0008528
#define HCI_DBG_SET_SW_RSSI_T0_ADDR                                   0XC00085F8
#define RWIP_PROG_DELAY_T0_ADDR                                       0XC0008E44
#define DATA_BACKUP_CNT_T0_ADDR                                       0XC0004070
#define DATA_BACKUP_ADDR_PTR_T0_ADDR                                  0XC0004074
#define DATA_BACKUP_VAL_PTR_T0_ADDR                                   0XC0004078
#define SCH_MULTI_IBRT_ADJUST_ENV_T0_ADDR                             0XC0008250
#define RF_HWAGC_RSSI_CORRECT_TBL_T0_ADDR                             0XC00048AC
#define RF_RX_GAIN_THS_TBL_LE_T0_ADDR                                 0XC000495C
#define RF_RX_GAIN_THS_TBL_LE_2M_T0_ADDR                              0XC000497C
#define RF_RPL_TX_PW_CONV_TBL_T0_ADDR                                 0XC00048BC
#define REPLACE_MOBILE_ADDR_T0_ADDR                                   0XC0005370
#define REPLACE_ADDR_VALID_T0_ADDR                                    0XC0004BAE
#define PCM_NEED_START_FLAG_T0_ADDR                                   0XC0004BD0
#define RT_SLEEP_FLAG_CLEAR_T0_ADDR                                   0XC0004C60
#define RF_RX_GAIN_THS_TBL_BT_3M_T0_ADDR                              0XC0008B98
#define NORMAL_IQTAB_T0_ADDR                                          0XC0008C4C
#define NORMAL_IQTAB_EN_T0_ADDR                                       0XC0004C04
#define LD_IBRT_ENV_T0_ADDR                                           0XC00058E8
#define LLM_LOCAL_LE_FEATS_T0_ADDR                                    0XC0004780
#define RF_RX_GAIN_THS_TBL_BT_T0_ADDR                                 0XC00048FC
#define DBG_BT_COMMON_SETTING_T2_T0_ADDR                              0XC0008578
#define LLD_CON_ENV_T0_ADDR                                           0XC000782C
#define LLD_ISO_ENV_T0_ADDR                                           0XC000787C
#define ECC_RX_MONITOR_T0_ADDR                                        0XC0008B90
#define __STACKLIMIT_T0_ADDR                                          0XC00098F8
#define PER_MONITOR_PARAMS_T0_ADDR                                    0XC0004778
#define RF_RX_GAIN_THS_TBL_ECC_T0_ADDR                                0XC000493C
#define RX_MONITOR_3M_RXGAIN_T0_ADDR                                  0XC00049E5
#define LLD_CIG_ENV_T0_ADDR                                           0XC0007D78
#define LLD_CIS_ENV_T0_ADDR                                           0XC0007D98
#define LLD_BIS_ENV_T0_ADDR                                           0XC0007E08
#define LLD_BIG_ENV_T0_ADDR                                           0XC0007DE8
#define CON_LATENCY_APPLY_T0_ADDR                                     0XC00047A0

#endif