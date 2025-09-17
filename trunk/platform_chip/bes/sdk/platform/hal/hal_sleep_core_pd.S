/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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

/*
=========================================================================
    Core Sleep Power Down and Wakeup Power Up Helper Functions
=========================================================================
*/

    #include "cmsis.h"

    .syntax unified

#ifdef CORE_SLEEP_POWER_DOWN

#ifndef CHIP_SUBSYS_SENS
#if defined(CHIP_BEST1501) || defined(CHIP_BEST1501P) || defined(CHIP_BEST2000) || \
        defined(CHIP_BEST2300) || defined(CHIP_BEST2300A) || defined(CHIP_BEST2300P)
#define PD_SLEEP_STACK_WORKAROUND
#define PD_SLEEP_STACK_CHECK
#include "plat_addr_map.h"
#endif
#endif

#ifndef ARM_CMNS
#if defined(ROM_BUILD) || defined(PROGRAMMER)
    .section .bss.core_sleep_stack
#else
    .section .sram_bss.core_sleep_stack
#endif
    .align    2
core_sleep_stack:
#ifdef PD_SLEEP_STACK_WORKAROUND
    /* 4 bytes to save SP, 4 bytes to save check result, and 16 bytes to save ROM stack used */
    .space 24
#else
    .space 4
#endif
#endif

#if defined(ROM_BUILD) || defined(PROGRAMMER)
    .section .text.hal_sleep_core_power_down, "ax", %progbits
#else
    .section .sram_text.hal_sleep_core_power_down, "ax", %progbits
#endif
    .thumb
    .thumb_func
    .align    2
    .globl    hal_sleep_core_power_down
    .type    hal_sleep_core_power_down, %function
hal_sleep_core_power_down:

#ifdef ARM_CMNS
    ldr pc, =hal_psc_core_auto_power_down
#else
    push {r4-r12,lr}
    movs r4, r0
    mrs r0, psp
    push {r0,r1}
#ifdef __ARM_ARCH_8M_MAIN__
    mrs r0, msplim
    mrs r1, psplim
    push {r0,r1}
#endif
    mrs r0, primask
    mrs r1, basepri
    mrs r2, faultmask
    mrs r3, control
    push {r0-r3}
#if defined(__ARM_ARCH_8M_MAIN__) && defined(ARM_CMSE)
    mrs r0, msp_ns
    mrs r1, psp_ns
    mrs r2, msplim_ns
    mrs r3, psplim_ns
    push {r0-r3}
    mrs r0, primask_ns
    mrs r1, basepri_ns
    mrs r2, faultmask_ns
    mrs r3, control_ns
    push {r0-r3}
#endif
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    /* TODO: Save FPU registers? */
#endif
    ldr r3, =core_sleep_stack
#ifdef __PIE__
    ldr r0, =hal_sleep_core_power_down
    adr r1, hal_sleep_core_power_down
    sub r1, r0
    add r3, r1
#endif
    str sp, [r3]
#ifdef PD_SLEEP_STACK_WORKAROUND
    ldr r0, =#ROM_BASE
    ldr r0, [r0]
#ifdef PD_SLEEP_STACK_CHECK
    /* Check last result */
    ldr r5, [r3, #4]
    cbnz r5, _check_done
    /* Check if core_sleep_stack is overlapped with the mem */
    cmp r0, r3
    bls _check_done
    sub r5, r0, 16
    add r6, r3, 24
    cmp r5, r6
    blt .
#ifdef SENS_RAMRET_BASE
    /* Check if the mem is used by SENS */
    ldr r5, =#SENS_RAMRET_BASE
    cmp r0, r5
    bgt .
#endif
    /* Save the result */
    movs r5, #1
    str r5, [r3, #4]
_check_done:
#endif
    ldmdb r0, {r5-r8}
    add r3, 8
    stmia r3, {r5-r8}
#endif
    dsb
    movs r0, r4
    bl hal_psc_core_auto_power_down
#if defined(POWER_DOWN_VIA_DEEP_SLEEP) || defined(CHIP_ROLE_CP)
    b hal_sleep_core_power_up
#else
    b .
#endif
#endif

    .pool
    .size    hal_sleep_core_power_down, . - hal_sleep_core_power_down


#ifndef ARM_CMNS
#if defined(ROM_BUILD) || defined(PROGRAMMER)
    .section .text.hal_sleep_core_power_up, "ax", %progbits
#else
    .section .sram_text.hal_sleep_core_power_up, "ax", %progbits
#endif
    .thumb
    .thumb_func
    .align    2
    .globl    hal_sleep_core_power_up
    .type    hal_sleep_core_power_up, %function
hal_sleep_core_power_up:

    ldr r3, =core_sleep_stack
#ifdef __PIE__
    ldr r0, =hal_sleep_core_power_up
    adr r1, hal_sleep_core_power_up
    sub r1, r0
    add r3, r1
#endif
    ldr sp, [r3]
#ifdef __ARM_ARCH_8M_MAIN__
    movs r0, #0
    msr msplim, r0
#endif
#ifdef PD_SLEEP_STACK_WORKAROUND
    add r3, 8
    ldmia r3, {r5-r8}
    ldr r0, =#ROM_BASE
    ldr r0, [r0]
    stmdb r0, {r5-r8}
#endif
    dsb
    isb
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    /* TODO: Restore FPU registers? */
#endif
#if defined(__ARM_ARCH_8M_MAIN__) && defined(ARM_CMSE)
    pop {r0-r3}
    msr primask_ns, r0
    msr basepri_ns, r1
    msr faultmask_ns, r2
    msr control_ns, r3
    pop {r0-r3}
    msr msp_ns, r0
    msr psp_ns, r1
    msr msplim_ns, r2
    msr psplim_ns, r3
#endif
    pop {r0-r3}
    msr primask, r0
    msr basepri, r1
    msr faultmask, r2
    msr control, r3
#ifdef __ARM_ARCH_8M_MAIN__
    pop {r0,r1}
    msr msplim, r0
    msr psplim, r1
#endif
    pop {r0,r1}
    msr psp, r0
    pop {r4-r12,lr}
    bx lr

    .pool
    .size    hal_sleep_core_power_up, . - hal_sleep_core_power_up
#endif

#endif

    .end
