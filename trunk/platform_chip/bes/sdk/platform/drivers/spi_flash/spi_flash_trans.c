/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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

#ifdef SPI_BASE

#include "plat_types.h"
#include "hal_spi.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#if (CHIP_FLASH_CTRL_VER >= 5)
#include "nandflash_drv.h"
#endif
#include "spi_flash_trans.h"
#include "hal_trace.h"
#include "string.h"
#include "hal_gpio.h"

#ifdef SPI_FLASH_DEBUG
    #define SPIFLASH_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
    #define SPIFLASH_ERROR(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#else
    #define SPIFLASH_TRACE(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define SPIFLASH_ERROR(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#endif

#define TO_LITTLE_ENDIAN(val)   (((val & 0xff) << 24) | ((val & 0xff00) << 8) | \
                                 ((val & 0xff0000) >> 8) | ((val & 0xff000000) >> 24))

#ifdef SPI_FLASH_USING_SPILCD
    extern void hal_iomux_set_spilcd(void);
    #define SPI_DMA_SEND             hal_spilcd_dma_send
    #define SPI_DMA_RECV             hal_spilcd_dma_recv
    #define SPI_ENABLE_AND_SEND      hal_spilcd_enable_and_send
    #define SPI_ENABLE_AND_RECV      hal_spilcd_enable_and_recv
    #define SPI_OPEN                 hal_spilcd_open
    #define SPI_CLOSE                hal_spilcd_close
    #define IOMUX_SET_SPI            hal_iomux_set_spilcd
#else
    extern void hal_iomux_set_spi(void);
    #define SPI_DMA_SEND             hal_spi_dma_send
    #define SPI_DMA_RECV             hal_spi_dma_recv
    #define SPI_ENABLE_AND_SEND      hal_spi_enable_and_send
    #define SPI_ENABLE_AND_RECV      hal_spi_enable_and_recv
    #define SPI_OPEN                 hal_spi_open
    #define SPI_CLOSE                hal_spi_close
    #define IOMUX_SET_SPI            hal_iomux_set_spi
#endif

static uint8_t spi_send_buffer[SPI_CMD_SIZE + SPI_BUFFER_SIZE];
static uint8_t spi_recv_buffer[SPI_CMD_SIZE + SPI_BUFFER_SIZE];
static struct HAL_SPI_CTRL_T _spi_ctrl_8bit;
static uint8_t spi_cs = 0;

static enum HAL_IOMUX_PIN_T spi_cs_pin = HAL_IOMUX_PIN_NUM;
#define SPI_CS_PIN_SET()  hal_gpio_pin_set(spi_cs_pin)
#define SPI_CS_PIN_CLR()  hal_gpio_pin_clr(spi_cs_pin)


static void spi_cs_pin_init(enum HAL_IOMUX_PIN_T cs_pin)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL};

    spi_cs_pin = cs_pin;
    ASSERT(spi_cs_pin < HAL_IOMUX_PIN_NUM, "%s spi_cs_pin %02d invalid", __func__, spi_cs_pin);

    gpio_map.pin = spi_cs_pin;
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(spi_cs_pin,  HAL_GPIO_DIR_OUT,  1);
    SPIFLASH_TRACE(1, "spi_cs_pin = %02d\n", spi_cs_pin);
}

static void spi_flash_to_little_endian(uint8_t *out, uint8_t *in, uint32_t len)
{
    uint32_t *pout;
    uint32_t *pin;

    pin = (uint32_t *)in;
    pout = (uint32_t *)out;

    while (len > 0) {
        *pout = TO_LITTLE_ENDIAN(*pin);
        pin ++;
        pout ++;
        len -= 4;
    }
}

//---------------------------------------------------------------------------
// SPI FLASH IO API
//---------------------------------------------------------------------------
int spi_flash_send_word(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len)
{
    int32_t ret;
    uint32_t len;
    uint8_t *res_buff = (uint8_t *)spi_recv_buffer;
    uint8_t *dst_buff = (uint8_t *)spi_send_buffer;

    ASSERT((buff_len + cmd_len <= SPI_CMD_SIZE + SPI_BUFFER_SIZE),
           "%s: length too large, cmd_len = %d,buff_len = %d",
           __func__, cmd_len, buff_len);
    ASSERT((buff == NULL && buff_len == 0)
           || (buff != NULL && buff_len > 0),
           "%s: buff is null,but buff_len = %d",
           __func__, buff_len);
    ASSERT((cmd_len / 4) * 4 == cmd_len, "%s: No word alignment! cmd_len = %d", __func__, cmd_len);
    ASSERT((buff_len / 4) * 4 == buff_len, "%s: No word alignment! buff_len = %d", __func__, buff_len);

    memcpy(res_buff, cmd, cmd_len);
    len = cmd_len;
    if (buff_len > 0) {
        memcpy(res_buff + cmd_len, buff, buff_len);
        len += buff_len;
    }

    spi_flash_to_little_endian(dst_buff, res_buff, len);

    SPI_CS_PIN_CLR();
    ret = SPI_DMA_SEND(dst_buff, len, NULL);
    SPI_CS_PIN_SET();
    if (ret) {
        SPIFLASH_ERROR(1, "%s, spi dma send failed! ret = %d",
                       __func__, ret);
        return SPI_FLASH_TRANS_ERROR;
    }

    return SPI_FLASH_TRANS_OK;
}

int spi_flash_recv_word(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len)
{
    int32_t ret;
    uint32_t len;
    uint8_t cmd_buff[SPI_CMD_SIZE];

    ASSERT((buff_len + cmd_len <= SPI_CMD_SIZE + SPI_BUFFER_SIZE),
           "%s: length too large, cmd_len = %d,buff_len = %d",
           __func__, cmd_len, buff_len);
    ASSERT(buff != NULL, "%s: buff is null", __func__);

    ASSERT((cmd_len / 4) * 4 == cmd_len, "%s: No word alignment! cmd_len = %d", __func__, cmd_len);
    ASSERT((buff_len / 4) * 4 == buff_len, "%s: No word alignment! buff_len = %d", __func__, buff_len);

    memset(cmd_buff, 0xFF, SPI_CMD_SIZE);
    memcpy(cmd_buff, cmd, cmd_len);
    spi_flash_to_little_endian(spi_send_buffer, cmd_buff, cmd_len);
    len = (buff_len + cmd_len);

    SPI_CS_PIN_CLR();
    ret = SPI_DMA_RECV(spi_send_buffer, spi_recv_buffer, len, NULL);
    SPI_CS_PIN_SET();
    if (ret) {
        SPIFLASH_ERROR(1, "%s, spi dma recv failed! ret = %d",
                       __func__, ret);
        return SPI_FLASH_TRANS_ERROR;
    }

    spi_flash_to_little_endian(buff, spi_recv_buffer + cmd_len, buff_len);

    return SPI_FLASH_TRANS_OK;
}

int spi_flash_send_byte(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len)
{
    int32_t ret;
    uint32_t len;

    ASSERT((buff_len + cmd_len <= SPI_CMD_SIZE + SPI_BUFFER_SIZE),
           "%s: length too large, cmd_len = %d,buff_len = %d",
           __func__, cmd_len, buff_len);
    ASSERT((buff == NULL && buff_len == 0)
           || (buff != NULL && buff_len > 0),
           "%s: buff is null,but buff_len = %d",
           __func__, buff_len);

    memcpy(spi_send_buffer, cmd, cmd_len);
    len = cmd_len;
    if (buff_len > 0) {
        memcpy(spi_send_buffer + cmd_len, buff, buff_len);
        len += buff_len;
    }

    SPI_CS_PIN_CLR();
    ret = SPI_ENABLE_AND_SEND(&_spi_ctrl_8bit, spi_send_buffer, len);
    SPI_CS_PIN_SET();
    if (ret) {
        SPIFLASH_ERROR(1, "%s, spi enable and send failed! ret = %d",
                       __func__, ret);
        return SPI_FLASH_TRANS_ERROR;
    }

    return SPI_FLASH_TRANS_OK;
}

int spi_flash_recv_byte(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len)
{
    int32_t ret;
    uint32_t len;

    ASSERT((buff_len + cmd_len <= SPI_CMD_SIZE + SPI_BUFFER_SIZE),
           "%s: length too large, cmd_len = %d,buff_len = %d",
           __func__, cmd_len, buff_len);

    ASSERT(buff != NULL,
           "%s: buff is null or buff_len = %d",
           __func__, buff_len);

    len = cmd_len + buff_len;
    memset(spi_send_buffer, 0x0, len);
    memcpy(spi_send_buffer, cmd, cmd_len);

    SPI_CS_PIN_CLR();
    ret = SPI_ENABLE_AND_RECV(&_spi_ctrl_8bit, spi_send_buffer, spi_recv_buffer, len);
    SPI_CS_PIN_SET();
    if (ret) {
        SPIFLASH_ERROR(1, "%s, spi enable and recv failed! ret = %d",
                       __func__, ret);
        return SPI_FLASH_TRANS_ERROR;
    }

    memcpy(buff, spi_recv_buffer + cmd_len, buff_len);

    return SPI_FLASH_TRANS_OK;
}

//------------------------------------------------------------------
// spi_flash_trans function
//------------------------------------------------------------------
int spi_flash_spi_init(const struct HAL_SPI_CFG_T *spi_cfg, uint32_t cs_pin)
{
    int32_t ret;
    struct HAL_SPI_CFG_T spi_cfg_8bit;

    spi_cfg_8bit = *spi_cfg;
    spi_cfg_8bit.dma_rx = false;
    spi_cfg_8bit.dma_tx = false;
    spi_cfg_8bit.tx_bits = 8;
    spi_cfg_8bit.rx_bits = 8;

    spi_cs = spi_cfg->cs;

    SPIFLASH_TRACE(0, "  ");
    SPIFLASH_TRACE(1, "spi_cfg_8bit.clk_delay_half:%d", spi_cfg_8bit.clk_delay_half);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.clk_polarity  :%d", spi_cfg_8bit.clk_polarity);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.slave         :%d", spi_cfg_8bit.slave);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.dma_rx        :%d", spi_cfg_8bit.dma_rx);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.dma_tx        :%d", spi_cfg_8bit.dma_tx);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.rx_sep_line   :%d", spi_cfg_8bit.rx_sep_line);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.cs            :%d", spi_cfg_8bit.cs);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.rate          :%d", spi_cfg_8bit.rate);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.rx_bits       :%d", spi_cfg_8bit.rx_bits);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.tx_bits       :%d", spi_cfg_8bit.tx_bits);
    SPIFLASH_TRACE(1, "spi_cfg_8bit.rx_frame_bits :%d", spi_cfg_8bit.rx_frame_bits);
    SPIFLASH_TRACE(0, "  ");

    IOMUX_SET_SPI();//
    ret = SPI_OPEN(spi_cfg);
    if (ret) {
        SPIFLASH_ERROR(1, "%s, spi open failed! ret = %d", __func__, ret);
        return SPI_FLASH_TRANS_ERROR;
    }

    hal_spi_init_ctrl(&spi_cfg_8bit, &_spi_ctrl_8bit);//Use additional SPI configuration

    spi_cs_pin_init((enum HAL_IOMUX_PIN_T)cs_pin);

    return SPI_FLASH_TRANS_OK;
}

int spi_flash_spi_deinit(void)
{
    int ret;
    ret = SPI_CLOSE(spi_cs);
    if (ret) {
        SPIFLASH_ERROR(1, "%s, spi close failed! ret = %d", __func__, ret);
        return SPI_FLASH_TRANS_ERROR;
    }

    return SPI_FLASH_TRANS_OK;
}

#endif
