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
#ifndef __REG_LM_SENSOR_H__
#define __REG_LM_SENSOR_H__

#include "plat_types.h"

// LM_SENSOR Registers
struct LM_SENSOR_T
{
    __IO uint32_t SENS_PWR;                 // 0x00
    __IO uint32_t CNT_THRESHOLD;            // 0x04
    __O  uint32_t SENS_INT;                 // 0x08
    __IO uint32_t SENS_INTMSK;              // 0x0C
    __IO uint32_t PULSE_CNT_THRESHOLD;      // 0x10
};

// reg_00
#define SENSOR_PWR                          (1 << 0)
#define CNT_CLR                             (1 << 1)

// reg_04
#define CNT_THRESHOLD_SHIFT                 (0)
#define CNT_THRESHOLD_MASK                  (0xFFFFFF << CNT_THRESHOLD_SHIFT)
#define CNT_THRESHOLD(n)                    ((n << CNT_THRESHOLD_SHIFT)&CNT_THRESHOLD_MASK)

// reg_08
#define PULSE_CNT_OK_SHIFT                  (0)
#define PULSE_CNT_OK_MASK                   (0xFFF << PULSE_CNT_OK_SHIFT)
#define SENSOR_INTR_SHIFT                   (14)
#define SENSOR_MAX_INTR                     (1 << 14)
#define SENSOR_DONE_INTR                    (1 << 15)

// reg_0C
#define SENSOR_DONE_INTR_EN                 (1 << 0)
#define SENSOR_MAX_INTR_EN                  (1 << 0)
#define SENSOR_DONE_INTR_CLR                (1 << 1)
#define SENSOR_MAX_INTR_CLR                 (1 << 1)
#define SENSOR_DONE_INTR_MASK               (1 << 2)
#define SENSOR_MAX_INTR_MASK                (1 << 2)

// reg_10
#define PULSE_CNT_THRESHOLD_SHIFT           (0)
#define PULSE_CNT_THRESHOLD_MASK            (0xFFFFFF << PULSE_CNT_THRESHOLD_SHIFT)
#define PULSE_CNT_THRESHOLD(n)              ((n << PULSE_CNT_THRESHOLD_SHIFT)&PULSE_CNT_THRESHOLD_MASK)

#endif
