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

#if defined(UTILS_ESHELL_EN) && defined(__SYS_AS_MAIN__)

#include <stdio.h>
#include <stdarg.h>
#include "plat_types.h"
#include "string.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "crc32_c.h"
#include "eshell.h"
#include "app_eshell.h"

/* XMODEM protocol bytes */
#define SOH     0x01
#define STX     0x02
#define EOT     0x04
#define ACK     0x06
#define NAK     0x15
#define CAN     0x18
#define CTRLZ   0x1A

/* return values */
#define X_OK        0
#define X_EOT       -1
#define X_FAILED    -2

/* packet receive retry max times */
#define X_RETRIES   10

/* packet header wait timeout in us, 3s */
#define X_HDR_TIMEOUT   3000000

/* packet data wait timeout in us */
#define X_PKT_TIMEOUT   40000


#define X_BUFF_SIZE     (1024+32)
static char xbuff[X_BUFF_SIZE];

static void xmodem_putc(char c)
{
    eshell_putchar(c);
}

static char xmodem_getc(void)
{
    return eshell_getchar();
}

static int xmodem_tstc(void)
{
    return eshell_tstc();
}

static void xmodem_clear_rx(void)
{
    /* ignore all rx bytes */
    while (xmodem_tstc())
        xmodem_getc();

    hal_sys_timer_delay_us(10000);

    /* again */
    while (xmodem_tstc())
        xmodem_getc();
}

static int xmodem_recv_packet(uint32_t *pktlen)
{
    int i = 0;
    int sz = 0;
    int retry = 0;
    int timeout = X_HDR_TIMEOUT;
    char csum_new = 0;
    char csum_expect = 0;

    if (pktlen)
        *pktlen = 0;

    while (retry < X_RETRIES) {

        /* request packet again */
        if (retry > 0) {
            xmodem_clear_rx();
            xmodem_putc(NAK);
        }
        retry++;

        timeout = X_HDR_TIMEOUT;
        while (timeout > 0 && xmodem_tstc() == 0) {
            hal_sys_timer_delay_us(10);
            timeout -= 10;
        }
        if (xmodem_tstc() == 0)
            continue;

        /* check EOT to stop data receive */
        if (xmodem_tstc() == 1){
            char c = xmodem_getc();
            if (c == EOT) {
                xmodem_putc(ACK);
                return X_EOT;
            }
        }

        /* wait all packet data ready */
        timeout = X_PKT_TIMEOUT;
        while (timeout > 0) {
            if (xmodem_tstc() == 132)
                break;
            if (xmodem_tstc() == 1028)
                break;
            hal_sys_timer_delay_us(10);
            timeout -= 10;
        }
        if (timeout <= 0)
            continue;

        /* read all data */
        i = 0;
        while (xmodem_tstc() > 0)
            xbuff[i++] = xmodem_getc();

        /* check header */
        if (xbuff[0] != SOH && xbuff[0] != STX)
            continue;

        if (xbuff[1] != ~xbuff[2])
            continue;

        sz = i;
        csum_expect = xbuff[sz-1];

        /* checksum */
        csum_new = 0;
        for (i = 3; i < sz-1; i++)
            csum_new += xbuff[i];
        if (csum_new != csum_expect)
            continue;

        /* clear CTRLZ */
        if (xbuff[sz-2] == CTRLZ && xbuff[sz-3] == CTRLZ) {
            while (sz > 5 && xbuff[sz-2] == CTRLZ)
                sz--;
        }

        sz -= 4;
        if (pktlen)
            *pktlen = sz;

        //eshell_putstring("receive %d bytes, pktnum = %02x, csum = 0x%02x\r\n",
        //                sz, (uint32_t)xbuff[1] & 0xff, (uint32_t)csum_new & 0xff);

        xmodem_putc(ACK);
        return X_OK;
    }

    return X_FAILED;
}

static int xmodem_start_transfer(void)
{
    int i = 0;
    int retries = X_RETRIES;
    int timeout = X_HDR_TIMEOUT;
    bool connect = false;

    while (retries-- > 0) {
        xmodem_clear_rx();

        /* request first packet */
        xmodem_putc(NAK);

        timeout = X_HDR_TIMEOUT;
        while (timeout > 0 && xmodem_tstc() == 0) {
            hal_sys_timer_delay_us(10);
            timeout -= 10;
        }
        if (xmodem_tstc() == 0)
            continue;

        hal_sys_timer_delay_us(X_PKT_TIMEOUT);

        memset(xbuff, 0, sizeof(xbuff));

        i = 0;
        while (xmodem_tstc() > 0) {
            xbuff[i++] = xmodem_getc();
            if (i >= X_BUFF_SIZE)
                break;
        }

        if (xbuff[0] == 'r' && xbuff[1] == 'x') {
            for (int j = 0; j < i; j++) {
                if (xbuff[j] == 0xd || xbuff[j] == 0xa){
                    xbuff[j] = '\0';
                }
            }
            connect = true;
            break;

        } else if (xbuff[0] == SOH || xbuff[0] == STX) {
            xbuff[0] = '\0';
            connect = true;
            break;
        }
    }

    if (connect) {
        /* re-request first packet to start data transfer */
        xmodem_clear_rx();
        xmodem_putc(NAK);
        return X_OK;
    } else {
        return X_FAILED;
    }
}

static void xmodem_stop_transfer(void)
{
    xmodem_putc(ACK);
    hal_sys_timer_delay_us(10000);
    xmodem_putc(ACK);
    xmodem_clear_rx();
}

static void xmodem_recv_stream(uint32_t addr)
{
    int ret;
    uint32_t total = 0;
    uint32_t pktlen = 0;
    uint32_t pktnum = 0;
    char *p = (char *)addr;
    uint32_t crc = 0;

    eshell_putstring("XMODEM waiting for transfer (0x%08x):\r\n", addr);

    ret = xmodem_start_transfer();
    if (ret == X_FAILED) {
        eshell_putstring("XMODEM is not connected!\r\n");
        return;
    }

    eshell_putstring("XMODEM transfer start..\r\n");
    if (strlen(xbuff) > 0)
        eshell_putstring("XMODEM transfer info: [%s]\r\n", xbuff);

    while (1) {
        ret = xmodem_recv_packet(&pktlen);
        if (pktlen > 0) {
            memcpy(p, &xbuff[3], pktlen);
            crc = crc32_c(crc, (const unsigned char *)p, pktlen);
            p += pktlen;
            total += pktlen;
            pktnum++;
            eshell_putstring(".");
            if (pktnum % 64 == 0)
                eshell_putstring("\r\n");
        }
        if (ret == X_OK) {
            continue;

        } else if (ret == X_EOT) {
            eshell_putstring("\r\nXMODEM transfer is done.\r\n");
            break;

        } else if (ret == X_FAILED) {
            eshell_putstring("\r\nXMODEM transfer is failed!\r\n");
            return;
        }
    }

    xmodem_stop_transfer();

    eshell_putstring("\r\nXMODEM received %u (0x%x) bytes, CRC = 0x%08x.\r\n",
                total, total, crc);

}

static void do_xmodem_xfer(int argc, char *argv[])
{
    uint32_t addr;

    if (argc != 2) {
        eshell_putstring("\r\nUsage: xmodem addr\r\n");
        return;
    }

    addr = strtol(argv[1], NULL, 16);

    xmodem_recv_stream(addr);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "xmodem", "receive data by xmodem", do_xmodem_xfer);

#endif //UTILS_ESHELL_EN
