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

#include "hal_cmu.h"
#include "hal_lm_sensor.h"
#include "reg_lm_sensor.h"
#include "hal_iomux.h"

#ifdef LM_SENSOR0_BASE
static const enum HAL_CMU_MOD_ID_T cmu_mode_table[HAL_LM_SENSOR_ID_NUM] = {
    HAL_CMU_MOD_P_LM0,
#ifdef LM_SENSOR1_BASE
    HAL_CMU_MOD_P_LM1,
#endif
};

static struct LM_SENSOR_T * const lm_sensor_base[HAL_LM_SENSOR_ID_NUM] = {
    (struct LM_SENSOR_T *)LM_SENSOR0_BASE,
#ifdef LM_SENSOR1_BASE
    (struct LM_SENSOR_T *)LM_SENSOR1_BASE,
#endif
};

static const IRQn_Type irq_num[HAL_LM_SENSOR_ID_NUM] = {
    LM_SENSOR0_IRQn,
#ifdef LM_SENSOR1_BASE
    LM_SENSOR1_IRQn,
#endif
};

static HAL_LM_SENSOR_IRQ_HANDLER_T irq_handler[HAL_LM_SENSOR_ID_NUM];
static enum HAL_LM_SENSOR_STATE_T lm_sensor_state[HAL_LM_SENSOR_ID_NUM] = {HAL_LM_SENSOR_NULL,};

static void hal_lm_sensor0_irq_handle(void);
#ifdef LM_SENSOR1_BASE
static void hal_lm_sensor1_irq_handle(void);
#endif

static const uint32_t irq_entry[HAL_LM_SENSOR_ID_NUM] = {
    (uint32_t)hal_lm_sensor0_irq_handle,
#ifdef LM_SENSOR1_BASE
    (uint32_t)hal_lm_sensor1_irq_handle,
#endif
};

static void hal_lm_sensor_irq_handle(enum HAL_LM_SENSOR_ID_T id)
{
    uint32_t status;
    uint32_t pulse_cnt;
    float temp;

    status = lm_sensor_base[id]->SENS_INT;

    lm_sensor_base[id]->SENS_INTMSK |= SENSOR_DONE_INTR_CLR;
    lm_sensor_base[id]->SENS_PWR |= CNT_CLR;

    pulse_cnt = ((status & PULSE_CNT_OK_MASK) >> PULSE_CNT_OK_SHIFT);

    temp = (float)pulse_cnt * MAX_TEMP_RANGE/MAX_PULSE_CNT + BASE_TEMP_VALUE;

    if (irq_handler[id]) {
        irq_handler[id](id, temp, status);
    }
}

static void hal_lm_sensor0_irq_handle(void)
{
    hal_lm_sensor_irq_handle(HAL_LM_SENSOR_ID_0);
}

#ifdef LM_SENSOR1_BASE
static void hal_lm_sensor1_irq_handle(void)
{
    hal_lm_sensor_irq_handle(HAL_LM_SENSOR_ID_1);
}
#endif

int hal_lm_sensor_enable(enum HAL_LM_SENSOR_ID_T id, HAL_LM_SENSOR_IRQ_HANDLER_T cb)
{
    if ((id >= HAL_LM_SENSOR_ID_NUM) || (id < HAL_LM_SENSOR_ID_0)) {
        return HAL_LM_SENSOR_ERRCODE_INVALID_ID;
    }

    if (lm_sensor_state[id] == HAL_LM_SENSOR_ENABLE) {
        return HAL_LM_SENSOR_ERRCODE_STATE_ERR;
    }

    if (id == HAL_LM_SENSOR_ID_0) {
        hal_iomux_set_lm_sensor0();
#ifdef LM_SENSOR1_BASE
    } else if (id == HAL_LM_SENSOR_ID_1) {
        hal_iomux_set_lm_sensor1();
#endif
    }

    lm_sensor_state[id] = HAL_LM_SENSOR_ENABLE;

    hal_cmu_clock_enable(cmu_mode_table[id]);
    hal_cmu_reset_clear(cmu_mode_table[id]);

    lm_sensor_base[id]->CNT_THRESHOLD = CNT_THRESHOLD(DEFAULT_CNT_THRESHOLD);
    lm_sensor_base[id]->PULSE_CNT_THRESHOLD = PULSE_CNT_THRESHOLD(DEFAULT_PULSE_CNT_THRESHOLD);
    lm_sensor_base[id]->SENS_INTMSK = SENSOR_DONE_INTR_EN | SENSOR_DONE_INTR_MASK;

    irq_handler[id] = cb;
    NVIC_SetVector(irq_num[id], irq_entry[id]);
    NVIC_SetPriority(irq_num[id], IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(irq_num[id]);
    NVIC_EnableIRQ(irq_num[id]);

    lm_sensor_base[id]->SENS_PWR = SENSOR_PWR;

    return HAL_LM_SENSOR_ERRCODE_NULL;
}

int hal_lm_sensor_disable(enum HAL_LM_SENSOR_ID_T id)
{
    if (id >= HAL_LM_SENSOR_ID_NUM) {
        return HAL_LM_SENSOR_ERRCODE_INVALID_ID;
    }

    if ((lm_sensor_state[id] == HAL_LM_SENSOR_NULL) || (lm_sensor_state[id] == HAL_LM_SENSOR_DISABLE)) {
        return HAL_LM_SENSOR_ERRCODE_STATE_ERR;
    }

    NVIC_DisableIRQ(irq_num[id]);
    NVIC_ClearPendingIRQ(irq_num[id]);
    irq_handler[id] = NULL;

    lm_sensor_base[id]->SENS_PWR = ~(SENSOR_PWR | CNT_CLR);

    hal_cmu_reset_set(cmu_mode_table[id]);
    hal_cmu_clock_disable(cmu_mode_table[id]);

    if (id == HAL_LM_SENSOR_ID_0) {
        hal_iomux_clear_lm_sensor0();
#ifdef LM_SENSOR1_BASE
    } else if (id == HAL_LM_SENSOR_ID_1) {
        hal_iomux_clear_lm_sensor1();
#endif
    }

    lm_sensor_state[id] = HAL_LM_SENSOR_DISABLE;

    return HAL_LM_SENSOR_ERRCODE_NULL;
}

#endif //LM_SENSOR0_BASE

