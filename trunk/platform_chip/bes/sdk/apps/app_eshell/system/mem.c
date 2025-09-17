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

#ifdef UTILS_ESHELL_EN

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
#include "pmu.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"

static void mem_dump_1byte(uint32_t addr, uint32_t size)
{
    volatile uint8_t *reg = (volatile uint8_t *)addr;
    uint8_t val;
    for (uint32_t i = 0; i < size; i++) {
        val = (uint8_t)(*reg & 0xff);
        if (i % 16 == 0)
            eshell_putstring("\r\n%p: ", reg);
        eshell_putstring("%02x ", val);
        reg++;
    }
    eshell_putstring("\r\n");
}

static void mem_dump_2bytes(uint32_t addr, uint32_t size)
{
    volatile uint16_t *reg = (volatile uint16_t *)addr;
    uint16_t val;
    for (uint32_t i = 0; i < size; i++) {
        val = (uint16_t)(*reg & 0xffff);
        if (i % 8 == 0)
            eshell_putstring("\r\n%p: ", reg);
        eshell_putstring("%04x ", val);
        reg++;
    }
    eshell_putstring("\r\n");
}

static void mem_dump_4bytes(uint32_t addr, uint32_t size)
{
    volatile uint32_t *reg = (volatile uint32_t *)addr;
    uint32_t val;
    for (uint32_t i = 0; i < size; i++) {
        val = (uint32_t)(*reg);
        if (i % 8 == 0)
            eshell_putstring("\r\n%p: ", reg);
        eshell_putstring("%08x ", val);
        reg++;
    }
    eshell_putstring("\r\n");
}

static int mem_compare_1byte(uint32_t addr1, uint32_t addr2, uint32_t size)
{
    volatile uint8_t *reg1 = (volatile uint8_t *)addr1;
    volatile uint8_t *reg2 = (volatile uint8_t *)addr2;
    uint8_t val1, val2;
    bool diff = false;

    for (uint32_t i = 0; i < size; i++) {
        val1 = (uint8_t)(*reg1 & 0xff);
        val2 = (uint8_t)(*reg2 & 0xff);
        if (val1 != val2){
            eshell_putstring("%p: 0x%02x != %p: 0x%02x\r\n",
                            reg1, val1, reg2, val2);
            diff = true;
        }
        reg1++;
        reg2++;
    }

    if (diff)
        eshell_putstring("compare %u bytes on 0x%x and 0x%x: NOT same.\r\n",
                        size, addr1, addr2);
    else
        eshell_putstring("compare %u bytes on 0x%x and 0x%x: same.\r\n",
                        size, addr1, addr2);
    return diff ? -1 : 0;
}

static int mem_compare_2bytes(uint32_t addr1, uint32_t addr2, uint32_t size)
{
    volatile uint16_t *reg1 = (volatile uint16_t *)addr1;
    volatile uint16_t *reg2 = (volatile uint16_t *)addr2;
    uint16_t val1, val2;
    bool diff = false;

    for (uint32_t i = 0; i < size; i++) {
        val1 = (uint16_t)(*reg1 & 0xffff);
        val2 = (uint16_t)(*reg2 & 0xffff);
        if (val1 != val2){
            eshell_putstring("%p: 0x%04x != %p: 0x%04x\r\n",
                            reg1, val1, reg2, val2);
            diff = true;
        }
        reg1++;
        reg2++;
    }

    if (diff)
        eshell_putstring("compare %u bytes on 0x%x and 0x%x: NOT same.\r\n",
                        size * 2, addr1, addr2);
    else
        eshell_putstring("compare %u bytes on 0x%x and 0x%x: same.\r\n",
                        size * 2, addr1, addr2);
    return diff ? -1 : 0;
}

static int mem_compare_4bytes(uint32_t addr1, uint32_t addr2, uint32_t size)
{
    volatile uint32_t *reg1 = (volatile uint32_t *)addr1;
    volatile uint32_t *reg2 = (volatile uint32_t *)addr2;
    uint32_t val1, val2;
    bool diff = false;

    for (uint32_t i = 0; i < size; i++) {
        val1 = (uint32_t)(*reg1);
        val2 = (uint32_t)(*reg2);
        if (val1 != val2){
            eshell_putstring("%p: 0x%08x != %p: 0x%08x\r\n",
                            reg1, val1, reg2, val2);
            diff = true;
        }
        reg1++;
        reg2++;
    }

    if (diff)
        eshell_putstring("compare %u bytes on 0x%x and 0x%x: NOT same.\r\n",
                        size * 4, addr1, addr2);
    else
        eshell_putstring("compare %u bytes on 0x%x and 0x%x: same.\r\n",
                        size * 4, addr1, addr2);
    return diff ? -1 : 0;
}

static void mem_write_1byte(uint32_t addr, uint8_t val, uint32_t size)
{
    volatile uint8_t *reg = (volatile uint8_t *)addr;
    for (uint32_t i = 0; i < size; i++) {
        *reg = val;
        reg++;
    }
    eshell_putstring("Write %u bytes to 0x%08x\r\n", size, addr);
}

static void mem_write_2bytes(uint32_t addr, uint16_t val, uint32_t size)
{
    volatile uint16_t *reg = (volatile uint16_t *)addr;
    for (uint32_t i = 0; i < size; i++) {
        *reg = val;
        reg++;
    }
    eshell_putstring("Write %u bytes to 0x%08x\r\n", size * 2, addr);
}

static void mem_write_4bytes(uint32_t addr, uint32_t val, uint32_t size)
{
    volatile uint32_t *reg = (volatile uint32_t *)addr;
    for (uint32_t i = 0; i < size; i++) {
        *reg = val;
        reg++;
    }
    eshell_putstring("Write %u bytes to 0x%08x\r\n", size * 4, addr);
}


static void md_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  md addr len\r\n");
    eshell_putstring("      addr: memory or register address, in HEX.\r\n");
    eshell_putstring("      len:  how many bytes to dump, default is 1.\r\n");
    eshell_putstring("  md16 addr len\r\n");
    eshell_putstring("      addr: memory or register address, in HEX.\r\n");
    eshell_putstring("      len:  how many words (16bits) to dump, default is 1.\r\n");
    eshell_putstring("  md32 addr len\r\n");
    eshell_putstring("      addr: memory or register address, in HEX.\r\n");
    eshell_putstring("      len:  how many long-integers (32bits) to dump, default is 1.\r\n");
}

static void do_mem_read(int argc, char *argv[])
{
    uint32_t addr;
    uint32_t size = 1;

    if (argc < 2)
        goto usage;

    addr = strtoul(argv[1], NULL, 16);

    if (argc >= 3) {
        size = strtoul(argv[2], NULL, 10);
        if (size > 0x100000)
            size = 0x100000;
        if (size < 1)
            size = 1;
    }

    mem_dump_1byte(addr, size);
    return;
usage:
    md_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "md", "dump memory or registers", do_mem_read);

static void do_mem_read_2bytes(int argc, char *argv[])
{
    uint32_t addr;
    uint32_t size = 1;

    if (argc < 2)
        goto usage;

    addr = strtoul(argv[1], NULL, 16);

    if (argc >= 3) {
        size = strtoul(argv[2], NULL, 10);
        if (size > 0x100000)
            size = 0x100000;
        if (size < 1)
            size = 1;
    }

    mem_dump_2bytes(addr, size);
    return;
usage:
    md_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "md16", "dump memory or registers in 2 bytes", do_mem_read_2bytes);


static void do_mem_read_4bytes(int argc, char *argv[])
{
    uint32_t addr;
    uint32_t size = 1;

    if (argc < 2)
        goto usage;

    addr = strtoul(argv[1], NULL, 16);

    if (argc >= 3) {
        size = strtoul(argv[2], NULL, 10);
        if (size > 0x100000)
            size = 0x100000;
        if (size < 1)
            size = 1;
    }

    mem_dump_4bytes(addr, size);
    return;
usage:
    md_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "md32", "dump memory or registers in 4 bytes", do_mem_read_4bytes);

static void mw_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  mw addr val len\r\n");
    eshell_putstring("      addr: memory or register address, in HEX.\r\n");
    eshell_putstring("      val:  the value to be written, in HEX, 1 byte.\r\n");
    eshell_putstring("      len:  how many bytes to write, default is 1.\r\n");
    eshell_putstring("  mw16 addr val len\r\n");
    eshell_putstring("      addr: memory or register address, in HEX.\r\n");
    eshell_putstring("      val:  the value to be written, in HEX, 2 bytes.\r\n");
    eshell_putstring("      len:  how many words (16bits) to write, default is 1.\r\n");
    eshell_putstring("  mw32 addr val len\r\n");
    eshell_putstring("      addr: memory or register address, in HEX.\r\n");
    eshell_putstring("      val:  the value to be written, in HEX, 4 bytes.\r\n");
    eshell_putstring("      len:  how many long-integers (32bits) to write, default is 1.\r\n");
}

static void do_mem_write(int argc, char *argv[])
{
    uint32_t addr;
    uint32_t size = 1;
    uint32_t val;

    if (argc < 3)
        goto usage;

    addr = strtoul(argv[1], NULL, 16);
    val = strtoul(argv[2], NULL, 16);

    if (argc > 3) {
        size = strtoul(argv[3], NULL, 10);
        if (size > 0x100000)
            size = 0x100000;
        if (size < 1)
            size = 1;
    }

    mem_write_1byte(addr, (uint8_t)(val & 0xff), size);
    return;
usage:
    mw_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "mw", "write data to memory or registers", do_mem_write);

static void do_mem_write_2bytes(int argc, char *argv[])
{
    uint32_t addr;
    uint32_t size = 1;
    uint32_t val;

    if (argc < 3)
        goto usage;

    addr = strtoul(argv[1], NULL, 16);
    val = strtoul(argv[2], NULL, 16);

    if (argc > 3) {
        size = strtoul(argv[3], NULL, 10);
        if (size > 0x100000)
            size = 0x100000;
        if (size < 1)
            size = 1;
    }

    mem_write_2bytes(addr, (uint16_t)(val & 0xffff), size);
    return;
usage:
    mw_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "mw16", "write data to memory or registers in 2 bytes", do_mem_write_2bytes);

static void do_mem_write_4bytes(int argc, char *argv[])
{
    uint32_t addr;
    uint32_t size = 1;
    uint32_t val;

    if (argc < 3)
        goto usage;

    addr = strtoul(argv[1], NULL, 16);
    val = strtoul(argv[2], NULL, 16);

    if (argc > 3) {
        size = strtoul(argv[3], NULL, 10);
        if (size > 0x100000)
            size = 0x100000;
        if (size < 1)
            size = 1;
    }

    mem_write_4bytes(addr, val, size);
    return;
usage:
    mw_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "mw32", "write data to memory or registers in 4 bytes", do_mem_write_4bytes);

static void mem_compare_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  comp addr1 addr2 len\r\n");
    eshell_putstring("      addr1: memory or register address, in HEX.\r\n");
    eshell_putstring("      addr2: memory or register address, in HEX.\r\n");
    eshell_putstring("      len:   how many bytes to compare, default is 4.\r\n");
    eshell_putstring("  comp16 addr1 addr2 len\r\n");
    eshell_putstring("      addr1: memory or register address, in HEX.\r\n");
    eshell_putstring("      addr2: memory or register address, in HEX.\r\n");
    eshell_putstring("      len:   how many words (16bits) to compare, default is 2.\r\n");
    eshell_putstring("  comp32 addr1 addr2 len\r\n");
    eshell_putstring("      addr1: memory or register address, in HEX.\r\n");
    eshell_putstring("      addr2: memory or register address, in HEX.\r\n");
    eshell_putstring("      len:   how many long-integers (32bits) to compare, default is 1.\r\n");
}

static void do_mem_compare(int argc, char *argv[])
{
    uint32_t addr1, addr2;
    uint32_t size = 4;

    if (argc < 3)
        goto usage;

    addr1 = strtoul(argv[1], NULL, 16);
    addr2 = strtoul(argv[2], NULL, 16);

    if (argc > 3) {
        size = strtoul(argv[3], NULL, 10);
        if (size > 0x100000)
            size = 0x100000;
        if (size < 4)
            size = 4;
    }

    mem_compare_1byte(addr1, addr2, size);
    return;
usage:
    mem_compare_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "comp", "compare data of two addresses", do_mem_compare);

static void do_mem_compare_2bytes(int argc, char *argv[])
{
    uint32_t addr1, addr2;
    uint32_t size = 2;

    if (argc < 3)
        goto usage;

    addr1 = strtoul(argv[1], NULL, 16);
    addr2 = strtoul(argv[2], NULL, 16);

    if (argc > 3) {
        size = strtoul(argv[3], NULL, 10);
        if (size > 0x100000)
            size = 0x100000;
        if (size < 4)
            size = 4;
    }

    mem_compare_2bytes(addr1, addr2, size);
    return;
usage:
    mem_compare_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "comp16", "compare data of two addresses, in 2 bytes", do_mem_compare_2bytes);

static void do_mem_compare_4bytes(int argc, char *argv[])
{
    uint32_t addr1, addr2;
    uint32_t size = 2;

    if (argc < 3)
        goto usage;

    addr1 = strtoul(argv[1], NULL, 16);
    addr2 = strtoul(argv[2], NULL, 16);

    if (argc > 3) {
        size = strtoul(argv[3], NULL, 10);
        if (size > 0x100000)
            size = 0x100000;
        if (size < 4)
            size = 4;
    }

    mem_compare_4bytes(addr1, addr2, size);
    return;
usage:
    mem_compare_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "comp32", "compare data of two addresses, in 4 bytes", do_mem_compare_4bytes);

#ifdef CHIP_BEST1600
static void do_mem_map(int argc, char *argv[])
{
    eshell_putstring("SRAM:\r\n");
    eshell_putstring("  M55_ITCM_BASE = 0x%08x\r\n", M55_ITCM_BASE);
    eshell_putstring("  M55_ITCM_SIZE = 0x%08x\r\n", M55_ITCM_SIZE);
    eshell_putstring("  M55_DTCM_BASE = 0x%08x\r\n", M55_DTCM_BASE);
    eshell_putstring("  M55_DTCM_SIZE = 0x%08x\r\n", M55_DTCM_SIZE);
    eshell_putstring("  M55_SYS_RAM_BASE = 0x%08x\r\n", M55_SYS_RAM_BASE);
    eshell_putstring("  M55_SYS_RAM_SIZE = 0x%08x\r\n", M55_SYS_RAM_SIZE);
    eshell_putstring("  HIFI4_ITCM_BASE = 0x%08x\r\n", HIFI4_ITCM_BASE);
    eshell_putstring("  HIFI4_ITCM_SIZE = 0x%08x\r\n", HIFI4_ITCM_SIZE);
    eshell_putstring("  HIFI4_DTCM_BASE = 0x%08x\r\n", HIFI4_DTCM_BASE);
    eshell_putstring("  HIFI4_DTCM_SIZE = 0x%08x\r\n", HIFI4_DTCM_SIZE);
    eshell_putstring("  HIFI4_SYS_RAM_BASE = 0x%08x\r\n", HIFI4_SYS_RAM_BASE);
    eshell_putstring("  HIFI4_SYS_RAM_SIZE = 0x%08x\r\n", HIFI4_SYS_RAM_SIZE);
    eshell_putstring("  BTH_RAM_BASE = 0x%08x\r\n", BTH_RAM_BASE);
    eshell_putstring("  BTH_RAM_SIZE = 0x%08x\r\n", BTH_RAM_SIZE);
    eshell_putstring("  SENS_RAM_BASE = 0x%08x\r\n", SENS_RAM_BASE);
    eshell_putstring("  SENS_RAM_SIZE = 0x%08x\r\n", SENS_RAM_SIZE);
#if defined(PSRAM_ENABLE) && defined(PSRAM_BASE) && defined(PSRAM_SIZE)
    eshell_putstring("PSRAM:\r\n");
    eshell_putstring("  PSRAM_BASE = 0x%08x\r\n", PSRAM_BASE);
    eshell_putstring("  PSRAM_SIZE = 0x%08x\r\n", PSRAM_SIZE);
#endif
    eshell_putstring("FLASH:\r\n");
    eshell_putstring("  FLASH_BASE = 0x%08x\r\n", FLASH_BASE);
    eshell_putstring("  FLASH_SIZE = 0x%08x\r\n", FLASH_SIZE);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "map", "show memory map", do_mem_map);
#endif

#endif // UTILS_ESHELL_EN
