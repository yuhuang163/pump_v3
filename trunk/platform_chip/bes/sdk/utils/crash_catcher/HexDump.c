/* Copyright (C) 2018  Adam Green (https://github.com/adamgreen)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include <CrashCatcher.h>
#include "hal_timer.h"
#include "hal_trace.h"
#include <string.h>
#include "cmsis.h"
#include "xyzmodem.h"
#ifdef CORE_DUMP_TO_FLASH
#include "coredump_section.h"
#endif
#ifdef __ARMCC_VERSION
#include "link_sym_armclang.h"
#endif
#ifdef CORE_DUMP_TO_SPI
#include "test_spi_slave.h"
#endif
#ifdef USER_DATA_TO_FLASH
#include "user_data_section.h"
#endif
#include "app_flash_api.h"
#include "crc32_c.h"

static DUMP_INFO_T dump_info = {0};

#ifdef TRACE_CRLF
#define NEW_LINE_STR                    "\r\n"
#else
#define NEW_LINE_STR                    "\n"
#endif

//CRASH_CATCHER_TEST_WRITEABLE CrashCatcherReturnCodes g_crashCatcherDumpEndReturn = CRASH_CATCHER_TRY_AGAIN;
static void printString(const char *pString);
//static void waitForUserInput(void);
static void dumpBytes(const uint8_t* pMemory, size_t elementCount);
static void dumpByteAsHex(uint8_t byte);
static void dumpHexDigit(uint8_t nibble);
static void dumpHalfwords(const uint16_t* pMemory, size_t elementCount);
static void dumpWords(const uint32_t* pMemory, size_t elementCount);

void CrashCatcher_putc(char c)
{
    static uint32_t cnt = 0;
    hal_trace_output((const unsigned char *)&c,1);
    cnt++;
    if (cnt % 1024 == 0)
        hal_trace_flush_buffer();
}

extern uint32_t __StackLimit[];
extern uint32_t __HeapLimit[];

const CrashCatcherMemoryRegion* CrashCatcher_GetMemoryRegions(void)
{
#ifdef PSRAM_ENABLE
    static CrashCatcherMemoryRegion regions[5];
#else
    static CrashCatcherMemoryRegion regions[4];
#endif
    int j=0;

    regions[j].startAddress = (uint32_t)RAM0_BASE;
    regions[j].endAddress = (uint32_t)&__HeapLimit;
    regions[j].elementSize = CRASH_CATCHER_BYTE;
    j++;

    regions[j].startAddress = (uint32_t)&__StackLimit;
    regions[j].endAddress = (uint32_t)(RAM0_BASE + RAM_TOTAL_SIZE);
    regions[j].elementSize = CRASH_CATCHER_BYTE;
    j++;

    regions[j].startAddress = (uint32_t)DWT_BASE;
    regions[j].endAddress = (uint32_t)(DWT_BASE + 0x1000);
    regions[j].elementSize = CRASH_CATCHER_BYTE;
    j++;

#ifdef PSRAM_ENABLE
    regions[j].startAddress = (uint32_t)PSRAM_BASE;
    regions[j].endAddress = (uint32_t)(PSRAM_BASE + PSRAM_SIZE);
    regions[j].elementSize = CRASH_CATCHER_BYTE;
    j++;
#endif

    regions[j].startAddress = 0xFFFFFFFF;
    regions[j].endAddress = 0xFFFFFFFF;
    regions[j].elementSize = CRASH_CATCHER_BYTE;
    return regions;
}

const CrashCatcherMemoryRegion* CrashCatcher_GetSramMemoryRegions(void)
{
    static CrashCatcherMemoryRegion regions[2];
    int j=0;

    regions[j].startAddress = (uint32_t)RAM0_BASE;
    regions[j].endAddress = (uint32_t)(RAM0_BASE + RAM_TOTAL_SIZE);
    regions[j].elementSize = CRASH_CATCHER_BYTE;
    j++;

    regions[j].startAddress = 0xFFFFFFFF;
    regions[j].endAddress = 0xFFFFFFFF;
    regions[j].elementSize = CRASH_CATCHER_BYTE;
    return regions;
}

static enum {
    DUMP_TERM,
    DUMP_XMODEM,
    DUMP_FLASH,
    DUMP_SPI,
} dump_direction = DUMP_TERM;

int CrashCatcher_DumpStart(const CrashCatcherInfo* pInfo)
{
    printString(NEW_LINE_STR NEW_LINE_STR);
    printString("CRASH ENCOUNTERED" NEW_LINE_STR NEW_LINE_STR);
    //waitForUserInput();
    hal_trace_flush_buffer();

#ifdef CORE_DUMP_TO_FLASH
    core_dump_erase_section();
    dump_direction = DUMP_FLASH;
    return 0;
#elif defined (CORE_DUMP_TO_SPI)
    dump_direction = DUMP_SPI;
    return 0;
#else
    int ret;
    ret = xmodem_start_xfer(120);
    if (ret < 0) {
        // Error
        return ret;
    }
    if (ret > 0) {
        dump_direction = DUMP_TERM;
        return 0;
    }

    dump_direction = DUMP_XMODEM;
    return 0;
#endif
}

static void printString(const char *pString)
{
    while (*pString)
        CrashCatcher_putc(*pString++);
}

#if 0
static void waitForUserInput(void)
{
    CrashCatcher_getc();
}
#endif

void CrashCatcher_DumpMemory(const void* pvMemory, CrashCatcherElementSizes elementSize, size_t elementCount)
{
    switch (elementSize)
    {
    case CRASH_CATCHER_BYTE:
        dumpBytes(pvMemory, elementCount);
        break;
    case CRASH_CATCHER_HALFWORD:
        dumpHalfwords(pvMemory, elementCount);
        break;
    case CRASH_CATCHER_WORD:
        dumpWords(pvMemory, elementCount);
        break;
    }
    printString(NEW_LINE_STR);

#ifdef CORE_DUMP_TO_FLASH
    if (elementSize * elementCount >= COREDUMP_SECTOR_SIZE)
    {
        core_dump_write_large(pvMemory, elementSize * elementCount);
    }
    else
    {
        core_dump_write(pvMemory, elementSize * elementCount);
    }
    uint8_t norflash_read_buf[1024] = {0};
    uint32_t read_cnt = (elementSize * elementCount + 1024 - 1) / 1024;
    for (uint32_t i=0; i<read_cnt; i++) {
        core_dump_read_data(norflash_read_buf, 1024, i*1024);
        dump_info.crc = crc32_c(dump_info.crc, norflash_read_buf, 1024);
    }
    dump_info.len += (elementSize * elementCount);
#elif defined (CORE_DUMP_TO_SPI)
    xtc_spi_slave_dump_crash(pvMemory, elementSize * elementCount, NULL);
#endif
}

static void dumpBytes(const uint8_t* pMemory, size_t elementCount)
{
    size_t i;

    if (dump_direction == DUMP_TERM) {
        for (i = 0 ; i < elementCount ; i++) {
            /* Only dump 16 bytes to a single line before introducing a line break. */
            if (i != 0 && (i & 0xF) == 0) {
                printString(NEW_LINE_STR);
                hal_trace_flush_buffer();
            }
            dumpByteAsHex(*pMemory++);
        }
    } else if (dump_direction == DUMP_XMODEM) {
        const uint8_t *buf = pMemory;
        int len;

        len = xmodem_send_stream(buf, elementCount, 0);
        if (len < 0) {
            //printString("#####error");
        }
    }
}

static void dumpByteAsHex(uint8_t byte)
{
    dumpHexDigit(byte >> 4);
    dumpHexDigit(byte & 0xF);
}

static void dumpHexDigit(uint8_t nibble)
{
    static const char hexToASCII[] = "0123456789ABCDEF";

    CrashCatcher_putc(hexToASCII[nibble]);
}

static void dumpHalfwords(const uint16_t* pMemory, size_t elementCount)
{
    size_t i;
    for (i = 0 ; i < elementCount ; i++)
    {
        uint16_t val = *pMemory++;
        /* Only dump 8 halfwords to a single line before introducing a line break. */
        if (i != 0 && (i & 0x7) == 0)
            printString(NEW_LINE_STR);
        dumpBytes((uint8_t*)&val, sizeof(val));
    }
}

static void dumpWords(const uint32_t* pMemory, size_t elementCount)
{
    size_t i;
    for (i = 0 ; i < elementCount ; i++)
    {
        uint32_t val = *pMemory++;
        /* Only dump 4 words to a single line before introducing a line break. */
        if (i != 0 && (i & 0x3) == 0) {
            printString(NEW_LINE_STR);
        }
        dumpBytes((uint8_t*)&val, sizeof(val));
    }
}

CrashCatcherReturnCodes CrashCatcher_DumpEnd(void)
{
    if (dump_direction == DUMP_XMODEM) {
        xmodem_stop_xfer();
    } else {
        static const char end_info[] = NEW_LINE_STR "End of crash dump" NEW_LINE_STR;

        printString(end_info);

#ifdef CORE_DUMP_TO_SPI
        xtc_spi_slave_dump_crash(NULL, NULL, DUMP_CRASH_END);
#endif
    }

#ifdef USER_DATA_TO_FLASH
    dump_info.dump_flag = CRASH_MAGIC;
    user_data_write(&dump_info, sizeof(dump_info));
#endif
    return CRASH_CATCHER_EXIT;
}
