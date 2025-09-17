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
#ifndef __BT_DRV_1503_INTERNAL_H__
#define __BT_DRV_1503_INTERNAL_H__


#define BT_PATCH_WR(addr, value)         (*(volatile uint32_t *)(addr)) = (value)

/// Macro to read a BT register
#define REG_BT_RD(addr)              (*(volatile uint32_t *)(addr))

/// Macro to write a BT register
#define REG_BT_WR(addr, value)       (*(volatile uint32_t *)(addr)) = (value)

/// Macro to read a platform register
#define REG_PL_RD(addr)              (*(volatile uint32_t *)(addr))

/// Macro to write a platform register
#define REG_PL_WR(addr, value)       (*(volatile uint32_t *)(addr)) = (value)


//EM size extend
#define EM_SHIFT_EXT  1

/**************************************
HCI cmd define
****************************/
#define  HCI_RD_LOCAL_VER_INFO_CMD_OPCODE                   0x1001

//set bt and ble addr
#define HCI_DBG_SET_BD_ADDR_CMD_OPCODE                      0xFC32

//set clk drift and jitter
#define HCI_DBG_SET_LPCLK_DRIFT_JITTER_CMD_OPCODE           0xFC44

//set sleep enable and external wakeup enable
#define HCI_DBG_SET_SLEEP_EXWAKEUP_EN_CMD_OPCODE            0xFC47

//set edr threshold
#define HCI_DBG_SET_EDR_THRESHOLD_CMD_OPCODE                0xFC4C

//set edr alorithm
#define HCI_DBG_SET_EDR_ALGORITHM_CMD_OPCODE                0xFC4E

//set ble rl size
#define HCI_DBG_SET_RL_SIZE_CMD_OPCODE                      0xFC5D

//set exernal wake up time oscillater wakeup time and radio wakeup time
#define HCI_DBG_SET_WAKEUP_TIME_CMD_OPCODE                  0xFC71

//set pcm setting
#define HCI_DBG_SET_PCM_SETTING_CMD_OPCODE                  0xFC74

//set tx pwr ctrl rssi thd
#define HCI_DBG_SET_RSSI_THRHLD_CMD_OPCODE                  0xFC76

//set sleep setting
#define HCI_DBG_SET_SLEEP_SETTING_CMD_OPCODE                0xFC77

//set local feature
#define HCI_DBG_SET_LOCAL_FEATURE_CMD_OPCODE                0xFC81

//set local extend feature
#define HCI_DBG_SET_LOCAL_EX_FEATURE_CMD_OPCODE             0xFC82

//set bt rf timing
#define HCI_DBG_SET_BT_RF_TIMING_CMD_OPCODE                 0xFC83

//set ble rf timing
#define HCI_DBG_SET_BLE_RF_TIMING_CMD_OPCODE                0xFC84

//bt setting interface
#define HCI_DBG_SET_BT_SETTING_CMD_OPCODE                   0xFC86

//set customer param
#define HCI_DBG_SET_CUSTOM_PARAM_CMD_OPCODE                 0xFC88

//switch sco path
#define HCI_DBG_SET_SCO_SWITCH_CMD_OPCODE                   0xFC89

//set sco path
#define HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE                  0xFC8F

//lmp message record
#define HCI_DBG_LMP_MESSAGE_RECORD_CMD_OPCODE               0xFC9C

//normal sync pos set
#define HCI_DBG_SET_NORMAL_SYNC_POS_CMD_OPCODE              0xFCA5

//bt setting interface
#define HCI_DBG_SET_BT_SETTING_EXT1_CMD_OPCODE              0xFCAE

//set tx pwr ctrl rssi thd
#define HCI_DBG_SET_RSSI_TX_POWER_DFT_THR_CMD_OPCODE        0xFCB3

//flush patch
#define HCI_DBG_SET_FUNC_PATCH_CMD_OPCODE                   0xFCB8

//set tx pwr mode
#define HCI_DBG_SET_TXPWR_MODE_CMD_OPCODE                   0xFCC0

#define HCI_DBG_SET_SW_RSSI_CMD_OPCODE                      0xFCC2


//bt setting interface
#define HCI_DBG_SET_BT_SCHE_SETTING_CMD_OPCODE              0xFCC3

//bt setting interface
#define HCI_DBG_SET_BT_IBRT_SETTING_CMD_OPCODE              0xFCC4

//bt setting interface
#define HCI_DBG_SET_BT_HW_FEAT_SETTING_CMD_OPCODE           0xFCC5

//bt setting interface
#define HCI_DBG_BT_COMMON_SETTING_T1_CMD_OPCODE             0xFCCA

//set afh assess mode
#define HCI_DBG_PERIODIC_MONITOR_CMD_OPCODE                 0xFCCB
//BT/BLE RX gain and TX gain setting
#define HCI_DBG_SET_BT_BLE_TXRX_GAIN_CMD_OPCODE             0xFCCD

#define HCI_DBG_AFH_ASSESS_CMD_OPCODE                       0xFCD4

#define HCI_DBG_RF_IMPEDANCE_CMD_OPCODE                     0xFCD5

#define HCI_DBG_RF_IMPEDANCE_CMD_OPCODE                     0xFCD5

#define HCI_DBG_BLE_AUDIO_SETTING_CMD_OPCODE                0xFCD6

#define HCI_DBG_CUSTOM_CMD_OPCODE                           0xFCD7

#define HCI_DBG_BT_INQ_RXGAIN_CHANGED_CMD_OPCODE            0xFCD9

#define HCI_DBG_SET_BT_SCHE_SETTING_COEX_CIS_CMD_OPCODE     0xFCDA

#define HCI_DBG_ISO_QUALITY_REPORT_CMD_OPCODE               0xFCDC

#define HCI_DBG_SET_BT_BLE_ACTIVE_LINK_CMD_OPCODE           0xFCDD

#define HCI_DBG_LE_TX_POWER_REQUEST_CMD_OPCODE              0xFCDE

#define HCI_DBG_TCL_MHDT_MODE_CMD_OPCODE                    0xFCDF

#define HCI_DBG_TCL_READ_REMOTE_MHDT_FEATURES_CMD_OPCODE    0xFCE0

#define HCI_DBG_LE_MTK_RD_LOCAL_SUPP_FEATS_CMD_OPCODE       0xFCE1

#define HCI_DBG_SET_IBRT_RELAY_MODE_CMD_OPCODE              0xFCE2

#ifdef __BESTRX_SUPPORT__
#define HCI_DBG_SET_BESTRX_EN_CMD_OPCODE                    0xFCF0

#define HCI_DBG_SET_BESTRX_RF_SETTING_CMD_OPCODE            0xFCF1

#define HCI_DBG_SET_BESTRX_EVT_SETTING_CMD_OPCODE           0xFCF2

#define HCI_DBG_SET_BESTRX_HOP_INC_CMD_OPCODE               0xFCF3
#endif

#define HCI_DBG_SET_BESTRX_SCO_ECHANGE_EN_CMD_OPCODE        0xFCF4

#define HCI_DBG_PTACH_TEST_CMD_OPCODE                       0xFCF5

#define HCI_DBG_SET_RFTRX_EN_CMD_OPCODE                     0xFCF7

#define HCI_DBG_BES_TWS_FASTPAIR_CONFIG_CMD_OPCODE          0xFCF8

#define HCI_DBG_BES_GPIO_SETTING_CMD_OPCODE                 0xFCF9

#define HCI_DBG_SET_TX_IQ_MANUAL_CAL_CMD_OPCODE             0xFCFA

#define HCI_DBG_SET_TX_IQ_RF_REG_CMD_OPCODE                 0xFCFB

#define HCI_DBG_RESTORE_TX_IQ_RF_REG_CMD_OPCODE             0xFCFC

#define HCI_DBG_SET_BES_UART1_BAUDRATE_CMD_OPCODE           0xFCFD
#ifdef __RF_APB_RESTORE_SUPPORT__
#define HCI_DBG_SET_POWEROFF_BACKUP_RF_REG_CMD_OPCODE       0xFCFE
#endif
#define HCI_DBG_SET_POWERON_REG_CMD_OPCODE                  0xFCFF
#ifdef __cplusplus
extern "C" {
#endif

#define FAST_LOCK_ENABLE 1
#define FAST_LOCK_DISABLE 0

#define NEW_SYNC_SWAGC_MODE   1
#define OLD_SWAGC_MODE   2

#define BT_LOW_TXPWR_MODE    0
#define BLE_LOW_TXPWR_MODE   1
#define BLE_ADV_LOW_TXPWR_MODE   2

#define FACTOR_ATTENUATION_0DBM  0
#define FACTOR_ATTENUATION_6DBM  1
#define FACTOR_ATTENUATION_12DBM  2
#define FACTOR_ATTENUATION_18DBM  3


#if (__FASTACK_ECC_CONFIG_BLOCK__ == 1)    // 1 BLOCK
    #define ECC_MODU_MODE ECC_DPSK
    #define ECC_BLK_MODE ECC_1BLOCK
#elif (__FASTACK_ECC_CONFIG_BLOCK__ == 2)    // 2BLOCK
    #define ECC_MODU_MODE ECC_8PSK
    #define ECC_BLK_MODE ECC_2BLOCK
#elif (__FASTACK_ECC_CONFIG_BLOCK__ == 3)    // 3BLOCK
    #define ECC_MODU_MODE ECC_8PSK
    #define ECC_BLK_MODE ECC_3BLOCK
#endif


#define  INVALID_SYNC_WORD  (0)

#define BTDIGITAL_REG(a)                        (*(volatile uint32_t *)(a))

#define BTDIGITAL_REG_SET_FIELD(reg, mask, shift, v)\
                                                do{ \
                                                    volatile unsigned int tmp = *(volatile unsigned int *)(reg); \
                                                    tmp &= ~(mask<<shift); \
                                                    tmp |= (v<<shift); \
                                                    *(volatile unsigned int *)(reg) = tmp; \
                                                }while(0)

#define RWIP_MAX_CLOCK_TIME              ((1L<<28) - 1)
#define CLK_SUB(clock_a, clock_b)     ((uint32_t)(((clock_a) - (clock_b)) & RWIP_MAX_CLOCK_TIME))
#define CLK_ADD_2(clock_a, clock_b)     ((uint32_t)(((clock_a) + (clock_b)) & RWIP_MAX_CLOCK_TIME))
#define CLK_LOWER_EQ(clock_a, clock_b)      (CLK_SUB(clock_b, clock_a) < (RWIP_MAX_CLOCK_TIME >> 1))
#define CLK_DIFF(clock_a, clock_b)     ( (CLK_SUB((clock_b), (clock_a)) > ((RWIP_MAX_CLOCK_TIME+1) >> 1)) ? \
                          ((int32_t)((-CLK_SUB((clock_a), (clock_b))))) : ((int32_t)((CLK_SUB((clock_b), (clock_a))))) )
    /// Combined duration of Preamble and Access Address depending on the PHY used (in us)
#define BLE_PREAMBLE_ACCESS_ADDR_DUR_1MBPS      (5*8)
#define BLE_PREAMBLE_ACCESS_ADDR_DUR_2MBPS      (6*4)
#define BLE_PREAMBLE_ACCESS_ADDR_DUR_125KBPS    (80+256)
#define BLE_PREAMBLE_ACCESS_ADDR_DUR_500KBPS    (80+256)

typedef struct
{
    uint8_t is_act;
    uint16_t opcode;
    uint8_t parlen;
    const uint8_t *param;

} BTDRV_CFG_TBL_STRUCT;

#define BTDRV_CONFIG_ACTIVE   1
#define BTDRV_CONFIG_INACTIVE 0

enum{
    ECC_8PSK = 0,
    ECC_DPSK = 1,
    ECC_GFSK = 2,
    ECC_MODU_MAX,
};

enum{
    ECC_1BLOCK = 0,
    ECC_2BLOCK = 1,
    ECC_3BLOCK = 2,
    ECC_BLOCK_MAX,
};

enum{
    FA_SYNCWORD_32BIT = 0,
    FA_SYNCWORD_64BIT = 1,
    FA_SYNCWORD_MAX,
};

enum{
    ECC_8_BYTE_MODE = 0,
    ECC_16_BYTE_MODE = 1,
    ECC_12_BYTE_MODE = 2,
    ECC_MAX_BYTE_MODE,
};

enum
{
    METRIC_TYPE_TX_DM1_COUNTS = 0,
    METRIC_TYPE_TX_DH1_COUNTS,//1
    METRIC_TYPE_TX_DM3_COUNTS,//2
    METRIC_TYPE_TX_DH3_COUNTS,//3
    METRIC_TYPE_TX_DM5_COUNTS,//4
    METRIC_TYPE_TX_DH5_COUNTS,//5
    METRIC_TYPE_TX_2_DH1_COUNTS,//6
    METRIC_TYPE_TX_3_DH1_COUNTS,//7
    METRIC_TYPE_TX_2_DH3_COUNTS,//8
    METRIC_TYPE_TX_3_DH3_COUNTS,//9
    METRIC_TYPE_TX_2_DH5_COUNTS,//10
    METRIC_TYPE_TX_3_DH5_COUNTS,//11
    METRIC_TYPE_RX_DM1_COUNTS,//12
    METRIC_TYPE_RX_DH1_COUNTS,//13
    METRIC_TYPE_RX_DM3_COUNTS,//14
    METRIC_TYPE_RX_DH3_COUNTS,//15
    METRIC_TYPE_RX_DM5_COUNTS,//16
    METRIC_TYPE_RX_DH5_COUNTS,//17
    METRIC_TYPE_RX_2_DH1_COUNTS,//18
    METRIC_TYPE_RX_3_DH1_COUNTS,//19
    METRIC_TYPE_RX_2_DH3_COUNTS,//20
    METRIC_TYPE_RX_3_DH3_COUNTS,//21
    METRIC_TYPE_RX_2_DH5_COUNTS,//22
    METRIC_TYPE_RX_3_DH5_COUNTS,//23
    METRIC_TYPE_RX_HEC_ERROR_COUNTS,//24
    METRIC_TYPE_RX_CRC_ERROR_COUNTS,//25
    METRIC_TYPE_RX_FEC_ERROR_COUNTS,//26
    METRIC_TYPE_RX_GUARD_ERROR_COUNTS,//27
    METRIC_TYPE_ECC_OK_COUNTS,//28
    METRIC_TYPE_RADIO_COUNTS,//29
    METRIC_TYPE_SCO_ERROR_RX_TYPE_COUNTS,//30
    METRIC_TYPE_RADIO_TX_SUCCESS_COUNTS,//31
    METRIC_TYPE_RADIO_TX_COUNTS,//32
    METRIC_TYPE_SCO_PLD_ERR_COUNTS,//33
    METRIC_TYPE_BUFF_ERROR_COUNTS,//34
    METRIC_TYPE_RXSEQERR_COUNTS,//35
    METRIC_TYPE_FA_NO_SYNC_COUNTS,//36
    METRIC_TYPE_RX_2_EV3_COUNTS,//37
    METRIC_TYPE_RX_NOSYNC_2_EV3_COUNTS,//38
    METRIC_TYPE_MAX,
};

enum
{
    TX_PWR_IDX_0 = 0,
    TX_PWR_IDX_1 = 1,
    TX_PWR_IDX_2 = 2,
    TX_PWR_IDX_3 = 3,
    TX_PWR_IDX_4 = 4,
    TX_PWR_IDX_MAX = 5,
    TX_PWR_IDX_PAGE = 6,
    TX_PWR_IDX_6 = 6,
    TX_PWR_IDX_7 = 7,

};

enum
{
    RX_GAIN_IDX_0 = 0,
    RX_GAIN_IDX_1 = 1,
    RX_GAIN_IDX_2 = 2,
    RX_GAIN_IDX_3 = 3,
    RX_GAIN_IDX_4 = 4,
    RX_GAIN_IDX_5 = 5,
    RX_GAIN_IDX_6 = 6,
    RX_GAIN_IDX_7 = 7,
};

typedef struct
{
    bool fa_tx_gain_en;
    uint8_t fa_tx_gain_idx;
    bool fa_rx_gain_en;
    uint8_t fa_rx_gain_idx;
    bool fa_2m_mode;
    uint8_t fa_rx_winsz;
    uint8_t syncword_len;
    bool fa_multi_mode0_en;
    bool fa_multi_mode1_en;
    uint8_t fa_multi_tx_count;
}btdrv_fa_basic_config_t;

typedef struct
{
    bool ecc_mode_enable;
    uint8_t ecc_modu_mode_acl;
    uint8_t ecc_modu_mode_sco;
    uint8_t ecc_blk_mode;
    uint8_t ecc_len_mode_sel;
}btdrv_ecc_basic_config_t;

typedef struct
{
    uint32_t dat_arr[4];
}ecc_trx_dat_arr_t;


typedef struct
{
    ecc_trx_dat_arr_t trx_dat;
}ecc_trx_dat_t;

struct hci_dbg_set_sleep_para_cmd
{
    uint16_t  twrm;
    uint16_t twosc;
    uint16_t twext;
    uint8_t rwip_prog_delay;
    uint8_t clk_corr;
    uint16_t wait_26m_cnt_us;
    uint8_t slp_cfg; //bit0:cmu_intmask  bit1:ext_lpo
    uint8_t reserved;
    uint32_t poweroff_flag;

};

struct hci_acl_data_pkt
{
    uint16_t handle  :10;
    uint16_t ecc_flg : 1;
    uint16_t softbit_flg : 1;

    //0b00 First non-automatically-flushable packet of a higher layer message
    //(start of a non-automatically-flushable L2CAP PDU) from Host to Controller.
    //0b01 Continuing fragment of a higher layer message
    //0b10 First automatically flushable packet of a higher layer message (start of an automatically-flushable L2CAP PDU)
    //0b11 A complete L2CAP PDU. Automatically flushable.

    uint16_t pb_flag :2;
    uint16_t bc_flag :2;
    uint16_t data_load_len;
    uint8_t sdu_data[0];
}__attribute__((packed));

#ifdef BT_LOG_POWEROFF
enum set_poweron_reg_mode{
    CONFIGURE_MODE,
    BACKUP_MODE
};

struct hci_dbg_set_poweron_reg_cmd
{
    //mode   CONFIGURE_MODE:configure register  BACKUP_MODE:backup register
    uint8_t mode;
    //poweroff_flag   0:disable 1:enable
    uint8_t poweroff_flag;
    uint8_t reg_num;
    const uint32_t *reg_and_val;
};

#define CFG_MAX_REG_NUM_PER_BLK 30
#define BKP_MAX_REG_NUM_PER_BLK 61

/*
*  @brief: hci_dbg_set_poweron_reg_cmd struct
*  @param:
*          poweroff_flag       --> only take effect for CONFIGURE_MODE
*                                  0:only write registers and don't write the table     1:write both registers and the table
*          mode                --> 0:CONFIGURE_MODE          1:BACKUP_MODE
*          flush_tbl_flag      --> 0:no free and no malloc   1:free the table and then malloc memory for new table
*          reg_num_per_page    --> range from 1 to MAX
*                                  [MAX=CFG_MAX_REG_NUM_PER_BLK when CONFIGURE_MODE, MAX=BKP_MAX_REG_NUM_PER_BLK when BACKUP_MODE
*          reg_page_cur_index  --> indicate which block it writes now
*
*  @Note:  1. as for per HCI cmd, the sub-number of reg_total_num should be less than or equal to
*             CFG_MAX_REG_NUM_PER_BLK when CONFIGURE_MODE or BKP_MAX_REG_NUM_PER_BLK when BACKUP_MODE
*          2. when just free old table and don't take any action, it is expected to configure reg_total_num=0,
*             flush_tbl_flag=1(in addition, poweroff_flag=1 when CONFIGURE_MODE)
*          3. the configuration for reg_total_num has no constrain in BTH driver layer, but it will be restricted in single hci
*             to be less than or equal to reg_num_per_page when split all registers in reg_num_per_page.
*/
struct hci_dbg_set_poweron_reg_cmd_v2
{
    uint8_t poweroff_flag;
    uint8_t mode;
    uint8_t flush_tbl_flag;
    uint8_t reg_num_per_page;
    uint8_t reg_page_cur_index;
    uint8_t reg_total_num;
    const uint32_t *reg_and_val;
};
#endif

#define MULTI_IBRT_SUPPORT_NUM     3
struct sch_adjust_timeslice_per_link
{
    uint8_t link_id;
    uint32_t timeslice;
};

struct sch_multi_ibrt_adjust_timeslice_env_t
{
    uint8_t update;
    struct sch_adjust_timeslice_per_link ibrt_link[MULTI_IBRT_SUPPORT_NUM];
};

struct sch_multi_ibrt_par_per_link
{
    /// Used flag
    bool used;
    /// Link id
    uint8_t link_id;
    /******************* acl mode param *******************/
    /// active flag, indicate this link has alive activity
    uint8_t active_flag;
    /// Activity time slice (in half-slot), 0 if not used
    uint32_t time_slice;
    /// Activity min duration (in half-us), 0 if not used
    uint32_t min_duration;
    /// Anchor point
    uint32_t anchor_point;
    /******************* sco mode param *******************/
    /// Activity time slice (in sco event count), 0 if not used
    uint32_t sco_count;
    /// Activity time anchor in sco event count offset.
    uint32_t sco_count_offset;
    /// Activity min duration in sco mode (in half-us), 0 if not used
    uint32_t min_duration_in_sco;
};

struct sch_multi_ibrt_per_mode
{
    uint32_t ibrt_interval_total;

    uint32_t sco_count_total;

    struct sch_multi_ibrt_par_per_link ibrt_link[MULTI_IBRT_SUPPORT_NUM];
};

///BD Address structure
struct bd_addr
{
    ///6-byte array address value
    uint8_t  addr[6];
};
struct dbg_afh_assess_params
{
    struct bd_addr bd_addr;

    uint8_t afh_monitor_negative_num;
    uint8_t afh_monitor_positive_num;

    uint8_t average_cnt;
    uint8_t sch_prio_dft;
    uint8_t auto_resch_att;

    uint8_t rxgain;
    int8_t  afh_good_chl_thr;

    uint8_t lt_addr;
    bool edr;

    uint16_t interval;
    uint16_t sch_expect_assess_num;
    uint16_t prog_spacing_hus;
};

enum
{
    FUNC_NOT_CARE = 0x00,
    ///Disable
    FUNC_DIS,
    ///Enable
    FUNC_EN,
};

struct hci_dbg_set_periodic_monitor_params
{
    uint8_t afh_assess;
    uint8_t rf_impedance;

    uint8_t afh_interval_factor;
    uint8_t rf_interval_factor;

    uint8_t sch_prio_dft;
    uint8_t auto_resch_att;

    uint8_t afh_average_cnt;
    int8_t  afh_good_chl_thr;

    uint16_t afh_sch_expect_assess_num;
    /// Interval (in slots)
    uint16_t monitor_interval;
    uint16_t prog_spacing_hus;
}__attribute__((packed));

struct hci_dbg_set_afh_assess_params
{
    uint8_t enable;
    uint8_t interval_factor;
    uint8_t afh_average_cnt;
    int8_t  afh_good_chl_thr;

    uint16_t afh_sch_expect_assess_num;
}__attribute__((packed));

struct hci_dbg_set_rf_impedance_params
{
    uint8_t enable;
    uint8_t interval_factor;
}__attribute__((packed));

struct hci_dbg_set_txpwr_mode_cmd
{
    uint8_t     enable;
    //1: -6dbm, 2:-12dbm
    uint8_t     factor;
    //0:bt  1:ble 2:ble adv
    uint8_t     bt_or_ble;

    uint8_t     link_id;
}__attribute__((packed));


struct per_monitor_params_tag
{
    uint8_t sch_prio_dft;
    uint8_t auto_resch_att;

    /// Interval (in slots)
    uint16_t monitor_interval;
    uint16_t prog_spacing_hus;
};

#define RX_RECORD_NUMBER 15
#define EM_BT_RXSYNCERR_BIT     ((uint16_t)0x00000001)

struct ld_rx_status_record
{
    uint32_t clock;
    uint16_t rxstat;
    uint8_t link_id;
    uint8_t type;
    uint8_t rxchannel;
    uint8_t rx_gain;
    uint8_t snr;
    uint8_t fa_ok;
}__attribute__ ((__packed__));

#ifdef __BESTRX_SUPPORT__
struct hci_dbg_set_bestrx_en_cmd
{
    // Enable/Disable bestrx (0: disable / 1: enable)
    uint8_t  enable;
    uint8_t  mode;
};

struct hci_dbg_set_bestrx_rf_setting_cmd
{
    uint16_t conhdl;
    uint8_t  offset;
    uint8_t  txsetuptime;
    uint8_t  rxsetuptime;
    uint16_t  rxwin;
    uint8_t  ifs;
}__attribute__((packed));
#endif

extern const int8_t btdrv_rxgain_ths_tbl_le[0xf * 2];
extern const int8_t btdrv_rxgain_ths_tbl_le_2m[0xf * 2];
extern int8_t  btdrv_txpwr_conv_tbl[8];
extern int8_t  btdrv_ble_txpwr_conv_tbl[8];
extern const int8_t btdrv_rxgain_ths_tbl_ecc[0xf * 2];
#ifdef __NEW_SWAGC_MODE__
extern const int8_t btdrv_rxgain_gain_ths_3m[0xf * 2];
#endif

void btdrv_digital_init(void);
void bt_drv_reg_op_for_test_mode_disable(void);
void btdrv_txpower_calib(void);
#ifdef __NEW_SWAGC_MODE__
void bt_drv_rf_set_bt_sync_agc_enable(bool enable);
#endif
void bt_drv_reg_op_le_ext_adv_txpwr_independent(uint8_t en);
void bt_drv_ble_rf_reg_txpwr_set(uint16_t rf_reg, uint16_t val);
void bt_drv_reg_op_le_rf_reg_txpwr_set(uint16_t rf_reg, uint16_t val);
void bt_drv_reg_op_ble_sync_agc_mode_set(uint8_t en);
void btdrv_regop_set_txpwr_flg(uint32_t flg);
void btdrv_regop_host_set_txpwr(uint16_t acl_connhdl, uint8_t txpwr_idx);
void bt_drv_reg_op_low_txpwr(uint8_t enable, uint8_t factor, uint8_t bt_or_ble, uint8_t link_id);
void btdrv_bridge_send_data(const uint8_t *buff,uint8_t len);
void bt_drv_reg_op_set_afh_rxgain(uint8_t rxgain);
void bt_drv_reg_op_per_monitor_params(const uint8_t * per_params);
uint32_t btdrv_reg_op_cig_anchor_timestamp(uint8_t link_id);
uint32_t btdrv_reg_op_cig_subevt_anchor_timestamp(uint8_t link_id);
uint32_t btdrv_reg_op_big_anchor_timestamp(uint8_t link_id);
void bt_drv_reg_op_init(void);
void bt_drv_reg_op_rf_impedance_en(bool en, uint8_t interval_factor);
void bt_drv_reg_op_sel_btc_intersys(bool sel_sys_flag);
void btdrv_restart_bt(void);
void btdrv_cal_data_write(void);
void btdrv_temperature_dig_comp(const uint16_t dig_gain);
void btdrv_regop_temperature_calib_get_initial_val(uint16_t *tmx_gain_initial, uint16_t *pa_current_initial, uint16_t *dig_gain_initial);
void btdrv_regop_temp_calib_set_calib_value_v1(uint16_t tmx_gain_final, uint16_t pa_current_final);
void btdrv_regop_temp_calib_get_tx_gain_table_value_v2(uint16_t *bt_pa_current, uint16_t *ble_pa_current);
void btdrv_regop_temp_calib_set_tx_gain_table_value_v2(uint8_t i, uint16_t bt_pa_current_final, uint16_t ble_pa_current_final);
void btdrv_regop_tx_power_dig_comp(const uint16_t dig_gain);
void btdtv_rf_vco_drv_workaround(int temperature_celsius);
bool btdrv_reg_op_check_btc_boot_finish(void);
void btdrv_clear_spi_trig_pos_enable(void);
void bt_drv_reg_op_set_sleep_duration(uint32_t tws_slp_dura_dec, uint32_t normal_slp_dura_inc);
#ifdef __RF_APB_RESTORE_SUPPORT__
void bt_drv_rf_inf_enable_auto_power_down(bool enable);
#endif
void dc_iq_calib_1503(void);
#ifdef REDUCE_EDGE_CHL_TXPWR
void btdrv_reduce_edge_chl_txpwr_signal_mode(const unsigned char *data);
void btdrv_reduce_edge_chl_txpwr_nosignal_ble_mode(const unsigned char *data);
void btdrv_reduce_edge_chl_txpwr_nosignal_bt_mode(const unsigned char *data);
#endif

#ifdef __HW_AGC__
void btdrv_hwagc_lock_mode(const unsigned char *data);
#endif
void rx_dc_cal_1503(void);
void btdrv_rf_pll_aac_default_val(void);
void bt_drv_clear_wakeup_request(void);
#ifdef __cplusplus
}
#endif

#endif
