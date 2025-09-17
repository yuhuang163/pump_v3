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
#include "plat_addr_map.h"

#ifdef SDIO_DEVICE_BASE

#include "reg_sdio_device.h"
#include "hal_sdio_device.h"
#include "cmsis_nvic.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_chipid.h"
#include <string.h>

#ifdef SDIO_DEVICE_DEBUG
#ifdef FPGA
    #define HAL_SDIO_DEVICE_TRACE(n, s, ...)    TRACE(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
#else
    #define HAL_SDIO_DEVICE_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#endif
#else
    #define HAL_SDIO_DEVICE_TRACE(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

static struct SDIO_DEVICE_T *const sdio_device[HAL_SDIO_ID_NUM] = {
    (struct SDIO_DEVICE_T *)SDIO_DEVICE_BASE,
#ifdef SDIO1_DEVICE_BASE
    (struct SDIO_DEVICE_T *)SDIO1_DEVICE_BASE,
#endif
};

static IRQn_Type const sdio_irq[HAL_SDIO_ID_NUM] = {
    (IRQn_Type)SDIO_DEVICE_IRQn,
#ifdef SDIO1_DEVICE_BASE
    (IRQn_Type)SDIO1_DEVICE_IRQn,
#endif
};

enum SDIO_DEVICE_ADMA_ATT_T {
    SDIO_DEVICE_ADMA_ATT_VALID  = 0x01,
    SDIO_DEVICE_ADMA_ATT_END    = 0x02,
    SDIO_DEVICE_ADMA_ATT_INT    = 0x04,
    SDIO_DEVICE_ADMA_ATT_NOP    = 0x00,
    SDIO_DEVICE_ADMA_ATT_RSV    = 0x10,
    SDIO_DEVICE_ADMA_ATT_TRAN   = 0x20,
    SDIO_DEVICE_ADMA_ATT_LINK   = 0x30,
};

struct SDIO_DEVICE_RXTX_CTRL_T {
    uint8_t *rxaddr;//device perspective, device is rx, host is tx
    uint16_t rx_real_len;
    uint32_t rxlen;
    uint8_t *txaddr;//device perspective, device is tx, host is rx
    uint16_t tx_real_len;
    uint16_t txlen;
    uint8_t status;
    uint8_t error;
    uint8_t rxbuf_cnt;
    struct HAL_SDIO_DEVICE_CB_T *sdio_callback;
#ifdef SDIO_DEVICE_AGGR
    uint32_t tx_aggr_cnt;
    uint32_t rx_aggr_cnt;
#endif
};

static struct SDIO_DEVICE_RXTX_CTRL_T sdio_rxtx_ctrl[HAL_SDIO_ID_NUM];

#define DMA_BOUNDARY_SIZE   4096
#define ADMA_BOUNDARY_SIZE  1024

#ifndef SDIO_DEVICE_ENABLE_DMA

struct SDIO_ADMA_DESC_T {
    uint32_t adma_desc_ctrl;
    uint32_t adma_desc_addr;
};

struct SDIO_DEVICE_DESC_T {
    struct SDIO_ADMA_DESC_T adma_desc[SDIO_DEVICE_ADMA_DESC_NUM];
    uint32_t cnt;
};

struct SDIO_DEVICE_DESC_T tx_desc[HAL_SDIO_ID_NUM];
struct SDIO_DEVICE_DESC_T rx_desc[HAL_SDIO_ID_NUM];

static void hal_sdio_device_adma_desc_init(enum HAL_SDIO_ID_T id, struct SDIO_DEVICE_DESC_T *desc)
{
    desc[id].cnt = 0;
    // memset(&desc[id].adma_desc, 0, sizeof(desc[id].adma_desc));
}

static void hal_sdio_device_adma_desc_add(enum HAL_SDIO_ID_T id, struct SDIO_DEVICE_DESC_T *desc, uint32_t addr, uint32_t len)
{
    HAL_SDIO_DEVICE_TRACE(0, "hal_sdio_device_adma_desc_add addr:0x%x, len:0x%x", addr, len);
#if !defined(CHIP_BEST2002)
    ASSERT(desc[id].cnt < SDIO_DEVICE_ADMA_DESC_NUM, "adma %d overflow", desc[id].cnt);
    desc[id].adma_desc[desc[id].cnt].adma_desc_ctrl = len << 16;
    desc[id].adma_desc[desc[id].cnt].adma_desc_ctrl |= SDIO_DEVICE_ADMA_ATT_TRAN | SDIO_DEVICE_ADMA_ATT_VALID;
    desc[id].adma_desc[desc[id].cnt].adma_desc_addr = addr;
    desc[id].cnt++;
#else
    uint32_t desc_len;

    if (addr % ADMA_BOUNDARY_SIZE) {
        desc_len = ADMA_BOUNDARY_SIZE - (addr % ADMA_BOUNDARY_SIZE);
        desc_len = MIN(desc_len, len);
        ASSERT(desc[id].cnt < SDIO_DEVICE_ADMA_DESC_NUM, "adma %d overflow", desc[id].cnt);
        desc[id].adma_desc[desc[id].cnt].adma_desc_ctrl = desc_len << 16;
        desc[id].adma_desc[desc[id].cnt].adma_desc_ctrl |= SDIO_DEVICE_ADMA_ATT_TRAN | SDIO_DEVICE_ADMA_ATT_VALID;
        desc[id].adma_desc[desc[id].cnt].adma_desc_addr = addr;
        desc[id].cnt++;
        addr += desc_len;
        len -= desc_len;
    }

    while (len > ADMA_BOUNDARY_SIZE) {
        desc_len = ADMA_BOUNDARY_SIZE;
        ASSERT(desc[id].cnt < SDIO_DEVICE_ADMA_DESC_NUM, "adma %d overflow", desc[id].cnt);
        desc[id].adma_desc[desc[id].cnt].adma_desc_ctrl = desc_len << 16;
        desc[id].adma_desc[desc[id].cnt].adma_desc_ctrl |= SDIO_DEVICE_ADMA_ATT_TRAN | SDIO_DEVICE_ADMA_ATT_VALID;
        desc[id].adma_desc[desc[id].cnt].adma_desc_addr = addr;
        desc[id].cnt++;
        addr += desc_len;
        len -= desc_len;
    }

    if (len) {
        desc_len = len;
        ASSERT(desc[id].cnt < SDIO_DEVICE_ADMA_DESC_NUM, "adma %d overflow", desc[id].cnt);
        desc[id].adma_desc[desc[id].cnt].adma_desc_ctrl = desc_len << 16;
        desc[id].adma_desc[desc[id].cnt].adma_desc_ctrl |= SDIO_DEVICE_ADMA_ATT_TRAN | SDIO_DEVICE_ADMA_ATT_VALID;
        desc[id].adma_desc[desc[id].cnt].adma_desc_addr = addr;
        desc[id].cnt++;
    }
#endif
}

static void hal_sdio_device_adma_desc_end(enum HAL_SDIO_ID_T id, struct SDIO_DEVICE_DESC_T *desc)
{
    uint32_t i;
    desc[id].adma_desc[desc[id].cnt].adma_desc_ctrl = SDIO_DEVICE_ADMA_ATT_NOP | SDIO_DEVICE_ADMA_ATT_VALID | SDIO_DEVICE_ADMA_ATT_END;
    desc[id].adma_desc[desc[id].cnt].adma_desc_addr = 0;
    desc[id].cnt++;

    HAL_SDIO_DEVICE_TRACE(0, "desc[%d]:%p", id, &desc[id]);
    for (i = 0; i < desc[id].cnt; ++i) {
        HAL_SDIO_DEVICE_TRACE(0, "desc[%d]->adma_desc[%u] ctrl:0x%x, addr:0x%x", id, i,
            desc[id].adma_desc[i].adma_desc_ctrl, desc[id].adma_desc[i].adma_desc_addr);
    }
}

#endif

#ifdef SDIO_DEVICE_DEBUG
#define SDIO_STATUS_FORMAT(x)  {"sdio device irq:SDIO_"#x}
struct SDIO_INT_STATUS_NAME_T {
    char *name;
};
const static struct SDIO_INT_STATUS_NAME_T sdio_int_status[] = {
    SDIO_STATUS_FORMAT(TRX_COMPLETE_INT),
    SDIO_STATUS_FORMAT(DMA1_INT),
    SDIO_STATUS_FORMAT(SLEEP_AWAKE_INT),
    SDIO_STATUS_FORMAT(WR_START_INT),
    SDIO_STATUS_FORMAT(RD_START_INT),
    SDIO_STATUS_FORMAT(PASSWORD_SET_INT),
    SDIO_STATUS_FORMAT(PASSWORD_RESET_INT),
    SDIO_STATUS_FORMAT(LOCK_CARD_INT),
    SDIO_STATUS_FORMAT(UNLOCK_CARD_INT),
    SDIO_STATUS_FORMAT(FORCE_ERASE_INT),
    SDIO_STATUS_FORMAT(ERASE_INT),
    SDIO_STATUS_FORMAT(CMD11_INT),
    SDIO_STATUS_FORMAT(CMD0_CMD52_SOFTRST_INT),
    SDIO_STATUS_FORMAT(CMD6_CHECK_DONE_INT),
    SDIO_STATUS_FORMAT(CMD6_SWITCH_DONE_INT),
    SDIO_STATUS_FORMAT(PROGRAM_CSD_INT),
    SDIO_STATUS_FORMAT(ACMD23_INT),
    SDIO_STATUS_FORMAT(CMD20_INT),
    SDIO_STATUS_FORMAT(HOST_INT),
    SDIO_STATUS_FORMAT(CMD4_INT),
    SDIO_STATUS_FORMAT(BOOT_START_INT),
    SDIO_STATUS_FORMAT(FUNC1_RESET_INT),
    SDIO_STATUS_FORMAT(FUNC2_RESET_INT),
    SDIO_STATUS_FORMAT(CMD11_CLK_STOP_INT),
    SDIO_STATUS_FORMAT(CMD11_CLK_START_INT),
    SDIO_STATUS_FORMAT(PROGRAM_START_INT),
    SDIO_STATUS_FORMAT(CMD40_INT),
    SDIO_STATUS_FORMAT(CMDR1B_INT),
    SDIO_STATUS_FORMAT(FUNCX_CRC_END_ERROR_INT),
    SDIO_STATUS_FORMAT(FUNCX_ABORT_INT),
    SDIO_STATUS_FORMAT(LRST_INT),
    SDIO_STATUS_FORMAT(BOOT_COMPLETE_INT),
};
const static struct SDIO_INT_STATUS_NAME_T sdio_int_status2[] = {
    SDIO_STATUS_FORMAT(FUNC3_RESET_INT),
    SDIO_STATUS_FORMAT(FUNC4_RESET_INT),
    SDIO_STATUS_FORMAT(FUNC5_RESET_INT),
    SDIO_STATUS_FORMAT(RESERVED_BIT3),
    SDIO_STATUS_FORMAT(RESERVED_BIT4),
    SDIO_STATUS_FORMAT(PACKED_INT),
    SDIO_STATUS_FORMAT(ADMA_ERROR_INT),
};
#endif

static void hal_sdio_device_read_cccr(enum HAL_SDIO_ID_T id, uint32_t *cccr)
{
    *cccr = sdio_device[id]->SDIO_CCCR_CONTROL;
}

static void hal_sdio_device_write_cccr(enum HAL_SDIO_ID_T id, uint32_t cccr)
{
    sdio_device[id]->SDIO_CCCR_CONTROL = cccr;
}

POSSIBLY_UNUSED static void hal_sdio_device_read_card_addr(enum HAL_SDIO_ID_T id, uint32_t *addr)
{
    //cpu cannot read,2021-05-17
    return;
}

static void hal_sdio_device_write_card_addr(enum HAL_SDIO_ID_T id, uint32_t addr)
{
    sdio_device[id]->CARD_ADDRESS = addr;
}

static void hal_sdio_device_interrupt_status_enable(enum HAL_SDIO_ID_T id)
{
    sdio_device[id]->INTERRUPT_STATUS_ENABLE = 0xFFFFFFFF;
    sdio_device[id]->INTERRUPT_STATUS2_ENABLE = SDIO_FUNC3_RESET_EN    \
                                          | SDIO_FUNC4_RESET_EN    \
                                          | SDIO_FUNC5_RESET_EN    \
                                          | SDIO_PACKED_EN         \
                                          | SDIO_ADMA_ERROR_EN;
}

POSSIBLY_UNUSED static void hal_sdio_device_interrupt_status_disable(enum HAL_SDIO_ID_T id, uint32_t disable_bit)
{
    if (disable_bit) {
        sdio_device[id]->INTERRUPT_STATUS_ENABLE &= ~disable_bit;
    }
}

POSSIBLY_UNUSED static void hal_sdio_device_interrupt_status2_disable(enum HAL_SDIO_ID_T id, uint32_t disable_bit)
{
    if (disable_bit) {
        sdio_device[id]->INTERRUPT_STATUS2_ENABLE &= ~disable_bit;
    }
}

POSSIBLY_UNUSED static void hal_sdio_device_interrupt_signal_enable(enum HAL_SDIO_ID_T id)
{
    sdio_device[id]->INTERRUPT_SIGNAL_ENABLE = 0xFFFFFFFF;
    sdio_device[id]->INTERRUPT_SIGNAL2_ENABLE = SDIO_FUNC3_RESET_EN    \
                                          | SDIO_FUNC4_RESET_EN    \
                                          | SDIO_FUNC5_RESET_EN    \
                                          | SDIO_PACKED_EN         \
                                          | SDIO_ADMA_ERROR_EN;
}

POSSIBLY_UNUSED static void hal_sdio_device_interrupt_signal_disable(enum HAL_SDIO_ID_T id)
{
    sdio_device[id]->INTERRUPT_SIGNAL_ENABLE = 0;
    sdio_device[id]->INTERRUPT_SIGNAL2_ENABLE = 0;
}

static void hal_sdio_device_io_ready(enum HAL_SDIO_ID_T id)
{
    sdio_device[id]->IO_READY = SDIO_FUNC1_READY | SDIO_FUNC2_READY;
}

POSSIBLY_UNUSED static void hal_sdio_device_read_ocr(enum HAL_SDIO_ID_T id, uint32_t *ocr)
{
    *ocr = sdio_device[id]->CARD_OCR;
}

static void hal_sdio_device_write_ocr(enum HAL_SDIO_ID_T id)
{
    uint32_t s18a = 0;

    if (HAL_IOMUX_PIN_VOLTAGE_VIO == hal_iomux_get_sdio_voltage()) {
        s18a = SDIO_SWITCH_TO_1V8;
    }

    //card ocr:1.8v, 2.7v ~ 3.3v
    sdio_device[id]->CARD_OCR = SDIO_CARD_OCR(0x1F8000) \
                          | s18a                    \
                          | SDIO_NUM_FUNCTIONS(1);
}

static void hal_sdio_device_set_card_ready(enum HAL_SDIO_ID_T id)
{
    sdio_device[id]->CONTROL |= SDIO_CARD_INIT_DONE;
}

POSSIBLY_UNUSED static void hal_sdio_device_enable_adma(enum HAL_SDIO_ID_T id)
{
    sdio_device[id]->CONTROL2 |= SDIO_ADMA_ENABLE;
}

POSSIBLY_UNUSED static void hal_sdio_device_disable_adma(enum HAL_SDIO_ID_T id)
{
    sdio_device[id]->CONTROL2 &= (~SDIO_ADMA_ENABLE);
}

POSSIBLY_UNUSED static void hal_sdio_device_gen_int_to_host(enum HAL_SDIO_ID_T id, uint16_t signal_value)
{
    sdio_device[id]->FUNCTION1_CONTROL = signal_value;
}

static void hal_sdio_device_clr_host_int(enum HAL_SDIO_ID_T id)
{
    sdio_device[id]->HOST_INT_CLR |= SDIO_HOST_INT_CLR;
}

static void hal_sdio_device_read_tplmanid(enum HAL_SDIO_ID_T id, uint32_t *tplmanid)
{
    *tplmanid = sdio_device[id]->TPLMANID;
}

static void hal_sdio_device_write_tplmanid(enum HAL_SDIO_ID_T id, uint32_t tplmanid)
{
    sdio_device[id]->TPLMANID = tplmanid;
}

void hal_sdio_device_change_tplmanid_manf(enum HAL_SDIO_ID_T id, uint16_t new_manf_id)
{
    uint32_t val = 0;

    val = SET_BITFIELD(sdio_device[id]->TPLMANID, SDIO_TPLMID_MANF, new_manf_id);
    sdio_device[id]->TPLMANID = val;//Write for the first time: generate a falling edge
    sdio_device[id]->TPLMANID = val;//The second real write: a rising edge
}

void hal_sdio_device_change_tplmanid_card(enum HAL_SDIO_ID_T id, uint16_t new_card_id)
{
    uint32_t val = 0;

    val = SET_BITFIELD(sdio_device[id]->TPLMANID, SDIO_TPLMID_CARD, new_card_id);
    sdio_device[id]->TPLMANID = val;//Write for the first time: generate a falling edge
    sdio_device[id]->TPLMANID = val;//The second real write: a rising edge
}

static void hal_sdio_device_rxbuf_cfg(enum HAL_SDIO_ID_T id, uint16_t buf_len, uint8_t buf_cnt)
{
    sdio_device[id]->RX_BUF_CFG = (sdio_device[id]->RX_BUF_CFG & ~(SDIO_RX_BUF_CNT_MASK | SDIO_RX_BUF_LEN_MASK)) |
                              SDIO_RX_BUF_CNT(buf_cnt) | SDIO_RX_BUF_LEN(buf_len);
}

POSSIBLY_UNUSED static void hal_sdio_device_set_transfer_status(enum HAL_SDIO_ID_T id, enum HAL_SDIO_DEVICE_TRANSFER_STATUS status)
{
    sdio_device[id]->TRANSFER_STATUS = status;
}

enum HAL_SDIO_DEVICE_TRANSFER_STATUS hal_sdio_device_get_transfer_status(enum HAL_SDIO_ID_T id)
{
    return sdio_device[id]->TRANSFER_STATUS;
}

uint32_t hal_sdio_device_get_irq_status(enum HAL_SDIO_ID_T id)
{
    return sdio_device[id]->IRQ_STATUS;
}

static void hal_sdio_device_rxbuf_cnt_cfg(enum HAL_SDIO_ID_T id, uint8_t buf_cnt)
{
    sdio_device[id]->RX_BUF_CFG = SET_BITFIELD(sdio_device[id]->RX_BUF_CFG, SDIO_RX_BUF_CNT, buf_cnt);
}

static void hal_sdio_irq_proc(enum HAL_SDIO_ID_T id)
{
    uint32_t i;
    uint32_t branch;
    uint32_t status;
    uint32_t status2;
    uint8_t  re_open = 0;

    status = sdio_device[id]->INTERRUPT_STATUS;
    status2 = sdio_device[id]->INTERRUPT_STATUS2;
    hal_sdio_device_interrupt_status_disable(id, status);
    hal_sdio_device_interrupt_status2_disable(id, status2);
    hal_sdio_device_interrupt_signal_disable(id);
    sdio_device[id]->INTERRUPT_STATUS = status;  //clear int flag
    sdio_device[id]->INTERRUPT_STATUS2 = status2;//clear int flag

    //Ensure that the flag clearing action is completed
    sdio_device[id]->INTERRUPT_STATUS;
    sdio_device[id]->INTERRUPT_STATUS2;

    HAL_SDIO_DEVICE_TRACE(0, "   ");
    HAL_SDIO_DEVICE_TRACE(1, "---sdio device irq status=0x%X", status);
    HAL_SDIO_DEVICE_TRACE(1, "---sdio device irq status2=0x%X", status2);
#ifdef SDIO_DEVICE_DEBUG
    //print interrupt status
    for (i = 0; i < 32; i++) {
        if (status & (1 << i))
            HAL_SDIO_DEVICE_TRACE(1, "%s", sdio_int_status[i].name);
    }
    for (i = 0; i < 7; i++) {
        if (status2 & (1 << i))
            HAL_SDIO_DEVICE_TRACE(1, "%s", sdio_int_status2[i].name);
    }
#endif

    //-----------------High priority states are processed first-----------------
    if (status & SDIO_CMD11_CLK_STOP_INT) {
        status &= ~SDIO_CMD11_CLK_STOP_INT;

        hal_iomux_set_sdio_voltage(HAL_IOMUX_PIN_VOLTAGE_MEM);
        //Complete voltage switching within 5ms,2021-07-04
        HAL_SDIO_DEVICE_TRACE(0, "------sdio device signal voltage switching completed");
    }
    if (status2 & SDIO_ADMA_ERROR_INT) {
        status2 &= ~SDIO_ADMA_ERROR_INT;
        //No need to do anything under the current situation,2021-07-04
    }
    if (status & SDIO_FUNCX_ABORT_INT) {
        status &= ~SDIO_FUNCX_ABORT_INT;
        //No need to do anything under the current situation,2021-07-04
    }
    if (status & SDIO_CMD0_CMD52_SOFTRST_INT) {
        status &= ~SDIO_CMD0_CMD52_SOFTRST_INT;
        re_open = 1;
    }
    if (status & SDIO_FUNC1_RESET_INT) {
        status &= ~SDIO_FUNC1_RESET_INT;
        // TODO:2021-05-21
    }
    if (status & SDIO_FUNC2_RESET_INT) {
        status &= ~SDIO_FUNC2_RESET_INT;
        // TODO:2021-05-21
    }
    if (status & SDIO_FUNCX_CRC_END_ERROR_INT) {
        status &= ~SDIO_FUNCX_CRC_END_ERROR_INT;
        sdio_rxtx_ctrl[id].error = 1;
    }
    if (status & SDIO_HOST_INT) {
        status &= ~SDIO_HOST_INT;
        hal_sdio_device_clr_host_int(id);
        if (sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_int_from_host) {
            sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_int_from_host();
        }
    }

    //--------------------Low priority state post processing--------------------
    while (status) {
        i = get_lsb_pos(status);
        branch = status & (1 << i);
        status &= ~(1 << i);
        switch (branch) {
            case SDIO_PROGRAM_START_INT: {
                sdio_rxtx_ctrl[id].status = HAL_SDIO_DEVICE_IDLE;
                if (sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_enum_done) {
                    sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_enum_done(HAL_SDIO_DEVICE_RDY);
                }
                break;
            }
            case SDIO_WR_START_INT:
            case SDIO_RD_START_INT: {
                uint32_t cmd = 0;
                uint32_t arg;
                uint16_t block_size;
                uint32_t address;
                uint16_t length;
                uint32_t register_address;
                uint32_t block_mode;
                uint32_t size;

                cmd = sdio_device[id]->COMMAND;
                arg = sdio_device[id]->ARGUMENT;
                block_size = (cmd >> 1) & 0xFFF;
                register_address = (arg >> 9) & 0x1FFFF;
                block_mode = (arg >> 27) & 0x1;
                size = arg & 0x1FF;
                if (block_mode) {
                    length = size * block_size;
                } else {
                    if (size == 0)
                        length = block_size;
                    else
                        length = size;
                }
                HAL_SDIO_DEVICE_TRACE(1, "***length:%d", length);

#if defined(CHIP_BEST2002)
                hal_sdio_device_set_transfer_status(id, HAL_SDIO_DEVICE_TRANSFER_BUSY);
#endif

                if (branch == SDIO_WR_START_INT) {
                    sdio_rxtx_ctrl[id].status = HAL_SDIO_DEVICE_RX;
                    if (sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_rx_start) {
                        sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_rx_start(register_address);
                    }

                    ASSERT(sdio_rxtx_ctrl[id].rxbuf_cnt != 0, "%s:%d, sdio%d no rx buf", __func__, __LINE__, id);
                    ASSERT(sdio_rxtx_ctrl[id].rxaddr != NULL, "%s:%d, sdio%d rx buf is NULL", __func__, __LINE__, id);

                    address = (uint32_t)sdio_rxtx_ctrl[id].rxaddr;
                    sdio_rxtx_ctrl[id].rx_real_len = MIN(length, sdio_rxtx_ctrl[id].rxlen);

                    sdio_rxtx_ctrl[id].rxbuf_cnt--;
                    hal_sdio_device_rxbuf_cnt_cfg(id, sdio_rxtx_ctrl[id].rxbuf_cnt);
                } else {
                    sdio_rxtx_ctrl[id].status = HAL_SDIO_DEVICE_TX;
                    if (sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_tx_start) {
                        sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_tx_start(register_address);
                    }

                    address = (uint32_t)sdio_rxtx_ctrl[id].txaddr;
                    sdio_rxtx_ctrl[id].tx_real_len = MIN(length, sdio_rxtx_ctrl[id].txlen);
                    ASSERT(block_size != 0, "%s:%d, error: block_size = 0", __func__, __LINE__);
                }

                sdio_rxtx_ctrl[id].error = 0;

#ifdef SDIO_DEVICE_ENABLE_DMA
                sdio_device[id]->DMA1_ADDRESS = address;//device view
                sdio_device[id]->DMA1_CONTROL = SDIO_DMA1_ADDR_VALID | SDIO_DMA1_BUF_SIZE(length / DMA_BOUNDARY_SIZE - 1);
#else
                if (branch == SDIO_WR_START_INT) {
#ifdef SDIO_DEVICE_AGGR
                    if (sdio_rxtx_ctrl[id].rx_aggr_cnt > 0) {
                        sdio_rxtx_ctrl[id].rx_aggr_cnt = 0;
                    } else
#endif
                    {
                        hal_sdio_device_adma_desc_init(id, rx_desc);
                        hal_sdio_device_adma_desc_add(id, rx_desc, address, length);
                        hal_sdio_device_adma_desc_end(id, rx_desc);
                    }
                    sdio_device[id]->DMA1_ADDRESS = (uint32_t)&rx_desc[id].adma_desc[0].adma_desc_ctrl;
                } else {
#ifdef SDIO_DEVICE_AGGR
                    if (sdio_rxtx_ctrl[id].tx_aggr_cnt > 0) {
                        sdio_rxtx_ctrl[id].tx_aggr_cnt = 0;
                    } else
#endif
                    {
                        hal_sdio_device_adma_desc_init(id, tx_desc);
                        hal_sdio_device_adma_desc_add(id, tx_desc, address, length);
                        hal_sdio_device_adma_desc_end(id, tx_desc);
                    }
                    sdio_device[id]->DMA1_ADDRESS = (uint32_t)&tx_desc[id].adma_desc[0].adma_desc_ctrl;
                }
                HAL_SDIO_DEVICE_TRACE(1, "***sdio_device[%d]->DMA1_ADDRESS=0x%X", id, sdio_device[id]->DMA1_ADDRESS);
                sdio_device[id]->DMA1_CONTROL = SDIO_DEVICE_ADMA_ATT_VALID;
#endif
                HAL_SDIO_DEVICE_TRACE(1, "cmd=0x%X received", cmd);
                HAL_SDIO_DEVICE_TRACE(1, "   Application cmd :%s", (cmd & 0x1) ? "Application cmd" : "Not an Application cmd");
                HAL_SDIO_DEVICE_TRACE(1, "   block/byte size :%d", block_size);
                HAL_SDIO_DEVICE_TRACE(1, "   cmd             :cmd%d", (cmd >> 13) & 0x3F);
                if (((cmd >> 19) & 0x3) == 0)
                    HAL_SDIO_DEVICE_TRACE(1, "   bus width       :1 bit");
                else if (((cmd >> 19) & 0x3) == 1)
                    HAL_SDIO_DEVICE_TRACE(1, "   bus width       :4 bit");
                else if (((cmd >> 19) & 0x3) == 2)
                    HAL_SDIO_DEVICE_TRACE(1, "   bus width       :5 bit");
                else
                    HAL_SDIO_DEVICE_TRACE(1, "   bus width       :Reserved");
                HAL_SDIO_DEVICE_TRACE(1, "   current speed   :0x%X", (cmd >> 21) & 0x7);
                HAL_SDIO_DEVICE_TRACE(1, "   card state      :0x%X", (cmd >> 24) & 0xF);
                HAL_SDIO_DEVICE_TRACE(1, "arg=0x%X received", arg);
                HAL_SDIO_DEVICE_TRACE(1, "   byte/block count:%d", arg & 0x1FF);
                HAL_SDIO_DEVICE_TRACE(1, "   register addr   :0x%X", register_address);
                HAL_SDIO_DEVICE_TRACE(1, "   OP code         :%s", ((arg >> 26) & 0x1) ? "incrementing address" : "fixed address");
                HAL_SDIO_DEVICE_TRACE(1, "   block mode      :%s", ((arg >> 27) & 0x1) ? "block mode" : "byte mode");
                HAL_SDIO_DEVICE_TRACE(1, "   function number :%d", (arg >> 28) & 0x7);
                HAL_SDIO_DEVICE_TRACE(1, "   r/w flag        :%s", ((arg >> 31) & 0x1) ? "write mode" : "read mode");
                HAL_SDIO_DEVICE_TRACE(1, "sdio_device[%d]->DMA1_ADDRESS=0x%X@0x%X", id,
                                      (uint32_t)sdio_device[id]->DMA1_ADDRESS,
                                      (uint32_t)&sdio_device[id]->DMA1_ADDRESS);
                HAL_SDIO_DEVICE_TRACE(1, "sdio_device[%d]->DMA1_CONTROL=0x%X@0x%X", id,
                                      (uint32_t)sdio_device[id]->DMA1_CONTROL,
                                      (uint32_t)&sdio_device[id]->DMA1_CONTROL);
                break;
            }
            case SDIO_TRX_COMPLETE_INT: {
                if (sdio_rxtx_ctrl[id].status == HAL_SDIO_DEVICE_TX) { //send to host
                    sdio_rxtx_ctrl[id].status = HAL_SDIO_DEVICE_IDLE;
                    if (!sdio_rxtx_ctrl[id].error) {
                        if (sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_send_done) {
                            sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_send_done(sdio_rxtx_ctrl[id].txaddr, sdio_rxtx_ctrl[id].tx_real_len);
                        }
                        HAL_SDIO_DEVICE_TRACE(0, "---device tx ok");
                        HAL_SDIO_DEVICE_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
                    }
                } else if (sdio_rxtx_ctrl[id].status == HAL_SDIO_DEVICE_RX) { //read from host
                    sdio_rxtx_ctrl[id].status = HAL_SDIO_DEVICE_IDLE;
                    if (!sdio_rxtx_ctrl[id].error) {
                        uint8_t *rxaddr = sdio_rxtx_ctrl[id].rxaddr;

                        sdio_rxtx_ctrl[id].rxaddr = NULL;

                        if (sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_recv_done) {
                            sdio_rxtx_ctrl[id].sdio_callback->hal_sdio_device_recv_done(rxaddr, sdio_rxtx_ctrl[id].rx_real_len);
                        }
                        HAL_SDIO_DEVICE_TRACE(0, "---device rx ok");
                        HAL_SDIO_DEVICE_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
                    }
                    //enable next block transfer
                    sdio_device[id]->CONTROL |= SDIO_PROGRAM_DONE;
                }

#if defined(CHIP_BEST2002)
                hal_sdio_device_set_transfer_status(id, HAL_SDIO_DEVICE_TRANSFER_IDLE);
#endif
                break;
            }
            case SDIO_DMA1_INT: {
                HAL_SDIO_DEVICE_TRACE(1, "---DMA1 int ,After enabling adma, the interrupt is no longer used");
                break;
            }
            default: {
                break;
            }
        }
    }

    if (!status2)
        goto out;

    switch (status2) {
        default:
            HAL_SDIO_DEVICE_TRACE(2, "%s:%d,error:Unknown status2=0x%X", __func__, __LINE__, status2);
            break;
    }

out:
    hal_sdio_device_interrupt_status_enable(id);
    hal_sdio_device_interrupt_signal_enable(id);

    if (re_open) {
        re_open = 0;

        //Re-initialization
        hal_sdio_device_open(id, sdio_rxtx_ctrl[id].sdio_callback);
    }
}

static void hal_sdio_device_irq_handler(void)
{
    enum HAL_SDIO_ID_T id;

    for (id = HAL_SDIO_ID_0; id < HAL_SDIO_ID_NUM; id++) {
        if (NVIC_GetActive(sdio_irq[id])) {
            hal_sdio_irq_proc(id);
        }
    }
}

static void hal_sdio_device_irq_enable(enum HAL_SDIO_ID_T id)
{
    NVIC_SetVector(sdio_irq[id], (uint32_t)hal_sdio_device_irq_handler);
    NVIC_SetPriority(sdio_irq[id], IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(sdio_irq[id]);
    NVIC_EnableIRQ(sdio_irq[id]);
}

static void hal_sdio_device_irq_disable(enum HAL_SDIO_ID_T id)
{
    NVIC_DisableIRQ(sdio_irq[id]);
}

void hal_sdio_device_open(enum HAL_SDIO_ID_T id, struct HAL_SDIO_DEVICE_CB_T *callback)
{
    uint32_t val = 0;

    HAL_SDIO_DEVICE_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
    HAL_SDIO_DEVICE_TRACE(2, "%s:%d, start", __func__, __LINE__);

    if (id == HAL_SDIO_ID_0)
        hal_cmu_sdio_device_clock_enable();
#ifdef SDIO1_DEVICE_BASE
    else if (id == HAL_SDIO_ID_1) {
        hal_cmu_sdio1_device_clock_enable();
    }
#endif
    hal_sys_timer_delay_us(10);

    memset(&sdio_rxtx_ctrl[id], 0, sizeof(struct SDIO_DEVICE_RXTX_CTRL_T));
    sdio_rxtx_ctrl[id].status = HAL_SDIO_DEVICE_NOT_RDY;
    hal_sdio_device_read_cccr(id, &val);                //Cross clock domain, so read and write again
#if defined(CHIP_BEST2002)
    val &= ~SDIO_SDR104; //clear SDR104 bit
#endif
    hal_sdio_device_write_cccr(id, val);

    // disable an eco
    sdio_device[id]->CONTROL |= SDIO_RESERVED;

    hal_sdio_device_read_tplmanid(id, &val);            //Cross clock domain, so read and write again
#if defined(SDIO_VENDOR_ID) && defined(SDIO_DEVICE_ID)
    val = SDIO_TPLMID_MANF(SDIO_VENDOR_ID) | SDIO_TPLMID_CARD(SDIO_DEVICE_ID);
#endif
    hal_sdio_device_write_tplmanid(id, val);
    hal_sdio_device_write_card_addr(id, 0);
    hal_sdio_device_interrupt_status_enable(id);
    hal_sdio_device_io_ready(id);
    hal_sdio_device_write_ocr(id);
#ifndef SDIO_DEVICE_ENABLE_DMA
    hal_sdio_device_enable_adma(id);
#endif
    hal_sdio_device_set_card_ready(id);

    sdio_rxtx_ctrl[id].status = HAL_SDIO_DEVICE_RDY;
    sdio_rxtx_ctrl[id].sdio_callback = callback;
    hal_sdio_device_irq_enable(id);
    hal_sdio_device_set_transfer_status(id, HAL_SDIO_DEVICE_TRANSFER_IDLE);

#if !defined(CHIP_BEST2002)
    // enable eco
    sdio_device[id]->DMA2_ADDRESS |= SDIO_DMA_ABORT_IDLE | SDIO_DMA_KEEP_BLKSIZE | SDIO_DMA_UPDATE_ON_COMPLETION | SDIO_DATA1_INT_CLR;
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->DMA2_ADDRESS=0x%x", id, sdio_device[id]->DMA2_ADDRESS);
#endif
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->SDIO_CCCR_CONTROL=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->SDIO_CCCR_CONTROL,
                          (uint32_t)&sdio_device[id]->SDIO_CCCR_CONTROL);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->CARD_ADDRESS=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->CARD_ADDRESS,
                          (uint32_t)&sdio_device[id]->CARD_ADDRESS);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->INTERRUPT_STATUS_ENABLE=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->INTERRUPT_STATUS_ENABLE,
                          (uint32_t)&sdio_device[id]->INTERRUPT_STATUS_ENABLE);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->INTERRUPT_STATUS2_ENABLE=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->INTERRUPT_STATUS2_ENABLE,
                          (uint32_t)&sdio_device[id]->INTERRUPT_STATUS2_ENABLE);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->IO_READY=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->IO_READY,
                          (uint32_t)&sdio_device[id]->IO_READY);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->CARD_OCR=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->CARD_OCR,
                          (uint32_t)&sdio_device[id]->CARD_OCR);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->CONTROL=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->CONTROL,
                          (uint32_t)&sdio_device[id]->CONTROL);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->CONTROL2=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->CONTROL2,
                          (uint32_t)&sdio_device[id]->CONTROL2);
    HAL_SDIO_DEVICE_TRACE(2, "%s:%d,end", __func__, __LINE__);
    HAL_SDIO_DEVICE_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
}

void hal_sdio_device_reopen(enum HAL_SDIO_ID_T id, struct HAL_SDIO_DEVICE_CB_T *callback)
{
    HAL_SDIO_DEVICE_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
    HAL_SDIO_DEVICE_TRACE(2, "%s:%d, start", __func__, __LINE__);

    memset(&sdio_rxtx_ctrl[id], 0, sizeof(struct SDIO_DEVICE_RXTX_CTRL_T));
    sdio_rxtx_ctrl[id].status = HAL_SDIO_DEVICE_IDLE;
    sdio_rxtx_ctrl[id].sdio_callback = callback;
    hal_sdio_device_irq_enable(id);
    hal_sdio_device_set_transfer_status(id, HAL_SDIO_DEVICE_TRANSFER_IDLE);

#if !defined(CHIP_BEST2002)
    // enable eco
    sdio_device[id]->DMA2_ADDRESS |= SDIO_DMA_ABORT_IDLE | SDIO_DMA_KEEP_BLKSIZE | SDIO_DMA_UPDATE_ON_COMPLETION | SDIO_DATA1_INT_CLR;
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->DMA2_ADDRESS=0x%x", id, sdio_device[id]->DMA2_ADDRESS);
#endif

    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->SDIO_CCCR_CONTROL=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->SDIO_CCCR_CONTROL,
                          (uint32_t)&sdio_device[id]->SDIO_CCCR_CONTROL);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->CARD_ADDRESS=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->CARD_ADDRESS,
                          (uint32_t)&sdio_device[id]->CARD_ADDRESS);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->INTERRUPT_STATUS_ENABLE=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->INTERRUPT_STATUS_ENABLE,
                          (uint32_t)&sdio_device[id]->INTERRUPT_STATUS_ENABLE);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->INTERRUPT_STATUS2_ENABLE=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->INTERRUPT_STATUS2_ENABLE,
                          (uint32_t)&sdio_device[id]->INTERRUPT_STATUS2_ENABLE);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->IO_READY=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->IO_READY,
                          (uint32_t)&sdio_device[id]->IO_READY);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->CARD_OCR=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->CARD_OCR,
                          (uint32_t)&sdio_device[id]->CARD_OCR);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->CONTROL=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->CONTROL,
                          (uint32_t)&sdio_device[id]->CONTROL);
    HAL_SDIO_DEVICE_TRACE(2, "sdio_device[%d]->CONTROL2=0x%X@0x%X", id,
                          (uint32_t)sdio_device[id]->CONTROL2,
                          (uint32_t)&sdio_device[id]->CONTROL2);
    HAL_SDIO_DEVICE_TRACE(2, "%s:%d,end", __func__, __LINE__);
    HAL_SDIO_DEVICE_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
}

void hal_sdio_device_close(enum HAL_SDIO_ID_T id)
{
    sdio_rxtx_ctrl[id].status = HAL_SDIO_DEVICE_NOT_RDY;
    sdio_device[id]->CONTROL &= ~SDIO_CARD_INIT_DONE;
    hal_sdio_device_disable_adma(id);
    hal_sdio_device_irq_disable(id);
    if (id == HAL_SDIO_ID_0)
        hal_cmu_sdio_device_clock_disable();
#ifdef SDIO1_DEVICE_BASE
    else if (id == HAL_SDIO_ID_1) {
        hal_cmu_sdio1_device_clock_disable();
    }
#endif
}

void hal_sdio_device_txbuf_cfg(enum HAL_SDIO_ID_T id, uint32_t buf_attr)
{
    sdio_device[id]->TX_BUF_CFG = buf_attr;
}

uint32_t hal_sdio_device_get_txbuf_cfg(enum HAL_SDIO_ID_T id)
{
    return sdio_device[id]->TX_BUF_CFG;
}

bool hal_sdio_device_get_card_ready(enum HAL_SDIO_ID_T id)
{
    enum HAL_CMU_MOD_ID_T clk = HAL_CMU_MOD_QTY;

    if (id == HAL_SDIO_ID_0)
        clk = HAL_CMU_MOD_H_SDIO_SLAVE;
#ifdef SDIO1_DEVICE_BASE
    else if (id == HAL_SDIO_ID_1) {
        clk = HAL_CMU_MOD_H_SDIO1_SLAVE;
    }
#endif

    if (hal_cmu_clock_get_status(clk) == HAL_CMU_CLK_DISABLED)
        return false;

    return !!(sdio_device[id]->CONTROL & SDIO_CARD_INIT_DONE);
}

#ifdef SDIO_DEVICE_AGGR
enum HAL_SDIO_DEVICE_STATUS hal_sdio_device_multi_send(enum HAL_SDIO_ID_T id, const struct HAL_SDIO_DEVICE_AGGR_DESC_T *desc, uint32_t buf_attr)
{
    uint32_t i;
    uint32_t max_len = 0;
    uint32_t lock;

    if (desc == NULL) {
        TRACE(TR_LEVEL_ERROR, "desc is NULL");
        return HAL_SDIO_DEVICE_ERR;
    }
    if (desc->cnt == 0 || desc->cnt > SDIO_DEVICE_AGGR_DESC_CNT) {
        TRACE(TR_LEVEL_ERROR, "desc cnt error :0x%x", desc->cnt);
        return HAL_SDIO_DEVICE_ERR;
    }

    HAL_SDIO_DEVICE_TRACE(1, "%s, buf[0]:0x%x, len[0]:%d, cnt:%d", __func__, desc->addr[0], desc->len[0], desc->cnt);

    // if (sdio_rxtx_ctrl[id].status != HAL_SDIO_DEVICE_IDLE) {
    //     HAL_SDIO_DEVICE_TRACE(1, "***sdio%d device status:%d,device is not idle", id, sdio_rxtx_ctrl[id].status);
    //     return sdio_rxtx_ctrl[id].status;
    // }

    lock = int_lock();
    hal_sdio_device_adma_desc_init(id, tx_desc);
    for (i = 0; i < desc->cnt; i++) {
        if (desc->addr[i] == 0 || desc->len[i] == 0) {
            TRACE(TR_LEVEL_ERROR, "desc[%u] error. addr:0x%x, len:0x%x", i,  desc->addr[i], desc->len[i]);
            int_unlock(lock);
            return HAL_SDIO_DEVICE_ERR;
        }
        hal_sdio_device_adma_desc_add(id, tx_desc, desc->addr[i], desc->len[i]);
        max_len += desc->len[i];
    }
    hal_sdio_device_adma_desc_end(id, tx_desc);
    sdio_rxtx_ctrl[id].tx_aggr_cnt = desc->cnt;

    sdio_rxtx_ctrl[id].txaddr = (uint8_t *)desc->addr[0];
    sdio_rxtx_ctrl[id].txlen = max_len;

    hal_sdio_device_gen_int_to_host(id, buf_attr);
    int_unlock(lock);

    return HAL_SDIO_DEVICE_IDLE;
}
#endif

enum HAL_SDIO_DEVICE_STATUS hal_sdio_device_send(enum HAL_SDIO_ID_T id, const uint8_t *buf, uint16_t len, uint32_t buf_attr)
{
    uint32_t lock;

    // if (sdio_rxtx_ctrl[id].status != HAL_SDIO_DEVICE_IDLE) {
    //     HAL_SDIO_DEVICE_TRACE(1, "***sdio%d device status:%d,device is not idle", id, sdio_rxtx_ctrl[id].status);
    //     return sdio_rxtx_ctrl[id].status;
    // }

    lock = int_lock();
    sdio_rxtx_ctrl[id].txaddr = (uint8_t *)buf;
    sdio_rxtx_ctrl[id].txlen = len;

    hal_sdio_device_gen_int_to_host(id, buf_attr);//Notify the host to read data from the device
    int_unlock(lock);

    HAL_SDIO_DEVICE_TRACE(1, "---Notify the host to read data:0x%X", len);
    HAL_SDIO_DEVICE_TRACE(1, "sdio_rxtx_ctrl[%d].txaddr=0x%X", id,
                          (uint32_t)sdio_rxtx_ctrl[id].txaddr);
    HAL_SDIO_DEVICE_TRACE(1, "sdio_rxtx_ctrl[%d].txlen=0x%X", id,
                          (uint32_t)sdio_rxtx_ctrl[id].txlen);

    return HAL_SDIO_DEVICE_IDLE;
}

enum HAL_SDIO_DEVICE_STATUS hal_sdio_device_resend(enum HAL_SDIO_ID_T id, const uint8_t *buf, uint16_t len, uint32_t buf_attr)
{
    uint32_t status;
    uint32_t lock;

    if (sdio_rxtx_ctrl[id].status != HAL_SDIO_DEVICE_TX) {
        HAL_SDIO_DEVICE_TRACE(1, "***sdio%d tx reconfig:%d,not in tx", id, sdio_rxtx_ctrl[id].status);
        return sdio_rxtx_ctrl[id].status;
    }

    lock = int_lock();
    sdio_rxtx_ctrl[id].txaddr = (uint8_t *)buf;
    sdio_rxtx_ctrl[id].txlen = len;
    status = sdio_rxtx_ctrl[id].status;
    int_unlock(lock);

    return status;
}

void hal_sdio_device_recv(enum HAL_SDIO_ID_T id, uint8_t *buf, uint32_t buf_len, uint8_t buf_cnt)
{
    uint32_t lock;

    HAL_SDIO_DEVICE_TRACE(1, "%s, buf:%p, len:%d, buf_cnt:%d", __func__, buf, buf_len, buf_cnt);

    if (buf == NULL || buf_len == 0) {
        TR_ERROR(0, "rx buf is NULL. buf:%p, len:%d", buf, buf_len);
        return;
    }
    if (buf_len > SDIO_DEVICE_ADMA_TXRX_LEN) {
        TR_ERROR(0, "rx buf len %d > %d", buf_len, SDIO_DEVICE_ADMA_TXRX_LEN);
        return;
    }

    //rxbuf_cnt should be equal or greater than the old
    ASSERT(buf_cnt >= sdio_rxtx_ctrl[id].rxbuf_cnt, "%s, rxbuf_cnt mismatch:%d, %d",
           __func__, buf_cnt, sdio_rxtx_ctrl[id].rxbuf_cnt);

    lock = int_lock();
    sdio_rxtx_ctrl[id].rxaddr = buf;
    sdio_rxtx_ctrl[id].rxlen = buf_len;

    sdio_rxtx_ctrl[id].rxbuf_cnt = buf_cnt;
    hal_sdio_device_rxbuf_cfg(id, buf_len, buf_cnt);
    int_unlock(lock);
}

#ifdef SDIO_DEVICE_AGGR
enum HAL_SDIO_DEVICE_STATUS hal_sdio_device_multi_recv(enum HAL_SDIO_ID_T id, const struct HAL_SDIO_DEVICE_AGGR_DESC_T *desc)
{
    uint32_t i;
    uint32_t max_len = 0;
    uint32_t lock;

    if (desc == NULL) {
        TRACE(TR_LEVEL_ERROR, "desc is NULL");
        return HAL_SDIO_DEVICE_ERR;
    }
    if (desc->cnt == 0 || desc->cnt > SDIO_DEVICE_AGGR_DESC_CNT) {
        TRACE(TR_LEVEL_ERROR, "desc cnt error :0x%x", desc->cnt);
        return HAL_SDIO_DEVICE_ERR;
    }

    HAL_SDIO_DEVICE_TRACE(1, "%s, buf[0]:0x%x, len[0]:%d, cnt:%d", __func__, desc->addr[0], desc->len[0], desc->cnt);

    lock = int_lock();
    hal_sdio_device_adma_desc_init(id, rx_desc);
    for (i = 0; i < desc->cnt; i++) {
        if (desc->addr[i] == 0 || desc->len[i] == 0) {
            TRACE(TR_LEVEL_ERROR, "desc[%u] error. addr:0x%x, len:0x%x", i,  desc->addr[i], desc->len[i]);
            int_unlock(lock);
            return HAL_SDIO_DEVICE_ERR;
        }
        hal_sdio_device_adma_desc_add(id, rx_desc, desc->addr[i], desc->len[i]);
        max_len += desc->len[i];
    }
    hal_sdio_device_adma_desc_end(id, rx_desc);
    sdio_rxtx_ctrl[id].rx_aggr_cnt = desc->cnt;

    sdio_rxtx_ctrl[id].rxaddr = (uint8_t *)desc->addr[0];
    sdio_rxtx_ctrl[id].rxlen = max_len;
    sdio_rxtx_ctrl[id].rxbuf_cnt = 1;
    int_unlock(lock);

    return HAL_SDIO_DEVICE_IDLE;
}
#endif /*SDIO_DEVICE_AGGR*/

#endif