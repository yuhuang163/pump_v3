/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifdef CHIP_FUNCTION_TEST_SAM
#include <string.h>
#include <stdlib.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_uart.h"
#include "hal_sysfreq.h"

#ifdef RTOS
#include "cmsis_os.h"
#ifdef KERNEL_RTX
#include "rt_Time.h"
#endif
#endif

// CMD HEADER 2B, CMD LEN 2B DATA nB CRC 1B
#define SAM_DATA_LEN                            256
#define SAM_CMD_HEADER                          0xAA
#define SAM_CMD_HEADER1                         0x66
#define SAM_CMD_RESP_HEADER1_CRYSTAL            0x55
#define SAM_CMD_RESP_HEADER1_RC                 0x66

// CMD 37  SAM RESET
#define SAM_CMD_SAM_RESET                       0x37
#define SAM_CMD_SAM_RESET_LEN                   4
#define SAM_CMD_RESET_BAUD_RATE_SHIFT           0
#define SAM_CMD_RESET_RESET_SHIFT               2
#define SAM_CMD_RESET_RESET                     0x00
#define SAM_CMD_RESET_SWITCH_38400             0x01
#define SAM_CMD_RESET_SELECT_SAM_SHIFT          4
#define SAM_CMD_SAM_IND_1                       0x00
#define SAM_CMD_SAM_IND_2                       0x01
#define SAM_CMD_SAM_IND_3                       0x02
#define SAM_CMD_SAM_IND_4                       0x03
#define SAM_CMD_SAM_IND_5                       0x04
#define SAM_CMD_SAM_IND_6                       0x05
#define SAM_CMD_SAM_IND_7                       0x06
#define SAM_CMD_RESET_RESP_LEN_IS_SUCC(n)       (n > 2)
#define SAM_CMD_RESET_RESP_SUCC                 0x37
#define SAM_CMD_RESET_RESP_FAIL                 0xC8
#define SAM_CMD_RESET_TIMEOUT_MS                20

// CMD 38  COS
#define SAM_CMD_COS                              0x38
#define SAM_CMD_COS_LEN(n)                       (3 + n)
#define SAM_CMD_COS_SAM_IND_OFFSET               0
#define SAM_CMD_COS_CMD_OFFSET                   1
#define SAM_CMD_COS_LEN_IS_SUCC(n)               (n > 2)
#define SAM_CMD_COS_RESP_SUCC                    0x38
#define SAM_CMD_COS_RESP_FAIL                    0xC7
#define SAM_CMD_COS_TIMEOUT_MS                   10

// CMD 36  FREQUENCY
#define SAM_CMD_FREQ                             0x36
#define SAM_CMD_FREQ_LEN(n)                      (3 + n)
#define SAM_CMD_FREQ_OFFSET                      0
#define SAM_CMD_FREQ_MHZ_1                       1
#define SAM_CMD_FREQ_MHZ_2                       2
#define SAM_CMD_FREQ_MHZ_3                       3
#define SAM_CMD_FREQ_MHZ_4                       4
#define SAM_CMD_FREQ_MHZ_6                       6
#define SAM_CMD_FREQ_MHZ_12                      12
#define SAM_CMD_FREQ_RESP_LEN_IS_SUCC(n)         (n == 2)
#define SAM_CMD_FREQ_RESP_SUCC                   0x36
#define SAM_CMD_FREQ_RESP_FAIL                   0xC9
#define SAM_CMD_FREQ_TIMEOUT_MS                  10

// CMD 36  BAUD RATE
#define SAM_CMD_BAUD_RATE                         0x15
#define SAM_CMD_BAUD_RATE_LEN(n)                  (3 + n)
#define SAM_CMD_BAUD_RATE_OFFSET                  0
#define SAM_CMD_BAUD_RATE_4800                    0x0
#define SAM_CMD_BAUD_RATE_9600                    0x1
#define SAM_CMD_BAUD_RATE_14400                   0x2
#define SAM_CMD_BAUD_RATE_19200                   0x3
#define SAM_CMD_BAUD_RATE_28800                   0x4
#define SAM_CMD_BAUD_RATE_38400                   0x5
#define SAM_CMD_BAUD_RATE_57600                   0x6
#define SAM_CMD_BAUD_RATE_115200                  0x7
#define SAM_CMD_BAUD_RATE_375000                  0x8
#define SAM_CMD_BAUD_RATE_500000                  0x9

#define SAM_CMD_BAUD_RATE_RESP_LEN_IS_SUCC(n)     (n == 2)
#define SAM_CMD_BAUD_RATE_RESP_SUCC               0x15
#define SAM_CMD_BAUD_RATE_RESP_FAIL               0xEA
#define SAM_CMD_BAUD_RATE_TIMEOUT_MS              10

#define BYTE_EXCHANGE(val)   (0xFFFF & (((val & 0xFFFF) >> 8) | ((val & 0xFFFF) << 8)))

enum SAM_IND_T {
    SAM_IND_1 = SAM_CMD_SAM_IND_1,
    SAM_IND_2 = SAM_CMD_SAM_IND_2,
    SAM_IND_3 = SAM_CMD_SAM_IND_3,
    SAM_IND_4 = SAM_CMD_SAM_IND_4,
    SAM_IND_5 = SAM_CMD_SAM_IND_5,
    SAM_IND_6 = SAM_CMD_SAM_IND_6,
    SAM_IND_7 = SAM_CMD_SAM_IND_7,
    SAM_IND_QTY,
};

enum SAM_BAUD_RATE_T {
  SAM_BAUD_RATE_4800   = SAM_CMD_BAUD_RATE_4800,
  SAM_BAUD_RATE_9600   = SAM_CMD_BAUD_RATE_9600,
  SAM_BAUD_RATE_14400  = SAM_CMD_BAUD_RATE_14400,
  SAM_BAUD_RATE_19200  = SAM_CMD_BAUD_RATE_19200,
  SAM_BAUD_RATE_28800  = SAM_CMD_BAUD_RATE_28800,
  SAM_BAUD_RATE_38400  = SAM_CMD_BAUD_RATE_38400,
  SAM_BAUD_RATE_57600  = SAM_CMD_BAUD_RATE_57600,
  SAM_BAUD_RATE_115200 = SAM_CMD_BAUD_RATE_115200,
  SAM_BAUD_RATE_375000 = SAM_CMD_BAUD_RATE_375000,
  SAM_BAUD_RATE_500000 = SAM_CMD_BAUD_RATE_500000,
  SAM_BAUD_RATE_QTY,
};

enum SAM_RESET_TYPE_T {
    SAM_RESET_TYPE_ONLY = 0x00,
    SAM_RESET_TYPE_SWITCH_38400 = 0x02,
    SAM_RESET_TYPE_QTY,
};

enum SAM_FREQ_MHZ_T {
  SAM_FREQ_MHZ_1  =  0x1,
  SAM_FREQ_MHZ_2  =  0x2,
  SAM_FREQ_MHZ_3  =  0x3,
  SAM_FREQ_MHZ_4  =  0x4,
  SAM_FREQ_MHZ_6  =  0x6,
  SAM_FREQ_MHZ_12 =  0x12,
  SAM_FREQ_MHZ_QTY = 0xFF,
};


typedef struct {
    uint8_t header[2];
    uint16_t size;
    uint8_t cmd;
    uint8_t data[SAM_DATA_LEN];
} SAM_CMD_RESP_T;

typedef struct {
    uint8_t header[2];
    uint16_t size;
    uint8_t cmd;
    uint8_t data[1];
    uint8_t crc;
} SAM_CMD_RESET_T;


#ifndef UART_PORT_ID
#define  UART_PORT_ID    HAL_UART_ID_1
#endif

#ifndef SAM_UART_BANDRATE
#define SAM_UART_BANDRATE 4800
#endif

static enum HAL_UART_ID_T test_uart_id = UART_PORT_ID;
//static SAM_CMD_T _sam_cmd;
static SAM_CMD_RESP_T _sam_cmd_resp;
static const struct HAL_UART_CFG_T sam_uart_cfg = {
    HAL_UART_PARITY_NONE,
    HAL_UART_STOP_BITS_1,
    HAL_UART_DATA_BITS_8,
    HAL_UART_FLOW_CONTROL_NONE, // RTC/CTS pins might be unavailable for some chip packages
    HAL_UART_FIFO_LEVEL_1_2,
    HAL_UART_FIFO_LEVEL_1_2,
    SAM_UART_BANDRATE,
#ifdef SAM_UART_ENABLE_DMA
    true,
    true,
#else
    false,
    false,
#endif
    false,
};

#ifdef SAM_UART_ENABLE_DMA
#define UART_DMA_RX_DONE()   {while(uart_rx_done == 0);}
#define UART_DMA_TX_DONE()   {while(uart_tx_done == 0);}

static uint8_t uart_rx_done = 0;
static uint8_t uart_tx_done = 0;
static uint32_t uart_rx_len = 0;

static void _uart_rx(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status)
{
    if (dma_error) {
        TRACE(1, "uart rx dma error: xfer_size=%d", xfer_size);
        uart_rx_len = 0;
    } else if (status.BE || status.FE || status.OE || status.PE) {
        TRACE(2, "uart rx uart error: xfer_size=%d, status=0x%08x", xfer_size, status.reg);
        uart_rx_len = 0;
    } else {
        uart_rx_len = xfer_size;
    }
    uart_rx_done = 1;
}

static void _uart_tx(uint32_t xfer_size, int dma_error)
{
    if (dma_error) {
        TRACE(1, "uart tx dma error: xfer_size=%d", xfer_size);
    }
    uart_tx_done = 1;
}
#endif

static int32_t sam_uart_write(enum HAL_UART_ID_T uart_id, uint8_t *buf, uint32_t len, uint8_t enable_dma)
{
    int32_t ret = 0;
    uint32_t sent = 0;

    if (enable_dma) {
#ifdef SAM_UART_ENABLE_DMA
        uart_tx_done = 0;
#endif
        ret = hal_uart_dma_send(uart_id, buf, len, NULL, NULL);
    } else {
        while (sent < len) {
            hal_uart_blocked_putc((enum HAL_UART_ID_T)uart_id, buf[sent++]);
        }
        if (sent != len) {
            ret = -2021;
        }
        ret = 0;
    }

    return ret;
}

static int32_t sam_uart_read(enum HAL_UART_ID_T uart_id, uint8_t *buf, uint32_t len, uint8_t enable_dma)
{
    int32_t ret = 0;
    uint32_t recv = 0;

    if (enable_dma) {
#ifdef SAM_UART_ENABLE_DMA
        uart_rx_done = 0;
#endif
        //TRACE(1, "dma recv = %d, len = %d", recv, len); TRACE_FLUSH();
        ret = hal_uart_dma_recv(uart_id, buf, len, NULL, NULL);
    } else {
        while (recv < len) {
            buf[recv++] = hal_uart_blocked_getc((enum HAL_UART_ID_T)uart_id);
        }
        if (recv != len) {
            ret = -2021;
        }
        ret = 0;
    }

    return ret;
}

static void _init_uart(enum HAL_UART_ID_T uart_id)
{
    if(uart_id == HAL_UART_ID_1)
    {
        TRACE(1, "INIT UART1");
        TRACE_FLUSH();
        hal_iomux_set_uart1();
    }
    else
    {
        TRACE(1, "INIT UART0");
        hal_iomux_set_uart0();
    }

    hal_uart_open((enum HAL_UART_ID_T)uart_id, &sam_uart_cfg);
    //TRACE(1, "UART %d OPEN DONE", uart_id);
#ifdef SAM_UART_ENABLE_DMA
    hal_uart_irq_set_dma_handler((enum HAL_UART_ID_T)uart_id, _uart_rx, _uart_tx);
    //TRACE(1, "UART %d SET DMA HANDLE DONE", uart_id);
#endif
    TRACE(1, "UART %d INIT DONE", uart_id);
}

static uint8_t _sam_crc8(uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint8_t crc = 0;

    for(i = 0;  i < len; i++)
    {
        crc += data[i];
    }

    return crc;
}

static bool _sam_check_resp(SAM_CMD_RESP_T *resp_cmd, uint8_t cmd)
{
    uint8_t crc;
    uint8_t* p;
    uint32_t len;

    if(resp_cmd->cmd != cmd)
    {
        return false;
    }

    len = BYTE_EXCHANGE(resp_cmd->size);
    p = (uint8_t*)resp_cmd + 2;

    crc = _sam_crc8(p,len);
    TRACE(1, "resp_cmd->crc: 0x%x, crc: 0x%x", p[len], crc);
    if(p[len] != crc)
    {
        return false;
    }
    return true;
}

static int _creat_cmd_reset(enum SAM_IND_T sam_id,
                                   enum SAM_BAUD_RATE_T baud_rate,
                                   enum SAM_RESET_TYPE_T reset_type,
                                   SAM_CMD_RESET_T *cmd_str)
{
    uint8_t data = 0;

    if(sam_id >= SAM_IND_QTY)
    {
        return 1;
    }
    else
    {
        data |= (sam_id << SAM_CMD_RESET_SELECT_SAM_SHIFT);
    }

    if(baud_rate >= SAM_BAUD_RATE_QTY)
    {
        return 2;
    }
    else
    {
        data |= (sam_id << SAM_CMD_RESET_BAUD_RATE_SHIFT);
    }

    if(reset_type >= SAM_RESET_TYPE_QTY)
    {
        return 3;
    }
    else
    {
        data |= (reset_type << SAM_CMD_RESET_RESET_SHIFT);
    }

    cmd_str->header[0] = SAM_CMD_HEADER;
    cmd_str->header[1] = SAM_CMD_HEADER1;
    cmd_str->size = BYTE_EXCHANGE(SAM_CMD_SAM_RESET_LEN);
    cmd_str->cmd = SAM_CMD_SAM_RESET;
    cmd_str->data[0] = data;
    cmd_str->crc = _sam_crc8((uint8_t*)&cmd_str->size, BYTE_EXCHANGE(cmd_str->size));
    return 0;
}


int sam_reset_cmd(enum SAM_IND_T sam_id, enum SAM_BAUD_RATE_T baud_rate, enum SAM_RESET_TYPE_T reset_type)
{
    int ret;
    uint32_t result;
    SAM_CMD_RESET_T sam_cmd;
    uint8_t *p;

    memset(&sam_cmd, 0, sizeof(sam_cmd));
    ret = _creat_cmd_reset(sam_id, baud_rate, reset_type, &sam_cmd);
    if(ret)
    {
        TRACE(1, "%s _creat_cmd_reset fail, r = %d", __func__, ret);
        return ret;
    }
    TRACE(1, "SAM RESET SEDN:");
    DUMP8("0x%x,", (uint8_t*)&sam_cmd, 7);

    result = sam_uart_write(test_uart_id, (uint8_t*)&sam_cmd, 7, sam_uart_cfg.dma_tx);
    if(result)
    {
        TRACE(1, "%s: sam_uart_write fail, result = %d", __func__, result);
        return -5;
    }

#ifdef SAM_UART_ENABLE_DMA
    UART_DMA_TX_DONE();
#endif
    result = sam_uart_read(test_uart_id, (uint8_t*)&_sam_cmd_resp, 5, sam_uart_cfg.dma_rx);
    if(result)
    {
        TRACE(1, "%s: sam_uart_read fail, result = %d", __func__, result);
        return -5;
    }
#ifdef SAM_UART_ENABLE_DMA
    UART_DMA_RX_DONE();
#endif

    if(!(_sam_cmd_resp.header[0] == SAM_CMD_HEADER &&
       (_sam_cmd_resp.header[1] == SAM_CMD_RESP_HEADER1_CRYSTAL ||
        _sam_cmd_resp.header[1] == SAM_CMD_RESP_HEADER1_RC) &&
       SAM_CMD_RESET_RESP_LEN_IS_SUCC(BYTE_EXCHANGE(_sam_cmd_resp.size))))
    {
        DUMP8("0x%x,",(uint8_t*)&_sam_cmd_resp, 4);
        return -6;
    }

    result = sam_uart_read(test_uart_id, (uint8_t*)_sam_cmd_resp.data, BYTE_EXCHANGE(_sam_cmd_resp.size) - 2, sam_uart_cfg.dma_rx);
    if(result)
    {
        TRACE(1, "%s: sam_uart_read fail, result = %d", __func__, result);
        return -7;
    }
    TRACE(1, "SAM RESET RECV:");
    p = (uint8_t*)&_sam_cmd_resp;
    DUMP8("0x%x,", p, 16);
    p += 16;
    DUMP8("0x%x,", p, BYTE_EXCHANGE(_sam_cmd_resp.size) + 3 - 16);

    if(_sam_check_resp(&_sam_cmd_resp, SAM_CMD_SAM_RESET))
    {
        return 0;
    }
    else
    {
        return -8;
    }
}

int sam_init(void)
{
    int ret;

    TRACE(1, "\n");
    TRACE(1, "--------------SAM_INIT-------------");
    ret = sam_reset_cmd(SAM_CMD_SAM_IND_1, SAM_BAUD_RATE_4800, SAM_RESET_TYPE_ONLY);
    if(ret)
    {
        TRACE(1, "SAM TEST RESET NG");
    }
    else
    {
        TRACE(1, "SAM TEST RESET PASS");
    }
    TRACE(1, "---------------------------------");
   // TRACE(1, "SAM_INIT DONE");
    TRACE(1, "\n");
    TRACE_FLUSH();
    return ret;
}

void _sam_test(void)
{
    int ret;

    TRACE(1, "SAM TEST: BEGIN.");
    _init_uart(test_uart_id);
    ret = sam_init();
    if(ret)
    {
        goto _TEST_NG;
    }

    TRACE(1, "SAM TEST: PASS.");
    return;
_TEST_NG:
    TRACE(1, "SAM TEST: NG.");
    return;
}


void function_test_sam(void)
{
    _sam_test();
    osDelay(10);
    while(1);
}

#endif


