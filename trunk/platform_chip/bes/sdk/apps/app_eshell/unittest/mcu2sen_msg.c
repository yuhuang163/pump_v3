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

#if defined(UTILS_ESHELL_EN) && defined (SENSOR_HUB_TEST) && defined(CORE_BRIDGE_DEMO_MSG)
#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "hal_gpio.h"
#include "hal_sleep.h"
#include "cmsis_os2.h"
#include "eshell.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "cmsis_os.h"
#include "hal_sysfreq.h"
//#include "pmu_best1600.h"
#include "mcu_sensor_hub_app.h"

static void app_test_mcu_sensorhub_demo_req_with_rsp(int argc, char *argv[])
{
   app_mcu_sensor_hub_send_demo_req_with_rsp();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_sensor_req_with_rsp", "usage: utest_sensor_req_with_rsp help",
                   app_test_mcu_sensorhub_demo_req_with_rsp);


static void app_test_mcu_sensorhub_demo_instant_req(int argc, char *argv[])
{
    app_mcu_sensor_hub_send_demo_instant_req();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_sensor_instant_req", "usage: utest_sensor_instant_req help",
                   app_test_mcu_sensorhub_demo_instant_req);


static void app_test_mcu_sensorhub_demo_req_no_rsp(int argc, char *argv[])
{
   app_mcu_sensor_hub_send_demo_req_no_rsp();
}


ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_sensor_req_no_rsp", "usage: utest_sensor_req_no_rsp help",
                  app_test_mcu_sensorhub_demo_req_no_rsp);

#endif // UTILS_ESHELL_EN
