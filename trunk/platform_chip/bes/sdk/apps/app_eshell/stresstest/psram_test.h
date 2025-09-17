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

#ifndef __PSRAM_TEST_H__
#define __PSRAM_TEST_H__

typedef enum {
    TEST_FIX2_PATTERN,
    TEST_FIX4_PATTERN,
    TEST_FFF_PATTERN,
    TEST_INC_PATTERN,
    TEST_DEC_PATTERN,
} TEST_PATTERN_TYPE;

#define write8(reg, val) \
    do { \
        *(volatile uint8_t *)(reg) = val; \
    } while(0)

#define read8(reg) \
    (*((volatile uint8_t *)(reg)))

#define write16(reg, val) \
    do { \
        *(volatile uint16_t *)(reg) = val; \
    } while(0)

#define read16(reg) \
    (*((volatile uint16_t *)(reg)))

#define write32(reg, val) \
    do { \
        *(volatile uint32_t *)(reg) = val; \
    } while(0)

#define read32(reg) \
    (*((volatile uint32_t *)(reg)))

int psram_bonding_test(uint32_t test_addr);
int psram_die_chipping_test(uint32_t test_addr);
int psram_reduced_arrary_test(uint32_t test_addr);

int psram_test_8bits(uint32_t test_addr, uint32_t test_size, TEST_PATTERN_TYPE pattern_type);
int psram_test_16bits(uint32_t test_addr, uint32_t test_size, TEST_PATTERN_TYPE pattern_type);
int psram_test_32bits(uint32_t test_addr, uint32_t test_size, TEST_PATTERN_TYPE pattern_type);

#endif /* __RAMRW_TEST_H__ */
