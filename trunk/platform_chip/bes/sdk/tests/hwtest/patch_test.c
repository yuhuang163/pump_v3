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
#ifndef __ARM_ARCH_ISA_ARM

#include "hwtest.h"
#include "patch.h"
#include "hal_cmu.h"
#include "cmsis.h"

#define MAX_TEST_PATCH_NUM              64

#ifndef PATCH_CTRL_BASE
#define PATCH_ENTRY_NUM                 0
#endif

#define OLD_WORD(n)                     \
    (((n * 4) & 0xFF) | ((((n * 4) + 1) & 0xFF) << 8) | ((((n * 4) + 2) & 0xFF) << 16) | ((((n * 4) + 3) & 0xFF) << 24))

#define NEW_WORD(n)                     \
    (OLD_WORD(n) ^ 0xFFFFFFFF)

#define OLD_FUNC_BODY(n)                \
    void __old_func_body_ ## n (void) { code_bitmap[n / 32] &= ~(1 << (n % 32)); }

#define NEW_FUNC_BODY(n)                \
    void __attribute__((section(".rom_ramx"))) __new_func_body_ ## n (void) \
    { code_bitmap[n / 32] |= (1 << (n % 32)); }

#define OLD_FUNC(n)                     __old_func_body_ ## n
#define NEW_FUNC(n)                     __new_func_body_ ## n

static uint32_t code_bitmap[(MAX_TEST_PATCH_NUM + 31) / 32];

static const uint32_t old_data[MAX_TEST_PATCH_NUM] = {
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
    OLD_WORD(__LINE__),
};

static const uint32_t new_data[MAX_TEST_PATCH_NUM] = {
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
    NEW_WORD(__LINE__),
};

OLD_FUNC_BODY(0);
OLD_FUNC_BODY(1);
OLD_FUNC_BODY(2);
OLD_FUNC_BODY(3);
OLD_FUNC_BODY(4);
OLD_FUNC_BODY(5);
OLD_FUNC_BODY(6);
OLD_FUNC_BODY(7);
OLD_FUNC_BODY(8);
OLD_FUNC_BODY(9);
OLD_FUNC_BODY(10);
OLD_FUNC_BODY(11);
OLD_FUNC_BODY(12);
OLD_FUNC_BODY(13);
OLD_FUNC_BODY(14);
OLD_FUNC_BODY(15);
OLD_FUNC_BODY(16);
OLD_FUNC_BODY(17);
OLD_FUNC_BODY(18);
OLD_FUNC_BODY(19);
OLD_FUNC_BODY(20);
OLD_FUNC_BODY(21);
OLD_FUNC_BODY(22);
OLD_FUNC_BODY(23);
OLD_FUNC_BODY(24);
OLD_FUNC_BODY(25);
OLD_FUNC_BODY(26);
OLD_FUNC_BODY(27);
OLD_FUNC_BODY(28);
OLD_FUNC_BODY(29);
OLD_FUNC_BODY(30);
OLD_FUNC_BODY(31);
OLD_FUNC_BODY(32);
OLD_FUNC_BODY(33);
OLD_FUNC_BODY(34);
OLD_FUNC_BODY(35);
OLD_FUNC_BODY(36);
OLD_FUNC_BODY(37);
OLD_FUNC_BODY(38);
OLD_FUNC_BODY(39);
OLD_FUNC_BODY(40);
OLD_FUNC_BODY(41);
OLD_FUNC_BODY(42);
OLD_FUNC_BODY(43);
OLD_FUNC_BODY(44);
OLD_FUNC_BODY(45);
OLD_FUNC_BODY(46);
OLD_FUNC_BODY(47);
OLD_FUNC_BODY(48);
OLD_FUNC_BODY(49);
OLD_FUNC_BODY(50);
OLD_FUNC_BODY(51);
OLD_FUNC_BODY(52);
OLD_FUNC_BODY(53);
OLD_FUNC_BODY(54);
OLD_FUNC_BODY(55);
OLD_FUNC_BODY(56);
OLD_FUNC_BODY(57);
OLD_FUNC_BODY(58);
OLD_FUNC_BODY(59);
OLD_FUNC_BODY(60);
OLD_FUNC_BODY(61);
OLD_FUNC_BODY(62);
OLD_FUNC_BODY(63);

NEW_FUNC_BODY(0);
NEW_FUNC_BODY(1);
NEW_FUNC_BODY(2);
NEW_FUNC_BODY(3);
NEW_FUNC_BODY(4);
NEW_FUNC_BODY(5);
NEW_FUNC_BODY(6);
NEW_FUNC_BODY(7);
NEW_FUNC_BODY(8);
NEW_FUNC_BODY(9);
NEW_FUNC_BODY(10);
NEW_FUNC_BODY(11);
NEW_FUNC_BODY(12);
NEW_FUNC_BODY(13);
NEW_FUNC_BODY(14);
NEW_FUNC_BODY(15);
NEW_FUNC_BODY(16);
NEW_FUNC_BODY(17);
NEW_FUNC_BODY(18);
NEW_FUNC_BODY(19);
NEW_FUNC_BODY(20);
NEW_FUNC_BODY(21);
NEW_FUNC_BODY(22);
NEW_FUNC_BODY(23);
NEW_FUNC_BODY(24);
NEW_FUNC_BODY(25);
NEW_FUNC_BODY(26);
NEW_FUNC_BODY(27);
NEW_FUNC_BODY(28);
NEW_FUNC_BODY(29);
NEW_FUNC_BODY(30);
NEW_FUNC_BODY(31);
NEW_FUNC_BODY(32);
NEW_FUNC_BODY(33);
NEW_FUNC_BODY(34);
NEW_FUNC_BODY(35);
NEW_FUNC_BODY(36);
NEW_FUNC_BODY(37);
NEW_FUNC_BODY(38);
NEW_FUNC_BODY(39);
NEW_FUNC_BODY(40);
NEW_FUNC_BODY(41);
NEW_FUNC_BODY(42);
NEW_FUNC_BODY(43);
NEW_FUNC_BODY(44);
NEW_FUNC_BODY(45);
NEW_FUNC_BODY(46);
NEW_FUNC_BODY(47);
NEW_FUNC_BODY(48);
NEW_FUNC_BODY(49);
NEW_FUNC_BODY(50);
NEW_FUNC_BODY(51);
NEW_FUNC_BODY(52);
NEW_FUNC_BODY(53);
NEW_FUNC_BODY(54);
NEW_FUNC_BODY(55);
NEW_FUNC_BODY(56);
NEW_FUNC_BODY(57);
NEW_FUNC_BODY(58);
NEW_FUNC_BODY(59);
NEW_FUNC_BODY(60);
NEW_FUNC_BODY(61);
NEW_FUNC_BODY(62);
NEW_FUNC_BODY(63);

static void (* const old_func[MAX_TEST_PATCH_NUM])(void) = {
    OLD_FUNC(0),
    OLD_FUNC(1),
    OLD_FUNC(2),
    OLD_FUNC(3),
    OLD_FUNC(4),
    OLD_FUNC(5),
    OLD_FUNC(6),
    OLD_FUNC(7),
    OLD_FUNC(8),
    OLD_FUNC(9),
    OLD_FUNC(10),
    OLD_FUNC(11),
    OLD_FUNC(12),
    OLD_FUNC(13),
    OLD_FUNC(14),
    OLD_FUNC(15),
    OLD_FUNC(16),
    OLD_FUNC(17),
    OLD_FUNC(18),
    OLD_FUNC(19),
    OLD_FUNC(20),
    OLD_FUNC(21),
    OLD_FUNC(22),
    OLD_FUNC(23),
    OLD_FUNC(24),
    OLD_FUNC(25),
    OLD_FUNC(26),
    OLD_FUNC(27),
    OLD_FUNC(28),
    OLD_FUNC(29),
    OLD_FUNC(30),
    OLD_FUNC(31),
    OLD_FUNC(32),
    OLD_FUNC(33),
    OLD_FUNC(34),
    OLD_FUNC(35),
    OLD_FUNC(36),
    OLD_FUNC(37),
    OLD_FUNC(38),
    OLD_FUNC(39),
    OLD_FUNC(40),
    OLD_FUNC(41),
    OLD_FUNC(42),
    OLD_FUNC(43),
    OLD_FUNC(44),
    OLD_FUNC(45),
    OLD_FUNC(46),
    OLD_FUNC(47),
    OLD_FUNC(48),
    OLD_FUNC(49),
    OLD_FUNC(50),
    OLD_FUNC(51),
    OLD_FUNC(52),
    OLD_FUNC(53),
    OLD_FUNC(54),
    OLD_FUNC(55),
    OLD_FUNC(56),
    OLD_FUNC(57),
    OLD_FUNC(58),
    OLD_FUNC(59),
    OLD_FUNC(60),
    OLD_FUNC(61),
    OLD_FUNC(62),
    OLD_FUNC(63),
};

static void (* const new_func[MAX_TEST_PATCH_NUM])(void) = {
    NEW_FUNC(0),
    NEW_FUNC(1),
    NEW_FUNC(2),
    NEW_FUNC(3),
    NEW_FUNC(4),
    NEW_FUNC(5),
    NEW_FUNC(6),
    NEW_FUNC(7),
    NEW_FUNC(8),
    NEW_FUNC(9),
    NEW_FUNC(10),
    NEW_FUNC(11),
    NEW_FUNC(12),
    NEW_FUNC(13),
    NEW_FUNC(14),
    NEW_FUNC(15),
    NEW_FUNC(16),
    NEW_FUNC(17),
    NEW_FUNC(18),
    NEW_FUNC(19),
    NEW_FUNC(20),
    NEW_FUNC(21),
    NEW_FUNC(22),
    NEW_FUNC(23),
    NEW_FUNC(24),
    NEW_FUNC(25),
    NEW_FUNC(26),
    NEW_FUNC(27),
    NEW_FUNC(28),
    NEW_FUNC(29),
    NEW_FUNC(30),
    NEW_FUNC(31),
    NEW_FUNC(32),
    NEW_FUNC(33),
    NEW_FUNC(34),
    NEW_FUNC(35),
    NEW_FUNC(36),
    NEW_FUNC(37),
    NEW_FUNC(38),
    NEW_FUNC(39),
    NEW_FUNC(40),
    NEW_FUNC(41),
    NEW_FUNC(42),
    NEW_FUNC(43),
    NEW_FUNC(44),
    NEW_FUNC(45),
    NEW_FUNC(46),
    NEW_FUNC(47),
    NEW_FUNC(48),
    NEW_FUNC(49),
    NEW_FUNC(50),
    NEW_FUNC(51),
    NEW_FUNC(52),
    NEW_FUNC(53),
    NEW_FUNC(54),
    NEW_FUNC(55),
    NEW_FUNC(56),
    NEW_FUNC(57),
    NEW_FUNC(58),
    NEW_FUNC(59),
    NEW_FUNC(60),
    NEW_FUNC(61),
    NEW_FUNC(62),
    NEW_FUNC(63),
};

#ifdef __ARM_ARCH_7EM__
static uint32_t ALIGNED(0x100) remap_base[MAX_TEST_PATCH_NUM * 2];
#endif

int patch_run(void)
{
    uint32_t i;
    int error = 0;
    volatile uint32_t *rdata;

#ifdef __ARM_ARCH_7EM__

    rdata = (volatile uint32_t *)&old_data[0];

    for (i = 0; i < __NUM_LIT_PATCH; i++) {
        TRACE(2,"OLD DATA VALUE [%u]: 0x%08x", i, rdata[i]);
    }

    for (i = 0; i < (__NUM_CODE_PATCH + 31) / 32; i++) {
        TRACE(2,"OLD CODE BITMAP [%u]: 0x%08x", i, code_bitmap[i]);
    }

    patch_open((uint32_t)remap_base);

    for (i = 0; i < __NUM_LIT_PATCH; i++) {
        patch_enable(PATCH_TYPE_DATA, (uint32_t)&rdata[i], new_data[i]);
    }

    for (i = 0; i < __NUM_CODE_PATCH; i++) {
        patch_enable(PATCH_TYPE_FUNC, (uint32_t)old_func[i], (uint32_t)new_func[i]);
    }

    for (i = 0; i < __NUM_LIT_PATCH; i++) {
        if (rdata[i] == new_data[i]) {
            TRACE(2,"NEW DATA VALUE [%u]: 0x%08x OK", i, rdata[i]);
        } else {
            TRACE(3,"NEW DATA VALUE [%u]: 0x%08x / 0x%08x expected", i, rdata[i], new_data[i]);
            error = 1;;
        }
    }

    for (i = 0; i < __NUM_CODE_PATCH; i++) {
        old_func[i]();
    }
    for (i = 0; i < (__NUM_CODE_PATCH + 31) / 32; i++) {
        TRACE(2,"NEW CODE BITMAP [%u]: 0x%08x", i, code_bitmap[i]);
    }
    for (i = 0; i < __NUM_CODE_PATCH; i++) {
        if ((code_bitmap[i / 32] & (1 << (i % 32)))) {
            TRACE(1,"NEW FUNC [%u]: OK", i);
        } else {
            TRACE(1,"NEW FUNC [%u]: BAD", i);
            error = 1;
        }
    }

#else

    patch_open(0);

    rdata = (volatile uint32_t *)&old_data[0];

    for (i = 0; i < PATCH_ENTRY_NUM; i++) {
        TRACE(2,"OLD DATA VALUE [%u]: 0x%08x", i, rdata[i]);
    }

    for (i = 0; i < PATCH_ENTRY_NUM; i++) {
        patch_enable(PATCH_TYPE_DATA, (uint32_t)&rdata[i], new_data[i]);
    }

    for (i = 0; i < PATCH_ENTRY_NUM; i++) {
        if (rdata[i] == new_data[i]) {
            TRACE(2,"NEW DATA VALUE [%u]: 0x%08x OK", i, rdata[i]);
        } else {
            TRACE(3,"NEW DATA VALUE [%u]: 0x%08x / 0x%08x expected", i, rdata[i], new_data[i]);
            error = 1;;
        }
    }

    if (error) {
        return error;
    }

    patch_close();
    patch_open(0);

    for (i = 0; i < (PATCH_ENTRY_NUM + 31) / 32; i++) {
        TRACE(2,"OLD CODE BITMAP [%u]: 0x%08x", i, code_bitmap[i]);
    }

    for (i = 0; i < PATCH_ENTRY_NUM; i++) {
        patch_enable(PATCH_TYPE_FUNC, (uint32_t)old_func[i], (uint32_t)new_func[i]);
    }

    for (i = 0; i < PATCH_ENTRY_NUM; i++) {
        old_func[i]();
    }
    for (i = 0; i < (PATCH_ENTRY_NUM + 31) / 32; i++) {
        TRACE(2,"NEW CODE BITMAP [%u]: 0x%08x", i, code_bitmap[i]);
    }
    for (i = 0; i < PATCH_ENTRY_NUM; i++) {
        if ((code_bitmap[i / 32] & (1 << (i % 32)))) {
            TRACE(1,"NEW FUNC [%u]: OK", i);
        } else {
            TRACE(1,"NEW FUNC [%u]: BAD", i);
            error = 1;
        }
    }

    if (error) {
        return error;
    }

#endif

    return error;
}

void patch_test(void)
{
    int error;

    error = patch_run();

    if (error) {
        hal_cmu_simu_fail();
    } else {
        hal_cmu_simu_pass();
    }
    return;
}

#endif

