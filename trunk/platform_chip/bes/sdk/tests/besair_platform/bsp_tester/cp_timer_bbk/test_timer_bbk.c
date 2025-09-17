#if defined(TEST_TIMER_BBK) && defined(CHIP_ROLE_CP)
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_timer_fast_irq.h"
#include "cmsis_os.h"
#include "cp_subsys.h"
#include "hal_sleep.h"

#define HAL_CPU_WAKE_LOCK_USER_CP HAL_CPU_WAKE_LOCK_USER_7
#define GPIO_PIN_OUT        HAL_GPIO_PIN_P1_0
#define GPIO_IRQ_PIN_IN     HAL_GPIO_PIN_P0_0


uint8_t ipc_buf[2] = {0x01};

#ifndef NO_TIMER
void fast_timer_irq_handler(void *param)
{
    static uint8_t cnt = 100;
    if (cnt == 0) {
        cnt = 100;
        hal_fast_timer_stop();
        hal_cpu_wake_unlock(HAL_CPU_WAKE_LOCK_USER_CP);
        ipc_buf[1] = HAL_CMU_FREQ_32K;
        uint32_t ret = cp_subsys_send(ipc_buf, sizeof(ipc_buf));
        ASSERT(ret == 0, "cp_subsys_send failed: %d", ret);
    } else {
        hal_fast_timer_start(MS_TO_FAST_TICKS(1));
        cnt--;
        hal_gpio_pin_set(GPIO_PIN_OUT);
        hal_gpio_pin_clr(GPIO_PIN_OUT);
    }
}
#endif

#ifdef TIMER_TRIGGER
static void cp_timer_gpio_irq_handler(void const *param);
osTimerDef(CP_OSTIMER, cp_timer_gpio_irq_handler);
static osTimerId cp_ostimer = NULL;
#endif

static void cp_timer_gpio_irq_handler(void const *param)
{
    // TRACE(0, "[%s] %d", __func__, __LINE__);
    ipc_buf[1] = HAL_CMU_FREQ_104M;
    uint32_t ret = cp_subsys_send(ipc_buf, sizeof(ipc_buf));
    ASSERT(ret == 0, "cp_subsys_send failed: %d", ret);
    hal_cpu_wake_lock(HAL_CPU_WAKE_LOCK_USER_CP);
    hal_fast_timer_start(MS_TO_FAST_TICKS(1));
}

void cp_timer_test(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
#ifdef GPIO_TRIGGER
        {GPIO_IRQ_PIN_IN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#endif
        {GPIO_PIN_OUT, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(GPIO_PIN_OUT, HAL_GPIO_DIR_OUT, 0);
    hal_fast_timer_setup(HAL_TIMER_TYPE_ONESHOT, fast_timer_irq_handler);

#ifdef  TIMER_TRIGGER
    if (!cp_ostimer) {
        cp_ostimer = osTimerCreate(osTimer(CP_OSTIMER), osTimerPeriodic, NULL);
        if (cp_ostimer) {
            osTimerStart(cp_ostimer, 10000);
        }
    }

#elif defined GPIO_TRIGGER
    hal_gpio_pin_set_dir(GPIO_IRQ_PIN_IN, HAL_GPIO_DIR_IN, 1);
    struct HAL_GPIO_IRQ_CFG_T cfg = {0,};
    cfg.irq_enable = true;
    if (cfg.irq_enable) {
        cfg.irq_handler = cp_timer_gpio_irq_handler;
    } else {
        cfg.irq_handler = NULL;
    }
    cfg.irq_debounce = false;
    cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
    cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    hal_gpio_setup_irq(GPIO_IRQ_PIN_IN, &cfg);
#endif

}

#endif
