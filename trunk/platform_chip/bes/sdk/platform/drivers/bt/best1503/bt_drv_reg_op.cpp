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
//Common
#include <string.h>
#include <besbt_string.h>
#include "hal_chipid.h"
#include "hal_intersys.h"
#include "bt_drv.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_internal.h"

#include "bt_drv_interface.h"


//CHIP related
#include "bt_drv_1503_internal.h"
#include "bt_1503_reg_map.h"
#include "bt_drv_1503_config.h"
#ifdef __BT_RAMRUN_NEW__
#include "bt_drv_ramrun_symbol_1503.h"
#else
#include "bt_drv_symbol_1503_t0.h"
#include "bt_drv_symbol_1503_t1.h"
#include "bt_drv_symbol_1503_t2.h"
#include "bt_drv_symbol_1503_t3.h"
#endif
#include "bt_patch_1503_t0.h"
#include "bt_patch_1503_t1.h"
#include "bt_patch_1503_t2.h"
#include "bt_patch_1503_t3.h"
#include CHIP_SPECIFIC_HDR(bt_drv_modem_reg_map)

/***************************************************************************
*
*Warning: Prohibit reading and writing EM memory in Bluetooth drivers
*
*****************************************************************************/
//MCU_WAKEUP_BT_V2 macro will be removed ASAP

#ifdef MCU_WAKEUP_BT_V2
#define BT_DRV_REG_OP_FORCE_WAKE_ENB()  do {    \
                                            bt_drv_oper_btcore_with_rsp(WAKEUP_BT_USER_DRV, MSG_WAKEUP_BT_AND_DIS_SLP);  \

#define BT_DRV_REG_OP_FORCE_WAKE_DIB()  \
                                            bt_drv_oper_btcore_with_rsp(WAKEUP_BT_USER_DRV, MSG_WAKEUP_BT_AND_EN_SLP);   \
                                        } while(0);

#define BT_DRV_REG_OP_CLK_ENB()         BT_DRV_REG_OP_FORCE_WAKE_ENB()
#define BT_DRV_REG_OP_CLK_TRY_ENB()     BT_DRV_REG_OP_FORCE_WAKE_ENB()
#define BT_DRV_REG_OP_CLK_DIB()         BT_DRV_REG_OP_FORCE_WAKE_DIB()
#else /* MCU_WAKEUP_BT_V2 */
static uint32_t reg_op_clk_enb = 0;
POSSIBLY_UNUSED static bool assert_onging = false;
#define BT_DRV_REG_OP_FORCE_WAKE_ENB()  do{\
                                            uint32_t op_clk_lock = int_lock(); \
                                            uint32_t local_enb = reg_op_clk_enb; \
                                            int_unlock(op_clk_lock); \
                                            if (!local_enb){ \
                                                    uint32_t curr_tick,temp_tick,loop_cnt; \
                                                    hal_intersys_wakeup_btcore(); \
                                                    curr_tick = hal_sys_timer_get();  \
                                                    loop_cnt = 0;\
                                                    do{  \
                                                        if(assert_onging){\
                                                            break; \
                                                        } \
                                                        hal_sys_timer_delay_us(100);\
                                                        temp_tick = hal_sys_timer_get();\
                                                        if(temp_tick - curr_tick > MS_TO_TICKS(3)) \
                                                        { \
                                                            TRACE(0,"BT:Wait BT rsp 3ms,cnt=%d,loop=%d", reg_op_clk_enb, loop_cnt);\
                                                            hal_intersys_wakeup_btcore();  \
                                                            curr_tick = hal_sys_timer_get();  \
                                                            loop_cnt++;\
                                                            if(loop_cnt >= 10) \
                                                            {\
                                                                assert_onging = true; \
                                                                ASSERT(0,"BT:wakeup BT ERROR");\
                                                                break;\
                                                            }\
                                                        }  \
                                                    }while(hal_intersys_get_wakeup_flag());  \
                                            } \
                                            op_clk_lock = int_lock(); \
                                            reg_op_clk_enb++; \
                                            int_unlock(op_clk_lock);

#define BT_DRV_REG_OP_FORCE_WAKE_DIB()      op_clk_lock = int_lock(); \
                                            reg_op_clk_enb--; \
                                            if (!reg_op_clk_enb){ \
                                                bt_drv_clear_wakeup_request(); \
                                            } \
                                            int_unlock(op_clk_lock); \
                                        }while(0);

#ifdef BT_LOG_POWEROFF
#define BT_DRV_REG_OP_CLK_ENB()         BT_DRV_REG_OP_FORCE_WAKE_ENB()

//only for BTC crash dump
#define BT_DRV_REG_OP_CLK_TRY_ENB()     do{\
                                            uint32_t op_clk_lock = int_lock(); \
                                            uint32_t local_enb = reg_op_clk_enb; \
                                            int_unlock(op_clk_lock); \
                                            if (!local_enb){ \
                                                    uint32_t curr_tick,temp_tick,loop_cnt; \
                                                    hal_intersys_wakeup_btcore(); \
                                                     curr_tick = hal_sys_timer_get();  \
                                                     loop_cnt = 0;\
                                                    do{  \
                                                        if(assert_onging){\
                                                            break; \
                                                        } \
                                                        hal_sys_timer_delay_us(100);\
                                                        temp_tick = hal_sys_timer_get();\
                                                        if(temp_tick - curr_tick > MS_TO_TICKS(3)) \
                                                        { \
                                                            hal_intersys_wakeup_btcore();  \
                                                            TRACE(0,"BT:Wait BT rsp 3ms");\
                                                            curr_tick = hal_sys_timer_get();  \
                                                            loop_cnt++;\
                                                            if(loop_cnt >= 10) \
                                                            {\
                                                                assert_onging = true; \
                                                                TRACE(0,"BT:TRY_ENB ERROR");\
                                                                break;\
                                                            }\
                                                        }  \
                                                    }while(hal_intersys_get_wakeup_flag());  \
                                            } \
                                            op_clk_lock = int_lock(); \
                                            reg_op_clk_enb++; \
                                            TRACE(0,"%s force in, cnt=%d",__func__, reg_op_clk_enb);\
                                            int_unlock(op_clk_lock);

#define BT_DRV_REG_OP_CLK_DIB()             op_clk_lock = int_lock(); \
                                            reg_op_clk_enb--; \
                                            if (!reg_op_clk_enb){ \
                                                bt_drv_clear_wakeup_request(); \
                                            } \
                                            int_unlock(op_clk_lock); \
                                    }while(0);

#else /* BT_LOG_POWEROFF */
#define BT_DRV_REG_OP_CLK_ENB()    do{\
                                        uint32_t op_clk_lock = int_lock(); \
                                        if (!reg_op_clk_enb){ \
                                            hal_cmu_bt_sys_clock_force_on(); \
                                        } \
                                        reg_op_clk_enb++; \
                                        int_unlock(op_clk_lock);

#define BT_DRV_REG_OP_CLK_TRY_ENB() BT_DRV_REG_OP_CLK_ENB()

#define BT_DRV_REG_OP_CLK_DIB()         op_clk_lock = int_lock(); \
                                        reg_op_clk_enb--; \
                                        if (!reg_op_clk_enb){ \
                                            hal_cmu_bt_sys_clock_auto(); \
                                        } \
                                        int_unlock(op_clk_lock); \
                                    }while(0);
#endif /* BT_LOG_POWEROFF */
#endif /* MCU_WAKEUP_BT_V2 */

struct dbg_set_ebq_test_cmd
{
    uint8_t ssr;
    uint8_t aes_ccm_daycounter;
    uint8_t bb_prot_flh_bv02;
    uint8_t bb_prot_arq_bv43;
    uint8_t enc_mode_req;
    uint8_t lmp_lih_bv126;
    uint8_t lsto_add;
    uint8_t bb_xcb_bv06;
    uint8_t bb_xcb_bv20;
    uint8_t bb_inq_pag_bv01;
    uint8_t sam_status_change_evt;
    uint8_t sam_remap;
    uint8_t ll_con_sla_bi02c;
    int16_t ll_con_sla_bi02c_offset;
    uint8_t ll_pcl_mas_sla_bv01;
    uint8_t ll_pcl_sla_bv29;
    uint8_t bb_prot_arq_bv0608;
    uint8_t lmp_lih_bv48;
    uint8_t hfp_hf_acs_bv17_i;
    uint8_t ll_not_support_phy;
    uint8_t ll_iso_hci_out_buf_num;
    uint8_t ll_iso_hci_in_sdu_len_flag;
    uint8_t ll_iso_hci_in_buf_num;
    uint8_t ll_cis_mic_present;
    uint8_t ll_cig_param_check_disable;
    uint8_t ll_total_num_iso_data_pkts;
    uint32_t cis_offset_min;
    uint32_t cis_sub_event_duration_config;
    uint8_t ll_bi_alarm_init_distance;//(in half slots)
    uint16_t lld_sync_duration_min;//(in half us)
    uint16_t lld_le_duration_min;//(in half us)
    uint16_t lld_cibi_sync_win_min_distance;//(in half us)
    uint8_t iso_sub_event_duration_config;
    uint8_t ll_bis_snc_bv7_bv11;//ebq bug
};

struct hci_dbg_ble_cmd
{
    uint16_t ble_agc_record_num;
    uint16_t cis_con_accept_to;//(in ms)
    uint16_t isoohci_in_buf_size;
    uint8_t isoohci_in_buf_nb;
    uint8_t isoohci_in_buf_malloc_en;
    uint8_t ll_iso_hci_out_buf_num;
    uint8_t ble_con_sch_prio_en;
    uint8_t ble_con_sch_prio_inc_max_en;
    uint8_t ble_con_sch_dft_prio_en;
    uint8_t bis_first_sch_min_distance;//(in half slots)
    uint8_t bis_channel_scan;
    uint8_t ble_con_sync_found_check;
    uint8_t ble_cis_sync_found_check;
    uint8_t ble_bis_sync_found_check;
    uint8_t ble_sync_sync_found_check;
    uint8_t ble_adv_high_duty_prio_max;
    uint8_t ble_agc_record_en;
};

struct dbg_set_txpwr_mode_cmd
{
    uint8_t     enable;
    //1: -6dbm, 2:-12dbm
    uint8_t     factor;
    uint8_t     bt_or_ble;//bt_or_ble  0: bt 1:ble
    uint8_t     link_id;
};

static uint32_t bt_ram_start_addr=0;
static uint32_t hci_fc_env_addr=0;
static uint32_t ld_acl_env_addr=0;
static uint32_t bt_util_buf_env_addr=0;
static uint32_t ble_util_buf_env_addr=0;
static uint32_t ld_bes_bt_env_addr=0;
static uint32_t lc_state_addr=0;
static uint32_t ld_sco_env_addr=0;
static uint32_t rx_monitor_addr=0;
static uint32_t lm_env_addr=0;
static uint32_t lc_sco_env_addr=0;
static uint32_t llm_env_addr=0;
static uint32_t rwip_env_addr=0;
static uint32_t ble_rx_monitor_addr=0;
#ifdef __DEBUG_FIX_RX_GAIN_CNTL__
static uint32_t rf_rx_gain_fixed_tbl_addr = 0;
#endif
static uint32_t hci_dbg_ebq_test_mode_addr = 0;
static uint32_t dbg_bt_common_setting_addr=0;
static uint32_t dbg_bt_sche_setting_addr=0;
static uint32_t dbg_bt_ibrt_setting_addr=0;
static uint32_t dbg_bt_hw_feat_setting_addr=0;
static uint32_t hci_dbg_set_sw_rssi_addr = 0;
static uint32_t data_backup_cnt_addr=0;
static uint32_t data_backup_addr_ptr_addr=0;
static uint32_t data_backup_val_ptr_addr=0;
static uint32_t sch_multi_ibrt_adjust_env_addr=0;
static uint32_t POSSIBLY_UNUSED workmode_patch_version_addr = 0;
static uint32_t rf_rx_gain_ths_tbl_le_addr = 0;
static uint32_t pcm_need_start_flag_addr = 0;
static uint32_t POSSIBLY_UNUSED rt_sleep_flag_clear_addr = 0;
static uint32_t le_ext_adv_tx_pwr_independent_addr = 0;
static uint32_t llm_local_le_feats_addr = 0;
static uint32_t txpwr_host_set_flg_addr = 0;
static uint32_t txpwr_val_buf_addr = 0;
static uint32_t rf_rx_gain_ths_tbl_le_2m_addr = 0;
static uint32_t btdrv_txpwr_conv_tbl_addr = 0;
static uint32_t btdrv_ble_txpwr_conv_tbl_addr = 0;
static uint32_t ecc_rx_monitor_addr =0;
static uint32_t btc_stack_limited_addr=0;
static uint32_t lld_iso_env_addr = 0;
static uint32_t dbg_bt_common_setting_t2_addr=0;
static uint32_t POSSIBLY_UNUSED bt_wifi_coexistence_disable_addr = 0;
static uint32_t sens2bt_en_addr=0;
static uint32_t rf_rx_gain_ths_tbl_ecc_addr = 0;
static uint32_t POSSIBLY_UNUSED rx_record_addr = 0;
#ifdef __NEW_SWAGC_MODE__
static uint32_t rf_rx_gain_ths_tbl_3m_addr = 0;
static uint32_t rx_monitor_3m_rxgain_addr = 0;
#endif
static uint32_t lld_bis_env_addr = 0;
static uint32_t lld_big_env_addr = 0;
static uint32_t lld_cis_env_addr = 0;
static uint32_t lld_cig_env_addr = 0;
static uint32_t con_latency_apply_addr = 0;
static uint32_t host_set_srand_seed_addr = 0;

static uint16_t off_rr_in_laet_ld_val = 0;
static uint16_t off_lsbo_in_laet_ld_val = 0;
static uint16_t off_tp_in_laet_ld_val = 0;
static uint16_t off_lsco_in_laet_ld_val = 0;
static uint16_t off_r1_in_laet_ld_val = 0;
static uint16_t off_arf_in_bubet_bt_val = 0;
static uint16_t off_atf_in_bubet_bt_val = 0;
static uint16_t off_atf_in_bubet_ble_val = 0;
static uint16_t off_mpl_in_dbssc_dbg_val = 0;
static uint16_t off_asiim_in_dbssc_dbg_val = 0;
static uint16_t off_aiiinm_in_dbssc_dbg_val = 0;
static uint16_t off_tl_in_dbcsc_dbg_val = 0;
static uint16_t off_li_in_lbbet_ld_val = 0;
static uint16_t off_ares_in_dbisc_dbg_val = 0;
static uint16_t off_issd_in_dbisc_dbg_val = 0;
static uint16_t off_li_in_lset_lc_val = 0;
static uint16_t off_mpt_in_lsnpt_lc_val = 0;
static uint16_t off_am_in_lsnpt_lc_val = 0;
#ifdef __NEW_SWAGC_MODE__
static uint16_t off_bsse_in_dbhfsc_dbg_val = 0;
#endif
static uint16_t off_lc_in_ret_rwip_val = 0;
static uint16_t off_h_in_ret_rwip_0_val = 0;
static uint16_t off_h_in_ret_rwip_1_val = 0;
static uint16_t off_lust_in_ret_rwip_val = 0;
static uint16_t off_gh_in_lce_lld_val = 0;
static uint16_t off_ah_in_lce_lld_0_val = 0;
static uint16_t off_ah_in_lce_lld_1_val = 0;
static uint16_t off_ii_in_lce_lld_val = 0;
static uint16_t off_gh_in_lbe_lld_val = 0;
static uint16_t off_ah_in_lbe_lld_0_val = 0;
static uint16_t off_ah_in_lbe_lld_1_val = 0;
static uint16_t off_ii_in_lbe_lld_val = 0;
static uint16_t off_tsdd_in_dbcst_dbg_t2_val = 0;
static uint16_t off_nsdi_in_dbcst_dbg_t2_val = 0;

void bt_drv_reg_op_global_symbols_init(void)
{
    bt_ram_start_addr = BT_RAM_BASE;
#ifndef __BT_RAMRUN_NEW__
    enum HAL_CHIP_METAL_ID_T metal_id = hal_get_chip_metal_id();
    if (metal_id < HAL_CHIP_METAL_ID_3)
    {
#ifdef __NEW_SWAGC_MODE__
        rf_rx_gain_ths_tbl_3m_addr = RF_RX_GAIN_THS_TBL_BT_3M_T0_ADDR;
        rx_monitor_3m_rxgain_addr = RX_MONITOR_3M_RXGAIN_T0_ADDR;
        off_bsse_in_dbhfsc_dbg_val = OFF_BSSE_IN_DBHFSC_DBG_T0;
#endif
        workmode_patch_version_addr = WORKMODE_PATCH_VERSION_1503_T0_ADDR;
        rf_rx_gain_ths_tbl_ecc_addr = RF_RX_GAIN_THS_TBL_ECC_T0_ADDR;
        hci_fc_env_addr = HCI_FC_ENV_T0_ADDR;
        ld_acl_env_addr = LD_ACL_ENV_T0_ADDR;
        bt_util_buf_env_addr = BT_UTIL_BUF_ENV_T0_ADDR;
        ble_util_buf_env_addr = BLE_UTIL_BUF_ENV_T0_ADDR;
        lc_state_addr = LC_STATE_T0_ADDR;
        ld_sco_env_addr = LD_SCO_ENV_T0_ADDR;
        rx_monitor_addr = RX_MONITOR_T0_ADDR;
        lm_env_addr = LM_ENV_T0_ADDR;
        lc_sco_env_addr = LC_SCO_ENV_T0_ADDR;
        llm_env_addr = LLM_ENV_T0_ADDR;
        rwip_env_addr = RWIP_ENV_T0_ADDR;
        ble_rx_monitor_addr = BLE_RX_MONITOR_T0_ADDR;
        ld_bes_bt_env_addr = LD_BES_BT_ENV_T0_ADDR;
        #ifdef __DEBUG_FIX_RX_GAIN_CNTL__
        rf_rx_gain_fixed_tbl_addr = RF_RX_GAIN_FIXED_TBL_T0_ADDR;
        #endif
        hci_dbg_ebq_test_mode_addr = HCI_DBG_EBQ_TEST_MODE_T0_ADDR;
        dbg_bt_common_setting_addr = DBG_BT_COMMON_SETTING_T0_ADDR;
        dbg_bt_sche_setting_addr = DBG_BT_SCHE_SETTING_T0_ADDR;
        dbg_bt_ibrt_setting_addr = DBG_BT_IBRT_SETTING_T0_ADDR;
        dbg_bt_hw_feat_setting_addr = DBG_BT_HW_FEAT_SETTING_T0_ADDR;
        hci_dbg_set_sw_rssi_addr = HCI_DBG_SET_SW_RSSI_T0_ADDR;
        data_backup_cnt_addr = DATA_BACKUP_CNT_T0_ADDR;
        data_backup_addr_ptr_addr = DATA_BACKUP_ADDR_PTR_T0_ADDR;
        data_backup_val_ptr_addr = DATA_BACKUP_VAL_PTR_T0_ADDR;
        sch_multi_ibrt_adjust_env_addr = SCH_MULTI_IBRT_ADJUST_ENV_T0_ADDR;
        rf_rx_gain_ths_tbl_le_addr = RF_RX_GAIN_THS_TBL_LE_T0_ADDR;
        llm_local_le_feats_addr = LLM_LOCAL_LE_FEATS_T0_ADDR;
        rt_sleep_flag_clear_addr = RT_SLEEP_FLAG_CLEAR_T0_ADDR;
        rf_rx_gain_ths_tbl_le_2m_addr = RF_RX_GAIN_THS_TBL_LE_2M_T0_ADDR;
        btdrv_txpwr_conv_tbl_addr = RF_RPL_TX_PW_CONV_TBL_T0_ADDR;
        pcm_need_start_flag_addr = PCM_NEED_START_FLAG_T0_ADDR;
        ecc_rx_monitor_addr = ECC_RX_MONITOR_T0_ADDR;
        btc_stack_limited_addr = __STACKLIMIT_T0_ADDR;
        lld_iso_env_addr = LLD_ISO_ENV_T0_ADDR;
        dbg_bt_common_setting_t2_addr = DBG_BT_COMMON_SETTING_T2_T0_ADDR;
        lld_bis_env_addr = LLD_BIS_ENV_T0_ADDR;
        lld_big_env_addr = LLD_BIG_ENV_T0_ADDR;
        lld_cis_env_addr = LLD_CIS_ENV_T0_ADDR;
        lld_cig_env_addr = LLD_CIG_ENV_T0_ADDR;
        con_latency_apply_addr = CON_LATENCY_APPLY_T0_ADDR;
        //BT ROM T0 struct offset
        off_rr_in_laet_ld_val = OFF_RR_IN_LAET_LD_T0;
        off_lsbo_in_laet_ld_val = OFF_LSBO_IN_LAET_LD_T0;
        off_tp_in_laet_ld_val = OFF_TP_IN_LAET_LD_T0;
        off_lsco_in_laet_ld_val = OFF_LSCO_IN_LAET_LD_T0;
        off_r1_in_laet_ld_val = OFF_R1_IN_LAET_LD_T0;
        off_arf_in_bubet_bt_val = OFF_ARF_IN_BUBET_BT_T0;
        off_atf_in_bubet_bt_val = OFF_ATF_IN_BUBET_BT_T0;
        off_atf_in_bubet_ble_val = OFF_ATF_IN_BUBET_BLE_T0;
        off_mpl_in_dbssc_dbg_val = OFF_MPL_IN_DBSSC_DBG_T0;
        off_asiim_in_dbssc_dbg_val = OFF_ASIIM_IN_DBSSC_DBG_T0;
        off_aiiinm_in_dbssc_dbg_val = OFF_AIIINM_IN_DBSSC_DBG_T0;
        off_tl_in_dbcsc_dbg_val = OFF_TL_IN_DBCSC_DBG_T0;
        off_li_in_lbbet_ld_val = OFF_LI_IN_LBBET_LD_T0;
        off_ares_in_dbisc_dbg_val = OFF_ARES_IN_DBISC_DBG_T0;
        off_issd_in_dbisc_dbg_val = OFF_ISSD_IN_DBISC_DBG_T0;
        off_li_in_lset_lc_val = OFF_LI_IN_LSET_LC_T0;
        off_mpt_in_lsnpt_lc_val = OFF_MPT_IN_LSNPT_LC_T0;
        off_am_in_lsnpt_lc_val = OFF_AM_IN_LSNPT_LC_T0;
        off_lc_in_ret_rwip_val = OFF_LC_IN_RET_RWIP_T0;
        off_h_in_ret_rwip_0_val = OFF_H_IN_RET_RWIP_0_T0;
        off_h_in_ret_rwip_1_val = OFF_H_IN_RET_RWIP_1_T0;
        off_lust_in_ret_rwip_val = OFF_LUST_IN_RET_RWIP_T0;
        off_gh_in_lce_lld_val = OFF_GH_IN_LCE_LLD_T0;
        off_ah_in_lce_lld_0_val = OFF_AH_IN_LCE_LLD_0_T0;
        off_ah_in_lce_lld_1_val = OFF_AH_IN_LCE_LLD_1_T0;
        off_ii_in_lce_lld_val = OFF_II_IN_LCE_LLD_T0;
        off_gh_in_lbe_lld_val = OFF_GH_IN_LBE_LLD_T0;
        off_ah_in_lbe_lld_0_val = OFF_AH_IN_LBE_LLD_0_T0;
        off_ah_in_lbe_lld_1_val = OFF_AH_IN_LBE_LLD_1_T0;
        off_ii_in_lbe_lld_val = OFF_II_IN_LBE_LLD_T0;
    }
    else if ((metal_id >= HAL_CHIP_METAL_ID_3) && (metal_id < HAL_CHIP_METAL_ID_6))
    {
#ifdef __NEW_SWAGC_MODE__
        rf_rx_gain_ths_tbl_3m_addr = RF_RX_GAIN_THS_TBL_BT_3M_T1_ADDR;
        rx_monitor_3m_rxgain_addr = RX_MONITOR_3M_RXGAIN_T1_ADDR;
        off_bsse_in_dbhfsc_dbg_val = OFF_BSSE_IN_DBHFSC_DBG_T1;
#endif
        workmode_patch_version_addr = WORKMODE_PATCH_VERSION_1503_T1_ADDR;
        rf_rx_gain_ths_tbl_ecc_addr = RF_RX_GAIN_THS_TBL_ECC_T1_ADDR;
        hci_fc_env_addr = HCI_FC_ENV_T1_ADDR;
        ld_acl_env_addr = LD_ACL_ENV_T1_ADDR;
        bt_util_buf_env_addr = BT_UTIL_BUF_ENV_T1_ADDR;
        ble_util_buf_env_addr = BLE_UTIL_BUF_ENV_T1_ADDR;
        lc_state_addr = LC_STATE_T1_ADDR;
        ld_sco_env_addr = LD_SCO_ENV_T1_ADDR;
        rx_monitor_addr = RX_MONITOR_T1_ADDR;
        lm_env_addr = LM_ENV_T1_ADDR;
        lc_sco_env_addr = LC_SCO_ENV_T1_ADDR;
        llm_env_addr = LLM_ENV_T1_ADDR;
        rwip_env_addr = RWIP_ENV_T1_ADDR;
        ble_rx_monitor_addr = BLE_RX_MONITOR_T1_ADDR;
        ld_bes_bt_env_addr = LD_BES_BT_ENV_T1_ADDR;
        #ifdef __DEBUG_FIX_RX_GAIN_CNTL__
        rf_rx_gain_fixed_tbl_addr = RF_RX_GAIN_FIXED_TBL_T1_ADDR;
        #endif
        hci_dbg_ebq_test_mode_addr = HCI_DBG_EBQ_TEST_MODE_T1_ADDR;
        dbg_bt_common_setting_addr = DBG_BT_COMMON_SETTING_T1_ADDR;
        dbg_bt_sche_setting_addr = DBG_BT_SCHE_SETTING_T1_ADDR;
        dbg_bt_ibrt_setting_addr = DBG_BT_IBRT_SETTING_T1_ADDR;
        dbg_bt_hw_feat_setting_addr = DBG_BT_HW_FEAT_SETTING_T1_ADDR;
        hci_dbg_set_sw_rssi_addr = HCI_DBG_SET_SW_RSSI_T1_ADDR;
        data_backup_cnt_addr = DATA_BACKUP_CNT_T1_ADDR;
        data_backup_addr_ptr_addr = DATA_BACKUP_ADDR_PTR_T1_ADDR;
        data_backup_val_ptr_addr = DATA_BACKUP_VAL_PTR_T1_ADDR;
        sch_multi_ibrt_adjust_env_addr = SCH_MULTI_IBRT_ADJUST_ENV_T1_ADDR;
        rf_rx_gain_ths_tbl_le_addr = RF_RX_GAIN_THS_TBL_LE_T1_ADDR;
        llm_local_le_feats_addr = LLM_LOCAL_LE_FEATS_T1_ADDR;
        rt_sleep_flag_clear_addr = RT_SLEEP_FLAG_CLEAR_T1_ADDR;
        rf_rx_gain_ths_tbl_le_2m_addr = RF_RX_GAIN_THS_TBL_LE_2M_T1_ADDR;
        btdrv_txpwr_conv_tbl_addr = RF_RPL_TX_PW_CONV_TBL_T1_ADDR;
        pcm_need_start_flag_addr = PCM_NEED_START_FLAG_T1_ADDR;
        ecc_rx_monitor_addr = ECC_RX_MONITOR_T1_ADDR;
        btc_stack_limited_addr = __STACKLIMIT_T1_ADDR;
        lld_iso_env_addr = LLD_ISO_ENV_T1_ADDR;
        dbg_bt_common_setting_t2_addr = DBG_BT_COMMON_SETTING_T2_T1_ADDR;
        lld_bis_env_addr = LLD_BIS_ENV_T1_ADDR;
        lld_big_env_addr = LLD_BIG_ENV_T1_ADDR;
        lld_cis_env_addr = LLD_CIS_ENV_T1_ADDR;
        lld_cig_env_addr = LLD_CIG_ENV_T1_ADDR;
        con_latency_apply_addr = CON_LATENCY_APPLY_T1_ADDR;
        //BT ROM T0 struct offset
        off_rr_in_laet_ld_val = OFF_RR_IN_LAET_LD_T1;
        off_lsbo_in_laet_ld_val = OFF_LSBO_IN_LAET_LD_T1;
        off_tp_in_laet_ld_val = OFF_TP_IN_LAET_LD_T1;
        off_lsco_in_laet_ld_val = OFF_LSCO_IN_LAET_LD_T1;
        off_r1_in_laet_ld_val = OFF_R1_IN_LAET_LD_T1;
        off_arf_in_bubet_bt_val = OFF_ARF_IN_BUBET_BT_T1;
        off_atf_in_bubet_bt_val = OFF_ATF_IN_BUBET_BT_T1;
        off_atf_in_bubet_ble_val = OFF_ATF_IN_BUBET_BLE_T1;
        off_mpl_in_dbssc_dbg_val = OFF_MPL_IN_DBSSC_DBG_T1;
        off_asiim_in_dbssc_dbg_val = OFF_ASIIM_IN_DBSSC_DBG_T1;
        off_aiiinm_in_dbssc_dbg_val = OFF_AIIINM_IN_DBSSC_DBG_T1;
        off_tl_in_dbcsc_dbg_val = OFF_TL_IN_DBCSC_DBG_T1;
        off_li_in_lbbet_ld_val = OFF_LI_IN_LBBET_LD_T1;
        off_ares_in_dbisc_dbg_val = OFF_ARES_IN_DBISC_DBG_T1;
        off_issd_in_dbisc_dbg_val = OFF_ISSD_IN_DBISC_DBG_T1;
        off_li_in_lset_lc_val = OFF_LI_IN_LSET_LC_T1;
        off_mpt_in_lsnpt_lc_val = OFF_MPT_IN_LSNPT_LC_T1;
        off_am_in_lsnpt_lc_val = OFF_AM_IN_LSNPT_LC_T1;
        off_lc_in_ret_rwip_val = OFF_LC_IN_RET_RWIP_T1;
        off_h_in_ret_rwip_0_val = OFF_H_IN_RET_RWIP_0_T1;
        off_h_in_ret_rwip_1_val = OFF_H_IN_RET_RWIP_1_T1;
        off_lust_in_ret_rwip_val = OFF_LUST_IN_RET_RWIP_T1;
        off_gh_in_lce_lld_val = OFF_GH_IN_LCE_LLD_T1;
        off_ah_in_lce_lld_0_val = OFF_AH_IN_LCE_LLD_0_T1;
        off_ah_in_lce_lld_1_val = OFF_AH_IN_LCE_LLD_1_T1;
        off_ii_in_lce_lld_val = OFF_II_IN_LCE_LLD_T1;
        off_gh_in_lbe_lld_val = OFF_GH_IN_LBE_LLD_T1;
        off_ah_in_lbe_lld_0_val = OFF_AH_IN_LBE_LLD_0_T1;
        off_ah_in_lbe_lld_1_val = OFF_AH_IN_LBE_LLD_1_T1;
        off_ii_in_lbe_lld_val = OFF_II_IN_LBE_LLD_T1;
    }
    else if (metal_id == HAL_CHIP_METAL_ID_6)
    {
#ifdef __NEW_SWAGC_MODE__
        rf_rx_gain_ths_tbl_3m_addr = RF_RX_GAIN_THS_TBL_BT_3M_T2_ADDR;
        rx_monitor_3m_rxgain_addr = RX_MONITOR_3M_RXGAIN_T2_ADDR;
        off_bsse_in_dbhfsc_dbg_val = OFF_BSSE_IN_DBHFSC_DBG_T2;
#endif
        //workmode_patch_version_addr = WORKMODE_PATCH_VERSION_1503_T2_ADDR;
        rf_rx_gain_ths_tbl_ecc_addr = RF_RX_GAIN_THS_TBL_ECC_T2_ADDR;
        hci_fc_env_addr = HCI_FC_ENV_T2_ADDR;
        ld_acl_env_addr = LD_ACL_ENV_T2_ADDR;
        bt_util_buf_env_addr = BT_UTIL_BUF_ENV_T2_ADDR;
        ble_util_buf_env_addr = BLE_UTIL_BUF_ENV_T2_ADDR;
        lc_state_addr = LC_STATE_T2_ADDR;
        ld_sco_env_addr = LD_SCO_ENV_T2_ADDR;
        rx_monitor_addr = RX_MONITOR_T2_ADDR;
        lm_env_addr = LM_ENV_T2_ADDR;
        lc_sco_env_addr = LC_SCO_ENV_T2_ADDR;
        llm_env_addr = LLM_ENV_T2_ADDR;
        rwip_env_addr = RWIP_ENV_T2_ADDR;
        ble_rx_monitor_addr = BLE_RX_MONITOR_T2_ADDR;
        ld_bes_bt_env_addr = LD_BES_BT_ENV_T2_ADDR;
        #ifdef __DEBUG_FIX_RX_GAIN_CNTL__
        rf_rx_gain_fixed_tbl_addr = RF_RX_GAIN_FIXED_TBL_T2_ADDR;
        #endif
        hci_dbg_ebq_test_mode_addr = HCI_DBG_EBQ_TEST_MODE_T2_ADDR;
        dbg_bt_common_setting_addr = DBG_BT_COMMON_SETTING_T2_ADDR;
        dbg_bt_sche_setting_addr = DBG_BT_SCHE_SETTING_T2_ADDR;
        dbg_bt_ibrt_setting_addr = DBG_BT_IBRT_SETTING_T2_ADDR;
        dbg_bt_hw_feat_setting_addr = DBG_BT_HW_FEAT_SETTING_T2_ADDR;
        hci_dbg_set_sw_rssi_addr = HCI_DBG_SET_SW_RSSI_T2_ADDR;
        data_backup_cnt_addr = DATA_BACKUP_CNT_T2_ADDR;
        data_backup_addr_ptr_addr = DATA_BACKUP_ADDR_PTR_T2_ADDR;
        data_backup_val_ptr_addr = DATA_BACKUP_VAL_PTR_T2_ADDR;
        sch_multi_ibrt_adjust_env_addr = SCH_MULTI_IBRT_ADJUST_ENV_T2_ADDR;
        rf_rx_gain_ths_tbl_le_addr = RF_RX_GAIN_THS_TBL_LE_T2_ADDR;
        llm_local_le_feats_addr = LLM_LOCAL_LE_FEATS_T2_ADDR;
        rt_sleep_flag_clear_addr = RT_SLEEP_FLAG_CLEAR_T2_ADDR;
        rf_rx_gain_ths_tbl_le_2m_addr = RF_RX_GAIN_THS_TBL_LE_2M_T2_ADDR;
        btdrv_txpwr_conv_tbl_addr = RF_RPL_TX_PW_CONV_TBL_T2_ADDR;
        pcm_need_start_flag_addr = PCM_NEED_START_FLAG_T2_ADDR;
        ecc_rx_monitor_addr = ECC_RX_MONITOR_T2_ADDR;
        btc_stack_limited_addr = __STACKLIMIT_T2_ADDR;
        lld_iso_env_addr = LLD_ISO_ENV_T2_ADDR;
        dbg_bt_common_setting_t2_addr = DBG_BT_COMMON_SETTING_T2_T2_ADDR;
        lld_bis_env_addr = LLD_BIS_ENV_T2_ADDR;
        lld_big_env_addr = LLD_BIG_ENV_T2_ADDR;
        lld_cis_env_addr = LLD_CIS_ENV_T2_ADDR;
        lld_cig_env_addr = LLD_CIG_ENV_T2_ADDR;
        con_latency_apply_addr = CON_LATENCY_APPLY_T2_ADDR;
        //BT ROM T0 struct offset
        off_rr_in_laet_ld_val = OFF_RR_IN_LAET_LD_T2;
        off_lsbo_in_laet_ld_val = OFF_LSBO_IN_LAET_LD_T2;
        off_tp_in_laet_ld_val = OFF_TP_IN_LAET_LD_T2;
        off_lsco_in_laet_ld_val = OFF_LSCO_IN_LAET_LD_T2;
        off_r1_in_laet_ld_val = OFF_R1_IN_LAET_LD_T2;
        off_arf_in_bubet_bt_val = OFF_ARF_IN_BUBET_BT_T2;
        off_atf_in_bubet_bt_val = OFF_ATF_IN_BUBET_BT_T2;
        off_atf_in_bubet_ble_val = OFF_ATF_IN_BUBET_BLE_T2;
        off_mpl_in_dbssc_dbg_val = OFF_MPL_IN_DBSSC_DBG_T2;
        off_asiim_in_dbssc_dbg_val = OFF_ASIIM_IN_DBSSC_DBG_T2;
        off_aiiinm_in_dbssc_dbg_val = OFF_AIIINM_IN_DBSSC_DBG_T2;
        off_tl_in_dbcsc_dbg_val = OFF_TL_IN_DBCSC_DBG_T2;
        off_li_in_lbbet_ld_val = OFF_LI_IN_LBBET_LD_T2;
        off_ares_in_dbisc_dbg_val = OFF_ARES_IN_DBISC_DBG_T2;
        off_issd_in_dbisc_dbg_val = OFF_ISSD_IN_DBISC_DBG_T2;
        off_li_in_lset_lc_val = OFF_LI_IN_LSET_LC_T2;
        off_mpt_in_lsnpt_lc_val = OFF_MPT_IN_LSNPT_LC_T2;
        off_am_in_lsnpt_lc_val = OFF_AM_IN_LSNPT_LC_T2;
        off_lc_in_ret_rwip_val = OFF_LC_IN_RET_RWIP_T2;
        off_h_in_ret_rwip_0_val = OFF_H_IN_RET_RWIP_0_T2;
        off_h_in_ret_rwip_1_val = OFF_H_IN_RET_RWIP_1_T2;
        off_lust_in_ret_rwip_val = OFF_LUST_IN_RET_RWIP_T2;
        off_gh_in_lce_lld_val = OFF_GH_IN_LCE_LLD_T2;
        off_ah_in_lce_lld_0_val = OFF_AH_IN_LCE_LLD_0_T2;
        off_ah_in_lce_lld_1_val = OFF_AH_IN_LCE_LLD_1_T2;
        off_ii_in_lce_lld_val = OFF_II_IN_LCE_LLD_T2;
        off_gh_in_lbe_lld_val = OFF_GH_IN_LBE_LLD_T2;
        off_ah_in_lbe_lld_0_val = OFF_AH_IN_LBE_LLD_0_T2;
        off_ah_in_lbe_lld_1_val = OFF_AH_IN_LBE_LLD_1_T2;
        off_ii_in_lbe_lld_val = OFF_II_IN_LBE_LLD_T2;
        off_tsdd_in_dbcst_dbg_t2_val = OFF_TSDD_IN_DBCST_DBG_T2;
        off_nsdi_in_dbcst_dbg_t2_val = OFF_NSDI_IN_DBCST_DBG_T2;
        host_set_srand_seed_addr = HOST_SET_SRAND_SEED_1503_T2_ADDR;
    }
    else if (metal_id >= HAL_CHIP_METAL_ID_7)
    {
#ifdef __NEW_SWAGC_MODE__
        rf_rx_gain_ths_tbl_3m_addr = RF_RX_GAIN_THS_TBL_BT_3M_T3_ADDR;
        rx_monitor_3m_rxgain_addr = RX_MONITOR_3M_RXGAIN_T3_ADDR;
        off_bsse_in_dbhfsc_dbg_val = OFF_BSSE_IN_DBHFSC_DBG_T3;
#endif
        //workmode_patch_version_addr = WORKMODE_PATCH_VERSION_1503_T3_ADDR;
        rf_rx_gain_ths_tbl_ecc_addr = RF_RX_GAIN_THS_TBL_ECC_T3_ADDR;
        hci_fc_env_addr = HCI_FC_ENV_T3_ADDR;
        ld_acl_env_addr = LD_ACL_ENV_T3_ADDR;
        bt_util_buf_env_addr = BT_UTIL_BUF_ENV_T3_ADDR;
        ble_util_buf_env_addr = BLE_UTIL_BUF_ENV_T3_ADDR;
        lc_state_addr = LC_STATE_T3_ADDR;
        ld_sco_env_addr = LD_SCO_ENV_T3_ADDR;
        rx_monitor_addr = RX_MONITOR_T3_ADDR;
        lm_env_addr = LM_ENV_T3_ADDR;
        lc_sco_env_addr = LC_SCO_ENV_T3_ADDR;
        llm_env_addr = LLM_ENV_T3_ADDR;
        rwip_env_addr = RWIP_ENV_T3_ADDR;
        ble_rx_monitor_addr = BLE_RX_MONITOR_T3_ADDR;
        ld_bes_bt_env_addr = LD_BES_BT_ENV_T3_ADDR;
        #ifdef __DEBUG_FIX_RX_GAIN_CNTL__
        rf_rx_gain_fixed_tbl_addr = RF_RX_GAIN_FIXED_TBL_T3_ADDR;
        #endif
        hci_dbg_ebq_test_mode_addr = HCI_DBG_EBQ_TEST_MODE_T3_ADDR;
        dbg_bt_common_setting_addr = DBG_BT_COMMON_SETTING_T3_ADDR;
        dbg_bt_sche_setting_addr = DBG_BT_SCHE_SETTING_T3_ADDR;
        dbg_bt_ibrt_setting_addr = DBG_BT_IBRT_SETTING_T3_ADDR;
        dbg_bt_hw_feat_setting_addr = DBG_BT_HW_FEAT_SETTING_T3_ADDR;
        hci_dbg_set_sw_rssi_addr = HCI_DBG_SET_SW_RSSI_T3_ADDR;
        sch_multi_ibrt_adjust_env_addr = SCH_MULTI_IBRT_ADJUST_ENV_T3_ADDR;
        rf_rx_gain_ths_tbl_le_addr = RF_RX_GAIN_THS_TBL_LE_T3_ADDR;
        llm_local_le_feats_addr = LLM_LOCAL_LE_FEATS_T3_ADDR;
        rf_rx_gain_ths_tbl_le_2m_addr = RF_RX_GAIN_THS_TBL_LE_2M_T3_ADDR;
        btdrv_txpwr_conv_tbl_addr = RF_RPL_TX_PW_CONV_TBL_T3_ADDR;
        pcm_need_start_flag_addr = PCM_NEED_START_FLAG_T3_ADDR;
        ecc_rx_monitor_addr = ECC_RX_MONITOR_T3_ADDR;
        btc_stack_limited_addr = __STACKLIMIT_T3_ADDR;
        lld_iso_env_addr = LLD_ISO_ENV_T3_ADDR;
        dbg_bt_common_setting_t2_addr = DBG_BT_COMMON_SETTING_T2_T3_ADDR;
        lld_bis_env_addr = LLD_BIS_ENV_T3_ADDR;
        lld_big_env_addr = LLD_BIG_ENV_T3_ADDR;
        lld_cis_env_addr = LLD_CIS_ENV_T3_ADDR;
        lld_cig_env_addr = LLD_CIG_ENV_T3_ADDR;
        //BT ROM T0 struct offset
        off_rr_in_laet_ld_val = OFF_RR_IN_LAET_LD_T3;
        off_lsbo_in_laet_ld_val = OFF_LSBO_IN_LAET_LD_T3;
        off_tp_in_laet_ld_val = OFF_TP_IN_LAET_LD_T3;
        off_lsco_in_laet_ld_val = OFF_LSCO_IN_LAET_LD_T3;
        off_r1_in_laet_ld_val = OFF_R1_IN_LAET_LD_T3;
        off_arf_in_bubet_bt_val = OFF_ARF_IN_BUBET_BT_T3;
        off_atf_in_bubet_bt_val = OFF_ATF_IN_BUBET_BT_T3;
        off_atf_in_bubet_ble_val = OFF_ATF_IN_BUBET_BLE_T3;
        off_mpl_in_dbssc_dbg_val = OFF_MPL_IN_DBSSC_DBG_T3;
        off_asiim_in_dbssc_dbg_val = OFF_ASIIM_IN_DBSSC_DBG_T3;
        off_aiiinm_in_dbssc_dbg_val = OFF_AIIINM_IN_DBSSC_DBG_T3;
        off_tl_in_dbcsc_dbg_val = OFF_TL_IN_DBCSC_DBG_T3;
        off_li_in_lbbet_ld_val = OFF_LI_IN_LBBET_LD_T3;
        off_ares_in_dbisc_dbg_val = OFF_ARES_IN_DBISC_DBG_T3;
        off_issd_in_dbisc_dbg_val = OFF_ISSD_IN_DBISC_DBG_T3;
        off_li_in_lset_lc_val = OFF_LI_IN_LSET_LC_T3;
        off_mpt_in_lsnpt_lc_val = OFF_MPT_IN_LSNPT_LC_T3;
        off_am_in_lsnpt_lc_val = OFF_AM_IN_LSNPT_LC_T3;
        off_lc_in_ret_rwip_val = OFF_LC_IN_RET_RWIP_T3;
        off_h_in_ret_rwip_0_val = OFF_H_IN_RET_RWIP_0_T3;
        off_h_in_ret_rwip_1_val = OFF_H_IN_RET_RWIP_1_T3;
        off_lust_in_ret_rwip_val = OFF_LUST_IN_RET_RWIP_T3;
        off_gh_in_lce_lld_val = OFF_GH_IN_LCE_LLD_T3;
        off_ah_in_lce_lld_0_val = OFF_H_IN_LCE_LLD_0_T3;
        off_ah_in_lce_lld_1_val = OFF_H_IN_LCE_LLD_1_T3;
        off_ii_in_lce_lld_val = OFF_II_IN_LCE_LLD_T3;
        off_gh_in_lbe_lld_val = OFF_GH_IN_LBE_LLD_T3;
        off_ah_in_lbe_lld_0_val = OFF_H_IN_LBE_LLD_0_T3;
        off_ah_in_lbe_lld_1_val = OFF_H_IN_LBE_LLD_1_T3;
        off_ii_in_lbe_lld_val = OFF_II_IN_LBE_LLD_T3;
        off_tsdd_in_dbcst_dbg_t2_val = OFF_TSDD_IN_DBCST_DBG_T3;
        off_nsdi_in_dbcst_dbg_t2_val = OFF_NSDI_IN_DBCST_DBG_T3;
        host_set_srand_seed_addr = HOST_SET_SRAND_SEED_1503_T3_ADDR;
    }
#else
#ifdef __NEW_SWAGC_MODE__
    rf_rx_gain_ths_tbl_3m_addr = RF_RX_GAIN_THS_TBL_BT_3M_ADDR;
    rx_monitor_3m_rxgain_addr = RX_MONITOR_3M_RXGAIN_ADDR;
    off_bsse_in_dbhfsc_dbg_val = OFF_BSSE_IN_DBHFSC_DBG;
#endif
    hci_fc_env_addr = HCI_FC_ENV_ADDR;
    ld_acl_env_addr = LD_ACL_ENV_ADDR;
    bt_util_buf_env_addr = BT_UTIL_BUF_ENV_ADDR;
    ble_util_buf_env_addr = BLE_UTIL_BUF_ENV_ADDR;
    lc_state_addr = LC_STATE_ADDR;
    ld_sco_env_addr = LD_SCO_ENV_ADDR;
    rx_monitor_addr = RX_MONITOR_ADDR;
    lm_env_addr = LM_ENV_ADDR;
    lc_sco_env_addr = LC_SCO_ENV_ADDR;
    llm_env_addr = LLM_ENV_ADDR;
    rwip_env_addr = RWIP_ENV_ADDR;
    ble_rx_monitor_addr = BLE_RX_MONITOR_ADDR;
    ld_bes_bt_env_addr = LD_BES_BT_ENV_ADDR;
    #ifdef __DEBUG_FIX_RX_GAIN_CNTL__
    rf_rx_gain_fixed_tbl_addr = RF_RX_GAIN_FIXED_TBL_ADDR;
    #endif
    hci_dbg_ebq_test_mode_addr = HCI_DBG_EBQ_TEST_MODE_ADDR;
    dbg_bt_common_setting_addr = DBG_BT_COMMON_SETTING_ADDR;
    dbg_bt_sche_setting_addr = DBG_BT_SCHE_SETTING_ADDR;
    dbg_bt_ibrt_setting_addr = DBG_BT_IBRT_SETTING_ADDR;
    dbg_bt_hw_feat_setting_addr = DBG_BT_HW_FEAT_SETTING_ADDR;
    hci_dbg_set_sw_rssi_addr = HCI_DBG_SET_SW_RSSI_ADDR;
    data_backup_cnt_addr = DATA_BACKUP_CNT_ADDR;
    data_backup_addr_ptr_addr = DATA_BACKUP_ADDR_PTR_ADDR;
    data_backup_val_ptr_addr = DATA_BACKUP_VAL_PTR_ADDR;
    sch_multi_ibrt_adjust_env_addr = SCH_MULTI_IBRT_ADJUST_ENV_ADDR;
    rf_rx_gain_ths_tbl_le_addr = RF_RX_GAIN_THS_TBL_LE_ADDR;
    llm_local_le_feats_addr = LLM_LOCAL_LE_FEATS_ADDR;
    rf_rx_gain_ths_tbl_le_2m_addr = RF_RX_GAIN_THS_TBL_LE_2M_ADDR;
    btdrv_txpwr_conv_tbl_addr = RF_RPL_TX_PW_CONV_TBL_ADDR;
    btdrv_ble_txpwr_conv_tbl_addr = RF_BLE_TX_PW_CONV_TBL_ADDR;
    pcm_need_start_flag_addr = PCM_NEED_START_FLAG_ADDR;
    ecc_rx_monitor_addr = ECC_RX_MONITOR_ADDR;
    btc_stack_limited_addr = __STACKLIMIT_ADDR;
    lld_iso_env_addr = LLD_ISO_ENV_ADDR;
    dbg_bt_common_setting_t2_addr = DBG_BT_COMMON_SETTING_T2_ADDR;
    rt_sleep_flag_clear_addr = RT_SLEEP_FLAG_CLEAR_ADDR;
    rf_rx_gain_ths_tbl_ecc_addr = RF_RX_GAIN_THS_TBL_ECC_ADDR;
    rx_record_addr = RX_RECORD_ADDR;
    lld_bis_env_addr = LLD_BIS_ENV_ADDR;
    lld_big_env_addr = LLD_BIG_ENV_ADDR;
    lld_cis_env_addr = LLD_CIS_ENV_ADDR;
    lld_cig_env_addr = LLD_CIG_ENV_ADDR;
    //BT RAMRUN struct offset init
    off_rr_in_laet_ld_val = OFF_RR_IN_LAET_LD;
    off_lsbo_in_laet_ld_val = OFF_LSBO_IN_LAET_LD;
    off_tp_in_laet_ld_val = OFF_TP_IN_LAET_LD;
    off_lsco_in_laet_ld_val = OFF_LSCO_IN_LAET_LD;
    off_r1_in_laet_ld_val = OFF_R1_IN_LAET_LD;
    off_arf_in_bubet_bt_val = OFF_ARF_IN_BUBET_BT;
    off_atf_in_bubet_bt_val = OFF_ATF_IN_BUBET_BT;
    off_atf_in_bubet_ble_val = OFF_ATF_IN_BUBET_BLE;
    off_mpl_in_dbssc_dbg_val = OFF_MPL_IN_DBSSC_DBG;
    off_asiim_in_dbssc_dbg_val = OFF_ASIIM_IN_DBSSC_DBG;
    off_aiiinm_in_dbssc_dbg_val = OFF_AIIINM_IN_DBSSC_DBG;
    off_tl_in_dbcsc_dbg_val = OFF_TL_IN_DBCSC_DBG;
    off_li_in_lbbet_ld_val = OFF_LI_IN_LBBET_LD;
    off_ares_in_dbisc_dbg_val = OFF_ARES_IN_DBISC_DBG;
    off_issd_in_dbisc_dbg_val = OFF_ISSD_IN_DBISC_DBG;
    off_li_in_lset_lc_val = OFF_LI_IN_LSET_LC;
    off_mpt_in_lsnpt_lc_val = OFF_MPT_IN_LSNPT_LC;
    off_am_in_lsnpt_lc_val = OFF_AM_IN_LSNPT_LC;
    off_lc_in_ret_rwip_val = OFF_LC_IN_RET_RWIP;
    off_h_in_ret_rwip_0_val = OFF_H_IN_RET_RWIP_0;
    off_h_in_ret_rwip_1_val = OFF_H_IN_RET_RWIP_1;
    off_lust_in_ret_rwip_val = OFF_LUST_IN_RET_RWIP;
    off_gh_in_lce_lld_val = OFF_GH_IN_LCE_LLD;
    off_ah_in_lce_lld_0_val = OFF_AH_IN_LCE_LLD_0;
    off_ah_in_lce_lld_1_val = OFF_AH_IN_LCE_LLD_1;
    off_ii_in_lce_lld_val = OFF_II_IN_LCE_LLD;
    off_gh_in_lbe_lld_val = OFF_GH_IN_LBE_LLD;
    off_ah_in_lbe_lld_0_val = OFF_AH_IN_LBE_LLD_0;
    off_ah_in_lbe_lld_1_val = OFF_AH_IN_LBE_LLD_1;
    off_ii_in_lbe_lld_val = OFF_II_IN_LBE_LLD;
    off_tsdd_in_dbcst_dbg_t2_val = OFF_TSDD_IN_DBCST_DBG;
    off_nsdi_in_dbcst_dbg_t2_val = OFF_NSDI_IN_DBCST_DBG;
#endif
}

void bt_drv_reg_op_hci_vender_ibrt_ll_monitor(uint8_t* ptr, uint16_t* p_sum_err,uint16_t* p_rx_total)
{
    const char * const monitor_str[METRIC_TYPE_MAX] =
    {
        "TX DM1",
        "TX DH1",
        "TX DM3",
        "TX DH3",
        "TX DM5",
        "TX DH5",
        "TX 2DH1",
        "TX 3DH1",
        "TX 2DH3",
        "TX 3DH3",
        "TX 2DH5",
        "TX 3DH5",  //11

        "RX DM1",
        "RX DH1",
        "RX DM3",
        "RX DH3",
        "RX DM5",
        "RX DH5",
        "RX 2DH1",
        "RX 3DH1",
        "RX 2DH3",
        "RX 3DH3",
        "RX 2DH5",
        "RX 3DH5",  //23
        "hec err",
        "crc err",
        "fec err",
        "gard err",
        "ecc ok",  //28

        "rf_cnt",
        "sco_err_rxtype",
        "tx_suc_cnt",
        "tx_cnt",
        "sco_pld_err",
        "buff err",
        "rx_seq_err", //35
        "fa_no_sync",
        "RX 2EV3",
        "sco_no_sync",
    };

    uint32_t *p = ( uint32_t * )ptr;
    uint32_t sum_err = 0;
    uint32_t rx_tx_data_sum = 0;

    //BT_DRV_TRACE(0,"ibrt_ui_log:ll_monitor");

    for (uint8_t i = 0; i < METRIC_TYPE_MAX; i++)
    {
        if (*p)
        {
            if((i>= 0 && i<=29)||(i==35)||(i==37))
            {
                rx_tx_data_sum += *p;
            }

            if((i > 23) && (i < 29))
            {
                sum_err += *p;
            }
            BT_DRV_TRACE(2,"%s %d", monitor_str[i], *p);
        }
        p++;
    }
    *p_sum_err = sum_err;
    *p_rx_total = rx_tx_data_sum;
}

int bt_drv_reg_op_currentfreeaclbuf_get(void)
{
    int nRet = 0;

#ifdef BT_INFO_CHECKER
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    //ACL packert number of host set - ACL number of controller has been send to host
    //hci_fc_env.host_set.acl_pkt_nb - hci_fc_env.cntr.acl_pkt_sent
    if(hci_fc_env_addr != 0)
    {
        nRet = (*(volatile uint16_t *)(hci_fc_env_addr+0x2) - *(volatile uint16_t *)(hci_fc_env_addr+0xa));//acl_pkt_nb - acl_pkt_sent
    }
    else
    {
        nRet = 0;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
#endif
    return nRet;
}


int bt_drv_reg_op_acl_chnmap(uint16_t hciHandle, uint8_t *chnmap, uint8_t chnmap_len)
{
    return -1;
}

void bt_drv_reg_op_bt_info_checker(void)
{
#ifdef BT_INFO_CHECKER
    uint32_t *rx_buf_ptr=NULL;
    uint32_t *tx_buf_ptr=NULL;
    uint8_t rx_free_buf_count=0;
    uint8_t tx_free_buf_count=0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(bt_util_buf_env_addr)
    {
        //acl_rx_free off:0x14
        //acl_tx_free off:0x28
        rx_buf_ptr = (uint32_t *)(bt_util_buf_env_addr+off_arf_in_bubet_bt_val); //bt_util_buf_env.acl_rx_free
        tx_buf_ptr = (uint32_t *)(bt_util_buf_env_addr+off_atf_in_bubet_bt_val); //bt_util_buf_env.acl_tx_free
    }

    while(rx_buf_ptr && *rx_buf_ptr)
    {
        rx_free_buf_count++;
        rx_buf_ptr = (uint32_t *)(*rx_buf_ptr);
    }

    BT_DRV_TRACE(3,"BT_REG_OP:acl_rx_free ctrl rxbuff %d, host free %d", \
                 rx_free_buf_count, \
                 bt_drv_reg_op_currentfreeaclbuf_get());

    //check tx buff
    while(tx_buf_ptr && *tx_buf_ptr)
    {
        tx_free_buf_count++;
        tx_buf_ptr = (uint32_t *)(*tx_buf_ptr);
    }

    BT_DRV_TRACE(2,"BT_REG_OP:acl_tx_free ctrl txbuff %d", \
                 tx_free_buf_count);

    if(lc_state_addr != 0)
    {
        BT_DRV_TRACE(1,"BT_REG_OP:LC_STATE=0x%x",*(uint32_t *)lc_state_addr);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
#endif
}

uint8_t bt_drv_reg_op_get_controller_tx_free_buffer(void)
{
    uint32_t *tx_buf_ptr=NULL;
    uint8_t tx_free_buf_count = 0;

    BT_DRV_REG_OP_CLK_ENB();

    if(bt_util_buf_env_addr)
    {
        //acl_tx_free off:0x28
        tx_buf_ptr = (uint32_t *)(bt_util_buf_env_addr+off_atf_in_bubet_bt_val); //bt_util_buf_env.acl_tx_free
    }
    else
    {
        BT_DRV_TRACE(1, "BT_REG_OP:please fix %s", __func__);
        tx_free_buf_count = 0;
        goto exit;
    }

    //check tx buff
    while(tx_buf_ptr && *tx_buf_ptr)
    {
        tx_free_buf_count++;
        tx_buf_ptr = (uint32_t *)(*tx_buf_ptr);
    }

exit:
    BT_DRV_REG_OP_CLK_DIB();

    return tx_free_buf_count;
}

void bt_drv_reg_op_controller_state_checker(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(lc_state_addr != 0)
    {
        BT_DRV_TRACE(1,"BT_REG_OP: LC_STATE=0x%x",*(uint32_t *)lc_state_addr);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


void bt_drv_reg_op_crash_dump(void)
{
    BT_DRV_REG_OP_CLK_TRY_ENB();

    uint8_t *bt_dump_mem_start = (uint8_t*)bt_ram_start_addr;
    uint32_t bt_dump_len_max = BT_SRAM_SIZE;

    uint8_t *em_dump_area_2_start = (uint8_t*)EM_BASE_ADDR;
    uint32_t em_area_2_len_max = EM_SIZE;
    uint8_t metal_id = hal_get_chip_metal_id();
    BT_DRV_TRACE(1,"BTC 1503: metal id=%d", metal_id);

    #ifdef __BT_RAMRUN_NEW__
    BT_DRV_TRACE(1,"BT ramrun %s", BT_CONTROLLER_COMMIT_ID);
    BT_DRV_TRACE(1,"BT ramrun %s", BT_CONTROLLER_COMMIT_DATE);
    #else
    if (workmode_patch_version_addr)
    {
        BT_DRV_TRACE(1,"BT_REG_OP:BT 1503: metal id=%d,patch version=%08x", hal_get_chip_metal_id(), BTDIGITAL_REG(workmode_patch_version_addr));
    }
    #endif
    //first move R3 to R9, lost R9
    BT_DRV_TRACE(1,"BT controller BusFault_Handler:\nREG:[PC] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_PC));
    BT_DRV_TRACE(1,"REG:[LR] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE));
    BT_DRV_TRACE(1,"REG:[R0] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +160));
    BT_DRV_TRACE(1,"REG:[R1] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +164));
    BT_DRV_TRACE(1,"REG:[R2] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +168));
    BT_DRV_TRACE(1,"REG:[R3] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +172));
    BT_DRV_TRACE(1,"REG:[R4] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +4));
    BT_DRV_TRACE(1,"REG:[R5] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +8));
    BT_DRV_TRACE(1,"REG:[R6] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +12));
    BT_DRV_TRACE(1,"REG:[R7] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +16));
    BT_DRV_TRACE(1,"REG:[R8] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +20));
    hal_sys_timer_delay(MS_TO_TICKS(100));

    //BT_DRV_TRACE(1,"REG:[R9] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +24));
    BT_DRV_TRACE(1,"REG:[sl] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +28));
    BT_DRV_TRACE(1,"REG:[fp] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +32));
    BT_DRV_TRACE(1,"REG:[ip] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +36));
    BT_DRV_TRACE(1,"REG:[SP,#0] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +40));
    BT_DRV_TRACE(1,"REG:[SP,#4] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +44));
    BT_DRV_TRACE(1,"REG:[SP,#8] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +48));
    BT_DRV_TRACE(1,"REG:[SP,#12] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +52));
    BT_DRV_TRACE(1,"REG:[SP,#16] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +56));
    BT_DRV_TRACE(1,"REG:[SP,#20] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +60));
    BT_DRV_TRACE(1,"REG:[SP,#24] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +64));
    BT_DRV_TRACE(1,"REG:[SP,#28] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +68));
    hal_sys_timer_delay(MS_TO_TICKS(100));

    BT_DRV_TRACE(1,"REG:[SP,#32] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +72));
    BT_DRV_TRACE(1,"REG:[SP,#36] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +76));
    BT_DRV_TRACE(1,"REG:[SP,#40] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +80));
    BT_DRV_TRACE(1,"REG:[SP,#44] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +84));
    BT_DRV_TRACE(1,"REG:[SP,#48] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +88));
    BT_DRV_TRACE(1,"REG:[SP,#52] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +92));
    BT_DRV_TRACE(1,"REG:[SP,#56] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +96));
    BT_DRV_TRACE(1,"REG:[SP,#60] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +100));
    BT_DRV_TRACE(1,"REG:[SP,#64] = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +104));
    BT_DRV_TRACE(1,"REG:SP = 0x%08x",  BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +108));
    BT_DRV_TRACE(1,"REG:MSP = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +112));
    BT_DRV_TRACE(1,"REG:PSP = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +116));
    BT_DRV_TRACE(1,"REG:CFSR = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +120));
    BT_DRV_TRACE(1,"REG:BFAR = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +124));
    BT_DRV_TRACE(1,"REG:HFSR = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +128));
    BT_DRV_TRACE(1,"REG:ICSR = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +132));
    BT_DRV_TRACE(1,"REG:AIRCR = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +136));
    BT_DRV_TRACE(1,"REG:SCR = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +140));
    BT_DRV_TRACE(1,"REG:CCR = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +144));
    BT_DRV_TRACE(1,"REG:SHCSR = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +148));
    BT_DRV_TRACE(1,"REG:AFSR = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +152));
    BT_DRV_TRACE(1,"REG:MMFAR = 0x%08x", BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE +156));
    hal_sys_timer_delay(MS_TO_TICKS(100));

    BT_DRV_TRACE(1,"REG:IP_ERRORTYPESTAT: 0x%x",BTDIGITAL_REG(BT_ERRORTYPESTAT_ADDR));
    BT_DRV_TRACE(1,"REG:IP_BLE_ERRORTYPESTAT: 0x%x",BTDIGITAL_REG(BLE_ERRORTYPESTAT_ADDR));
    BT_DRV_TRACE(1,"REG:BT_CURRENTRXDESCPTR: 0x%x",BTDIGITAL_REG(BT_CURRENTRXDESCPTR_ADDR));

    BT_DRV_TRACE(1,"CONTROLLER RAM BASE:0x%08x",(uint32_t)bt_dump_mem_start);

    btdrv_dump_mem(bt_dump_mem_start, bt_dump_len_max, BT_SUB_SYS_TYPE);

    btdrv_dump_mem(em_dump_area_2_start, em_area_2_len_max, BT_EM_AREA_2_TYPE);

    BT_DRV_TRACE(0,"BT crash dump complete!");

    BT_DRV_REG_OP_CLK_DIB();
}

bool bt_drv_reg_op_read_rssi_in_dbm(uint16_t connHandle,rx_agc_t* rx_val)
{
    bool ret = false;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if (connHandle == 0xFFFF)
    {
        goto exit;
    }

    do{
        uint8_t idx = btdrv_conhdl_to_linkid(connHandle);
        /// Accumulated RSSI (to compute an average value)
        //  int16_t rssi_acc = 0;
        /// Counter of received packets used in RSSI average
        //     uint8_t rssi_avg_cnt = 1;
        rx_agc_t * rx_monitor_env = NULL;
        uint32_t acl_par_ptr = 0;
        uint32_t rssi_record_ptr=0;

        if (!btdrv_is_link_index_valid(idx))
        {
            goto exit;
        }

        if(ld_acl_env_addr)
        {
            acl_par_ptr = *(uint32_t *)(ld_acl_env_addr+idx*4);
            if(acl_par_ptr)
                rssi_record_ptr = acl_par_ptr+off_rr_in_laet_ld_val;
        }

        //rx_monitor
        if(rx_monitor_addr)
        {
            rx_monitor_env = (rx_agc_t*)rx_monitor_addr;
        }

        if(rssi_record_ptr != 0 && rx_monitor_env)
        {
            ret = true;
            // BT_DRV_TRACE(1,"addr=%x,rssi=%d",rssi_record_ptr,*(int8 *)rssi_record_ptr);
#ifdef __NEW_SWAGC_MODE__
            if(bt_drv_reg_op_bt_sync_swagc_en_get())
            {
                rx_val->rssi = *(int8_t *)rssi_record_ptr; // work mode NEW_SYNC_SWAGC_MODE
            }
            else
            {
                rx_val->rssi = rx_monitor_env[idx].rssi; //idle mode OLD_SWAGC_MODE
            }
#else
            rx_val->rssi = rx_monitor_env[idx].rssi;
#endif
            rx_val->rxgain = rx_monitor_env[idx].rxgain;
        }
    }while(0);

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return ret;
}

bool bt_drv_reg_op_read_ble_rssi_in_dbm(uint16_t connHandle,rx_agc_t* rx_val)
{
    bool ret = false;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if (connHandle == 0xFFFF)
    {
        goto exit;
    }

    do{
        uint8_t idx = connHandle;
        /// Accumulated RSSI (to compute an average value)
        int16_t rssi_acc = 0;
        /// Counter of received packets used in RSSI average
        uint8_t rssi_avg_cnt = 1;
        rx_agc_t * rx_monitor_env = NULL;
        if(idx >= MAX_NB_ACTIVE_ACL)
        {
            goto exit;
        }

        //rx_monitor

        if(ble_rx_monitor_addr)
        {
            rx_monitor_env = (rx_agc_t*)ble_rx_monitor_addr;
        }

        if(rx_monitor_env != NULL)
        {
            ret = true;

            for(int i=0; i< rssi_avg_cnt; i++)
            {
                rssi_acc += rx_monitor_env[idx].rssi;
            }
            rx_val->rssi = rssi_acc / rssi_avg_cnt;
            rx_val->rxgain = rx_monitor_env[idx].rxgain;
        }
    }while(0);

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return ret;
}

uint16_t bt_drv_reg_op_bitoff_getf(int elt_idx)
{
    uint16_t bitoff = 0;
    uint32_t acl_evt_ptr = 0x0;

    BT_DRV_REG_OP_CLK_ENB();
    if(ld_acl_env_addr)
    {
        acl_evt_ptr = *(uint32_t *)(ld_acl_env_addr+elt_idx*4);
    }

    if (acl_evt_ptr != 0)
    {
        //last_sync_bit_off = acl_evt_ptr + 0xbe;
        bitoff = *(uint16_t *)(acl_evt_ptr + off_lsbo_in_laet_ld_val);
    }
    else
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR RD bitoff %x", elt_idx);
    }
    BT_DRV_REG_OP_CLK_DIB();

    return bitoff;
}

void bt_drv_reg_op_set_tpoll(uint8_t linkid,uint16_t poll_interval)
{
    uint32_t acl_evt_ptr = 0x0;
    uint32_t poll_addr;
    if(linkid >= MAX_NB_ACTIVE_ACL)
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR TPOLL %x", linkid);
        return;
    }
    BT_DRV_REG_OP_CLK_ENB();
    if(ld_acl_env_addr)
    {
        acl_evt_ptr = *(uint32_t *)(ld_acl_env_addr+linkid*4);
    }

    if (acl_evt_ptr != 0)
    {
        //tpoll off:0xE0
        poll_addr = acl_evt_ptr + off_tp_in_laet_ld_val;
        *(uint16_t *)poll_addr = poll_interval;
    }
    else
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR TPOLL %x", linkid);
    }
    BT_DRV_REG_OP_CLK_DIB();
}

uint16_t bt_drv_reg_op_get_tpoll(uint8_t linkid)
{
    uint32_t acl_evt_ptr = 0x0;
    uint32_t poll_addr;
    uint16_t poll_interval = 0;

    BT_DRV_REG_OP_CLK_ENB();
    if(ld_acl_env_addr)
    {
        acl_evt_ptr = *(uint32_t *)(ld_acl_env_addr+linkid*4);
    }

    if (acl_evt_ptr != 0)
    {
        //tpoll off:0xE0
        poll_addr = acl_evt_ptr + off_tp_in_laet_ld_val;
        poll_interval = *(uint16_t *)poll_addr;
    }
    BT_DRV_REG_OP_CLK_DIB();

    return poll_interval;
}

void bt_drv_reg_op_set_music_link(uint8_t link_id)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(dbg_bt_sche_setting_addr)
    {
        *(volatile uint8_t *)(dbg_bt_sche_setting_addr+off_mpl_in_dbssc_dbg_val) = link_id;
        BT_DRV_TRACE(1,"%s:%d", __func__, link_id);
    }
    BT_DRV_REG_OP_CLK_DIB();
}

void bt_drv_reg_op_music_link_config(uint16_t active_link,uint8_t active_role,uint16_t inactive_link,uint8_t inactive_role)
{
    BT_DRV_TRACE(4,"BT_REG_OP:bt_drv_reg_op_music_link_config %x %d %x %d",active_link,active_role,inactive_link,inactive_role);
    BT_DRV_REG_OP_CLK_ENB();
    if (active_role == 0) //MASTER
    {
        bt_drv_reg_op_set_tpoll(active_link-0x80, 0x10);
        if (inactive_role == 0)
        {
            bt_drv_reg_op_set_tpoll(inactive_link-0x80, 0x40);
        }
    }
    else
    {
        bt_drv_reg_op_set_music_link(active_link-0x80);
        if (inactive_role == 0)
        {
            bt_drv_reg_op_set_tpoll(inactive_link-0x80, 0x40);
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
}

bool bt_drv_reg_op_check_bt_controller_state(void)
{
    bool ret=true;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    uint32_t reg_data = BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE);
    if((reg_data ==0x42) ||(reg_data==0))
    {
        ret = true;
    }
    else
    {
        ret = false;
    }
    if (false == ret)
    {
        BT_DRV_TRACE(1,"controller dead!!! data=%x",reg_data);
    }
    reg_data = BTDIGITAL_REG(BT_ERRORTYPESTAT_ADDR);
    if((reg_data&(~0x40)) !=0)
    {
        ret = false;
        BT_DRV_TRACE(1,"controller dead!!! BT_ERRORTYPESTAT=%x",reg_data);
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

    return ret;
}

void bt_drv_reg_op_piconet_clk_offset_get(uint16_t connHandle, int32_t *clock_offset, uint16_t *bit_offset)
{
    uint8_t index = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if (connHandle)
    {
        index = btdrv_conhdl_to_linkid(connHandle);

        if (btdrv_is_link_index_valid(index))
        {
            *bit_offset = bt_drv_reg_op_bitoff_getf(index);
            *clock_offset = bt_drv_reg_op_get_clkoffset(index);
        }
        else
        {
            *bit_offset = 0;
            *clock_offset = 0;
        }
    }
    else
    {
        *bit_offset = 0;
        *clock_offset = 0;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint8_t dma_tc_used_index = 0;
#define DMA_TC_MAX_CNT  4
void bt_drv_reg_op_enable_dma_tc(uint8_t adma_ch, uint32_t dma_base)
{
    uint32_t val = 0;
    uint8_t i = 0;
    uint32_t lock;
    adma_ch = adma_ch&0xF;
    BT_DRV_REG_OP_CLK_ENB();
    lock = int_lock();
    if(dma_tc_used_index >= DMA_TC_MAX_CNT)
    {
        //  int_unlock(lock);
        TRACE(0,"enable_dma_tc err more than max ch");
        goto exit;
    }
    val = BTDIGITAL_REG(BT_CAP_SEL_ADDR);
    for(i = 0; i < DMA_TC_MAX_CNT; i++)
    {
        if((val>>(i*8)&0x1F)==(uint32_t)(8+adma_ch))
        {
            // int_unlock(lock);
            TRACE(1,"enable_dma_tc err ch already enabled adma ch %d",adma_ch);
            goto exit;
        }
    }
    for(i = 0; i < DMA_TC_MAX_CNT; i++)
    {
        if((val>>(i*8)&0x1F)==0)
            break;
    }
    if(i >= DMA_TC_MAX_CNT)
    {
        //int_unlock(lock);
        TRACE(0,"enable_dma_tc err cannot find unused ch");
        goto exit;
    }
    dma_tc_used_index++;
    BTDIGITAL_REG_SET_FIELD(BT_CAP_SEL_ADDR,1,31,1);//reg_trig_enable
    BTDIGITAL_REG_SET_FIELD(BT_CAP_SEL_ADDR,0x1F,(i*8),(8+adma_ch));//reg_trig_selx
    BTDIGITAL_REG_SET_FIELD(BT_BES_TOG_CNTL_ADDR,1,(i+4),1);//cap mode
    BT_DRV_TRACE(2,"enable_dma_tc succ sel reg 0x%x/%x,toggle reg 0x%x,used %d",val,BTDIGITAL_REG(BT_CAP_SEL_ADDR),BTDIGITAL_REG(BT_BES_TOG_CNTL_ADDR),dma_tc_used_index);
exit:
    int_unlock(lock);
    BT_DRV_REG_OP_CLK_DIB();
}

void bt_drv_reg_op_disable_dma_tc(uint8_t adma_ch, uint32_t dma_base)
{
    uint32_t val = 0;
    uint8_t i = 0;
    uint32_t lock;
    adma_ch = adma_ch&0xF;
    BT_DRV_REG_OP_CLK_ENB();
    lock = int_lock();
    if(dma_tc_used_index == 0)
    {
        // int_unlock(lock);
        TRACE(0,"disable_dma_tc err no ch enabled");
        goto exit;
    }
    val = BTDIGITAL_REG(BT_CAP_SEL_ADDR);
    for(i = 0; i < DMA_TC_MAX_CNT; i++)
    {
        if((val>>(i*8)&0x1F)==(uint32_t)(8+adma_ch))
            break;
    }
    if(i >= DMA_TC_MAX_CNT)
    {
        // int_unlock(lock);
        TRACE(0,"disable_dma_tc err cannot find same ch id to disable");
        goto exit;
    }
    dma_tc_used_index--;
    BTDIGITAL_REG_SET_FIELD(BT_CAP_SEL_ADDR,0x1F,(i*8),0);//reg_trig_selx
    BTDIGITAL_REG_SET_FIELD(BT_BES_TOG_CNTL_ADDR,1,(i+4),0);//cap mode
    BT_DRV_TRACE(2,"disable_dma_tc succ sel reg 0x%x/%x,used %d",val,BTDIGITAL_REG(BT_CAP_SEL_ADDR),dma_tc_used_index);
exit:
    int_unlock(lock);
    BT_DRV_REG_OP_CLK_DIB();
}

void bt_drv_reg_op_dma_tc_clkcnt_get(uint32_t *btclk, uint16_t *btcnt)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    *btclk = BTDIGITAL_REG(BT_REG_CLKNCNT_CAP4_REG_ADDR);
    *btcnt = (BTDIGITAL_REG(BT_BES_FINECNT_CAP4_REG_ADDR) & 0x3ff);
    BT_DRV_TRACE(2, "dma tc clk,cnt %d,%d", *btclk, *btcnt);
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(uint32_t *btclk, uint16_t *btcnt, uint8_t adma_ch, uint32_t dma_base)
{
    uint32_t val = 0;
    uint8_t i = 0;
    uint32_t cap_reg_base = BT_REG_CLKNCNT_CAP4_REG_ADDR;
    adma_ch = adma_ch&0xF;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    val = BTDIGITAL_REG(BT_CAP_SEL_ADDR);
    for(i = 0; i < DMA_TC_MAX_CNT; i++)
    {
        if((val>>(i*8)&0x1F)==(uint32_t)(8+adma_ch))
            break;
    }
    if(i >= DMA_TC_MAX_CNT)
    {
        TRACE(0,"get_dma_tc err cannot find same ch id, not enabled?");
        goto exit;
        return;
    }
    *btclk = BTDIGITAL_REG(cap_reg_base+8*i);
    *btcnt = (BTDIGITAL_REG(cap_reg_base+4+8*i) & 0x3ff);
exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();

}

const uint8_t msbc_mute_patten[]=
{
    0x01,0x38,
    0xad, 0x0,  0x0,  0xc5, 0x0,  0x0, 0x0, 0x0,
    0x77, 0x6d, 0xb6, 0xdd, 0xdb, 0x6d, 0xb7,
    0x76, 0xdb, 0x6d, 0xdd, 0xb6, 0xdb, 0x77,
    0x6d, 0xb6, 0xdd, 0xdb, 0x6d, 0xb7, 0x76,
    0xdb, 0x6d, 0xdd, 0xb6, 0xdb, 0x77, 0x6d,
    0xb6, 0xdd, 0xdb, 0x6d, 0xb7, 0x76, 0xdb,
    0x6d, 0xdd, 0xb6, 0xdb, 0x77, 0x6d, 0xb6,
    0xdd, 0xdb, 0x6d, 0xb7, 0x76, 0xdb, 0x6c,
    0x00
};

//only this function write BTC EM
#define SCO_TX_FIFO_BASE (EM_BASE_ADDR)

#if defined(CVSD_BYPASS)
#define SCO_TX_MUTE_PATTERN (0x5555)
#else
#define SCO_TX_MUTE_PATTERN (0x0000)
#endif

void bt_drv_reg_op_sco_txfifo_reset(uint16_t codec_id)
{
    BT_DRV_REG_OP_CLK_ENB();
    uint32_t reg_val = BTDIGITAL_REG(BT_E_SCOCURRENTTXPTR_ADDR);
    uint32_t reg_offset0, reg_offset1;
    uint16_t *patten_p = (uint16_t *)msbc_mute_patten;
    BT_DRV_TRACE(2,"BT_REG_OP sco reset=%x", reg_val);

    reg_offset0 = (reg_val & 0x3fff)<<(2+EM_SHIFT_EXT);
    reg_offset1 = ((reg_val >> 16) & 0x3fff)<<(2+EM_SHIFT_EXT);

    if(reg_offset0 == 0 || reg_offset1 == 0)
    {
        goto exit;
    }

    if (codec_id == 2)
    {
        for (uint8_t i=0; i<60; i+=2)
        {
            BTDIGITAL_BT_EM(SCO_TX_FIFO_BASE+reg_offset0+i) = *patten_p;
            BTDIGITAL_BT_EM(SCO_TX_FIFO_BASE+reg_offset1+i) = *patten_p;
            patten_p++;
        }
    }
    else
    {
        for (uint8_t i=0; i<120; i+=2)
        {
            BTDIGITAL_BT_EM(SCO_TX_FIFO_BASE+reg_offset0+i) = SCO_TX_MUTE_PATTERN;
            BTDIGITAL_BT_EM(SCO_TX_FIFO_BASE+reg_offset1+i) = SCO_TX_MUTE_PATTERN;
        }
    }
exit:
    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_reg_op_set_private_tws_poll_interval(uint16_t poll_interval, uint16_t poll_interval_in_sco)
{
    //interval_ibrt_estab=poll_interval/2 and interval must be odd
    ASSERT_ERR((poll_interval % 4 == 0));

    BT_DRV_REG_OP_CLK_ENB();
    uint32_t interval_addr = 0;
    uint32_t interval_insco_addr = 0;

    if(dbg_bt_sche_setting_addr != 0)
    {
        interval_insco_addr =  dbg_bt_sche_setting_addr;
        interval_addr = (dbg_bt_sche_setting_addr + off_aiiinm_in_dbssc_dbg_val);

        do{
            if(BT_DRIVER_GET_U16_REG_VAL(interval_addr) != poll_interval && poll_interval!=0xffff)
            {
                BT_DRIVER_PUT_U16_REG_VAL(interval_addr, poll_interval);
                BT_DRV_TRACE(2,"BT_REG_OP:Set private tws interval,acl interv=%d", poll_interval);
            }
            if(BT_DRIVER_GET_U16_REG_VAL(interval_insco_addr) != poll_interval_in_sco && poll_interval_in_sco!=0xffff)
            {
                BT_DRIVER_PUT_U16_REG_VAL(interval_insco_addr, poll_interval_in_sco);

                BT_DRV_TRACE(2," acl interv insco =%d", poll_interval_in_sco);
           }
        }while(0);
    }
    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_reg_op_set_tws_link_duration(uint8_t slot_num)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    //dbg_setting address
    uint32_t op_addr = 0;

    if(dbg_bt_sche_setting_addr)
    {
        op_addr = dbg_bt_sche_setting_addr;
    }

    if(op_addr != 0 && slot_num != 0xff)
    {
        *(volatile uint8_t *)(op_addr+off_asiim_in_dbssc_dbg_val) = slot_num;//acl_slot_in_snoop_mode
        BT_DRV_TRACE(1,"BT_REG_OP:Set private tws link duration,val=%d",slot_num);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


void bt_drv_reg_op_write_private_public_key(uint8_t* private_key,uint8_t* public_key)
{
}

void bt_drv_reg_op_for_test_mode_disable(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(dbg_bt_common_setting_addr)
    {
        *(volatile uint8_t *)(dbg_bt_common_setting_addr+off_tl_in_dbcsc_dbg_val) = 0;////dbg_trace_level set 0
    }
    BT_DRV_REG_OP_CLK_DIB();
}

uint16_t bt_drv_reg_op_get_ibrt_sco_hdl(uint16_t acl_hdl)
{
    // FIXME
    return acl_hdl|0x100;
}

bool bt_drv_is_support_dual_sco(void)
{
    return true;
}

bool bt_drv_is_support_hci_set_tws_link(void)
{
    return true;
}

bool bt_drv_is_support_hci_config_multi_ibrt_sche(void)
{
    bool ret = false;

    uint32_t btc_version = bt_drv_get_btc_sw_version();
    if (btc_version >= IBRT_MULTI_SCH_HCI_SUPPORT_VERSION)
    {
        ret = true;
    }

    return ret;
}

bool bt_drv_is_support_hci_enable_btpcm(void)
{
    return true;
}

void bt_drv_reg_op_set_tws_link_id(uint8_t link_id)
{
    BT_DRV_TRACE(1,"set tws link id =%x",link_id);
    ASSERT(link_id <MAX_NB_ACTIVE_ACL || link_id == 0xff,"BT_REG_OP:error tws link id set");
    uint32_t tws_link_id_addr = 0;

    BT_DRV_REG_OP_CLK_ENB();
    if(ld_bes_bt_env_addr != 0)
    {
        tws_link_id_addr = ld_bes_bt_env_addr + off_li_in_lbbet_ld_val;
    }

    if(tws_link_id_addr != 0)
    {
        if(link_id == 0xff)
        {
            link_id = MAX_NB_ACTIVE_ACL;
        }
        *(uint8_t *)tws_link_id_addr = link_id;
    }
    BT_DRV_REG_OP_CLK_DIB();
}

void bt_drv_reg_op_ble_sup_timeout_set(uint16_t ble_conhdl, uint16_t sup_to)
{
}

void bt_drv_reg_op_force_set_sniff_att(uint16_t conhdle)
{
//sniff Timeout BUG has been fixed
}

void bt_drv_reg_op_fa_gain_direct_set(uint8_t gain_idx)
{
}

uint8_t bt_drv_reg_op_fa_gain_direct_get(void)
{
    return 0;
}

#ifdef __DEBUG_FIX_RX_GAIN_CNTL__
struct rx_gain_fixed_t
{
    uint8_t     enable;//enable or disable
    uint8_t     bt_or_ble;//0:bt 1:ble
    uint8_t     cs_id;//link id
    uint8_t     gain_idx;//gain index
};

void bt_drv_reg_op_dgb_link_gain_ctrl_set(uint16_t connHandle, uint8_t bt_ble_mode, uint8_t gain_idx, uint8_t enable)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_fixed_tbl_addr != 0)
    {
        //struct rx_gain_fixed_t RF_RX_GAIN_FIXED_TBL[HOST_GAIN_TBL_MAX];
        struct rx_gain_fixed_t *rx_gain_fixed_p = (struct rx_gain_fixed_t *)rf_rx_gain_fixed_tbl_addr;
        uint8_t cs_id = btdrv_conhdl_to_linkid(connHandle);

        if (btdrv_is_link_index_valid(cs_id))
        {
            for (uint8_t i=0; i<MAX_NB_ACTIVE_ACL; i++)
            {
                if ((rx_gain_fixed_p->cs_id == cs_id) &&
                    (rx_gain_fixed_p->bt_or_ble == bt_ble_mode))
                {
                    rx_gain_fixed_p->enable    = enable;
                    rx_gain_fixed_p->bt_or_ble = bt_ble_mode;
                    rx_gain_fixed_p->gain_idx = gain_idx;
                    BT_DRV_TRACE(5,"BT_REG_OP:%s hdl:%x/%x mode:%d idx:%d", __func__, connHandle, cs_id, rx_gain_fixed_p->bt_or_ble, gain_idx);
                }
                rx_gain_fixed_p++;
            }
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
}


void bt_drv_reg_op_dgb_link_gain_ctrl_clear(uint16_t connHandle, uint8_t bt_ble_mode)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_fixed_tbl_addr != 0)
    {
        //struct rx_gain_fixed_t RF_RX_GAIN_FIXED_TBL[HOST_GAIN_TBL_MAX];
        struct rx_gain_fixed_t *rx_gain_fixed_p = (struct rx_gain_fixed_t *)rf_rx_gain_fixed_tbl_addr;
        uint8_t cs_id = btdrv_conhdl_to_linkid(connHandle);

        if (btdrv_is_link_index_valid(cs_id))
        {
            for (uint8_t i=0; i<MAX_NB_ACTIVE_ACL; i++)
            {
                if ((rx_gain_fixed_p->cs_id == cs_id) &&
                    (rx_gain_fixed_p->bt_or_ble == bt_ble_mode))
                {
                    rx_gain_fixed_p->enable    = 0;
                    rx_gain_fixed_p->bt_or_ble = bt_ble_mode;
                    rx_gain_fixed_p->gain_idx = 0;
                }
                rx_gain_fixed_p++;
            }
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
}


void bt_drv_reg_op_dgb_link_gain_ctrl_init(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_fixed_tbl_addr != 0)
    {
        //struct rx_gain_fixed_t RF_RX_GAIN_FIXED_TBL[HOST_GAIN_TBL_MAX];
        struct rx_gain_fixed_t *rx_gain_fixed_p = (struct rx_gain_fixed_t *)rf_rx_gain_fixed_tbl_addr;

        for (uint8_t i=0; i<MAX_NB_ACTIVE_ACL; i++)
        {
            rx_gain_fixed_p->cs_id = i;
            rx_gain_fixed_p++;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
}

void bt_drv_reg_op_rx_gain_fix(uint16_t connHandle, uint8_t bt_ble_mode, uint8_t gain_idx, uint8_t enable, uint8_t table_idx)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_fixed_tbl_addr != 0)
    {
        //struct rx_gain_fixed_t RF_RX_GAIN_FIXED_TBL[HOST_GAIN_TBL_MAX];
        struct rx_gain_fixed_t *rx_gain_fixed_p = (struct rx_gain_fixed_t *)rf_rx_gain_fixed_tbl_addr;
        uint8_t cs_id;

        if(bt_ble_mode == 0)//bt
        {
            cs_id = btdrv_conhdl_to_linkid(connHandle);
        }
        else if(bt_ble_mode == 1)//ble
        {
            cs_id = connHandle;
        }
        else
        {
            BT_DRV_TRACE(1,"BT_REG_OP:%s:fix gain fail",__func__);
            goto exit;
        }

        if(table_idx < MAX_NB_ACTIVE_ACL)
        {
            rx_gain_fixed_p[table_idx].enable = enable;
            rx_gain_fixed_p[table_idx].bt_or_ble = bt_ble_mode;
            rx_gain_fixed_p[table_idx].cs_id = cs_id;
            rx_gain_fixed_p[table_idx].gain_idx = gain_idx;
        }
    }
exit:
    BT_DRV_REG_OP_CLK_DIB();
}
#endif

void bt_drv_reg_op_set_ibrt_reject_sniff_req(bool en)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    uint8_t *accept_remote_enter_sniff_ptr = NULL;
    uint32_t sniff_policy_addr = 0;

    if (dbg_bt_ibrt_setting_addr == 0)
    {
        BT_DRV_TRACE(1, "BT_REG_OP:set reject sniff req %d, please fix it", en);
        goto exit;
    }

    sniff_policy_addr = dbg_bt_ibrt_setting_addr + off_ares_in_dbisc_dbg_val;

    if(BT_DRIVER_GET_U8_REG_VAL(sniff_policy_addr) == en)
    {
        BT_DRV_TRACE(1, "BT_REG_OP:set reject sniff req %d", en);

        accept_remote_enter_sniff_ptr = (uint8_t *)(sniff_policy_addr);

        *accept_remote_enter_sniff_ptr = en ? 0 : 1;
    }
exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_ibrt_second_sco_decision(uint8_t value)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    uint8_t *ibrt_second_sco_decision_ptr = NULL;

    if (dbg_bt_ibrt_setting_addr == 0)
    {
        BT_DRV_TRACE(1, "BT_REG_OP:set ibrt second sco decision %d, please fix it", value);
        goto exit;
    }

    ibrt_second_sco_decision_ptr = (uint8_t *)(dbg_bt_ibrt_setting_addr + off_issd_in_dbisc_dbg_val);

    *ibrt_second_sco_decision_ptr = value;

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint8_t bt_drv_reg_op_get_sync_id_by_conhdl(uint16_t conhdl)
{
    uint8_t sco_link_id = MAX_NUM_SCO_LINKS;
    uint32_t p_link_params_addr = 0;
    uint32_t link_id_addr = 0;

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    uint8_t idx = btdrv_conhdl_to_linkid(conhdl);
    if (!btdrv_is_link_index_valid(idx))
    {
        goto exit;
    }

    if(lc_sco_env_addr != 0)
    {
        // Find the SCO link under negotiation
        for(uint8_t i = 0; i < MAX_NUM_SCO_LINKS ; i++)
        {
            p_link_params_addr = BT_DRIVER_GET_U32_REG_VAL(lc_sco_env_addr + i * 8);
            link_id_addr = p_link_params_addr + off_li_in_lset_lc_val;
            // Check if sco link ID is free
            if(p_link_params_addr != 0)
            {
                if(BT_DRIVER_GET_U16_REG_VAL(link_id_addr) == idx)
                {
                    sco_link_id = i;
                    break;
                }
            }
        }
    }
exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return sco_link_id;
}

uint8_t bt_drv_reg_op_get_sco_type(uint8_t sco_link_id)
{
    uint8_t sco_type = 0xff;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(lc_sco_env_addr != 0)
    {
        uint32_t p_nego_params_addr = BT_DRIVER_GET_U32_REG_VAL(lc_sco_env_addr + 4 + sco_link_id * 8);
        if (sco_link_id >= 2 || p_nego_params_addr == 0)
        {
            goto exit;
        }
        //m2s_pkt_type offset:57
        sco_type = BT_DRIVER_GET_U8_REG_VAL(p_nego_params_addr + off_mpt_in_lsnpt_lc_val);

        BT_DRV_TRACE(1,"BT_REG_OP:Get nego sco type =%d",sco_type);
    }

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return sco_type;
}

uint8_t bt_drv_reg_op_get_esco_nego_airmode(uint8_t sco_link_id)
{
    uint8_t airmode = 0xff;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(lc_sco_env_addr != 0)
    {
        uint32_t p_nego_params_addr = BT_DRIVER_GET_U32_REG_VAL(lc_sco_env_addr + 4 + sco_link_id * 8);
        if (sco_link_id >= 2 || p_nego_params_addr == 0)
        {
            goto exit;
        }
        //air_mode off:5e
        airmode = BT_DRIVER_GET_U8_REG_VAL(p_nego_params_addr + off_am_in_lsnpt_lc_val);

        BT_DRV_TRACE(1,"BT_REG_OP:Get nego esco airmode=%d",airmode);
    }

exit:
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return airmode;
}

#if defined(PCM_FAST_MODE) && defined(PCM_PRIVATE_DATA_FLAG)
void bt_drv_reg_op_set_pcm_flag()
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    //config btpcm slot
    BTDIGITAL_REG_SET_FIELD(BT_BES_ENHPCM_CNTL_ADDR,0x1,0,1);
    BTDIGITAL_REG_SET_FIELD(BT_BES_ENHPCM_CNTL_ADDR,0x4,1,7);

    //config private data
    bt_bes_pcmcntl_pcm_enhmode_setf(1)
    bt_bes_pcmcntl_pcm_fastmode_setf(1)
    bt_bes_pcmcntl_pcm_esco_frame_num_setf(0x4b);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
#endif

//to do...
void bt_drv_reg_op_ebq_test_setting(void)
{
    #if 0
    BT_DRV_REG_OP_CLK_ENB();
    struct dbg_set_ebq_test_cmd* set_ebq_test = (struct dbg_set_ebq_test_cmd*)hci_dbg_ebq_test_mode_addr;
    if(set_ebq_test)
    {
        BT_DRV_TRACE(0,"BT_REG_OP:ebq_test_setting");
        set_ebq_test->ssr = 0;//ebq case bb/prot/ssr/bv03 06
        set_ebq_test->aes_ccm_daycounter = 1;
        set_ebq_test->bb_prot_flh_bv02 = 0;
        set_ebq_test->bb_prot_arq_bv43 = 0;
        set_ebq_test->enc_mode_req = 0;
        set_ebq_test->lmp_lih_bv126 = 0;
        set_ebq_test->lsto_add = 5;
        set_ebq_test->bb_xcb_bv06 = 0;
        set_ebq_test->bb_xcb_bv20 = 0;
        set_ebq_test->bb_inq_pag_bv01 = 0;
        set_ebq_test->sam_status_change_evt = 0;
        set_ebq_test->sam_remap = 0;
        set_ebq_test->ll_con_sla_bi02c = 0;
        set_ebq_test->ll_con_sla_bi02c_offset = 0;
        set_ebq_test->ll_pcl_mas_sla_bv01 = 0;
        set_ebq_test->ll_pcl_sla_bv29 = 0;
        set_ebq_test->bb_prot_arq_bv0608 = 0;
        set_ebq_test->lmp_lih_bv48 = 0;
        set_ebq_test->hfp_hf_acs_bv17_i = 0;
        set_ebq_test->ll_not_support_phy = 0;
        set_ebq_test->ll_iso_hci_in_sdu_len_flag = 1;
        set_ebq_test->ll_iso_hci_in_buf_num = 0;
        set_ebq_test->ll_cis_mic_present = 0;
        set_ebq_test->ll_cig_param_check_disable = 1;
        set_ebq_test->ll_total_num_iso_data_pkts = 6;
        set_ebq_test->cis_offset_min = SLOT_SIZE*2;//us
        set_ebq_test->cis_sub_event_duration_config = 0;//24*HALF_SLOT_SIZE
        set_ebq_test->ll_bi_alarm_init_distance = 12;//(in half slots)
        set_ebq_test->lld_sync_duration_min = HALF_SLOT_SIZE;//(in half us)
        //iso rx test case
        set_ebq_test->lld_le_duration_min = 0;//HALF_SLOT_SIZE;//(in half us)
        //iso rx test case
        set_ebq_test->lld_cibi_sync_win_min_distance = 500;//(in half us)
        //cis last subevent duration min use half slot size
        set_ebq_test->iso_sub_event_duration_config = 1;
        //ebq bug only ll_bis_snc_bv7 and ll_bis_snc_bv11 enable
        set_ebq_test->ll_bis_snc_bv7_bv11 = 0;
    }
    BT_DRV_REG_OP_CLK_DIB();
    #endif
}


void bt_drv_reg_op_ble_audio_config_init(void)
{
#if 0
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    struct dbg_set_ebq_test_cmd* set_ebq_test = (struct dbg_set_ebq_test_cmd*)hci_dbg_ebq_test_mode_addr;
    struct hci_dbg_ble_cmd* dbg_ble_par = (struct hci_dbg_ble_cmd*)hci_dbg_ble_addr;
    if(set_ebq_test)
    {
        BT_DRV_TRACE(0,"BT_REG_OP:ble audio config");
        set_ebq_test->cis_offset_min = SLOT_SIZE*2;//us
        set_ebq_test->cis_sub_event_duration_config = 0;//24*HALF_SLOT_SIZE
        set_ebq_test->ll_bi_alarm_init_distance = 12;//(in half slots)
        set_ebq_test->lld_sync_duration_min = HALF_SLOT_SIZE;//(in half us)
        //iso rx test case
        set_ebq_test->lld_le_duration_min = HALF_SLOT_SIZE;//(in half us)
        //iso rx test case
        set_ebq_test->lld_cibi_sync_win_min_distance = 500;//(in half us)
        //cis last subevent duration min use half slot size
        set_ebq_test->iso_sub_event_duration_config = 1;
    }
    if(dbg_ble_par)
    {
        dbg_ble_par->isoohci_in_buf_size = ISO_BUF_SIZE;
        dbg_ble_par->isoohci_in_buf_nb = ISO_BUF_NB;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
#endif
}

uint8_t bt_drv_reg_op_get_controller_ble_tx_free_buffer(void)
{
    uint32_t *tx_buf_ptr=NULL;
    uint8_t tx_free_buf_count=0;
    BT_DRV_REG_OP_CLK_ENB();

    if(ble_util_buf_env_addr)
    {
        tx_buf_ptr = (uint32_t *)(ble_util_buf_env_addr+off_atf_in_bubet_ble_val); //ble_util_buf_env.acl_tx_free
    }
    else
    {
        BT_DRV_TRACE(1, "REG_OP: please fix %s", __func__);
        tx_free_buf_count =  0;
    }

    //check tx buff
    while(tx_buf_ptr && *tx_buf_ptr)
    {
        tx_free_buf_count++;
        tx_buf_ptr = (uint32_t *)(*tx_buf_ptr);
    }
    BT_DRV_REG_OP_CLK_DIB();

    return tx_free_buf_count;
}

#ifdef __NEW_SWAGC_MODE__
void bt_drv_reg_op_bt_sync_swagc_en_set(uint8_t en)
{
    BT_DRV_REG_OP_CLK_ENB();

    uint8_t *bt_sync_swagc_en = NULL;

    if (dbg_bt_hw_feat_setting_addr != 0)
    {
        bt_sync_swagc_en = (uint8_t *)(dbg_bt_hw_feat_setting_addr + off_bsse_in_dbhfsc_dbg_val);

        *bt_sync_swagc_en = en;
    }

    BT_DRV_REG_OP_CLK_DIB();
}

uint8_t bt_drv_reg_op_bt_sync_swagc_en_get(void)
{
    uint8_t *bt_sync_swagc_en = NULL;
    uint8_t ret = 0;
    BT_DRV_REG_OP_CLK_ENB();

    if (dbg_bt_hw_feat_setting_addr != 0)
    {
        bt_sync_swagc_en = (uint8_t *)(dbg_bt_hw_feat_setting_addr + off_bsse_in_dbhfsc_dbg_val);
    }

    if (bt_sync_swagc_en)
    {
        ret = *bt_sync_swagc_en;
    }

    BT_DRV_REG_OP_CLK_DIB();

    return ret;
}
#endif

#ifdef PCM_PRIVATE_DATA_FLAG
void bt_drv_reg_op_sco_pri_data_init()
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    BTDIGITAL_REG_WR(BT_BES_PCM_DF_REG1_ADDR,BTDIGITAL_REG(BT_BES_PCM_DF_REG1_ADDR)|=0x000000ff);
    BTDIGITAL_REG_WR(BT_BES_PCM_DF_REG_ADDR,BTDIGITAL_REG(BT_BES_PCM_DF_REG_ADDR)|=0x00ffffff);
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
#endif

void bt_drv_reg_op_init_rssi_setting(void)
{
    BT_DRV_REG_OP_CLK_ENB();

    if(hci_dbg_set_sw_rssi_addr)
    {

        *(uint8_t *)(hci_dbg_set_sw_rssi_addr+0) = 1; //sw_rssi_en
        *(uint32_t *)(hci_dbg_set_sw_rssi_addr+4) = 80;  //link_agc_thd_mobile
        *(uint32_t *)(hci_dbg_set_sw_rssi_addr+8) = 100;  //link_agc_thd_mobile_time
        *(uint32_t *)(hci_dbg_set_sw_rssi_addr+12) = 5;  //link_agc_thd_tws
        *(uint32_t *)(hci_dbg_set_sw_rssi_addr+16) = 352;  //link_agc_thd_tws_time
        *(uint8_t *)(hci_dbg_set_sw_rssi_addr+20) = 3;  //rssi_mobile_step
        *(uint8_t *)(hci_dbg_set_sw_rssi_addr+21) = 3;  //rssi_tws_step
        *(int8_t *)(hci_dbg_set_sw_rssi_addr+22) = -100;  //rssi_min_value_mobile
        *(int8_t *)(hci_dbg_set_sw_rssi_addr+23) = -100;  //rssi_min_value_tws
        *(uint8_t *)(hci_dbg_set_sw_rssi_addr+24) = 0;  //ble_sw_rssi_en
        *(uint32_t *)(hci_dbg_set_sw_rssi_addr+28) = 80;  //ble_link_agc_thd
        *(uint32_t *)(hci_dbg_set_sw_rssi_addr+32) = 100;  //ble_link_agc_thd_time
        *(uint8_t *)(hci_dbg_set_sw_rssi_addr+36) = 3;  //ble_rssi_step
        *(int8_t *)(hci_dbg_set_sw_rssi_addr+37) = -100;  //ble_rssidbm_min_value

        //NO UESD
        *(uint8_t *)(hci_dbg_set_sw_rssi_addr+38) = 0;  //bt_no_sync_en by BTC
        *(int8_t *)(hci_dbg_set_sw_rssi_addr+39) = -90;  //bt_link_no_sync_rssi
        *(uint16_t *)(hci_dbg_set_sw_rssi_addr+40) = 80;  //bt_link_no_snyc_thd
        *(uint16_t *)(hci_dbg_set_sw_rssi_addr+42) = 200;  //bt_link_no_sync_timeout

        *(uint8_t *)(hci_dbg_set_sw_rssi_addr+44) = 1;  //ble_no_sync_en
        *(int8_t *)(hci_dbg_set_sw_rssi_addr+45) = -90;  //ble_link_no_sync_rssi
        *(uint16_t *)(hci_dbg_set_sw_rssi_addr+46) = 20;  //ble_link_no_snyc_thd
        *(uint16_t *)(hci_dbg_set_sw_rssi_addr+48) = 800;  //ble_link_no_sync_timeout
    }

    BT_DRV_REG_OP_CLK_DIB();
}

uint32_t bt_drv_us_2_lpcycles(uint32_t us,uint32_t lp_clk)
{
    uint64_t lpcycles;
    lpcycles = ((uint64_t)us*lp_clk)/1000000ll;
    return((uint32_t)lpcycles);
}

void bt_drv_reg_op_data_bakeup_init(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(data_backup_cnt_addr && btc_stack_limited_addr)
    {
        *(uint32_t *)data_backup_cnt_addr = btc_stack_limited_addr+0x10;////set the buf at the stack bottom
        *(uint32_t *)data_backup_addr_ptr_addr = btc_stack_limited_addr+0x14;//set 32 reg backup max
        *(uint32_t *)data_backup_val_ptr_addr = btc_stack_limited_addr+0x94;
    }
    BT_DRV_REG_OP_CLK_DIB();
}

void memcpy32(uint32_t * dst, const uint32_t * src, size_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        dst[i] = src[i];
    }
}

void bt_drv_reg_op_data_backup_write(const uint32_t *ptr,uint32_t cnt)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(data_backup_cnt_addr)
    {
        *(uint32_t *)(*(uint32_t *)data_backup_cnt_addr) = cnt;
        for(uint32_t i=0; i<cnt; i++)
        {
            memcpy32((uint32_t *)(*(uint32_t *)data_backup_addr_ptr_addr),ptr,cnt);
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
}


#ifdef __SW_TRIG__
uint16_t bt_drv_reg_op_rxbit_1us_get(uint16_t conhdl)
{
    uint8_t conidx;
    conidx = btdrv_conhdl_to_linkid(conhdl);
    uint16_t rxbit_1us = 0;
    uint32_t acl_evt_ptr = 0x0;

    BT_DRV_REG_OP_CLK_ENB();
    if(ld_acl_env_addr)
    {
        acl_evt_ptr = *(uint32_t *)(ld_acl_env_addr+conidx*4);
    }

    if (acl_evt_ptr != 0)
    {
        rxbit_1us = *(uint16_t *)(acl_evt_ptr + off_r1_in_laet_ld_val);//acl_par->rxbit_1us
        BT_DRV_TRACE(1,"[%s] rxbit_1us=%d\n",__func__,rxbit_1us);
    }
    else
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR RD rxbit_1us %x", conidx);
    }
    BT_DRV_REG_OP_CLK_DIB();

    return rxbit_1us;
}
#endif

void bt_drv_reg_op_set_btpcm_trig_flag(bool flag)
{
    BT_DRV_REG_OP_CLK_ENB();

    if(pcm_need_start_flag_addr != 0)
    {
        BT_DRV_TRACE(1,"Enable pcm %d", flag);
        BTDIGITAL_REG(pcm_need_start_flag_addr) = flag;
    }
    BT_DRV_REG_OP_CLK_DIB();
}

void bt_drv_reg_op_multi_ibrt_sche_adjust(struct sch_multi_ibrt_adjust_timeslice_env_t* updated_sch_para)
{
    struct sch_multi_ibrt_adjust_timeslice_env_t* sch_multi_ibrt_adjust_env_ptr = 0x0;
    BT_DRV_REG_OP_CLK_ENB();

    if(sch_multi_ibrt_adjust_env_addr)
    {
        sch_multi_ibrt_adjust_env_ptr = (sch_multi_ibrt_adjust_timeslice_env_t *)sch_multi_ibrt_adjust_env_addr;
        if(sch_multi_ibrt_adjust_env_ptr->update)
        {
            TRACE(0,"WARNING: previous update sch not finished");
        }
        memcpy(sch_multi_ibrt_adjust_env_ptr->ibrt_link,updated_sch_para->ibrt_link,sizeof(struct sch_adjust_timeslice_per_link)*MULTI_IBRT_SUPPORT_NUM);
        sch_multi_ibrt_adjust_env_ptr->update = 1;
    }
    BT_DRV_REG_OP_CLK_DIB();

}

void bt_drv_reg_op_multi_ibrt_music_config(uint8_t* link_id, uint8_t* active, uint8_t num)
{
    sch_multi_ibrt_adjust_timeslice_env_t updated_sch_para;
    uint8_t i = 0;
    if(link_id[i]>=MAX_NB_ACTIVE_ACL)
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR multi_ibrt_music_config %x", link_id[i]);
        return;
    }
    BT_DRV_REG_OP_CLK_ENB();

    while(num > 0)
    {
        TRACE(0,"multi_ibrt_music linkid %d,active %d",link_id[i],active[i]);
        updated_sch_para.ibrt_link[i].link_id = link_id[i];
        if(active[i] == 1)
        {
            updated_sch_para.ibrt_link[i].timeslice = MULTI_IBRT_FG_SLICE;//active timeslice
        }
        else
        {
            updated_sch_para.ibrt_link[i].timeslice = MULTI_IBRT_BG_SLICE;//inactive timeslice
        }
        num--;
        i++;
    }
    while(i < MULTI_IBRT_SUPPORT_NUM)
    {
        updated_sch_para.ibrt_link[i].link_id = 0xFF;
        i++;
    }
    bt_drv_reg_op_multi_ibrt_sche_adjust(&updated_sch_para);
    BT_DRV_REG_OP_CLK_DIB();
}

void bt_drv_reg_op_low_txpwr_set(uint8_t enable, uint8_t factor, uint8_t bt_or_ble, uint8_t link_id)
{
    struct dbg_set_txpwr_mode_cmd low_txpwr;

    if(bt_or_ble)
    {
        ASSERT_ERR(link_id < BLE_ACTIVITY_MAX);
    }
    else
    {
        ASSERT_ERR(link_id < MAX_NB_ACTIVE_ACL);
    }

    low_txpwr.enable = enable;
    low_txpwr.factor = factor;
    low_txpwr.bt_or_ble = bt_or_ble;
    low_txpwr.link_id  = link_id;

    btdrv_send_cmd(HCI_DBG_SET_TXPWR_MODE_CMD_OPCODE,sizeof(low_txpwr),(uint8_t *)&low_txpwr);
}

bool bt_drv_error_check_handler(void)
{
    bool ret = false;
    BT_DRV_REG_OP_CLK_ENB();
    if((BTDIGITAL_REG(BT_ERRORTYPESTAT_ADDR)&(~0x40)) !=0 ||
       (BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE) !=0 &&
        BTDIGITAL_REG(BT_CONTROLLER_CRASH_DUMP_ADDR_BASE) !=0x42))
    {
        BT_DRV_TRACE(1,"BT_DRV:digital assert,error code=0x%x", BTDIGITAL_REG(BT_ERRORTYPESTAT_ADDR));
        ret = true;
    }
    BT_DRV_REG_OP_CLK_DIB();
    return ret;
}

#ifdef MCU_WAKEUP_BT_V2
void bt_drv_i2v_disable_sleep_for_bt_access(void)
{
    bt_drv_oper_btcore_with_rsp(WAKEUP_BT_USER_TRIG, MSG_WAKEUP_BT_AND_DIS_SLP);
    BT_DRV_TRACE(0, "BT:trigger req wakeup");
}

void bt_drv_i2v_enable_sleep_for_bt_access(void)
{
    bt_drv_oper_btcore_with_rsp(WAKEUP_BT_USER_TRIG, MSG_WAKEUP_BT_AND_EN_SLP);
    BT_DRV_TRACE(0, "BT:resume sleep");
}

void bt_drv_only_wakeup_btcore(void)
{
    bt_drv_oper_btcore_with_rsp(WAKEUP_BT_USER_TRIG, MSG_ONLY_WAKEUP_BT);
}
#else
void bt_drv_i2v_disable_sleep_for_bt_access(void)
{
    uint32_t curr_tick,temp_tick;
    hal_intersys_wakeup_btcore();

    curr_tick = hal_sys_timer_get();
    do{
        hal_sys_timer_delay_us(100);
        temp_tick = hal_sys_timer_get();
        if(temp_tick - curr_tick > MS_TO_TICKS(3))
        {
            TRACE(0,"BT:EXT Wait BT rsp 3ms");
            hal_intersys_wakeup_btcore();
            curr_tick = hal_sys_timer_get();
        }
    }while(hal_intersys_get_wakeup_flag());
    BT_DRV_TRACE(0,"BT:inter req wakeup cnt=%d", reg_op_clk_enb);
}

void bt_drv_i2v_enable_sleep_for_bt_access(void)
{
    uint32_t flag_addr = rt_sleep_flag_clear_addr;

    if(flag_addr !=0)
    {
        if(*(uint32_t *)flag_addr == 0)
        {
            *(uint32_t *)flag_addr = 1;
            hal_sys_timer_delay_us(100);
        }
    }
    BT_DRV_TRACE(0,"BT:resume sleep cnt=%d", reg_op_clk_enb);
}

void bt_drv_clear_wakeup_request(void)
{
    uint32_t flag_addr = rt_sleep_flag_clear_addr;
    if(flag_addr !=0)
    {
        if(*(uint32_t *)flag_addr == 0)
        {
            *(uint32_t *)flag_addr = 1;
            hal_sys_timer_delay_us(100);
        }
    }
    BT_DRV_TRACE(0, "%s", __func__);
}
#endif /* MCU_WAKEUP_BT_V2 */

void bt_drv_reg_op_trigger_controller_assert(void)
{
    ASSERT(0, "Trigger BTC crash on purpose!");
}

void bt_drv_reg_op_afh_assess_en(bool en)
{
#if defined(__AFH_ASSESS__)
    static bool en_back = false;
    if(en == en_back)
    {
        return;
    }
    en_back = en;

    BT_DRV_TRACE(1,"BT_REG_OP:set afh assess=%d",en);

    struct hci_dbg_set_afh_assess_params hci_afh_assess_init_config
    {
        .enable  = 0,
        .interval_factor = 1,
        .afh_average_cnt = 7,
        .afh_good_chl_thr = -90,

        .afh_sch_expect_assess_num = 40,
    };

    if(en)
    {
        hci_afh_assess_init_config.enable = 1;
    }

    btdrv_send_hci_cmd_bystack(HCI_DBG_AFH_ASSESS_CMD_OPCODE, (uint8_t *)&hci_afh_assess_init_config, sizeof(struct hci_dbg_set_afh_assess_params));
#endif
}

uint32_t bt_drv_lp_clk_get()
{
    uint32_t lp_clk = 0xFFFFFFFF;
    BT_DRV_REG_OP_CLK_ENB();
    if(rwip_env_addr)
        lp_clk = BTDIGITAL_REG(rwip_env_addr+off_lc_in_ret_rwip_val);

    BT_DRV_REG_OP_CLK_DIB();
    return lp_clk;
}

void btdrv_reg_op_vco_test_start(uint8_t chnl)
{
    BT_DRV_REG_OP_CLK_ENB();
    bt_bes_testmode_set((chnl & 0x7f) | 0xa0000);
    btdrv_delay(10);
    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_reg_op_vco_test_stop(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    bt_bes_testmode_set(0);
    btdrv_delay(10);
    BT_DRV_REG_OP_CLK_DIB();
}

uint32_t btdrv_reg_op_syn_get_curr_ticks(void)
{
    uint32_t value;

    BT_DRV_REG_OP_CLK_ENB();
    value = BTDIGITAL_REG(BT_BES_CLK_REG1_ADDR) & 0x0fffffff;
    BT_DRV_REG_OP_CLK_DIB();
    return value;
}

uint32_t btdrv_reg_op_syn_get_cis_curr_time(void)
{
    uint32_t value;

    BT_DRV_REG_OP_CLK_ENB();
    value = bt_isocnt_freerun_get();
    BT_DRV_REG_OP_CLK_DIB();
    return value;
}

void btdrv_syn_clr_trigger(uint8_t trig_route)
{
    BT_DRV_REG_OP_CLK_ENB();
    switch(trig_route)
    {
        case 0:
        {
            BTDIGITAL_REG(BT_BES_CNTL1_ADDR) |= (1<<4);
            break;
        }
        case 1:
        {
            BTDIGITAL_REG(BT_BES_CNTL1_ADDR) |= (1<<8);
            break;
        }
        case 2:
        {
            BTDIGITAL_REG(BT_BES_CNTL1_ADDR) |= (1<<9);
            break;
        }
        case 3:
        {
            BTDIGITAL_REG(BT_BES_CNTL1_ADDR) |= (1<<10);
            break;
        }
    }
    BT_DRV_TRACE(2,"[%s] [%x]=0x%x",__func__,BT_BES_CNTL1_ADDR,*(volatile uint32_t *)(BT_BES_CNTL1_ADDR));
    BT_DRV_REG_OP_CLK_DIB();
}

#ifdef __SW_TRIG__
void btdrv_reg_op_sw_trig_tg_finecnt_set(uint16_t tg_bitcnt, uint8_t trig_route)
{
    BT_DRV_REG_OP_CLK_ENB();
    switch(trig_route)
    {
        case 0:
        {
            //0xc98 bit[9:0]
            BTDIGITAL_REG(BT_BES_TG_FINECNT_ADDR) = (BTDIGITAL_REG(BT_BES_TG_FINECNT_ADDR) & 0xFFFFFC00) | tg_bitcnt;
            BT_DRV_TRACE(1,"[%s] route0=%x, tg_bitcnt=%d\n",__func__,BTDIGITAL_REG(BT_BES_TG_FINECNT_ADDR), tg_bitcnt);
            break;
        }
        case 1:
        {
            //0xd0c bit[9:0]
            BTDIGITAL_REG(BT_BES_TG_FINECNT1_ADDR) = (BTDIGITAL_REG(BT_BES_TG_FINECNT1_ADDR) & 0xFFFFFC00) | tg_bitcnt;
            BT_DRV_TRACE(1,"[%s] route1=%x, tg_bitcnt=%d\n",__func__,BTDIGITAL_REG(BT_BES_TG_FINECNT1_ADDR), tg_bitcnt);
            break;
        }
        case 2:
        {
            //0xd14 bit[9:0]
            BTDIGITAL_REG(BT_BES_TG_FINECNT2_ADDR) = (BTDIGITAL_REG(BT_BES_TG_FINECNT2_ADDR) & 0xFFFFFC00) | tg_bitcnt;
            BT_DRV_TRACE(1,"[%s] route2=%x, tg_bitcnt=%d\n",__func__,BTDIGITAL_REG(BT_BES_TG_FINECNT2_ADDR), tg_bitcnt);
            break;
        }
        case 3:
        {
            //0xd1c bit[9:0]
            BTDIGITAL_REG(BT_BES_TG_FINECNT3_ADDR) = (BTDIGITAL_REG(BT_BES_TG_FINECNT3_ADDR) & 0xFFFFFC00) | tg_bitcnt;
            BT_DRV_TRACE(1,"[%s] route3=%x, tg_bitcnt=%d\n",__func__,BTDIGITAL_REG(BT_BES_TG_FINECNT3_ADDR), tg_bitcnt);
            break;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
}

uint16_t btdrv_reg_op_sw_trig_tg_finecnt_get(uint8_t trig_route)
{
    uint16_t finecnt = 0;
    BT_DRV_REG_OP_CLK_ENB();
    switch(trig_route)
    {
        case 0:
        {
            finecnt = BTDIGITAL_REG(BT_BES_TG_FINECNT_ADDR) & 0x000003FF;
            break;
        }
        case 1:
        {
            finecnt = BTDIGITAL_REG(BT_BES_TG_FINECNT1_ADDR) & 0x000003FF;
            break;
        }
        case 2:
        {
            finecnt = BTDIGITAL_REG(BT_BES_TG_FINECNT2_ADDR) & 0x000003FF;
            break;
        }
        case 3:
        {
            finecnt = BTDIGITAL_REG(BT_BES_TG_FINECNT3_ADDR) & 0x000003FF;
            break;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_TRACE(1,"[%s] finecnt=%d\n",__func__,finecnt);
    return finecnt;
}

void btdrv_reg_op_sw_trig_tg_clkncnt_set(uint32_t num, uint8_t trig_route)
{
    BT_DRV_REG_OP_CLK_ENB();
    switch(trig_route)
    {
        case 0:
        {
            //0xc94 bit[27:0]
            BTDIGITAL_REG(BT_BES_TG_CLKNCNT_ADDR) = (BTDIGITAL_REG(BT_BES_TG_CLKNCNT_ADDR) & 0xf0000000) | (num & 0x0fffffff);
            BT_DRV_TRACE(1,"[%s] route0=0x%x, num=%d\n",__func__,BTDIGITAL_REG(BT_BES_TG_CLKNCNT_ADDR), num);
            break;
        }
        case 1:
        {
            //0xd08 bit[27:0]
            BTDIGITAL_REG(BT_BES_TG_CLKNCNT1_ADDR) = (BTDIGITAL_REG(BT_BES_TG_CLKNCNT1_ADDR) & 0xf0000000) | (num & 0x0fffffff);
            BT_DRV_TRACE(1,"[%s] route1=0x%x, num=%d\n",__func__,BTDIGITAL_REG(BT_BES_TG_CLKNCNT1_ADDR), num);
            break;
        }
        case 2:
        {
            //0xd10 bit[27:0]
            BTDIGITAL_REG(BT_BES_TG_CLKNCNT2_ADDR) = (BTDIGITAL_REG(BT_BES_TG_CLKNCNT2_ADDR) & 0xf0000000) | (num & 0x0fffffff);
            BT_DRV_TRACE(1,"[%s] route2=0x%x, num=%d\n",__func__,BTDIGITAL_REG(BT_BES_TG_CLKNCNT2_ADDR), num);
            break;
        }
        case 3:
        {
            //0xd18 bit[27:0]
            BTDIGITAL_REG(BT_BES_TG_CLKNCNT3_ADDR) = (BTDIGITAL_REG(BT_BES_TG_CLKNCNT3_ADDR) & 0xf0000000) | (num & 0x0fffffff);
            BT_DRV_TRACE(1,"[%s] route3=0x%x, num=%d\n",__func__,BTDIGITAL_REG(BT_BES_TG_CLKNCNT3_ADDR), num);
            break;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_sw_trig_en_set(uint8_t trig_route)
{
    BT_DRV_REG_OP_CLK_ENB();
    switch(trig_route)
    {
        case 0:
        {
            BTDIGITAL_REG(BT_BES_CNTLX_ADDR) |= 0x1;
            BT_DRV_TRACE(1,"[%s] c7c=0x%x, c94=0x%x,c98=0x%x\n",__func__,
                         BTDIGITAL_REG(BT_BES_CNTLX_ADDR),BTDIGITAL_REG(BT_BES_TG_CLKNCNT_ADDR),BTDIGITAL_REG(BT_BES_TG_FINECNT_ADDR));
            break;
        }
        case 1:
        {
            BTDIGITAL_REG(BT_BES_TG_FINECNT1_ADDR) |= (1<<31);
            BT_DRV_TRACE(1,"[%s] d08=0x%x, d0c=0x%x\n",__func__,
                         BTDIGITAL_REG(BT_BES_TG_CLKNCNT1_ADDR),BTDIGITAL_REG(BT_BES_TG_FINECNT1_ADDR));
            break;
        }
        case 2:
        {
            BTDIGITAL_REG(BT_BES_TG_FINECNT2_ADDR) |= (1<<31);
            BT_DRV_TRACE(1,"[%s] d10=0x%x, d14=0x%x\n",__func__,
                         BTDIGITAL_REG(BT_BES_TG_CLKNCNT2_ADDR),BTDIGITAL_REG(BT_BES_TG_FINECNT2_ADDR));
            break;
        }
        case 3:
        {
            BTDIGITAL_REG(BT_BES_TG_FINECNT3_ADDR) |= (1<<31);
            BT_DRV_TRACE(1,"[%s] d18=0x%x, d1c=0x%x\n",__func__,
                         BTDIGITAL_REG(BT_BES_TG_CLKNCNT3_ADDR),BTDIGITAL_REG(BT_BES_TG_FINECNT3_ADDR));
            break;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_sw_trig_disable_set(uint8_t trig_route)
{
    BT_DRV_REG_OP_CLK_ENB();
    switch(trig_route)
    {
        case 0:
        {
            BTDIGITAL_REG(BT_BES_CNTLX_ADDR) &= ~0x1;
            BT_DRV_TRACE(1,"[%s] c7c=0x%x, c94=0x%x, c98=0x%x\n",__func__,
                         BTDIGITAL_REG(BT_BES_CNTLX_ADDR),BTDIGITAL_REG(BT_BES_TG_CLKNCNT_ADDR),BTDIGITAL_REG(BT_BES_TG_FINECNT_ADDR));
            break;
        }
        case 1:
        {
            BTDIGITAL_REG(BT_BES_TG_FINECNT1_ADDR) &= ~(1<<31);
            BT_DRV_TRACE(1,"[%s] d08=0x%x, d0c=0x%x\n",__func__,
                         BTDIGITAL_REG(BT_BES_TG_CLKNCNT1_ADDR),BTDIGITAL_REG(BT_BES_TG_FINECNT1_ADDR));
            break;
        }
        case 2:
        {
            BTDIGITAL_REG(BT_BES_TG_FINECNT2_ADDR) &= ~(1<<31);
            BT_DRV_TRACE(1,"[%s] d10=0x%x, d14=0x%x\n",__func__,
                         BTDIGITAL_REG(BT_BES_TG_CLKNCNT2_ADDR),BTDIGITAL_REG(BT_BES_TG_FINECNT2_ADDR));
            break;
        }
        case 3:
        {
            BTDIGITAL_REG(BT_BES_TG_FINECNT3_ADDR) &= ~(1<<31);
            BT_DRV_TRACE(1,"[%s] d18=0x%x, d1c=0x%x\n",__func__,
                         BTDIGITAL_REG(BT_BES_TG_CLKNCNT3_ADDR),BTDIGITAL_REG(BT_BES_TG_FINECNT3_ADDR));
            break;
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_reg_op_sw_trig_route_extra_set(uint8_t trig_route)
{
    if((trig_route == 1) || (trig_route == 2))
    {
        BTDIGITAL_REG(BT_BES_SYNC_INTR_ADDR) |= (1<<31);
    }
    else if(trig_route == 3)
    {
        BTDIGITAL_REG(BT_BES_SYNC_INTR_ADDR) |= (1<<31);
        BTDIGITAL_REG(BT_BES_TOG_CNTL_ADDR) &= ~(1<<9);
    }
    BT_DRV_TRACE(1,"[%s] trig_route=%d d28=0x%x d70=0x%x\n",__func__,
                trig_route,BTDIGITAL_REG(BT_BES_SYNC_INTR_ADDR),BTDIGITAL_REG(BT_BES_TOG_CNTL_ADDR));
}

#endif

void btdrv_reg_op_syn_set_tg_ticks_master_role(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    BT_DRV_TRACE(2,"primary c84:%x,c7c:%x\n",*(volatile uint32_t *)(BT_TWSBTCLKREG_ADDR),*(volatile uint32_t *)(BT_BES_CNTLX_ADDR));
    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_reg_op_syn_set_tg_ticks_slave_role(uint32_t num)
{
    BT_DRV_REG_OP_CLK_ENB();
    BTDIGITAL_REG(BT_TWSBTCLKREG_ADDR) = (BTDIGITAL_REG(BT_TWSBTCLKREG_ADDR) & 0xf0000000) | (num & 0x0fffffff);
    BTDIGITAL_REG(BT_BES_CNTLX_ADDR) = (BTDIGITAL_REG(BT_BES_CNTLX_ADDR) & 0xffffe0ff) | (0x12<<8);//bit[12:8] trip_delay=18
    BTDIGITAL_REG(BT_BES_CNTLX_ADDR) |= (1<<15);//rxbit sel en
    BTDIGITAL_REG(BT_BES_CNTLX_ADDR) = (BTDIGITAL_REG(BT_BES_CNTLX_ADDR) & 0xfffffffe) | (0x1);
    BTDIGITAL_REG(BT_BES_CNTL1_ADDR) |= (1<<7);
    BT_DRV_TRACE(1,"secondary 0xc84:0x%x,c7c:0x%x,c08:0x%x\n",BTDIGITAL_REG(BT_TWSBTCLKREG_ADDR),BTDIGITAL_REG(BT_BES_CNTLX_ADDR),BTDIGITAL_REG(BT_BES_CNTL1_ADDR));
    BT_DRV_REG_OP_CLK_DIB();
}

uint32_t btdrv_reg_op_get_syn_trigger_codec_en(void)
{
    uint32_t value;
    BT_DRV_REG_OP_CLK_ENB();
    value = BTDIGITAL_REG(BT_BES_CNTLX_ADDR);
    BT_DRV_REG_OP_CLK_DIB();
    return value;
}

uint32_t btdrv_reg_op_get_trigger_ticks(void)
{
    uint32_t value;
    BT_DRV_REG_OP_CLK_ENB();
    value = BTDIGITAL_REG(BT_BES_CNTLX_ADDR);
    BT_DRV_REG_OP_CLK_DIB();
    return value;
}

void btdrv_reg_op_syn_set_tg_ticks_linkid(uint8_t link_id)
{
    BT_DRV_REG_OP_CLK_ENB();
    BTDIGITAL_REG_SET_FIELD(BT_BES_CNTLX_ADDR,0xF,16,(link_id+1));
    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_reg_op_enable_playback_triggler(uint8_t triggle_mode)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(triggle_mode == ACL_TRIGGLE_MODE)
    {
        BTDIGITAL_REG(BT_BES_CNTLX_ADDR) &= ~(1<<4|1<<3);
        BTDIGITAL_REG(BT_BES_CNTLX_ADDR) |= (1<<3);
    }
    else if(triggle_mode == SCO_TRIGGLE_MODE)
    {
        BTDIGITAL_REG(BT_BES_CNTLX_ADDR) &= ~(1<<4|1<<3);
        BTDIGITAL_REG(BT_BES_CNTLX_ADDR) |= (1<<4);
    }
    BT_DRV_TRACE(2,"[%s] c7c=%x",__func__,*(volatile uint32_t *)(BT_BES_CNTLX_ADDR));
    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_reg_op_play_trig_mode_set(uint8_t mode)
{
    BT_DRV_REG_OP_CLK_ENB();
    ///0xc7c bit[14:13] 00:hw trig, 01:sw trig, 1x:self trig
    BTDIGITAL_REG(BT_BES_CNTLX_ADDR) = (BTDIGITAL_REG(BT_BES_CNTLX_ADDR) & 0xFFFF9FFF) | (mode << 13);
    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_reg_op_disable_playback_triggler(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    BTDIGITAL_REG(BT_BES_CNTLX_ADDR) &= ~(1<<4|1<<3);
    BT_DRV_TRACE(2,"[%s] c7c=%x\n",__func__,*(volatile uint32_t *)(BT_BES_CNTLX_ADDR));
    BT_DRV_REG_OP_CLK_DIB();
}

#ifdef __SW_TRIG__

uint16_t Tbit_M_ori_flag = 200;
#define RF_DELAY  18

#define ROUNDDOWN(x)   ((int)(x))
#define ROUNDUP(x)     ((int)(x) + ((x-(int)(x)) > 0 ? 1 : 0))
#define ROUND(x)       ((int)((x) + ((x) > 0 ? 0.5 : -0.5)))

static uint16_t btdrv_Tbit_M_h_get(uint32_t Tclk_M, int16_t Tbit_M_h_ori)
{
    uint16_t Tbit_M_h;
    Tbit_M_h = Tbit_M_h_ori;
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,Tbit_M_h);
    return Tbit_M_h;
}

static uint16_t btdrv_Tbit_M_get(uint32_t Tclk_M, uint16_t Tbit_M_h)
{
    uint16_t Tbit_M;
    if(Tbit_M_h % 2)///if Tbit_M_h is odd
    {
        Tbit_M = (uint16_t)ROUNDUP(Tbit_M_h/2) + 1;
    }
    else
    {
        Tbit_M = (uint16_t)ROUNDUP(Tbit_M_h/2);
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,Tbit_M);
    return Tbit_M;
}

int32_t bt_syn_get_clkoffset(uint16_t conhdl)
{
    int32_t offset;

    if(conhdl>=0x80)
        offset = bt_drv_reg_op_get_clkoffset(conhdl-0x80);
    else
        offset = 0;

    return offset;
}

int32_t btdrv_slotoff_get(uint16_t conhdl,int32_t clkoff)
{
    int32_t slotoff;

    if(clkoff < 0)
    {
        slotoff = -(int32_t)ROUNDDOWN((-clkoff)/2);
    }
    else
    {
        if(clkoff % 2)
        {
            slotoff = (int32_t)ROUNDUP(clkoff/2) + 1;
        }
        else
        {
            slotoff = (int32_t)ROUNDUP(clkoff/2);
        }
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,slotoff);
    return slotoff;
}

int16_t btdrv_bitoff_get(uint16_t rxbit_1us,int32_t clkoff)
{
    int16_t bitoff;

    bitoff = rxbit_1us - 68 - RF_DELAY;
    BT_DRV_TRACE(1,"[%s] bitoff=%d\n",__func__,bitoff);
    if((bitoff < 0) && (clkoff % 2))
    {
        bitoff += 624;
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,bitoff);
    return bitoff;
}


uint8_t btdrv_clk_adj_M_get(uint16_t Tbit_M,int16_t bitoff)
{
    uint8_t clk_adj_M;

    if(bitoff >= Tbit_M)
    {
        clk_adj_M = 1;
    }
    else
    {
        clk_adj_M = 0;
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,clk_adj_M);
    return clk_adj_M;
}

static uint8_t btdrv_clk_adj_S_get(uint8_t clk_adj_M,uint16_t Tbit_M,int16_t bitoff)
{
    uint8_t clk_adj_S;

    if(clk_adj_M > 0)
    {
        int16_t temp = (bitoff - Tbit_M)*2;
        if(temp > 624)
        {
            clk_adj_S = 1;
        }
        else
        {
            clk_adj_S = 0;
        }
    }
    else
    {
        clk_adj_S = 0;
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,clk_adj_S);
    return clk_adj_S;
}

static uint32_t btdrv_Tclk_S_get(uint32_t Tclk_M, uint8_t clk_adj_M,
                                uint8_t clk_adj_S, int32_t slotoff)
{
    uint32_t Tclk_S;
    Tclk_S = Tclk_M - slotoff*2 + clk_adj_M + clk_adj_S;
    Tclk_S &= 0x0fffffff;
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,Tclk_S);
    return Tclk_S;
}

static uint16_t btdrv_Tclk_S_h_get(uint8_t clk_adj_M,uint8_t clk_adj_S,
                                    uint16_t Tbit_M,int16_t bitoff)
{
    uint16_t Tclk_S_h;
    if(clk_adj_M == 0)
    {
        Tclk_S_h = (Tbit_M - bitoff)*2;
    }
    else
    {
        if(clk_adj_S == 1)
        {
            Tclk_S_h = (1248-(bitoff-Tbit_M)*2);
        }
        else
        {
            Tclk_S_h = (624-(bitoff-Tbit_M)*2);
        }
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,Tclk_S_h);
    return Tclk_S_h;
}

static void btdrv_sw_trig_slave_calculate_and_set(uint16_t conhdl, uint32_t Tclk_M, uint16_t Tbit_M_h_ori, uint8_t trig_route)
{
    uint16_t Tbit_M_h;
    uint16_t Tbit_M;
    uint16_t rxbit_1us;
    int16_t bitoff;
    int32_t clkoff;
    int32_t slotoff;
    uint8_t clk_adj_M;
    uint8_t clk_adj_S;
    uint32_t Tclk_S;
    uint32_t Tclk_S_h;

    Tbit_M_h = Tbit_M_ori_flag;
    Tbit_M = btdrv_Tbit_M_get(Tclk_M,Tbit_M_h);
    rxbit_1us = bt_drv_reg_op_rxbit_1us_get(conhdl);
    clkoff = bt_syn_get_clkoffset(conhdl);
    slotoff = btdrv_slotoff_get(conhdl,clkoff);
    bitoff = btdrv_bitoff_get(rxbit_1us,clkoff);
    clk_adj_M = btdrv_clk_adj_M_get(Tbit_M,bitoff);
    clk_adj_S = btdrv_clk_adj_S_get(clk_adj_M,Tbit_M,bitoff);
    Tclk_S = btdrv_Tclk_S_get(Tclk_M,clk_adj_M,clk_adj_S,slotoff);
    Tclk_S_h = btdrv_Tclk_S_h_get(clk_adj_M,clk_adj_S,Tbit_M,bitoff);

    btdrv_reg_op_sw_trig_tg_clkncnt_set(Tclk_S,trig_route);
    btdrv_reg_op_sw_trig_tg_finecnt_set(Tclk_S_h,trig_route);
    btdrv_sw_trig_en_set(trig_route);
    btdrv_reg_op_sw_trig_route_extra_set(trig_route);
}

void btdrv_sw_trig_master_set(uint32_t Tclk_M, uint16_t Tbit_M_h_ori, uint8_t trig_route)
{
    uint16_t Tbit_M_h;

    btdrv_reg_op_sw_trig_tg_clkncnt_set(Tclk_M,trig_route);
    Tbit_M_h = btdrv_Tbit_M_h_get(Tclk_M,Tbit_M_h_ori);
    btdrv_reg_op_sw_trig_tg_finecnt_set(Tbit_M_h,trig_route);
    btdrv_sw_trig_en_set(trig_route);
    btdrv_reg_op_sw_trig_route_extra_set(trig_route);
}
#endif

void bt_syn_cancel_tg_ticks(uint8_t trig_route)
{
#ifdef __SW_TRIG__
    btdrv_sw_trig_disable_set(trig_route);
#endif
}

static void btdrv_syn_set_tg_ticks(uint32_t num, uint8_t mode, uint16_t conhdl, uint8_t trig_route)
{
    BT_DRV_REG_OP_CLK_ENB();

    if (mode == BT_TRIG_MASTER_ROLE)
    {
#ifdef __SW_TRIG__
        BT_DRV_TRACE(1,"[%s] __SW_TRIG__ conhdl=0x%x num=%d trig_route=%d\n",__func__,conhdl,num,trig_route);
        btdrv_sw_trig_master_set(num, Tbit_M_ori_flag, trig_route);
#else
        btdrv_reg_op_syn_set_tg_ticks_master_role();
#endif
    }
    else
    {
#ifdef __SW_TRIG__
        BT_DRV_TRACE(1,"[%s] __SW_TRIG__ conhdl=0x%x num=%d trig_route=%d\n",__func__,conhdl,num,trig_route);
        btdrv_sw_trig_slave_calculate_and_set(conhdl, num, Tbit_M_ori_flag,trig_route);
#else
        btdrv_reg_op_syn_set_tg_ticks_slave_role(num);
#endif
    }

    BT_DRV_REG_OP_CLK_DIB();
}

// Can be used by master or slave
// Ref: Master bt clk
uint32_t bt_syn_get_curr_ticks(uint16_t conhdl)
{
    int32_t curr,offset;

    curr = btdrv_syn_get_curr_ticks();

    if (btdrv_is_link_index_valid(btdrv_conhdl_to_linkid(conhdl)))
        offset = bt_drv_reg_op_get_clkoffset(btdrv_conhdl_to_linkid(conhdl));
    else
        offset = 0;
//    BT_DRV_TRACE(4,"[%s] curr(%d) + offset(%d) = %d", __func__, curr , offset,curr + offset);
    return (curr + offset) & 0x0fffffff;
}
// Ref: Master bt clk
void bt_syn_set_tg_ticks(uint32_t val,uint16_t conhdl, uint8_t mode, uint8_t trig_route, bool no_link_trig)
{
    BT_DRV_REG_OP_CLK_ENB();

    int32_t offset;
    uint8_t link_id = btdrv_conhdl_to_linkid(conhdl);
    if(no_link_trig == false)
    {
        if(HCI_LINK_INDEX_INVALID == link_id)
        {
            BT_DRV_TRACE(3,"%s,ERR INVALID CONHDL 0x%x!! ca=%p",__func__,conhdl, __builtin_return_address(0));
            return;
        }
    }
    if (btdrv_is_link_index_valid(link_id))
    {
        offset = bt_drv_reg_op_get_clkoffset(link_id);
    }
    else
    {
        offset = 0;
    }

#if !defined(__SW_TRIG__)
    btdrv_reg_op_syn_set_tg_ticks_linkid(link_id);
#endif

    if ((mode == BT_TRIG_MASTER_ROLE) && (offset !=0))
    {
        BT_DRV_TRACE(0,"ERROR OFFSET !!");
    }

#if !defined(__SW_TRIG__)
    val = val>>1;
    val = val<<1;
    val += 1;
#endif

    bt_syn_cancel_tg_ticks(trig_route);
    BT_DRV_TRACE(4,"bt_syn_set_tg_ticks val:%d num:%d mode:%d conhdl:%02x", val, val - offset, mode, conhdl);
    btdrv_syn_set_tg_ticks(val, mode, conhdl, trig_route);
    bt_syn_trig_checker(conhdl);

    BT_DRV_REG_OP_CLK_DIB();
}

void bt_syn_ble_set_tg_ticks(uint32_t val,uint8_t trig_route)
{
#ifdef __SW_TRIG__
    BT_DRV_REG_OP_CLK_ENB();

    uint32_t clkncnt = 0;
    uint16_t finecnt = 0;

    btdrv_syn_clr_trigger(trig_route);
    btdrv_reg_op_bts_to_bt_time(val, &clkncnt, &finecnt);

    btdrv_reg_op_sw_trig_tg_clkncnt_set(clkncnt,trig_route);
    btdrv_reg_op_sw_trig_tg_finecnt_set(finecnt,trig_route);
    btdrv_sw_trig_en_set(trig_route);
    btdrv_reg_op_sw_trig_route_extra_set(trig_route);

    BT_DRV_REG_OP_CLK_DIB();
#endif
}

#ifdef PCM_FAST_MODE
void btdrv_reg_op_open_pcm_fast_mode_enable(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    BT_DRV_TRACE(0,"pcm fast mode %x,%x\n",BTDIGITAL_REG(BT_BES_PCMCNTL_ADDR),BTDIGITAL_REG(BT_PCMPHYSCNTL1_ADDR));
    uint8_t esco_len;
    uint32_t val = 0xf00000cf;
    BTDIGITAL_REG_GET_FIELD(BT_E_SCOTRCNTL_ADDR,0x3FF,4,esco_len);
    val |= ((esco_len-1)<<8 | 1<<15);
    BTDIGITAL_REG_WR(BT_BES_PCMCNTL_ADDR,val);
    BTDIGITAL_REG_SET_FIELD(BT_PCMPHYSCNTL1_ADDR, 0x1FF, 0, 8);
    BTDIGITAL_REG_SET_FIELD(BT_PCMPHYSCNTL1_ADDR, 1, 31, 0);
    BT_DRV_TRACE(0,"pcm fast mode config done %x,%x esco_len %d\n",BTDIGITAL_REG(BT_BES_PCMCNTL_ADDR),BTDIGITAL_REG(BT_PCMPHYSCNTL1_ADDR),esco_len);
    BT_DRV_REG_OP_CLK_DIB();
}
void btdrv_reg_op_open_pcm_fast_mode_disable(void)
{
}
#endif

#if defined(CVSD_BYPASS)
void btdrv_reg_op_cvsd_bypass_enable(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    BTDIGITAL_REG(BT_E_SCOMUTECNTL_0_ADDR) |= 0x5555;
    BTDIGITAL_REG(BT_E_SCOMUTECNTL_1_ADDR) |= 0x5555;
#ifdef PCM_PRIVATE_DATA_FLAG
    BTDIGITAL_REG_SET_FIELD(BT_E_SCOMUTECNTL_0_ADDR, 1, 23, 0);
    BTDIGITAL_REG_SET_FIELD(BT_E_SCOMUTECNTL_1_ADDR, 1, 23, 0);
#endif
    BTDIGITAL_REG(BT_AUDIOCNTL0_ADDR) &= ~(1<<7); //soft cvsd
    BTDIGITAL_REG(BT_AUDIOCNTL1_ADDR) &= ~(1<<7); //soft cvsd

    BT_DRV_REG_OP_CLK_DIB();
}

#endif

int8_t btdrv_reg_op_txpwr_idx_to_rssidbm(uint8_t txpwr_idx)
{
    if(txpwr_idx > BT_MAX_TX_PWR_IDX)
    {
        txpwr_idx = BT_MAX_TX_PWR_IDX;
    }

    return btdrv_txpwr_conv_tbl[txpwr_idx];
}

void bt_drv_reg_op_ble_rx_gain_thr_tbl_set(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_ths_tbl_le_addr)
    {
        int sRet = 0;
        sRet = memcpy_s((uint8_t *)rf_rx_gain_ths_tbl_le_addr,
                            sizeof(btdrv_rxgain_ths_tbl_le),
                            btdrv_rxgain_ths_tbl_le,
                            sizeof(btdrv_rxgain_ths_tbl_le));
        if (sRet)
        {
            TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
}

void bt_drv_reg_op_ble_rx_gain_thr_tbl_2m_set(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_ths_tbl_le_2m_addr)
    {
        int sRet = 0;
        sRet = memcpy_s((uint8_t *)rf_rx_gain_ths_tbl_le_2m_addr,
                            sizeof(btdrv_rxgain_ths_tbl_le_2m),
                            btdrv_rxgain_ths_tbl_le_2m,
                            sizeof(btdrv_rxgain_ths_tbl_le_2m));
        if (sRet)
        {
            TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
}

void bt_drv_reg_op_ecc_rx_gain_thr_tbl_set(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(rf_rx_gain_ths_tbl_ecc_addr)
    {
        int sRet = 0;
        sRet = memcpy_s((uint8_t *)rf_rx_gain_ths_tbl_ecc_addr,
                            sizeof(btdrv_rxgain_ths_tbl_ecc),
                            btdrv_rxgain_ths_tbl_ecc,
                            sizeof(btdrv_rxgain_ths_tbl_ecc));
        if (sRet)
        {
            TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
}

void bt_drv_reg_op_txpwr_conv_tbl_set(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    if(btdrv_txpwr_conv_tbl_addr)
    {
        int sRet = 0;
        sRet = memcpy_s((uint8_t *)btdrv_txpwr_conv_tbl_addr,
                            sizeof(btdrv_txpwr_conv_tbl),
                            (btdrv_txpwr_conv_tbl),
                            sizeof((btdrv_txpwr_conv_tbl)));
        if (sRet)
        {
            TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }
    //1503 rom t0 has no BLE independent conv table
    if(btdrv_ble_txpwr_conv_tbl_addr)
    {
        int sRet = 0;
        sRet = memcpy_s((uint8_t *)btdrv_ble_txpwr_conv_tbl_addr,
                            sizeof(btdrv_ble_txpwr_conv_tbl),
                            (btdrv_ble_txpwr_conv_tbl),
                            sizeof((btdrv_ble_txpwr_conv_tbl)));
        if (sRet)
        {
            TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
}

void btdrv_reg_op_bts_to_bt_time(uint32_t bts, uint32_t* p_hs, uint16_t* p_hus)
{
    BT_DRV_REG_OP_CLK_ENB();
    /// Integer part of the time (in half-slot)
    uint32_t last_hs = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_0_val);
    static uint32_t record_last_hs = 0;
    /// Fractional part of the time (in half-us) (range: 0-624)
    uint16_t last_hus = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_1_val);
    static uint16_t record_last_hus = 0;
    /// Bluetooth timestamp value (in us) 32 bits counter
    uint32_t last_bts = *(volatile uint32_t *)(rwip_env_addr + off_lust_in_ret_rwip_val);
    static uint32_t record_last_bts = 0;
    /// the times that record bt time
    uint8_t record_time = 0;
    static bool bt_time_recorded = false;
    uint8_t i=0;
    static uint32_t bt2bts_systick;

    uint32_t current_systick = hal_sys_timer_get();

    if (TICKS_TO_MS(current_systick - bt2bts_systick) > 15 * 60000)
    {
        bt_time_recorded = false;
    }
    else if (last_bts - record_last_bts > 15 * 60000)
    {
        bt_time_recorded = false;
    }

    if (false == bt_time_recorded)
    {
        record_last_hs = last_hs;
        record_last_hus = last_hus;
        record_last_bts = last_bts;
        for (i=0; i<100; i++)
        {
            last_hs = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_0_val);
            last_hus = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_1_val);
            last_bts = *(volatile uint32_t *)(rwip_env_addr + off_lust_in_ret_rwip_val);

            if ((record_last_hs == last_hs) &&
                (record_last_hus == last_hus) &&
                (record_last_bts == last_bts))
            {
                record_time++;
                if (record_time >= 3)
                {
                    bt_time_recorded = true;
                    break;
                }
            }
            else
            {
                record_last_hs = last_hs;
                record_last_hus = last_hus;
                record_last_bts = last_bts;
                last_hs = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_0_val);
                last_hus = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_1_val);
                last_bts = *(volatile uint32_t *)(rwip_env_addr + off_lust_in_ret_rwip_val);
                record_time = 0;
            }
        }
        ASSERT(i<100, "%s i %d record %d", __func__, i, record_time);
    }

    // compute time in half microseconds between last sampled time and BT time provided in parameters
    int32_t clk_diff_hus = CLK_DIFF(record_last_hs, last_hs) * HALF_SLOT_SIZE;
    clk_diff_hus        += (int32_t)last_hus - (int32_t)record_last_hus;
    if (last_bts == (record_last_bts + (clk_diff_hus >> 1)))
    {
        record_last_hs = last_hs;
        record_last_hus = last_hus;
        record_last_bts = last_bts;
    }

    clk_diff_hus = 0;
    int32_t clk_diff_us = (int32_t) (bts - record_last_bts);

    //*p_hus = HALF_SLOT_SIZE - last_hus - 1;
    *p_hus = record_last_hus;
    *p_hs  = record_last_hs;

    // check if clock difference is positive or negative
    if(clk_diff_us > 0)
    {
        clk_diff_hus = (clk_diff_us << 1);
        *p_hus += CO_MOD(clk_diff_hus, HALF_SLOT_SIZE);
        *p_hs  += (clk_diff_hus / HALF_SLOT_SIZE);

        if(*p_hus >= HALF_SLOT_SIZE)
        {
            *p_hus -= HALF_SLOT_SIZE;
            *p_hs  += 1;
        }
        *p_hus = HALF_SLOT_SIZE - *p_hus - 1;
    }
    else
    {
        uint32_t diff_hus;
        clk_diff_hus = (record_last_bts - bts) << 1;
        diff_hus = CO_MOD(clk_diff_hus, HALF_SLOT_SIZE);

        if(*p_hus < diff_hus)
        {
            *p_hus += HALF_SLOT_SIZE;
            *p_hs  -= 1;
        }

        *p_hus -= diff_hus;
        *p_hus = HALF_SLOT_SIZE - *p_hus - 1;
        *p_hs  -= (clk_diff_hus / HALF_SLOT_SIZE);
    }

    // wrap clock
    *p_hs = (*p_hs & RWIP_MAX_CLOCK_TIME);

    BT_DRV_REG_OP_CLK_DIB();
}

uint32_t btdrv_reg_op_bt_time_to_bts(uint32_t hs, uint16_t hus)
{
    uint32_t bts;

    BT_DRV_REG_OP_CLK_ENB();
    /// Integer part of the time (in half-slot)
    uint32_t last_hs = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_0_val);
    static uint32_t record_last_hs = 0;
    /// Fractional part of the time (in half-us) (range: 0-624)
    uint16_t last_hus = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_1_val);
    static uint16_t record_last_hus = 0;
    /// Bluetooth timestamp value (in us) 32 bits counter
    uint32_t last_bts = *(volatile uint32_t *)(rwip_env_addr + off_lust_in_ret_rwip_val);
    static uint32_t record_last_bts = 0;
    /// the times that record bt time
    uint8_t record_time = 0;
    static bool bt_time_recorded = false;
    uint8_t i=0;
    static uint32_t systick;

    uint32_t current_systick = hal_sys_timer_get();

    if (TICKS_TO_MS(current_systick - systick) > 15 * 60000)
    {
        bt_time_recorded = false;
    }
    else if (CLK_DIFF(record_last_hs, hs) > 3 * 15 * 60000)
    {
        bt_time_recorded = false;
    }

    if (false == bt_time_recorded)
    {
        record_last_hs = last_hs;
        record_last_hus = last_hus;
        record_last_bts = last_bts;
        for (i=0; i<150; i++)
        {
            last_hs = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_0_val);
            last_hus = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_1_val);
            last_bts = *(volatile uint32_t *)(rwip_env_addr + off_lust_in_ret_rwip_val);

            if ((record_last_hs == last_hs) &&
                (record_last_hus == last_hus) &&
                (record_last_bts == last_bts))
            {
                record_time++;
                if (record_time >= 2)
                {
                    bt_time_recorded = true;
                    break;
                }
            }
            else
            {
                record_last_hs = last_hs;
                record_last_hus = last_hus;
                record_last_bts = last_bts;
                last_hs = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_0_val);
                last_hus = *(volatile uint32_t *)(rwip_env_addr + off_h_in_ret_rwip_1_val);
                last_bts = *(volatile uint32_t *)(rwip_env_addr + off_lust_in_ret_rwip_val);
                record_time = 0;
            }
        }
        ASSERT(i<150, "%s i %d record %d", __func__, i, record_time);
    }

    // compute time in half microseconds between last sampled time and BT time provided in parameters
    int32_t clk_diff_hus = CLK_DIFF(record_last_hs, last_hs) * HALF_SLOT_SIZE;
    clk_diff_hus        += (int32_t)last_hus - (int32_t)record_last_hus;
    if (last_bts == (record_last_bts + (clk_diff_hus >> 1)))
    {
        record_last_hs = last_hs;
        record_last_hus = last_hus;
        record_last_bts = last_bts;
        systick = current_systick;
    }

    // compute time in half microseconds between last record sampled time and BT time provided in parameters
    clk_diff_hus = CLK_DIFF(record_last_hs, hs) * HALF_SLOT_SIZE;
    clk_diff_hus        += (int32_t)(HALF_SLOT_SIZE - hus + 1) - (int32_t)record_last_hus;

    bts = record_last_bts + (clk_diff_hus >> 1);

    BT_DRV_REG_OP_CLK_DIB();

    // compute BTS time in microseconds
    return bts;
}

void bt_drv_reg_op_le_pwr_ctrl_feats_disable(void)
{
    if(llm_local_le_feats_addr)
    {
        BT_DRV_REG_OP_ENTER();
        BT_DRV_REG_OP_CLK_ENB();
        //close BLE_FEATURES_BYTE4 for ss
        *(uint8_t *) (llm_local_le_feats_addr + 4) = 0;
        BT_DRV_REG_OP_CLK_DIB();
        BT_DRV_REG_OP_EXIT();
    }
}

void bt_drv_reg_op_le_ext_adv_txpwr_independent(uint8_t en)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(le_ext_adv_tx_pwr_independent_addr)
    {
        if(en)
        {
            bt_drv_reg_op_le_pwr_ctrl_feats_disable();
        }


        *(uint8_t *) le_ext_adv_tx_pwr_independent_addr = en;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_le_rf_reg_txpwr_set(uint16_t rf_reg, uint16_t val)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    bt_drv_ble_rf_reg_txpwr_set(rf_reg, val);
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

int32_t bt_drv_reg_op_get_clkoffset(uint16_t linkid)
{
    uint32_t clkoff = 0;
    uint32_t acl_evt_ptr = 0x0;
    int32_t offset;
    uint32_t local_offset;

    BT_DRV_REG_OP_CLK_ENB();
    if(ld_acl_env_addr)
    {
        acl_evt_ptr = *(uint32_t *)(ld_acl_env_addr+linkid*4);
    }

    if (acl_evt_ptr != 0)
    {
        //[last_sync_clk_off]=0xa8
        clkoff = *(uint32_t *)(acl_evt_ptr + off_lsco_in_laet_ld_val);
    }
    else
    {
        BT_DRV_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR RD clkoff %x", linkid);
    }

    BT_DRV_REG_OP_CLK_DIB();
    local_offset = clkoff & 0x0fffffff;
    offset = local_offset;
    offset = (offset << 4)>>4;
   return offset;
}

void btdrv_regop_dump_btpcm_reg(void)
{
    BT_DRV_REG_OP_CLK_ENB();
    BT_DRV_TRACE(2,"btpcm reg = %x,%x\n",BTDIGITAL_REG(BT_BES_PCMCNTL_ADDR),BTDIGITAL_REG(BT_PCMPHYSCNTL1_ADDR));
    BT_DRV_REG_OP_CLK_DIB();
}


void btdrv_regop_host_set_txpwr(uint16_t acl_connhdl, uint8_t txpwr_idx)
{
    static uint8_t txpwr_buf[4] = {0, 0, 0, 0,};

    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(txpwr_val_buf_addr)
    {
        uint8_t* p_txpwr_buf = (uint8_t*)txpwr_val_buf_addr;
        uint8_t link_id = btdrv_conhdl_to_linkid(acl_connhdl);

        txpwr_buf[link_id] = txpwr_idx;
        memcpy(p_txpwr_buf, txpwr_buf, 4);
    }

    btdrv_regop_set_txpwr_flg(1);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}


void btdrv_regop_set_txpwr_flg(uint32_t flg)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(txpwr_host_set_flg_addr)
    {
        *(volatile uint32_t*)txpwr_host_set_flg_addr = flg;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

uint32_t btdrv_reg_op_cig_anchor_timestamp(uint8_t link_id)
{
    uint32_t cis_addr = 0;
    uint32_t anchor_hs = 0, cig_timestamp = 0, cig_addr = 0;
    uint16_t anchor_hus = 0, iso_interval = 0;
    uint8_t cig_hdl;

    if(lld_cis_env_addr && lld_cig_env_addr && (link_id < BLE_ACTIVITY_MAX))
    {
        BT_DRV_REG_OP_ENTER();
        BT_DRV_REG_OP_CLK_ENB();

        cis_addr = *(uint32_t *)(lld_cis_env_addr + link_id * 4);

        if(cis_addr)
        {
            cig_hdl = *(uint8_t *)(cis_addr + off_gh_in_lce_lld_val);

            cig_addr = *(uint32_t *)(lld_cig_env_addr + cig_hdl * 4);

            if(cig_addr)
            {
                anchor_hs = *(uint32_t *)(cig_addr + off_ah_in_lce_lld_0_val);
                anchor_hus = *(uint16_t *)(cig_addr + off_ah_in_lce_lld_1_val);
                iso_interval = *(uint16_t *)(cig_addr + off_ii_in_lce_lld_val);
            }
        }
        if (anchor_hs < btdrv_reg_op_syn_get_curr_ticks())
        {
            anchor_hs += iso_interval;
        }
        BT_DRV_REG_OP_CLK_DIB();
        BT_DRV_REG_OP_EXIT();
    }

    if(cig_addr)
    {
        cig_timestamp = btdrv_reg_op_bt_time_to_bts(anchor_hs, anchor_hus);
    }

    BT_DRV_TRACE(0,"BT_REG_OP:link %d,cis_cig_addr:0x%x,0x%x timestamp:%u\n",link_id, cis_addr, cig_addr, cig_timestamp);
    return cig_timestamp;
}

//todo
uint32_t btdrv_reg_op_big_anchor_timestamp(uint8_t link_id)
{
    uint32_t bis_addr = 0;
    uint32_t anchor_hs = 0, big_timestamp = 0, big_addr = 0;
    uint16_t anchor_hus = 0, iso_interval = 0;
    uint8_t big_hdl;

    if(lld_bis_env_addr && lld_big_env_addr && (link_id < BLE_ACTIVITY_MAX))
    {
        BT_DRV_REG_OP_ENTER();
        BT_DRV_REG_OP_CLK_ENB();

        bis_addr = *(uint32_t *)(lld_bis_env_addr + link_id * 4);

        if(bis_addr)
        {
            big_hdl = *(uint8_t *)(bis_addr + off_gh_in_lbe_lld_val);

            big_addr = *(uint32_t *)(lld_big_env_addr + big_hdl * 4);

            if(big_addr)
            {
                anchor_hs = *(uint32_t *)(big_addr + off_ah_in_lbe_lld_0_val);
                anchor_hus = *(uint16_t *)(big_addr + off_ah_in_lbe_lld_1_val);
                iso_interval = *(uint16_t *)(big_addr + off_ii_in_lbe_lld_val);
            }
        }
        if (anchor_hs < btdrv_reg_op_syn_get_curr_ticks())
        {
            anchor_hs += iso_interval;
        }
        BT_DRV_REG_OP_CLK_DIB();
        BT_DRV_REG_OP_EXIT();
    }

    if(big_addr)
    {
        big_timestamp = btdrv_reg_op_bt_time_to_bts(anchor_hs, anchor_hus);
    }

    BT_DRV_TRACE(0,"BT_REG_OP:link %d,cis_cig_addr:0x%x,0x%x timestamp:%u\n",link_id, bis_addr, big_addr, big_timestamp);
    return big_timestamp;
}

void bt_drv_reg_op_low_txpwr(uint8_t enable, uint8_t factor, uint8_t bt_or_ble, uint8_t link_id)
{
    BT_DRV_TRACE(3,"BT_REG_OP:low txpwr: %u %u ,link_id:%u", enable, bt_or_ble,link_id);

    struct hci_dbg_set_txpwr_mode_cmd hci_param;

    hci_param.enable = enable;
    hci_param.factor = factor;
    hci_param.bt_or_ble = bt_or_ble;
    hci_param.link_id = link_id;

    btdrv_send_hci_cmd_bystack(HCI_DBG_SET_TXPWR_MODE_CMD_OPCODE, (uint8_t *)&hci_param, sizeof(struct hci_dbg_set_txpwr_mode_cmd));

}

void bt_drv_reg_op_get_pkt_ts_rssi(uint16_t connhdl, CLKN_SER_NUM_T* buf)
{
}

bool bt_drv_reg_op_read_fa_rssi_in_dbm(rx_agc_t* rx_val)
{
    bool ret = false;
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(ecc_rx_monitor_addr && rx_val != NULL)
    {
        *rx_val = *(rx_agc_t*)ecc_rx_monitor_addr;
        ret = true;
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
    return ret;
}

uint8_t bt_drv_reg_op_get_max_acl_nb(void)
{
    return MAX_NB_ACTIVE_ACL;
}

void bt_drv_reg_op_sel_btc_intersys(bool sel_sys_flag)
{
    /*
    * NO need for clock gating protection,
    * because this function can only be called before HCI reset
    */
    if(sens2bt_en_addr)
    {
        *(volatile uint8_t *)(sens2bt_en_addr) = sel_sys_flag;
    }
}

#ifdef __NEW_SWAGC_MODE__
void btdrv_regop_rx_gain_ths_3m_init(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(rf_rx_gain_ths_tbl_3m_addr)
    {
        memcpy((int8_t *)rf_rx_gain_ths_tbl_3m_addr, btdrv_rxgain_gain_ths_3m, 30);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}
#endif

uint8_t bt_drv_reg_op_get_slot_num_by_type(uint8_t type)
{
    uint8_t slot_number = 0;
    //only for EDR en
    switch(type)
    {
        case ID_NUL_TYPE:
        case POLL_TYPE:
        case FHS_TYPE:
        case DM1_TYPE:
        case DH1_2_TYPE:
        case HV1_TYPE:
        case EV3_2_TYPE:
        case DH1_3_TYPE:
        case EV3_3_TYPE:
        case AUX1_TYPE:
            slot_number = 1;
        break;
        case EV5_2_TYPE:
        case EV5_3_TYPE:
        case DH3_2_TYPE:
        case DH3_3_TYPE:
            slot_number = 3;
        break;
        case DH5_2_TYPE:
        case DH5_3_TYPE:
            slot_number = 5;
        default:
        break;
    }
    return slot_number;
}

void bt_drv_reg_op_dump_rx_record(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if(rx_record_addr)
    {
        struct ld_rx_status_record * rx_status_ptr = (struct ld_rx_status_record *)rx_record_addr;
        uint32_t current_clk = btdrv_reg_op_syn_get_curr_ticks();
        BT_DRV_TRACE(2, "%s, Current CLK=0x%x",__func__, current_clk);
        for(uint8_t i=0; i < RX_RECORD_NUMBER; i++)
        {
            if (rx_status_ptr[i].link_id < MAX_NB_ACTIVE_ACL)
            {
                uint32_t clk_offset = bt_drv_reg_op_get_clkoffset(rx_status_ptr[i].link_id);
                uint32_t refs_clk = CLK_ADD_2(rx_status_ptr[i].clock, clk_offset);
                if(((rx_status_ptr[i].rxstat) & EM_BT_RXSYNCERR_BIT) == 0)
                {
                    uint8_t slot_nb = bt_drv_reg_op_get_slot_num_by_type(rx_status_ptr[i].type);
                    refs_clk = CLK_SUB(refs_clk, 2*slot_nb);
                }

                BT_DRV_TRACE(9,"id=%d,clkn=0x%x,refs=0x%x,chanl=%d,snr=%d,gain=%d,sta=0x%0x,type=%x,fa=%d",
                    rx_status_ptr[i].link_id,
                    rx_status_ptr[i].clock,
                    refs_clk,
                    rx_status_ptr[i].rxchannel,
                    rx_status_ptr[i].snr,
                    rx_status_ptr[i].rx_gain,
                    rx_status_ptr[i].rxstat,
                    rx_status_ptr[i].type,
                    rx_status_ptr[i].fa_ok);
            }
        }
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_set_sleep_duration(uint32_t tws_slp_dura_dec, uint32_t normal_slp_dura_inc)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if( hal_get_chip_metal_id()>= HAL_CHIP_METAL_ID_6 && dbg_bt_common_setting_t2_addr)
    {
        *(uint8_t *)(dbg_bt_common_setting_t2_addr+off_tsdd_in_dbcst_dbg_t2_val) = tws_slp_dura_dec;
        *(uint8_t *)(dbg_bt_common_setting_t2_addr+off_nsdi_in_dbcst_dbg_t2_val) = normal_slp_dura_inc;
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_regop_disable_le_slave_latency(uint8_t idx)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(con_latency_apply_addr)
    {
        uint8_t bit_temp = 0xff;
        BTDIGITAL_REG_GET_FIELD(con_latency_apply_addr, 1, idx, bit_temp);
        if(bit_temp == 1)
        {
            BTDIGITAL_REG_SET_FIELD(con_latency_apply_addr, 1, idx, 0);
            BT_DRV_TRACE(1,"BT_REG_OP:disable LE slave latency idx=%d, flag=0x%x",
                idx, BTDIGITAL_REG(con_latency_apply_addr));
        }
        else
        {
            BT_DRV_TRACE(1,"BT_REG_OP:warning error latency flag idx=%d, flag=0x%x",
                idx, BTDIGITAL_REG(con_latency_apply_addr));
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_regop_clear_le_slave_latency_flag(uint8_t idx)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    if(con_latency_apply_addr)
    {
        uint8_t bit_temp = 0xff;
        BTDIGITAL_REG_GET_FIELD(con_latency_apply_addr, 1, idx, bit_temp);
        if(bit_temp == 0)
        {
            BTDIGITAL_REG_SET_FIELD(con_latency_apply_addr, 1, idx, 1);
            BT_DRV_TRACE(1,"BT_REG_OP:clear LE slave latency idx=%d, flag=0x%x",
                idx, BTDIGITAL_REG(con_latency_apply_addr));
        }
    }
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_regop_tx_power_dig_comp(const uint16_t dig_gain)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    besmdm_int_dig_gain0_pack(dig_gain, dig_gain, dig_gain);
    besmdm_int_dig_gain1_pack(dig_gain, dig_gain, dig_gain);
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_regop_temperature_calib_get_initial_val(uint16_t *tmx_gain_initial, uint16_t *pa_current_initial, uint16_t *dig_gain_initial)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    BTRF_REG_GET_FIELD(0x191, 0xF, 0, *tmx_gain_initial);
    BTRF_REG_GET_FIELD(0x191, 0x1F, 4, *pa_current_initial);
    BTDIGITAL_REG_GET_FIELD(BESMDM_INT_DIG_GAIN0_ADDR, 0x3FF, 0, *dig_gain_initial);
    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void btdrv_regop_temp_calib_set_calib_value_v1(uint16_t tmx_gain_final, uint16_t pa_current_final)
{
    BTRF_REG_SET_FIELD(0x191, 0xF, 0, tmx_gain_final);
    BTRF_REG_SET_FIELD(0x191, 0x1F, 4, pa_current_final);
}

void btdrv_regop_temp_calib_get_tx_gain_table_value_v2(uint16_t *bt_pa_current, uint16_t *ble_pa_current)
{
    const uint16_t bt_gain_table_addr = 0x18E;
    const uint16_t ble_gain_table_addr = 0x196;

    for (uint8_t i = 0; i < 8; i++) {
        BTRF_REG_GET_FIELD(bt_gain_table_addr + i, 0x1F, 4, bt_pa_current[i]);
        BTRF_REG_GET_FIELD(ble_gain_table_addr + i, 0x1F, 4, ble_pa_current[i]);
    }
}

void btdrv_regop_temp_calib_set_tx_gain_table_value_v2(uint8_t i, uint16_t bt_pa_current_final, uint16_t ble_pa_current_final)
{
    const uint16_t bt_gain_table_addr = 0x18E;
    const uint16_t ble_gain_table_addr = 0x196;

    BTRF_REG_SET_FIELD(bt_gain_table_addr + i, 0x1F, 4, bt_pa_current_final);
    BTRF_REG_SET_FIELD(ble_gain_table_addr + i, 0x1F, 4, ble_pa_current_final);
}

void btdrv_regop_set_btc_srand_seed_initial(void)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    if( hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_6 && host_set_srand_seed_addr)
    {
        uint32_t seed = hal_sys_timer_get() * hal_fast_sys_timer_get();
        *(uint32_t *)(host_set_srand_seed_addr) = seed;
         BT_DRV_TRACE(2,"%s,%d", __func__,seed);
    }

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_config_tport_type(uint32_t tport_type, uint8_t iomux_type)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();

    BTDIGITAL_REG(IP_DIAGCNTL_ADDR) = tport_type;

    BT_DRV_TRACE(1,"%s:0x%x,iomux_type=%d", __func__, tport_type, iomux_type);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_read_register(uint32_t addr, uint32_t* reg_val)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    uint32_t bt_addr = addr;
    //TODO add address valid check
    *reg_val = BTDIGITAL_REG(bt_addr);

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_write_register(uint32_t addr, uint32_t reg_val)
{
    BT_DRV_REG_OP_ENTER();
    BT_DRV_REG_OP_CLK_ENB();
    uint32_t bt_addr = addr;
    //TODO add address valid check
    BTDIGITAL_REG(bt_addr) = reg_val ;

    BT_DRV_REG_OP_CLK_DIB();
    BT_DRV_REG_OP_EXIT();
}

void bt_drv_reg_op_init(void)
{
    BT_DRV_TRACE(1,"%s", __func__);

    bt_drv_reg_op_ble_rx_gain_thr_tbl_set();

    bt_drv_reg_op_ble_rx_gain_thr_tbl_2m_set();

    bt_drv_reg_op_ecc_rx_gain_thr_tbl_set();

    bt_drv_reg_op_txpwr_conv_tbl_set();

    bt_drv_reg_op_init_rssi_setting();

    bt_drv_reg_op_ble_audio_config_init();

#ifdef __NEW_SWAGC_MODE__
    btdrv_regop_rx_gain_ths_3m_init();
#endif

#ifdef PCM_PRIVATE_DATA_FLAG
    bt_drv_reg_op_sco_pri_data_init();
#endif

#ifdef __EBQ_TEST__
    bt_drv_reg_op_ebq_test_setting();
#endif

#ifdef __SW_TRIG__
    btdrv_reg_op_play_trig_mode_set(1);
#endif
}
