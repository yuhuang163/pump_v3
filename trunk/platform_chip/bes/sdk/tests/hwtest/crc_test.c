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
#include "plat_addr_map.h"

#if defined(CRC_BASE)
#include "hal_uart.h"
#include "hal_trace.h"
#include "hwtest.h"
#include "hal_crc.h"

#define BIT(x)  (1u << (x))

static unsigned int table[256];

static unsigned int reflected(unsigned int input, unsigned char bits)
{
    unsigned int var = 0;
    while (bits--) {
        var <<= 1;
        if (input & 0x01)
            var |= 1;
        input >>= 1;
    }
    return var;
}

static void crc_table_init(const struct HAL_CRC_CFG_T* info)
{
    unsigned short i;
    unsigned char j;
    unsigned int poly, value;
    unsigned int width;
    width = info->type_crc32 == 1 ? 32 : 16;
    unsigned int valid_bits = (2 << (width - 1)) - 1;

    if (info->refin) {
        poly = reflected(info->poly, width);

        for (i = 0; i < 256; i++) {
            value = i;
            for (j = 0; j < 8; j++) {
                if (value & 1)
                    value = (value >> 1) ^ poly;
                else
                    value = (value >> 1);
            }
            table[i] = value & valid_bits;
        }
    } else {
        poly = width < 8 ? info->poly << (8 - width) : info->poly;
        unsigned int bit = width > 8 ? BIT(width - 1) : 0x80;

        for (i = 0; i < 256; i++) {
            value = width > 8 ? i << (width - 8) : i;

            for (j = 0; j < 8; j++) {
                if (value & bit)
                    value = (value << 1) ^ poly;
                else
                    value = (value << 1);
            }
            table[i] = value & (width < 8 ? 0xff : valid_bits);
        }
    }
}

static unsigned int crc_calculate(const struct HAL_CRC_CFG_T* info, const unsigned char* ptr, unsigned int len)
{
    unsigned int value;
    unsigned char high;
    unsigned int xorout = 0;
    unsigned int width;

    width = info->type_crc32 == 1 ? 32 : 16;
    if (info->xorout_flag) {
        xorout = width == 16 ? 0xffff : 0xffffffff;
    }

    if (info->refin) {
        value = reflected(info->init, width);

        if (width > 8) {
            while (len--) {
                value = (value >> 8) ^ table[(value & 0xff) ^ *ptr++];
            }
        }
        else
            while (len--) {
                value = table[value ^ *ptr++];
            }
    } else {
        if (width > 8) {
            value = info->init;

            while (len--) {
                high = value >> (width - 8);
                value = (value << 8) ^ table[high ^ *ptr++];
            }
        } else {
            value = info->init << (8 - width);
            while (len--) {
                value = table[value ^ *ptr++];
            }
            value >>= 8 - width;
        }
    }

    if (info->refout != info->refin)
        value = reflected(value, width);

    value ^= xorout;

    return value & ((2 << (width - 1)) - 1);
}

static const uint32_t crc_res[] = {0xc6ba, 0x3945, 0x2245, 0xddba, 0x4f81, 0xd71c, 0xb80e, 0xd90c, 0x49f5, 0x81f67724, 0x59e6d3a5};

static const struct HAL_CRC_CFG_T hal_crc_struct[] = {
    {0, 0x8005, 0x0000, true, true, 0},
    {0, 0x8005, 0x0000, true, true, 1},
    {0, 0x8005, 0xffff, true, true, 1},
    {0, 0x8005, 0xffff, true, true, 0},
    {0, 0x1021, 0x0000, true, true, 0},
    {0, 0x1021, 0xffff, false, false, 0},
    {0, 0x1021, 0xffff, true, true, 1},
    {0, 0x1021, 0x0000, false, false, 0},
    {0, 0x3d65, 0x0000, true, true, 1},
    {1, 0x04C11DB7, 0xffffffff, true, true, 1},
    {1, 0x04C11DB7, 0xffffffff, false, false, 0},
};

static uint8_t test_data[] = {0x01,0x02,0x03,0x04,0x05,0x06};

static void crc_cb(const uint32_t res, uint32_t error)
{
    static uint16_t cnt = 0;
    if (error)
        ASSERT(false,"CRC irq error: error_state=%#x\n", res);
    ASSERT(res == crc_res[cnt++],"CRC RESULT error,index %d\n",cnt-1);
}

void crc_test(void)
{
    TRACE(0,"crc test begin\n");
    int len = sizeof(test_data);
    hal_crc_open();

    for (int i = 0; i < ARRAY_SIZE(hal_crc_struct); i++) {
        crc_table_init(&hal_crc_struct[i]);
        ASSERT(crc_calculate(&hal_crc_struct[i], (unsigned char*)test_data, len) == crc_res[i],"soft crc error,index %d\n",i);
        /*blocked mode*/
        ASSERT(hal_crc_start(&hal_crc_struct[i], test_data, len, NULL) == crc_res[i],"blocked hw crc error,index %d\n",i);
        /*unblocked mode*/
        hal_crc_start(&hal_crc_struct[i], test_data, len, crc_cb);
    }

    ASSERT(hal_crc16_ibm(test_data, len) == crc_res[0],"hal_crc16_ibm error");
    ASSERT(hal_crc16_maxim(test_data, len) == crc_res[1],"hal_crc16_maxim error");
    ASSERT(hal_crc16_usb(test_data, len) == crc_res[2],"hal_crc16_usb error");
    ASSERT(hal_crc16_modbus(test_data, len) == crc_res[3],"hal_crc16_modbus error");
    ASSERT(hal_crc16_ccitt(test_data, len) == crc_res[4],"hal_crc16_ccitt error");
    ASSERT(hal_crc16_ccitt_false(test_data, len) == crc_res[5],"hal_crc16_ccitt_false error");
    ASSERT(hal_crc16_x25(test_data, len) == crc_res[6],"hal_crc16_x25 error");
    ASSERT(hal_crc16_xmodem(test_data, len) == crc_res[7],"hal_crc16_xmodem error");
    ASSERT(hal_crc16_dnp(test_data, len) == crc_res[8],"hal_crc16_dnp error");
    ASSERT(hal_crc32(test_data, len) == crc_res[9],"hal_crc32 error");
    ASSERT(hal_crc32_mpeg(test_data, len) == crc_res[10],"hal_crc32_mpeg error");

    TRACE(0,"crc test success!!!!\n");
}

#endif
