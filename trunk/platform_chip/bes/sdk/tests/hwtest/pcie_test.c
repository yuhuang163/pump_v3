/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifdef PCIE_TEST

#include "string.h"
#include "hal_spi.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "cmsis.h"
#include "pciephy.h"
#include "reg_pcie.h"
#include "hal_pcie.h"

void pcie_test(void)
{
    TRACE(0, "pcie_test");
    hal_pcie_open();
    // hal_pcie_sel_wifi();
}

#endif
