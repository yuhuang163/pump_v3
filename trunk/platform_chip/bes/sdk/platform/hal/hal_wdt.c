/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "plat_addr_map.h"

#ifdef WDT_BASE

#include "plat_types.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_uart.h"
#include "hal_wdt.h"
#include "hal_location.h"

#if defined(CHIP_BEST3001) || defined(CHIP_BEST3003) || defined(CHIP_BEST3005) || defined(CHIP_BEST1400) || defined(CHIP_BEST1402)
#define CLOCK_SYNC_WORKAROUND
#endif

#if defined(PSRAM_INIT_CHECK)
#define WDT_BSS_LOC                         BOOT_BSS_LOC
#define WDT_TEXT_LOC                        BOOT_TEXT_FLASH_LOC
#else
#define WDT_BSS_LOC
#define WDT_TEXT_LOC
#endif

#define SLOW_TIMER_VAL_DELTA                1
#define FAST_TIMER_VAL_DELTA                20

/* wdt controller */
/* reg address */
/* default timeout in seconds */
#define DEFAULT_TIMEOUT                     60

/* watchdog register offsets and masks */
#define WDTLOAD_REG_OFFSET                  0x000
#define WDTLOAD_LOAD_MIN                    0x00000001
#define WDTLOAD_LOAD_MAX                    0xFFFFFFFF

#define WDTVALUE_REG_OFFSET                 0x004

#define WDTCONTROL_REG_OFFSET               0x008
#define WDTCONTROL_REG_INT_ENABLE           (1 << 0)
#define WDTCONTROL_REG_RESET_ENABLE         (1 << 1)

#define WDTINTCLR_REG_OFFSET                0x00C

#define WDTRIS_REG_OFFSET                   0x010
#define WDTRIS_REG_INT_MASK                 (1 << 0)

#define WDTMIS_REG_OFFSET                   0x014
#define WDTMIS_REG_INT_MASK                 (1 << 0)

#define WDTLOCK_REG_OFFSET                  0xC00
#define WDTLOCK_REG_UNLOCK                  0x1ACCE551
#define WDTLOCK_REG_LOCK                    0x00000001

/* read write */
#define wdtip_write32(v,b,a) \
    (*((volatile unsigned int *)(b+a)) = v)
#define wdtip_read32(b,a) \
    (*((volatile unsigned int *)(b+a)))

typedef void (*HAL_WDT_IRQ_HANDLER)(void);

struct HAL_WDT_CTX {
    unsigned int load_val;
};

struct HAL_WDT_CTX WDT_BSS_LOC hal_wdt_ctx[HAL_WDT_ID_NUM];
static void _wdt1_irq_handler(void);
HAL_WDT_IRQ_HANDLER hal_wdt_irq_handler[HAL_WDT_ID_NUM] =
{
    _wdt1_irq_handler,
};
HAL_WDT_IRQ_CALLBACK hal_wdt_irq_callback[HAL_WDT_ID_NUM];

static void _wdt1_irq_handler(void)
{
    if (hal_wdt_irq_callback[HAL_WDT_ID_0] != 0) {
        hal_wdt_irq_callback[HAL_WDT_ID_0](HAL_WDT_ID_0, HAL_WDT_EVENT_FIRE);
    }
}

static unsigned int _wdt_get_base(enum HAL_WDT_ID_T id)
{
    switch(id) {
        default:
        case HAL_WDT_ID_0:
            return WDT_BASE;
            break;
    }

    return 0;
}

static void WDT_TEXT_LOC _wdt_load(enum HAL_WDT_ID_T id)
{
    unsigned int reg_base = 0;
    struct HAL_WDT_CTX *wdt = &hal_wdt_ctx[id];
    uint32_t lock;

    reg_base = _wdt_get_base(id);

    lock = int_lock();

    wdtip_write32(WDTLOCK_REG_UNLOCK, reg_base, WDTLOCK_REG_OFFSET);

#ifdef CLOCK_SYNC_WORKAROUND
    uint32_t val;

    do {
        wdtip_write32(wdt->load_val, reg_base, WDTLOAD_REG_OFFSET);
        val = wdtip_read32(reg_base, WDTVALUE_REG_OFFSET);
    } while ((wdt->load_val < val) || (wdt->load_val > val + SLOW_TIMER_VAL_DELTA));
#else
    wdtip_write32(wdt->load_val, reg_base, WDTLOAD_REG_OFFSET);
#endif

    wdtip_write32(WDTLOCK_REG_LOCK, reg_base, WDTLOCK_REG_OFFSET);

    int_unlock(lock);

    /* Flush posted writes. */
    wdtip_read32(reg_base, WDTLOCK_REG_OFFSET);
}

static void WDT_TEXT_LOC _wdt_config(enum HAL_WDT_ID_T id, int reset)
{
    unsigned int reg_base = 0;
    uint32_t lock;

    reg_base = _wdt_get_base(id);

    lock = int_lock();
    wdtip_write32(WDTLOCK_REG_UNLOCK, reg_base, WDTLOCK_REG_OFFSET);
    wdtip_write32(WDTRIS_REG_INT_MASK, reg_base, WDTINTCLR_REG_OFFSET);
    wdtip_write32(WDTCONTROL_REG_INT_ENABLE | (reset ? WDTCONTROL_REG_RESET_ENABLE : 0), reg_base, WDTCONTROL_REG_OFFSET);
    wdtip_write32(WDTLOCK_REG_LOCK, reg_base, WDTLOCK_REG_OFFSET);
    int_unlock(lock);

    /* Flush posted writes. */
    wdtip_read32(reg_base, WDTLOCK_REG_OFFSET);
}

static int WDT_BSS_LOC wdt_start;

/* mandatory operations */
int WDT_TEXT_LOC hal_wdt_start(enum HAL_WDT_ID_T id)
{
    _wdt_load(id);
    _wdt_config(id, true);
    wdt_start = 1;
    return 0;
}

int hal_wdt_start_without_reset(enum HAL_WDT_ID_T id)
{
    _wdt_load(id);
    _wdt_config(id, false);
    wdt_start = 1;
    return 0;
}

int hal_wdt_stop(enum HAL_WDT_ID_T id)
{
    unsigned int reg_base = 0;
    uint32_t lock;

    reg_base = _wdt_get_base(id);

    lock = int_lock();
    wdtip_write32(WDTLOCK_REG_UNLOCK, reg_base, WDTLOCK_REG_OFFSET);
    wdtip_write32(0, reg_base, WDTCONTROL_REG_OFFSET);
    wdtip_write32(WDTLOCK_REG_LOCK, reg_base, WDTLOCK_REG_OFFSET);
    int_unlock(lock);

    /* Flush posted writes. */
    wdtip_read32(reg_base, WDTLOCK_REG_OFFSET);

    wdt_start = 0;
    return 0;
}

void hal_wdt_disable_irq(enum HAL_WDT_ID_T id)
{
    unsigned int reg_base = 0;
    uint32_t lock;
    uint32_t val;

    if (!wdt_start)
        return;

    reg_base = _wdt_get_base(id);

    lock = int_lock();
    wdtip_write32(WDTLOCK_REG_UNLOCK, reg_base, WDTLOCK_REG_OFFSET);
    val = wdtip_read32(reg_base, WDTCONTROL_REG_OFFSET);
    val &= ~WDTCONTROL_REG_INT_ENABLE;
    wdtip_write32(val, reg_base, WDTCONTROL_REG_OFFSET);
    wdtip_write32(WDTLOCK_REG_LOCK, reg_base, WDTLOCK_REG_OFFSET);
    int_unlock(lock);
}

/* optional operations */
int hal_wdt_ping(enum HAL_WDT_ID_T id)
{
    if (wdt_start) {
        _wdt_load(id);
    }

    return 0;
}

int WDT_TEXT_LOC hal_wdt_set_timeout_ms(enum HAL_WDT_ID_T id, unsigned int ms)
{
    uint64_t load;
    struct HAL_WDT_CTX *wdt = &hal_wdt_ctx[id];

    /*
     * sp805 runs counter with given value twice, after the end of first
     * counter it gives an interrupt and then starts counter again. If
     * interrupt already occurred then it resets the system. This is why
     * load is half of what should be required.
     */
    load = MS_TO_TICKS(ms);
    if (load) {
        load -= 1;
    }

    load = (load > WDTLOAD_LOAD_MAX) ? WDTLOAD_LOAD_MAX: load;
    load = (load < WDTLOAD_LOAD_MIN) ? WDTLOAD_LOAD_MIN: load;

    wdt->load_val = load;

    return 0;
}

unsigned int hal_wdt_get_timeleft_ms(enum HAL_WDT_ID_T id)
{
    uint64_t load;
    unsigned int reg_base = 0;

    reg_base = _wdt_get_base(id);

    load = wdtip_read32(reg_base, WDTVALUE_REG_OFFSET);

    return TICKS_TO_MS(load);
}

int WDT_TEXT_LOC hal_wdt_set_timeout(enum HAL_WDT_ID_T id, unsigned int sec)
{
    return hal_wdt_set_timeout_ms(id, sec * 1000);
}

unsigned int hal_wdt_get_timeleft(enum HAL_WDT_ID_T id)
{
    return (hal_wdt_get_timeleft_ms(id) + 500) / 1000;
}

void hal_wdt_set_irq_callback(enum HAL_WDT_ID_T id, HAL_WDT_IRQ_CALLBACK cb)
{
    switch(id) {
        default:
        case HAL_WDT_ID_0:
            NVIC_SetNMIHandler(hal_wdt_irq_handler[id]);
            NVIC_SetVector(WDT_IRQn, (uint32_t)hal_wdt_irq_handler[id]);
            NVIC_SetPriority(WDT_IRQn, IRQ_PRIORITY_REALTIME);
            NVIC_ClearPendingIRQ(WDT_IRQn);
            NVIC_EnableIRQ(WDT_IRQn);
        break;
    }

    hal_wdt_irq_callback[id] = cb;
}

void hal_wdt_irq_clr(enum HAL_WDT_ID_T id)
{
    unsigned int reg_base = 0;
    uint32_t lock;

    reg_base = _wdt_get_base(id);

    lock = int_lock();
    wdtip_write32(WDTLOCK_REG_UNLOCK, reg_base, WDTLOCK_REG_OFFSET);
    wdtip_write32(WDTRIS_REG_INT_MASK, reg_base, WDTINTCLR_REG_OFFSET);
    wdtip_write32(WDTLOCK_REG_LOCK, reg_base, WDTLOCK_REG_OFFSET);
    int_unlock(lock);

    /* Flush posted writes. */
    wdtip_read32(reg_base, WDTLOCK_REG_OFFSET);
}

WEAK void hal_cmu_wdt_sleep(void)
{
}

WEAK void hal_cmu_wdt_wakeup(void)
{
}

void hal_wdt_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
#ifdef CORE_SLEEP_POWER_DOWN
    if (mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        // TODO: Save wdt regs
    }
#endif
    // Change IRQ to normal external IRQ if it is NMI IRQ
    hal_cmu_wdt_sleep();
}

void hal_wdt_wakeup(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
#ifdef CORE_SLEEP_POWER_DOWN
    if (mode == HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        // TODO: Restore wdt regs
    }
#endif
    // Change IRQ back to NMI IRQ if it is NMI IRQ at initialization
    hal_cmu_wdt_wakeup();
}

#endif
