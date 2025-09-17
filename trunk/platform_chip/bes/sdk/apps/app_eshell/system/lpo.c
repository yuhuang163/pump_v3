#if defined(UTILS_ESHELL_EN) && defined(CHIP_BEST1502X) && defined(LED1_32K_LPO_ENABLE)

#include <stdio.h>
#include <stdarg.h>
#include "plat_types.h"
#include "string.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "eshell.h"
#include "app_eshell.h"
#include "hal_timer.h"
#include "hal_iomux.h"

static void sys_lpo_clock_calib_freq(int argc, char *argv[])
{
    uint32_t lpo_freq;
    bool flag=true;
    uint32_t time = atoi(argv[2]);
    hal_iomux_tportopen();
    if((strncmp(argv[1], "start", 5) == 0)){
        for (uint8_t i = 0; i < time; i++){
            if (flag == true) {
                hal_sys_timer_calib();
                lpo_freq = hal_sys_timer_systick_hz();
                hal_iomux_tportset(0);
                eshell_putstring("gpio_set_lpo_calib freq/%d\r\n", lpo_freq);
            } else {
                hal_sys_timer_calib();
                lpo_freq = hal_sys_timer_systick_hz();
                hal_iomux_tportclr(0);
                eshell_putstring("gpio_clr_lpo_calib freq/%d\r\n", lpo_freq);
            }
            flag = !flag;
            hal_sys_timer_delay(MS_TO_TICKS(1000));
        }
    }
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "lpo", "1502x led1 use external lpo clock calib and set gpio to sleep or wakeup ", sys_lpo_clock_calib_freq);

#endif

