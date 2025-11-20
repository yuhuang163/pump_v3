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
#ifdef POWER_PROTECT_TEST
#include "hal_timer.h"
#include "hal_trace.h"
#include "pmu.h"

// make T=noapp_test -j CURRENT_TEST=1 CHIP=best1603 BTH_AS_MAIN_MCU=1 CHIP_SUBSYS=bth POWER_PROTECT_TEST=1 PMU_POWER_PROTECT_DYNAMIC_CONFIG=1

static struct PMU_POWER_PROTECT_TYPE_T power_protect_cfg_base = {
    .ov_det_en = false,
    .uv_det_en = false,
    .osc_det_en = false,
    .irq_en = true,
};

static void pmu_power_protect_irq_handler(enum PMU_POWER_PROTECT_MODULE_T module, uint8_t source)
{
    TRACE(0, "%s: ##abort happend! module=%d source=0x%02X", __func__, module, source);
}

static void pmu_power_protect_all_module_config(void)
{
    enum PMU_POWER_PROTECT_MODULE_T module;
    enum PMU_POWER_PROTECT_HW_TYPE_T hw_type = PMU_POWER_PROTECT_HW_TYPE_LDO_OFF;
#if 1
    enum PMU_POWER_PROTECT_MODE_T mode = PMU_POWER_PROTECT_MODE_SW;
#else
    enum PMU_POWER_PROTECT_MODE_T mode = PMU_POWER_PROTECT_MODE_HW;

    hw_type = PMU_POWER_PROTECT_HW_TYPE_PMU_OFF;
#endif
    enum PMU_POWER_PROTECT_RET_T ret;
    struct PMU_POWER_PROTECT_TYPE_T power_protect_cfg_act = {0, };

    if (mode == PMU_POWER_PROTECT_MODE_HW) {
        TRACE(0, "%s: Set hw mode, hw_type:%s", __func__, hw_type == PMU_POWER_PROTECT_HW_TYPE_PMU_OFF ? "pmu off" : "ldo off");
    } else {
        TRACE(0, "%s: Set sw mode.", __func__);
    }

    for (module = PMU_POWER_PROTECT_MODULE_LDO_VBAT2VCORE; module < PMU_POWER_PROTECT_MODULE_QTY; module++) {
        power_protect_cfg_act = power_protect_cfg_base;

        // Support OV module
        if (((module != PMU_POWER_PROTECT_MODULE_LDO_VMEM2) && (module != PMU_POWER_PROTECT_MODULE_LDO_VMEM3)) && module < PMU_POWER_PROTECT_MODULE_VBAT) {
            power_protect_cfg_act.ov_det_en = true;
        }

        // Support UV module
        if (!(module == PMU_POWER_PROTECT_MODULE_VBAT || module == PMU_POWER_PROTECT_MODULE_OTP)) {
            power_protect_cfg_act.uv_det_en = true;
        }

        // Support OSCP module
        if (module < PMU_POWER_PROTECT_MODULE_VBAT) {
            power_protect_cfg_act.osc_det_en = true;
        }

        ret =  pmu_power_protect_config(module, mode, hw_type, &power_protect_cfg_act);
        if (ret == PMU_POWER_PROTECT_RET_INVALID_TEMP) {
            TRACE(0, "%s: Continue, module:%d", __func__, module);
            continue;
        }
        if (ret) {
            ASSERT(false, "%s module:%d oscp_en:%d ov_en:%d uv_en:%d ret:%d",
                __func__, module, power_protect_cfg_act.osc_det_en, power_protect_cfg_act.ov_det_en, power_protect_cfg_act.uv_det_en, ret);
        } else {
            TRACE(0, "%s: module:%02d oscp_en:%d ov_en:%d uv_en:%d",
                __func__, module, power_protect_cfg_act.osc_det_en, power_protect_cfg_act.ov_det_en, power_protect_cfg_act.uv_det_en);
        }
    }
}

void power_protect_test(void)
{
    enum PMU_POWER_PROTECT_RET_T ret;
    uint32_t cnt = 0;

    ret = pmu_power_protect_irq_handler_set(pmu_power_protect_irq_handler);
    if (ret) {
        ASSERT(false, "pmu_power_protect_irq_handler_set fail! ret=%d", ret);
    }

    // Set all module work with software mode.
    pmu_power_protect_all_module_config();

    while(1) {
        TRACE(0, "alive! cnt=%d", cnt);
        cnt++;
        hal_sys_timer_delay(MS_TO_TICKS(1000 * 3));
    }
}
#endif
