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
#if (CHIP_FLASH_CTRL_VER >= 5)

#include "nandflash_drv.h"
#include "cmsis.h"
#include "hal_dma.h"
#include "hal_norflaship.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "nandflash_gd5f4gq4.h"
#include "nandflash_w25m02gv.h"
#include "plat_addr_map.h"
#include "norflash_cfg.h"
#include "nandflash_ecc.h"
#include "string.h"
#include "tool_msg.h"
#include "hal_sleep.h"
// #include "system_utils.h"

#ifdef PROGRAMMER
#include "sys_api_programmer.h"
#include "task_schedule.h"
#else
#define TASK_SCHEDULE                           true
#endif

#define NORFLASH_TRACE(level, ...)              TR_INFO(level, __VA_ARGS__)

#define SAMP_DELAY_PRIO_FALLING_EDGE

//#define NANDFLASH_UNIQUE_ID_LEN                 18

#define NANDFLASH_MAX_DIV                       0xFF

#define NANDFLASH_DEFAULT_MAX_SPEED             (104 * 1000 * 1000)

#define NANDFLASH_DIV1_MAX_SPEED                (99 * 1000 * 1000)

#define NANDFLASH_SPEED_RATIO_DENOMINATOR       8

enum NANDFLASH_CALIB_T {
    NANDFLASH_CALIB_FLASH_ID,
    NANDFLASH_CALIB_MAGIC_WORD,
    NANDFLASH_CALIB_DTR,

    NANDFLASH_CALIB_QTY,
};

struct NANDFLASH_CTX_T {
    bool opened;
    enum NANDFLASH_RET_T open_state;
#ifdef NANDFLASH_DELAY_WAKEUP
    bool in_sleep;
    enum HAL_CMU_LPU_SLEEP_MODE_T sleep_mode;
#endif
    uint8_t device_id[NANDFLASH_ID_LEN];
    uint32_t blocks_per_die;
    uint32_t cur_block;
    uint32_t cur_die;
};

static struct NANDFLASH_CTX_T nandflash_ctx[HAL_FLASH_ID_NUM];

static const char * const err_not_opened = "nandflash not opened";

POSSIBLY_UNUSED static uint32_t nand_nc_base[HAL_FLASH_ID_NUM] = {
    FLASH_NC_BASE,
#ifdef FLASH1_CTRL_BASE
    FLASH1_NC_BASE,
#endif
#ifdef FLASH2_CTRL_BASE
    FLASH2_NC_BASE,
#endif
};

static const struct NANDFLASH_CONFIG_T nandflash_cfg = {
#if defined(FLASH_SRC_FREQ_MHZ) || defined(FLASH_SPEED_MHZ)
    .source_clk = FLASH_SRC_FREQ_MHZ * 1000 * 1000,
    .speed = FLASH_SPEED_MHZ * 1000 * 1000,
#elif !defined(FLASH_HIGH_SPEED) && (defined(FPGA) || defined(FLASH_ULTRA_LOW_SPEED))
    .source_clk = NANDFLASH_SPEED_13M * 2,
    .speed = NANDFLASH_SPEED_13M,
#elif !defined(FLASH_HIGH_SPEED) && defined(FLASH_LOW_SPEED)
    .source_clk = NANDFLASH_SPEED_26M * 2,
    .speed = NANDFLASH_SPEED_26M,
#elif !defined(FLASH_HIGH_SPEED) && defined(FLASH_CLK_DIV_1)
    .source_clk = NANDFLASH_SPEED_52M,
    .speed = NANDFLASH_SPEED_52M,
#elif !defined(FLASH_HIGH_SPEED) && defined(OSC_26M_X4_AUD2BB)
    .source_clk = NANDFLASH_SPEED_52M * 2,
    .speed = NANDFLASH_SPEED_52M,
#else
    .source_clk = NANDFLASH_SPEED_104M * 2,
    .speed = NANDFLASH_SPEED_104M,
#endif
    .mode = NANDFLASH_OP_MODE_STAND_SPI |
            NANDFLASH_OP_MODE_FAST_SPI |
            NANDFLASH_OP_MODE_DUAL_OUTPUT |
            NANDFLASH_OP_MODE_DUAL_IO |
            NANDFLASH_OP_MODE_QUAD_OUTPUT |
            NANDFLASH_OP_MODE_QUAD_IO |
            NANDFLASH_OP_MODE_PAGE_PROGRAM |
            NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
            NANDFLASH_OP_MODE_INTERNAL_ECC |
#if (CHIP_FLASH_CTRL_VER >= 5) && defined(FLASH_DTR)
            NANDFLASH_OP_MODE_DTR |
#endif
            0,
    .override_config = 0,
};

// GigaDevice
extern const struct NANDFLASH_CFG_T gd5f1gm7_cfg;
extern const struct NANDFLASH_CFG_T gd5f4gq4_cfg;
extern const struct NANDFLASH_CFG_T gd5f4gm5_cfg;
extern const struct NANDFLASH_CFG_T gd5f2gm7r_cfg;
extern const struct NANDFLASH_CFG_T gd5f4gm8r_cfg;
extern const struct NANDFLASH_CFG_T gd5f1gq5_cfg;

// XTS
extern const struct NANDFLASH_CFG_T xt26g02a_cfg;
extern const struct NANDFLASH_CFG_T xt26q01d_cfg;
extern const struct NANDFLASH_CFG_T xt26q02d_cfg;
extern const struct NANDFLASH_CFG_T xt26q04d_cfg;
// WINBOND
extern const struct NANDFLASH_CFG_T w25m02gv_cfg;
extern const struct NANDFLASH_CFG_T w25n02kw_cfg;
extern const struct NANDFLASH_CFG_T w25n04kw_cfg;
extern const struct NANDFLASH_CFG_T w25n08lw_cfg;

// MXIC
extern const struct NANDFLASH_CFG_T mx35lf1g_cfg;
extern const struct NANDFLASH_CFG_T mt29f4g_cfg;

// Dosilicon
extern const struct NANDFLASH_CFG_T ds35m4gm_cfg;
extern const struct NANDFLASH_CFG_T ds35m2gb_cfg;
extern const struct NANDFLASH_CFG_T ds35m1ga_cfg;
extern const struct NANDFLASH_CFG_T ds35m4gb_cfg;

// UNIM
extern const struct NANDFLASH_CFG_T um19a1lisw_cfg;

// FORESEE
extern const struct NANDFLASH_CFG_T f35uqa001g_cfg;

static const struct NANDFLASH_CFG_T * const flash_list[] = {
    // ----------------------
    // GigaDevice
    // ----------------------
#if defined(__NANDFLASH_GD5F1GM7__) || defined(__NANDFLASH_ALL__)
    &gd5f1gm7_cfg,
#endif
#if defined(__NANDFLASH_GD5F4GQ4__) || defined(__NANDFLASH_ALL__)
    &gd5f4gq4_cfg,
#endif
#if defined(__NANDFLASH_GD5F4GM5__) || defined(__NANDFLASH_ALL__)
    &gd5f4gm5_cfg,
#endif
#if defined(__NANDFLASH_GD5F2GM7R__) || defined(__NANDFLASH_ALL__)
    &gd5f2gm7r_cfg,
#endif
#if defined(__NANDFLASH_GD5F4GM8R__) || defined(__NANDFLASH_ALL__)
    &gd5f4gm8r_cfg,
#endif
#if defined(__NANDFLASH_GD5F1GQ5__) || defined(__NANDFLASH_ALL__)
    &gd5f1gq5_cfg,
#endif

    // ----------------------
    // XTS
    // ----------------------
#if defined(__NANDFLASH_XT26G02A__)
    &xt26g02a_cfg,
#endif
#if defined(__NANDFLASH_XT26Q01D__) || defined(__NANDFLASH_ALL__)
    &xt26q01d_cfg,
#endif
#if defined(__NANDFLASH_XT26Q02D__) || defined(__NANDFLASH_ALL__)
    &xt26q02d_cfg,
#endif
#if defined(__NANDFLASH_XT26Q04D__)
    &xt26q04d_cfg,
#endif
    // ----------------------
    // WINBOND
    // ----------------------
#if defined(__NANDFLASH_W25M02GV__) || defined(__NANDFLASH_ALL__)
    &w25m02gv_cfg,
#endif
#if defined(__NANDFLASH_W25N02KV__) || defined(__NANDFLASH_ALL__)
    &w25n02kw_cfg,
#endif
#if defined(__NANDFLASH_W25N04KW__) || defined(__NANDFLASH_ALL__)
    &w25n04kw_cfg,
#endif
#if defined(__NANDFLASH_W25N08LW__) || defined(__NANDFLASH_ALL__)
    &w25n08lw_cfg,
#endif

    // ----------------------
    // Macronix
    // ----------------------
#if defined(__NANDFLASH_MX35LF1G__) || defined(__NANDFLASH_ALL__)
    &mx35lf1g_cfg,
#endif
#if defined(__NANDFLASH_MX35LF1G__) || defined(__NANDFLASH_ALL__)
    &mt29f4g_cfg,
#endif

    // ----------------------
    // Dosilicon
    // ----------------------
#if defined(__NANDFLASH_DS35M4G__) || defined(__NANDFLASH_ALL__)
    &ds35m4gm_cfg,
#endif
#if defined(__NANDFLASH_DS35M2G__) || defined(__NANDFLASH_ALL__)
    &ds35m2gb_cfg,
#endif
#if defined(__NANDFLASH_DS35M1G__) || defined(__NANDFLASH_ALL__)
    &ds35m1ga_cfg,
#endif
#if defined(__NANDFLASH_DS35M4G__)
    &ds35m4gb_cfg,
#endif

    // ----------------------
    // UNIM
    // ----------------------
#if defined(__NANDFLASH_UM19A1LISW__) || defined(__NANDFLASH_ALL__)
    &um19a1lisw_cfg,
#endif

    // ----------------------
    // FORESEE
    // ----------------------
#if defined(__NANDFLASH_F35UQA001G__) || defined(__NANDFLASH_ALL__)
    &f35uqa001g_cfg,
#endif

};

// Sample delay will be larger if:
// 1) flash speed is higher (major impact)
// 2) vcore voltage is lower (secondary major impact)
// 3) flash voltage is lower (minor impact)

// Sample delay unit:
// V1: 1/2 source_clk cycle when <= 2, 1 source_clk cycle when >= 2
// V2: 1/2 source_clk cycle when <= 4, 1 source_clk cycle when >= 4

// Flash clock low to output valid delay:
// T_clqv: 7 ns

// Flash IO latency:
// BEST1000/3001/1400: 4 ns
// BEST2000: 5 ns
// BEST2300: 2 ns

// Flash output time: T_clqv + T_io_latency
// Falling edge sample time: one spi_clk cycle (should > flash output time)

#define FALLING_EDGE_SAMPLE_ADJ_FREQ            (77 * 1000 * 1000) // about 13 ns

#ifdef SAMP_DELAY_PRIO_FALLING_EDGE
#define DIV1_SAMP_DELAY_FALLING_EDGE_IDX        1
static const uint8_t samdly_list_div1[] = { 0, 1, 2, 3, };
#define DIV2_SAMP_DELAY_FALLING_EDGE_IDX        1
#define DIVN_SAMP_DELAY_FALLING_EDGE_IDX        3
static const uint8_t samdly_list_divn[] = { 2, 3, 4, 5, 6, 7, 8, 9, };
#else
static const uint8_t samdly_list_divn[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, };
#endif

static uint8_t flash_idx[HAL_FLASH_ID_NUM];
static uint8_t div_read[HAL_FLASH_ID_NUM];
static uint32_t nandflash_op_mode[HAL_FLASH_ID_NUM];
static bool falling_edge_adj[HAL_FLASH_ID_NUM];
static uint8_t sample_delay_index[HAL_FLASH_ID_NUM];

#ifdef FLASH_CALIB_DEBUG
static uint32_t nandflash_source_clk;
static uint32_t nandflash_speed;
static uint8_t calib_matched_idx[NANDFLASH_CALIB_QTY];
static uint8_t calib_matched_cnt[NANDFLASH_CALIB_QTY];
static uint8_t calib_final_idx[NANDFLASH_CALIB_QTY];
#endif

#ifdef FLASH_DMA
static struct HAL_DMA_CH_CFG_T dma_cfg[HAL_FLASH_ID_NUM];
static struct HAL_DMA_DESC_T dma_desc[HAL_FLASH_ID_NUM][2];
#endif

//#define BUFFER_READ

static void nandflash_delay(uint32_t us)
{
    hal_sys_timer_delay_us(us);
}

static inline uint32_t nandflash_block_to_row_addr(uint32_t block)
{
    return (block << 6);
}

static inline uint32_t nandflash_block_page_to_row_addr(uint32_t block, uint32_t page)
{
    return (block << 6) | (page & 0x3F);
}

static inline uint32_t nandflash_block_page_offset_to_col_addr(enum HAL_FLASH_ID_T id, uint32_t block, uint32_t offset)
{
    uint8_t plane_num = (flash_list[flash_idx[id]]->plane_cfg >> 4) & 0xF;
    uint8_t plane_offset = (flash_list[flash_idx[id]]->plane_cfg) & 0xF;

    if (plane_num > 1)
        return ((block & (plane_num - 1)) << plane_offset) | offset;
    else
        return offset;
}

static inline uint32_t nandflash_block_to_die(enum HAL_FLASH_ID_T id, uint32_t block)
{
    uint8_t plane_num = (flash_list[flash_idx[id]]->plane_cfg >> 4) & 0xF;

    if (plane_num > 1) { //Dosilicon 2G/4G multi plane
        return (block & (plane_num - 1));
    } else if (nandflash_ctx[id].blocks_per_die) { //Winbond multi die
        return block / nandflash_ctx[id].blocks_per_die;
    } else {
        return 0;
    }
}

#if (defined(FLASH_DMA) && !defined(FLASH_DMA_RX_M2M)) || defined(BUFFER_READ)
static void nandflash_read_from_cache(enum HAL_FLASH_ID_T id, uint32_t offset, uint32_t len)
{
    // Switch to traditional command format
    norflaship_cmd_done(id);

    norflaship_blksize(id, len);

    if (nandflash_op_mode[id] & NANDFLASH_OP_MODE_DTR) {
        /* Quad DTR, only fast */
        norflaship_cmd_addr(id, GD5F4GQ4_CMD_FAST_QUAD_DTR_READ, offset);
    } else if (nandflash_op_mode[id] & NANDFLASH_OP_MODE_QUAD_IO) {
        /* Quad , only fast */
        norflaship_cmd_addr(id, GD5F4GQ4_CMD_FAST_QUAD_IO_READ, offset);
    } else if (nandflash_op_mode[id] & NANDFLASH_OP_MODE_QUAD_OUTPUT) {
        /* Dual, only fast */
        norflaship_cmd_addr(id, GD5F4GQ4_CMD_FAST_QUAD_OUTPUT_READ, offset);
    } else if (nandflash_op_mode[id] & NANDFLASH_OP_MODE_DUAL_IO) {
        /* Dual, only fast */
        norflaship_cmd_addr(id, GD5F4GQ4_CMD_FAST_DUAL_IO_READ, offset);
    } else if (nandflash_op_mode[id] & NANDFLASH_OP_MODE_DUAL_OUTPUT) {
        /* Dual, only fast */
        norflaship_cmd_addr(id, GD5F4GQ4_CMD_FAST_DUAL_OUTPUT_READ, offset);
    } else if (nandflash_op_mode[id] & NANDFLASH_OP_MODE_FAST_SPI){
        /* fast */
        norflaship_cmd_addr(id, GD5F4GQ4_CMD_STANDARD_FAST_READ, offset);
    } else {
        /* normal */
        norflaship_cmd_addr(id, GD5F4GQ4_CMD_STANDARD_READ, offset);
    }
}
#endif

#ifdef FLASH_DMA
#ifdef FLASH_DMA_RX_M2M
static int nandflash_start_rx_dma_m2m(enum HAL_FLASH_ID_T id, uint32_t dst, uint32_t src, uint32_t len)
{
    enum HAL_DMA_PERIPH_T periph;
    enum HAL_DMA_WIDTH_T dst_width;
    enum HAL_DMA_WIDTH_T src_width;
    enum HAL_DMA_BSIZE_T dst_bsize;
    enum HAL_DMA_BSIZE_T src_bsize;
    enum HAL_DMA_RET_T ret;
    uint8_t ch;
    uint8_t src_div;
    uint32_t total_xfer;
    uint32_t xfer;
    uint32_t remains;

    if (len < 64) {
        memcpy((void *)dst, (void *)src, len);
        return 0;
    }

    periph = HAL_GPDMA_MEM;
    ch = hal_gpdma_get_chan(periph, HAL_DMA_LOW_PRIO);
    if (ch == HAL_DMA_CHAN_NONE) {
        memcpy((void *)dst, (void *)src, len);
        return 0;
    }

    if ((dst & 0x3) == (src & 0x3)) {
        while (dst & 0x3) {
            *(uint8_t *)dst++ = *(uint8_t *)src++;
            len--;
        }
    } else if ((dst & 0x1) == (src & 0x1)) {
        if (dst & 0x1) {
            *(uint8_t *)dst++ = *(uint8_t *)src++;
            len--;
        }
    }

    if (dst & 0x1) {
        dst_width = HAL_DMA_WIDTH_BYTE;
        dst_bsize = HAL_DMA_BSIZE_16;
    } else if (dst & 0x3) {
        dst_width = HAL_DMA_WIDTH_HALFWORD;
        dst_bsize = HAL_DMA_BSIZE_8;
    } else {
        dst_width = HAL_DMA_WIDTH_WORD;
        dst_bsize = HAL_DMA_BSIZE_4;
    }

    if (src & 0x1) {
        src_width = HAL_DMA_WIDTH_BYTE;
        src_bsize = HAL_DMA_BSIZE_16;
        src_div = 1;
    } else if (src & 0x3) {
        src_width = HAL_DMA_WIDTH_HALFWORD;
        src_bsize = HAL_DMA_BSIZE_8;
        src_div = 2;
    } else {
        src_width = HAL_DMA_WIDTH_WORD;
        src_bsize = HAL_DMA_BSIZE_4;
        src_div = 4;
    }

    if (dst_width == HAL_DMA_WIDTH_WORD || src_width == HAL_DMA_WIDTH_WORD) {
        remains = (len & 0x3);
        total_xfer = (len - remains) / src_div;
    } else if (dst_width == HAL_DMA_WIDTH_HALFWORD || src_width == HAL_DMA_WIDTH_HALFWORD) {
        remains = (len & 0x1);
        total_xfer = (len - remains) / src_div;
    } else {
        remains = 0;
        total_xfer = len / src_div;
    }

    memset(&dma_cfg[id], 0, sizeof(dma_cfg[id]));
    dma_cfg[id].handler = NULL;
    dma_cfg[id].dst = dst;
    dma_cfg[id].dst_bsize = dst_bsize;
    dma_cfg[id].dst_periph = 0;
    dma_cfg[id].dst_width = dst_width;
    dma_cfg[id].src = src;
    dma_cfg[id].src_bsize = src_bsize;
    dma_cfg[id].src_periph = periph;
    dma_cfg[id].src_width = src_width;
    dma_cfg[id].type = HAL_DMA_FLOW_M2M_DMA;
    dma_cfg[id].try_burst = 0;
    dma_cfg[id].ch = ch;

    xfer = (HAL_DMA_MAX_DESC_XFER_SIZE & ~0x3) / src_div;
    if (xfer < total_xfer) {
        dma_cfg[id].src_tsize = xfer;
        ret = hal_dma_init_desc(&dma_desc[id][0], &dma_cfg[id], &dma_desc[id][1], 0);
        if (ret != HAL_DMA_OK) {
            ASSERT(false, "%s Failed to init dma desc: %d", __func__, ret);
        }
        dma_cfg[id].src += xfer * src_div;
        dma_cfg[id].dst += xfer * src_div;
        dma_cfg[id].src_tsize = total_xfer - xfer;
        ret = hal_dma_init_desc(&dma_desc[id][1], &dma_cfg[id], NULL, 0);
    } else {
        dma_cfg[id].src_tsize = total_xfer;
        ret = hal_dma_init_desc(&dma_desc[id][0], &dma_cfg[id], NULL, 0);
    }
    if (ret != HAL_DMA_OK) {
        ASSERT(false, "%s Failed to init dma desc: %d", __func__, ret);
    }

    ret = hal_dma_sg_start(&dma_desc[id][0], &dma_cfg[id]);
    if (ret != HAL_DMA_OK) {
        ASSERT(false, "%s: Failed to start dma: %d", __func__, ret);
    }

    while (hal_dma_chan_busy(dma_cfg[id].ch) && TASK_SCHEDULE);

    hal_dma_free_chan(dma_cfg[id].ch);
    dma_cfg[id].ch = HAL_DMA_CHAN_NONE;

    dst += (len - remains);
    src += (len - remains);
    while (remains) {
        *(uint8_t *)dst++ = *(uint8_t *)src++;
        remains--;
    }

    return 0;
}
#else // !FLASH_DMA_RX_M2M
static uint32_t nandflash_start_rx_dma(enum HAL_FLASH_ID_T id, uint8_t *buf, uint32_t len)
{
    enum HAL_DMA_PERIPH_T periph;
    enum HAL_DMA_WIDTH_T dst_width;
    enum HAL_DMA_BSIZE_T dst_bsize;
    enum HAL_DMA_RET_T ret;
    uint8_t ch;
    uint32_t xfer;

    if (len < 64) {
        return len;
    }

    norflaship_clear_rxfifo(id);

    if (0) {
#ifdef FLASH1_CTRL_BASE
    } else if (id == HAL_FLASH_ID_1) {
        periph = HAL_GPDMA_FLASH1;
#endif
#ifdef FLASH2_CTRL_BASE
    } else if (id == HAL_FLASH_ID_2) {
        periph = HAL_GPDMA_FLASH2;
#endif
    } else {
        periph = HAL_GPDMA_FLASH0;
    }

    ch = hal_gpdma_get_chan(periph, HAL_DMA_HIGH_PRIO);
    if (ch == HAL_DMA_CHAN_NONE) {
        return len;
    }

    norflaship_dmactrl_rx_en(id, true);
    norflaship_busy_wait(id);

    if ((uint32_t)buf & 0x1) {
        dst_width = HAL_DMA_WIDTH_BYTE;
        dst_bsize = HAL_DMA_BSIZE_16;
    } else if ((uint32_t)buf & 0x3) {
        dst_width = HAL_DMA_WIDTH_HALFWORD;
        dst_bsize = HAL_DMA_BSIZE_8;
    } else {
        dst_width = HAL_DMA_WIDTH_WORD;
        dst_bsize = HAL_DMA_BSIZE_4;
    }

    memset(&dma_cfg[id], 0, sizeof(dma_cfg[id]));
    dma_cfg[id].handler = NULL;
    dma_cfg[id].dst = (uint32_t)buf;
    dma_cfg[id].dst_bsize = dst_bsize;
    dma_cfg[id].dst_periph = 0;
    dma_cfg[id].dst_width = dst_width;
    dma_cfg[id].src = 0;
    dma_cfg[id].src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg[id].src_periph = periph;
    dma_cfg[id].src_width = HAL_DMA_WIDTH_WORD;
    dma_cfg[id].type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg[id].try_burst = 0;
    dma_cfg[id].ch = ch;

    xfer = (len & ~0x3);
    dma_cfg[id].src_tsize = xfer / 4;

    ret = hal_dma_start(&dma_cfg[id]);
    if (ret != HAL_DMA_OK) {
        ASSERT(false, "%s: Failed to start dma: %d", __func__, ret);
    }

    return len - xfer;
}
#endif // !FLASH_DMA_RX_M2M

static uint32_t nandflash_start_tx_dma(enum HAL_FLASH_ID_T id, const uint8_t *buf, uint32_t len)
{
    enum HAL_DMA_PERIPH_T periph;
    enum HAL_DMA_WIDTH_T src_width;
    enum HAL_DMA_BSIZE_T src_bsize;
    enum HAL_DMA_RET_T ret;
    uint8_t ch;
    uint8_t src_div;
    uint32_t total_xfer;
    uint32_t xfer;

    if (len < 64) {
        return len;
    }

    if (0) {
#ifdef FLASH1_CTRL_BASE
    } else if (id == HAL_FLASH_ID_1) {
        periph = HAL_GPDMA_FLASH1;
#endif
#ifdef FLASH2_CTRL_BASE
    } else if (id == HAL_FLASH_ID_2) {
        periph = HAL_GPDMA_FLASH2;
#endif
    } else {
        periph = HAL_GPDMA_FLASH0;
    }

    ch = hal_gpdma_get_chan(periph, HAL_DMA_HIGH_PRIO);
    if (ch == HAL_DMA_CHAN_NONE) {
        return len;
    }

    norflaship_dmactrl_tx_en(id, true);
    norflaship_blksize(id, len);
    norflaship_busy_wait(id);

    if ((uint32_t)buf & 0x1) {
        src_width = HAL_DMA_WIDTH_BYTE;
        src_bsize = HAL_DMA_BSIZE_16;
        src_div = 1;
    } else if ((uint32_t)buf & 0x3) {
        src_width = HAL_DMA_WIDTH_HALFWORD;
        src_bsize = HAL_DMA_BSIZE_8;
        src_div = 2;
    } else {
        src_width = HAL_DMA_WIDTH_WORD;
        src_bsize = HAL_DMA_BSIZE_4;
        src_div = 4;
    }

    memset(&dma_cfg[id], 0, sizeof(dma_cfg[id]));
    dma_cfg[id].handler = NULL;
    dma_cfg[id].dst = 0;
    dma_cfg[id].dst_bsize = HAL_DMA_BSIZE_8;
    dma_cfg[id].dst_periph = periph;
    dma_cfg[id].dst_width = HAL_DMA_WIDTH_WORD;
    dma_cfg[id].src = (uint32_t)buf;
    dma_cfg[id].src_bsize = src_bsize;
    dma_cfg[id].src_periph = 0;
    dma_cfg[id].src_width = src_width;
    dma_cfg[id].type = HAL_DMA_FLOW_M2P_DMA;
    dma_cfg[id].try_burst = 0;
    dma_cfg[id].ch = ch;

    total_xfer = (len & ~0x3) / src_div;
    xfer = (HAL_DMA_MAX_DESC_XFER_SIZE & ~0x3) / src_div;
    if (xfer < total_xfer) {
        dma_cfg[id].src_tsize = xfer;
        ret = hal_dma_init_desc(&dma_desc[id][0], &dma_cfg[id], &dma_desc[id][1], 0);
        if (ret != HAL_DMA_OK) {
            ASSERT(false, "%s Failed to init dma desc: %d", __func__, ret);
        }
        dma_cfg[id].src += xfer * src_div;
        dma_cfg[id].src_tsize = total_xfer - xfer;
        ret = hal_dma_init_desc(&dma_desc[id][1], &dma_cfg[id], NULL, 0);
    } else {
        dma_cfg[id].src_tsize = total_xfer;
        ret = hal_dma_init_desc(&dma_desc[id][0], &dma_cfg[id], NULL, 0);
    }
    if (ret != HAL_DMA_OK) {
        ASSERT(false, "%s Failed to init dma desc: %d", __func__, ret);
    }

    ret = hal_dma_sg_start(&dma_desc[id][0], &dma_cfg[id]);
    if (ret != HAL_DMA_OK) {
        ASSERT(false, "%s: Failed to start dma: %d", __func__, ret);
    }

    return len - total_xfer * src_div;
}

static void nandflash_dma_busy_wait(enum HAL_FLASH_ID_T id)
{
    if (dma_cfg[id].ch == HAL_DMA_CHAN_NONE) {
        return;
    }
    while (hal_dma_chan_busy(dma_cfg[id].ch) && TASK_SCHEDULE);

    hal_dma_free_chan(dma_cfg[id].ch);
    dma_cfg[id].ch = HAL_DMA_CHAN_NONE;

    //norflaship_busy_wait(id);
    if (dma_cfg[id].type == HAL_DMA_FLOW_P2M_DMA) {
#if 0
        // Workaround for extra reads?
        while (norflaship_is_busy(id) || !norflaship_rxfifo_is_empty(id)) {
            norflaship_rxfifo_empty_wait(id);
            norflaship_read_rxfifo(id);
        }
#endif
        norflaship_dmactrl_rx_en(id, false);
    } else {
        norflaship_dmactrl_tx_en(id, false);
    }
    //norflaship_busy_wait(id);
}
#endif // FLASH_DMA

POSSIBLY_UNUSED
static int nandflash_read_reg(enum HAL_FLASH_ID_T id, uint8_t cmd, uint8_t *val, uint32_t len)
{
    int i;

    norflaship_clear_fifos(id);
    norflaship_ext_rx_cmd(id, cmd, 0, len);
    norflaship_rxfifo_count_wait(id, len);
    for (i = 0; i < len; i++) {
        val[i] = norflaship_read_rxfifo(id);
    }
    norflaship_cmd_done(id);

    return 0;
}

static int nandflash_read_reg_ex(enum HAL_FLASH_ID_T id, uint8_t cmd, uint8_t *param, uint32_t param_len, uint8_t *val, uint32_t len)
{
    int i;

    norflaship_clear_fifos(id);
    if (param && param_len > 0) {
        norflaship_write_txfifo(id, param, param_len);
    } else {
        param_len = 0;
    }
    norflaship_ext_rx_cmd(id, cmd, param_len, len);
    for (i = 0; i < len; i++) {
        norflaship_rxfifo_empty_wait(id);
        val[i] = norflaship_read_rxfifo(id);
    }
    norflaship_cmd_done(id);

    return 0;
}

static uint8_t nandflash_read_statusid(enum HAL_FLASH_ID_T id)
{
    uint8_t val;
    uint8_t param;

    param = GD5F4GQ4_REG_STATUS;
    nandflash_read_reg_ex(id, GD5F4GQ4_CMD_GET_FEATURE, &param, 1, &val, 1);
    return val;
}

static uint8_t nandflash_read_statusid_ext(enum HAL_FLASH_ID_T id, uint8_t param)
{
    uint8_t val;

    nandflash_read_reg_ex(id, GD5F4GQ4_CMD_GET_FEATURE, &param, 1, &val, 1);
    return val;
}

static int nandflash_status_WEL(enum HAL_FLASH_ID_T id)
{
    uint32_t status;
    status = nandflash_read_statusid(id);
    return !!(status & GD5F4GQ4_ST_WEL);
}

static int nandflash_status_OIP(enum HAL_FLASH_ID_T id)
{
    uint32_t status;
    status = nandflash_read_statusid(id);
    return !!(status & GD5F4GQ4_ST_OIP);
}

POSSIBLY_UNUSED
static void nandflash_status_WEL_0_wait(enum HAL_FLASH_ID_T id)
{
    while (nandflash_status_WEL(id) == 0 && TASK_SCHEDULE);
}

static void nandflash_status_OIP_1_waitid(enum HAL_FLASH_ID_T id)
{
    while (nandflash_status_OIP(id) && TASK_SCHEDULE);
}

static int nandflash_write_reg(enum HAL_FLASH_ID_T id, uint8_t cmd, const uint8_t *val, uint32_t len)
{
    norflaship_clear_txfifo(id);
    norflaship_write_txfifo(id, val, len);

    norflaship_ext_tx_cmd(id, cmd, len);

    nandflash_status_OIP_1_waitid(id);
    norflaship_cmd_done(id);

    return 0;
}

static int nandflash_read_device_id(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len)
{
    uint8_t id_addr;

    if (len > NANDFLASH_ID_LEN) {
        len = NANDFLASH_ID_LEN;
    }
    id_addr = 0x00;
    nandflash_read_reg_ex(id, GD5F4GQ4_CMD_ID, &id_addr, 1, value, len);
    norflaship_cmd_done(id);

    return 0;
}

static uint8_t nandflash_read_protection(enum HAL_FLASH_ID_T id)
{
    uint8_t val;
    uint8_t param;

    param = GD5F4GQ4_REG_PROTECTION;
    nandflash_read_reg_ex(id, GD5F4GQ4_CMD_GET_FEATURE, &param, 1, &val, 1);
    return val;
}

static void nandflash_write_protection(enum HAL_FLASH_ID_T id, uint8_t val)
{
    uint8_t param[2];

    param[0] = GD5F4GQ4_REG_PROTECTION;
    param[1] = val;
    nandflash_write_reg(id, GD5F4GQ4_CMD_SET_FEATURE, &param[0], 2);
}

static uint8_t nandflash_read_configuration(enum HAL_FLASH_ID_T id)
{
    uint8_t val;
    uint8_t param;

    param = GD5F4GQ4_REG_CONFIGURATION;
    nandflash_read_reg_ex(id, GD5F4GQ4_CMD_GET_FEATURE, &param, 1, &val, 1);
    return val;
}

static void nandflash_write_configuration(enum HAL_FLASH_ID_T id, uint8_t val)
{
    uint8_t param[2];

    param[0] = GD5F4GQ4_REG_CONFIGURATION;
    param[1] = val;
    nandflash_write_reg(id, GD5F4GQ4_CMD_SET_FEATURE, &param[0], 2);
}

static void nandflash_write_ext_eccs_st_bfd(enum HAL_FLASH_ID_T id, uint8_t val)
{
    uint8_t param[2];

    param[0] = W25N02KW_REG_ECC_BFD;
    param[1] = ((val << 4) & 0xFF);
    nandflash_write_reg(id, GD5F4GQ4_CMD_SET_FEATURE, &param[0], 2);
}

static enum NANDFLASH_RET_T nandflash_check_state(enum HAL_FLASH_ID_T id)
{
    if (!nandflash_ctx[id].opened) {
        return NANDFLASH_RET_NOT_OPENED;
    }
#ifdef NANDFLASH_DELAY_WAKEUP
    if (nandflash_ctx[id].in_sleep) {
        nandflash_wakeup(id, nandflash_ctx[id].sleep_mode);
    }
#endif
    return NANDFLASH_RET_OK;
}

void nandflash_dump_features(enum HAL_FLASH_ID_T id)
{
    uint8_t val;

    if (nandflash_check_state(id) != NANDFLASH_RET_OK) {
        NORFLASH_TRACE(1, "%s", err_not_opened);
        return;
    }

    NORFLASH_TRACE(1, "Mode: cur=0x%04X supported=0x%04X", nandflash_op_mode[id], flash_list[flash_idx[id]]->mode);
    val = nandflash_read_protection(id);
    NORFLASH_TRACE(1, "Protection: 0x%02X", val);
    val = nandflash_read_configuration(id);
    NORFLASH_TRACE(1, "Configuration: 0x%02X", val);
    val = nandflash_read_statusid(id);
    NORFLASH_TRACE(1, "Status: 0x%02X", val);
}

static int nandflash_set_quad(enum HAL_FLASH_ID_T id, uint8_t on)
{
    uint8_t cfg;
    uint32_t mode;

    mode = flash_list[flash_idx[id]]->mode;
    if (mode & NANDFLASH_OP_MODE_NO_QE_BIT) {
        return 0;
    }

    cfg = nandflash_read_configuration(id);
    if (on) {
        cfg |= GD5F4GQ4_CFG_QE;
    } else {
        cfg &= ~GD5F4GQ4_CFG_QE;
    }
    nandflash_write_configuration(id, cfg);
    return 0;
}

static int nandflash_set_quad_io_mode(enum HAL_FLASH_ID_T id, uint8_t on)
{
    nandflash_set_quad(id, on);
    if (on) {
        norflaship_quad_mode(id, 1);
    } else {
        norflaship_quad_mode(id, 0);
    }
    norflaship_busy_wait(id);
    return 0;
}

static int nandflash_set_quad_output_mode(enum HAL_FLASH_ID_T id, uint8_t on)
{
    nandflash_set_quad(id, on);
    if (on) {
        norflaship_rdcmd(id, GD5F4GQ4_CMD_FAST_QUAD_OUTPUT_READ);
    } else {
        norflaship_rdcmd(id, GD5F4GQ4_CMD_STANDARD_READ);
    }
    norflaship_busy_wait(id);
    return 0;
}

static uint8_t nandflash_set_dual_io_mode(enum HAL_FLASH_ID_T id, uint8_t on)
{
    if (on) {
        norflaship_dual_mode(id, 1);
    } else {
        norflaship_dual_mode(id, 0);
    }
    norflaship_busy_wait(id);

    return 0;
}

static uint8_t nandflash_set_dual_output_mode(enum HAL_FLASH_ID_T id, uint8_t on)
{
    if (on) {
        norflaship_rdcmd(id, GD5F4GQ4_CMD_FAST_DUAL_OUTPUT_READ);
    } else {
        norflaship_rdcmd(id, GD5F4GQ4_CMD_STANDARD_READ);
    }
    norflaship_busy_wait(id);
    return 0;
}

static uint8_t nandflash_set_fast_mode(enum HAL_FLASH_ID_T id, uint8_t on)
{
    if (on) {
        norflaship_rdcmd(id, GD5F4GQ4_CMD_STANDARD_FAST_READ);
    } else {
        norflaship_rdcmd(id, GD5F4GQ4_CMD_STANDARD_READ);
    }
    norflaship_busy_wait(id);
    return 0;
}

static uint8_t nandflash_set_stand_mode(enum HAL_FLASH_ID_T id)
{
    norflaship_rdcmd(id, GD5F4GQ4_CMD_STANDARD_READ);
    norflaship_busy_wait(id);
    return 0;
}

static uint32_t nandflash_get_supported_mode(enum HAL_FLASH_ID_T id)
{
     return flash_list[flash_idx[id]]->mode;
}

static uint32_t nandflash_get_block_protect_mask(enum HAL_FLASH_ID_T id)
{
    return flash_list[flash_idx[id]]->block_protect_mask;
}

static uint32_t nandflash_get_ext_eccs_st_bfd(enum HAL_FLASH_ID_T id)
{
    return flash_list[flash_idx[id]]->ecc_cfg.st_bfd;
}

static void nandflash_release_from_dp(enum HAL_FLASH_ID_T id)
{
    norflaship_ext_tx_cmd(id, GD5F4GQ4_CMD_RELEASE_FROM_DP, 0);
    norflaship_busy_wait(id);
    // Wait 20us for flash to finish
    nandflash_delay(40);
}

static void nandflash_reset(enum HAL_FLASH_ID_T id)
{
    norflaship_clear_fifos(id);

#ifdef FLASH_RESET_PIN
    norflaship_reset_pin(id, true);
    // Wait at least 1us
    nandflash_delay(20);
    norflaship_reset_pin(id, false);
#endif

    // Release from deep power-down
#ifdef FLASH_QPI
    norflaship_qpi_mode(id, 1);
    nandflash_release_from_dp(id);
    norflaship_qpi_mode(id, 0);
#endif
    nandflash_release_from_dp(id);

    // Quit from quad/dual continuous read mode
    norflaship_quad_mode(id, 0);
    norflaship_dual_mode(id, 0);
    norflaship_busy_wait(id);

    // Software reset
    norflaship_ext_tx_cmd(id, GD5F4GQ4_CMD_RESET, 0);
    // Reset recovery time: 5~500 us
    nandflash_delay(500);

    norflaship_cmd_done(id);

    // Reset cfg
    flash_idx[id] = 0;
    div_read[id] = 0;
    nandflash_op_mode[id] = 0;
    falling_edge_adj[id] = false;
}

static void nandflash_software_die_select(enum HAL_FLASH_ID_T id, uint8_t die_id)
{
    norflaship_clear_txfifo(id);
    norflaship_write_txfifo(id, &die_id, 1);
    norflaship_ext_tx_cmd(id, W25M02GV_CMD_DIE_SEL, 1);
    norflaship_cmd_done(id);
}

static void nandflash_select_die(enum HAL_FLASH_ID_T id, uint32_t block)
{
    uint32_t die;

    if (nandflash_ctx[id].blocks_per_die == 0) {
        return;
    }
    if (nandflash_ctx[id].cur_block == block) {
        return;
    }
    nandflash_ctx[id].cur_block = block;
    die = block / nandflash_ctx[id].blocks_per_die;
    if (nandflash_ctx[id].cur_die == die) {
        return;
    }
    nandflash_ctx[id].cur_die = die;

    nandflash_software_die_select(id, die);
}

#if (CHIP_FLASH_CTRL_VER >= 5) && defined(FLASH_DTR)
static int nandflash_set_dtr_quad_mode(enum HAL_FLASH_ID_T id, uint8_t on)
{
    uint8_t cmd;
    if (on) {
        norflaship_dummy_n_rcqio_dtr(id, 8);
        cmd = GD5F4GQ4_CMD_FAST_QUAD_DTR_READ;
    } else {
        cmd = GD5F4GQ4_CMD_FAST_QUAD_IO_READ;
    }
    norflaship_qrdcmd(id, cmd);
    norflaship_dtr_mode(id, on);
    norflaship_busy_wait(id);
    return 0;
}
#endif

static int nandflash_set_mode(enum HAL_FLASH_ID_T id, uint32_t op)
{
    uint32_t read_mode = 0;
    uint32_t ext_mode = 0;
    uint32_t program_mode = 0;
    uint32_t self_mode;
    uint32_t mode;
    uint8_t cfg;

    self_mode = nandflash_get_supported_mode(id);
    mode = (self_mode & op);

    if (mode & NANDFLASH_OP_MODE_QUAD_IO) {
        read_mode |= NANDFLASH_OP_MODE_QUAD_IO;
        if (mode & NANDFLASH_OP_MODE_DTR) {
            read_mode |= NANDFLASH_OP_MODE_DTR;
        }
    } else if (mode & NANDFLASH_OP_MODE_QUAD_OUTPUT) {
        read_mode |= NANDFLASH_OP_MODE_QUAD_OUTPUT;
    } else if (mode & NANDFLASH_OP_MODE_DUAL_IO) {
        read_mode |= NANDFLASH_OP_MODE_DUAL_IO;
    } else if (mode & NANDFLASH_OP_MODE_DUAL_OUTPUT) {
        read_mode |= NANDFLASH_OP_MODE_DUAL_OUTPUT;
    } else if (mode & NANDFLASH_OP_MODE_FAST_SPI) {
        read_mode |= NANDFLASH_OP_MODE_FAST_SPI;
    } else if(mode & NANDFLASH_OP_MODE_STAND_SPI) {
        read_mode |= NANDFLASH_OP_MODE_STAND_SPI;
    } else {
        // Op error
        return  1;
    }

    if (mode & NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM) {
        program_mode |= NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM;
    } else if (mode & NANDFLASH_OP_MODE_PAGE_PROGRAM) {
        program_mode |= NANDFLASH_OP_MODE_PAGE_PROGRAM;
    } else {
        // Op error
        return 1;
    }

    if (mode & NANDFLASH_OP_MODE_INTERNAL_ECC) {
        ext_mode |= NANDFLASH_OP_MODE_INTERNAL_ECC;
    }
    if (self_mode & NANDFLASH_OP_MODE_READ_ADDR_24BIT) {
        ext_mode |= NANDFLASH_OP_MODE_READ_ADDR_24BIT;
    }

    mode = (read_mode | ext_mode | program_mode);

    if (nandflash_op_mode[id] != mode) {
#if (CHIP_FLASH_CTRL_VER >= 5) && defined(FLASH_DTR)
        nandflash_set_dtr_quad_mode(id, 0);
#endif
        norflaship_quad_mode(id, 0);

        norflaship_dual_mode(id, 0);

        if (mode & NANDFLASH_OP_MODE_QUAD_IO) {
            // Quad io mode
            nandflash_set_quad_io_mode(id, 1);
#if (CHIP_FLASH_CTRL_VER >= 5) && defined(FLASH_DTR)
            if (mode & NANDFLASH_OP_MODE_DTR) {
                // Quad dtr read mode on
                nandflash_set_dtr_quad_mode(id, 1);
            }
#endif
        } else if (mode & NANDFLASH_OP_MODE_QUAD_OUTPUT) {
            // Quad output mode
            nandflash_set_quad_output_mode(id, 1);
        } else if (mode & NANDFLASH_OP_MODE_DUAL_IO) {
            // Dual io mode
            nandflash_set_dual_io_mode(id, 1);
        } else if (mode & NANDFLASH_OP_MODE_DUAL_OUTPUT) {
            // Dual output mode
            nandflash_set_dual_output_mode(id, 1);
        } else if (mode & NANDFLASH_OP_MODE_FAST_SPI) {
            // Fast mode
            nandflash_set_fast_mode(id, 1);
        } else if (mode & NANDFLASH_OP_MODE_STAND_SPI) {
            // Standard spi mode
            nandflash_set_stand_mode(id);
        }

        cfg = nandflash_read_configuration(id);
        if (mode & NANDFLASH_OP_MODE_INTERNAL_ECC) {
            cfg |= GD5F4GQ4_CFG_ECC_EN;
        } else {
            cfg &= ~GD5F4GQ4_CFG_ECC_EN;
        }
        nandflash_write_configuration(id, cfg);

        if (mode & NANDFLASH_OP_MODE_READ_ADDR_24BIT) {
            norflaship_nand_addr_byte_num(id, 2);
        } else {
            // Address 16BIT
            norflaship_nand_addr_byte_num(id, 1);
        }

        norflaship_cmd_done(id);

        // Update current mode at last
        nandflash_op_mode[id] = mode;
    }

    return 0;
}

static void nandflash_get_samdly_list(uint32_t div, const uint8_t **samdly_list_p, uint32_t *size_p)
{
    const uint8_t *samdly_list = NULL;
    uint32_t size = 0;

    if (div >= 1) {
        if (div == 1) {
            samdly_list = samdly_list_div1;
            size = ARRAY_SIZE(samdly_list_div1);
        } else {
            samdly_list = samdly_list_divn;
            size = ARRAY_SIZE(samdly_list_divn);
        }
    }

    if (samdly_list_p) {
        *samdly_list_p = samdly_list;
    }
    if (size_p) {
        *size_p = size;
    }
}

static void nandflash_set_sample_delay_index(enum HAL_FLASH_ID_T id, uint32_t index)
{
    const uint8_t *samdly_list;
    uint32_t size;
    uint32_t div;

    sample_delay_index[id] = index;

    div = norflaship_get_div(id);

    nandflash_get_samdly_list(div, &samdly_list, &size);

    if (index < size) {
        norflaship_samdly(id, samdly_list[index]);
    }
}

static bool nandflash_calib_flash_id_valid(enum HAL_FLASH_ID_T id)
{
    uint8_t flash_id[NANDFLASH_ID_LEN];
    const uint8_t *cmp_id;

    nandflash_read_device_id(id, flash_id, sizeof(flash_id));
    cmp_id = flash_list[flash_idx[id]]->id;

    if (flash_id[0] == cmp_id[0] && flash_id[1] == cmp_id[1] && flash_id[2] == cmp_id[2]) {
        return true;
    }
    return false;
}

static bool nandflash_calib_magic_word_valid(enum HAL_FLASH_ID_T id)
{
    uint32_t magic = 0;

#ifdef BUFFER_READ
    uint32_t len = 4;
    norflaship_clear_rxfifo(id);
    nandflash_read_from_cache(id, 0, len);
    while (len--) {
        norflaship_rxfifo_empty_wait(id);

        magic = (magic << 8) | norflaship_read_rxfifo(id);
    }
#else
    norflaship_clear_rxfifo(id);
    magic = *(volatile uint32_t *)nand_nc_base[id];
#endif
    if (magic == BOOT_MAGIC_NUMBER) {
        return true;
    }
    return false;
}

static enum NANDFLASH_RET_T nandflash_start_page_read(enum HAL_FLASH_ID_T id, uint32_t row_addr)
{
    enum NANDFLASH_ECC_ST_TYPE_T st_type;
    uint8_t ecc10_reg = 0, ecc32_reg = 0;
    uint8_t status10 = 0, status32 = 0;
    bool no_err;

    norflaship_cmd_addr(id, GD5F4GQ4_CMD_PAGE_READ, row_addr);
    norflaship_busy_wait(id);
    nandflash_status_OIP_1_waitid(id);

    st_type = flash_list[flash_idx[id]]->ecc_cfg.st_type;
    ASSERT(st_type < NANDFLASH_ECC_ST_TYPE_QTY, "%s st_type error: 0x%x",
           __func__, st_type);

    nandflash_ecc_get_eccs(st_type, &ecc10_reg, &ecc32_reg);
    status10 = nandflash_read_statusid_ext(id, ecc10_reg);
    if (ecc10_reg != ecc32_reg && ecc32_reg != 0) {
        status32 = nandflash_read_statusid_ext(id, ecc32_reg);
    }

    no_err = nandflash_ecc_check(st_type, status10, status32, flash_list[flash_idx[id]]->ecc_cfg.st_bad);
    if (!no_err) {
        return NANDFLASH_RET_READ_ERR;
    }

    return NANDFLASH_RET_OK;
}

static bool nandflash_dtr_calib_valid(enum HAL_FLASH_ID_T id)
{
#if defined(FLASH_DTR_CALIB_BLOCK) && defined(FLASH_DTR_CALIB_PAGE)
    uint32_t calib_read = 0;
    uint32_t index = 0;
    uint32_t row_addr = 0;
    volatile uint32_t *nand_base = (volatile uint32_t *)nand_nc_base[id];

    nandflash_select_die(id, FLASH_DTR_CALIB_BLOCK);
    row_addr = nandflash_block_page_to_row_addr(FLASH_DTR_CALIB_BLOCK, FLASH_DTR_CALIB_PAGE);
    nandflash_start_page_read(id, row_addr);
    norflaship_cmd_done(id);
    for (uint32_t i = 0; i < 0x20; i++) {
        index = (i & 0xFF) | (((~i) & 0xFF) << 8) | (((~i) & 0xFF) << 16) | ((i & 0xFF) << 24);
        calib_read = *nand_base++;
        if (calib_read != index) {
            return false;
        }
    }
#else
    ASSERT(0, "Please write the DTR mode calibration data (32-bit pattern) into the corresponding area of the NAND flash under other operating modes.");
#endif
    return true;
}

static int nandflash_sample_delay_calib(enum HAL_FLASH_ID_T id, enum NANDFLASH_CALIB_T type)
{
    int i;
    uint32_t matched_cnt = 0;
    uint32_t matched_idx = 0;
    uint32_t div;
    uint32_t size;
    bool valid;

    if (type >= NANDFLASH_CALIB_QTY) {
        return 1;
    }
#if defined(ROM_BUILD) || defined(PROGRAMMER)
    if (type != NANDFLASH_CALIB_FLASH_ID) {
        return 0;
    }
#endif

    div = norflaship_get_div(id);

    if (div == 0) {
        return -1;
    }

    nandflash_get_samdly_list(div, NULL, &size);

    for (i = 0; i < size; i++) {
        norflaship_busy_wait(id);

        nandflash_set_sample_delay_index(id, i);

        if (type == NANDFLASH_CALIB_FLASH_ID) {
            valid = nandflash_calib_flash_id_valid(id);
        } else if (type == NANDFLASH_CALIB_DTR) {
            valid = nandflash_dtr_calib_valid(id);
        } else {
            valid = nandflash_calib_magic_word_valid(id);
        }

        if (valid) {
            if (matched_cnt == 0) {
                matched_idx = i;
            }
            matched_cnt++;
        } else if (matched_cnt) {
            break;
        }
    }

#ifdef FLASH_CALIB_DEBUG
    calib_matched_idx[type] = matched_idx;
    calib_matched_cnt[type] = matched_cnt;
#endif

    if (matched_cnt) {
#ifdef SAMP_DELAY_PRIO_FALLING_EDGE
        if (matched_cnt == 2) {
            uint32_t falling_edge_idx;

            if (0) {
            } else if (div == 1) {
                falling_edge_idx = DIV1_SAMP_DELAY_FALLING_EDGE_IDX;
            } else if (div == 2) {
                falling_edge_idx = DIV2_SAMP_DELAY_FALLING_EDGE_IDX;
                if (falling_edge_adj[id]) {
                    falling_edge_idx++;
                }
            } else {
                falling_edge_idx = DIVN_SAMP_DELAY_FALLING_EDGE_IDX;
            }
            if (matched_idx <= falling_edge_idx &&
                    falling_edge_idx < matched_idx + matched_cnt) {
                matched_idx = falling_edge_idx;
                matched_cnt = 1;
            }
        }
#endif
        matched_idx += matched_cnt / 2;
        nandflash_set_sample_delay_index(id, matched_idx);

#ifdef FLASH_CALIB_DEBUG
        calib_final_idx[type] = matched_idx;
#endif

        return 0;
    }

#ifdef FLASH_CALIB_DEBUG
    calib_final_idx[type] = -1;
#endif

    return 1;
}

void nandflash_show_calib_result(enum HAL_FLASH_ID_T id)
{
#ifdef FLASH_CALIB_DEBUG
    uint32_t div;
    uint32_t size;
    const uint8_t *list;
    int i;
#ifdef PROGRAMMER
    unsigned char buf[16];
#endif

    NORFLASH_TRACE(0, "FLASH_CALIB_RESULT:");
    NORFLASH_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "<FREQ>\nsource_clk=%u speed=%u flash_max=%u",
        nandflash_source_clk, nandflash_speed, flash_list[flash_idx[id]]->max_speed);
#ifdef PROGRAMMER
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &nandflash_source_clk, sizeof(nandflash_source_clk));
    memcpy(buf + 4, &nandflash_speed, sizeof(nandflash_speed));
    memcpy(buf + 8, &flash_list[flash_idx[id]]->max_speed, sizeof(flash_list[flash_idx[id]]->max_speed));
    send_debug_event(buf, 12);
#endif

#ifdef NANDFLASH_DELAY_WAKEUP
    if (nandflash_ctx[id].in_sleep) {
        nandflash_wakeup(id, nandflash_ctx[id].sleep_mode);
    }
#endif

    div = norflaship_get_div(id);
    NORFLASH_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "<DIV>\ndiv=%u", div);
#ifdef PROGRAMMER
    memset(buf, 0, sizeof(buf));
    buf[0] = div;
    send_debug_event(buf, 1);
#endif

    nandflash_get_samdly_list(div, &list, &size);
    NORFLASH_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "<SAMDLY LIST>");
    if (list == NULL || size == 0) {
        NORFLASH_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "NONE");
    } else {
        DUMP8("%02X ", list, size);
#ifdef PROGRAMMER
        send_debug_event(list, size);
#endif
    }
    NORFLASH_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "<CALIB RESULT>");
    for (i = 0; i < NANDFLASH_CALIB_QTY; i++) {
        NORFLASH_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "type=%d idx=%02u cnt=%02u final=%02u",
            i, calib_matched_idx[i], calib_matched_cnt[i], calib_final_idx[i]);
#ifdef PROGRAMMER
        memset(buf, 0, sizeof(buf));
        buf[0] = i;
        buf[1] = calib_matched_idx[i];
        buf[2] = calib_matched_cnt[i];
        buf[3] = calib_final_idx[i];
        send_debug_event(buf, 4);
#endif
    }
    NORFLASH_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "\t");
#endif
}

static int nandflash_init_sample_delay_by_div(enum HAL_FLASH_ID_T id, uint32_t div)
{
    uint32_t samdly;

    if (div == 0) {
        return -1;
    } if (div == 1) {
        samdly = 1;
    } else if (div == 2 && !falling_edge_adj[id]) {
        // Set sample delay to clock falling edge
        samdly = 3;
    } else {
        // Set sample delay to nearest to but not later than clock falling edge
        samdly = 4;
    }
    norflaship_samdly(id, samdly);

    return 0;
}

static int nandflash_init_div(enum HAL_FLASH_ID_T id, const struct NANDFLASH_CONFIG_T *cfg)
{
    uint32_t max_speed;
    uint32_t read_speed;
    uint32_t div;

#ifdef FLASH_CALIB_DEBUG
    nandflash_source_clk = cfg->source_clk;
    nandflash_speed = cfg->speed;
#endif

    max_speed = flash_list[flash_idx[id]]->max_speed;
    if (max_speed == 0) {
        max_speed = NANDFLASH_DEFAULT_MAX_SPEED;
    }

    read_speed = max_speed;
    if (read_speed > cfg->speed) {
        read_speed = cfg->speed;
    }
    if (read_speed > cfg->source_clk) {
        read_speed = cfg->source_clk;
    }

    div = (cfg->source_clk + read_speed - 1) / read_speed;
    div_read[id] = (div < NANDFLASH_MAX_DIV) ? div : NANDFLASH_MAX_DIV;

    if (div_read[id] == 2 && read_speed >= FALLING_EDGE_SAMPLE_ADJ_FREQ) {
        falling_edge_adj[id] = true;
    } else {
        falling_edge_adj[id] = false;
    }

    // Init sample delay according to div_read[id]
    nandflash_init_sample_delay_by_div(id, div_read[id]);
    norflaship_div(id, div_read[id]);

    return 0;
}

static int nandflash_match_chip(enum HAL_FLASH_ID_T id, const uint8_t *dev_id, uint32_t len)
{
    const uint8_t *cmp_id;

    if (len == NANDFLASH_ID_LEN) {
        for (flash_idx[id] = 0; flash_idx[id] < ARRAY_SIZE(flash_list); flash_idx[id]++) {
            cmp_id = flash_list[flash_idx[id]]->id;
            if (dev_id[0] == cmp_id[0] && dev_id[1] == cmp_id[1] && dev_id[2] == cmp_id[2]) {
                return true;
            }
        }
    }

    return false;
}

static enum NANDFLASH_RET_T nandflash_check_block(enum HAL_FLASH_ID_T id, uint32_t block)
{
    if ((block * flash_list[flash_idx[id]]->block_size) >= flash_list[flash_idx[id]]->total_size) {
        return NANDFLASH_RET_BAD_BLOCK;
    }
    return NANDFLASH_RET_OK;
}

static enum NANDFLASH_RET_T nandflash_check_page(enum HAL_FLASH_ID_T id, uint32_t page)
{
    if ((page * flash_list[flash_idx[id]]->page_size) >= flash_list[flash_idx[id]]->block_size) {
        return NANDFLASH_RET_BAD_PAGE;
    }
    return NANDFLASH_RET_OK;
}

static uint32_t nandflash_get_page_oob_size(enum HAL_FLASH_ID_T id)
{
    if (nandflash_op_mode[id] & NANDFLASH_OP_MODE_INTERNAL_ECC) {
        return flash_list[flash_idx[id]]->page_spare_size / 2;
    } else {
        return flash_list[flash_idx[id]]->page_spare_size;
    }
}

static enum NANDFLASH_RET_T nandflash_check_page_offset_len(enum HAL_FLASH_ID_T id, uint32_t page_offset, uint32_t len)
{
    uint32_t page_oob_size = nandflash_get_page_oob_size(id);

    if (page_offset >= flash_list[flash_idx[id]]->page_size + page_oob_size) {
        return NANDFLASH_RET_BAD_PAGE_OFFSET;
    }
    if (len > flash_list[flash_idx[id]]->page_size + page_oob_size) {
        return NANDFLASH_RET_BAD_LEN;
    }
    if (page_offset + len > flash_list[flash_idx[id]]->page_size + page_oob_size) {
        return NANDFLASH_RET_BAD_LEN;
    }
    return NANDFLASH_RET_OK;
}

static enum NANDFLASH_RET_T nandflash_check_oob_offset_len(enum HAL_FLASH_ID_T id, uint32_t oob_offset, uint32_t len)
{
    uint32_t page_oob_size = nandflash_get_page_oob_size(id);

    if (oob_offset >= page_oob_size) {
        return NANDFLASH_RET_BAD_PAGE_OFFSET;
    }
    if (len > page_oob_size) {
        return NANDFLASH_RET_BAD_LEN;
    }
    if (oob_offset + len > page_oob_size) {
        return NANDFLASH_RET_BAD_LEN;
    }
    return NANDFLASH_RET_OK;
}

static enum NANDFLASH_RET_T nandflash_erase_raw_block(enum HAL_FLASH_ID_T id, uint32_t block)
{
    uint32_t row_addr;
    uint8_t status;

    row_addr = nandflash_block_to_row_addr(block);

    nandflash_select_die(id, block);

    norflaship_cmd_addr(id, GD5F4GQ4_CMD_WRITE_ENABLE, 0);
    norflaship_cmd_addr(id, GD5F4GQ4_CMD_BLOCK_ERASE, row_addr);
    norflaship_busy_wait(id);
    nandflash_status_OIP_1_waitid(id);
    status = nandflash_read_statusid(id);
    norflaship_cmd_done(id);

    if (status & GD5F4GQ4_ST_E_FAIL) {
        return NANDFLASH_RET_ERASE_ERR;
    }
    return NANDFLASH_RET_OK;
}

enum NANDFLASH_RET_T nandflash_erase_block(enum HAL_FLASH_ID_T id, uint32_t block)
{
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    ret = nandflash_check_block(id, block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    return nandflash_erase_raw_block(id, block);
}

static void nandflash_program_load(enum HAL_FLASH_ID_T id, uint8_t cmd, uint32_t col_addr, const uint8_t *buffer, uint32_t len)
{
    uint32_t remains = 0;
    bool dma_en = false;;

    norflaship_clear_txfifo(id);

#ifdef FLASH_DMA
    remains = nandflash_start_tx_dma(id, buffer, len);
    if (remains < len) {
        dma_en = true;
    }
#endif

    if (!dma_en) {
#if (CHIP_FLASH_CTRL_VER >= 3) && defined(UNALIGNED_ACCESS)
        remains = norflaship_write4_txfifo_start(id, buffer, len);
#else
        remains = norflaship_write_txfifo_start(id, buffer, len);
#endif
    }

    norflaship_cmd_addr(id, cmd, col_addr);

#ifdef FLASH_DMA
    if (dma_en) {
        nandflash_dma_busy_wait(id);
    }
#endif

    while (remains > 0) {
        buffer += len - remains;
        len = remains;
#if (CHIP_FLASH_CTRL_VER >= 3) && defined(UNALIGNED_ACCESS)
        remains = norflaship_write4_txfifo_cont(id, buffer, len);
#else
        remains = norflaship_write_txfifo_cont(id, buffer, len);
#endif
    }

    norflaship_busy_wait(id);
}

static enum NANDFLASH_RET_T nandflash_program_execute(enum HAL_FLASH_ID_T id, uint32_t row_addr)
{
    uint8_t status;

    norflaship_cmd_addr(id, GD5F4GQ4_CMD_PROGRAM_EXECUTRE, row_addr);
    norflaship_busy_wait(id);
    nandflash_status_OIP_1_waitid(id);
    status = nandflash_read_statusid(id);

    if (status & GD5F4GQ4_ST_P_FAIL) {
        return NANDFLASH_RET_PROGRAM_ERR;
    }
    return NANDFLASH_RET_OK;
}

enum NANDFLASH_RET_T nandflash_write_page_offset(enum HAL_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, const uint8_t *buffer, uint32_t len)
{
    enum NANDFLASH_RET_T ret;
    uint8_t cmd;
    uint32_t row_addr, col_addr;

    if (nandflash_op_mode[id] & NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM) {
        cmd = GD5F4GQ4_CMD_QUAD_PROGRAM_LOAD;
    } else {
        cmd = GD5F4GQ4_CMD_PROGRAM_LOAD;
    }

    row_addr = nandflash_block_page_to_row_addr(block, page);

    nandflash_select_die(id, block);

    norflaship_cmd_addr(id, GD5F4GQ4_CMD_WRITE_ENABLE, 0);

    col_addr = nandflash_block_page_offset_to_col_addr(id, block, offset);
    nandflash_program_load(id, cmd, col_addr, buffer, len);

    ret = nandflash_program_execute(id, row_addr);

    norflaship_cmd_done(id);

    return ret;
}

enum NANDFLASH_RET_T nandflash_write_page(enum HAL_FLASH_ID_T id, uint32_t block, uint32_t page, const uint8_t *buffer, uint32_t len)
{
    enum NANDFLASH_RET_T ret;
    uint32_t offset;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    ret = nandflash_check_block(id, block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page(id, page);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page_offset_len(id, 0, len);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    if (len == 0) {
        return NANDFLASH_RET_OK;
    }

    offset = 0;
    return nandflash_write_page_offset(id, block, page, offset, buffer, len);
}

enum NANDFLASH_RET_T nandflash_write_oob(enum HAL_FLASH_ID_T id, uint32_t block, uint32_t page, const uint8_t *buffer, uint32_t len)
{
    enum NANDFLASH_RET_T ret;
    uint32_t offset;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_block(id, block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page(id, page);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_oob_offset_len(id, 0, len);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    if (len == 0) {
        return NANDFLASH_RET_OK;
    }

    offset = flash_list[flash_idx[id]]->page_size;
    return nandflash_write_page_offset(id, block, page, offset, buffer, len);
}

static enum NANDFLASH_RET_T nandflash_update_page_offset(enum HAL_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, const uint8_t *buffer, uint32_t len)
{
    enum NANDFLASH_RET_T ret;
    uint8_t cmd;
    uint32_t row_addr, col_addr;

    if (nandflash_op_mode[id] & NANDFLASH_OP_MODE_QUAD_PAGE_PROGRAM) {
        cmd = GD5F4GQ4_CMD_QUAD_PROGRAM_LOAD_RANDOM;
    } else {
        cmd = GD5F4GQ4_CMD_PROGRAM_LOAD_RANDOM;
    }

    row_addr = nandflash_block_page_to_row_addr(block, page);

    nandflash_select_die(id, block);

    ret = nandflash_start_page_read(id, row_addr);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        return ret;
    }

    norflaship_cmd_addr(id, GD5F4GQ4_CMD_WRITE_ENABLE, 0);

    col_addr = nandflash_block_page_offset_to_col_addr(id, block, offset);
    nandflash_program_load(id, cmd, col_addr, buffer, len);

    ret = nandflash_program_execute(id, row_addr);

    norflaship_cmd_done(id);

    return ret;
}

enum NANDFLASH_RET_T nandflash_update_page(enum HAL_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, const uint8_t *buffer, uint32_t len)
{
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    ret = nandflash_check_block(id, block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page(id, page);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page_offset_len(id, offset, len);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    if (len == 0) {
        return NANDFLASH_RET_OK;
    }

    return nandflash_update_page_offset(id, block, page, offset, buffer, len);
}

enum NANDFLASH_RET_T nandflash_update_oob(enum HAL_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, const uint8_t *buffer, uint32_t len)
{
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    ret = nandflash_check_block(id, block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page(id, page);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_oob_offset_len(id, offset, len);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    if (len == 0) {
        return NANDFLASH_RET_OK;
    }

    offset += flash_list[flash_idx[id]]->page_size;
    return nandflash_update_page_offset(id, block, page, offset, buffer, len);
}

enum NANDFLASH_RET_T nandflash_copy_page(enum HAL_FLASH_ID_T id, uint32_t dst_block, uint32_t dst_page, uint32_t src_block, uint32_t src_page)
{
    enum NANDFLASH_RET_T ret;
    uint32_t row_addr;
    uint32_t src_die, dst_die;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    ret = nandflash_check_block(id, dst_block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page(id, dst_page);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_block(id, src_block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page(id, src_page);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    src_die = nandflash_block_to_die(id, src_block);
    dst_die = nandflash_block_to_die(id, dst_block);
    if (src_die == dst_die) {
        row_addr = nandflash_block_page_to_row_addr(src_block, src_page);

        nandflash_select_die(id, src_block);

        ret = nandflash_start_page_read(id, row_addr);
        if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
            return ret;
        }

        row_addr = nandflash_block_page_to_row_addr(dst_block, dst_page);

        nandflash_select_die(id, dst_block);

        norflaship_cmd_addr(id, GD5F4GQ4_CMD_WRITE_ENABLE, 0);

        ret = nandflash_program_execute(id, row_addr);

        norflaship_cmd_done(id);
    } else {
        ASSERT(0, "cann't copy from different dir/plane through flash cache buffer");
        ret = NANDFLASH_RET_ERR;
    }

    return ret;
}

static enum NANDFLASH_RET_T nandflash_read_raw_page(enum HAL_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, uint8_t *buffer, uint32_t len)
{
    enum NANDFLASH_RET_T ret;
    uint32_t row_addr, col_addr;
    uint32_t remains;

    row_addr = nandflash_block_page_to_row_addr(block, page);

    nandflash_select_die(id, block);

    ret = nandflash_start_page_read(id, row_addr);
    norflaship_cmd_done(id);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        return ret;
    }

#if defined(FLASH_DMA) && !defined(FLASH_DMA_RX_M2M)
    remains = nandflash_start_rx_dma(id, buffer, len);

    if (remains < len) {
        uint32_t dma_len = len - remains;

        norflaship_busy_wait(id);
        // Rx word size
        norflaship_dma_rx_size(id, 4);

        col_addr = nandflash_block_page_offset_to_col_addr(id, block, offset);
        nandflash_read_from_cache(id, col_addr, dma_len);

        offset += dma_len;
        buffer += dma_len;

        nandflash_dma_busy_wait(id);
        norflaship_busy_wait(id);
        // Rx byte size
        norflaship_dma_rx_size(id, 1);
    }
#else // !(FLASH_DMA && !FLASH_DMA_RX_M2M)
    remains = len;
#endif // !(FLASH_DMA && !FLASH_DMA_RX_M2M)

    if (remains) {
#ifdef BUFFER_READ
        uint32_t index = 0;
        uint8_t val;
        norflaship_clear_rxfifo(id);
        col_addr = nandflash_block_page_offset_to_col_addr(id, block, offset);
        nandflash_read_from_cache(id, col_addr, remains);
        while (1) {
            norflaship_rxfifo_empty_wait(id);

            val = norflaship_read_rxfifo(id);
            if (buffer) {
                buffer[index] = val;
            }

            ++index;
            if (index >= remains) {
                break;
            }
        }
#else // !BUFFER_READ
        col_addr = nandflash_block_page_offset_to_col_addr(id, block, offset);
#if defined(FLASH_DMA) && defined(FLASH_DMA_RX_M2M)
        nandflash_start_rx_dma_m2m(id, (uint32_t)buffer, (nand_nc_base[id] + col_addr), remains);
#else
        memcpy(buffer, (uint8_t *)(nand_nc_base[id] + col_addr), remains);
#endif
#endif // !BUFFER_READ
    }

    return ret;
}

enum NANDFLASH_RET_T nandflash_read_page(enum HAL_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, uint8_t *buffer, uint32_t len)
{
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    ret = nandflash_check_block(id, block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page(id, page);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page_offset_len(id, offset, len);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    if (len == 0) {
        return NANDFLASH_RET_OK;
    }

    ret = nandflash_read_raw_page(id, block, page, offset, buffer, len);

    return ret;
}

enum NANDFLASH_RET_T nandflash_read_oob(enum HAL_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, uint8_t *buffer, uint32_t len)
{
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    ret = nandflash_check_block(id, block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_page(id, page);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    ret = nandflash_check_oob_offset_len(id, offset, len);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }
    if (len == 0) {
        return NANDFLASH_RET_OK;
    }

    offset += flash_list[flash_idx[id]]->page_size;
    ret = nandflash_read_raw_page(id, block, page, offset, buffer, len);

    return ret;
}

static int nandflash_raw_block_is_bad(enum HAL_FLASH_ID_T id, uint32_t block)
{
    enum NANDFLASH_RET_T ret;
    uint32_t page;
    uint8_t mark[2];

    nandflash_select_die(id, block);
    for (page = 0; page < 2; page++) {
#ifdef BUFFER_READ
        ret = nandflash_read_raw_page(id, block, page, flash_list[flash_idx[id]]->page_size, mark+page, 1);
#else
        uint32_t row_addr, col_addr;
        row_addr = nandflash_block_page_to_row_addr(block, page);
        col_addr = nandflash_block_page_offset_to_col_addr(id, block, flash_list[flash_idx[id]]->page_size);
        ret = nandflash_start_page_read(id, row_addr);
        norflaship_cmd_done(id);
        mark[page] = *(volatile uint8_t *)(nand_nc_base[id] + col_addr);
#endif
        if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
            return true;
        }
    }

    if (mark[0] != 0xFF || mark[1] != 0xFF) {
        return true;
    }

    return false;
}

int nandflash_block_is_bad(enum HAL_FLASH_ID_T id, uint32_t block)
{
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    ASSERT(ret == NANDFLASH_RET_OK, "%s", err_not_opened);

    ret = nandflash_check_block(id, block);
    if (ret != NANDFLASH_RET_OK) {
        return true;
    }

    return nandflash_raw_block_is_bad(id, block);
}

enum NANDFLASH_RET_T nandflash_mark_bad_block(enum HAL_FLASH_ID_T id, uint32_t block)
{
    enum NANDFLASH_RET_T ret;
    uint32_t offset;
    uint8_t mark;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    ret = nandflash_check_block(id, block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    offset = flash_list[flash_idx[id]]->page_size;
    mark = 0;

    nandflash_erase_raw_block(id, block);
    nandflash_write_page_offset(id, block, 0, offset, &mark, 1);
    nandflash_write_page_offset(id, block, 1, offset, &mark, 1);

    return NANDFLASH_RET_OK;
}

static enum NANDFLASH_RET_T nandflash_skip_bad_block(enum HAL_FLASH_ID_T id, uint32_t *p_block, uint32_t block_limit)
{
    bool bad_block;
    uint32_t block = *p_block;

    while (1) {
        bad_block = nandflash_raw_block_is_bad(id, *p_block);
        if (bad_block) {
            block++;
            if (block >= block_limit) {
                return NANDFLASH_RET_LIMIT_REACHED;
            }
            continue;
        }
        break;
    }

    *p_block = block;

    return NANDFLASH_RET_OK;
}

enum NANDFLASH_RET_T nandflash_load_valid_data(enum HAL_FLASH_ID_T id, uint32_t start_block, uint32_t block_cnt, uint8_t *buffer, uint32_t len)
{
    enum NANDFLASH_RET_T ret;
    uint32_t block_size;
    uint32_t page_size;
    uint32_t pages_per_block;
    uint32_t block;
    uint32_t block_limit;
    uint32_t page;
    uint8_t *op_buf;
    uint32_t op_size;
    uint32_t remain_len;
    bool read_corrected = false;

    if (len == 0) {
        return NANDFLASH_RET_OK;
    }

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    ret = nandflash_check_block(id, start_block);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    block_size = flash_list[flash_idx[id]]->block_size;
    page_size = flash_list[flash_idx[id]]->page_size;
    pages_per_block = block_size / page_size;

    if (block_cnt) {
        if (block_cnt * block_size < len) {
            return NANDFLASH_RET_BAD_LEN;
        }
    } else {
        block_cnt = (len + block_size - 1) / block_size;
    }

    ret = nandflash_check_block(id, start_block + block_cnt - 1);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    op_buf = buffer;
    remain_len = len;
    block = start_block;
    block_limit = start_block + block_cnt;
    page = 0;

    while (remain_len > 0) {
        ret = nandflash_skip_bad_block(id, &block, block_limit);
        if (ret != NANDFLASH_RET_OK) {
            return ret;
        }

        op_size = page_size;
        if (op_size > remain_len) {
            op_size = remain_len;
        }
        ret = nandflash_read_raw_page(id, block, page, 0, op_buf, op_size);
        if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
            return ret;
        }
        if (ret == NANDFLASH_RET_READ_CORRECTED) {
            read_corrected = true;
        }

        page++;
        if (page >= pages_per_block) {
            page = 0;
            block++;
        }
        op_buf += op_size;
        remain_len -= op_size;
    }

    return read_corrected ? NANDFLASH_RET_READ_CORRECTED : NANDFLASH_RET_OK;
}

int nandflash_busy(enum HAL_FLASH_ID_T id)
{
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    return norflaship_is_busy(id);
}

static void nandflash_set_block_protection(enum HAL_FLASH_ID_T id, uint32_t bp)
{
    uint8_t prt;
    uint32_t bp_mask;

    bp_mask = nandflash_get_block_protect_mask(id);

    prt = nandflash_read_protection(id);
    prt = (prt & ~bp_mask) | (bp & bp_mask);
    nandflash_write_protection(id, prt);
}

POSSIBLY_UNUSED
static void nandflash_init_protection(enum HAL_FLASH_ID_T id)
{
    nandflash_set_block_protection(id, 0);
}

static uint32_t nandflash_get_bp_cfg(enum HAL_FLASH_ID_T id)
{
    uint32_t val = 0;

    // Set BP and CMP bits
    if (flash_list[flash_idx[id]]->die_num) {
        // TODO: Set different BP value for each die
        return 0;
    }

    if (FLASH_SIZE != flash_list[flash_idx[id]]->total_size) {
        return val;
    }

    return val;
}

enum NANDFLASH_RET_T nandflash_enable_protection(enum HAL_FLASH_ID_T id)
{
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    nandflash_set_block_protection(id, nandflash_get_bp_cfg(id));

    return NANDFLASH_RET_OK;
}

enum NANDFLASH_RET_T nandflash_disable_protection(enum HAL_FLASH_ID_T id)
{
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    nandflash_set_block_protection(id, 0);

    return NANDFLASH_RET_OK;
}

#ifdef FLASH_SECURITY_REGISTER
static union NANDFLASH_SEC_REG_CFG_T nandflash_get_security_register_config(enum HAL_FLASH_ID_T id)
{
    return flash_list[flash_idx[id]]->sec_reg_cfg;
}

int nandflash_security_register_is_locked(enum HAL_FLASH_ID_T id)
{
    union NANDFLASH_SEC_REG_CFG_T sec_cfg;
    uint8_t cfg;
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    sec_cfg = nandflash_get_security_register_config(id);
    if (!sec_cfg.s.enabled) {
        return false;
    }

    cfg = nandflash_read_configuration(id);

    return !!(cfg & GD5F4GQ4_CFG_OTP_PRT);
}

enum NANDFLASH_RET_T nandflash_security_register_lock(enum HAL_FLASH_ID_T id)
{
    uint8_t cfg;
    uint8_t backup_cfg;
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    cfg = nandflash_read_configuration(id);
    backup_cfg = cfg;
    cfg |= (GD5F4GQ4_CFG_OTP_PRT | GD5F4GQ4_CFG_OTP_EN);
    nandflash_write_configuration(id, cfg);

    nandflash_program_execute(id, 0); // execute otp lock bit

    backup_cfg &= ~(GD5F4GQ4_CFG_OTP_PRT);
    nandflash_write_configuration(id, backup_cfg);

    return NANDFLASH_RET_OK;
}

uint32_t nandflash_security_register_enable(enum HAL_FLASH_ID_T id)
{
    uint32_t mode;
    uint8_t cfg;
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    cfg = nandflash_read_configuration(id);
    mode = !!(cfg & GD5F4GQ4_CFG_OTP_EN);
    if (!mode) {
        cfg |= GD5F4GQ4_CFG_OTP_EN;
        nandflash_write_configuration(id, cfg);
        norflaship_busy_wait(id);
        norflaship_clear_rxfifo(id);
        norflaship_busy_wait(id);
    }

    return mode;
}

void nandflash_security_register_disable(enum HAL_FLASH_ID_T id, uint32_t mode)
{
    uint8_t cfg;

    if (nandflash_check_state(id) != NANDFLASH_RET_OK) {
        return;
    }

    if (!mode) {
        norflaship_busy_wait(id);
        cfg = nandflash_read_configuration(id);
        cfg &= ~GD5F4GQ4_CFG_OTP_EN;
        nandflash_write_configuration(id, cfg);
        norflaship_clear_rxfifo(id);
        norflaship_busy_wait(id);
    }
}

enum NANDFLASH_RET_T nandflash_security_register_read(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len)
{
    return nandflash_read_page(id, 0, start_address/flash_list[flash_idx[id]]->page_size, 0, buffer, len);
}

enum NANDFLASH_RET_T nandflash_security_register_write(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len)
{
    return nandflash_write_page(id, 0, start_address/flash_list[flash_idx[id]]->page_size, buffer, len);
}
#endif

int nandflash_opened(enum HAL_FLASH_ID_T id)
{
    return nandflash_ctx[id].opened;
}

enum NANDFLASH_RET_T nandflash_get_open_state(enum HAL_FLASH_ID_T id)
{
    return nandflash_ctx[id].open_state;
}

enum NANDFLASH_RET_T nandflash_get_size(enum HAL_FLASH_ID_T id, uint32_t *total_size, uint32_t *block_size,
                                        uint32_t *page_size, uint32_t *page_oob_size, uint32_t *die_num)
{
    if (!nandflash_ctx[id].opened) {
        return NANDFLASH_RET_NOT_OPENED;
    }

    if (total_size) {
        *total_size  = flash_list[flash_idx[id]]->total_size;
    }
    if (block_size) {
        *block_size  = flash_list[flash_idx[id]]->block_size;
    }
    if (page_size) {
        *page_size   = flash_list[flash_idx[id]]->page_size;
    }
    if (page_oob_size) {
        *page_oob_size = nandflash_get_page_oob_size(id);
    }
    if (die_num) {
        *die_num = flash_list[flash_idx[id]]->die_num;
    }

    return NANDFLASH_RET_OK;
}

enum NANDFLASH_RET_T nandflash_get_boundary(enum HAL_FLASH_ID_T id, uint32_t offset, uint32_t *block_boundary, uint32_t *page_boundary)
{
    uint32_t block;
    uint32_t remain;
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    if (offset >= flash_list[flash_idx[id]]->total_size) {
        return NANDFLASH_RET_BAD_OFFSET;
    }

    block = offset / flash_list[flash_idx[id]]->block_size;
    remain = offset % flash_list[flash_idx[id]]->block_size;

    if (block_boundary) {
        *block_boundary  = block;
    }
    if (page_boundary) {
        *page_boundary = remain / flash_list[flash_idx[id]]->page_size;
    }

    return NANDFLASH_RET_OK;
}

enum NANDFLASH_RET_T nandflash_get_id(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len)
{
    enum NANDFLASH_RET_T ret;

    ret = nandflash_check_state(id);
    if (ret != NANDFLASH_RET_OK) {
        return ret;
    }

    len = MIN(len, sizeof(nandflash_ctx[id].device_id));

    memcpy(value, nandflash_ctx[id].device_id, len);

    return NANDFLASH_RET_OK;
}

static enum HAL_CMU_FREQ_T nandflash_clk_to_cmu_freq(uint32_t clk)
{
    enum HAL_CMU_FREQ_T freq;

    if (clk >= NANDFLASH_SPEED_208M) {
        freq = HAL_CMU_FREQ_208M;
    } else if (clk >= NANDFLASH_SPEED_104M) {
        freq = HAL_CMU_FREQ_104M;
    } else if (clk >= NANDFLASH_SPEED_78M) {
        freq = HAL_CMU_FREQ_78M;
    } else if (clk >= NANDFLASH_SPEED_52M) {
        freq = HAL_CMU_FREQ_52M;
    } else {
        freq = HAL_CMU_FREQ_26M;
    }

    return freq;
}

static void _nandflash_set_source_freq(enum HAL_FLASH_ID_T id, enum HAL_CMU_FREQ_T freq)
{
    if (0) {
#ifdef FLASH1_CTRL_BASE
    } else if (id == HAL_FLASH_ID_1) {
        hal_cmu_flash1_set_freq(freq);
#endif
#ifdef FLASH2_CTRL_BASE
    } else if (id == HAL_FLASH_ID_2) {
        hal_cmu_flash2_set_freq(freq);
#endif
    } else {
        hal_cmu_flash_set_freq(freq);
    }
}

static void nandflash_reset_timing(enum HAL_FLASH_ID_T id)
{
    const uint32_t default_div = 8;

    // Restore default divider
    norflaship_div(id, default_div);
    nandflash_init_sample_delay_by_div(id, default_div);
}

static void nandflash_init_die_cfg(enum HAL_FLASH_ID_T id)
{
    if (flash_list[flash_idx[id]]->die_num) {
        nandflash_ctx[id].blocks_per_die = flash_list[flash_idx[id]]->total_size / flash_list[flash_idx[id]]->block_size / flash_list[flash_idx[id]]->die_num;
        nandflash_ctx[id].cur_block = 0;
        nandflash_ctx[id].cur_die = 0;
    } else {
        nandflash_ctx[id].blocks_per_die = 0;
    }
}

static void nandflash_init_dummy_cycle(enum HAL_FLASH_ID_T id)
{
    if (flash_list[flash_idx[id]]->dummy_cycle_cfg.s.quad_io_en) {
        norflaship_dummy_n_rcqio(id, flash_list[flash_idx[id]]->dummy_cycle_cfg.s.quad_io_val);
    }
}

void nandflash_sleep(enum HAL_FLASH_ID_T id, enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    if (!nandflash_ctx[id].opened) {
        return;
    }
#ifdef FLASH_DEEP_SLEEP
    if (flash_list[flash_idx[id]]->mode & NANDFLASH_OP_MODE_DEEP_POWER_DOWN) {
        norflaship_busy_wait(id);
        norflaship_ext_tx_cmd(id, GD5F4GQ4_CMD_DEEP_POWER_DOWN, 0);
        nandflash_delay(6);
    }
#endif
    norflaship_busy_wait(id);
    norflaship_sleep(id, mode);

#ifdef NANDFLASH_DELAY_WAKEUP
    if (nandflash_ctx[id].sleep_mode != HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD) {
        nandflash_ctx[id].sleep_mode = mode;
    }
    nandflash_ctx[id].in_sleep = true;
#endif
}

void nandflash_wakeup(enum HAL_FLASH_ID_T id, enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    if (!nandflash_ctx[id].opened) {
        return;
    }
#ifdef NANDFLASH_DELAY_WAKEUP
    if (!nandflash_ctx[id].in_sleep) {
        return;
    }
#endif

    norflaship_wakeup(id, mode);

#ifdef FLASH_DEEP_SLEEP
    if (flash_list[flash_idx[id]]->mode & NANDFLASH_OP_MODE_DEEP_POWER_DOWN) {
        norflaship_busy_wait(id);
        nandflash_release_from_dp(id);
        norflaship_clear_rxfifo(id);
        norflaship_busy_wait(id);
    }
#endif

#ifdef NANDFLASH_DELAY_WAKEUP
    nandflash_ctx[id].in_sleep = false;
    nandflash_ctx[id].sleep_mode = HAL_CMU_LPU_SLEEP_MODE_SUBSYS_32K;
#endif
}

static int nandflash_pm_notify_handler(enum HAL_PM_STATE_T state)
{
    enum HAL_FLASH_ID_T id;

    for (id = HAL_FLASH_ID_0; id < HAL_FLASH_ID_NUM; id ++) {
        if (nandflash_ctx[id].opened) {
            if (state == HAL_PM_STATE_NORMAL_SLEEP) {
                nandflash_sleep(id, HAL_CMU_LPU_SLEEP_MODE_BUS_32K);
            } else if (state == HAL_PM_STATE_POWER_DOWN_SLEEP) {
                nandflash_sleep(id, HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD);
#ifndef NANDFLASH_DELAY_WAKEUP
            } else if (state == HAL_PM_STATE_NORMAL_WAKEUP) {
                nandflash_wakeup(id, HAL_CMU_LPU_SLEEP_MODE_BUS_32K);
            } else if (state == HAL_PM_STATE_POWER_DOWN_WAKEUP) {
                nandflash_wakeup(id, HAL_CMU_LPU_SLEEP_MODE_SUBSYS_PD);
#endif
            }
        }
    }

    return 0;
}

static void nandflsh_pm_notiy_set(enum HAL_FLASH_ID_T id, bool on)
{
    enum HAL_FLASH_ID_T cur_id;
    uint32_t opened_num = 0;
    uint32_t lock;

    for (cur_id = HAL_FLASH_ID_0; cur_id < HAL_FLASH_ID_NUM; cur_id ++) {
        if (nandflash_ctx[cur_id].opened) {
            opened_num ++;
        }
    }

    if (opened_num == 1) {
        lock = int_lock();
        if (on) {
            hal_pm_notif_register(HAL_PM_USER_DRV, nandflash_pm_notify_handler);
        } else {
            hal_pm_notif_deregister(HAL_PM_USER_DRV, nandflash_pm_notify_handler);
        }
        int_unlock(lock);
    }
}

enum NANDFLASH_RET_T nandflash_open(enum HAL_FLASH_ID_T id, const struct NANDFLASH_CONFIG_T *cfg)
{
    enum HAL_CMU_FREQ_T source_freq;
    int result;
    uint32_t op;
    int found;
    uint8_t eccs_st_bfd;

    // Place the config into ram
    if (cfg == NULL) {
        return NANDFLASH_RET_CFG_NULL;
    }

    if (0) {
#ifdef FLASH1_CTRL_BASE
    } else if (id == HAL_FLASH_ID_1) {
        hal_cmu_clock_enable(HAL_CMU_MOD_O_FLASH1);
        hal_cmu_clock_enable(HAL_CMU_MOD_H_FLASH1);
        hal_cmu_reset_clear(HAL_CMU_MOD_O_FLASH1);
        hal_cmu_reset_clear(HAL_CMU_MOD_H_FLASH1);
#endif
#ifdef FLASH2_CTRL_BASE
    } else if (id == HAL_FLASH_ID_2) {
        hal_cmu_clock_enable(HAL_CMU_MOD_O_FLASH2);
        hal_cmu_clock_enable(HAL_CMU_MOD_H_FLASH2);
        hal_cmu_reset_clear(HAL_CMU_MOD_O_FLASH2);
        hal_cmu_reset_clear(HAL_CMU_MOD_H_FLASH2);
#endif
    } else {
        hal_cmu_clock_enable(HAL_CMU_MOD_O_FLASH);
        hal_cmu_clock_enable(HAL_CMU_MOD_H_FLASH);
        hal_cmu_reset_clear(HAL_CMU_MOD_O_FLASH);
        hal_cmu_reset_clear(HAL_CMU_MOD_H_FLASH);
    }

    norflaship_busy_wait(id);

    norflaship_nand_sel(id, true);
    norflaship_dmactrl_tx_en(id, false);
    norflaship_dmactrl_rx_en(id, false);
#ifdef FLASH_DMA
    norflaship_dma_tx_threshold(id, NORFLASHIP_TXFIFO_SIZE / 2);
    norflaship_dma_rx_threshold(id, NORFLASHIP_RXFIFO_SIZE / 2);
#endif

    // Reset states
    nandflash_ctx[id].opened = false;
#ifdef NANDFLASH_DELAY_WAKEUP
    nandflash_ctx[id].in_sleep = false;
    nandflash_ctx[id].sleep_mode = HAL_CMU_LPU_SLEEP_MODE_SUBSYS_32K;
#endif
    nandflash_ctx[id].open_state = NANDFLASH_RET_NOT_OPENED;

    // Set the direction of 4 IO pins to output when in idle
    norflaship_set_idle_io_dir(id, 0);

    // Reset norflash source clock
    _nandflash_set_source_freq(id, HAL_CMU_FREQ_26M);

    // Reset controller timing
    nandflash_reset_timing(id);

    // Reset norflash in slow clock configuration
    nandflash_reset(id);

    // Get device ID
    nandflash_read_device_id(id, nandflash_ctx[id].device_id, sizeof(nandflash_ctx[id].device_id));

    // For each driver in array, match chip and select drv_ops
    found = nandflash_match_chip(id, nandflash_ctx[id].device_id, sizeof(nandflash_ctx[id].device_id));
    if (!found) {
        result = NANDFLASH_RET_BAD_ID;
        goto _exit;
    }

    nandflash_init_die_cfg(id);

    // Init dummy cycle
    nandflash_init_dummy_cycle(id);

    // Set norflash source clock
    source_freq = nandflash_clk_to_cmu_freq(cfg->source_clk);
    _nandflash_set_source_freq(id, source_freq);

    /* over write config */
    if (cfg->override_config) {
        /* div */
        norflaship_div(id, cfg->div);
        /* sample delay */
        norflaship_samdly(id, cfg->samdly);
        /* ru en */
        norflaship_ruen(id, cfg->spiruen);
        /* rd en */
        norflaship_rden(id, cfg->spirden);
        /* rd cmd */
        norflaship_rdcmd(id, cfg->rdcmd);
        /* frd cmd */
        norflaship_frdcmd(id, cfg->frdcmd);
        /* qrd cmd */
        norflaship_qrdcmd(id, cfg->qrdcmd);
    } else {
        // Init divider
        result = nandflash_init_div(id, cfg);
        if (result != 0) {
            result = NANDFLASH_RET_BAD_DIV;
            goto _exit;
        }

        // Calib with the new divider
        result = nandflash_sample_delay_calib(id, NANDFLASH_CALIB_FLASH_ID);
        if (result != 0) {
            result = NANDFLASH_RET_BAD_CALIB_ID;
            goto _exit;
        }
    }

#if defined(PROGRAMMER) || defined(OTA_PROGRAMMER)
    nandflash_init_protection(id);
#else
    nandflash_set_block_protection(id, nandflash_get_bp_cfg(id));
#endif

    op = cfg->mode;

    // Divider will be set to normal read mode
    result = nandflash_set_mode(id, op);
    if (result != 0) {
        result = NANDFLASH_RET_BAD_OP;
        goto _exit;
    }

    // set internal ecc st_bfd
    if (nandflash_op_mode[id] & NANDFLASH_OP_MODE_INTERNAL_ECC) {
        eccs_st_bfd = nandflash_get_ext_eccs_st_bfd(id);
        if (eccs_st_bfd > NANDFLASH_ECC_ST_BFD_RESERVED && eccs_st_bfd <= NANDFLASH_ECC_ST_BFD_7) {
            nandflash_write_ext_eccs_st_bfd(id, eccs_st_bfd);
        }
    }

#if 0
    result = nandflash_sample_delay_calib(id, NANDFLASH_CALIB_MAGIC_WORD);
    if (result != 0) {
        result = NANDFLASH_RET_BAD_CALIB_MAGIC;
        goto _exit;
    }
#endif

    // Init cfg for each die
    if (flash_list[flash_idx[id]]->die_num) {
        uint8_t prt_val;
        uint8_t cfg_val;

        // TODO: Set different BP value for each die
        prt_val = nandflash_read_protection(id);
        cfg_val = nandflash_read_configuration(id);
        for (uint32_t i = 1; i < flash_list[flash_idx[id]]->die_num; i++) {
            nandflash_software_die_select(id, i);
            nandflash_write_protection(id, prt_val);
            nandflash_write_configuration(id, cfg_val);
        }
        nandflash_software_die_select(id, 0);
    }

    nandflash_ctx[id].opened = true;

    nandflsh_pm_notiy_set(id, true);

#if (CHIP_FLASH_CTRL_VER >= 5) && defined(FLASH_DTR)
    norflaship_nand_addr_byte_num(id, 3);
    result = nandflash_sample_delay_calib(id, NANDFLASH_CALIB_DTR);
    if (result != 0) {
        result = NANDFLASH_RET_BAD_CALIB_DTR;
        goto _exit;
    }
#endif

    result = NANDFLASH_RET_OK;

_exit:
    if (result != NANDFLASH_RET_OK) {
        _nandflash_set_source_freq(id, HAL_CMU_FREQ_26M);
        nandflash_reset_timing(id);
        // Flash might be accessed again
    }

    nandflash_ctx[id].open_state = result;

    return result;
}

enum NANDFLASH_RET_T nandflash_close(enum HAL_FLASH_ID_T id)
{
    if (!nandflash_ctx[id].opened) {
        NORFLASH_TRACE(1, "%s", err_not_opened);
        return NANDFLASH_RET_NOT_OPENED;
    }

    nandflsh_pm_notiy_set(id, false);

    return NANDFLASH_RET_OK;
}

enum NANDFLASH_RET_T nandflash_init(enum HAL_FLASH_ID_T id)
{
    enum NANDFLASH_RET_T ret;
    const struct NANDFLASH_CONFIG_T *cfg = &nandflash_cfg;

    ret = nandflash_open(id, cfg);

    return ret;
}

void nandflash_get_flash_list(const struct NANDFLASH_CFG_T **list, uint32_t *len)
{
    *list = (struct NANDFLASH_CFG_T *)&flash_list[0];
    *len = ARRAY_SIZE(flash_list);
}
#endif
