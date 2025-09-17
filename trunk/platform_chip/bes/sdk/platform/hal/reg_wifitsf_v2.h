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
#ifndef __REG_WIFITSF_V2_H__
#define __REG_WIFITSF_V2_H__

#include "plat_types.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WIFITSF_T {
    __IO uint32_t CTRL;             // 0x00
    __IO uint32_t TIME_OFFSET;      // 0x04
    __IO uint32_t TIME_LOW;         // 0x08
    __IO uint32_t TIME_HIGH;        // 0x0C
    __IO uint32_t EVENT0;           // 0x10
    __IO uint32_t EVENT1;           // 0x14
    __IO uint32_t EVENT2;           // 0x18
    __IO uint32_t EVENT3;           // 0x1C
    __IO uint32_t INT_STATUS;       // 0x20
    __IO uint32_t INT_MASK;         // 0x24

    __IO uint32_t EVENT0_LOW;       // 0x28
    __IO uint32_t EVENT0_HIGH;      // 0x2C
    __IO uint32_t EVENT1_LOW;       // 0x30
    __IO uint32_t EVENT1_HIGH;      // 0x34
    __IO uint32_t EVENT2_LOW;       // 0x38
    __IO uint32_t EVENT2_HIGH;      // 0x3C
    __IO uint32_t EVENT3_LOW;       // 0x40
    __IO uint32_t EVENT3_HIGH;      // 0x44

    __IO uint32_t TIME_ADJ_LOW;     // 0x48
    __IO uint32_t TIME_ADJ_HIGH;    // 0x4C

    __IO uint32_t XTAL_CYCLE_STEP;  // 0x50
    __IO uint32_t USB_TRIG_THD;     // 0x54
    __IO uint32_t DMA_TRIG_SEL;     // 0x58
    __IO uint32_t SYNC_INTERVAL;    //0x5C
    __IO uint32_t PULSE_WIDTH;      //0x60
    __IO uint32_t TRIG_CTRL;        //0x64

    __IO uint32_t RESERVED_68[6];   // 0x68

    __IO uint32_t DMA_TIME0_LOW;      // 0x80
    __IO uint32_t DMA_TIME0_HIGH;     // 0x84
    __IO uint32_t DMA_TIME0_FRAC;     // 0x88
    __IO uint32_t DMA_TIME1_LOW;      // 0x8C
    __IO uint32_t DMA_TIME1_HIGH;     // 0x90
    __IO uint32_t DMA_TIME1_FRAC;     // 0x94
    __IO uint32_t DMA_TIME2_LOW;      // 0x98
    __IO uint32_t DMA_TIME2_HIGH;     // 0x9C
    __IO uint32_t DMA_TIME2_FRAC;     // 0xA0
    __IO uint32_t DMA_TIME3_LOW;      // 0xA4
    __IO uint32_t DMA_TIME3_HIGH;     // 0xA8
    __IO uint32_t DMA_TIME3_FRAC;     // 0xAC

    __IO uint32_t STAT_TSF_WF_LOW;    // 0xB0
    __IO uint32_t STAT_TSF_WF_HIGH;   // 0xB4
    __IO uint32_t STAT_TSF_MINE_LOW;  // 0xB8
    __IO uint32_t STAT_TSF_MINE_HIGH; // 0xBC
    __IO uint32_t STAT_TSF_FRAC_MINE; // 0xC0

    __IO uint32_t SW_TRIG_TIME_LOW;   // 0xC4
    __IO uint32_t SW_TRIG_TIME_HIGH;  // 0xC8
    __IO uint32_t EVENT0_OFFSET_LOW;  // 0xCC
    __IO uint32_t EVENT0_OFFSET_HIGH; // 0xD0
};

// reg_00
#define TSF_TIMER_ENABLE                         (1 << 0)
#define CAPTURE_TRIG0_ENABLE                     (1 << 1)
#define CAPTURE_TRIG1_ENABLE                     (1 << 2)
#define CAPTURE_TRIG2_ENABLE                     (1 << 3)
#define CAPTURE_TRIG3_ENABLE                     (1 << 4)
#define CAPTURE_TRIG0_EDGE_SEL                   (1 << 5)
#define CAPTURE_TRIG1_EDGE_SEL                   (1 << 6)
#define CAPTURE_TRIG2_EDGE_SEL                   (1 << 7)
#define CAPTURE_TRIG3_EDGE_SEL                   (1 << 8)
#define CAPTURE_TRIG0_SRC_SEL(n)                 (((n) & 0x03) << 9)
#define CAPTURE_TRIG0_SRC_SEL_MASK               (0x03 << 9)
#define CAPTURE_TRIG0_SRC_SEL_SHIFT              (9)
#define CAPTURE_TRIG1_SRC_SEL(n)                 (((n) & 0x03) << 11)
#define CAPTURE_TRIG1_SRC_SEL_MASK               (0x03 << 11)
#define CAPTURE_TRIG1_SRC_SEL_SHIFT              (11)
#define CAPTURE_TRIG2_SRC_SEL(n)                 (((n) & 0x03) << 13)
#define CAPTURE_TRIG2_SRC_SEL_MASK               (0x03 << 13)
#define CAPTURE_TRIG2_SRC_SEL_SHIFT              (13)
#define CAPTURE_TRIG3_SRC_SEL(n)                 (((n) & 0x03) << 15)
#define CAPTURE_TRIG3_SRC_SEL_MASK               (0x03 << 15)
#define CAPTURE_TRIG3_SRC_SEL_SHIFT              (15)
#define TSF_SW_STORE_TRIG                        (17)
#define TSF_PRESCALE(n)                          (((n) & 0xFF) << 18)
#define TSF_PRESCALE_MASK                        (0xFF << 18)
#define TSF_PRESCALE_SHIFT                       (18)
#define TSF_WIFI_UPDATE_ENABLE                   (1 << 26)
#define TSF_CYCLE_COMP_ENABLE                    (1 << 27)
#define TSF_WIFI_UPDATE_QUICK                    (1 << 28)

// reg_04 tsf adjust
#define TSF_ADJUST_REQUEST                       (1 << 0)

// reg_10
#define TSF_EVENT0_ENABLE                        (1 << 31)

// reg_14
#define TSF_EVENT1_ENABLE                        (1 << 31)

// reg_18
#define TSF_EVENT2_ENABLE                        (1 << 31)

// reg_1C
#define TSF_EVENT3_ENABLE                        (1 << 31)

// reg_20
#define TSF_EVENT0_INT_STATUS                    (1 << 0)
#define TSF_EVENT1_INT_STATUS                    (1 << 1)
#define TSF_EVENT2_INT_STATUS                    (1 << 2)
#define TSF_EVENT3_INT_STATUS                    (1 << 3)
#define CAPTURE_TRIG0_INT_STATUS                 (1 << 4)
#define CAPTURE_TRIG1_INT_STATUS                 (1 << 5)
#define CAPTURE_TRIG2_INT_STATUS                 (1 << 6)
#define CAPTURE_TRIG3_INT_STATUS                 (1 << 7)
#define TSF_UPDATE_WIFI_STATUS                   (1 << 16)

// reg_24
#define TSF_EVENT0_INT_MASK                      (1 << 0)
#define TSF_EVENT1_INT_MASK                      (1 << 1)
#define TSF_EVENT2_INT_MASK                      (1 << 2)
#define TSF_EVENT3_INT_MASK                      (1 << 3)
#define CAPTURE_TRIG0_INT_MASK                   (1 << 4)
#define CAPTURE_TRIG1_INT_MASK                   (1 << 5)
#define CAPTURE_TRIG2_INT_MASK                   (1 << 6)
#define CAPTURE_TRIG3_INT_MASK                   (1 << 7)

// reg_50
#define CFG_CYCLE_STEP_VALUE(n)                  (((n) & 0xFFFFF) << 0)
#define CFG_CYCLE_STEP_VALUE_MASK                (0xFFFFF << 0)
#define CFG_CYCLE_STEP_VALUE_SHIFT               (0)

// reg_54
#define USB_TRIG_THRESHOLD(n)                    (((n) & 0xFFFF) << 0)
#define USB_TRIG_THRESHOLD_MASK                  (0xFFFF << 0)
#define USB_TRIG_THRESHOLD_SHIFT                 (0)

// reg_58
#define DMA_INT0_TRIG_SEL(n)                     (((n) & 0x0F) << 0)
#define DMA_INT0_TRIG_SEL_MASK                   (0x0F << 0)
#define DMA_INT0_TRIG_SEL_SHIFT                  (0)
#define DMA_INT1_TRIG_SEL(n)                     (((n) & 0x0F) << 8)
#define DMA_INT1_TRIG_SEL_MASK                   (0x0F << 8)
#define DMA_INT1_TRIG_SEL_SHIFT                  (8)
#define DMA_INT2_TRIG_SEL(n)                     (((n) & 0x0F) << 16)
#define DMA_INT2_TRIG_SEL_MASK                   (0x0F << 16)
#define DMA_INT2_TRIG_SEL_SHIFT                  (16)
#define DMA_INT3_TRIG_SEL(n)                     (((n) & 0x0F) << 24)
#define DMA_INT3_TRIG_SEL_MASK                   (0x0F << 24)
#define DMA_INT3_TRIG_SEL_SHIFT                  (24)

// reg_60
#define EVENT_PULSE_WIDTH(n)                     (((n) & 0xFFFF) << 16)
#define EVENT_PULSE_WIDTH_MASK                   (0xFFFF << 16)
#define EVENT_PULSE_WIDTH_SHIFT                  (16)

// reg_64
#define TSF_CODEC_TRIG_EN                        (1 << 4)
#define TSF_GPIO_TRIG_EN                         (1 << 5)
#define TSF_DSI_TRIG_EN                          (1 << 6)
#define EVENT_DSI_SEL(n)                         (((n) & 0x03) << 7)
#define EVENT_DSI_SEL_MASK                       (0x03 << 7)
#define EVENT_DSI_SEL_SHIFT                      (7)
#define EVENT_PWM_SEL(n)                         (((n) & 0x03) << 9)
#define EVENT_PWM_SEL_MASK                       (0x03 << 9)
#define EVENT_PWM_SEL_SHIFT                      (9)


#ifdef __cplusplus
}
#endif

#endif
