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

#ifdef SDMMC0_BASE

#include "hal_sdio_host.h"
#include "hal_location.h"
#include "reg_sdmmcip.h"
#include "cmsis_nvic.h"
#include "hal_sleep.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sdmmc.h"
#include "hal_dma.h"
#include "errno.h"
#include <string.h>
#ifdef RTOS
    #include "cmsis_os.h"
#endif

#ifdef SDIO_HOST_DEBUG
    #ifdef FPGA
        #define HAL_SDIO_HOST_TRACE(n, s, ...)      TRACE(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
        #define HAL_SDIO_HOST_ASSERT(n, s, ...)     ASSERT(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
    #else
        #define HAL_SDIO_HOST_TRACE(n, s, ...)      TRACE(n, s, ##__VA_ARGS__)
        #define HAL_SDIO_HOST_ASSERT(n, s, ...)     ASSERT(n, s, ##__VA_ARGS__)
    #endif
#else
    #define HAL_SDIO_HOST_TRACE(n, s, ...)          TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #ifdef FPGA
        #define HAL_SDIO_HOST_ASSERT(n, s, ...)     ASSERT(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
    #else
        #define HAL_SDIO_HOST_ASSERT(n, s, ...)     ASSERT(n, s, ##__VA_ARGS__)
    #endif
#endif

#if 1
    #define HAL_SDIO_HOST_ERROR(n, s, ...)          TRACE(n, s, ##__VA_ARGS__)
#else
    #define HAL_SDIO_HOST_ERROR(n, s, ...)          TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

/******************************************************************************/
/******************The first part: sdio host ip related************************/
/******************************************************************************/
#define SDIO_HOST_FIFO_DEPTH            16  //width 32bits
#define SDIO_HOST_FIFO_BYTES            (SDIO_HOST_FIFO_DEPTH * 4)

/* sdio bus speed */
#define SDIO_SPEED_DS                   25000000
#define SDIO_SPEED_HS                   50000000
#define SDIO_SPEED_SDR12                25000000
#define SDIO_SPEED_SDR25                50000000
#define SDIO_SPEED_DDR50                50000000
#define SDIO_SPEED_SDR50                100000000
#define SDIO_SPEED_SDR104               208000000

/* sdio cmd list */
#define SDIO_CMD_GO_IDLE_STATE          0   /* RSP:no */
#define SDIO_CMD_SEND_RELATIVE_ADDR     3   /* RSP:R6 */
#define SDIO_CMD_IO_SEND_OP_COND        5   /* RSP:R4 */
#define SDIO_CMD_SELECT_CARD            7   /* RSP:R1b */
#define SDIO_CMD_SWITCH_UHS18V          11  /* RSP:R1 */
#define SDIO_CMD_GO_INACTIVE_STATE      15  /* RSP:no */
#define SDIO_CMD_SEND_TUNING_BLOCK      19  /* RSP:R1 */
#define SDIO_CMD_IO_RW_DIRECT           52  /* RSP:R5 */
#define SDIO_CMD_IO_RW_EXTENDED         53  /* RSP:R5 */
//SD command
#define MMC_CMD_STOP_TRANSMISSION       12

/* rsp list */
#define SDIO_RSP_PRESENT                (1 << 0)
#define SDIO_RSP_136                    (1 << 1)    /* 136 bit response */
#define SDIO_RSP_CRC                    (1 << 2)    /* expect valid crc */
#define SDIO_RSP_BUSY                   (1 << 3)    /* card may send busy */
#define SDIO_RSP_OPCODE                 (1 << 4)    /* response contains opcode */

#define SDIO_RSP_NONE                   (0)
#define SDIO_RSP_R1                     (SDIO_RSP_PRESENT|SDIO_RSP_CRC|SDIO_RSP_OPCODE)
#define SDIO_RSP_R1b                    (SDIO_RSP_PRESENT|SDIO_RSP_CRC|SDIO_RSP_OPCODE|SDIO_RSP_BUSY)
#define SDIO_RSP_R2                     (SDIO_RSP_PRESENT|SDIO_RSP_136|SDIO_RSP_CRC)
#define SDIO_RSP_R3                     (SDIO_RSP_PRESENT)
#define SDIO_RSP_R4                     (SDIO_RSP_PRESENT)
#define SDIO_RSP_R5                     (SDIO_RSP_PRESENT|SDIO_RSP_CRC|SDIO_RSP_OPCODE)
#define SDIO_RSP_R6                     (SDIO_RSP_PRESENT|SDIO_RSP_CRC|SDIO_RSP_OPCODE)
#define SDIO_RSP_R7                     (SDIO_RSP_PRESENT|SDIO_RSP_CRC|SDIO_RSP_OPCODE)

/* OCR: VDD Voltage Window */
#define SDIO_VDD_165_195                0x00000080  /* VDD voltage 1.65 - 1.95 */
#define SDIO_VDD_20_21                  0x00000100  /* VDD voltage 2.0 ~ 2.1 */
#define SDIO_VDD_21_22                  0x00000200  /* VDD voltage 2.1 ~ 2.2 */
#define SDIO_VDD_22_23                  0x00000400  /* VDD voltage 2.2 ~ 2.3 */
#define SDIO_VDD_23_24                  0x00000800  /* VDD voltage 2.3 ~ 2.4 */
#define SDIO_VDD_24_25                  0x00001000  /* VDD voltage 2.4 ~ 2.5 */
#define SDIO_VDD_25_26                  0x00002000  /* VDD voltage 2.5 ~ 2.6 */
#define SDIO_VDD_26_27                  0x00004000  /* VDD voltage 2.6 ~ 2.7 */
#define SDIO_VDD_27_28                  0x00008000  /* VDD voltage 2.7 ~ 2.8 */
#define SDIO_VDD_28_29                  0x00010000  /* VDD voltage 2.8 ~ 2.9 */
#define SDIO_VDD_29_30                  0x00020000  /* VDD voltage 2.9 ~ 3.0 */
#define SDIO_VDD_30_31                  0x00040000  /* VDD voltage 3.0 ~ 3.1 */
#define SDIO_VDD_31_32                  0x00080000  /* VDD voltage 3.1 ~ 3.2 */
#define SDIO_VDD_32_33                  0x00100000  /* VDD voltage 3.2 ~ 3.3 */
#define SDIO_VDD_33_34                  0x00200000  /* VDD voltage 3.3 ~ 3.4 */
#define SDIO_VDD_34_35                  0x00400000  /* VDD voltage 3.4 ~ 3.5 */
#define SDIO_VDD_35_36                  0x00800000  /* VDD voltage 3.5 ~ 3.6 */

#define SDIO_DATA_READ                  1
#define SDIO_DATA_WRITE                 2

#define DATA0_SM_BUSY_TIMEOUT           2000 //ms
#define SEND_CMD_TIMEOUT                1000 //ms
#define REG_CFG_TIMEOUT                 1000 //ms
#define DMA_XFER_TIMEOUT                5000 //ms
#define FIFO_XFER_TIMEOUT               1000 //ms  40ns/byte,1s=23.75MB

#define CMD5_RETRY_CNT_MAX              6

#define __SDMMC_DIV_ROUND_UP(n,d)       (((n) + (d) - 1) / (d))

#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
    #define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif

struct SDIO_CMD_T {
    uint16_t cmdidx;
    uint32_t resp_type;
    uint32_t cmdarg;
    uint32_t response[4];
};

struct SDIO_DATA_T {
    union {
        char *dest;
        const char *src; /* src buffers don't get written to */
    };
    uint32_t flags;
    uint32_t blocks;
    uint32_t blocksize;
};

struct SDIO_CONFIG_T {
    const struct SDIO_OPS_T *ops;
    uint32_t voltages;
    uint32_t f_min;
    uint32_t f_max;
    uint32_t b_max;
};

struct SDIO_T {
    void *priv;
    uint8_t ddr_mode;
    uint32_t clock;
    uint32_t has_init;
    uint16_t block_size;
    uint8_t func_en;
    uint8_t send_abort;
    uint8_t func1_only;
    uint8_t async_int_en;
    uint8_t set_func_blks;
    uint8_t op_cond_pending;   /* 1 if we are waiting on an op_cond command */
    uint8_t init_in_progress;  /* 1 if we have done mmc_start_init() */
    const struct SDIO_CONFIG_T *cfg;
    enum HAL_SDIO_HOST_BUS_WIDTH bus_width;     /* current bus width */
    enum HAL_SDIO_HOST_BUS_WIDTH bus_width_user;/* user config bus width */
};

struct SDIO_IP_HOST_T {
    uint8_t host_id;
    uint32_t src_clk_hz;        /* Clock input to the module */
    uint32_t desired_bus_speed; /* The speed the user wishes to set */
    uint32_t final_bus_speed;   /* The final clock speed on the wire */
    uint32_t div;
    void *ioaddr;
    uint32_t fifoth_val;
    uint32_t period_st_ns;
    uint8_t volt_switch;
    uint8_t blocking_init_en;
    volatile uint8_t volt_switch_flag;
    void *priv;

    struct SDIO_CONFIG_T cfg;
    struct SDIO_T *sdio;

    uint8_t yield;
    uint8_t dma_en;
    uint8_t dma_ch;
    uint8_t dma_in_use;
    uint8_t dma_error;
    volatile uint8_t sdio_dma_lock;
    HAL_DMA_IRQ_HANDLER_T tx_dma_handler;
    HAL_DMA_IRQ_HANDLER_T rx_dma_handler;
};

struct SDIO_OPS_T {
    int (*send_cmd)(struct SDIO_T *sdio, struct SDIO_CMD_T *cmd, struct SDIO_DATA_T *data);
    void (*set_ios_clock)(struct SDIO_T *sdio);
    void (*set_ios_width)(struct SDIO_T *sdio);
    int (*init)(struct SDIO_T *sdio);
};

static struct HAL_SDIO_HOST_CB_T sdio_host_callback_default[HAL_SDIO_HOST_ID_NUM];
static struct HAL_SDIO_HOST_CB_T *sdio_host_callback[HAL_SDIO_HOST_ID_NUM] = {NULL};
static uint32_t sdio_ip_base[HAL_SDIO_HOST_ID_NUM] = {
    SDMMC0_BASE,
#ifdef SDMMC1_BASE
    SDMMC1_BASE,
#endif
};
static struct SDIO_IP_HOST_T sdio_host[HAL_SDIO_HOST_ID_NUM];
static struct SDIO_T sdio_devices[HAL_SDIO_HOST_ID_NUM];

#define SDIO_DMA_LINK_SIZE      16368       //one desc can send and receive 16368(=4092*4) bytes
#define SDIO_DMA_TSIZE_MAX      0xFFC       //4092,must be an integer multiple of 4, otherwise the host rx may have a problem
#ifndef SDIO_DMA_DESC_CNT
    #define SDIO_DMA_DESC_CNT   4           //4*16368=63.93KB
#endif
SYNC_FLAGS_LOC static struct HAL_DMA_DESC_T sdio_dma_desc[HAL_SDIO_HOST_ID_NUM][SDIO_DMA_DESC_CNT];
static void sdio_ip0_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
static void sdio_ip0_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
#ifdef SDMMC1_BASE
static void sdio_ip1_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
static void sdio_ip1_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
#endif

static HAL_DMA_IRQ_HANDLER_T sdio_ip_ext_dma_irq_handlers[HAL_SDIO_HOST_ID_NUM * 2] = {
    sdio_ip0_ext_dma_tx_handler, sdio_ip0_ext_dma_rx_handler,
#ifdef SDMMC1_BASE
    sdio_ip1_ext_dma_tx_handler, sdio_ip1_ext_dma_rx_handler,
#endif
};

#ifdef SDIO_RUN_RECORD
#ifndef SDIO_RUN_RECORD_CNT
#define SDIO_RUN_RECORD_CNT     32
#endif

SRAM_BSS_LOC volatile uint32_t sdio_status_ori[HAL_SDIO_HOST_ID_NUM][SDIO_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t sdio_status_ori_index[HAL_SDIO_HOST_ID_NUM] = {0};

SRAM_BSS_LOC volatile uint8_t  sdio_opcode_ori[HAL_SDIO_HOST_ID_NUM][SDIO_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t sdio_opcode_ori_index[HAL_SDIO_HOST_ID_NUM] = {0};
SRAM_BSS_LOC volatile uint8_t  sdio_opcode_copy[HAL_SDIO_HOST_ID_NUM][SDIO_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t sdio_opcode_copy_index[HAL_SDIO_HOST_ID_NUM] = {0};

SRAM_BSS_LOC volatile uint32_t sdio_resp[HAL_SDIO_HOST_ID_NUM][SDIO_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t sdio_resp_index[HAL_SDIO_HOST_ID_NUM] = {0};

SRAM_BSS_LOC volatile uint8_t sdio_sleep_wakeup_record[HAL_SDIO_HOST_ID_NUM][SDIO_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t sdio_sleep_wakeup_record_index[HAL_SDIO_HOST_ID_NUM] = {0};
#endif
/******************************************************************************/
/************************The first part: end***********************************/
/******************************************************************************/

/******************************************************************************/
/*****************The second part: sdio standard related***********************/
/******************************************************************************/
/* CCCR regiter */
#define SDIO_CCCR_SDIO_VERSION      0x0
#define SDIO_CCCR_SD_VERSION        0x1
#define SDIO_CCCR_IO_ENABLE         0x2
#define SDIO_CCCR_IO_READY          0x3
#define SDIO_CCCR_INT_ENABLE        0x4
#define SDIO_CCCR_INT_PENDING       0x5
#define SDIO_CCCR_IO_ABORT          0x6
#define SDIO_CCCR_BUS_CONTROL       0x7
#define SDIO_CCCR_CARD_CAP          0x8
#define SDIO_CCCR_CIS_PTR           0x9
#define SDIO_CCCR_BUS_SUSPEND       0xc
#define SDIO_CCCR_FUNC_SEL          0xd
#define SDIO_CCCR_EXEC_FLAG         0xe
#define SDIO_CCCR_READY_FLG         0xf
#define SDIO_CCCR_BLK_SIZE          0x10
#define SDIO_CCCR_PWR_CONTROL       0x12
#define SDIO_CCCR_BUS_SPEED_SEL     0x13
#define SDIO_CCCR_UHSI_SUPPORT      0x14
#define SDIO_CCCR_DRV_STRENGTH      0x15
#define SDIO_CCCR_INT_EXTERN        0x16

/* int pending regiter bit*/
#define SDIO_CCCR5_INT1             0x02
#define SDIO_CCCR5_INT2             0x04
#define SDIO_CCCR5_INT3             0x08
#define SDIO_CCCR5_INT4             0x10
#define SDIO_CCCR5_INT5             0x20
#define SDIO_CCCR5_INT6             0x40
#define SDIO_CCCR5_INT7             0x80

/* function1 regiter */
#define SDIO_FUNC1_LOAD_CFG         0x104
#define SDIO_FUNC1_RX_BUF_CFG       0x108
#define SDIO_FUNC1_INT_TO_DEVICE    0x120

#define SDIO_FBR_BASE(x)            ((x) * 0x100)

enum SDIO_DEVICE_BUS_WIDTH {
    SDIO_DEVICE_BUS_WIDTH_1 = 0x0,
    SDIO_DEVICE_BUS_WIDTH_4 = 0x2,
    SDIO_DEVICE_BUS_WIDTH_8 = 0x3,
};

/* CIS tuple code */
enum CIS_TUPLE_CODE {
    CISTPL_NULL     = 0x00, /* Null tuple */
    CISTPL_CHECKSUM = 0x10, /* Checksum control */
    CISTPL_VERS_1   = 0x15, /* Level 1 version/product-information */
    CISTPL_ALTSTR   = 0x16, /* The Alternate Language String Tuple */
    CISTPL_MANFID   = 0x20, /* Manufacturer Identification String Tuple */
    CISTPL_FUNCID   = 0x21, /* Function Identification Tuple */
    CISTPL_FUNCE    = 0x22, /* Function Extensions */
    CISTPL_VUT      = 0x80, /* Vendor Unique Tuples */
    CISTPL_SDIO_STD = 0x91, /* Additional information for functions built to support application specifications for standard SDIO functions */
    CISTPL_SDIO_EXD = 0x92, /* Reserved for future use with SDIO devices */
    CISTPL_END      = 0xff, /* The End-of-chain Tuple */
};

enum FUNCTION_ENABLE_LIST {
    FUNC_DISABLE = 0,
    FUNC_ENABLE = 1,
};

enum FUNCTION_INT_ENABLE_LIST {
    FUNC_INT_DISABLE = 0,
    FUNC_INT_ENABLE = 1,
};

struct SDIO_FUNC_T {
    uint8_t func_num;       /* func id */
    uint8_t func_status;    /* func enable status 0:disable 1:enable  */
    uint8_t func_int_status;/* func int status 0:disable 1:enable */
    uint16_t cur_blk_size;  /* func block size */
    uint16_t max_blk_size;  /* func max block size */
};

struct SDIO_CORE_T {
    uint8_t func_total_num; /* number of func */
    uint8_t cccr_version;   /* CCCR version */
    uint8_t sdio_version;   /* SDIO version*/
    uint8_t sdio_int_ctrl;  /* int control */
    uint16_t manf_code;     /* Manufacturer code */
    uint16_t manf_info;     /* Manufacturer info */
    struct SDIO_FUNC_T *func[HAL_SDIO_FUNC_NUM];
};

/* Mainly store the return value information of the command */
struct SDIO_CARD_RSP_T {
    uint32_t    cmd5r4_io_ocr: 24;
    uint8_t     cmd5r4_s18a: 1;
    uint8_t     cmd5r4_mp: 1;
    uint32_t    cmd5r4_funcs: 3;
    uint8_t     cmd5r4_busy: 1;
    uint16_t    cmd3r6_rca;
    uint8_t     cmd3r6_com_crc_error: 1;
    uint8_t     cmd3r6_illegal_command: 1;
    uint8_t     cmd3r6_error: 1;
    uint32_t    cmd7r1b_card_status;
};

static struct SDIO_FUNC_T host_sdio_func[HAL_SDIO_HOST_ID_NUM][HAL_SDIO_FUNC_NUM];
static struct SDIO_CORE_T host_sdio_core[HAL_SDIO_HOST_ID_NUM];
static struct SDIO_CORE_T *phost_sdio_core = host_sdio_core;
/******************************************************************************/
/************************The second part: end**********************************/
/******************************************************************************/

/******************************************************************************/
/********************sdio host register-level functions************************/
/******************************************************************************/
static inline void sdio_ip_writel(struct SDIO_IP_HOST_T *host, uint32_t reg, uint32_t val)
{
    *((volatile uint32_t *)((uint32_t)(host->ioaddr) + reg)) = val;
}

static inline uint32_t sdio_ip_readl(struct SDIO_IP_HOST_T *host, uint32_t reg)
{
    return *((volatile uint32_t *)((uint32_t)(host->ioaddr) + reg));
}

static enum HAL_SDIO_ERR sdio_ip_check_status(struct SDIO_IP_HOST_T *host, uint32_t timeout_ticks)
{
    uint32_t busy_t;
    uint32_t status;

    busy_t = hal_fast_sys_timer_get();
    do {
        status = sdio_ip_readl(host, SDMMCIP_REG_CMD);
        if (hal_fast_sys_timer_get() - busy_t > timeout_ticks) {
            return HAL_SDIO_CMD_START_TIMEOUT;
        }
    } while (status & (uint32_t)SDMMCIP_REG_CMD_START);

    return HAL_SDIO_ERR_NONE;
}

static int sdio_ip_wait_reset(struct SDIO_IP_HOST_T *host, uint32_t value)
{
    uint32_t ctrl;
    uint32_t timeout = MS_TO_FAST_TICKS(REG_CFG_TIMEOUT), busy_t;

    sdio_ip_writel(host, SDMMCIP_REG_CTRL, value);
    busy_t = hal_fast_sys_timer_get();
    while (hal_fast_sys_timer_get() - busy_t < timeout) {
        ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
        if (!(ctrl & SDMMCIP_REG_RESET_ALL)) {
            return 0;//reset success
        }
    }

    return 1;//reset failure
}

static void sdio_ip_reset_fifo(struct SDIO_IP_HOST_T *host)
{
    uint32_t ctrl;

    ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl |= SDMMCIP_REG_CTRL_FIFO_RESET;
    if (sdio_ip_wait_reset(host, ctrl)) {
        if (sdio_ip_wait_reset(host, ctrl)) {
            HAL_SDIO_HOST_ERROR(0, "%s failed", __func__);
        }
    }
}

static void sdio_ip_reset_dma(struct SDIO_IP_HOST_T *host)
{
    uint32_t ctrl;

    if (host->dma_in_use) {
        host->dma_in_use = 0;
    } else {
        return;//Prevent multiple calls
    }

    //reset sdmmc ip dma
    ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl |= SDMMCIP_REG_CTRL_DMA_RESET;
    if (sdio_ip_wait_reset(host, ctrl)) {
        if (sdio_ip_wait_reset(host, ctrl)) {
            HAL_SDIO_HOST_ERROR(0, "%s failed", __func__);
        }
    }

    //free gpdma channel
    hal_gpdma_free_chan(host->dma_ch);

    //close sdmmc ip dma
    ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl &= ~SDMMCIP_REG_DMA_EN;
    sdio_ip_writel(host, SDMMCIP_REG_CTRL, ctrl);
}

static int sdio_ip_reset_controller(struct SDIO_IP_HOST_T *host)
{
    uint32_t ctrl;
    uint32_t status;
    uint32_t timeout = MS_TO_FAST_TICKS(REG_CFG_TIMEOUT);

    ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl |= SDMMCIP_REG_CTRL_RESET;
    if (sdio_ip_wait_reset(host, ctrl)) {
        if (sdio_ip_wait_reset(host, ctrl)) {
            HAL_SDIO_HOST_ERROR(0, "%s failed", __func__);
            return HAL_SDIO_RESET_FAIL;
        }
    }

    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                    SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    status = sdio_ip_check_status(host, timeout);
    if (status) {
        HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, status);
    }

    return status;
}

static void sdio_base_dma_tx_handler(enum HAL_SDIO_HOST_ID_T id, uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    uint32_t ip_raw_int_status = 0;
    struct SDIO_IP_HOST_T *host = &sdio_host[id];

    ip_raw_int_status = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);
    HAL_SDIO_HOST_TRACE(3, "%s:%d, tx ip_raw_int_status 0x%x", __func__, __LINE__, (uint32_t)ip_raw_int_status);
    if (ip_raw_int_status & (SDMMCIP_REG_DATA_ERR | SDMMCIP_REG_DATA_TOUT)) {
        HAL_SDIO_HOST_ERROR(3, "%s:%d, sdmmcip0 tx dma error 0x%x", __func__, __LINE__, (uint32_t)ip_raw_int_status);
    }

    sdio_ip_reset_dma(host);
    if (sdio_host_callback[id]->hal_sdio_host_dma_tx_done) {
        sdio_host_callback[id]->hal_sdio_host_dma_tx_done(remain_tsize, error);
    } else {
        if (error) {
            host->dma_error = HAL_SDIO_DMA_TX_ERR;
            HAL_SDIO_HOST_ERROR(4, "---tx dma handler, chan:%d, remain:%d, error:%d, lli:0x%X", chan, remain_tsize, error, (uint32_t)lli);
        } else {
            host->sdio_dma_lock = 0;
        }
    }
}

static void sdio_base_dma_rx_handler(enum HAL_SDIO_HOST_ID_T id, uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    uint32_t ip_raw_int_status = 0;
    struct SDIO_IP_HOST_T *host = &sdio_host[id];

    ip_raw_int_status = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);
    HAL_SDIO_HOST_TRACE(3, "%s:%d, ip_raw_int_status 0x%x", __func__, __LINE__, (uint32_t)ip_raw_int_status);
    if (ip_raw_int_status & (SDMMCIP_REG_DATA_ERR | SDMMCIP_REG_DATA_TOUT)) {
        HAL_SDIO_HOST_ERROR(3, "%s:%d, sdmmcip0 rx dma error 0x%x", __func__, __LINE__, (uint32_t)ip_raw_int_status);
    }

    sdio_ip_reset_dma(host);
    if (sdio_host_callback[id]->hal_sdio_host_dma_rx_done) {
        sdio_host_callback[id]->hal_sdio_host_dma_rx_done(remain_rsize, error);
    } else {
        if (error) {
            host->dma_error = HAL_SDIO_DMA_RX_ERR;
            HAL_SDIO_HOST_ERROR(4, "---rx dma handler, chan:%d, remain:%d, error:%d, lli:0x%X", chan, remain_rsize, error, (uint32_t)lli);
        } else {
            host->sdio_dma_lock = 0;
        }
    }
}

static void sdio_ip0_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdio_base_dma_tx_handler(HAL_SDIO_HOST_ID_0, chan, remain_tsize, error, lli);
}

static void sdio_ip0_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdio_base_dma_rx_handler(HAL_SDIO_HOST_ID_0, chan, remain_rsize, error, lli);
}

#ifdef SDMMC1_BASE
static void sdio_ip1_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdio_base_dma_tx_handler(HAL_SDIO_HOST_ID_1, chan, remain_tsize, error, lli);
}

static void sdio_ip1_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdio_base_dma_rx_handler(HAL_SDIO_HOST_ID_1, chan, remain_rsize, error, lli);
}
#endif

static int sdio_ip_prepare_data(struct SDIO_IP_HOST_T *host, struct SDIO_DATA_T *data)
{
    uint8_t  real_src_width;
    uint16_t i;
    uint32_t ctrl;
    uint16_t link_cnt;
    uint32_t remain_transfer_size;
    enum HAL_DMA_RET_T dret;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    uint8_t id;
    uint32_t real_size;

    real_size = data->blocksize * data->blocks;
    sdio_ip_writel(host, SDMMCIP_REG_BLKSIZ, data->blocksize);
    sdio_ip_writel(host, SDMMCIP_REG_BYTCNT, real_size);

    if (host->dma_en) {
        if (host->dma_in_use) {
            return HAL_SDIO_DMA_IN_USE;
        }
        host->sdio_dma_lock = 1;
        id = host->host_id;

        /* enable sdio ip dma function */
        ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
        ctrl |= SDMMCIP_REG_DMA_EN;
        sdio_ip_writel(host, SDMMCIP_REG_CTRL, ctrl);

        memset(&dma_cfg, 0, sizeof(dma_cfg));
        if (data->flags & SDIO_DATA_READ) {
            dma_cfg.dst = (uint32_t)data->dest;
            if (dma_cfg.dst % 4) {
                dma_cfg.dst_width = HAL_DMA_WIDTH_BYTE;
                dma_cfg.dst_bsize = HAL_DMA_BSIZE_32;
            } else {
                dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
                dma_cfg.dst_bsize = HAL_DMA_BSIZE_8;
            }
            dma_cfg.dst_periph = HAL_DMA_PERIPH_NULL;//useless
            dma_cfg.handler = host->rx_dma_handler;
            dma_cfg.src_bsize = HAL_DMA_BSIZE_8;
            if (id == HAL_SDMMC_ID_0) {
                dma_cfg.src_periph = HAL_GPDMA_SDMMC0;
            }
#ifdef SDMMC1_BASE
            else if (id == HAL_SDMMC_ID_1) {
                dma_cfg.src_periph = HAL_GPDMA_SDMMC1;
            }
#endif
            else {
                return HAL_SDIO_INVALID_PARAMETER;
            }
            dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
            dma_cfg.try_burst = 1;
            dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
            dma_cfg.src = 0;//useless
            dma_cfg.ch = hal_gpdma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);
            if (dma_cfg.ch == HAL_DMA_CHAN_NONE) {
                return HAL_SDIO_DMA_CH_ERR;
            }
            real_src_width = dma_cfg.src_width ? dma_cfg.src_width * 2 : 1;

            HAL_SDIO_HOST_TRACE(0, "  ");
            HAL_SDIO_HOST_TRACE(0, "---sdio host use dma read");
            HAL_SDIO_HOST_TRACE(1, "---dma read len      :%d", real_size);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst       :0x%x", dma_cfg.dst);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst_width :%d", dma_cfg.dst_width ? dma_cfg.dst_width * 2 : 1);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src_width :%d", real_src_width);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst_bsize :%d", dma_cfg.dst_bsize ? (2 * (1 << dma_cfg.dst_bsize)) : 1);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src_bsize :%d", dma_cfg.src_bsize ? (2 * (1 << dma_cfg.src_bsize)) : 1);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src_periph:%d", dma_cfg.src_periph);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.ch        :%d", dma_cfg.ch);
            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");

            remain_transfer_size = real_size / 4;//Change according to the value of src_width
            if (remain_transfer_size > SDIO_DMA_TSIZE_MAX) {
                if (remain_transfer_size % SDIO_DMA_TSIZE_MAX) {
                    link_cnt = remain_transfer_size / SDIO_DMA_TSIZE_MAX + 1;
                } else {
                    link_cnt = remain_transfer_size / SDIO_DMA_TSIZE_MAX;
                }
            } else {
                link_cnt = 1;
            }
            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---link_cnt          :%d", link_cnt);

            //Generate dma link configuration
            for (i = 0; i < link_cnt; i++) {
                dma_cfg.dst = (uint32_t)(data->dest + SDIO_DMA_TSIZE_MAX * real_src_width * i);
                if (remain_transfer_size > SDIO_DMA_TSIZE_MAX) {
                    dma_cfg.src_tsize = SDIO_DMA_TSIZE_MAX;
                    remain_transfer_size -= SDIO_DMA_TSIZE_MAX;
                } else {
                    dma_cfg.src_tsize = remain_transfer_size;
                    remain_transfer_size = 0;
                }
                if (i + 1 == link_cnt) {
                    dret = hal_dma_init_desc(&sdio_dma_desc[id][i], &dma_cfg, NULL, 1);
                } else {
                    dret = hal_dma_init_desc(&sdio_dma_desc[id][i], &dma_cfg, &sdio_dma_desc[id][i + 1], 0);
                }
                HAL_SDIO_HOST_ASSERT(dret == HAL_DMA_OK, "%s:%d,sdio dma: Failed to init rx desc %d", __func__, __LINE__, i);
                if (dret != HAL_DMA_OK) {
                    hal_gpdma_free_chan(dma_cfg.ch);
                    HAL_SDIO_HOST_ERROR(0, "%s:%d, error dret:%d", __func__, __LINE__, dret);
                    return HAL_SDIO_DMA_DESC_ERR;
                }

                HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.dst       :0x%x", dma_cfg.dst);
                HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.src_tsize :%d", dma_cfg.src_tsize);
                HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "sdio_dma_desc[%d][%d].src/dst/lli/ctrl=0x%08X 0x%08X 0x%08X 0x%08X@0x%08X", id, i,
                                    sdio_dma_desc[id][i].src, sdio_dma_desc[id][i].dst, sdio_dma_desc[id][i].lli, sdio_dma_desc[id][i].ctrl, (uint32_t)&sdio_dma_desc[id][i]);
            }
            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
        } else {
            dma_cfg.dst = 0;//useless
            dma_cfg.dst_bsize = HAL_DMA_BSIZE_8;
            if (id == HAL_SDMMC_ID_0) {
                dma_cfg.dst_periph = HAL_GPDMA_SDMMC0;
            }
#ifdef SDMMC1_BASE
            else if (id == HAL_SDMMC_ID_1) {
                dma_cfg.dst_periph = HAL_GPDMA_SDMMC1;
            }
#endif
            else {
                return HAL_SDIO_INVALID_PARAMETER;
            }
            dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
            dma_cfg.handler = host->tx_dma_handler;
            dma_cfg.src_periph = HAL_DMA_PERIPH_NULL;//useless
            dma_cfg.try_burst = 0;//M2P mode is useless
            dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
            dma_cfg.src = (uint32_t)data->src;
            if (dma_cfg.src % 4) {
                dma_cfg.src_bsize = HAL_DMA_BSIZE_32;
                dma_cfg.src_width = HAL_DMA_WIDTH_BYTE;
                remain_transfer_size = real_size;
            } else {
                dma_cfg.src_bsize = HAL_DMA_BSIZE_8;
                dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
                remain_transfer_size = real_size / 4;
            }
            dma_cfg.ch = hal_gpdma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);
            if (dma_cfg.ch == HAL_DMA_CHAN_NONE) {
                return HAL_SDIO_DMA_CH_ERR;
            }
            real_src_width = dma_cfg.src_width ? dma_cfg.src_width * 2 : 1;

            HAL_SDIO_HOST_TRACE(0, "  ");
            HAL_SDIO_HOST_TRACE(0, "---sdio host use dma write");
            HAL_SDIO_HOST_TRACE(1, "---dma write len     :%d", real_size);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src       :0x%x", dma_cfg.src);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst_width :%d", dma_cfg.dst_width ? dma_cfg.dst_width * 2 : 1);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src_width :%d", real_src_width);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst_bsize :%d", dma_cfg.dst_bsize ? (2 * (1 << dma_cfg.dst_bsize)) : 1);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src_bsize :%d", dma_cfg.src_bsize ? (2 * (1 << dma_cfg.src_bsize)) : 1);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst_periph:%d", dma_cfg.dst_periph);
            HAL_SDIO_HOST_TRACE(1, "---dma_cfg.ch        :%d", dma_cfg.ch);
            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");

            if (remain_transfer_size > SDIO_DMA_TSIZE_MAX) {
                if (remain_transfer_size % SDIO_DMA_TSIZE_MAX) {
                    link_cnt = remain_transfer_size / SDIO_DMA_TSIZE_MAX + 1;
                } else {
                    link_cnt = remain_transfer_size / SDIO_DMA_TSIZE_MAX;
                }
            } else {
                link_cnt = 1;
            }
            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---link_cnt          :%d", link_cnt);

            //Generate dma link configuration
            for (i = 0; i < link_cnt; i++) {
                dma_cfg.src = (uint32_t)(data->src + SDIO_DMA_TSIZE_MAX * real_src_width * i);
                if (remain_transfer_size > SDIO_DMA_TSIZE_MAX) {
                    dma_cfg.src_tsize = SDIO_DMA_TSIZE_MAX;
                    remain_transfer_size -= SDIO_DMA_TSIZE_MAX;
                } else {
                    dma_cfg.src_tsize = remain_transfer_size;
                    remain_transfer_size = 0;
                }
                if (i + 1 == link_cnt) {
                    dret = hal_dma_init_desc(&sdio_dma_desc[id][i], &dma_cfg, NULL, 1);
                } else {
                    dret = hal_dma_init_desc(&sdio_dma_desc[id][i], &dma_cfg, &sdio_dma_desc[id][i + 1], 0);
                }
                HAL_SDIO_HOST_ASSERT(dret == HAL_DMA_OK, "%s:%d,sdio dma: Failed to init rx desc %d", __func__, __LINE__, i);
                if (dret != HAL_DMA_OK) {
                    hal_gpdma_free_chan(dma_cfg.ch);
                    HAL_SDIO_HOST_ERROR(0, "%s:%d, error dret:%d", __func__, __LINE__, dret);
                    return HAL_SDIO_DMA_DESC_ERR;
                }

                HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.src       :0x%x", dma_cfg.src);
                HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.src_tsize :%d", dma_cfg.src_tsize);
                HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "sdio_dma_desc[%d][%d].src/dst/lli/ctrl=0x%08X 0x%08X 0x%08X 0x%08X@0x%08X", id, i,
                                    sdio_dma_desc[id][i].src, sdio_dma_desc[id][i].dst, sdio_dma_desc[id][i].lli, sdio_dma_desc[id][i].ctrl, (uint32_t)&sdio_dma_desc[id][i]);
            }
            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
        }

        host->dma_in_use = 1;
        host->dma_ch = dma_cfg.ch;
        dret = hal_gpdma_sg_start(&sdio_dma_desc[id][0], &dma_cfg);
        if (dret != HAL_DMA_OK) {
            hal_gpdma_free_chan(dma_cfg.ch);
            HAL_SDIO_HOST_ERROR(0, "%s:%d, error dret:%d", __func__, __LINE__, dret);
            return HAL_SDIO_DMA_START_ERR;
        }
    }

    return HAL_SDIO_ERR_NONE;
}

static int sdio_ip_set_transfer_mode(struct SDIO_IP_HOST_T *host, struct SDIO_DATA_T *data)
{
    uint32_t mode;

    mode = SDMMCIP_REG_CMD_DATA_EXP;
    if (data->flags & SDIO_DATA_WRITE)
        mode |= SDMMCIP_REG_CMD_RW;

    return mode;
}

static void sdio_ip_func_int_mask(enum HAL_SDIO_HOST_ID_T id, uint8_t mask0_unmask1)
{
    uint32_t mask;
    struct SDIO_IP_HOST_T *host = &sdio_host[id];

    mask = sdio_ip_readl(host, SDMMCIP_REG_INTMASK);
    if (mask0_unmask1) {
        //unmask(enable) func interrupt
        mask |= (SDMMCIP_REG_INTMSK_SDIO_FUNC1 | SDMMCIP_REG_INTMSK_SDIO_FUNC2
                 | SDMMCIP_REG_INTMSK_SDIO_FUNC3 | SDMMCIP_REG_INTMSK_SDIO_FUNC4
                 | SDMMCIP_REG_INTMSK_SDIO_FUNC5 | SDMMCIP_REG_INTMSK_SDIO_FUNC6
                 | SDMMCIP_REG_INTMSK_SDIO_FUNC7);
    } else {
        //mask(disable) func interrupt
        mask &= ~(SDMMCIP_REG_INTMSK_SDIO_FUNC1 | SDMMCIP_REG_INTMSK_SDIO_FUNC2
                  | SDMMCIP_REG_INTMSK_SDIO_FUNC3 | SDMMCIP_REG_INTMSK_SDIO_FUNC4
                  | SDMMCIP_REG_INTMSK_SDIO_FUNC5 | SDMMCIP_REG_INTMSK_SDIO_FUNC6
                  | SDMMCIP_REG_INTMSK_SDIO_FUNC7);
    }
    sdio_ip_writel(host, SDMMCIP_REG_INTMASK, mask);
}

POSSIBLY_UNUSED static void sdio_ip_dto_int_cfg(enum HAL_SDIO_HOST_ID_T id, int en)
{
    uint32_t mask;
    struct SDIO_IP_HOST_T *host = &sdio_host[id];

    mask = sdio_ip_readl(host, SDMMCIP_REG_INTMASK);
    if (en) {
        //unmask(enable) dto interrupt
        mask |= SDMMCIP_REG_RINTSTS_DTO;
    } else {
        //mask(disable) dto interrupt
        mask &= ~SDMMCIP_REG_RINTSTS_DTO;
    }
    sdio_ip_writel(host, SDMMCIP_REG_INTMASK, mask);
}

static int sdio_ip_send_cmd(struct SDIO_T *sdio, struct SDIO_CMD_T *cmd, struct SDIO_DATA_T *data)
{
    int ret = 0;
    int flags = 0;
    uint32_t mask = 0;
    uint32_t status;
    uint32_t cmd_reg;
    uint32_t busy_timeout, busy_t;
    struct SDIO_IP_HOST_T *host = sdio->priv;
#ifdef POLLING_TIME_MEASUER
    uint32_t time_start = 0, time_run;
    int time_measuer_en = 0;
#endif

#ifdef SDIO_RUN_RECORD
    sdio_opcode_ori[host->host_id][sdio_opcode_ori_index[host->host_id]++] = cmd->cmdidx;
    if (sdio_opcode_ori_index[host->host_id] >= SDIO_RUN_RECORD_CNT) {
        sdio_opcode_ori_index[host->host_id] = 0;
    }
#endif

    //Check if data0 or state machine is busy
    busy_timeout = MS_TO_FAST_TICKS(DATA0_SM_BUSY_TIMEOUT);
    busy_t = hal_fast_sys_timer_get();
    while (1) {
        status = sdio_ip_readl(host, SDMMCIP_REG_STATUS);
        cmd_reg = sdio_ip_readl(host, SDMMCIP_REG_CMD);
        if ((status & (SDMMCIP_REG_DATA0_BUSY | SDMMCIP_REG_STATE_MACHINE_BUSY)) || (cmd_reg & SDMMCIP_REG_CMD_START)) {
            if (hal_fast_sys_timer_get() - busy_t < busy_timeout) {
                HAL_SDIO_HOST_TRACE(0, "CMD%d, sdio %s busy", cmd->cmdidx, (status & SDMMCIP_REG_DATA0_BUSY) ? "data0" : "state machine");
                //hal_sys_timer_delay_us(10);
            } else {
                HAL_SDIO_HOST_ERROR(0, "%s: CMD%d timeout", __func__, cmd->cmdidx);
                if (status & SDMMCIP_REG_DATA0_BUSY) {
                    HAL_SDIO_HOST_ERROR(0, "%s: sdio data0 busy, %dms timeout, status:0x%X", __func__, DATA0_SM_BUSY_TIMEOUT, status);
                    return HAL_SDIO_DATA0_BUSY;
                } else {
                    HAL_SDIO_HOST_ERROR(0, "%s: sdio cmd start error, %dms timeout, cmd reg:0x%X", __func__, DATA0_SM_BUSY_TIMEOUT, cmd_reg);
                    return HAL_SDIO_CMD_START_ERROR;
                }
            }

            if (status & SDMMCIP_REG_STATE_MACHINE_BUSY) {
                HAL_SDIO_HOST_ERROR(0, "%s: sdio data state machine busy, status:0x%X", __func__, status);
                HAL_SDIO_HOST_ERROR(0, "FIFO count:%d", (status & 0x3FFE0000) >> 17);
                HAL_SDIO_HOST_ERROR(0, "REG_20 BYTCNT:%d", sdio_ip_readl(host, SDMMCIP_REG_BYTCNT));
                HAL_SDIO_HOST_ERROR(0, "REG_5C TCBCNT:%d", sdio_ip_readl(host, SDMMCIP_REG_TCMCNT));
                HAL_SDIO_HOST_ERROR(0, "REG_60 TBBCNT:%d", sdio_ip_readl(host, SDMMCIP_REG_TBBCNT));
                ret = sdio_ip_reset_controller(host);
                if (ret) {
                    return (ret << 8) | HAL_SDIO_DATA_STE_MACHINE_BUSY;
                } else {
                    break;
                }
            }
        } else {
            break;
        }
    }

    if (host->dma_en) {
        sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_CDONE |
                       SDMMCIP_REG_RINTSTS_RTO | SDMMCIP_REG_RINTSTS_RE); //clear command flag
    } else {
        sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_ALL);//clear interrupt status
    }
    sdio_ip_writel(host, SDMMCIP_REG_CMDARG, cmd->cmdarg);

    if (data) {
        flags = sdio_ip_set_transfer_mode(host, data);
    }

    if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION) {
        flags |= SDMMCIP_REG_CMD_ABORT_STOP;
    } else {
        flags |= SDMMCIP_REG_CMD_PRV_DAT_WAIT;
    }

    if (cmd->cmdidx == SDIO_CMD_SWITCH_UHS18V) {
        flags |= SDMMCIP_REG_CMD_VOLT_SWITCH;
    }

    if (cmd->resp_type & SDIO_RSP_PRESENT) {
        flags |= SDMMCIP_REG_CMD_RESP_EXP;
        if (cmd->resp_type & SDIO_RSP_136) {
            flags |= SDMMCIP_REG_CMD_RESP_LENGTH;
        }
    }

    if (cmd->resp_type & SDIO_RSP_CRC) {
        flags |= SDMMCIP_REG_CMD_CHECK_CRC;
    }

    if (data) {
        ret = sdio_ip_prepare_data(host, data);
        if (ret) {
            HAL_SDIO_HOST_ERROR(0, "%s, error:%d", __func__, ret);
            return ret;
        }
    }

    flags |= (cmd->cmdidx | SDMMCIP_REG_CMD_START | SDMMCIP_REG_CMD_USE_HOLD_REG);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, flags);
    busy_timeout = MS_TO_FAST_TICKS(SEND_CMD_TIMEOUT);
    busy_t = hal_fast_sys_timer_get();
    while (cmd->cmdidx != SDIO_CMD_SWITCH_UHS18V) {
        //When there is subsequent data transmission, no longer wait for cmd resp
        if (cmd->cmdidx == SDIO_CMD_IO_RW_EXTENDED) {
            break;
        }
        mask = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);
        if (mask & SDMMCIP_REG_RINTSTS_CDONE) {
            sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_CDONE);
            break;
        } else if (hal_fast_sys_timer_get() - busy_t > busy_timeout) {
            HAL_SDIO_HOST_ERROR(1, "%s: CMD%d Timeout", __func__, cmd->cmdidx);
            HAL_SDIO_HOST_ERROR(1, "sdio raw int status:0x%X", sdio_ip_readl(host, SDMMCIP_REG_RINTSTS));
            HAL_SDIO_HOST_ERROR(1, "sdio status        :0x%X", sdio_ip_readl(host, SDMMCIP_REG_STATUS));

            sdio_ip_reset_fifo(host);
            if (data) {
                if (host->dma_en) {
                    sdio_ip_reset_dma(host);
                }
            }
            return HAL_SDIO_CMD_SEND_TIMEOUT;
        } else {
            ;
        }
    }

    if (mask & SDMMCIP_REG_RINTSTS_RTO) {
        /*
         * Timeout here is not necessarily fatal. (e)MMC cards
         * will splat here when they receive CMD55 as they do
         * not support this command and that is exactly the way
         * to tell them apart from SD cards. Thus, this output
         * below shall be TRACE(). eMMC cards also do not favor
         * CMD8, please keep that in mind.
         */
        HAL_SDIO_HOST_ERROR(1, "%s: CMD%d Response Timeout", __func__, cmd->cmdidx);
        HAL_SDIO_HOST_ERROR(1, "sdio raw int status:0x%X", sdio_ip_readl(host, SDMMCIP_REG_RINTSTS));
        HAL_SDIO_HOST_ERROR(1, "sdio status        :0x%X", sdio_ip_readl(host, SDMMCIP_REG_STATUS));

        sdio_ip_reset_fifo(host);
        if (data) {
            if (host->dma_en) {
                sdio_ip_reset_dma(host);
            }
        }
        return HAL_SDIO_RESPONSE_TIMEOUT;
    } else if (mask & SDMMCIP_REG_RINTSTS_RE) {
        HAL_SDIO_HOST_ERROR(1, "%s: CMD%d Response Error", __func__, cmd->cmdidx);
        HAL_SDIO_HOST_ERROR(1, "sdio raw int status:0x%X", sdio_ip_readl(host, SDMMCIP_REG_RINTSTS));
        HAL_SDIO_HOST_ERROR(1, "sdio status        :0x%X", sdio_ip_readl(host, SDMMCIP_REG_STATUS));

        sdio_ip_reset_fifo(host);
        if (data) {
            if (host->dma_en) {
                sdio_ip_reset_dma(host);
            }
        }
        return HAL_SDIO_RESPONSE_ERR;
    }

    if (cmd->resp_type & SDIO_RSP_PRESENT) {
        if (cmd->resp_type & SDIO_RSP_136) {
            /*
                1. This driver does not use these 4 resp
                2. This change is to be consistent with the emmc/sdmmc driver
            */
            cmd->response[0] = sdio_ip_readl(host, SDMMCIP_REG_RESP3);//high
            cmd->response[1] = sdio_ip_readl(host, SDMMCIP_REG_RESP2);
            cmd->response[2] = sdio_ip_readl(host, SDMMCIP_REG_RESP1);
            cmd->response[3] = sdio_ip_readl(host, SDMMCIP_REG_RESP0);//low
        } else {
            if (cmd->cmdidx != SDIO_CMD_IO_RW_EXTENDED) {
                cmd->response[0] = sdio_ip_readl(host, SDMMCIP_REG_RESP0);
            }
        }
#ifdef SDIO_RUN_RECORD
        sdio_resp[host->host_id][sdio_resp_index[host->host_id]++] = sdio_ip_readl(host, SDMMCIP_REG_RESP0);
        if (sdio_resp_index[host->host_id] >= SDIO_RUN_RECORD_CNT) {
            sdio_resp_index[host->host_id] = 0;
        }
#endif
    }

    if (data) {
        //Processing after the completion of the transfer process: use dma
        if (host->dma_en) {
            if ((sdio_host_callback[host->host_id]->hal_sdio_host_dma_tx_done) && (data->flags == SDIO_DATA_WRITE)) {
                //Non-blocking mode: there will be a callback function after the transmission is completed
            } else if ((sdio_host_callback[host->host_id]->hal_sdio_host_dma_rx_done) && (data->flags == SDIO_DATA_READ)) {
                //Non-blocking mode: there will be a callback function after the transmission is completed
            } else {
                busy_timeout = MS_TO_FAST_TICKS(DMA_XFER_TIMEOUT);
                busy_t = hal_fast_sys_timer_get();
                while (host->sdio_dma_lock) {
                    if (host->yield) {
                        /*
                            1) here can be replaced by a semaphore operation
                            2) when using semaphores, you may need to lock to prevent sleep, such as
                                hal_bus_wake_lock(user);
                                hal_bus_wake_unlock(user);
                        */
                        osDelay(1);
                    }

                    while (hal_fast_sys_timer_get() - busy_t > busy_timeout) {
                        sdio_ip_reset_dma(host);
                        sdio_ip_reset_fifo(host);
                        ret = HAL_SDIO_DMA_XFER_TIMEOUT;
                        HAL_SDIO_HOST_ERROR(0, "%s: DMA transfer timeout:%dms", __func__, DMA_XFER_TIMEOUT);
                        goto out;
                    }

                    //Exit timeout waiting in time when there is an error in DMA transmission
                    if (host->dma_error) {
                        ret = host->dma_error;
                        host->dma_error = 0;
                        sdio_ip_reset_fifo(host);
                        HAL_SDIO_HOST_ERROR(0, "%s: DMA transfer error:%d", __func__, ret);
                        goto out;
                    }
                }
            }
            ret = 0;
        } else {
            //Processing after the completion of the transfer process: do not use dma
            int has_data = 0;
            uint32_t i = 0;
            uint32_t fifo_data;
            uint32_t data_size;
            uint32_t byte_in_fifo;
#ifdef POLLING_TIME_MEASUER
            HAL_SDIO_HOST_ERROR(0, "======polling start");
            time_measuer_en = 1;
            time_start = hal_fast_sys_timer_get();
#endif
            data_size = data->blocks * data->blocksize;
            busy_timeout = MS_TO_FAST_TICKS(FIFO_XFER_TIMEOUT);
            busy_t = hal_fast_sys_timer_get();
            while (1) {
                mask = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);
                if (mask & (SDMMCIP_REG_DATA_ERR | SDMMCIP_REG_DATA_TOUT)) {
                    HAL_SDIO_HOST_ERROR(0, "***%s: READ DATA ERROR!", __func__);
                    ret = HAL_SDIO_READ_DATA_ERR;
                    goto out;
                }
                status = sdio_ip_readl(host, SDMMCIP_REG_STATUS);
                byte_in_fifo = (status & SDMMCIP_REG_FIFO_COUNT_MASK) >> SDMMCIP_REG_FIFO_COUNT_SHIFT;
                byte_in_fifo *= sizeof(fifo_data);//Change it to the number of bytes
                if (data->flags == SDIO_DATA_READ) {
                    if (byte_in_fifo) {
                        while (byte_in_fifo) {
                            fifo_data = sdio_ip_readl(host, SDMMCIP_REG_FIFO_OFFSET);
                            if (i < data_size) {
                                memcpy(data->dest + i, &fifo_data, sizeof(fifo_data));
                                i += sizeof(fifo_data);
                                if (byte_in_fifo >= sizeof(fifo_data)) {
                                    byte_in_fifo -= sizeof(fifo_data);
                                } else {
                                    byte_in_fifo -= byte_in_fifo;
                                }
                            } else {
                                HAL_SDIO_HOST_ERROR(0, "***%s: fifo data too much", __func__);
                                ret = HAL_SDIO_FIFO_OVERFLOW;
                                goto out;
                            }
                        }
                        has_data = 1;
                    } else if (mask & SDMMCIP_REG_RINTSTS_DTO) {
                        /* nothing to read from fifo and DTO is set */
                        if (i != data_size) {
                            HAL_SDIO_HOST_ERROR(0, "***%s: need to read %d, actually read %d", __func__, data->blocks * data->blocksize, i);
                        }
                        ret = HAL_SDIO_ERR_NONE;
                        goto out;
                    } else {
                        has_data = 0;
                    }
                } else {
                    /* nothing to write to fifo and DTO is set */
                    if (mask & SDMMCIP_REG_RINTSTS_DTO) {
                        /* check if number is right */
                        if (i != data_size) {
                            HAL_SDIO_HOST_ERROR(0, "***%s: need to write %d, actually written %d", __func__, data->blocks * data->blocksize, i);
                        }
                        ret = HAL_SDIO_ERR_NONE;
                        goto out;
                    } else {
                        if (byte_in_fifo <= SDIO_HOST_FIFO_BYTES) {
                            byte_in_fifo = SDIO_HOST_FIFO_BYTES - byte_in_fifo;
                            has_data = 1;
                        } else {
                            HAL_SDIO_HOST_ERROR(0, "***%s: fifo data cnt error", __func__);
                            ret = HAL_SDIO_FIFO_DATA_CNT_ERR;
                            goto out;
                        }

                        while (byte_in_fifo) {
                            if (i < data_size) {
                                memcpy(&fifo_data, data->src + i, sizeof(fifo_data));
                                i += sizeof(fifo_data);
                                sdio_ip_writel(host, SDMMCIP_REG_FIFO_OFFSET, fifo_data);

                                if (byte_in_fifo >= sizeof(fifo_data)) {
                                    byte_in_fifo -= sizeof(fifo_data);
                                } else {
                                    byte_in_fifo -= byte_in_fifo;
                                }
                            } else {
                                HAL_SDIO_HOST_TRACE(0, "%s: no data to write to fifo, do nothing", __func__);
                                break;
                            }
                        }
                    }
                }

                /* fifo xfer timeout */
                if ((!has_data) && (hal_fast_sys_timer_get() - busy_t > busy_timeout)) {
                    sdio_ip_reset_fifo(host);
                    ret = HAL_SDIO_FIFO_XFER_TIMEOUT;
                    HAL_SDIO_HOST_ERROR(0, "***%s: fifo xfer timeout", __func__);
                    goto out;
                }
            }
        }
    }

out:
#ifdef POLLING_TIME_MEASUER
    if (time_measuer_en) {
        time_run = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
        HAL_SDIO_HOST_ERROR(0, "======polling rx/tx takes: %uus", time_run);
    }
#endif

#ifdef SDIO_RUN_RECORD
    sdio_opcode_copy[host->host_id][sdio_opcode_copy_index[host->host_id]++] = cmd->cmdidx;
    if (sdio_opcode_copy_index[host->host_id] >= SDIO_RUN_RECORD_CNT) {
        sdio_opcode_copy_index[host->host_id] = 0;
    }
#endif

    return ret;
}

static int sdio_ip_setup_bus(struct SDIO_IP_HOST_T *host, uint32_t freq)
{
    uint32_t div, status;
    uint32_t timeout = MS_TO_FAST_TICKS(REG_CFG_TIMEOUT);
    uint32_t sclk;

    if ((freq == host->desired_bus_speed) || (freq == 0))
        return HAL_SDIO_ERR_NONE;
    if (host->src_clk_hz)
        sclk = host->src_clk_hz;
    else {
        HAL_SDIO_HOST_ERROR(1, "%s: Didn't get source clock value.", __func__);
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (sclk <= freq)
        div = 0;    /* bypass mode */
    else
        div = __SDMMC_DIV_ROUND_UP(sclk, 2 * freq);

    HAL_SDIO_HOST_TRACE(5, "%s: freq %d, sclk %d, reg div %d, final div %d", __func__, freq, sclk, div, div * 2);
    host->div = div * 2;
    host->final_bus_speed = host->div ? sclk / host->div : sclk;

    sdio_ip_writel(host, SDMMCIP_REG_CLKENA, 0);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    status = sdio_ip_check_status(host, timeout);
    if (status) {
        HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, status);
        return status;
    }

    sdio_ip_writel(host, SDMMCIP_REG_CLKDIV, div);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    status = sdio_ip_check_status(host, timeout);
    if (status) {
        HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, status);
        return status;
    }

    sdio_ip_writel(host, SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE |
                   SDMMCIP_REG_CLKEN_LOW_PWR);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    status = sdio_ip_check_status(host, timeout);
    if (status) {
        HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, status);
        return status;
    }

    host->desired_bus_speed = freq;

    return HAL_SDIO_ERR_NONE;
}

#ifdef CORE_SLEEP_POWER_DOWN
static int sdio_ip_setup_div(struct SDIO_IP_HOST_T *host, uint32_t div)
{
    uint32_t status;
    uint32_t timeout = MS_TO_FAST_TICKS(REG_CFG_TIMEOUT);

    sdio_ip_writel(host, SDMMCIP_REG_CLKDIV, div);
    sdio_ip_writel(host, SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE |
                   SDMMCIP_REG_CLKEN_LOW_PWR);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    status = sdio_ip_check_status(host, timeout);
    if (status) {
        HAL_SDIO_HOST_ERROR(0, "%s: error:%d", __func__, status);
        return status;
    }

    return HAL_SDIO_ERR_NONE;
}
#endif

static int sdio_ip_clk_disable_low_power(struct SDIO_IP_HOST_T *host)
{
    uint32_t status;
    uint32_t timeout = MS_TO_FAST_TICKS(REG_CFG_TIMEOUT);

    sdio_ip_writel(host, SDMMCIP_REG_CLKENA, 0);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    status = sdio_ip_check_status(host, timeout);
    if (status) {
        HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, status);
        return status;
    }

    sdio_ip_writel(host, SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    status = sdio_ip_check_status(host, timeout);
    if (status) {
        HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, status);
        return status;
    }

    return HAL_SDIO_ERR_NONE;
}

static int sdio_ip_wait_cmd_start(struct SDIO_IP_HOST_T *host, uint8_t en_volt_switch)
{
    uint32_t status;
    uint32_t timeout = MS_TO_FAST_TICKS(REG_CFG_TIMEOUT);

    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START |
                   (en_volt_switch ? SDMMCIP_REG_CMD_VOLT_SWITCH : 0));

    status = sdio_ip_check_status(host, timeout);
    if (status) {
        HAL_SDIO_HOST_ERROR(0, "%s: error:%d", __func__, status);
        return status;
    }

    return HAL_SDIO_ERR_NONE;
}

static int sdio_ip_wait_clk_update(struct SDIO_IP_HOST_T *host)
{
    uint32_t status;
    uint32_t timeout = MS_TO_FAST_TICKS(REG_CFG_TIMEOUT);

    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    status = sdio_ip_check_status(host, timeout);
    if (status) {
        HAL_SDIO_HOST_ERROR(0, "%s: error:%d", __func__, status);
        return status;
    }

    return HAL_SDIO_ERR_NONE;
}

static void sdio_ip_set_ios_clock(struct SDIO_T *sdio)
{
    struct SDIO_IP_HOST_T *host = (struct SDIO_IP_HOST_T *)sdio->priv;

    HAL_SDIO_HOST_TRACE(2, "%s, set clock: %dHz", __func__, sdio->clock);
    if (sdio->clock) {
        sdio_ip_setup_bus(host, sdio->clock);
    }
}

static void sdio_ip_set_ios_width(struct SDIO_T *sdio)
{
    uint32_t ctype, regs;
    struct SDIO_IP_HOST_T *host = (struct SDIO_IP_HOST_T *)sdio->priv;

    HAL_SDIO_HOST_TRACE(2, "%s, set bus width: %d", __func__, sdio->bus_width);
    if (sdio->bus_width) {
        switch (sdio->bus_width) {
            case HAL_SDIO_HOST_BUS_WIDTH_4:
                ctype = SDMMCIP_REG_CTYPE_4BIT;
                break;
            default:
                ctype = SDMMCIP_REG_CTYPE_1BIT;
                break;
        }

        sdio_ip_writel(host, SDMMCIP_REG_CTYPE, ctype);
        regs = sdio_ip_readl(host, SDMMCIP_REG_UHS_REG);
        if (sdio->ddr_mode)
            regs |= SDMMCIP_REG_DDR_MODE;
        else
            regs &= ~SDMMCIP_REG_DDR_MODE;
        sdio_ip_writel(host, SDMMCIP_REG_UHS_REG, regs);
    }
}

static int sdio_ip_init(struct SDIO_T *sdio)
{
    struct SDIO_IP_HOST_T *host = sdio->priv;

    sdio_ip_writel(host, SDMMCIP_REG_PWREN, 1);
    HAL_SDIO_HOST_TRACE(2, "%s, host->ioaddr:0x%X, ip_pwr:%d", __func__, (uint32_t)host->ioaddr, sdio_ip_readl(host, SDMMCIP_REG_PWREN));

    if (sdio_ip_wait_reset(host, SDMMCIP_REG_RESET_ALL)) {
        HAL_SDIO_HOST_ASSERT(0, "%s:%d,Fail-reset!!", __func__, __LINE__);
        return HAL_SDIO_RESET_FAIL;
    }

    sdio_ip_writel(host, SDMMCIP_REG_INTMASK, ~SDMMCIP_REG_INTMSK_ALL); //disable all int
    sdio_ip_writel(host, SDMMCIP_REG_TMOUT, SDMMCIP_REG_RINTSTS_ALL);   //modify response timeout value for maximum
    sdio_ip_writel(host, SDMMCIP_REG_IDINTEN, 0);   //disable internal DMAC interrupt
    sdio_ip_writel(host, SDMMCIP_REG_BMOD, 1);      //software reset internal DMA controller
    sdio_ip_writel(host, SDMMCIP_REG_FIFOTH, host->fifoth_val);

    return HAL_SDIO_ERR_NONE;
}

static const struct SDIO_OPS_T sdio_ip_ops = {
    .send_cmd       = sdio_ip_send_cmd,
    .set_ios_clock  = sdio_ip_set_ios_clock,
    .set_ios_width  = sdio_ip_set_ios_width,
    .init           = sdio_ip_init,
};

/******************************************************************************/
/**************************sdio host hal layer function************************/
/******************************************************************************/
static int sdio_send_cmd(struct SDIO_T *sdio, struct SDIO_CMD_T *cmd, struct SDIO_DATA_T *data)
{
    int ret;

    HAL_SDIO_HOST_TRACE(1, "CMD_SEND:%d", cmd->cmdidx);
    HAL_SDIO_HOST_TRACE(1, "\t\tARG\t\t\t 0x%08X", cmd->cmdarg);
    ret = sdio->cfg->ops->send_cmd(sdio, cmd, data);
    HAL_SDIO_HOST_TRACE(0, "send cmd end...");
    switch (cmd->resp_type) {
        case SDIO_RSP_NONE:
            HAL_SDIO_HOST_TRACE(0, "\t\tMMC_RSP_NONE");
            break;
        case SDIO_RSP_R1:
            HAL_SDIO_HOST_TRACE(1, "\t\tMMC_RSP_R1,5,6,7 \t 0x%08X ",
                                cmd->response[0]);
            break;
        case SDIO_RSP_R1b:
            HAL_SDIO_HOST_TRACE(1, "\t\tMMC_RSP_R1b\t\t 0x%08X ",
                                cmd->response[0]);
            break;
        case SDIO_RSP_R2:
            HAL_SDIO_HOST_TRACE(1, "\t\tMMC_RSP_R2\t\t 0x%08X ",
                                cmd->response[0]);
            HAL_SDIO_HOST_TRACE(1, "\t\t          \t\t 0x%08X ",
                                cmd->response[1]);
            HAL_SDIO_HOST_TRACE(1, "\t\t          \t\t 0x%08X ",
                                cmd->response[2]);
            HAL_SDIO_HOST_TRACE(1, "\t\t          \t\t 0x%08X ",
                                cmd->response[3]);
            HAL_SDIO_HOST_TRACE(0, "  ");
            HAL_SDIO_HOST_TRACE(0, "\t\t\t\t\tDUMPING DATA");
#ifdef SDIO_HOST_DEBUG
            uint8_t i;
            uint8_t *ptr;
            for (i = 0; i < 4; i++) {
                uint8_t j;
                HAL_SDIO_HOST_TRACE(TR_ATTR_NO_LF, "\t\t\t\t\t%03d - ", i * 4);
                ptr = (uint8_t *)&cmd->response[i];
                ptr += 3;
                for (j = 0; j < 4; j++)
                    HAL_SDIO_HOST_TRACE(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, "%02X ", *ptr--);
                HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "   ");
            }
#endif
            break;
        case SDIO_RSP_R3:
            HAL_SDIO_HOST_TRACE(1, "\t\tMMC_RSP_R3,4\t\t 0x%08X ",
                                cmd->response[0]);
            break;
        default:
            HAL_SDIO_HOST_TRACE(0, "\t\tERROR MMC rsp not supported");
            break;
    }
    return ret;
}

static void sdio_set_ios_clock(struct SDIO_T *sdio)
{
    if (sdio->cfg->ops->set_ios_clock)
        sdio->cfg->ops->set_ios_clock(sdio);
}

static void sdio_set_ios_width(struct SDIO_T *sdio)
{
    if (sdio->cfg->ops->set_ios_width)
        sdio->cfg->ops->set_ios_width(sdio);
}

static void sdio_set_clock(struct SDIO_T *sdio, uint32_t clock)
{
    if (clock > sdio->cfg->f_max)
        clock = sdio->cfg->f_max;

    if (clock < sdio->cfg->f_min)
        clock = sdio->cfg->f_min;

    sdio->clock = clock;
    sdio_set_ios_clock(sdio);
}

static void sdio_set_host_bus_width(struct SDIO_T *sdio, enum HAL_SDIO_HOST_BUS_WIDTH width)
{
    sdio->bus_width = width;
    sdio_set_ios_width(sdio);
}

static struct SDIO_T *sdio_create(int id, const struct SDIO_CONFIG_T *cfg, void *priv)
{
    struct SDIO_T *sdio;

    /* quick validation */
    if (cfg == NULL || cfg->ops == NULL || cfg->ops->send_cmd == NULL ||
        cfg->f_min == 0 || cfg->f_max == 0 || cfg->b_max == 0)
        return NULL;

    sdio = &sdio_devices[id];
    if (sdio == NULL)
        return NULL;

    sdio->cfg = cfg;
    sdio->priv = priv;

    return sdio;
}

/******************************************************************************/
/***********************sdio standard related functions************************/
/******************************************************************************/
enum HAL_SDIO_ERR hal_sdio_host_send_cmd(enum HAL_SDIO_HOST_ID_T id, uint8_t cmdindex, uint32_t cmdarg, uint8_t rsp_type, uint32_t *rsp)
{
    int err;
    struct SDIO_CMD_T cmd;

    memset(&cmd, 0, sizeof(struct SDIO_CMD_T));
    cmd.cmdidx = cmdindex;
    cmd.cmdarg = cmdarg;
    cmd.resp_type = rsp_type;

    err = sdio_send_cmd(sdio_host[id].sdio, &cmd, NULL);
    if (err) {
        return (enum HAL_SDIO_ERR)err;
    }
    if (rsp) {
        *rsp = cmd.response[0];
    }

    return HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_io_rw_direct(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_DEVICE_RW read0_write1,
                                        enum HAL_SDIO_FUNC_NUM_T func_num, uint32_t addr, uint8_t data, uint8_t *rsp)
{
    struct SDIO_CMD_T cmd;
    int err = HAL_SDIO_ERR_NONE;

    memset(&cmd, 0, sizeof(struct SDIO_CMD_T));
    cmd.cmdidx  = SDIO_CMD_IO_RW_DIRECT;
    cmd.cmdarg  = read0_write1 ? 0x80000000 : 0x00000000;   //R/W flag
    cmd.cmdarg |= ((uint32_t)(func_num & 0x7)) << 28;       //funcion number
    cmd.cmdarg |= (read0_write1 && (uint32_t)rsp) ? 0x08000000 : 0x00000000; //RAW flag
    cmd.cmdarg |= addr << 9;    //register address
    cmd.cmdarg |= data;         //write data
    cmd.resp_type = SDIO_RSP_R5;

    sdio_ip_func_int_mask(id, 0);
    err = sdio_send_cmd(sdio_host[id].sdio, &cmd, NULL);
    if (err) {
        HAL_SDIO_HOST_ERROR(0, "%s, err:%d", __func__, err);
        goto err_cmd52;
    }
    if (rsp) {
        *rsp = cmd.response[0];
    }

err_cmd52:
    sdio_ip_func_int_mask(id, 1);

    return (enum HAL_SDIO_ERR)err;
}

enum HAL_SDIO_ERR hal_sdio_io_rw_extended(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_DEVICE_RW read0_write1,
        enum HAL_SDIO_FUNC_NUM_T func_num, uint32_t addr, uint8_t *rsp, uint8_t incr_addr, uint8_t *data_buff, uint32_t data_size)
{
    struct SDIO_CMD_T cmd;
    struct SDIO_DATA_T data;
    int err = HAL_SDIO_ERR_NONE;
    uint16_t func_cur_blk_size;
    uint16_t blk_count;

    if (phost_sdio_core[id].func[func_num]) {
        func_cur_blk_size = phost_sdio_core[id].func[func_num]->cur_blk_size;
        if (!func_cur_blk_size) {
            return HAL_SDIO_BLOCK_SIZE_ZERO;
        }
    } else {
        return HAL_SDIO_INVALID_FUNCTION_NUM;
    }

    HAL_SDIO_HOST_ASSERT(data_size, "%s:%d,data size cannot be 0", __func__, __LINE__);
    if (sdio_host[id].dma_en) {
        HAL_SDIO_HOST_ASSERT((data_size / SDIO_DMA_LINK_SIZE) < (SDIO_DMA_DESC_CNT - 1), "%s:%d,SDIO_DMA_DESC_CNT is too small", __func__, __LINE__);
    }

    //Automatically select byte mode or block mode
    if (data_size < func_cur_blk_size) {
        blk_count = 0;
        if (data_size % 4) {
            HAL_SDIO_HOST_ASSERT(0, "%s, %d is not an integer multiple of 4", __func__, data_size);
        }
    } else {
        if (data_size % func_cur_blk_size) {
            blk_count = data_size / func_cur_blk_size + 1;  //block count
        } else {
            blk_count = data_size / func_cur_blk_size;      //block count
        }

        if (blk_count > 511) {
            HAL_SDIO_HOST_ERROR(0, "blk_count %d is over 511", blk_count);
            return HAL_SDIO_INVALID_PARAMETER;
        }
    }

    memset(&cmd, 0, sizeof(struct SDIO_CMD_T));
    cmd.cmdidx  = SDIO_CMD_IO_RW_EXTENDED;
    cmd.cmdarg  = read0_write1 ? 0x80000000 : 0x00000000;   //R/W flag
    cmd.cmdarg |= ((uint32_t)(func_num & 0x7)) << 28;       //funcion number
    cmd.cmdarg |= incr_addr ? (1 << 26) : 0;                //op code
    cmd.cmdarg |= addr << 9;                                //register address
    if (blk_count) {
        //block mode
        cmd.cmdarg |= (1 << 27) | blk_count;
    } else {
        //bytes mode
        cmd.cmdarg |= (data_size == 512) ? 0 : data_size;
    }

    cmd.resp_type = SDIO_RSP_R5;

    memset(&data, 0, sizeof(struct SDIO_DATA_T));
    if (read0_write1) {
        data.src = (const char *)data_buff;
    } else {
        data.dest = (char *)data_buff;
    }

    data.blocks = blk_count ? blk_count : 1;
    data.blocksize = blk_count ? func_cur_blk_size : data_size;
    data.flags = read0_write1 ? SDIO_DATA_WRITE : SDIO_DATA_READ;

    HAL_SDIO_HOST_TRACE(0, "   ");
    HAL_SDIO_HOST_TRACE(1, "cmd53,arg = 0x%08x", cmd.cmdarg);
    HAL_SDIO_HOST_TRACE(1, "cmd53,data.blocks=%d", data.blocks);
    HAL_SDIO_HOST_TRACE(1, "cmd53,data.blocksize=%d", data.blocksize);
    HAL_SDIO_HOST_TRACE(1, "cmd53,data.flags=%d", data.flags);
    HAL_SDIO_HOST_TRACE(0, "   ");

    sdio_ip_func_int_mask(id, 0);
    err = sdio_send_cmd(sdio_host[id].sdio, &cmd, &data);
    if (err) {
        HAL_SDIO_HOST_ERROR(0, "%s, err:%d", __func__, err);
        goto err_cmd53;
    }
    if (rsp) {
        *rsp = cmd.response[0];
    }

err_cmd53:
    sdio_ip_func_int_mask(id, 1);

    return (enum HAL_SDIO_ERR)err;
}

enum HAL_SDIO_ERR hal_sdio_get_cccr_version(enum HAL_SDIO_HOST_ID_T id, uint8_t *cccr_version)
{
    uint8_t version = 0;

    if (!cccr_version) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_SDIO_VERSION, 0, &version)) {
        return HAL_SDIO_GET_VERSION_FAIL;
    }

    *cccr_version = version & 0xf;

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_sdio_version(enum HAL_SDIO_HOST_ID_T id, uint8_t *sdio_version)
{
    uint8_t version = 0;

    if (!sdio_version) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_SDIO_VERSION, 0, &version)) {
        return HAL_SDIO_GET_VERSION_FAIL;
    }

    *sdio_version = (version >> 4) & 0xf;

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_set_device_bus_width(enum HAL_SDIO_HOST_ID_T id, uint8_t bus_width)
{
    uint8_t width = 0;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_CONTROL, 0, &width)) {
        width &= ~0x3;
        width |= bus_width;
    } else {
        return HAL_SDIO_SET_BUS_WIDTH_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_CONTROL, width, NULL)) {
        return HAL_SDIO_SET_BUS_WIDTH_FAIL;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_bus_width(enum HAL_SDIO_HOST_ID_T id, uint8_t *bus_width)
{
    uint8_t width = 0;

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_CONTROL, 0, &width)) {
        return HAL_SDIO_GET_BUS_WIDTH_FAIL;
    }

    if (bus_width) {
        *bus_width = width & 0x3;
    } else {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_cis_ptr(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num, uint32_t *ptr_addr)
{
    uint8_t data = 0;
    uint8_t index;
    uint32_t temp = 0;

    if (func_num >= HAL_SDIO_FUNC_NUM || (!ptr_addr)) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    /* common cis pointer:0x09,0x0A,0x0B */
    for (index = 0; index < 3; index++) {
        hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_CIS_PTR + index, 0, &data);

        temp |= data << (index * 8);
    }

    *ptr_addr = temp;

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_cis_parse(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num, uint32_t cis_ptr)
{
    uint8_t data[255];
    uint8_t index = 0;
    uint8_t len = 0;
    uint8_t tpl_code = CISTPL_NULL;
    uint8_t tpl_link = 0;
    uint32_t temp_cis_ptr = cis_ptr;
    memset(data, 0, sizeof(data));
    while (1) {
        hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, temp_cis_ptr++, 0, &tpl_code);
        HAL_SDIO_HOST_TRACE(2, "CIS[0x%x] tpl_code=0x%X", temp_cis_ptr - 1, (uint32_t)tpl_code);
        if ((tpl_code == CISTPL_NULL) || (tpl_code == CISTPL_END)) {
            break;
        }

        hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, temp_cis_ptr++, 0, &tpl_link);
        HAL_SDIO_HOST_TRACE(2, "CIS[0x%x] tpl_link=%d", temp_cis_ptr - 1, tpl_link);

        for (index = 0; index < tpl_link; index++) {
            hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, temp_cis_ptr + index, 0, &data[index]);
            HAL_SDIO_HOST_TRACE(2, "tpl data[%d]=0x%x", index, data[index]);
        }
        switch (tpl_code) {
            case CISTPL_VERS_1:
                HAL_SDIO_HOST_TRACE(0, "Product Information:");
                for (index = 2; data[index] != 0xff; index += len + 1) {
                    len = strlen((char *)data + index);
                    if (len)
                        HAL_SDIO_HOST_TRACE(1, " %s", data + index);
                }
                break;
            case CISTPL_MANFID:
                // 16.6 CISTPL_MANFID: Manufacturer Identification String Tuple
                HAL_SDIO_HOST_TRACE(1, "Manufacturer Code: 0x%04x", *(uint16_t *)data);
                HAL_SDIO_HOST_TRACE(1, "Manufacturer Information: 0x%04x", *(uint16_t *)(data + 2));
                phost_sdio_core[id].manf_code = *(uint16_t *)data;
                phost_sdio_core[id].manf_info = *(uint16_t *)(data + 2);
                break;
            case CISTPL_FUNCID:
                // 16.7.1 CISTPL_FUNCID: Function Identification Tuple
                HAL_SDIO_HOST_TRACE(1, "Card Function Code: 0x%02X", data[0]);
                HAL_SDIO_HOST_TRACE(1, "System Initialization Bit Mask: 0x%02X", data[1]);
                break;
            case CISTPL_FUNCE:
                // 16.7.2 CISTPL_FUNCE: Function Extension Tuple
                if (data[0] == 0) {
                    // 16.7.3 CISTPL_FUNCE Tuple for Function 0 (Extended Data 00h)
                    HAL_SDIO_HOST_TRACE(1, "Maximum Block Size case1: func: %d,size %d", func_num, *(uint16_t *)(data + 1));
                    HAL_SDIO_HOST_TRACE(1, "Maximum Transfer Rate Code: 0x%02X", data[3]);
                    if (phost_sdio_core[id].func[func_num]) {
                        phost_sdio_core[id].func[func_num]->max_blk_size = *(uint16_t *)(data + 1);
                    }
                } else if (data[0] == 1) {
                    // 16.7.4 CISTPL_FUNCE Tuple for Function 1-7 (Extended Data 01h)
                    HAL_SDIO_HOST_TRACE(2, "Maximum Block Size case2 func: %d,size %d", func_num, *(uint16_t *)(data + 0x0c));
                    if (phost_sdio_core[id].func[func_num]) {
                        phost_sdio_core[id].func[func_num]->max_blk_size = *(uint16_t *)(data + 0x0c);
                    }
                } else {
                    HAL_SDIO_HOST_TRACE(2, "TPLFE_PS func: %d,size %d", func_num, *(uint16_t *)(data + 0x02));
                }
                break;
            default:
                HAL_SDIO_HOST_TRACE(3, "[CIS Tuple 0x%02X] addr=0x%08x size=%d", tpl_code, temp_cis_ptr - 2, tpl_link);
                break;
        }

        if (tpl_link == CISTPL_END)
            break;
        else
            temp_cis_ptr += tpl_link;
    }

    return HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_enable_func(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t enable = 0;
    uint8_t ready = 0;

    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, 0, &enable)) {
        enable |= (1 << func_num);
    } else {
        return HAL_SDIO_ENABLE_FUNCTION_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, enable, NULL)) {
        return HAL_SDIO_ENABLE_FUNCTION_FAIL;
    }

    while (!(ready & (1 << func_num))) {
        hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_READY, 0, &ready);
    }

    if (phost_sdio_core[id].func[func_num]) {
        phost_sdio_core[id].func[func_num]->func_status = FUNC_ENABLE;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_disable_func(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t enable = 0;

    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, 0, &enable)) {
        enable &= ~(1 << func_num);
    } else {
        return HAL_SDIO_DISABLE_FUNCTION_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, enable, NULL)) {
        return HAL_SDIO_DISABLE_FUNCTION_FAIL;
    }

    if (phost_sdio_core[id].func[func_num]) {
        phost_sdio_core[id].func[func_num]->func_status = FUNC_DISABLE;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_enable_master_int(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t enable = 0;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable)) {
        enable |= 0x1;
    } else {
        return HAL_SDIO_ENABLE_MASTER_INT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL)) {
        return HAL_SDIO_ENABLE_MASTER_INT_FAIL;
    }

    phost_sdio_core[id].sdio_int_ctrl = FUNC_INT_ENABLE;

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_disable_master_int(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t enable = 0;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable)) {
        enable &= ~0x1;
    } else {
        return HAL_SDIO_DISABLE_MASTER_INT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL)) {
        return HAL_SDIO_DISABLE_MASTER_INT_FAIL;
    }

    phost_sdio_core[id].sdio_int_ctrl = FUNC_INT_DISABLE;

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_enable_func_int(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t enable = 0;

    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable)) {
        enable |= (1 << func_num);
    } else {
        return HAL_SDIO_ENABLE_FUNCTION_INT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL)) {
        return HAL_SDIO_ENABLE_FUNCTION_INT_FAIL;
    }

    if (phost_sdio_core[id].func[func_num]) {
        phost_sdio_core[id].func[func_num]->func_int_status = FUNC_INT_ENABLE;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_disable_func_int(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t enable = 0;

    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable)) {
        enable &= ~(1 << func_num);
    } else {
        return HAL_SDIO_DISABLE_FUNCTION_INT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL)) {
        return HAL_SDIO_DISABLE_FUNCTION_INT_FAIL;
    }

    if (phost_sdio_core[id].func[func_num]) {
        phost_sdio_core[id].func[func_num]->func_int_status = FUNC_INT_DISABLE;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_set_block_size(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num, uint16_t block_size)
{
    /* set block size */
    hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_BLK_SIZE, block_size & 0xff, NULL);
    hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_BLK_SIZE + 1, (block_size >> 8) & 0xff, NULL);

    if (phost_sdio_core[id].func[func_num]) {
        phost_sdio_core[id].func[func_num]->cur_blk_size = block_size;
    } else {
        return HAL_SDIO_INVALID_FUNCTION_NUM;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_block_size(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num, uint16_t *block_size)
{
    if (!block_size) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (phost_sdio_core[id].func[func_num]) {
        *block_size = phost_sdio_core[id].func[func_num]->cur_blk_size;
    } else {
        return HAL_SDIO_INVALID_FUNCTION_NUM;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_int_pending(enum HAL_SDIO_HOST_ID_T id, uint8_t *int_pending)
{
    if (!int_pending) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_PENDING, 0, int_pending)) {
        return HAL_SDIO_GET_INT_PENDING_FAIL;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_set_func_abort(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t abort = 0;

    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ABORT, 0, &abort)) {
        abort |= func_num;
    } else {
        return HAL_SDIO_SET_ABORT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ABORT, abort, NULL)) {
        return HAL_SDIO_SET_ABORT_FAIL;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_enable_asyn_int(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t int_ext = 0;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_EXTERN, 0, &int_ext)) {
        int_ext |= 0x2;//set EAI bit
        HAL_SDIO_HOST_TRACE(1, "%s:%d,CCCR->int ext(asyn int en)=0x%X", __func__, __LINE__, int_ext);
    } else {
        return HAL_SDIO_ENABLE_INT_EXT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_EXTERN, int_ext, NULL)) {
        return HAL_SDIO_ENABLE_INT_EXT_FAIL;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_disable_asyn_int(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t int_ext = 0;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_EXTERN, 0, &int_ext)) {
        HAL_SDIO_HOST_TRACE(1, "CCCR->int ext=0x%X", int_ext);
        int_ext &= 0xFD;//clear EAI bit
    } else {
        return HAL_SDIO_DISABLE_INT_EXT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_EXTERN, int_ext, NULL)) {
        return HAL_SDIO_DISABLE_INT_EXT_FAIL;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_set_bus_speed(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_BUS_SPEED_MODE bus_speed)
{
    uint8_t val = 0;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_SPEED_SEL, 0, &val)) {
        val &= 0xF1;
        val |= (uint8_t)(bus_speed << 1);
    } else {
        return HAL_SDIO_SET_BUS_SPEED_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_SPEED_SEL, val, NULL)) {
        return HAL_SDIO_SET_BUS_SPEED_FAIL;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_bus_speed(enum HAL_SDIO_HOST_ID_T id, uint8_t *bus_speed)
{
    uint8_t val = 0;

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_SPEED_SEL, 0, &val)) {
        return HAL_SDIO_GET_BUS_SPEED_FAIL;
    }

    if (bus_speed) {
        *bus_speed = (val & 0x0E) >> 1;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_capability(enum HAL_SDIO_HOST_ID_T id, uint8_t *cap)
{
    uint8_t val = 0;

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_CARD_CAP, 0, &val)) {
        return HAL_SDIO_GET_CAPABILITY_FAIL;
    }

    if (cap) {
        *cap = val;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_gen_int_to_device(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t flag = 0;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_INT_TO_DEVICE, 0, &flag)) {
        flag |= 0x1;//set bit0
    } else {
        return HAL_SDIO_ENABLE_FUNC1_INT_TO_DEVICE_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_1, SDIO_FUNC1_INT_TO_DEVICE, flag, NULL)) {
        return HAL_SDIO_ENABLE_FUNC1_INT_TO_DEVICE_FAIL;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_check_device_load_ready(enum HAL_SDIO_HOST_ID_T id, uint8_t *status)
{
    if (!status) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_LOAD_CFG, 0, status)) {
        return HAL_SDIO_GET_LOAD_CFG_FAIL;
    }

    *status = *status & 0x01;
    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_check_device_rx_success(enum HAL_SDIO_HOST_ID_T id, uint8_t *status)
{
    if (!status) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_LOAD_CFG, 0, status)) {
        return HAL_SDIO_GET_LOAD_CFG_FAIL;
    }

    *status = (*status & 0x02) >> 1;
    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_check_device_fw_status(enum HAL_SDIO_HOST_ID_T id, uint32_t *status)
{
    uint8_t val = 0;

    if (!status) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_LOAD_CFG + 1, 0, &val)) {
        return HAL_SDIO_GET_LOAD_CFG_FAIL;
    }
    *status = val;

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_LOAD_CFG + 2, 0, &val)) {
        return HAL_SDIO_GET_LOAD_CFG_FAIL;
    }
    *status |= (val << 8);

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_LOAD_CFG + 3, 0, &val)) {
        return HAL_SDIO_GET_LOAD_CFG_FAIL;
    }
    *status |= (val << 16);

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_get_func1_rx_buf_cfg(enum HAL_SDIO_HOST_ID_T id, uint8_t *buf_cnt, uint16_t *buf_len)
{
    if ((!buf_cnt) && (!buf_len)) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    uint8_t u8temp = 0;
    uint16_t u16temp;

    //read buf cnt
    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_RX_BUF_CFG, 0, &u8temp)) {
        return HAL_SDIO_GET_FUNC1_RX_BUF_CFG_FAIL;
    }

    if (buf_cnt) {
        *buf_cnt = u8temp;
    }

    //read buf len
    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_RX_BUF_CFG + 1, 0, &u8temp)) {
        return HAL_SDIO_GET_FUNC1_RX_BUF_CFG_FAIL;
    }

    u16temp = u8temp;
    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_RX_BUF_CFG + 2, 0, &u8temp)) {
        return HAL_SDIO_GET_FUNC1_RX_BUF_CFG_FAIL;
    }

    if (buf_len) {
        u16temp |= (uint16_t)(u8temp << 8);
        *buf_len = u16temp;
    }

    return  HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_reset(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t abort = 0;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ABORT, 0, &abort)) {
        abort |= 0x8;//RES bit
    } else {
        return HAL_SDIO_RESET_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ABORT, abort, NULL)) {
        return HAL_SDIO_RESET_FAIL;
    }

    return  HAL_SDIO_ERR_NONE;
}

/*
 * Mask off any voltages we don't support and select
 * the lowest voltage
 */
uint32_t sdio_select_voltage(struct SDIO_CONFIG_T *cfg, uint32_t ocr)
{
    uint32_t bit;

    /*
     * Sanity check the voltages that the card claims to
     * support.
     */
    if (ocr & 0x7F) {
        HAL_SDIO_HOST_TRACE(0, "***card claims to support voltages below defined range");
        ocr &= ~0x7F;
    }

    HAL_SDIO_HOST_TRACE(1, "---host default ocr:0x%X", cfg->voltages);
    ocr &= cfg->voltages;
    if (!ocr) {
        HAL_SDIO_HOST_ERROR(0, "***no support for card's volts");
        return HAL_SDIO_ERR_NONE;
    } else {
        HAL_SDIO_HOST_TRACE(1, "---new ocr:0x%X", ocr);
    }

    bit = get_lsb_pos(ocr) - 1;
    ocr &= 3 << bit;
    HAL_SDIO_HOST_TRACE(1, "---final ocr:0x%X,bit%d", ocr, bit + 1);

    return ocr;
}

static int32_t sdio_device_init(enum HAL_SDIO_HOST_ID_T id, struct SDIO_T *sdio, struct SDIO_CONFIG_T *cfg, bool blocking_en)
{
    uint32_t val;
    int32_t err = 0;
    uint32_t rsp = 0;
    uint32_t rocr = 0;
    uint8_t func_index;
    uint8_t retry_cnt = 0;
    struct SDIO_CARD_RSP_T sdio_card_rsp;
    struct SDIO_IP_HOST_T *host = &sdio_host[id];

    memset(host_sdio_func[id], 0, sizeof(struct SDIO_FUNC_T) * HAL_SDIO_FUNC_NUM);
    memset(&phost_sdio_core[id], 0, sizeof(struct SDIO_CORE_T));
    host_sdio_func[id][HAL_SDIO_FUNC_0].func_status = FUNC_INT_ENABLE;

    //get device ocr:send cmd5
    do {
        err = hal_sdio_host_send_cmd(id, SDIO_CMD_IO_SEND_OP_COND, 0, SDIO_RSP_R4, &rsp);
        sdio_card_rsp.cmd5r4_io_ocr = rsp & 0x00FFFFFF;
        sdio_card_rsp.cmd5r4_s18a = (rsp >> 24) & 1;
        sdio_card_rsp.cmd5r4_mp = (rsp >> 27) & 1;
        sdio_card_rsp.cmd5r4_funcs = (rsp >> 28) & 7;
        sdio_card_rsp.cmd5r4_busy = (rsp >> 31) & 1;
        HAL_SDIO_HOST_TRACE(1, "R4->IO OCR=0x%X", (uint32_t)sdio_card_rsp.cmd5r4_io_ocr);
        HAL_SDIO_HOST_TRACE(1, "R4->S18A=%d", sdio_card_rsp.cmd5r4_s18a);
        HAL_SDIO_HOST_TRACE(1, "R4->Memory Present=%d", sdio_card_rsp.cmd5r4_mp);
        HAL_SDIO_HOST_TRACE(1, "R4->IO functions=%d", sdio_card_rsp.cmd5r4_funcs);
        HAL_SDIO_HOST_TRACE(1, "R4->sdio ready=%d", sdio_card_rsp.cmd5r4_busy);

        if (!sdio_card_rsp.cmd5r4_io_ocr) {
            HAL_SDIO_HOST_TRACE(0, "  ");
            HAL_SDIO_HOST_TRACE(0, "******wait for device init complete");
            if (!blocking_en) {
                retry_cnt++;
                HAL_SDIO_HOST_TRACE(0, "******wait for device init retry:%d", retry_cnt);
                if (retry_cnt >= CMD5_RETRY_CNT_MAX) {
                    HAL_SDIO_HOST_ERROR(0, "%s, wait device init error", __func__);
                    return HAL_SDIO_CMD5_FAIL;
                }
            }
            osDelay(1);
        }
    } while (!sdio_card_rsp.cmd5r4_io_ocr);

    rocr = sdio_select_voltage(cfg, sdio_card_rsp.cmd5r4_io_ocr);
    HAL_SDIO_HOST_ASSERT(rocr, "%s:%d,common ocr cannot be 0,error:%d", __func__, __LINE__, err);

    if (host->cfg.voltages & SDIO_VDD_165_195) {
        rocr |=  1 << 24;//UHS-I host
    }
    HAL_SDIO_HOST_TRACE(1, "---sdio common ocr:0x%X", rocr);

    //send io operate condition:send cmd5[S18R=1]
    retry_cnt = 0;
    do {
        err = hal_sdio_host_send_cmd(id, SDIO_CMD_IO_SEND_OP_COND, rocr, SDIO_RSP_R4, &rsp);
        sdio_card_rsp.cmd5r4_busy = (rsp >> 31) & 1;
        if (!sdio_card_rsp.cmd5r4_busy) {
            HAL_SDIO_HOST_TRACE(0, "  ");
            HAL_SDIO_HOST_TRACE(0, "******wait for device set ocr complete");
            if (!blocking_en) {
                retry_cnt++;
                HAL_SDIO_HOST_TRACE(0, "******wait for device set ocr retry:%d", retry_cnt);
                if (retry_cnt >= CMD5_RETRY_CNT_MAX) {
                    HAL_SDIO_HOST_ERROR(0, "%s, wait device set ocr error", __func__);
                    return HAL_SDIO_SET_OCR_FAIL;
                }
            }
            osDelay(1);
        }
    } while (!sdio_card_rsp.cmd5r4_busy); //wait for card init ready

    sdio_card_rsp.cmd5r4_io_ocr = rsp & 0x00FFFFFF;
    sdio_card_rsp.cmd5r4_s18a = (rsp >> 24) & 1;
    sdio_card_rsp.cmd5r4_mp = (rsp >> 27) & 1;
    sdio_card_rsp.cmd5r4_funcs = (rsp >> 28) & 7;
    sdio_card_rsp.cmd5r4_busy = (rsp >> 31) & 1;
    HAL_SDIO_HOST_TRACE(1, "R4->IO OCR=0x%X", (uint32_t)sdio_card_rsp.cmd5r4_io_ocr);
    HAL_SDIO_HOST_TRACE(1, "R4->S18A=%d", sdio_card_rsp.cmd5r4_s18a);
    HAL_SDIO_HOST_TRACE(1, "R4->Memory Present=%d", sdio_card_rsp.cmd5r4_mp);
    HAL_SDIO_HOST_TRACE(1, "R4->IO functions=%d", sdio_card_rsp.cmd5r4_funcs);
    HAL_SDIO_HOST_TRACE(1, "R4->sdio ready=%d", sdio_card_rsp.cmd5r4_busy);

    phost_sdio_core[id].func_total_num = sdio_card_rsp.cmd5r4_funcs;
    for (uint8_t index = 0; index <= phost_sdio_core[id].func_total_num; index++) {
        phost_sdio_core[id].func[index] = &host_sdio_func[id][index];
        host_sdio_func[id][index].func_num = index;
    }

    //check number of io functions and ocr
    if (sdio_card_rsp.cmd5r4_funcs && sdio_card_rsp.cmd5r4_io_ocr && (!sdio_card_rsp.cmd5r4_mp)) {
        if (sdio_card_rsp.cmd5r4_s18a) {
            if (host->volt_switch) {
                //stop low power clk function,2021-07-29
                HAL_SDIO_HOST_TRACE(0, "---clkena bit16 set to 0");
                val = sdio_ip_readl(&sdio_host[id], SDMMCIP_REG_CLKENA);
                val &= ~SDMMCIP_REG_CLKEN_LOW_PWR;
                sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_CLKENA, val);
                err = sdio_ip_wait_cmd_start(&sdio_host[id], 0);
                if (err) {
                    HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, err);
                    return err;
                }

                //send cmd11
                HAL_SDIO_HOST_TRACE(0, "---send cmd11");
                err = hal_sdio_host_send_cmd(id, SDIO_CMD_SWITCH_UHS18V, 0, SDIO_RSP_R1, &rsp);
                HAL_SDIO_HOST_TRACE(0, "---cmd11 rsp:0x%X,error:%d", rsp, err);

                int32_t timeout;
                timeout = 10;//ms, wait device driver cmd and data line to low, and host stop clk
                do {
                    if (!host->volt_switch_flag) {
                        osDelay(1);
                    }
                    if (timeout-- <= 0) {
                        HAL_SDIO_HOST_ERROR(2, "%s:%d, wait bus low timeout", __func__, __LINE__);
                        return HAL_SDIO_VOLT_SWITCH_FAIL;
                    }
                } while (!host->volt_switch_flag);

                //start a 5ms(min) timer
                HAL_SDIO_HOST_TRACE(0, "---delay 5ms");
                osDelay(5);

                //supply clk
                HAL_SDIO_HOST_TRACE(0, "---clkena bit0 set to 1,clk open");
                sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE);
                err = sdio_ip_wait_cmd_start(&sdio_host[id], 1);
                if (err) {
                    HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, err);
                    return err;
                }

                /* Wait for at least 1 ms according to spec */
                HAL_SDIO_HOST_TRACE(0, "---delay 1ms,last error:%d", err);
                osDelay(1);

                //wait device drive cmd and data line to high
                HAL_SDIO_HOST_TRACE(0, "---wait device drive cmd and data line to high");
                timeout = 30;//timeout 2ms
                do {
                    val = sdio_ip_readl(&sdio_host[id], SDMMCIP_REG_RINTSTS);
                    HAL_SDIO_HOST_TRACE(1, "---val:0x%X", val);
                    if ((val & (SDMMCIP_REG_RINTSTS_CDONE | SDMMCIP_REG_RINTSTS_HTO)) !=
                        (SDMMCIP_REG_RINTSTS_CDONE | SDMMCIP_REG_RINTSTS_HTO)) {
                        hal_sys_timer_delay_us(100);
                    }
                    if (timeout-- <= 0) {
                        HAL_SDIO_HOST_ERROR(0, "%s:%d, wait device done timeout", __func__, __LINE__);
                        return HAL_SDIO_VOLT_SWITCH_TIMEOUT;
                    }
                } while (!((val & SDMMCIP_REG_RINTSTS_CDONE) && (val & SDMMCIP_REG_RINTSTS_HTO)));
                sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_ALL); //clear interrupt status
                HAL_SDIO_HOST_TRACE(0, "---voltage switch success");

                //enable low power clk
                sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE |
                               SDMMCIP_REG_CLKEN_LOW_PWR);
                err = sdio_ip_wait_cmd_start(&sdio_host[id], 0);
                if (err) {
                    HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, err);
                    return err;
                }
            }
        } else {
            //This situation will be judged after cmd7
        }

        //Ask the card to publish a new relative address (RCA):cmd3
        err = hal_sdio_host_send_cmd(id, SDIO_CMD_SEND_RELATIVE_ADDR, 0, SDIO_RSP_R6, &rsp);

        //parse r6:get rca and status
        sdio_card_rsp.cmd3r6_rca = (rsp >> 16) & 0xFFFF;
        sdio_card_rsp.cmd3r6_com_crc_error = (rsp & 0x8000) >> 15;
        sdio_card_rsp.cmd3r6_illegal_command = (rsp & 0x4000) >> 14;
        sdio_card_rsp.cmd3r6_error = (rsp & 0x2000) >> 13;
        if ((sdio_card_rsp.cmd3r6_com_crc_error == 0) && (sdio_card_rsp.cmd3r6_illegal_command == 0) && (sdio_card_rsp.cmd3r6_error == 0)) {
            HAL_SDIO_HOST_TRACE(1, "R6->RCA=0x%X", (uint32_t)sdio_card_rsp.cmd3r6_rca);
            HAL_SDIO_HOST_TRACE(1, "R6->COM CRC ERROR=%d", sdio_card_rsp.cmd3r6_com_crc_error);
            HAL_SDIO_HOST_TRACE(1, "R6->ILLEGAL COMMAND=%d", sdio_card_rsp.cmd3r6_illegal_command);
            HAL_SDIO_HOST_TRACE(1, "R6->ERROR=%d", sdio_card_rsp.cmd3r6_error);
            HAL_SDIO_HOST_TRACE(0, "-------SDIO INIT SUCCESS------");

            //select card:cmd7
            err = hal_sdio_host_send_cmd(id, SDIO_CMD_SELECT_CARD, sdio_card_rsp.cmd3r6_rca << 16, SDIO_RSP_R1b, &rsp);
            sdio_card_rsp.cmd7r1b_card_status = rsp;
            HAL_SDIO_HOST_TRACE(1, "R1b->card status=0x%X", sdio_card_rsp.cmd7r1b_card_status);
            if ((sdio_card_rsp.cmd7r1b_card_status & 0x1E00) == 0x1E00) {
                HAL_SDIO_HOST_TRACE(0, "CURRENT_STATE is sdio mode,error:%d", err);
            }

            //The device signal voltage is already 1.8V or the card does not support UHS-I
            if (!sdio_card_rsp.cmd5r4_s18a) {
                HAL_SDIO_HOST_TRACE(0, "voltage of device is already 1.8V or the device does not support UHS-I");

                //Read cccr, check if the card supports UHS-I:cmd52
                err = hal_sdio_host_send_cmd(id, SDIO_CMD_IO_RW_DIRECT, SDIO_CCCR_UHSI_SUPPORT << 9, SDIO_RSP_R5, &rsp);
                if (rsp & 0x7) {    //check SDR50/SDR104/DDR50
                    HAL_SDIO_HOST_TRACE(0, "------device support UHS-I");

                    //The card supports UHS-I, and the host voltage is switched to 1.8v
                    if ((sdio_host_callback[id]->hal_sdio_host_signal_voltage_switch) \
                        && (host->cfg.voltages & SDIO_VDD_165_195)) {
                        sdio_host_callback[id]->hal_sdio_host_signal_voltage_switch();
                    } else {
                        HAL_SDIO_HOST_TRACE(0, "******warning: host does not support UHS-I");
                    }
                } else {
                    //The card does not support UHS-I, so the host does not do voltage switching
                    HAL_SDIO_HOST_TRACE(0, "device does not support UHS-I, so the host does not do voltage switching");
                }
            }

            hal_sdio_get_cccr_version(id, &phost_sdio_core[id].cccr_version);
            hal_sdio_get_sdio_version(id, &phost_sdio_core[id].sdio_version);

            HAL_SDIO_HOST_TRACE(2, "cccr_version:%d,%s", phost_sdio_core[id].cccr_version, (phost_sdio_core[id].cccr_version == 3) ? "3.00" : "*.00");
            HAL_SDIO_HOST_TRACE(2, "sdio_version:%d,%s", phost_sdio_core[id].sdio_version, (phost_sdio_core[id].sdio_version == 4) ? "3.00" : "*.00");

            //config device:UHS-I mode
            if (host->sdio->bus_width_user == HAL_SDIO_HOST_BUS_WIDTH_1) {
                hal_sdio_set_device_bus_width(id, SDIO_DEVICE_BUS_WIDTH_1);
            } else {
                hal_sdio_set_device_bus_width(id, SDIO_DEVICE_BUS_WIDTH_4);
            }
            HAL_SDIO_HOST_TRACE(0, "set sdio device bus width %d", host->sdio->bus_width_user);

            //config device:bus speed mode
            if (host->sdio->ddr_mode) {
                hal_sdio_set_bus_speed(id, HAL_SDIO_BUS_DDR50_MODE);
                HAL_SDIO_HOST_TRACE(0, "set sdio bus speed mode: DDR50");
            } else {
                hal_sdio_set_bus_speed(id, HAL_SDIO_BUS_SDR104_MODE);
                HAL_SDIO_HOST_TRACE(0, "set sdio bus speed mode: SDRxx");
            }

            //config host:UHS-I mode
            sdio_set_host_bus_width(sdio, host->sdio->bus_width_user);
            sdio_set_clock(sdio, sdio->cfg->f_max);
            if (sdio->cfg->f_max > SDIO_SPEED_SDR50) {
                //270 degrees phase shift: only valid when the ip frequency division is at least 2
                //sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_CLKSRC, SDMMCIP_REG_CLKSRC_CCLK_PRESAMPLE_SEL | SDMMCIP_REG_CLKSRC_SELECT_PRESAMPLE);
                //sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_CLKSRC, SDMMCIP_REG_CLKSRC_SELECT_PRESAMPLE | SDMMCIP_REG_CLKSRC_CCLK_DRV_SEL);

                //turn off oversampling
                sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_CLKSRC, SDMMCIP_REG_CLKSRC_CCLK_PRESAMPLE_SEL | SDMMCIP_REG_CLKSRC_CCLK_DRV_SEL);
                err = sdio_ip_wait_clk_update(&sdio_host[id]);
                if (err) {
                    HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, err);
                    return err;
                }
            }
#if 0
            for (func_index = 0; func_index <= phost_sdio_core[id].func_total_num; func_index++) {
#else
            for (func_index = 0; func_index < 1; func_index++) {
#endif
                uint32_t cis_ptr;

                hal_sdio_get_cis_ptr(id, (enum HAL_SDIO_FUNC_NUM_T)func_index, &cis_ptr);
                HAL_SDIO_HOST_TRACE(0, "------cis_ptr[%d]:0x%X", func_index, cis_ptr);
                hal_sdio_cis_parse(id, (enum HAL_SDIO_FUNC_NUM_T)func_index, cis_ptr);
            }

            enum HAL_SDIO_ERR sdio_err = HAL_SDIO_ERR_NONE;
            if (host->sdio->func_en) {
                for (func_index = HAL_SDIO_FUNC_1; func_index <= phost_sdio_core[id].func_total_num; func_index++) {
                    /* enable Func */
                    sdio_err = hal_sdio_enable_func(id, (enum HAL_SDIO_FUNC_NUM_T)func_index);
                    HAL_SDIO_HOST_TRACE(2, "sdio device func%d has been turned on,error:%d", func_index, sdio_err);

                    /* Enable interrupt of each FUNC */
                    sdio_err = hal_sdio_enable_func_int(id, (enum HAL_SDIO_FUNC_NUM_T)func_index);
                    HAL_SDIO_HOST_TRACE(2, "sdio device func%d int has been turned on,error:%d", func_index, sdio_err);
                }
                HAL_SDIO_HOST_TRACE(0, "sdio:enable function ok");
                HAL_SDIO_HOST_TRACE(1, "sdio:enable function int ok");
            }

            sdio_err = hal_sdio_enable_master_int(id);
            HAL_SDIO_HOST_TRACE(1, "sdio:enable master int ok,error:%d", sdio_err);
            if (host->sdio->async_int_en) {
                hal_sdio_enable_asyn_int(id);
            }

            val = sdio_ip_readl(&sdio_host[id], SDMMCIP_REG_INTMASK);
            val |= (SDMMCIP_REG_INTMSK_SDIO_FUNC1 | SDMMCIP_REG_INTMSK_SDIO_FUNC2
                    | SDMMCIP_REG_INTMSK_SDIO_FUNC3 | SDMMCIP_REG_INTMSK_SDIO_FUNC4
                    | SDMMCIP_REG_INTMSK_SDIO_FUNC5 | SDMMCIP_REG_INTMSK_SDIO_FUNC6
                    | SDMMCIP_REG_INTMSK_SDIO_FUNC7);
            sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_INTMASK, val);

            /* set block size */
            uint8_t func_cnt;
            if (host->sdio->set_func_blks) {
                func_cnt = phost_sdio_core[id].func_total_num + 1;
            } else {
                func_cnt = HAL_SDIO_FUNC_1;
            }

            for (func_index = HAL_SDIO_FUNC_0; func_index < func_cnt; func_index++) {
                hal_sdio_set_block_size(id, (enum HAL_SDIO_FUNC_NUM_T)func_index, host->sdio->block_size);
            }
            HAL_SDIO_HOST_TRACE(1, "sdio:enable function block size ok,size=%d", host->sdio->block_size);
        } else {
            HAL_SDIO_HOST_TRACE(0, "  ");
            HAL_SDIO_HOST_TRACE(0, "***assert***");
            HAL_SDIO_HOST_TRACE(1, "R6->RCA=0x%X", (uint32_t)sdio_card_rsp.cmd3r6_rca);
            HAL_SDIO_HOST_TRACE(1, "R6->COM CRC ERROR=%d", sdio_card_rsp.cmd3r6_com_crc_error);
            HAL_SDIO_HOST_TRACE(1, "R6->ILLEGAL COMMAND=%d", sdio_card_rsp.cmd3r6_illegal_command);
            HAL_SDIO_HOST_TRACE(1, "R6->ERROR=%d", sdio_card_rsp.cmd3r6_error);
            HAL_SDIO_HOST_ASSERT(0, "%s:%d CMD3 response error", __func__, __LINE__);
        }
    } else {
        HAL_SDIO_HOST_ASSERT(0, "%s:%d CMD5 TX error or response:R4 error or device is not an sdio device.", __func__, __LINE__);
    }

    return err;
}

/******************************************************************************/
/****************************General function**********************************/
/******************************************************************************/
SRAM_TEXT_LOC static void sdio_base_irq_handler(enum HAL_SDIO_HOST_ID_T id)
{
    uint32_t i;
    uint32_t branch;
    uint32_t raw_int_status;
    struct SDIO_IP_HOST_T *host = &sdio_host[HAL_SDIO_HOST_ID_0];
    uint32_t time_start = 0, time_ns;

    raw_int_status = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);//read raw interrupt status
    if (raw_int_status & SDMMCIP_REG_RINTSTS_HTO) {
        if (!host->volt_switch_flag) {
            time_start = hal_fast_sys_timer_get();
        }
    }
#ifdef SDIO_RUN_RECORD
    sdio_status_ori[id][sdio_status_ori_index[id]++] = raw_int_status;
    if (sdio_status_ori_index[id] >= SDIO_RUN_RECORD_CNT) {
        sdio_status_ori_index[id] = 0;
    }
#endif

    raw_int_status &= ~(SDMMCIP_REG_RINTSTS_CDONE | SDMMCIP_REG_RINTSTS_DTO);
    sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, raw_int_status); //clear interrupt status
    __DSB();
    HAL_SDIO_HOST_TRACE(3, "%s:%d,raw_int_status=0x%X", __func__, __LINE__, raw_int_status);

#if 0
    uint32_t int_status;
    int_status = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);
    HAL_SDIO_HOST_TRACE(3, "++++++int_status=0x%X", int_status);
#endif

    //Clear the redundant signs, because they are not used, the purpose is to reduce the number of subsequent cycles
    raw_int_status &= ~(SDMMCIP_REG_RINTSTS_TXDR | SDMMCIP_REG_RINTSTS_RXDR);

    while (raw_int_status) {
        i = get_lsb_pos(raw_int_status);
        branch = raw_int_status & (1 << i);
        raw_int_status &= ~(1 << i);
        switch (branch) {
            case SDMMCIP_REG_RINTSTS_EBE: {
                HAL_SDIO_HOST_ERROR(0, "bit 15:End-bit error (read)/write no CRC (EBE)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_READ_END_BIT_ERR_WRITE_NOCRC);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_SBE: {
                HAL_SDIO_HOST_ERROR(0, "bit 13:Start-bit error (SBE)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_START_BIT_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_HLE: {
                HAL_SDIO_HOST_ERROR(0, "bit 12:Hardware locked write error (HLE)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_HARDWARE_LOCKED_WRITE_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_FRUN: {
                sdio_ip_reset_fifo(host);
                HAL_SDIO_HOST_ERROR(0, "bit 11:FIFO underrun/overrun error (FRUN)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_FIFO_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_HTO: {
                if (host->volt_switch) {
                    uint32_t val;
                    uint32_t lock;
                    int32_t err = 0;

                    lock = int_lock();
                    //clear useless signs
                    raw_int_status &= ~(SDMMCIP_REG_RINTSTS_SDIO_FUNC1 | SDMMCIP_REG_RINTSTS_SDIO_FUNC2 |
                                        SDMMCIP_REG_RINTSTS_SDIO_FUNC3 | SDMMCIP_REG_RINTSTS_SDIO_FUNC4 |
                                        SDMMCIP_REG_RINTSTS_SDIO_FUNC5 | SDMMCIP_REG_RINTSTS_SDIO_FUNC6 |
                                        SDMMCIP_REG_RINTSTS_SDIO_FUNC7);

                    //turn off Volt switch interrupts
                    val = sdio_ip_readl(host, SDMMCIP_REG_INTMASK);
                    val &= ~SDMMCIP_REG_RINTSTS_HTO;
                    sdio_ip_writel(host, SDMMCIP_REG_INTMASK, val);
                    time_ns = FAST_TICKS_TO_NS(hal_fast_sys_timer_get() - time_start);

                    /*
                        clk=50k, time=20us
                        clk=100k,time=10us
                        clk=350k,time=2.9us
                        clk=400k,time=2.5us
                        3 <= clk cnt <=80 //the digital design is 90, and the software controls it at 80 here
                        time(min) = 10us * 6clk = 60us, time/2.9us=20clk,time/2.5us=24clk
                        time(max) = 10us * 80clk = 800us
                    */
                    time_start = 8 * host->period_st_ns;
                    if (!time_start) {
                        HAL_SDIO_HOST_ERROR(0, "%s:%d, period_st_ns is 0", __func__, __LINE__);
                    }

                    //The number of outputs before clk stops should be enough
                    if (time_start > time_ns) {
                        time_ns = (time_start - time_ns) / 1000;
                        hal_sys_timer_delay_us(time_ns);
                    }

                    //stop clk supply,2021-07-29
                    sdio_ip_writel(host, SDMMCIP_REG_CLKENA, 0);
                    err = sdio_ip_wait_cmd_start(host, 1);
                    if (err) {
                        HAL_SDIO_HOST_ERROR(0, "%s:%d, error:%d", __func__, __LINE__, err);
                    }
                    int_unlock(lock);
                    HAL_SDIO_HOST_TRACE(0, "---clkena bit0 set to 0,clk closed");

                    val = sdio_ip_readl(host, SDMMCIP_REG_UHS_REG);
                    HAL_SDIO_HOST_TRACE(0, "---UHS_REG set to 1.8V");
                    val |= SDMMCIP_REG_1V8_MODE;
                    sdio_ip_writel(host, SDMMCIP_REG_UHS_REG, val);

                    HAL_SDIO_HOST_TRACE(0, "bit 10:Volt_switch_int");
                    if (sdio_host_callback[id]->hal_sdio_host_signal_voltage_switch) {
                        sdio_host_callback[id]->hal_sdio_host_signal_voltage_switch();
                    }

                    host->volt_switch_flag = 1;
                } else {
                    HAL_SDIO_HOST_ERROR(0, "bit 10:Data starvation-by-host timeout (HTO)");
                    if (sdio_host_callback[id]->hal_sdio_host_error) {
                        sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_STARVATION_TIMEOUT);
                    }
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_DRTO: {
                HAL_SDIO_HOST_ERROR(0, "bit 9:Data read timeout (DRTO)/Boot Data Start (BDS)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_DATA_READ_TIMEOUT_BDS);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_RTO: {
                HAL_SDIO_HOST_ERROR(0, "bit 8:Response timeout (RTO)/Boot Ack Received (BAR)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_RESPONSE_TIMEOUT_BAR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_DCRC: {
                HAL_SDIO_HOST_ERROR(0, "bit 7:Data CRC error (DCRC)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_DATA_CRC_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_RCRC: {
                HAL_SDIO_HOST_ERROR(0, "bit 6:Response CRC error (RCRC)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_RESPONSE_CRC_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_RE: {
                HAL_SDIO_HOST_ERROR(0, "bit 1:Response error (RE)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_RESPONSE_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_CD: {
                HAL_SDIO_HOST_TRACE(0, "bit 0:Card detect (RE)");
                if (sdio_host_callback[id]->hal_sdio_host_card_detect) {
                    sdio_host_callback[id]->hal_sdio_host_card_detect();
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC1:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC2:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC3:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC4:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC5:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC6:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC7: {
                uint8_t int_pending = 0;

                raw_int_status &= ~(SDMMCIP_REG_RINTSTS_SDIO_FUNC1 | SDMMCIP_REG_RINTSTS_SDIO_FUNC2 |
                                    SDMMCIP_REG_RINTSTS_SDIO_FUNC3 | SDMMCIP_REG_RINTSTS_SDIO_FUNC4 |
                                    SDMMCIP_REG_RINTSTS_SDIO_FUNC5 | SDMMCIP_REG_RINTSTS_SDIO_FUNC6 |
                                    SDMMCIP_REG_RINTSTS_SDIO_FUNC7);
                if (host->sdio->func1_only) {
                    int_pending = SDIO_CCCR5_INT1;
                } else {
                    hal_sdio_get_int_pending(HAL_SDIO_HOST_ID_0, &int_pending);
                    HAL_SDIO_HOST_TRACE(0, "int pending:0x%X", int_pending);
                }

                if (int_pending & SDIO_CCCR5_INT1) {
                    HAL_SDIO_HOST_TRACE(0, "bit 16:func1 int");
                    if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                        sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_1);
                    }
                }

                if (!host->sdio->func1_only) {
                    if (int_pending & SDIO_CCCR5_INT2) {
                        HAL_SDIO_HOST_TRACE(0, "bit 17:func2 int");
                        if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                            sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_2);
                        }
                    }
                    if (int_pending & SDIO_CCCR5_INT3) {
                        HAL_SDIO_HOST_TRACE(0, "bit 18:func3 int");
                        if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                            sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_3);
                        }
                    }
                    if (int_pending & SDIO_CCCR5_INT4) {
                        HAL_SDIO_HOST_TRACE(0, "bit 19:func4 int");
                        if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                            sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_4);
                        }
                    }
                    if (int_pending & SDIO_CCCR5_INT5) {
                        HAL_SDIO_HOST_TRACE(0, "bit 20:func5 int");
                        if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                            sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_5);
                        }
                    }
                    if (int_pending & SDIO_CCCR5_INT6) {
                        HAL_SDIO_HOST_TRACE(0, "bit 21:func6 int");
                        if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                            sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_6);
                        }
                    }
                    if (int_pending & SDIO_CCCR5_INT7) {
                        HAL_SDIO_HOST_TRACE(0, "bit 22:func7 int");
                        if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                            sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_7);
                        }
                    }
                }
                break;
            }
            default : {
                //Don't care about other states
                break;
            }
        }
    }
}

static void hal_sdio0_irq_handler(void)
{
    sdio_base_irq_handler(HAL_SDIO_HOST_ID_0);
}

#ifdef SDMMC1_BASE
static void hal_sdio1_irq_handler(void)
{
    sdio_base_irq_handler(HAL_SDIO_HOST_ID_1);
}
#endif

enum HAL_SDIO_ERR hal_sdio_host_int_enable(enum HAL_SDIO_HOST_ID_T id)
{
    struct SDIO_IP_HOST_T *host = NULL;
    uint32_t irq_num = SDMMC0_IRQn;
    uint32_t val;

    HAL_SDIO_HOST_TRACE(2, "%s:%d", __func__, __LINE__);

    host = &sdio_host[id];
    sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_ALL); //clear interrupt status
    sdio_ip_writel(host, SDMMCIP_REG_INTMASK, SDMMCIP_REG_INTMSK_ALL & (~(SDMMCIP_REG_INTMSK_CDONE | \
                   SDMMCIP_REG_INTMSK_DTO | SDMMCIP_REG_INTMSK_TXDR | SDMMCIP_REG_INTMSK_RXDR |      \
                   SDMMCIP_REG_INTMSK_ACD | SDMMCIP_REG_INTMSK_SDIO_FUNC1 |                          \
                   SDMMCIP_REG_INTMSK_SDIO_FUNC2 | SDMMCIP_REG_INTMSK_SDIO_FUNC3 |                   \
                   SDMMCIP_REG_INTMSK_SDIO_FUNC4 | SDMMCIP_REG_INTMSK_SDIO_FUNC5 |                   \
                   SDMMCIP_REG_INTMSK_SDIO_FUNC6 | SDMMCIP_REG_INTMSK_SDIO_FUNC7)));    //open interrupt
    val = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
    val |= SDMMCIP_REG_INT_EN;
    sdio_ip_writel(host, SDMMCIP_REG_CTRL, val);                                        //enable interrupt

    if (id == HAL_SDIO_HOST_ID_0) {
        irq_num = SDMMC0_IRQn;
        NVIC_SetVector((IRQn_Type)irq_num, (uint32_t)hal_sdio0_irq_handler);
    }
#ifdef SDMMC1_BASE
    else if (id == HAL_SDIO_HOST_ID_1) {
        irq_num = SDMMC1_IRQn;
        NVIC_SetVector((IRQn_Type)irq_num, (uint32_t)hal_sdio1_irq_handler);
    }
#endif

    NVIC_SetPriority((IRQn_Type)irq_num, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ((IRQn_Type)irq_num);
    NVIC_EnableIRQ((IRQn_Type)irq_num);

    return HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_int_disable(enum HAL_SDIO_HOST_ID_T id)
{
    struct SDIO_IP_HOST_T *host = NULL;
    uint32_t irq_num = SDMMC0_IRQn;
    uint32_t val;

    HAL_SDIO_HOST_TRACE(2, "%s:%d", __func__, __LINE__);
    host = &sdio_host[id];
    val = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
    val &= ~SDMMCIP_REG_INT_EN;
    sdio_ip_writel(host, SDMMCIP_REG_INTMASK, ~SDMMCIP_REG_INTMSK_ALL);
    sdio_ip_writel(host, SDMMCIP_REG_CTRL, val);                        //disable interrupt
    sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_ALL); //clear interrupt status

    if (id == HAL_SDIO_HOST_ID_0) {
        irq_num = SDMMC0_IRQn;
    }
#ifdef SDMMC1_BASE
    else if (id == HAL_SDIO_HOST_ID_1) {
        irq_num = SDMMC1_IRQn;
    }
#endif

    NVIC_DisableIRQ((IRQn_Type)irq_num);
    NVIC_ClearPendingIRQ((IRQn_Type)irq_num);

    return HAL_SDIO_ERR_NONE;
}

#ifdef CORE_SLEEP_POWER_DOWN
struct SAVED_SDIO_REGS_T {
    uint32_t REG_00;
    uint32_t REG_04;
    uint32_t REG_08;
    uint32_t REG_0C;
    uint32_t REG_14;
    uint32_t REG_18;
    uint32_t REG_24;
    uint32_t REG_4C;
    uint32_t REG_74;
};
SRAM_BSS_LOC static struct SAVED_SDIO_REGS_T saved_sdio_regs[HAL_SDIO_HOST_ID_NUM];

static int sdio_base_pm_notif_handler(enum HAL_SDIO_HOST_ID_T id, enum HAL_PM_STATE_T state)
{
    struct SDIO_IP_HOST_T *host = &sdio_host[id];

    if (state == HAL_PM_STATE_POWER_DOWN_SLEEP) {
#ifdef SDIO_RUN_RECORD
        sdio_sleep_wakeup_record[id][sdio_sleep_wakeup_record_index[id]++] = 0xA0;
        if (sdio_sleep_wakeup_record_index[id] >= SDIO_RUN_RECORD_CNT) {
            sdio_sleep_wakeup_record_index[id] = 0;
        }
#endif
        //save digital side registers
        saved_sdio_regs[id].REG_00 = sdio_ip_readl(host, 0x00);
        saved_sdio_regs[id].REG_04 = sdio_ip_readl(host, 0x04);
        saved_sdio_regs[id].REG_08 = sdio_ip_readl(host, 0x08);
        saved_sdio_regs[id].REG_0C = sdio_ip_readl(host, 0x0C);
        saved_sdio_regs[id].REG_14 = sdio_ip_readl(host, 0x14);
        saved_sdio_regs[id].REG_18 = sdio_ip_readl(host, 0x18);
        saved_sdio_regs[id].REG_24 = sdio_ip_readl(host, 0x24);
        saved_sdio_regs[id].REG_4C = sdio_ip_readl(host, 0x4C);
        saved_sdio_regs[id].REG_74 = sdio_ip_readl(host, 0x74);
    } else if (state == HAL_PM_STATE_POWER_DOWN_WAKEUP) {
#ifdef SDIO_RUN_RECORD
        sdio_sleep_wakeup_record[id][sdio_sleep_wakeup_record_index[id]++] = 0xA1;
        if (sdio_sleep_wakeup_record_index[id] >= SDIO_RUN_RECORD_CNT) {
            sdio_sleep_wakeup_record_index[id] = 0;
        }
#endif
        //restore digital side registers
        sdio_ip_writel(host, 0x04, saved_sdio_regs[id].REG_04);//SDMMCIP_REG_PWREN
        if (sdio_ip_wait_reset(host, SDMMCIP_REG_RESET_ALL)) {
            if (sdio_ip_wait_reset(host, SDMMCIP_REG_RESET_ALL)) {
                HAL_SDIO_HOST_ERROR(0, "%s failed", __func__);
            }
        }
        sdio_ip_setup_div(host, saved_sdio_regs[id].REG_08);   //SDMMCIP_REG_CLKDIV + SDMMCIP_REG_CLKENA
        sdio_ip_writel(host, 0x0C, saved_sdio_regs[id].REG_0C);//SDMMCIP_REG_CLKSRC
        sdio_ip_writel(host, 0x14, saved_sdio_regs[id].REG_14);//SDMMCIP_REG_TMOUT
        sdio_ip_writel(host, 0x18, saved_sdio_regs[id].REG_18);//SDMMCIP_REG_CTYPE
        sdio_ip_writel(host, 0x24, saved_sdio_regs[id].REG_24);//SDMMCIP_REG_INTMASK
        sdio_ip_writel(host, 0x4C, saved_sdio_regs[id].REG_4C);//SDMMCIP_REG_FIFOTH
        sdio_ip_writel(host, 0x74, saved_sdio_regs[id].REG_74);//SDMMCIP_REG_UHS_REG

        sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_ALL); //clear interrupt status
        sdio_ip_writel(host, 0x00, saved_sdio_regs[id].REG_00);
    }

    return 0;
}

static int sdio0_pm_notif_handler(enum HAL_PM_STATE_T state)
{
    return sdio_base_pm_notif_handler(HAL_SDIO_HOST_ID_0, state);
}

#ifdef SDMMC1_BASE
static int sdio1_pm_notif_handler(enum HAL_PM_STATE_T state)
{
    return sdio_base_pm_notif_handler(HAL_SDIO_HOST_ID_1, state);
}
#endif

static HAL_PM_NOTIF_HANDLER hal_sdio_pm_notif_handler[HAL_SDIO_HOST_ID_NUM] = {
    sdio0_pm_notif_handler,
#ifdef SDMMC1_BASE
    sdio1_pm_notif_handler,
#endif
};
#endif

static int32_t hal_sdio_host_init_base(enum HAL_SDIO_HOST_ID_T id, const struct HAL_SDIO_HOST_CONFIG_T *cfg)
{
    int32_t ret = HAL_SDIO_ERR_NONE;
    uint32_t src_clk;
    uint32_t sdio_speed;
    struct SDIO_IP_HOST_T *host = NULL;
    HAL_SDIO_HOST_ASSERT(id < HAL_SDIO_HOST_ID_NUM, "Invalid sdio host id: %d", id);

    if (cfg && cfg->bus_speed) {
        sdio_speed = cfg->bus_speed;
    } else {
        sdio_speed = 12 * 1000 * 1000;
    }

#ifdef FPGA
    switch (id) {
        case HAL_SDIO_HOST_ID_0:
            hal_cmu_sdmmc0_set_freq(HAL_CMU_PERIPH_FREQ_26M);
            break;
#ifdef SDMMC1_BASE
        case HAL_SDIO_HOST_ID_1:
            hal_cmu_sdmmc1_set_freq(HAL_CMU_PERIPH_FREQ_26M);
            break;
#endif
        default:
            break;
    }
    src_clk = hal_cmu_get_crystal_freq();
    HAL_SDIO_HOST_ASSERT(src_clk >= sdio_speed, "%s:%d, sdio_speed %d > src clk %d", __func__, __LINE__, sdio_speed, src_clk);
#else
    if (0) {
#ifdef PERIPH_PLL_FREQ
    } else if (sdio_speed > 2 * hal_cmu_get_crystal_freq()) {
        uint32_t div;//division is one step in place, the sdmmc ip is no longer divided(Support even division)

        div = PERIPH_PLL_FREQ / sdio_speed;
        if (PERIPH_PLL_FREQ % sdio_speed) {
            div += 1;
        }

        src_clk = PERIPH_PLL_FREQ / div;
        switch (id) {
            case HAL_SDIO_HOST_ID_0:
                ret = hal_cmu_sdmmc0_set_div(div);
                break;
#ifdef SDMMC1_BASE
            case HAL_SDIO_HOST_ID_1:
                ret = hal_cmu_sdmmc1_set_div(div);
                break;
#endif
            default:
                break;
        }
        HAL_SDIO_HOST_ASSERT(!ret, "The sdio_speed value is invalid, causing the div to be out of range, ret %d, div is %d", ret, div);
        HAL_SDIO_HOST_ASSERT(src_clk <= sdio_speed, "%s:%d, div clk %d > sdio_speed %d", __func__, __LINE__, src_clk, sdio_speed);
        HAL_SDIO_HOST_TRACE(1, "PERIPH_PLL_FREQ is %d, the final PLL div is %d", PERIPH_PLL_FREQ, div);
#endif
    } else if (sdio_speed >= 3 * hal_cmu_get_crystal_freq() / 2) {  //sdmmc ip may also be divided(Only even division)
        switch (id) {
            case HAL_SDIO_HOST_ID_0:
                hal_cmu_sdmmc0_set_freq(HAL_CMU_PERIPH_FREQ_52M);
                break;
#ifdef SDMMC1_BASE
            case HAL_SDIO_HOST_ID_1:
                hal_cmu_sdmmc1_set_freq(HAL_CMU_PERIPH_FREQ_52M);
                break;
#endif
            default:
                break;
        }
        src_clk = 2 * hal_cmu_get_crystal_freq();
        HAL_SDIO_HOST_ASSERT(src_clk >= sdio_speed, "%s:%d, sdio_speed %d > src clk %d", __func__, __LINE__, sdio_speed, src_clk);
    } else {                                                        //sdmmc ip may also be divided(Only even division)
        switch (id) {
            case HAL_SDIO_HOST_ID_0:
                hal_cmu_sdmmc0_set_freq(HAL_CMU_PERIPH_FREQ_26M);
                break;
#ifdef SDMMC1_BASE
            case HAL_SDIO_HOST_ID_1:
                hal_cmu_sdmmc1_set_freq(HAL_CMU_PERIPH_FREQ_26M);
                break;
#endif
            default:
                break;
        }
        src_clk = hal_cmu_get_crystal_freq();
        HAL_SDIO_HOST_ASSERT(src_clk >= sdio_speed, "%s:%d, sdio_speed %d > src clk %d", __func__, __LINE__, sdio_speed, src_clk);
    }
#endif

    HAL_SDIO_HOST_TRACE(2, "sdio_speed %d, bus clk(Not necessarily the final speed) %d", sdio_speed, src_clk);

    /* sdio host iomux */
    switch (id) {
        case HAL_SDIO_HOST_ID_0:
            hal_iomux_set_sdmmc0();
            hal_cmu_sdmmc0_clock_enable();
            break;
#ifdef SDMMC1_BASE
        case HAL_SDIO_HOST_ID_1:
            hal_iomux_set_sdmmc1();
            hal_cmu_sdmmc1_clock_enable();
            break;
#endif
        default:
            break;
    }

#ifdef FPGA
#if defined(CHIP_BEST2002) || defined(CHIP_BEST2005)
    hal_sys_timer_delay_us(500);//palladium use:wait complete,2021-05-15
#endif
#endif

    host = &sdio_host[id];
    host->host_id    = id;
    host->src_clk_hz = src_clk;
    host->ioaddr     = (void *)sdio_ip_base[id];
    host->desired_bus_speed = 0;
    if (cfg) {
        host->yield = cfg->yield;
        host->dma_en = cfg->dma_en;
        host->volt_switch = cfg->volt_switch;
        host->blocking_init_en = cfg->blocking_init_en;
    } else {
        host->yield             = 0;
        host->dma_en            = 1;//default open dma
        host->volt_switch       = 1;
        host->blocking_init_en  = 1;
    }
    host->volt_switch_flag = 0;
    host->fifoth_val = MSIZE(2) | RX_WMARK(7) | TX_WMARK(8);

    host->cfg.ops = &sdio_ip_ops;
    host->cfg.voltages = SDIO_VDD_27_28 | SDIO_VDD_28_29 | SDIO_VDD_29_30 | SDIO_VDD_30_31 | SDIO_VDD_31_32 | SDIO_VDD_32_33 | SDIO_VDD_165_195;
    host->cfg.f_min = 400 * 1000;     //Don't modify
    host->cfg.f_max = sdio_speed;
    host->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
    host->sdio = sdio_create(host->host_id, &host->cfg, host);
    HAL_SDIO_HOST_TRACE(4, "%s:%d, sdio clk min %d, max %d", __func__, __LINE__, host->cfg.f_min, host->cfg.f_max);

    if (host->dma_en) {
        host->dma_ch = 0;
        host->dma_in_use = 0;
        host->dma_error = 0;
        host->sdio_dma_lock = 0;
        host->tx_dma_handler = sdio_ip_ext_dma_irq_handlers[id * 2];
        host->rx_dma_handler = sdio_ip_ext_dma_irq_handlers[id * 2 + 1];
    }

    if (cfg) {
        //The controller does not support this mode, 2022-06-11
        //HAL_SDIO_HOST_ASSERT(sdio_speed <= SDIO_SPEED_DDR50, "%s:%d, sdio ddr50 speed %d > max speed %d", __func__, __LINE__, sdio_speed, SDIO_SPEED_DDR50);
        host->sdio->ddr_mode = 0;//cfg->ddr_mode;

        host->sdio->func_en = cfg->func_en;
        host->sdio->send_abort = cfg->send_abort;
        host->sdio->func1_only = cfg->func1_only;
        host->sdio->async_int_en = cfg->async_int_en;
        host->sdio->set_func_blks = cfg->set_func_blks;
        host->sdio->block_size = cfg->block_size;
    } else {
        host->sdio->ddr_mode = 0;
        host->sdio->func_en = 1;
        host->sdio->send_abort = 1;
        host->sdio->func1_only = 1;
        host->sdio->async_int_en = 1;
        host->sdio->set_func_blks = 1;
        host->sdio->block_size = 256;
    }

    if (cfg && cfg->bus_width) {
        host->sdio->bus_width_user = cfg->bus_width;
    } else {
        host->sdio->bus_width_user = HAL_SDIO_HOST_BUS_WIDTH_4;
    }

    sdio_host_callback[id] = &sdio_host_callback_default[id];
    memset(sdio_host_callback[id], 0, sizeof(struct HAL_SDIO_HOST_CB_T));
    if (cfg && cfg->callback) {
        sdio_host_callback[id] = cfg->callback;
    }

    /* make sure it's not NULL earlier */
    ret = host->sdio->cfg->ops->init(host->sdio);
    if (ret) {
        HAL_SDIO_HOST_ERROR(3, "%s:%d, sdio ip init error,ret=%d", __func__, __LINE__, ret);
        return ret;
    } else {
        HAL_SDIO_HOST_TRACE(3, "%s:%d, sdio ip init ok,ret=%d", __func__, __LINE__, ret);
    }
    hal_sdio_host_int_enable(id);

#ifdef CORE_SLEEP_POWER_DOWN
    hal_pm_notif_register(HAL_PM_USER_HAL, hal_sdio_pm_notif_handler[id]);
#endif

    return ret;
}

static int32_t hal_sdio_device_init_base(enum HAL_SDIO_HOST_ID_T id, struct SDIO_IP_HOST_T *host, bool device_init)
{
    int32_t ret = HAL_SDIO_ERR_NONE;

    if (host->sdio->has_init) {
        HAL_SDIO_HOST_ERROR(2, "%s:%d,sdio device has been initialized", __func__, __LINE__);
        return HAL_SDIO_ERR_NONE;
    }
    if (!host->sdio->init_in_progress) {
        host->sdio->init_in_progress = 1;
        if (device_init) {
            sdio_set_host_bus_width(host->sdio, HAL_SDIO_HOST_BUS_WIDTH_1);
            sdio_set_clock(host->sdio, 0);//set 400KHz

            //Calculate period at start frequency
            uint32_t sdio_start_freq;
            sdio_start_freq = host->src_clk_hz / hal_cmu_get_crystal_freq();
            sdio_start_freq *= hal_cmu_get_crystal_freq();
            sdio_start_freq = sdio_start_freq / 1000 / 1000;
            host->period_st_ns = (1000 * host->div) / sdio_start_freq;
            HAL_SDIO_HOST_TRACE(1, "sdio period of start frequency: %dns", host->period_st_ns);

            ret = sdio_device_init(id, host->sdio, &host->cfg, host->blocking_init_en);
            if (ret) {
                HAL_SDIO_HOST_ERROR(0, "sdio_device_init error:%d", ret);
                return ret;
            }

            if (host->sdio->send_abort) {
                //Read the cis register, in order to get the interrupt of the device by the host,2021-06-25
                uint8_t cis_data[4];
                uint32_t cis_ptr;
                hal_sdio_get_cis_ptr(id, (enum HAL_SDIO_FUNC_NUM_T)0, &cis_ptr);
                hal_sdio_io_rw_extended(id, HAL_SDIO_DEVICE_READ, (enum HAL_SDIO_FUNC_NUM_T)0, cis_ptr, NULL, 1, cis_data, sizeof(cis_data));
                if (host->dma_en) {
                    while (host->dma_in_use) {
                        __asm("nop");
                        HAL_SDIO_HOST_TRACE(0, "wait dma idle");
                    }
                }
                hal_sdio_set_func_abort(id, (enum HAL_SDIO_FUNC_NUM_T)0);
            }
        } else {
            //config host:UHS-I mode
            sdio_set_host_bus_width(host->sdio, host->sdio->bus_width_user);
            sdio_set_clock(host->sdio, host->sdio->cfg->f_max);

            memset(&phost_sdio_core[id], 0, sizeof(struct SDIO_CORE_T));
            for (uint8_t index = 0; index < 8; index++) {
                phost_sdio_core[id].func[index] = &host_sdio_func[id][index];
                host_sdio_func[id][index].func_num = index;
                phost_sdio_core[id].func[index]->cur_blk_size = host->sdio->block_size;
            }

            //Disable the function of stopping output when clk is idle
            sdio_ip_clk_disable_low_power(host);
            ret = HAL_SDIO_ERR_NONE;
        }

        HAL_SDIO_HOST_TRACE(2, "%s:%d,sdio device init complete", __func__, __LINE__);
    } else {
        HAL_SDIO_HOST_TRACE(2, "%s:%d,sdio device is being initialized", __func__, __LINE__);
    }
    host->sdio->has_init = 1;
    host->sdio->init_in_progress = 0;

    return ret;
}

enum HAL_SDIO_ERR hal_sdio_host_init(enum HAL_SDIO_HOST_ID_T id, const struct HAL_SDIO_HOST_CONFIG_T *cfg)
{
    return hal_sdio_host_init_base(id, cfg);
}

enum HAL_SDIO_ERR hal_sdio_device_init(enum HAL_SDIO_HOST_ID_T id)
{
    struct SDIO_IP_HOST_T *host = NULL;
    host = &sdio_host[id];

    return hal_sdio_device_init_base(id, host, 1);
}

static int32_t hal_sdio_host_device_init(enum HAL_SDIO_HOST_ID_T id, const struct HAL_SDIO_HOST_CONFIG_T *cfg, bool device_init)
{
    struct SDIO_IP_HOST_T *host = NULL;
    host = &sdio_host[id];
    int32_t ret;

    ret = hal_sdio_host_init_base(id, cfg);
    if (ret) {
        return ret;
    }
    return hal_sdio_device_init_base(id, host, device_init);
}

enum HAL_SDIO_ERR hal_sdio_host_open(enum HAL_SDIO_HOST_ID_T id, const struct HAL_SDIO_HOST_CONFIG_T *cfg)
{
    return (enum HAL_SDIO_ERR)hal_sdio_host_device_init(id, cfg, 1);
}

enum HAL_SDIO_ERR hal_sdio_host_reopen(enum HAL_SDIO_HOST_ID_T id, struct HAL_SDIO_HOST_CB_T *callback)
{
    if (callback) {
        sdio_host_callback[id] = callback;
    }

    return HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_signal_out(enum HAL_SDIO_HOST_ID_T id, const struct HAL_SDIO_HOST_CONFIG_T *cfg)
{
    return (enum HAL_SDIO_ERR)hal_sdio_host_device_init(id, cfg, 0);
}

void hal_sdio_host_close(enum HAL_SDIO_HOST_ID_T id)
{
    struct SDIO_IP_HOST_T *host = NULL;
    HAL_SDIO_HOST_ASSERT(id < HAL_SDIO_HOST_ID_NUM, "Invalid sdio host id: %d", id);
    HAL_SDIO_HOST_TRACE(2, "%s:%d", __func__, __LINE__);

    host = &sdio_host[id];
    if (host->sdio == NULL) {
        return;
    }

    if (host->sdio->has_init == 0) {
        HAL_SDIO_HOST_TRACE(2, "%s:%d,sdio host has been closed", __func__, __LINE__);
        return;
    }

#ifdef CORE_SLEEP_POWER_DOWN
    hal_pm_notif_deregister(HAL_PM_USER_HAL, hal_sdio_pm_notif_handler[id]);
#endif

    while (host->sdio->init_in_progress)
        hal_sys_timer_delay_us(500);

    host->sdio->has_init = 0;

    if (host->dma_en) {
        if (host->dma_in_use) {
            sdio_ip_reset_dma(host);
        }
    }

    hal_sdio_host_int_disable(id);
    switch (id) {
        case HAL_SDIO_HOST_ID_0:
            hal_cmu_sdmmc0_clock_disable();
            hal_iomux_clear_sdmmc0();
            break;
#ifdef SDMMC1_BASE
        case HAL_SDIO_HOST_ID_1:
            hal_cmu_sdmmc1_clock_disable();
            hal_iomux_clear_sdmmc1();
            break;
#endif
        default:
            break;
    }
}

enum HAL_SDIO_STATUS hal_sdio_get_host_status(enum HAL_SDIO_HOST_ID_T id)
{
    struct SDIO_IP_HOST_T *host = NULL;

    host = &sdio_host[id];
    if (host->sdio == NULL) {
        return HAL_SDIO_STATUS_UNKNOWN;
    }

    return (enum HAL_SDIO_STATUS)host->sdio->has_init;
}

uint32_t hal_sdio_get_host_speed(enum HAL_SDIO_HOST_ID_T id)
{
    if (hal_sdio_get_host_status(id) == HAL_SDIO_STATUS_HAS_INIT) {
        return sdio_host[id].final_bus_speed;
    } else {
        return 0;
    }
}

bool hal_sdio_host_get_dma_cfg(enum HAL_SDIO_HOST_ID_T id)
{
    if (hal_sdio_get_host_status(id) == HAL_SDIO_STATUS_HAS_INIT) {
        return sdio_host[id].dma_en;
    } else {
        return 0;
    }
}

struct HAL_SDIO_HOST_CB_T *hal_sdio_host_get_callback_addr(enum HAL_SDIO_HOST_ID_T id)
{
    if (hal_sdio_get_host_status(id) == HAL_SDIO_STATUS_HAS_INIT) {
        return sdio_host_callback[id];
    } else {
        return NULL;
    }
}

enum HAL_SDIO_ERR hal_sdio_set_clock(enum HAL_SDIO_HOST_ID_T id, uint32_t clock)
{
    struct SDIO_IP_HOST_T *host = NULL;

    host = &sdio_host[id];
    sdio_set_clock(host->sdio, clock);

    return HAL_SDIO_ERR_NONE;
}

enum HAL_SDIO_ERR hal_sdio_set_bus_width(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_HOST_BUS_WIDTH width)
{
    struct SDIO_IP_HOST_T *host = NULL;

    host = &sdio_host[id];
    sdio_set_host_bus_width(host->sdio, width);

    return HAL_SDIO_ERR_NONE;
}

void hal_sdio_dump_run_record(enum HAL_SDIO_HOST_ID_T id)
{
    TRACE(0, "%s, line:%d", __func__, __LINE__);
#ifdef SDIO_RUN_RECORD
    uint16_t i, len, remain;

    TRACE_NOTS(0, "\n sdio_status_ori_index %d, sdio_status_ori[%d]:", sdio_status_ori_index[id], id);
    i = 0;
    remain = SDIO_RUN_RECORD_CNT;
    while (remain) {
        len = remain > 8 ? 8 : remain;
        DUMP32(" %08X", (const char *)((uint32_t)sdio_status_ori[id] + i * 32), len);
        remain -= len;
        i++;
    }

    TRACE_NOTS(0, "\n sdio_opcode_ori_index %d, sdio_opcode_ori[%d]:", sdio_opcode_ori_index[id], id);
    i = 0;
    remain = SDIO_RUN_RECORD_CNT;
    while (remain) {
        len = remain > 16 ? 16 : remain;
        DUMP8(" %3d", (const char *)((uint32_t)sdio_opcode_ori[id] + i * 16), len);
        remain -= len;
        i++;
    }

    TRACE_NOTS(0, "\n sdio_opcode_copy_index %d, sdio_opcode_copy[%d]:", sdio_opcode_copy_index[id], id);
    i = 0;
    remain = SDIO_RUN_RECORD_CNT;
    while (remain) {
        len = remain > 16 ? 16 : remain;
        DUMP8(" %3d", (const char *)((uint32_t)sdio_opcode_copy[id] + i * 16), len);
        remain -= len;
        i++;
    }

    TRACE_NOTS(0, "\n sdio_resp_index %d, sdio_resp[%d]:", sdio_resp_index[id], id);
    i = 0;
    remain = SDIO_RUN_RECORD_CNT;
    while (remain) {
        len = remain > 8 ? 8 : remain;
        DUMP32(" %08X", (const char *)((uint32_t)sdio_resp[id] + i * 32), len);
        remain -= len;
        i++;
    }

    TRACE_NOTS(0, "\n sdio_sleep_wakeup_record_index %d, sdio_sleep_wakeup_record[%d]:", sdio_sleep_wakeup_record_index[id], id);
    i = 0;
    remain = SDIO_RUN_RECORD_CNT;
    while (remain) {
        len = remain > 16 ? 16 : remain;
        DUMP8(" %02X", (const char *)((uint32_t)sdio_sleep_wakeup_record[id] + i * 16), len);
        remain -= len;
        i++;
    }
    TRACE_NOTS(0, " ================dump run record completed================");
#else
    TRACE(0, "SDIO_RUN_RECORD macro is not turned on and this function cannot be used");
#endif
    TRACE_FLUSH();
}

#endif
