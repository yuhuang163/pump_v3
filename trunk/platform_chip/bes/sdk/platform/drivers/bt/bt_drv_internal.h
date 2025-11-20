/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __BT_DRV_INTERNAL_H__
#define __BT_DRV_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "stdint.h"
#include "stdbool.h"
#include "cmsis.h"

/*
 * DEFINES
 ****************************************************************************************
 */
//LMP Opcodes
#define LMP_NAME_REQ_OPCODE                1
#define LMP_NAME_RES_OPCODE                2
#define LMP_ACCEPTED_OPCODE                3
#define LMP_NOT_ACCEPTED_OPCODE            4
#define LMP_CLK_OFF_REQ_OPCODE             5
#define LMP_CLK_OFF_RES_OPCODE             6
#define LMP_DETACH_OPCODE                  7
#define LMP_INRAND_OPCODE                  8
#define LMP_COMBKEY_OPCODE                 9
#define LMP_UNITKEY_OPCODE                10
#define LMP_AURAND_OPCODE                 11
#define LMP_SRES_OPCODE                   12
#define LMP_TEMPRAND_OPCODE               13
#define LMP_TEMPKEY_OPCODE                14
#define LMP_ENC_MODE_REQ_OPCODE           15
#define LMP_ENC_KEY_SIZE_REQ_OPCODE       16
#define LMP_START_ENC_REQ_OPCODE          17
#define LMP_STOP_ENC_REQ_OPCODE           18
#define LMP_SWITCH_REQ_OPCODE             19
#define LMP_HOLD_OPCODE                   20
#define LMP_HOLD_REQ_OPCODE               21
#define LMP_SNIFF_REQ_OPCODE              23
#define LMP_UNSNIFF_REQ_OPCODE            24
#define LMP_PARK_REQ_OPCODE               25
#define LMP_SET_BSWIN_OPCODE              27
#define LMP_MODIF_BEACON_OPCODE           28
#define LMP_UNPARK_BD_REQ_OPCODE          29
#define LMP_UNPARK_PM_REQ_OPCODE          30
#define LMP_INCR_PWR_REQ_OPCODE           31
#define LMP_DECR_PWR_REQ_OPCODE           32
#define LMP_MAX_PWR_OPCODE                33
#define LMP_MIN_PWR_OPCODE                34
#define LMP_AUTO_RATE_OPCODE              35
#define LMP_PREF_RATE_OPCODE              36
#define LMP_VER_REQ_OPCODE                37
#define LMP_VER_RES_OPCODE                38
#define LMP_FEATS_REQ_OPCODE              39
#define LMP_FEATS_RES_OPCODE              40
#define LMP_QOS_OPCODE                    41
#define LMP_QOS_REQ_OPCODE                42
#define LMP_SCO_LINK_REQ_OPCODE           43
#define LMP_RMV_SCO_LINK_REQ_OPCODE       44
#define LMP_MAX_SLOT_OPCODE               45
#define LMP_MAX_SLOT_REQ_OPCODE           46
#define LMP_TIMING_ACCU_REQ_OPCODE        47
#define LMP_TIMING_ACCU_RES_OPCODE        48
#define LMP_SETUP_CMP_OPCODE              49
#define LMP_USE_SEMI_PERM_KEY_OPCODE      50
#define LMP_HOST_CON_REQ_OPCODE           51
#define LMP_SLOT_OFF_OPCODE               52
#define LMP_PAGE_MODE_REQ_OPCODE          53
#define LMP_PAGE_SCAN_MODE_REQ_OPCODE     54
#define LMP_SUPV_TO_OPCODE                55
#define LMP_TEST_ACTIVATE_OPCODE          56
#define LMP_TEST_CTRL_OPCODE              57
#define LMP_ENC_KEY_SIZE_MASK_REQ_OPCODE  58
#define LMP_ENC_KEY_SIZE_MASK_RES_OPCODE  59
#define LMP_SET_AFH_OPCODE                60
#define LMP_ENCAPS_HDR_OPCODE             61
#define LMP_ENCAPS_PAYL_OPCODE            62
#define LMP_SP_CFM_OPCODE                 63
#define LMP_SP_NB_OPCODE                  64
#define LMP_DHKEY_CHK_OPCODE              65
#define LMP_PAUSE_ENC_AES_REQ_OPCODE      66

///LMP Escape 4 Extended Opcodes
#define LMP_ACCEPTED_EXT_EXTOPCODE         1
#define LMP_NOT_ACCEPTED_EXT_EXTOPCODE     2
#define LMP_FEATS_REQ_EXT_EXTOPCODE        3
#define LMP_FEATS_RES_EXT_EXTOPCODE        4
#define LMP_CLK_ADJ_EXTOPCODE              5
#define LMP_CLK_ADJ_ACK_EXTOPCODE          6
#define LMP_CLK_ADJ_REQ_EXTOPCODE          7
#define LMP_PKT_TYPE_TBL_REQ_EXTOPCODE    11
#define LMP_ESCO_LINK_REQ_EXTOPCODE       12
#define LMP_RMV_ESCO_LINK_REQ_EXTOPCODE   13
#define LMP_CH_CLASS_REQ_EXTOPCODE        16
#define LMP_CH_CLASS_EXTOPCODE            17
#define LMP_SSR_REQ_EXTOPCODE             21
#define LMP_SSR_RES_EXTOPCODE             22
#define LMP_PAUSE_ENC_REQ_EXTOPCODE       23
#define LMP_RESUME_ENC_REQ_EXTOPCODE      24
#define LMP_IO_CAP_REQ_EXTOPCODE          25
#define LMP_IO_CAP_RES_EXTOPCODE          26
#define LMP_NUM_COMPARISON_FAIL_EXTOPCODE 27
#define LMP_PASSKEY_FAIL_EXTOPCODE        28
#define LMP_OOB_FAIL_EXTOPCODE            29
#define LMP_KEYPRESS_NOTIF_EXTOPCODE      30
#define LMP_PWR_CTRL_REQ_EXTOPCODE        31
#define LMP_PWR_CTRL_RES_EXTOPCODE        32
#define LMP_PING_REQ_EXTOPCODE            33
#define LMP_PING_RES_EXTOPCODE            34

/// Bluetooth technologies version
#define BT40_VERSION                      (6)
#define BT41_VERSION                      (7)
#define BT42_VERSION                      (8)
#define BT50_VERSION                      (9)
#define BT51_VERSION                      (10)
#define BT52_VERSION                      (11)
#define BT53_VERSION                      (12)
#define BT54_VERSION                      (13)

#ifndef ASSERT_ERR
#define ASSERT_ERR(cond)                             { if (!(cond)) { BT_DRV_TRACE(2,"line is %d file is %s", __LINE__, __FILE__); } }
#endif

#define BT_DRV_REG_OP_ENTER()    do{ uint32_t stime, spent_time; stime= hal_sys_timer_get();
#define BT_DRV_REG_OP_EXIT()     spent_time = TICKS_TO_US(hal_sys_timer_get()-stime);if (spent_time>1000)TRACE(2,"%s exit, %dus",__func__, spent_time);}while(0);

#define BTPCM_MASTER_MODE  (0)
#define BTPCM_SLAVE_MODE   (1)

#define DUT_CONNECT_STATUS_CONNECTED (1)
#define DUT_CONNECT_STATUS_DISCONNECTED (2)

/// Linear format
#define LINEAR_FORMAT_8_BITS    0x00
#define LINEAR_FORMAT_13_BITS   0x01
#define LINEAR_FORMAT_14_BITS   0x02
#define LINEAR_FORMAT_16_BITS   0x03

//MONO_LR_SEL
#define LEFT_CHANNEL_MONO    0x0
#define RIGHT_CHANNEL_MONO   0X1

//MONO_STEREO
#define MONO_SAMPLE_MODE 0
#define MONO_STEREO_MODE 1

#define B(byte, feat)     ((FEAT_##feat##_SUPP << B##byte##_##feat##_POS) & B##byte##_##feat##_MSK)
/// Extended feature mask definition page 2 LMP:3.3
#define B1_SEC_CON_CTRL_POS         0
#define B1_SEC_CON_CTRL_MSK         0x01
#define B1_PING_POS                 1
#define B1_PING_MSK                 0x02
#define B1_SAM_POS                  2
#define B1_SAM_MSK                  0x04
#define B1_TRAIN_NUDGING_POS        3
#define B1_TRAIN_NUDGING_MSK        0x08

#define BT_DRIVER_GET_U8_REG_VAL(regAddr)       (*(volatile uint8_t *)(uintptr_t)(regAddr))
#define BT_DRIVER_GET_U16_REG_VAL(regAddr)      (*(volatile uint16_t *)(uintptr_t)(regAddr))
#define BT_DRIVER_GET_U32_REG_VAL(regAddr)      (*(volatile uint32_t *)(uintptr_t)(regAddr))

#define BT_DRIVER_PUT_U8_REG_VAL(regAddr, val)      *(volatile  uint8_t *)(uintptr_t)(regAddr) = (val)
#define BT_DRIVER_PUT_U16_REG_VAL(regAddr, val)     *(volatile  uint16_t *)(uintptr_t)(regAddr) = (val)
#define BT_DRIVER_PUT_U32_REG_VAL(regAddr, val)     *(volatile  uint32_t *)(uintptr_t)(regAddr) = (val)

#define BT_SUB_SYS_TYPE     0
#define MCU_SYS_TYPE        1
#define BT_EM_AREA_1_TYPE   2
#define BT_EM_AREA_2_TYPE   3

#define BT_FA_INVERT_EN   1
#define BT_FA_INVERT_DISABLE   0

/***************************************************************************
 *multi IBRT config
 ****************************************************************************/
#define MULTI_IBRT_BG_SLICE 24

#define MULTI_IBRT_FG_SLICE 72

#define TX_PWR_20DBM 20
#define TX_PWR_19DBM 19
#define TX_PWR_18DBM 18
#define TX_PWR_17DBM 17
#define TX_PWR_16DBM 16
#define TX_PWR_15DBM 15
#define TX_PWR_14DBM 14
#define TX_PWR_13DBM 13
#define TX_PWR_12DBM 12
#define TX_PWR_11DBM 11
#define TX_PWR_10DBM 10
#define TX_PWR_9DBM  9
#define TX_PWR_8DBM  8
#define TX_PWR_7DBM  7
#define TX_PWR_6DBM  6
#define TX_PWR_5DBM  5
#define TX_PWR_4DBM  4
#define TX_PWR_3DBM  3
#define TX_PWR_2DBM  2
#define TX_PWR_1DBM  1
#define TX_PWR_0DBM  0
#define TX_PWR_N1DBM -1
#define TX_PWR_N2DBM -2
#define TX_PWR_N3DBM -3
#define TX_PWR_N4DBM -4
#define TX_PWR_N5DBM -5
#define TX_PWR_N6DBM -6
#define TX_PWR_N7DBM -7
#define TX_PWR_N8DBM -8
#define TX_PWR_N9DBM -9
#define TX_PWR_N10DBM -10
#define TX_PWR_N11DBM -11
#define TX_PWR_N12DBM -12
#define TX_PWR_N13DBM -13
#define TX_PWR_N14DBM -14

#define GET_32BIT_0BYTE(value) ((uint8_t)(value & 0xFF))

#define GET_32BIT_1BYTE(value) ((uint8_t)((value >> 8) & 0xFF))

#define GET_32BIT_2BYTE(value) ((uint8_t)((value >> 16) & 0xFF))

#define GET_32BIT_3BYTE(value) ((uint8_t)((value >> 24) & 0xFF))

#define GET_16BIT_0BYTE(value) ((uint8_t)(value & 0xFF))

#define GET_16BIT_1BYTE(value) ((uint8_t)((value >> 8) & 0xFF))

#ifdef __AFH_ASSESS__
#define CHAS_ENABLE_BIT                                     (1)
#define CHAS_ENABLE_POS                                     (0)
#define CHAS_NOSYNC_ENABLE_BIT                              (2)
#define CHAS_NOSYNC_ENABLE_POS                              (1)

/// Macro to extract a bit from a bit field
/// @param[in] __r bit field value
/// @param[in] __b bit field name
/// @return the value of the register masked and shifted
#define GETB(__r, __b)                                                           \
    (( (__r) & (__b##_BIT) ) >> (__b##_POS))

/// Macro to set a bit into a bit field
/// @param[in] __r bit field value
/// @param[in] __b bit field name
/// @param[in] __v value to put in field
#define SETB(__r, __b, __v)                                                      \
    do {                                                                         \
        ASSERT_ERR( ( ( ( ((__v) ? 1 : 0) << (__b##_POS) ) & ( ~(__b##_BIT) ) ) ) == 0 ); \
        (__r) = (((__r) & ~(__b##_BIT)) | ((__v) ? 1 : 0) << (__b##_POS));                  \
    } while (false)
#endif /* __AFH_ASSESS__ */

/*
 * ENUMERATION
 ****************************************************************************************
 */

#ifdef MCU_WAKEUP_BT_V2
#define BT_DRV_OPER_BTCORE_MSG_LEN                  (8)
#define BT_DRV_OPER_BTCORE_RSP_MAX_RETRY_CNT        (30)
#define BT_DRV_OPER_BTCORE_IDLE_MAX_RETRY_CNT       (10)
#define BT_DRV_OPER_BTCORE_BASE_DELAY_US            (10)
#define BT_DRV_OPER_BTCORE_WAKE_UP_THD              (0) //(1000)

enum WAKEUP_BT_USER_T
{
    WAKEUP_BT_USER_DRV = 1,
    WAKEUP_BT_USER_TRIG,

    WAKEUP_BT_USER_QTY
};

enum WAKEUP_BT_MSG_T
{
    // disable sleep (1)  resume sleep (2) wakeup BT(3)
    MSG_WAKEUP_BT_AND_DIS_SLP = 1,
    MSG_WAKEUP_BT_AND_EN_SLP,
    MSG_ONLY_WAKEUP_BT,
    MSG_COEX_PARAM_SET,
    MSG_COEX_PARAM_RESTORE,

    WAKEUP_BT_MSG_QTY
};
#endif /* MCU_WAKEUP_BT_V2 */

enum bes_btc_tport_level
{
    SNOOP_LEVEL                  = 0x1,
    ACL_LEVEL                    = 0x2,
    SCO_LEVEL                    = 0x4,
    BLE_LEVEL                    = 0x8,
    IBRT_LMP_LEVEL               = 0x10,
    SCH_LEVEL                    = 0x20,
    ISR_LEVEL                    = 0x40,
    PATCH_LEVEL                  = 0x80,
    TRANSQ_LEVEL                 = 0x100,
    PD_LEVEL                     = 0x200,
};

enum off_tport_bit_mask
{
    SIGNAL_FRAME_EN              = 0x1,
    SIGNAL_FIFO_ISR              = 0x2,
    SIGNAL_EVT_TYPE              = 0x4,
    SIGNAL_EVT_PRIO              = 0x8,
    SIGNAL_LINK_ID               = 0x10,
    SIGNAL_FREQ                  = 0x20,
    SIGNAL_TX_PWR_RX_GAIN        = 0x40,
    SIGNAL_RSSI                  = 0x80,

    SIGNAL_RX_SEQ                = 0x1,
    SIGNAL_TX_SEQ_ACK            = 0x2,
    SIGNAL_RX_NESN_RX_SN         = 0x4,
    SIGNAL_TX_NESN_TX_SN         = 0x8,
    SIGNAL_RX_STATE              = 0x10,
    SIGNAL_LC_STATE              = 0x20,
    SIGNAL_LMP_TX_OP             = 0x40,
    SIGNAL_LMP_TX_CFM_OP         = 0x80,

    SIGNAL_LMP_RX_OP             = 0x1,
    SIGNAL_LLC_PROC_LOC          = 0x2,
    SIGNAL_LLC_PROC_REM          = 0x4,
    SIGNAL_LLC_OP_RX             = 0x8,
    SIGNAL_COEX_GRANTED_TS       = 0x10,
    SIGNAL_COEX_TDD              = 0x20,
    SIGNAL_COEX_ERROR_CODE       = 0x40,
    SIGNAL_CLKN                  = 0x80,

    SIGNAL_ET_STATUS             = 0x1,
    SIGNAL_LLC_PROC_LOC_STATE    = 0x2,
    SIGNAL_LLC_PROC_REM_STATE    = 0x4,
    SIGNAL_LLC_OP_TX             = 0x8,
};

enum rf_spi_bit_cfg_mode
{
    RF_SPI_8BIT_MODE = 0,
    RF_SPI_10BIT_MODE = 1,
};

enum btc_slp_cfg
{
    /// Boolean indicating whether cmu intmask
    BES_CMU_INTMASK_POS       = 0,
    BES_CMU_INTMASK_BIT       = 0x01,
    /// Boolean indicating whether to use external 32K LPO
    BES_EXT_LPO_POS           = 1,
    BES_EXT_LPO_BIT           = 0x02,
    /// Boolean indicating RF SPI BIT NUMBER
    BES_RF_SPI_BIT_NB_LSB     = 2,
    BES_RF_SPI_BIT_NB_MASK    = 0xC,
};

enum BT_TPORT_TYPE_T
{
    BT_TPORT_8WIRE_NORMAL = 0,
    BT_TPORT_3WIRE = 1,
    BT_TPORT_8WIRE_TYPE1 = 2,

    BT_TPORT_QTY
};


typedef enum
{
    CHAS_POLICY_GOOD = 0,                                   // Select the best 20 channels
    CHAS_POLICY_BAD,                                        // Select the worst 20 channels
    CHAS_POLICY_X,                                          // Select X specified channels
    CHAS_POLICY_X_LOW,                                      // Select X channels in low band
    CHAS_POLICY_X_HIGH,                                     // Select X channels in high band
    CHAS_POLICY_MAX,                                        // Maximum policy value (limit)
} chas_policy_t;

/*
 * COMMON FUNCTION DECLARATIONS
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint32_t co_mod(uint32_t val, uint32_t div)
{
   return ((val) % (div));
}

#define CO_MOD(val, div) co_mod(val, div)

__STATIC_FORCEINLINE uint32_t co_max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

/**
 ****************************************************************************************
 * @brief Read a packed 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @return The 16 bits value.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t co_read16p(void const *ptr16)
{
    uint16_t value = ((uint8_t *)ptr16)[0] | ((uint8_t *)ptr16)[1] << 8;
    return value;
}

/**
 ****************************************************************************************
 * @brief Read a packed 32 bits word.
 * @param[in] ptr32 The address of the first byte of the 32 bits word.
 * @return The 32 bits value.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint32_t co_read32p(void const *ptr32)
{
    uint16_t addr_l, addr_h;
    addr_l = co_read16p((uint16_t *)ptr32);
    addr_h = co_read16p((uint16_t *)ptr32 + 1);
    return ((uint32_t)addr_l | (uint32_t)addr_h << 16);
}

/**
 ****************************************************************************************
 * @brief Write a packed 32 bits word.
 * @param[in] ptr32 The address of the first byte of the 32 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE void co_write32(void const *ptr32, uint32_t value)
{
    uint8_t volatile *ptr=(uint8_t*)ptr32;

    *ptr++ = (uint8_t)(value&0xff);
    *ptr++ = (uint8_t)((value&0xff00)>>8);
    *ptr++ = (uint8_t)((value&0xff0000)>>16);
    *ptr = (uint8_t)((value&0xff000000)>>24);
}

/**
 ****************************************************************************************
 * @brief Write a packed 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE void co_write16(void const *ptr16, uint16_t value)
{
    uint8_t volatile *ptr=(uint8_t*)ptr16;

    *ptr++ = value&0xff;
    *ptr = (value&0xff00)>>8;
}

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void btdrv_poweron(uint8_t en);
uint8_t btdrv_rf_init(void);
void btdrv_test_mode_rf_txpwr_init(void);
void btdrv_ins_patch_init(void);
void btdrv_ins_patch_ram_cfg(void);
void btdrv_data_patch_init(void);
void btdrv_patch_en(uint8_t en);
void btdrv_config_init(void);
void btdrv_config_end(void);
void btdrv_testmode_config_init(void);
void btdrv_bt_spi_rawbuf_init(void);
void btdrv_bt_spi_xtal_init(void);
void btdrv_rf_rx_gain_adjust_req(uint32_t user, bool lowgain);
void btdrv_btc_fault_dump(void);
void btdrv_dump_mem(uint8_t *dump_mem_start, uint32_t dump_length, uint8_t dump_type);
void btdrv_fast_lock_config(bool fastlock_on);
void btdrv_ecc_config(void);
void btdrv_btc_fault_dump(void);
void btdrv_fa_config(void);
void btdrv_hciprocess(void);
void btdrv_rf_init_ext(void);
void bt_drv_reg_op_dump_rx_record(void);
int8_t btdrv_get_rssi_avg_thr(bool tws_link);
void bt_drv_reg_op_enable_3wire_tports(void);

void bt_drv_heap_init(void);
void btdrv_common_init(void);
void *bt_drv_malloc(unsigned int size);
void bt_drv_free(void *ptr);
void bt_tester_cmd_receive_evt_analyze(const unsigned char *data, unsigned int len);
uint32_t bt_drv_get_btc_sw_version(void);
void bt_drv_bt_tport_type_config(void);
void bt_drv_reg_op_set_sram_offset(uint32_t offset);
#ifdef MCU_WAKEUP_BT_V2
void bt_drv_oper_btcore_with_rsp(enum WAKEUP_BT_USER_T user, enum WAKEUP_BT_MSG_T msg);
#endif /* MCU_WAKEUP_BT_V2 */

#ifdef __cplusplus
}
#endif
#endif //__BT_DRV_INTERNAL_H__
