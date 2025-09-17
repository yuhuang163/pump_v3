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
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_psram.h"

#include "psram_test.h"

POSSIBLY_UNUSED
static int check8(uint32_t addr, uint8_t val)
{
    uint8_t v = read8(addr);
    if (v != val) {
        TRACE(0, "    ERROR: address 0x%08x, val 0x%02x, expect 0x%02x", addr, v, val);
        /* read again to confirm read issue */
        for (int i=0; i < 3; i++) {
            v = read8(addr);
            TRACE(0, "           address 0x%08x, val 0x%02x, expect 0x%02x", addr, v, val);
        }
        return 1;
    }
    return 0;
}

POSSIBLY_UNUSED
static int check16(uint32_t addr, uint16_t val)
{
    uint16_t v = read16(addr);
    if (v != val) {
        TRACE(0, "    ERROR: address 0x%08x, val 0x%04x, expect 0x%04x", addr, v, val);
        /* read again to confirm read issue */
        for (int i=0; i < 3; i++) {
            v = read16(addr);
            TRACE(0, "           address 0x%08x, val 0x%04x, expect 0x%04x", addr, v, val);
        }
        return 1;
    }
    return 0;
}

POSSIBLY_UNUSED
static int check32(uint32_t addr, uint32_t val)
{
    uint32_t v = read32(addr);
    if (v != val) {
        TRACE(0, "    ERROR: address 0x%08x, val 0x%08x, expect 0x%08x", addr, v, val);
        /* read again to confirm read issue */
        for (int i=0; i < 3; i++) {
            v = read32(addr);
            TRACE(0, "           address 0x%08x, val 0x%08x, expect 0x%08x", addr, v, val);
        }
        return 1;
    }
    return 0;
}

int psram_bonding_test(uint32_t test_addr)
{
    int err = 0;
    uint32_t addr = test_addr;

    for (int j = 0; j <= 8; j++) {
        uint8_t c = 1 << j;
        uint16_t v = (c << 8) | c;
        write16(addr, ~v);
        err += check16(addr, ~v);
        write16(addr, v);
        err += check16(addr, v);
    }

    if (err == 0)
        TRACE(0, "PSRAM bonding test ... OKAY");
    else
        TRACE(0, "PSRAM bonding test ... FAILED, %d errors", err);
    return err;
}

int psram_die_chipping_test(uint32_t test_addr)
{
    int err = 0;
    int row, col;
    uint32_t addr;

    for (row = 0; row < 16384; row++) {
        for (col = 0; col < 16; col += 4) {
            addr = test_addr + row * 2048 + col;
            write32(addr, 0x5555aaaa);
        }
        for (col = 0x3F8; col < 0x40E; col += 8) {
            addr = test_addr + row * 2048 + col;
            write32(addr, 0x5555aaaa);
        }
    }

    for (row = 0; row < 16384; row++) {
        for (col = 0; col < 16; col += 4) {
            addr = test_addr + row * 2048 + col;
            err += check32(addr, 0x5555aaaa);
        }
        for (col = 0x3F8; col < 0x40E; col += 8) {
            addr = test_addr + row * 2048 + col;
            err += check32(addr, 0x5555aaaa);
        }
    }

    if (err == 0)
        TRACE(0, "PSRAM die chipping test ... OKAY");
    else
        TRACE(0, "PSRAM die chipping test ... FAILED, %d errors", err);
    return err;
}

int psram_reduced_arrary_test(uint32_t test_addr)
{
    int err = 0;
    int row, col;
    uint32_t addr;

    for (row = 0; row < 16384; row++) {
        col = (row % 512) * 2;
        addr = test_addr + row * 2048 + col;
        if (row % 2 == 0)
            write32(addr, 0xffff0000);
        else
            write32(addr, 0x0000ffff);
    }

    for (row = 0; row < 16384; row++) {
        col = (row % 512) * 2;
        addr = test_addr + row * 2048 + col;
        if (row % 2 == 0)
            err += check32(addr, 0xffff0000);
        else
            err += check32(addr, 0x0000ffff);
    }

    if (err == 0)
        TRACE(0, "PSRAM reduced array test ... OKAY");
    else
        TRACE(0, "PSRAM reduced array test ... FAILED, %d errors", err);
    return err;
}

int psram_test_8bits(uint32_t test_addr, uint32_t test_size, TEST_PATTERN_TYPE pattern_type)
{
    int err = 0;
    uint32_t addr;

    if (pattern_type == TEST_FIX2_PATTERN) {
        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            write8(addr, 0x5a);
            write8(addr+1, 0xa5);
        }

        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            err += check8(addr, 0x5a);
            err += check8(addr+1, 0xa5);
        }

        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            write8(addr, 0xa5);
            write8(addr+1, 0x5a);
        }

        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            err += check8(addr, 0xa5);
            err += check8(addr+1, 0x5a);
        }
    }
    else if (pattern_type == TEST_FIX4_PATTERN) {
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            write8(addr, 0x5a);
            write8(addr+1, 0xaa);
            write8(addr+2, 0x55);
            write8(addr+3, 0xa5);
        }

        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            err += check8(addr, 0x5a);
            err += check8(addr+1, 0xaa);
            err += check8(addr+2, 0x55);
            err += check8(addr+3, 0xa5);
        }

        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            write8(addr, 0xa5);
            write8(addr+1, 0x55);
            write8(addr+2, 0xaa);
            write8(addr+3, 0x5a);
        }

        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            err += check8(addr, 0xa5);
            err += check8(addr+1, 0x55);
            err += check8(addr+2, 0xaa);
            err += check8(addr+3, 0x5a);
        }
    }
    else if (pattern_type == TEST_FFF_PATTERN) {
        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            write8(addr, 0x0);
            write8(addr+1, 0xff);
        }

        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            err += check8(addr, 0x0);
            err += check8(addr+1, 0xff);
        }

        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            write8(addr, 0xff);
            write8(addr+1, 0);
        }

        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            err += check8(addr, 0xff);
            err += check8(addr+1, 0);
        }
    }
    else if (pattern_type == TEST_INC_PATTERN) {
        uint8_t p = 0;
        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            write8(addr, p++);
            write8(addr+1, p++);
        }

        p = 0;
        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            err += check8(addr, p++);
            err += check8(addr+1, p++);
        }

        p = 0;
        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            write8(addr, p++);
            write8(addr+1, p++);
        }

        p = 0;
        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            err += check8(addr, p++);
            err += check8(addr+1, p++);
        }
    }
    else if (pattern_type == TEST_DEC_PATTERN) {
        uint8_t p = 0xff;
        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            write8(addr, p--);
            write8(addr+1, p--);
        }

        p = 0xff;
        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            err += check8(addr, p--);
            err += check8(addr+1, p--);
        }

        p = 0xff;
        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            write8(addr, p--);
            write8(addr+1, p--);
        }

        p = 0xff;
        for (int i = 0; i < test_size; i += 2) {
            addr = test_addr + i;
            err += check8(addr, p--);
            err += check8(addr+1, p--);
        }
    }

    if (err == 0)
        TRACE(0, "PSRAM access 8bits test ... OKAY");
    else
        TRACE(0, "PSRAM access 8bits test ... FAILED, %d errors", err);
    return err;
}

int psram_test_16bits(uint32_t test_addr, uint32_t test_size, TEST_PATTERN_TYPE pattern_type)
{
    int err = 0;
    uint32_t addr;

    if (pattern_type == TEST_FIX2_PATTERN) {
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            write16(addr, 0x55aa);
            write16(addr+2, 0xaa55);
        }

        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            err += check16(addr, 0x55aa);
            err += check16(addr+2, 0xaa55);
        }

        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            write16(addr, 0xaa55);
            write16(addr+2, 0x55aa);
        }

        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            err += check16(addr, 0xaa55);
            err += check16(addr+2, 0x55aa);
        }
    }
    else if (pattern_type == TEST_FIX4_PATTERN) {
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            write16(addr, 0x5a5a);
            write16(addr+2, 0xaa55);
            write16(addr+4, 0x55aa);
            write16(addr+6, 0xa5a5);
        }

        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            err += check16(addr, 0x5a5a);
            err += check16(addr+2, 0xaa55);
            err += check16(addr+4, 0x55aa);
            err += check16(addr+6, 0xa5a5);
        }

        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            write16(addr, 0xa5a5);
            write16(addr+2, 0x55aa);
            write16(addr+4, 0xaa55);
            write16(addr+6, 0x5a5a);
        }

        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            err += check16(addr, 0xa5a5);
            err += check16(addr+2, 0x55aa);
            err += check16(addr+4, 0xaa55);
            err += check16(addr+6, 0x5a5a);
        }
    }
    else if (pattern_type == TEST_FFF_PATTERN) {
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            write16(addr, 0x0);
            write16(addr+2, 0xffff);
        }

        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            err += check16(addr, 0x0);
            err += check16(addr+2, 0xffff);
        }

        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            write16(addr, 0xffff);
            write16(addr+2, 0);
        }

        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            err += check16(addr, 0xffff);
            err += check16(addr+2, 0);
        }
    }
    else if (pattern_type == TEST_INC_PATTERN) {
        uint16_t p = 0;
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            write16(addr, p);
            p += 0x0101;
            write16(addr+2, p);
            p += 0x0101;
        }

        p = 0;
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            err += check16(addr, p);
            p += 0x0101;
            err += check16(addr+2, p);
            p += 0x0101;
        }

        p = 0;
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            write16(addr, p);
            p += 0x0101;
            write16(addr+2, p);
            p += 0x0101;
        }

        p = 0;
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            err += check16(addr, p);
            p += 0x0101;
            err += check16(addr+2, p);
            p += 0x0101;
        }
    }
    else if (pattern_type == TEST_DEC_PATTERN) {
        uint16_t p = 0xffff;
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            write16(addr, p);
            p -= 0x0101;
            write16(addr+2, p);
            p -= 0x0101;
        }

        p = 0xffff;
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            err += check16(addr, p);
            p -= 0x0101;
            err += check16(addr+2, p);
            p -= 0x0101;
        }

        p = 0xffff;
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            write16(addr, p);
            p -= 0x0101;
            write16(addr+2, p);
            p -= 0x0101;
        }

        p = 0xffff;
        for (int i = 0; i < test_size; i += 4) {
            addr = test_addr + i;
            err += check16(addr, p);
            p -= 0x0101;
            err += check16(addr+2, p);
            p -= 0x0101;
        }
    }

    if (err == 0)
        TRACE(0, "PSRAM access 16bits test ... OKAY");
    else
        TRACE(0, "PSRAM access 16bits test ... FAILED, %d errors", err);
    return err;
}

int psram_test_32bits(uint32_t test_addr, uint32_t test_size, TEST_PATTERN_TYPE pattern_type)
{
    int err = 0;
    uint32_t addr;

    if (pattern_type == TEST_FIX2_PATTERN) {
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            write32(addr, 0x5555aaaa);
            write32(addr+4, 0xaaaa5555);
        }

        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            err += check32(addr, 0x5555aaaa);
            err += check32(addr+4, 0xaaaa5555);
        }

        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            write32(addr, 0xaaaa5555);
            write32(addr+4, 0x5555aaaa);
        }

        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            err += check32(addr, 0xaaaa5555);
            err += check32(addr+4, 0x5555aaaa);
        }
    }
    else if (pattern_type == TEST_FIX4_PATTERN) {
        for (int i = 0; i < test_size; i += 16) {
            addr = test_addr + i;
            write32(addr, 0x5a5a5a5a);
            write32(addr+4, 0xaaaa5555);
            write32(addr+8, 0x5555aaaa);
            write32(addr+12, 0xa5a5a5a5);
        }

        for (int i = 0; i < test_size; i += 16) {
            addr = test_addr + i;
            err += check32(addr, 0x5a5a5a5a);
            err += check32(addr+4, 0xaaaa5555);
            err += check32(addr+8, 0x5555aaaa);
            err += check32(addr+12, 0xa5a5a5a5);
        }

        for (int i = 0; i < test_size; i += 16) {
            addr = test_addr + i;
            write32(addr, 0xa5a5a5a5);
            write32(addr+4, 0x5555aaaa);
            write32(addr+8, 0xaaaa5555);
            write32(addr+12, 0x5a5a5a5a);
        }

        for (int i = 0; i < test_size; i += 16) {
            addr = test_addr + i;
            err += check32(addr, 0xa5a5a5a5);
            err += check32(addr+4, 0x5555aaaa);
            err += check32(addr+8, 0xaaaa5555);
            err += check32(addr+12, 0x5a5a5a5a);
        }
    }
    else if (pattern_type == TEST_FFF_PATTERN) {
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            write32(addr, 0);
            write32(addr+4, 0xffffffff);
        }

        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            err += check32(addr, 0);
            err += check32(addr+4, 0xffffffff);
        }

        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            write32(addr, 0xffffffff);
            write32(addr+4, 0);
        }

        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            err += check32(addr, 0xffffffff);
            err += check32(addr+4, 0);
        }
    }
    else if (pattern_type == TEST_INC_PATTERN) {
        uint32_t p = 0;
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            write32(addr, p);
            p += 0x01010101;
            write32(addr+4, p);
            p += 0x01010101;
        }

        p = 0;
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            err += check32(addr, p);
            p += 0x01010101;
            err += check32(addr+4, p);
            p += 0x01010101;
        }

        p = 0;
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            write32(addr, p);
            p += 0x01010101;
            write32(addr+4, p);
            p += 0x01010101;
        }

        p = 0;
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            err += check32(addr, p);
            p += 0x01010101;
            err += check32(addr+4, p);
            p += 0x01010101;
        }
    }
    else if (pattern_type == TEST_DEC_PATTERN) {
        uint32_t p = 0xffffffff;
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            write32(addr, p);
            p -= 0x01010101;
            write32(addr+4, p);
            p -= 0x01010101;
        }

        p = 0xffffffff;
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            err += check32(addr, p);
            p -= 0x01010101;
            err += check32(addr+4, p);
            p -= 0x01010101;
        }

        p = 0xffffffff;
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            write32(addr, p);
            p -= 0x01010101;
            write32(addr+4, p);
            p -= 0x01010101;
        }

        p = 0xffffffff;
        for (int i = 0; i < test_size; i += 8) {
            addr = test_addr + i;
            err += check32(addr, p);
            p -= 0x01010101;
            err += check32(addr+4, p);
            p -= 0x01010101;
        }
    }

    if (err == 0)
        TRACE(0, "PSRAM access 32bits test ... OKAY");
    else
        TRACE(0, "PSRAM access 32bits test ... FAILED, %d errors", err);
    return err;
}

