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
#ifdef CMU_FREE_TIMER
#include CHIP_SPECIFIC_HDR(reg_aoncmu)
#endif
#ifdef CMU_FREE_FAST_TIMER
#ifdef CHIP_SUBSYS_BTH
#include CHIP_SPECIFIC_HDR(reg_bthcmu)
#elif defined(CHIP_SUBSYS_SENS)
#include CHIP_SPECIFIC_HDR(reg_senscmu)
#elif defined(CHIP_SUBSYS_CSP)
#include CHIP_SPECIFIC_HDR(reg_cspcmu)
#else
#include CHIP_SPECIFIC_HDR(reg_cmu)
#endif
#endif // CMU_FREE_FAST_TIMER
#include "cmsis_nvic.h"
#include "dwt.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_sleep.h"
#define IGNORE_HAL_TIMER_RAW_API_CHECK
#include "hal_timer_raw.h"
#include "hal_timer_fast_irq.h"
#include "hal_timer_user_irq.h"
#include "hal_timer.h"
#include "reg_timer.h"
#include "hal_trace.h"

#if defined(CHIP_BEST3001) || defined(CHIP_BEST3003) || defined(CHIP_BEST3005) || defined(CHIP_BEST1400) || defined(CHIP_BEST1402)
#define CLOCK_SYNC_WORKAROUND
#endif

#if defined(TIMER1_BASE) && defined(LOW_SYS_FREQ) && defined(LOW_SYS_FREQ_6P5M)
#if defined(CHIP_BEST1305) || \
        defined(CHIP_BEST1501) || defined(CHIP_BEST1501P) || defined(CHIP_BEST1502X) || \
        defined(CHIP_BEST2001) || defined(CHIP_BEST2003) || \
        defined(CHIP_BEST2300A) || defined(CHIP_BEST2300P)
#define FAST_TIMER_WORKAROUND
#endif
#endif

#if defined(CALIB_SLOW_TIMER) && defined(SLOW_TIMER_USE_EXT_XTAL) && !defined(TIMER_USE_FPU)
#define TIMER_USE_FPU
#endif

#if !(defined(__FPU_USED) && (__FPU_USED == 1))
#undef TIMER_USE_FPU
#endif

#ifndef SLOW_TIMER_SUB_ID_WITH_IRQ
#define SLOW_TIMER_SUB_ID_WITH_IRQ  1
#endif

#ifndef FAST_TIMER_SUB_ID_WITH_IRQ
#define FAST_TIMER_SUB_ID_WITH_IRQ  1
#endif
#if (FAST_TIMER_SUB_ID_WITH_IRQ == 0) && !defined(CMU_FREE_FAST_TIMER)
#error "FAST_TIMER_SUB_ID_WITH_IRQ can be 0 only if CMU_FREE_FAST_TIMER defined"
#endif

#ifndef USER_TIMER_ID_WITH_IRQ
#define USER_TIMER_ID_WITH_IRQ      2
#endif

#ifndef SLOW_FREE_TIMER_ID
#define SLOW_FREE_TIMER_ID          0
#endif

#ifndef USER_TIMER_SUB_ID_WITH_IRQ
#define USER_TIMER_SUB_ID_WITH_IRQ  0
#endif

#define SLOW_TIMER_VAL_DELTA        1
#define SLOW_TIMER_VAL_DELTA_SLEEP  10
#define FAST_TIMER_VAL_DELTA        20

#ifdef CALIB_SLOW_TIMER
#define MAX_CALIB_SYSTICK_HZ        (CONFIG_SYSTICK_HZ_NOMINAL * 2)

#define MIN_CALIB_TICKS             (30 * (CONFIG_SYSTICK_HZ_NOMINAL / 1000))

#define MAX_CALIB_TICKS             (30 * CONFIG_SYSTICK_HZ_NOMINAL)

static
#ifdef TIMER_USE_FPU
    float
#else
    uint32_t
#endif
    BOOT_DATA_LOC sys_tick_hz = CONFIG_SYSTICK_HZ_NOMINAL;
static uint32_t BOOT_BSS_LOC slow_val;
static uint32_t BOOT_BSS_LOC fast_val;
#endif

#ifdef FAST_TIMER_COMPENSATE
#if !defined(TIMER1_BASE) && !defined(CMU_FREE_FAST_TIMER)
#error "FAST_TIMER_COMPENSATE should work with TIMER1_BASE or CMU_FREE_FAST_TIMER"
#endif
#if !(defined(__FPU_USED) && (__FPU_USED == 1))
#error "FAST_TIMER_COMPENSATE need FPU"
#endif
#ifdef FAST_TIMER_WORKAROUND
#error "FAST_TIMER_COMPENSATE conflicts with FAST_TIMER_WORKAROUND"
#endif
#define FAST_TICK_RATIO_NUM                 3
static float BOOT_BSS_LOC fast_tick_ratio_avg;
static float BOOT_BSS_LOC fast_tick_ratio[FAST_TICK_RATIO_NUM];
#endif

static struct DUAL_TIMER_T * const BOOT_RODATA_SRAM_LOC dual_timer[] = {
    (struct DUAL_TIMER_T *)TIMER0_BASE,
#ifdef TIMER1_BASE
    (struct DUAL_TIMER_T *)TIMER1_BASE,
#endif
#ifdef TIMER2_BASE
    (struct DUAL_TIMER_T *)TIMER2_BASE,
#endif
};

#ifdef CMU_FREE_TIMER
#if !defined(TIMER1_BASE) && !defined(CMU_FREE_FAST_TIMER)
#error "CMU_FREE_TIMER should work with TIMER1_BASE or CMU_FREE_FAST_TIMER"
#endif
#ifdef CLOCK_SYNC_WORKAROUND
#error "CMU_FREE_TIMER conflicts with CLOCK_SYNC_WORKAROUND"
#endif
#ifdef FAST_TIMER_WORKAROUND
#error "CMU_FREE_TIMER conflicts with FAST_TIMER_WORKAROUND"
#endif
static struct AONCMU_T * const aoncmu = (struct AONCMU_T *)AON_CMU_BASE;
#endif

#ifdef CMU_FREE_FAST_TIMER
#ifdef CHIP_SUBSYS_BTH
static struct BTHCMU_T * const cmu = (struct BTHCMU_T *)BTH_CMU_BASE;
#elif defined(CHIP_SUBSYS_CSP)
static struct CSPCMU_T * const cmu = (struct CSPCMU_T *)CSP_CMU_BASE;
#elif defined(CHIP_SUBSYS_SENS)
static struct SENSCMU_T * const cmu = (struct SENSCMU_T *)SENS_CMU_BASE;
#else
static struct CMU_T * const cmu = (struct CMU_T *)CMU_BASE;
#endif
#endif // CMU_FREE_FAST_TIMER

static const IRQn_Type irq_num[ARRAY_SIZE(dual_timer)][2] = {
    { TIMER00_IRQn, TIMER01_IRQn, },

#ifdef TIMER1_BASE
    { TIMER10_IRQn, TIMER11_IRQn, },
#endif

#ifdef TIMER2_BASE
#if defined(ASIC_SIMU)
    { INVALID_IRQn, INVALID_IRQn, },
#else
    { TIMER20_IRQn, TIMER21_IRQn, },
#endif
#endif
};

static HAL_TIMER_IRQ_HANDLER_T irq_handler[ARRAY_SIZE(dual_timer)][2];

static uint32_t start_time[ARRAY_SIZE(dual_timer)][2];

#ifdef CHIP_BEST1700
static uint32_t slow_timer_load_time;
#endif

static uint32_t hal_timer_common_get_elapsed_time(uint32_t id, uint32_t sub_id);
static int hal_timer_common_irq_pending(uint32_t id, uint32_t sub_id);

__STATIC_FORCEINLINE uint32_t get_timer_value(struct TIMER_T *timer, uint32_t delta)
{
#ifdef CLOCK_SYNC_WORKAROUND
    uint32_t lock;
    uint32_t v1, v2;

    lock = int_lock();
    do {
        v1 = timer->Value;
        v2 = timer->Value;
    } while ((v1 < v2) || (v1 > v2 + delta));
    int_unlock(lock);

    return v2;
#else
    return timer->Value;
#endif
}

__STATIC_FORCEINLINE void clear_timer_irq(struct TIMER_T *timer)
{
#ifdef CLOCK_SYNC_WORKAROUND
    do {
        timer->IntClr = 1;
    } while (timer->RIS & TIMER_RIS_RIS);
#else
    timer->IntClr = 1;
#endif
}

__STATIC_FORCEINLINE void set_timer_load(struct TIMER_T *timer, uint32_t load, uint32_t delta)
{
#ifdef CLOCK_SYNC_WORKAROUND
    uint32_t lock;
    uint32_t val;

    lock = int_lock();
    do {
        timer->Load = load;
        val = timer->Value;
    } while ((load < val) || (load > val + delta));
    int_unlock(lock);
#else
    timer->Load = load;
#endif
}

__STATIC_FORCEINLINE void fast_sys_timer_open(void)
{
#if defined(CMU_FREE_TIMER) && !defined(NO_CMU_INIT)
    hal_cmu_fast_timer_init();
#endif

#ifdef TIMER1_BASE
#ifndef NO_CMU_INIT
    hal_cmu_timer1_select_fast();
#endif
#ifndef CMU_FREE_FAST_TIMER
    dual_timer[1]->timer[0].Control = TIMER_CTRL_EN | TIMER_CTRL_PRESCALE_DIV_1 | TIMER_CTRL_SIZE_32_BIT;
#endif
#endif

#ifdef TIMER2_BASE
#ifndef NO_CMU_INIT
    hal_cmu_timer2_select_fast();
#endif
#if defined(ASIC_SIMU)
    dual_timer[2]->timer[0].Control = TIMER_CTRL_EN | TIMER_CTRL_PRESCALE_DIV_1 | TIMER_CTRL_SIZE_32_BIT;
#endif
#endif
}

static uint8_t timer_inited = false;

uint8_t hal_sys_timer_opened(void)
{
    return timer_inited;
}

void hal_sys_timer_set_opened(uint8_t en)
{
    timer_inited = en;
}

void BOOT_TEXT_FLASH_LOC hal_sys_timer_open(void)
{
#ifndef CMU_FREE_TIMER
#ifndef NO_CMU_INIT
    hal_cmu_timer0_select_slow();
#endif
    dual_timer[SLOW_FREE_TIMER_ID]->timer[0].Control = TIMER_CTRL_EN | TIMER_CTRL_PRESCALE_DIV_1 | TIMER_CTRL_SIZE_32_BIT;
#endif
    fast_sys_timer_open();
}

#ifdef CORE_SLEEP_POWER_DOWN
void SRAM_TEXT_LOC hal_sys_timer_wakeup(void)
{
    fast_sys_timer_open();
}
#endif

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_get(void)
{
#ifdef CMU_FREE_TIMER
    return aoncmu->FREE_TIMER;
#else
    return -get_timer_value(&dual_timer[SLOW_FREE_TIMER_ID]->timer[0], SLOW_TIMER_VAL_DELTA);
#endif
}

uint64_t SRAM_TEXT_LOC hal_sys_timer64_get(void)
{
#if defined(CMU_FREE_TIMER) && defined(CMU_FREE_TIMER64)
    uint32_t hi0;
    uint32_t hi1;
    uint32_t low;

    hi0 = aoncmu->FREE_TIMER_HI;
    low = aoncmu->FREE_TIMER;
    hi1 = aoncmu->FREE_TIMER_HI;
    if (hi0 != hi1) {
        low = aoncmu->FREE_TIMER;
    }
    return ((uint64_t)hi1 << 32) | low;
#else
    static uint64_t accum_tick = 0;
    static uint32_t last = 0;
    uint32_t cur;
    uint64_t result;
    uint32_t lock;

    lock = int_lock();
    cur = hal_sys_timer_get();
    if (cur < last) {
        accum_tick += (uint64_t)((0x80000000) * 2);
    }
    last = cur;
    result = accum_tick;
    int_unlock(lock);

    return (uint64_t)(result + cur);
#endif
}

uint32_t BOOT_TEXT_SRAM_LOC hal_aco_timer_get(void)
{
#if defined(TIMER2_BASE)
    return -get_timer_value(&dual_timer[2]->timer[0], SLOW_TIMER_VAL_DELTA);
#else
    return hal_sys_timer_get();
#endif
}

#ifdef CLOCK_SYNC_WORKAROUND
uint32_t SRAM_TEXT_LOC hal_sys_timer_get_in_sleep(void)
{
    return -get_timer_value(&dual_timer[SLOW_FREE_TIMER_ID]->timer[0], SLOW_TIMER_VAL_DELTA_SLEEP);
}
#else
uint32_t hal_sys_timer_get_in_sleep(void) __attribute__((alias("hal_sys_timer_get")));
#endif

uint32_t BOOT_TEXT_FLASH_LOC flash_hal_sys_timer_get(void)
{
#ifdef CMU_FREE_TIMER
    return aoncmu->FREE_TIMER;
#else
    return -get_timer_value(&dual_timer[SLOW_FREE_TIMER_ID]->timer[0], SLOW_TIMER_VAL_DELTA);
#endif
}

uint32_t hal_sys_ms_get(void)
{
    return GET_CURRENT_MS();
}

uint64_t hal_sys_ms64_get_safe(void)
{
    static uint64_t accum_ms = 0;
    static uint32_t last = 0;
    uint32_t cur;
    uint64_t result;
    uint32_t lock;

    lock = int_lock();
    cur = hal_sys_timer_get();
    if (cur < last) {
        accum_ms += TICKS_TO_MS64(0x80000000) * 2;
    }
    last = cur;
    result = accum_ms;
    int_unlock(lock);

    return result + TICKS_TO_MS64(cur);
}

uint64_t hal_sys_us64_get_safe(void)
{
    static uint64_t accum_us = 0;
    static uint32_t last = 0;
    uint32_t cur;
    uint64_t result;
    uint32_t lock;

    lock = int_lock();
    cur = hal_sys_timer_get();
    if (cur < last) {
        accum_us += TICKS_TO_US64(0x80000000) * 2;
    }
    last = cur;
    result = accum_us;
    int_unlock(lock);

    return result + TICKS_TO_US64(cur);
}

__STATIC_FORCEINLINE uint32_t _real_fast_sys_timer_get(void)
{
#ifdef CMU_FREE_FAST_TIMER
    return cmu->FAST_TIMER;
#elif defined(TIMER1_BASE)
    return -get_timer_value(&dual_timer[1]->timer[0], FAST_TIMER_VAL_DELTA);
#else
    return 0;
#endif
}

uint32_t BOOT_TEXT_SRAM_LOC hal_fast_sys_timer_get(void)
{
#ifdef FAST_TIMER_WORKAROUND
#ifdef TIMER_USE_FPU
    return (uint32_t)(hal_sys_timer_get() * ((float)CONFIG_FAST_SYSTICK_HZ / CONFIG_SYSTICK_HZ));
#else
    return (uint32_t)(hal_sys_timer_get() * (uint64_t)CONFIG_FAST_SYSTICK_HZ / CONFIG_SYSTICK_HZ);
#endif
#elif defined(TIMER1_BASE) || defined(CMU_FREE_FAST_TIMER)
    return _real_fast_sys_timer_get();
#else
    return 0;
#endif
}

static uint8_t fast_timer_wakeup[FAST_WAKEUP_QTY] = {0};

void hal_fast_sys_timer_flag_set(bool enable)
{
    for (uint8_t i=0; i<FAST_WAKEUP_QTY; i++) {
        fast_timer_wakeup[i] = enable;
    }
}

static bool hal_fast_sys_timer_flag_get(enum FAST_TIMER_WAKEUP_IDX idx)
{
    return fast_timer_wakeup[idx];
}

uint64_t SRAM_TEXT_LOC hal_fast_sys_timer64_get(void)
{
#if defined(CMU_FREE_FAST_TIMER) && defined(CMU_FREE_TIMER64)
    uint32_t hi0;
    uint32_t hi1;
    uint32_t low;

    hi0 = cmu->FAST_TIMER_HI;
    low = cmu->FAST_TIMER;
    hi1 = cmu->FAST_TIMER_HI;
    if (hi0 != hi1) {
        low = cmu->FAST_TIMER;
    }
    return ((uint64_t)hi1 << 32) | low;
#else
    static uint64_t accum_tick = 0;
    static uint32_t last = 0;
    uint32_t cur;
    uint64_t result;
    uint32_t lock;

    // 休眠唤醒后重置溢出次数
    if (hal_fast_sys_timer_flag_get(FAST_TIMER_TICK)) {
        accum_tick = 0;
        last = 0;
        fast_timer_wakeup[FAST_TIMER_TICK] = false;
    }

    lock = int_lock();
    cur = hal_fast_sys_timer_get();
    if (cur < last) {
        accum_tick += (uint64_t)(0x80000000) * 2;
    }
    last = cur;
    result = accum_tick;
    int_unlock(lock);

    return (uint64_t)(result + cur);
#endif
}

uint64_t hal_fast_sys_ms64_get_safe(void)
{
    static uint64_t accum_ms = 0;
    static uint32_t last = 0;
    uint32_t cur;
    uint64_t result;
    uint32_t lock;

    if (hal_fast_sys_timer_flag_get(FAST_TIMER_MS)) {
        accum_ms = 0;
        last = 0;
        fast_timer_wakeup[FAST_TIMER_MS] = false;
    }

    lock = int_lock();
    cur = hal_fast_sys_timer_get();
    if (cur < last) {
        accum_ms += FAST_TICKS_TO_MS64(0x80000000) * 2;
    }
    last = cur;
    result = accum_ms;
    int_unlock(lock);

    return result + FAST_TICKS_TO_MS64(cur);
}

uint64_t hal_fast_sys_us64_get_safe(void)
{
    static uint64_t accum_us = 0;
    static uint32_t last = 0;
    uint32_t cur;
    uint64_t result;
    uint32_t lock;

    if (hal_fast_sys_timer_flag_get(FAST_TIMER_US)) {
        accum_us = 0;
        last = 0;
        fast_timer_wakeup[FAST_TIMER_US] = false;
    }

    lock = int_lock();
    cur = hal_fast_sys_timer_get();
    if (cur < last) {
        accum_us += FAST_TICKS_TO_US64(0x80000000) * 2;
    }
    last = cur;
    result = accum_us;
    int_unlock(lock);

    return result + FAST_TICKS_TO_US64(cur);
}

__STATIC_FORCEINLINE void __real_fast_sys_timer_set(uint32_t val)
{
#ifdef CMU_FREE_FAST_TIMER
    cmu->FAST_TIMER_SET = val & ~0UL;
    hal_cmu_fast_timer_load();
#elif defined(TIMER1_BASE)
    set_timer_load(&dual_timer[1]->timer[0], val, FAST_TIMER_VAL_DELTA);
#endif
}

void SRAM_TEXT_LOC hal_fast_sys_timer_set(uint32_t val)
{
    __real_fast_sys_timer_set(val);
}

void SRAM_TEXT_LOC hal_fast_sys_timer64_set(uint64_t val)
{
#if defined(CMU_FREE_FAST_TIMER) && defined(CMU_FREE_TIMER64)
    cmu->FAST_TIMER_HI_SET = (val >> 32);
    cmu->FAST_TIMER_SET = val & ~0UL;
    hal_cmu_fast_timer_load();
#else
    hal_fast_sys_timer_set(val & ~0UL);
#endif
}

uint32_t hal_sys_timer_get_max(void)
{
    return 0xFFFFFFFF;
}

void BOOT_TEXT_SRAM_LOC hal_sys_timer_delay(uint32_t ticks)
{
    uint32_t start = hal_sys_timer_get();

    while (hal_sys_timer_get() - start < ticks);
}

#ifdef CLOCK_SYNC_WORKAROUND
void SRAM_TEXT_LOC hal_sys_timer_delay_in_sleep(uint32_t ticks)
{
    uint32_t start = hal_sys_timer_get_in_sleep();

    while (hal_sys_timer_get_in_sleep() - start < ticks);
}
#else
void hal_sys_timer_delay_in_sleep(uint32_t ticks) __attribute__((alias("hal_sys_timer_delay")));
#endif

void BOOT_TEXT_FLASH_LOC flash_hal_sys_timer_delay(uint32_t ticks)
{
    uint32_t start = flash_hal_sys_timer_get();

    while (flash_hal_sys_timer_get() - start < ticks);
}

void BOOT_TEXT_SRAM_LOC hal_sys_timer_delay_us(uint32_t us)
{
#if (defined(TIMER1_BASE) || defined(CMU_FREE_FAST_TIMER)) && !defined(FAST_TIMER_WORKAROUND)
    uint32_t start = hal_fast_sys_timer_get();
    uint32_t ticks = US_TO_FAST_TICKS(us);

    while (hal_fast_sys_timer_get() - start < ticks);
#else
    enum HAL_CMU_FREQ_T freq = hal_cmu_sys_get_freq();
    uint32_t loop;
    uint32_t i;

    // Assuming:
    // 1) system clock uses audio PLL
    // 2) audio PLL is configured as 48K series, 196.608M
    // 3) crystal is 26M

    if (freq == HAL_CMU_FREQ_208M) {
        loop = 197;
    } else if (freq == HAL_CMU_FREQ_104M) {
        loop = 197 / 2;
    } else if (freq == HAL_CMU_FREQ_78M) {
        loop = 197 / 3;
    } else if (freq == HAL_CMU_FREQ_52M) {
        loop = 52;
    } else {
        loop = 26;
    }

    loop = loop * us / 5;
    for (i = 0; i < loop; i++) {
        asm volatile("nop");
    }
#endif
}

void SRAM_TEXT_LOC hal_sys_timer_delay_ns(uint32_t ns)
{
#if (defined(TIMER1_BASE) || defined(CMU_FREE_FAST_TIMER)) && !defined(FAST_TIMER_WORKAROUND)
    uint32_t start = hal_fast_sys_timer_get();
    uint32_t ticks = NS_TO_FAST_TICKS(ns);

    while (hal_fast_sys_timer_get() - start < ticks);
#else
    enum HAL_CMU_FREQ_T freq = hal_cmu_sys_get_freq();
    uint32_t loop;
    uint32_t i;

    // Assuming:
    // 1) system clock uses audio PLL
    // 2) audio PLL is configured as 48K series, 196.608M
    // 3) crystal is 26M

    if (freq == HAL_CMU_FREQ_208M) {
        loop = 197;
    } else if (freq == HAL_CMU_FREQ_104M) {
        loop = 197 / 2;
    } else if (freq == HAL_CMU_FREQ_78M) {
        loop = 197 / 3;
    } else if (freq == HAL_CMU_FREQ_52M) {
        loop = 52;
    } else {
        loop = 26;
    }

    loop = loop * ns / 5000;
    for (i = 0; i < loop; i++) {
        asm volatile("nop");
    }
#endif
}

static uint32_t NOINLINE SRAM_TEXT_DEF(measure_cpu_freq_interval)(uint32_t cnt, uint32_t *read_cycles_p)
{
    uint32_t old_start;
    uint32_t start;
    uint32_t end;

#ifndef CMU_FREE_FAST_TIMER
    struct DUAL_TIMER_T *t;
    uint32_t delta;

#ifdef TIMER1_BASE
    t = dual_timer[1];
    delta = FAST_TIMER_VAL_DELTA;
#ifdef FAST_TIMER_WORKAROUND
    if (hal_cmu_fast_timer_offline()) {
        t = dual_timer[SLOW_FREE_TIMER_ID];
        delta = SLOW_TIMER_VAL_DELTA;
    }
#endif // FAST_TIMER_WORKAROUND
#else
    t = dual_timer[SLOW_FREE_TIMER_ID];
    delta = SLOW_TIMER_VAL_DELTA;
#endif
#endif

    if (read_cycles_p) {
        int ret;

        ret = dwt_enable();
        dwt_reset_cycle_cnt();
#ifdef CMU_FREE_FAST_TIMER
        _real_fast_sys_timer_get();
#else
        get_timer_value(&t->timer[0], delta);
#endif
        *read_cycles_p = dwt_read_cycle_cnt();
        if (ret == 0) {
            dwt_disable();
        }
    }

#ifdef CMU_FREE_FAST_TIMER
    old_start = _real_fast_sys_timer_get();
    while ((start = _real_fast_sys_timer_get()) == old_start);
#else
    old_start = get_timer_value(&t->timer[0], delta);
    while ((start = get_timer_value(&t->timer[0], delta)) == old_start);
#endif

    asm volatile(
        "1:\n"
#ifdef __ARM_ARCH_ISA_ARM
        "nop\n"
        "nop\n"
#endif
        "subs %0, #1 \n"
        "cmp %0, #0\n"
        "bne 1b\n"
        : : "r"(cnt));

#ifdef CMU_FREE_FAST_TIMER
    end = _real_fast_sys_timer_get();
    return (end - start);
#else
    end = get_timer_value(&t->timer[0], delta);
    return (start - end);
#endif
}

uint32_t hal_sys_timer_calc_cpu_freq(uint32_t osc_intvl_ms, int high_res)
{
    uint32_t ref_freq;
    uint32_t cnt;
    uint32_t one_sec;
    uint32_t lock;
    uint32_t run_interval;
    uint32_t base_interval;
    uint32_t timer_read_cycles = 0;
    uint32_t freq;

    // Default measurement interval
    if (osc_intvl_ms == 0) {
#if defined(TIMER1_BASE) || defined(CMU_FREE_FAST_TIMER)
        osc_intvl_ms = 10;
#ifdef FAST_TIMER_WORKAROUND
        if (hal_cmu_fast_timer_offline()) {
            osc_intvl_ms = 100;
        }
#endif
#else
        osc_intvl_ms = 100;
#endif
    }

    // Timer ticks per second
#if defined(TIMER1_BASE) || defined(CMU_FREE_FAST_TIMER)
    one_sec = CONFIG_FAST_SYSTICK_HZ;
#ifdef FAST_TIMER_WORKAROUND
    if (hal_cmu_fast_timer_offline()) {
        one_sec = CONFIG_SYSTICK_HZ;
        high_res = 0;
    }
#endif
#else
    if (high_res) {
        one_sec = CONFIG_FAST_SYSTICK_HZ;
    } else {
        one_sec = CONFIG_SYSTICK_HZ;
    }
#endif

    ref_freq = hal_cmu_get_crystal_freq();
    // CPU loop cycle count
    cnt = ref_freq / 4 * osc_intvl_ms / 1000;
    // Timer ticks per measurement interval
    base_interval = one_sec * osc_intvl_ms / 1000;

    lock = int_lock();

#if !defined(TIMER1_BASE) && !defined(CMU_FREE_FAST_TIMER)
    if (high_res) {
        hal_cmu_timer0_select_fast();
    }
#endif

    // CAUTION:
    // Simultaneous memory access (e.g., trace DMA) might block CPU instruction or data access,
    // and make the run interval longer than expected.
    run_interval = measure_cpu_freq_interval(cnt, high_res ? &timer_read_cycles : NULL);

#if !defined(TIMER1_BASE) && !defined(CMU_FREE_FAST_TIMER)
    if (high_res) {
        hal_cmu_timer0_select_slow();
    }
#endif

    int_unlock(lock);

    //TRACE(0, "calc_cpu_freq: base=%u run=%u read=%u", base_interval, run_interval, timer_read_cycles);

#ifdef TIMER_USE_FPU
    freq = (uint32_t)((float)ref_freq / run_interval * base_interval);
#else
    freq = (uint32_t)((uint64_t)ref_freq * base_interval / run_interval);
#endif

    if (high_res && timer_read_cycles) {
        //TRACE(0, "calc_cpu_freq: freq1=%u", freq);
        base_interval *= 100;
        run_interval = (100 * run_interval) - (100 * timer_read_cycles) * (uint64_t)one_sec / freq;
#ifdef TIMER_USE_FPU
        freq = (uint32_t)((float)ref_freq / run_interval * base_interval);
#else
        freq = (uint32_t)((uint64_t)ref_freq * base_interval / run_interval);
#endif
        //TRACE(0, "calc_cpu_freq: freq2=%u", freq);
    }

    if (high_res == 0) {
        freq = (freq + 500000) / 1000000 * 1000000;
    }

    // (1) Deviation = 1 / run_interval
    // (2) Deviation = 1_tick_time / ((osc_intvl_ms / 1000) / (freq / osc_freq)))
    return freq;
}

#ifdef CALIB_SLOW_TIMER
void BOOT_TEXT_SRAM_LOC hal_sys_timer_calib_start(void)
{
    uint32_t lock;
    uint32_t slow;
    uint32_t fast;

    lock = int_lock();
#ifdef FAST_TIMER_WORKAROUND
    ASSERT(!hal_cmu_fast_timer_offline(), "%s: Cannot calib timer while fast timer offline", __func__);
#endif
    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = _real_fast_sys_timer_get();
    int_unlock(lock);

    slow_val = slow + 1;
    fast_val = fast;
}

int BOOT_TEXT_SRAM_LOC hal_sys_timer_calib_end(void)
{
    uint32_t lock;
    uint32_t slow;
    uint32_t fast;
    uint32_t slow_diff;

    lock = int_lock();
#ifdef FAST_TIMER_WORKAROUND
    ASSERT(!hal_cmu_fast_timer_offline(), "%s: Cannot calib timer while fast timer offline", __func__);
#endif
    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = _real_fast_sys_timer_get();
    int_unlock(lock);

    slow += 1;
    slow_diff = slow - slow_val;

    // Avoid computation error
    if (slow_diff < MIN_CALIB_TICKS) {
        return 1;
    }
    // Avoid fast tick overflow
    if (slow_diff > MAX_CALIB_TICKS) {
        return 2;
    }

#ifdef TIMER_USE_FPU
    sys_tick_hz = (float)CONFIG_FAST_SYSTICK_HZ / (fast - fast_val) * slow_diff;
#ifdef FAST_TIMER_COMPENSATE
    uint32_t i;
    for (i = FAST_TICK_RATIO_NUM-1; i >= 1; --i)
       fast_tick_ratio[i] = fast_tick_ratio[i-1];
    fast_tick_ratio[0] = (float)(fast - fast_val) / (float)slow_diff;
    fast_tick_ratio_avg = 0;
    for (i = 0; i < FAST_TICK_RATIO_NUM; ++i) {
        if (fast_tick_ratio[i] < 100.0)
            break;
        fast_tick_ratio_avg += fast_tick_ratio[i];
    }
    fast_tick_ratio_avg = fast_tick_ratio_avg / i;
#endif /*FAST_TIMER_COMPENSATE*/
#else
    uint64_t mul;
    uint32_t fast_diff;

    fast_diff = fast - fast_val;
    mul = (uint64_t)CONFIG_FAST_SYSTICK_HZ * slow_diff + fast_diff / 2;
    if ((mul >> 32) == 0) {
        sys_tick_hz = (uint32_t)mul / fast_diff;
    } else {
        sys_tick_hz = mul / fast_diff;
    }
#endif

    if (sys_tick_hz > MAX_CALIB_SYSTICK_HZ) {
        sys_tick_hz = MAX_CALIB_SYSTICK_HZ;
    }

    return 0;
}

void BOOT_TEXT_SRAM_LOC hal_sys_timer_calib(void)
{
    hal_sys_timer_calib_start();
    hal_sys_timer_delay(MIN_CALIB_TICKS);
    hal_sys_timer_calib_end();
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_systick_hz(void)
{
#ifdef TIMER_USE_FPU
    return (uint32_t)(sys_tick_hz + 0.5);
#else
    return sys_tick_hz;
#endif
}

float BOOT_TEXT_SRAM_LOC hal_sys_timer_systick_hz_float(void)
{
    return sys_tick_hz;
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_ms_to_ticks(uint32_t ms)
{
#ifdef TIMER_USE_FPU
    return (uint32_t)((float)ms / 1000 * sys_tick_hz + 0.99);
#else
    if (ms <= (~0UL / MAX_CALIB_SYSTICK_HZ)) {
        return ((ms * sys_tick_hz + 1000 - 1) / 1000);
    } else {
        return (((uint64_t)ms * sys_tick_hz + 1000 - 1) / 1000);
    }
#endif
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_us_to_ticks(uint32_t us)
{
    uint32_t ticks;

#ifdef TIMER_USE_FPU
    ticks = (uint32_t)((float)us / (1000 * 1000) * sys_tick_hz + 0.99);
#else
    if (us <= (~0UL / MAX_CALIB_SYSTICK_HZ)) {
        ticks = ((us * sys_tick_hz / 1000 + 1000 - 1) / 1000);
    } else {
        ticks = (((uint64_t)us * sys_tick_hz / 1000 + 1000 - 1) / 1000);
    }
#endif

    if (ticks <= 1) {
        ticks += 1;
    }
    return ticks;
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_ticks_to_ms(uint32_t tick)
{
#ifdef TIMER_USE_FPU
    return (uint32_t)((float)tick / sys_tick_hz * 1000 + 0.5);
#else
    uint32_t hz = sys_tick_hz;
    if (tick <= ((~0UL - MAX_CALIB_SYSTICK_HZ / 2) / 1000)) {
        return (tick * 1000 + hz / 2) / hz;
    } else {
        return ((uint64_t)tick * 1000 + hz / 2) / hz;
    }
#endif
}

uint32_t BOOT_TEXT_SRAM_LOC hal_sys_timer_ticks_to_us(uint32_t tick)
{
#ifdef TIMER_USE_FPU
    return (uint32_t)((float)tick / sys_tick_hz * (1000 * 1000) + 0.5);
#else
    uint32_t hz = sys_tick_hz;
    // Simple round down is enough for us
    if (tick <= (~0UL / (1000 * 1000))) {
        return (tick * (1000 * 1000)) / hz;
    } else {
        return ((uint64_t)tick * (1000 * 1000)) / hz;
    }
#endif
}

uint64_t BOOT_TEXT_SRAM_LOC hal_sys_timer_ticks_to_ms64(uint32_t tick)
{
#ifdef TIMER_USE_FPU
    return (uint64_t)((float)tick / sys_tick_hz * 1000 + 0.5);
#else
    uint32_t hz = sys_tick_hz;
    if (tick <= ((~0UL - MAX_CALIB_SYSTICK_HZ / 2) / 1000)) {
        return (tick * 1000 + hz / 2) / hz;
    } else {
        return ((uint64_t)tick * 1000 + hz / 2) / hz;
    }
#endif
}

uint64_t BOOT_TEXT_SRAM_LOC hal_sys_timer_ticks_to_us64(uint32_t tick)
{
#ifdef TIMER_USE_FPU
    return (uint64_t)((float)tick / sys_tick_hz * (1000 * 1000) + 0.5);
#else
    uint32_t hz = sys_tick_hz;
    // Simple round down is enough for us
    if (tick <= (~0UL / (1000 * 1000))) {
        return (tick * (1000 * 1000)) / hz;
    } else {
        return ((uint64_t)tick * (1000 * 1000)) / hz;
    }
#endif
}
#endif

#ifdef FAST_TIMER_COMPENSATE
static uint32_t BOOT_BSS_LOC slow_timer_begin = 0;
static uint32_t BOOT_BSS_LOC fast_timer_begin = 0;

void SRAM_TEXT_LOC hal_fast_timer_sleep(void)
{
    uint32_t slow, fast;

    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = -_real_fast_sys_timer_get();
    fast_timer_begin = fast;
    slow_timer_begin = slow + 1;
}

void SRAM_TEXT_LOC hal_fast_timer_wakeup(void)
{
    uint32_t slow, fast;
    uint32_t load;
    float sleep_tick;

    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = _real_fast_sys_timer_get();
    __ISB();
    sleep_tick = fast_tick_ratio_avg * (float)(slow + 1 - slow_timer_begin) + 0.9;
    load = fast_timer_begin - (uint32_t)sleep_tick;
    __ISB();
    load -= _real_fast_sys_timer_get() - fast;//float compute time
    __real_fast_sys_timer_set(load);
}
#endif

#ifndef RTOS
int osDelay(uint32_t ms)
{
    hal_sys_timer_delay(MS_TO_TICKS(ms));
    return 0;
}
#endif

//==========================================================
// Timer IRQ APIs
//==========================================================

__STATIC_FORCEINLINE int timer_is_slow(uint32_t id)
{
    return (id == 0);
}

static void hal_timer_common_irq_handler(uint32_t id, uint32_t sub_id)
{
    uint32_t elapsed;

    if (!hal_timer_common_irq_pending(id, sub_id)) {
        return;
    }

#ifdef CHIP_BEST1700
    // Value might NOT be sync to read if just waking up from sleep
    uint32_t value = dual_timer[id]->timer[sub_id].Value;
    while (value && value == dual_timer[id]->timer[sub_id].Value) {}
#endif

    clear_timer_irq(&dual_timer[id]->timer[sub_id]);
    if (irq_handler[id][sub_id]) {
        elapsed = hal_timer_common_get_elapsed_time(id, sub_id);
        irq_handler[id][sub_id](elapsed);
    } else {
        dual_timer[id]->timer[sub_id].Control &= ~TIMER_CTRL_INTEN;
    }
}

static void hal_timer00_irq_handler(void)
{
    hal_timer_common_irq_handler(0, 0);
}

static void hal_timer01_irq_handler(void)
{
    hal_timer_common_irq_handler(0, 1);
}

#ifdef TIMER1_BASE
static void hal_timer10_irq_handler(void)
{
    hal_timer_common_irq_handler(1, 0);
}

static void hal_timer11_irq_handler(void)
{
    hal_timer_common_irq_handler(1, 1);
}
#endif

#ifdef TIMER2_BASE
static void hal_timer2x_irq_handler(void)
{
    uint32_t sub_id;

    // NOTE: Some chips might combine time20 and time21 IRQ into one interrupt line
    for (sub_id = 0; sub_id < 2; sub_id++) {
        if (hal_timer_common_irq_pending(2, sub_id)) {
            hal_timer_common_irq_handler(2, sub_id);
        }
    }
}

static void hal_timer20_irq_handler(void)
{
    if (TIMER20_IRQn == TIMER21_IRQn) {
        hal_timer2x_irq_handler();
    } else {
        hal_timer_common_irq_handler(2, 0);
    }
}

static void hal_timer21_irq_handler(void)
{
    if (TIMER20_IRQn == TIMER21_IRQn) {
        hal_timer2x_irq_handler();
    } else {
        hal_timer_common_irq_handler(2, 1);
    }
}
#endif

static const uint32_t irq_entry[ARRAY_SIZE(dual_timer)][2] = {
    { (uint32_t)hal_timer00_irq_handler, (uint32_t)hal_timer01_irq_handler, },
#ifdef TIMER1_BASE
    { (uint32_t)hal_timer10_irq_handler, (uint32_t)hal_timer11_irq_handler, },
#endif
#ifdef TIMER2_BASE
    { (uint32_t)hal_timer20_irq_handler, (uint32_t)hal_timer21_irq_handler, },
#endif
};

static void hal_timer_common_setup(uint32_t id, uint32_t sub_id, enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler)
{
    uint32_t mode;

    if (type == HAL_TIMER_TYPE_ONESHOT) {
        mode = TIMER_CTRL_ONESHOT;
    } else if (type == HAL_TIMER_TYPE_PERIODIC) {
        mode = TIMER_CTRL_MODE_PERIODIC;
    } else {
        mode = 0;
    }

    irq_handler[id][sub_id] = handler;

    clear_timer_irq(&dual_timer[id]->timer[sub_id]);
    __DSB();

    if (handler) {
        NVIC_SetVector(irq_num[id][sub_id], irq_entry[id][sub_id]);
        NVIC_SetPriority(irq_num[id][sub_id], IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(irq_num[id][sub_id]);
        NVIC_EnableIRQ(irq_num[id][sub_id]);
    }

    dual_timer[id]->timer[sub_id].Control = mode |
                                   (handler ? TIMER_CTRL_INTEN : 0) |
                                   TIMER_CTRL_PRESCALE_DIV_1 |
                                   TIMER_CTRL_SIZE_32_BIT;
}

POSSIBLY_UNUSED static void hal_timer_common_set_control(uint32_t id, uint32_t sub_id, uint32_t ctrl)
{
    dual_timer[id]->timer[sub_id].Control = ctrl;
}

POSSIBLY_UNUSED static uint32_t hal_timer_common_get_control(uint32_t id, uint32_t sub_id)
{
    return dual_timer[id]->timer[sub_id].Control;
}

static void hal_timer_common_reload(uint32_t id, uint32_t sub_id, uint32_t load)
{
    uint32_t delta;

#ifdef CHIP_BEST1700
    if (timer_is_slow(id)) {
        uint32_t diff = hal_sys_timer_get() - slow_timer_load_time;
        if (diff < 2) {
            hal_sys_timer_delay(2 - diff);
        }
    }
#endif

    delta = timer_is_slow(id) ? SLOW_TIMER_VAL_DELTA : FAST_TIMER_VAL_DELTA;
    if (load > HAL_TIMER_LOAD_DELTA) {
        load -= HAL_TIMER_LOAD_DELTA;
    } else {
        load = HAL_TIMER_LOAD_DELTA;
    }
    set_timer_load(&dual_timer[id]->timer[sub_id], load, delta);

#ifdef CHIP_BEST1700
    if (timer_is_slow(id)) {
        slow_timer_load_time = hal_sys_timer_get();
    }
#endif
}

POSSIBLY_UNUSED static uint32_t hal_timer_common_get_load(uint32_t id, uint32_t sub_id)
{
    return dual_timer[id]->timer[sub_id].Load + HAL_TIMER_LOAD_DELTA;
}

POSSIBLY_UNUSED static void hal_timer_common_bgload(uint32_t id, uint32_t sub_id, uint32_t load)
{
    if (load > HAL_TIMER_LOAD_DELTA) {
        load -= HAL_TIMER_LOAD_DELTA;
    } else {
        load = HAL_TIMER_LOAD_DELTA;
    }
    dual_timer[id]->timer[sub_id].BGLoad = load;
}

POSSIBLY_UNUSED static void hal_timer_common_pause(uint32_t id, uint32_t sub_id)
{
    dual_timer[id]->timer[sub_id].Control &= ~TIMER_CTRL_EN;
}

static void hal_timer_common_continue(uint32_t id, uint32_t sub_id)
{
    dual_timer[id]->timer[sub_id].Control |= TIMER_CTRL_EN;
}

static void hal_timer_common_start(uint32_t id, uint32_t sub_id, uint32_t load)
{
    start_time[id][sub_id] = timer_is_slow(id) ? hal_sys_timer_get() : hal_fast_sys_timer_get();
    hal_timer_common_reload(id, sub_id, load);
    hal_timer_common_continue(id, sub_id);
}

static void hal_timer_common_stop(uint32_t id, uint32_t sub_id)
{
    dual_timer[id]->timer[sub_id].Control &= ~TIMER_CTRL_EN;
#ifdef CHIP_BEST1700
    if (!timer_is_slow(id)) {
        uint32_t cur = hal_fast_sys_timer_get();
        while (hal_fast_sys_timer_get() - cur < 3) {}
    }
#endif
    clear_timer_irq(&dual_timer[id]->timer[sub_id]);
    __DSB();
    NVIC_ClearPendingIRQ(irq_num[id][sub_id]);
}

static int hal_timer_common_is_enabled(uint32_t id, uint32_t sub_id)
{
    return !!(dual_timer[id]->timer[sub_id].Control & TIMER_CTRL_EN);
}

static uint32_t hal_timer_common_get_raw_value(uint32_t id, uint32_t sub_id)
{
    uint32_t delta;

    delta = timer_is_slow(id) ? SLOW_TIMER_VAL_DELTA : FAST_TIMER_VAL_DELTA;
    return get_timer_value(&dual_timer[id]->timer[sub_id], delta);
}

static int hal_timer_common_irq_active(uint32_t id, uint32_t sub_id)
{
    return NVIC_GetActive(irq_num[id][sub_id]);
}

static int hal_timer_common_irq_pending(uint32_t id, uint32_t sub_id)
{
    return (dual_timer[id]->timer[sub_id].MIS & TIMER_MIS_MIS);
}

static uint32_t hal_timer_common_get_elapsed_time(uint32_t id, uint32_t sub_id)
{
    uint32_t time;

    time = timer_is_slow(id) ? hal_sys_timer_get() : hal_fast_sys_timer_get();
    return time - start_time[id][sub_id];
}

//----------------------------------------------------------
// Slow timer IRQ API
//----------------------------------------------------------

void hal_timer_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler)
{
    hal_timer_common_setup(0, SLOW_TIMER_SUB_ID_WITH_IRQ, type, handler);
}

void hal_timer_start(uint32_t load)
{
    hal_timer_common_start(0, SLOW_TIMER_SUB_ID_WITH_IRQ, load);
}

void hal_timer_stop(void)
{
    hal_timer_common_stop(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

void hal_timer_continue(void)
{
    hal_timer_common_continue(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

int hal_timer_is_enabled(void)
{
    return hal_timer_common_is_enabled(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

void hal_timer_reload(uint32_t load)
{
    hal_timer_common_reload(0, SLOW_TIMER_SUB_ID_WITH_IRQ, load);
}

uint32_t hal_timer_get_raw_value(void)
{
    return hal_timer_common_get_raw_value(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

int hal_timer_irq_active(void)
{
    return hal_timer_common_irq_active(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

int hal_timer_irq_pending(void)
{
    return hal_timer_common_irq_pending(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

uint32_t hal_timer_get_elapsed_time(void)
{
    return hal_timer_common_get_elapsed_time(0, SLOW_TIMER_SUB_ID_WITH_IRQ);
}

//----------------------------------------------------------
// Fast timer IRQ API
//----------------------------------------------------------

#ifdef TIMER1_BASE

#ifdef CORE_SLEEP_POWER_DOWN
static uint32_t fast_timer_ctrl;
static uint32_t fast_timer_load;

static void hal_fast_timer_set_control(uint32_t ctrl)
{
    hal_timer_common_set_control(1, FAST_TIMER_SUB_ID_WITH_IRQ, ctrl);
}

static uint32_t hal_fast_timer_get_control(void)
{
    return hal_timer_common_get_control(1, FAST_TIMER_SUB_ID_WITH_IRQ);
}

static int hal_fast_timer_pm_notif_handler(enum HAL_PM_STATE_T state)
{
    if (state == HAL_PM_STATE_POWER_DOWN_SLEEP) {
        fast_timer_ctrl = hal_fast_timer_get_control();
        fast_timer_load = hal_fast_timer_get_load();
    } else if (state == HAL_PM_STATE_POWER_DOWN_WAKEUP) {
        hal_fast_timer_set_control(fast_timer_ctrl);
        hal_fast_timer_reload(fast_timer_load);
    }

    return 0;
}
#endif

void hal_fast_timer_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler)
{
#ifndef NO_CMU_INIT
#ifdef CMU_FREE_FAST_TIMER
    hal_cmu_timer_clock_enable(TIMER1_BASE);
#endif
#endif

    hal_timer_common_setup(1, FAST_TIMER_SUB_ID_WITH_IRQ, type, handler);
#ifdef CORE_SLEEP_POWER_DOWN
    hal_pm_notif_register(HAL_PM_USER_HAL, hal_fast_timer_pm_notif_handler);
#endif
}

void hal_fast_timer_start(uint32_t load)
{
    hal_timer_common_start(1, FAST_TIMER_SUB_ID_WITH_IRQ, load);
}

void hal_fast_timer_stop(void)
{
    hal_timer_common_stop(1, FAST_TIMER_SUB_ID_WITH_IRQ);
}

void hal_fast_timer_pause(void)
{
    hal_timer_common_pause(1, FAST_TIMER_SUB_ID_WITH_IRQ);
}

void hal_fast_timer_continue(void)
{
    hal_timer_common_continue(1, FAST_TIMER_SUB_ID_WITH_IRQ);
}

int hal_fast_timer_is_enabled(void)
{
    return hal_timer_common_is_enabled(1, FAST_TIMER_SUB_ID_WITH_IRQ);
}

void hal_fast_timer_reload(uint32_t load)
{
    hal_timer_common_reload(1, FAST_TIMER_SUB_ID_WITH_IRQ, load);
}

void hal_fast_timer_bgload(uint32_t load)
{
    hal_timer_common_bgload(1, FAST_TIMER_SUB_ID_WITH_IRQ, load);
}

uint32_t hal_fast_timer_get_raw_value(void)
{
    return hal_timer_common_get_raw_value(1, FAST_TIMER_SUB_ID_WITH_IRQ);
}

int hal_fast_timer_irq_active(void)
{
    return hal_timer_common_irq_active(1, FAST_TIMER_SUB_ID_WITH_IRQ);
}

int hal_fast_timer_irq_pending(void)
{
    return hal_timer_common_irq_pending(1, FAST_TIMER_SUB_ID_WITH_IRQ);
}

uint32_t hal_fast_timer_get_elapsed_time(void)
{
    return hal_timer_common_get_elapsed_time(1, FAST_TIMER_SUB_ID_WITH_IRQ);
}

uint32_t hal_fast_timer_get_load(void)
{
    return hal_timer_common_get_load(1, FAST_TIMER_SUB_ID_WITH_IRQ);
}

uint32_t hal_fast_timer_get_count(void)
{
    return dual_timer[1]->timer[FAST_TIMER_SUB_ID_WITH_IRQ].Load -
        get_timer_value(&dual_timer[1]->timer[FAST_TIMER_SUB_ID_WITH_IRQ], FAST_TIMER_VAL_DELTA);
}

#endif // TIMER1_BASE

//----------------------------------------------------------
// User timer IRQ API
//----------------------------------------------------------

#if ((USER_TIMER_ID_WITH_IRQ == 1) && defined(TIMER1_BASE)) || \
    ((USER_TIMER_ID_WITH_IRQ == 2) && defined(TIMER2_BASE))

void hal_user_timer_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler)
{
#ifndef NO_CMU_INIT
#if ((USER_TIMER_ID_WITH_IRQ == 2) && defined(TIMER2_BASE))
    hal_cmu_timer_clock_enable(TIMER2_BASE);
#endif
#endif

    hal_timer_common_setup(USER_TIMER_ID_WITH_IRQ, USER_TIMER_SUB_ID_WITH_IRQ, type, handler);
}

void hal_user_timer_start(uint32_t load)
{
    hal_timer_common_start(USER_TIMER_ID_WITH_IRQ, USER_TIMER_SUB_ID_WITH_IRQ, load);
}

void hal_user_timer_stop(void)
{
    hal_timer_common_stop(USER_TIMER_ID_WITH_IRQ, USER_TIMER_SUB_ID_WITH_IRQ);
}

void hal_user_timer_continue(void)
{
    hal_timer_common_continue(USER_TIMER_ID_WITH_IRQ, USER_TIMER_SUB_ID_WITH_IRQ);
}

int hal_user_timer_is_enabled(void)
{
    return hal_timer_common_is_enabled(USER_TIMER_ID_WITH_IRQ, USER_TIMER_SUB_ID_WITH_IRQ);
}

void hal_user_timer_reload(uint32_t load)
{
    hal_timer_common_reload(USER_TIMER_ID_WITH_IRQ, USER_TIMER_SUB_ID_WITH_IRQ, load);
}

uint32_t hal_user_timer_get_raw_value(void)
{
    return hal_timer_common_get_raw_value(USER_TIMER_ID_WITH_IRQ, USER_TIMER_SUB_ID_WITH_IRQ);
}

int hal_user_timer_irq_active(void)
{
    return hal_timer_common_irq_active(USER_TIMER_ID_WITH_IRQ, USER_TIMER_SUB_ID_WITH_IRQ);
}

int hal_user_timer_irq_pending(void)
{
    return hal_timer_common_irq_pending(USER_TIMER_ID_WITH_IRQ, USER_TIMER_SUB_ID_WITH_IRQ);
}

uint32_t hal_user_timer_get_elapsed_time(void)
{
    return hal_timer_common_get_elapsed_time(USER_TIMER_ID_WITH_IRQ, USER_TIMER_SUB_ID_WITH_IRQ);
}

#endif
