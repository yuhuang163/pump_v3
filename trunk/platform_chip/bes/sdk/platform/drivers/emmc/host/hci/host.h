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
#ifndef __MMC_HOST_H__
#define __MMC_HOST_H__

#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
*   Device type Defintion
*/
#define DEV_TYPE_SD                     1
#define DEV_TYPE_SDIO                   2
#define DEV_TYPE_EMMC                   3

/*
*   SDH Register Defintion
*/
#define SDH_SYSADDR                     0x0000
#define SDH_BLOCK_SIZE                  0x0004
#define SDH_BLOCK_COUNT                 0x0006
#define SDH_ARGUMENT                    0x0008
#define SDH_TRANSFER_MODE               0x000C
#define SDH_TRANS_MODE                  0x0C
#define SDH_DMA_EN                      0x01
#define SDH_BLK_CNT_EN                  0x02
#define SDH_AUTO_CMD23                  0x08
#define SDH_AUTO_CMD12                  0x04
#define SDH_AUTO_CMD_DIS                0x00
#define SDH_READ                        0x10
#define SDH_WRITE                       0x00
#define SDH_MULTI_BLK_SEL               0x20
#define SDH_CMD                         0x000E

#define SDH_CMD_RESP_NONE               0x00
#define SDH_CMD_RESP_LONG               0x01
#define SDH_CMD_RESP_SHORT              0x02
#define SDH_CMD_RESP_SHORT_BUSY         0x03
#define SDH_CMD_CRC                     0x08
#define SDH_CMD_INDEX                   0x10
#define SDH_CMD_DATA                    0x20
#define SDH_CMD_ABORT                   0xc0
#define SDH_CMD_RESUME                  0x80
#define SDH_CMD_SUSPEND                 0x40
#define SDH_CMD_NORMAL                  0x00
#define SDH_MAKE_CMD(opcode)            ((opcode) << 8)
#define SDH_RESP0                       0x0010
#define SDH_RESP1                       0x0014
#define SDH_RESP2                       0x0018
#define SDH_RESP3                       0x001C
#define SDH_DP_BUFFER                   0x0020
#define SDH_PRESENT_STATE_1             0x0024
#define SDH_CMD_INHIBIT                 (1 << 0)
#define SDH_DAT_INHIBIT                 (1 << 1)
#define SDH_PRESENT_STATE_2             0x0026
#define SDH_HOST_CTRL                   0x0028
#define SDH_BLK_GAP_CTRL                0x002A
#define SDH_CLOCK_CTRL                  0x002C
#define SDH_TIMEOUT_CTRL_SW_RESET       0x002E
#define SDH_NORMAL_STATUS               0x0030
#define SDH_ERROR_STATUS                0x0032
#define SDH_NORMAL_STATUS_EN            0x0034
#define SDH_ERROR_STATUS_EN             0x0036
#define SDH_NORMAL_STATUS_INT_EN        0x0038
#define SDH_ERROR_STATUS_INT_EN         0x003A
#define SDH_AUTO_CMD_ERROR_STATUS       0x003C
#define SDH_HOST_CTRL_2                 0x003E
#define SDH_CAPABILITIES_1              0x0040
#define SDH_CAPABILITIES_2              0x0042
#define SDH_CAPABILITIES_3              0x0044
#define SDH_CAPABILITIES_4              0x0046
#define SDH_MAX_CURRENT_1               0x0048
#define SDH_MAX_CURRENT_2               0x004A
#define SDH_MAX_CURRENT_3               0x004C
#define SDH_MAX_CURRENT_4               0x004E
#define SDH_FORCE_EVENT_ACMD12ERR       0x0050
#define SDH_FORCE_EVENT_ERROR_STATUS    0x0052
#define SDH_ADMA_ERROR_STATUS           0x0054
#define SDH_ADMA_SYSADDR0               0x0058
#define SDH_ADMA_SYSADDR1               0x005C
#define PRESET_VALUE_FOR_INIT           0x0060
#define PRESET_VALUE_FOR_DS             0x0062
#define PRESET_VALUE_FOR_HS             0x0064
#define PRESET_VALUE_FOR_SDR12          0x0066
#define PRESET_VALUE_FOR_SDR25          0x0068
#define PRESET_VALUE_FOR_SDR50          0x006A
#define PRESET_VALUE_FOR_SDR104         0x006C
#define PRESET_VALUE_FOR_DDR50          0x006E
#define SHARED_BUS_CTRL                 0x00E0
#define SDH_SLOT_INT_STATUS             0x00FC
#define SDH_CTRL_VER                    0x00FE

#define IRQ_CMD_COMP                    (1 <<  0) //reg 0x30
#define IRQ_XFR_COMP                    (1 <<  1)
#define IRQ_BLK_GAP_EVENT               (1 <<  2)
#define IRQ_DMA_INT                     (1 <<  3)
#define IRQ_TX_RDY                      (1 <<  4)
#define IRQ_RX_RDY                      (1 <<  5)
#define IRQ_CARD_INSERTED               (1 <<  6)
#define IRQ_CARD_REMOVED                (1 <<  7)
#define IRQ_CARD_INT                    (1 <<  8)
#define IRQ_INT_A                       (1 <<  9)
#define IRQ_INT_B                       (1 << 10)
#define IRQ_INT_C                       (1 << 11)
#define IRQ_RETUNING_INT                (1 << 12)
#define IRQ_ANY_ERROR                   (1 << 15)
#define IRQ_CMD_TIMEOUT_ERR             (1 << 16) //reg 0x32
#define IRQ_CMD_CRC_ERR                 (1 << 17)
#define IRQ_CMD_ENDBIT_ERR              (1 << 18)
#define IRQ_CMD_INDEX_ERR               (1 << 19)
#define IRQ_DATA_TIMEOUT_ERR            (1 << 20)
#define IRQ_RD_DATA_CRC_ERR             (1 << 21)
#define IRQ_RD_DATA_ENDBIT_ERR          (1 << 22)
#define IRQ_CUR_LIMIT_ERR               (1 << 23)
#define IRQ_AUTOCMD_ERR                 (1 << 24)
#define IRQ_ADMA_ERR                    (1 << 25)
#define IRQ_TUNE_ERR                    (1 << 26) //resv
#define SPI_MODE_ERR                    (1 << 28)
#define AXI_RESP_ERR                    (1 << 29)
#define CPL_TIMEOUT_ERR                 (1 << 30)
#define CRC_STATUS_ERR                  (1 << 31)

#define EMMC_IRQ_CMD_ERR_MASK           (IRQ_CMD_TIMEOUT_ERR    | \
                                        IRQ_CMD_CRC_ERR         | \
                                        IRQ_CMD_ENDBIT_ERR      | \
                                        IRQ_CMD_INDEX_ERR)

#define EMMC_IRQ_DATA_ERR_MASK          (IRQ_DATA_TIMEOUT_ERR   | \
                                        IRQ_RD_DATA_CRC_ERR     | \
                                        IRQ_RD_DATA_ENDBIT_ERR  | \
                                        IRQ_AUTOCMD_ERR         | \
                                        IRQ_ADMA_ERR            | \
                                        CRC_STATUS_ERR)

#define EMMC_IRQ_CARD_MASK              (IRQ_CARD_INSERTED | IRQ_CARD_REMOVED | IRQ_CARD_INT)
#define EMMC_IRQ_ERR_MASK               (EMMC_IRQ_CMD_ERR_MASK | EMMC_IRQ_DATA_ERR_MASK)

/*
*   Bus Mode Definition
*/
#define DS                              0   //sd/sdio, emmc, clk max:25M
#define HS                              1   //sd/sdio,       clk max:50M
#define SDR12                           2   //sd/sdio,       clk max:25M
#define SDR25                           3   //sd/sdio,       clk max:50M
#define SDR50                           4   //sd/sdio,       clk max:100M
#define SDR104                          5   //sd/sdio,       clk max:208M
#define DDR50                           6   //sd/sdio,       clk max:50M
#define SDR52                           7   //emmc,          clk max:52M
#define DDR26                           8   //emmc,          clk max:26M
#define DDR52                           9   //emmc,          clk max:52M
#define HS25                            10  //emmc,          clk max:50M
#define HS50                            11  //emmc,          clk max:50M
#define HS100                           12  //emmc,          clk max:100M
#define HS200                           13  //emmc,          clk max:200M
#define HS400                           14  //emmc,          clk max:200M
#define EHS400                          15  //emmc,          clk max:200M
#define mmc_timing_type(x) ((x==0)?"DS":(x==1)?"HS":(x==2)?"SDR12":(x==3)?"SDR25":  \
                            (x==4)?"SDR50":(x==5)?"SDR104":(x==6)?"DDR50":  \
                            (x==7)?"SDR52":(x==8)?"DDR26":(x==9)?"DDR52":   \
                            (x==10)?"HS25":(x==11)?"HS50":(x==12)?"HS100": \
                            (x==13)?"HS200":(x==14)?"HS400":(x==15)?"EHS400":"UNKNOWN")

/*
*   Bus Clock
*/
#define F_OD                            400     //sd/sdio, emmc
#define F_DS                            24000   //sd/sdio, emmc, Modify according to the frequency of osc
#define F_HS                            48000   //sd/sdio, Modify according to the frequency of osc
#define F_SDR12                         24000   //sd/sdio, Modify according to the frequency of osc
#define F_SDR25                         48000   //sd/sdio, Modify according to the frequency of osc
#define F_DDR50                         48000   //sd/sdio, Modify according to the frequency of osc
#define F_SDR50                         96000   //sd/sdio, Modify according to the frequency of osc
#define F_SDR104                        200000  //sd/sdio, Modify according to the frequency of osc
#define F_SDR52                         48000   //emmc, Modify according to the frequency of osc
#define F_DDR26                         24000   //emmc, Modify according to the frequency of osc
#define F_DDR52                         48000   //emmc, Modify according to the frequency of osc
#define F_HS25                          25000   //emmc, Modify according to the frequency of pll
#define F_HS50                          50000   //emmc, Modify according to the frequency of pll
#define F_HS100                         100000  //emmc, Modify according to the frequency of pll
#define F_HS200                         200000  //emmc, Modify according to the frequency of pll
#define F_HS400                         200000  //emmc, Modify according to the frequency of pll

/*
*   Bus Width
*/
#define MMC_1BIT_WIDTH                  1
#define MMC_4BIT_WIDTH                  2
#define MMC_8BIT_WIDTH                  4
#define mmc_bitwidth_type(x)            ((x==1)?"MMC_1BIT_WIDTH":(x==2)?"MMC_4BIT_WIDTH":(x==4)?"MMC_8BIT_WIDTH":"UNKNOWN")

/*
*   MMC IO Voltage
*/
#define MMC_IO_VOLTAGE_1P2              0
#define MMC_IO_VOLTAGE_1P8              1
#define MMC_IO_VOLTAGE_3P3              2

/*
*   Driver Strength
*/
#define MMC_DRV_STRENGTH_B              0x20
#define MMC_DRV_STRENGTH_A              0x40
#define MMC_DRV_STRENGTH_C              0x80
#define MMC_DRV_STRENGTH_D              0x100

/*
*   DMA
*/
#define MM4_HOST_DMA_BDRY_4KB           0
#define MM4_HOST_DMA_BDRY_8KB           1
#define MM4_HOST_DMA_BDRY_16KB          2
#define MM4_HOST_DMA_BDRY_32KB          3
#define MM4_HOST_DMA_BDRY_64KB          4
#define MM4_HOST_DMA_BDRY_128KB         5
#define MM4_HOST_DMA_BDRY_256KB         6
#define MM4_HOST_DMA_BDRY_512KB         7

#define MM4_SDMA                        0
#define MM4_ADMA_1                      1   //Have been deleted
#define MM4_32_BIT_ADMA_2               2
#define MM4_64_BIT_ADMA_2               3   //not support
#define mmc_dma_mode(x)                 ((x==0)?"SDMA":(x==1)?"ADMA1":(x==2)?"ADMA2_32":(x==3)?"ADMA2_64":"UNKNOWN")

#define ADMA2_DESC_FIXED_SZ             0
#define ADMA2_DESC_FIXED_NR             1
#define ADMA2_DESC_SCATTER              2
#define ADMA2_DESC_MAX_LEN              0x10000 /*64KB*/
#define mmc_adma2_desc_type(x)          ((x==0)?"Fixed descriptor length":(x==1)?"Fixed descriptor number":(x==2)?"Scatter descriptor table":"UNKNOWN")

#define ADMA_DESC_ID_NOP                0x12345678
#define ADMA_DESC_ID_LINK               0x87654321

#define ADMA2_ATTRIBUTE_VALID           (1 << 0)
#define ADMA2_ATTRIBUTE_END             (1 << 1)
#define ADMA2_ATTRIBUTE_INT             (1 << 2)
#define ADMA2_ATTRIBUTE_ACT_NOP         (0 << 4)
#define ADMA2_ATTRIBUTE_ACT_RSV         (1 << 4)
#define ADMA2_ATTRIBUTE_ACT_TRAN        (2 << 4)
#define ADMA2_ATTRIBUTE_ACT_LINK        (3 << 4)
#define ADMA2_DESC_512B                 0x00200
#define ADMA2_DESC_1kB                  0x00400
#define ADMA2_DESC_2kB                  0x00800
#define ADMA2_DESC_4kB                  0x01000
#define ADMA2_DESC_8kB                  0x02000
#define ADMA2_DESC_16kB                 0x04000
#define ADMA2_DESC_32kB                 0x08000
#define ADMA2_DESC_64kB                 0x10000

/*
*   Auto CMD
*/
#define MM4_AUTO_CMD23                  0
#define MM4_AUTO_CMD12                  1
#define MM4_MANUAL_CMD23                2
#define MM4_MANUAL_CMD12                3
#define mmc_autocmd_type(x)             ((x==0)?"MM4_AUTO_CMD23":(x==1)?"MM4_AUTO_CMD12":(x==2)?"MM4_MANUAL_CMD23":(x==3)?"MM4_MANUAL_CMD12":"UNKNOWN")

/*
*   Error record count
*/
#ifndef EMMC_RUN_RECORD_CNT
#define EMMC_RUN_RECORD_CNT             32
#endif

/*
*   Error retry count
*/
#define EMMC_ERROR_RETRY_CNT            3

/*
*   Controller-specified
*/
#define EMMC_BLK_SIZE_MAX               4095
#define EMMC_BLK_CNT_MAX                65535

struct mmc_host;
struct mmc_ios;
struct mmc_cmd;
struct mmc_data;

struct mmc_host_ops {
    void (*set_bus_timing)(struct mmc_host *, unsigned int);
    void (*set_drv_strength)(struct mmc_host *, unsigned int);
    void (*set_io_voltage)(struct mmc_host *, unsigned int);
    void (*clk_gated)(struct mmc_host *, unsigned int);
    void (*vendor_irq)(struct mmc_host *);
    void (*verdor_reg_dump)(struct mmc_host *);
    void (*tuning_pre)(struct mmc_host *);
    void (*tuning_post)(struct mmc_host *);
    unsigned int (*max_delay)(struct mmc_host *);
    void (*set_delay)(struct mmc_host *, unsigned int);
    int (*hw_tuning)(struct mmc_host *, unsigned int);
};

struct mmc_ios {
    unsigned int voltage;
    unsigned int timing;
    unsigned int width;
    unsigned int khz;
    unsigned int drv;

    unsigned int vccmain;
};

enum EMMC_HOST_ERR {
    EMMC_HOST_CMD_LINE_CONFLICT_ERR = 1 << 10,//cmd resp crc error and cmd line error
    EMMC_HOST_CMD_TIMEOUT_ERR       = 1 << 11,//cmd resp timeout error
    EMMC_HOST_CMD_CRC_ERR           = 1 << 12,//cmd resp crc error
    EMMC_HOST_CMD_END_BIT_ERR       = 1 << 13,//cmd resp end bit error
    EMMC_HOST_CMD_INDEX_ERR         = 1 << 14,//cmd resp index error
    EMMC_HOST_DATA_TIMEOUT_ERR      = 1 << 15,//host rx data timeout or write crc status timeout error
    EMMC_HOST_RD_DATA_CRC_ERR       = 1 << 16,//host rx data crc or write crc status having a value other than 010 detected
    EMMC_HOST_RD_DATA_END_BIT_ERR   = 1 << 17,//host rx data end bit error or at the end bit position of the crc status error
    EMMC_HOST_CUR_LIMIT_ERR         = 1 << 18,//[not used]not supported and this bit will always be read as 0
    EMMC_HOST_AUTO_CMD12_ERR        = 1 << 19,//auto cmd12 error
    EMMC_HOST_ADMA_ERR              = 1 << 20,//advanced dma error
    EMMC_HOST_SPI_ERR               = 1 << 21,//[not used]spi mode error
    EMMC_HOST_AXI_RESP_ERR          = 1 << 22,//[not used]
    EMMC_HOST_CPL_TIMEOUT_ERR       = 1 << 23,//[not used]CE-ATA mode only
    EMMC_HOST_CRC_STATUS_ERR        = 1 << 24,//host tx error
};

enum EMMC_CLK_SRC_SEL_T {
    EMMC_CLK_SRC_SEL_UNKNOWN = 0,
    EMMC_CLK_SRC_SEL_OSCX4,
    EMMC_CLK_SRC_SEL_BBPLL,
    EMMC_CLK_SRC_SEL_DSIPLL,
    EMMC_CLK_SRC_SEL_OSCX2,
};

struct mmc_adma_desc {
    unsigned int adma_desc_ctrl;
    unsigned int adma_desc_addr;
};

struct mmc_host {
    char *name;
    unsigned int id;
    unsigned int version;

    /*hardware*/
    uintptr_t base;
    unsigned int irq;
    unsigned int clk_in_khz;

    /*Host Mode*/
    unsigned char in_use;
    unsigned char irq_en;
    unsigned char irq_en_cp;
    unsigned char dma_status;    //0: disable, 1: enable
    unsigned char tran_mode;     //0: no-blocking mode, 1: blocking mode
    unsigned char tran_status;   //1: (CARD_DATA_STATE|CARD_RCV_STATE), 0: other status
    unsigned char tuning_status; //1: tuning in progress, 0: normal status
    unsigned char clk_gating_en;
    unsigned char clk_gating_status;
    unsigned char internal_gating_en;//gating between xclk and oclk
    unsigned char resp_polling_mode; //0: interrupt mode, 1:polling mode
    unsigned int dma_mode;
    unsigned int auto_cmd_type;
    enum EMMC_CLK_SRC_SEL_T clk_src;

    /*ADMA run-time parameters*/
    unsigned int dma_bdry;
    unsigned int desc_sz;
    unsigned int desc_nr;
    unsigned int desc_type;
    unsigned int *desc;
    unsigned int desc_len;
    unsigned int desc_cnt;
    unsigned int desc_index;
    unsigned int desc_ctrl_max_len;

    void *aligned_buf;
    void *actual_buf;                       /*point to the current buff for transfer*/

#if defined(EMMC_EXTENDED_USE)
#define MMC_MAX_XFER_SZ         0x8000  /*32KB*/
    void *alloc_p;
    unsigned int data_irq;
#elif defined(EMMC_USE_HEAP)
#define MMC_MAX_XFER_SZ         0x2800  /*10KB*/
#else
#define MMC_MAX_XFER_SZ         8
#endif

    //Used for heap release
#ifdef EMMC_USE_HEAP
    unsigned int desc_copy;
    unsigned int aligned_buf_copy;
    unsigned int actual_buf_copy;
#endif

    unsigned int blksz;
    unsigned int blks;

    struct mmc_ios ios;

    struct mmc_card *card;
    struct mmc_card_info *card_info;
    struct mmc_cmd *cmd;
    struct mmc_data *data;

    struct mmc_host_ops *ops;

    void *private;

    unsigned int card_inserted;
    unsigned int card_removed;
    unsigned int card_interrupt;
    unsigned int blk_gap_evt;

    unsigned int quirks;

    /*timing related*/
    unsigned int tx;
    unsigned int rx;
    unsigned int strobe;
    unsigned int vmin_begin;
    unsigned int speed_mode;

    /*error code*/
    int error_code;//Used for function return values
    int host_error;//Used to log errors in interrupts

    /*delay*/
    void (*delay)(unsigned int);

    /*callback*/
    void (*emmc_host_error)(enum EMMC_HOST_ERR error);  //host tx/rx error
    void (*emmc_host_dma_done)(void);                   //Identification of data transfer completion in non-blocking mode
};

/*
*   host quirks
*/
#define MMC_QUIRKS_HW_TUNING            0x00000001
#define MMC_QUIRKS_SW_TUNING            0x00000002
#define MMC_QUIRKS_FIXED_DELAY          0x00000004
#define MMC_QUIRKS_HW_TUNING_SHMOO      0x00000008
#define MMC_QUIRKS_SHMOO_XML            0x00000010
#define MMC_QUIRKS_PERFORMANCE          0x00000020

#define XFER_TIMEOUT                    2000    //ms,slow timer,sdr52 20ns/byte,2s=95MB;hs200 5ns/byte,2s=381MB
#define SMALL_DATA_XFER_TIMEOUT         1000    //ms,slow timer
#define WAIT_SM_IDLE_TIMEOUT            2000    //ms,fast timer,SM=state machine
#define WAIT_BUS_IDLE_TIMEOUT           500     //ms,fast timer
#define WAIT_RESPONSE_TIMEOUT           500     //ms,fast timer
#define WAIT_STATUS_TIMEOUT             500     //ms,fast timer
#define WAIT_INTE_CLK_TIMEOUT           500     //ms,fast timer
#define DEFAULT_CMD6_TIMEOUT            500     //ms,fast timer

/*
*   Host Common Functions Definitions
*/
void sdhci_force_clk_on(struct mmc_host *host);
void sdhci_force_clk_off(struct mmc_host *host);
void sdhci_sel_base_clk(struct mmc_host *host);
void sdhci_sel_work_clk(struct mmc_host *host);
void sdhci_set_clk_div_bypass(struct mmc_host *host);
void sdhci_mask_irqs(struct mmc_host *host, unsigned int irqs);
void sdhci_mask_raw_status(struct mmc_host *host, unsigned int status);
void sdhci_unmask_irqs(struct mmc_host *host, unsigned int irqs);
void sdhci_unmask_raw_status(struct mmc_host *host, unsigned int status);
void sdhci_unmask_irqs_ext(struct mmc_host *host, unsigned int irqs, unsigned int status_en);
int  sdhci_start_int_clk(struct mmc_host *host);
int  sdhci_stop_int_clk(struct mmc_host *host);
void sdhci_start_bus_clk(struct mmc_host *host);
void sdhci_stop_bus_clk(struct mmc_host *host);
void sdhci_set_voltage(struct mmc_host *host, unsigned int voltage);
void sdhci_set_bus_clk(struct mmc_host *host, unsigned int khz);
void sdhci_set_bus_width(struct mmc_host *host, unsigned int width);
void sdhci_set_bus_timing(struct mmc_host *host, unsigned int timing);
void sdhci_set_drv_strength(struct mmc_host *host, unsigned int drv);
void sdhci_reg_dump(struct mmc_host *host);
int  sdhci_try_tuning(struct mmc_host *host, unsigned int cmd);
int sdhci_tuning_cmdline(struct mmc_host *host);
void sdhci_open_internal_cmd_gate(struct mmc_host *host);
void sdhci_close_internal_cmd_gate(struct mmc_host *host);
void sdhci_set_clk_cnt_bf_gate(struct mmc_host *host, unsigned int clkcnt);
int  sdhci_get_clk_cnt_bf_gate(struct mmc_host *host);
int  sdhci_set_turn_around_cycle(struct mmc_host *host, unsigned int data0_cycle, unsigned int cmd_cycle);
void sdhci_get_turn_around_cycle(struct mmc_host *host, unsigned int *data0_cycle, unsigned int *cmd_cycle);
void sdhci_set_oclk_out(struct mmc_host *host);

//Note: can only be used after emmc initialization is complete
int sdhci_set_bus_speed_and_clksrc(struct mmc_host *host, uint8_t bus_speed_mode, enum EMMC_CLK_SRC_SEL_T clk_src_sel);

#ifdef __cplusplus
}
#endif

#endif
