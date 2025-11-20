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
#ifndef __BT_DRV_1503_CONFIG_H__
#define __BT_DRV_1503_CONFIG_H__
/***************************************************************************
 *some vendor software version record
 ****************************************************************************/
#define IBRT_MULTI_SCH_HCI_SUPPORT_VERSION (0x230920)

/***************************************************************************
 *BTC sleep and wakeup configration
 *align BTC wakeup timing with MCU config (HAL_CMU_OSC_STABLE_TIME)
 ****************************************************************************/
#include CHIP_SPECIFIC_HDR(hal_cmu_pri)
/*
*   TWOSC: Time to wake-up osc_en before deepsleep_time expiration
*   TWRM  : Time to wake-up radio module(no used)
*   TWEXT : Time to wake-up osc_en on external wake-up request
*/
#ifdef BT_LOG_POWEROFF
#define BTC_LP_MODE                     (LOGIC_POWEROFF_EN)
#else
#define BTC_LP_MODE                     (BTC_LP_OFF)
#endif
//BTC power off section define
#define IP_PROG_DELAY_DFT (3)
#define BTC_LP_OFF       (0x0000)
#define LOGIC_POWEROFF_EN      (0x0004)

/***************************************************************************
 *HW AGC configration
 ****************************************************************************/
#ifdef __HW_AGC__
#define BTC_HW_AGC_ENABLE_FLAG 0x01
#else
#define BTC_HW_AGC_ENABLE_FLAG 0x00
#endif

#define GAIN_TBL_SIZE           0x0F

//#define __DEBUG_FIX_RX_GAIN_CNTL__
/***************************************************************************
 *BT phy configration
 ****************************************************************************/

#define PHY_TX_DELAY    (0x20)
#define PHY_RX_DELAY    (0x1E)

#define BT_CORE_RAMP_DOWN_DELAY (0x6)
/***************************************************************************
 *XTAL cap default val
 ****************************************************************************/
#define DEFAULT_XTAL_FCAP                       (0xC8)

#define RF_REG_XTAL_FCAP        (0x504)


#define RF_REG_PU_RF_INF         (0x0527)
#define REG_PU_RF_INF_SW_DR                    (1 << 8)
#define REG_ISO_EN_RF_INF_DR                   (1 << 10)
#define REG_ISO_EN_TO_DIG_DR                   (1 << 12)

#define RF_REG_SYNC_AGC         (0X22)
/***************************************************************************
 *channel assessment config
 ****************************************************************************/
#define AFH_ASSESS_GAIN  2

/***************************************************************************
 *TX power configration
 ****************************************************************************/
#define BLE_MIN_TX_PWR_IDX    (0)
#define BLE_MAX_TX_PWR_IDX    (3)
#define BT_MIN_TX_PWR_IDX     (0)
#define BT_MAX_TX_PWR_IDX     (3)
#define BT_INIT_TX_PWR_IDX    (BT_MAX_TX_PWR_IDX)
#define RF_MAX_RX_GAIN_IDX    (7)

#ifdef __HW_AGC__
#define BTC_HW_AGC_ENABLE_FLAG 0x01
#else
#define BTC_HW_AGC_ENABLE_FLAG 0x00
#endif

//RF BT TX register
#define RF_BT_TX_PWR_TBL0_IDX0_REG (0x14c)
#define RF_BT_TX_PWR_TBL0_IDX1_REG (0x14d)
#define RF_BT_TX_PWR_TBL0_IDX2_REG (0x14e)
#define RF_BT_TX_PWR_TBL0_IDX3_REG (0x14f)
#define RF_BT_TX_PWR_TBL0_IDX4_REG (0x150)
#define RF_BT_TX_PWR_TBL0_IDX5_REG (0x151)
#define RF_BT_TX_PWR_TBL0_IDX6_REG (0x152)
#define RF_BT_TX_PWR_TBL0_IDX7_REG (0x153)

#define RF_BT_TX_PWR_TBL1_IDX0_REG (0x18E)
#define RF_BT_TX_PWR_TBL1_IDX1_REG (0x18F)
#define RF_BT_TX_PWR_TBL1_IDX2_REG (0x190)
#define RF_BT_TX_PWR_TBL1_IDX3_REG (0x191)
#define RF_BT_TX_PWR_TBL1_IDX4_REG (0x192)
#define RF_BT_TX_PWR_TBL1_IDX5_REG (0x193)
#define RF_BT_TX_PWR_TBL1_IDX6_REG (0x194)
#define RF_BT_TX_PWR_TBL1_IDX7_REG (0x195)

#define RF_BT_TX_PWR_TBL0_IDX0_REG_EDGE_CHL (0x43A)
#define RF_BT_TX_PWR_TBL0_IDX1_REG_EDGE_CHL (0x43B)
#define RF_BT_TX_PWR_TBL0_IDX2_REG_EDGE_CHL (0x43C)
#define RF_BT_TX_PWR_TBL0_IDX3_REG_EDGE_CHL (0x43D)
#define RF_BT_TX_PWR_TBL0_IDX4_REG_EDGE_CHL (0x43E)
#define RF_BT_TX_PWR_TBL0_IDX5_REG_EDGE_CHL (0x43F)
#define RF_BT_TX_PWR_TBL0_IDX6_REG_EDGE_CHL (0x440)
#define RF_BT_TX_PWR_TBL0_IDX7_REG_EDGE_CHL (0x441)

#define RF_BT_TX_PWR_TBL1_IDX0_REG_EDGE_CHL (0x442)
#define RF_BT_TX_PWR_TBL1_IDX1_REG_EDGE_CHL (0x443)
#define RF_BT_TX_PWR_TBL1_IDX2_REG_EDGE_CHL (0x444)
#define RF_BT_TX_PWR_TBL1_IDX3_REG_EDGE_CHL (0x445)
#define RF_BT_TX_PWR_TBL1_IDX4_REG_EDGE_CHL (0x446)
#define RF_BT_TX_PWR_TBL1_IDX5_REG_EDGE_CHL (0x447)
#define RF_BT_TX_PWR_TBL1_IDX6_REG_EDGE_CHL (0x448)
#define RF_BT_TX_PWR_TBL1_IDX7_REG_EDGE_CHL (0x449)
//RF BLE TX register
#define RF_LE_TX_PWR_TBL0_IDX0_REG (0x154)
#define RF_LE_TX_PWR_TBL0_IDX1_REG (0x155)
#define RF_LE_TX_PWR_TBL0_IDX2_REG (0x156)
#define RF_LE_TX_PWR_TBL0_IDX3_REG (0x157)
#define RF_LE_TX_PWR_TBL0_IDX4_REG (0x158)
#define RF_LE_TX_PWR_TBL0_IDX5_REG (0x159)
#define RF_LE_TX_PWR_TBL0_IDX6_REG (0x15A)
#define RF_LE_TX_PWR_TBL0_IDX7_REG (0x15A)

#define RF_LE_TX_PWR_TBL1_IDX0_REG (0x196)
#define RF_LE_TX_PWR_TBL1_IDX1_REG (0x197)
#define RF_LE_TX_PWR_TBL1_IDX2_REG (0x198)
#define RF_LE_TX_PWR_TBL1_IDX3_REG (0x199)
#define RF_LE_TX_PWR_TBL1_IDX4_REG (0x19A)
#define RF_LE_TX_PWR_TBL1_IDX5_REG (0x19B)
#define RF_LE_TX_PWR_TBL1_IDX6_REG (0x19C)
#define RF_LE_TX_PWR_TBL1_IDX7_REG (0x19D)
/***************************************************************************
 *RX gain configration
 ****************************************************************************/

/***************************************************************************
 *Fast ack configration
 ****************************************************************************/
#ifdef mHDT_SUPPORT
#define FA_RX_WIN_SIZE            (0x4)
//MDHT todo
#define FA_CNT_PKT_US             (0x68)
#else
#define FA_RX_WIN_SIZE            (0xA)
#define FA_CNT_PKT_US             (0x1F)
#endif
#define FA_RXPWRUP_TIMING         (0x37)
#define FA_TXPWRUP_TIMING         (0x28)
#define FA_BW2M_RXPWRUP_TIMING    (0x2F)
#define FA_BW2M_TXPWRUP_TIMING    (0x28)

#define FA_FIX_TX_GIAN_IDX        (0x3)
#define FA_FIX_RX_GIAN_IDX        (0x1)
#define FA_MULTI_TX_COUNT         (0x3)
/***************************************************************************
 *BTC activity number configration
 ****************************************************************************/
#define BLE_ACTIVITY_MAX        (20)
#define MAX_NB_ACTIVE_ACL       (5)
#define HCI_HANDLE_MIN         (0x80)
#define HCI_HANDLE_MAX         (HCI_HANDLE_MIN + MAX_NB_ACTIVE_ACL - 1)

/***************************************************************************
 *BEStrx RF timing configration
 ****************************************************************************/
#define BESTRX_OFFSET   (0x32)
#define BESTRX_TXPWRUP_CNT                   (0x20)
#define BESTRX_RXPWRUP_CNT         (0x32)
#define BESTRX_MASTER_RX_WINSIZE         (0XF)
#define BESTRX_TIFS (150)

/***************************************************************************
 *HDT trx RF timing configration
 ****************************************************************************/
#ifdef mHDT_SUPPORT
#define MHDT4M_TXPWRUP_CNT (0x36)
#define MHDT4M_RXPWRUP_CNT (0x37)
#define MHDT4M_TXPATH_DLY (0x3)  // real value, do not change this
#define MHDT4M_RXPATH_DLY (0x0)

#define MHDT2M_TXPWRUP_CNT (0x36)
#define MHDT2M_RXPWRUP_CNT (0x41)
#define MHDT2M_TXPATH_DLY (0x5) // real value, do not change this
#define MHDT2M_RXPATH_DLY (0x0)

#define MHDT_LE4M_TXPWRUP_CNT (0x36)
#define MHDT_LE4M_RXPWRUP_CNT (0x37)
#define MHDT_LE4M_TXPATH_DLY (0x0)
#define MHDT_LE4M_RXPATH_DLY (0x3) // real value, do not change this

#endif

#endif //__BT_DRV_1503_CONFIG_H__
