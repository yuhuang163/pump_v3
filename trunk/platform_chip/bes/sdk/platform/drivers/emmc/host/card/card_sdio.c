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
#include "host.h"
#include "core.h"
#include "card.h"
#include "error.h"
#include "string.h"
#include "card_mmc.h"
#include "emmc_dbg.h"
#include "card_sdio.h"
#include "sdhci_emphy.h"
#include "emmc_platform.h"
#include "hal_sdio_host.h"
#include "sdhci_unifiedAl.h"

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

/* int pending regiter bit */
#define SDIO_CCCR5_INT1             0x02
#define SDIO_CCCR5_INT2             0x04
#define SDIO_CCCR5_INT3             0x08
#define SDIO_CCCR5_INT4             0x10
#define SDIO_CCCR5_INT5             0x20
#define SDIO_CCCR5_INT6             0x40
#define SDIO_CCCR5_INT7             0x80

/* function1 regiter(sdio device register) */
#define SDIO_FUNC1_LOAD_CFG         0x104
#define SDIO_FUNC1_RX_BUF_CFG       0x108
#define SDIO_FUNC1_INT_TO_DEVICE    0x120

#define SDIO_FBR_BASE(x)            ((x) * 0x100)

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
    uint8_t func_status;    /* func enable status 0:disable 1:enable */
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
    struct SDIO_FUNC_T *func[SDIO_FUNC_NUM];
};

static struct SDIO_FUNC_T host_sdio_func[SDIO_FUNC_NUM];
static struct SDIO_CORE_T host_sdio_core;

/* sdio use mmc_send_cmd_wait_resp(), send_stop argument must be 0
 * sdio not support cmd13, dont have status register,dont support
 */
static int sdio_io_rw_direct(struct mmc_host *host, enum SDIO_DEVICE_RW read0_write1, enum SDIO_FUNC_NUM_T func_num, uint32_t addr, uint8_t data, uint8_t *rsp)
{
    int ret = MMC_NO_ERR;
    uint32_t argument;

    argument  = read0_write1 ? 0x80000000 : 0x00000000;                    //R/W flag
    argument |= ((uint32_t)(func_num & 0x7)) << 28;                        //function number
    argument |= (read0_write1 && (uintptr_t)rsp) ? 0x08000000 : 0x00000000; //RAW flag
    argument |= addr << 9;                                                 //register address
    argument |= data;                                                      //write data

    ret = mmc_send_cmd_wait_resp(host, IO_RW_DIRECT, argument, MMC_RSP_R5, CARD_RESPONSE_R5, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    if (rsp) {
        *rsp = host->cmd->resp[0];
    }

    return ret;
}

static int sdio_get_cccr_version(struct mmc_host *host, uint8_t *cccr_version)
{
    int ret = MMC_NO_ERR;
    uint8_t cccrx = 0;

    if (!cccr_version) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_INVALID_PARAMETER;
    }

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_SDIO_VERSION, 0, &cccrx);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_GET_VERSION_FAIL;
    }

    *cccr_version = cccrx & 0xf;

    return ret;
}

static int sdio_get_sdio_version(struct mmc_host *host, uint8_t *sdio_version)
{
    int ret = MMC_NO_ERR;
    uint8_t sdiox = 0;

    if (!sdio_version) {
        HAL_EMMC_TRACE(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_INVALID_PARAMETER;
    }

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_SDIO_VERSION, 0, &sdiox);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_TRACE(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_GET_VERSION_FAIL;
    }

    *sdio_version = (sdiox >> 4) & 0xf;

    return  ret;
}

static int sdio_switch_voltage(struct mmc_host *host, int signal_voltage)
{
    int ret = MMC_NO_ERR;
    uint32_t status;
    uint32_t timeout, busy_t;

    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);

    if (signal_voltage == MMC_IO_VOLTAGE_3P3) {
        HAL_EMMC_ERROR(0, "%s, NOT SUPPORT", __FUNCTION__);
        return MMC_SWITCH_ERR;
    }

    /*
     * Send CMD11 only if the request is to switch the card to 1.8V signaling
     */
    ret = mmc_send_cmd_wait_resp(host, VOLTAGE_SWITCH, 0, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    /* when device respnse,device drive cmd and data to low level, master need to close clk */
    /* wait for 1 ms to checkout data */
    sdhci_stop_bus_clk(host);
    hal_sys_timer_delay(MS_TO_TICKS(1));
    timeout = MS_TO_FAST_TICKS(10);
    busy_t = hal_fast_sys_timer_get();
    while (1) {
        if (hal_fast_sys_timer_get() - busy_t < timeout) {
            status =  sdhciReadSdh_present_state_2Dat_level(host);
            if (!status) {
                break;
            } else {
                hal_sys_timer_delay(MS_TO_TICKS(1));
            }
        } else {
            HAL_EMMC_ERROR(0, "%s, data/cmd line sigaling still high", __FUNCTION__);
            return HAL_SDIO_VOLT_SWITCH_FAIL;
        }
    }

    /* clk idle low, data/cmd idle high */
    hal_sys_timer_delay(MS_TO_TICKS(5));
    /* supply clk */
    sdhci_start_bus_clk(host);
    sdhci_force_clk_on(host);
    /* Wait for at least 1 ms according to spec */
    hal_sys_timer_delay(MS_TO_TICKS(1));
    sdhci_force_clk_off(host);
    /* device drive data to high */
    timeout = US_TO_FAST_TICKS(3000);
    busy_t = hal_fast_sys_timer_get();
    while (1) {
        if (hal_fast_sys_timer_get() - busy_t < timeout) {
            status =  sdhciReadSdh_present_state_2Dat_level(host);
            if (status == 0xF) {
                break;
            } else {
                hal_sys_timer_delay(US_TO_TICKS(100));
            }
        } else {
            HAL_EMMC_ERROR(0, "%s, wait for bus idle timeout", __FUNCTION__);
            return MMC_BUS_IDLE_TIMEOUT;
        }
    }
    return ret;
}

int mmc_init_sdio_card(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    uint8_t r4_s18a, r4_funcs, r4_mp;
    uint8_t cmd3r6_com_crc_error, cmd3r6_illegal_command, cmd3r6_error;

    memset(host_sdio_func, 0, sizeof(struct SDIO_FUNC_T) * SDIO_FUNC_NUM);
    memset((void *)&host_sdio_core, 0, sizeof(struct SDIO_CORE_T));
    host_sdio_func[SDIO_FUNC_0].func_int_status = FUNC_INT_ENABLE;
    /* switch to 1.8v depends one the S18A value */
    r4_s18a = (host->cmd->resp[0] & ACCESS_MODE_S18A) >> 24;
    r4_funcs = (host->cmd->resp[0] & 0x70000000) >> 28;
    r4_mp = (host->cmd->resp[0] & 0x8000000) >> 27;

    HAL_EMMC_TRACE(1, "R4->IO OCR=0x%X", host->cmd->resp[0] & 0x00FFFFFF);
    HAL_EMMC_TRACE(1, "R4->S18A=%d", r4_s18a);
    HAL_EMMC_TRACE(1, "R4->Memory Present=%d", r4_mp);
    HAL_EMMC_TRACE(1, "R4->IO functions=%d", r4_funcs);

    host_sdio_core.func_total_num = r4_funcs;
    for (uint8_t index = 0; index <= host_sdio_core.func_total_num; index++) {
        host_sdio_core.func[index] = &host_sdio_func[index];
        host_sdio_func[index].func_num = index;
    }

    /*
     *3.3 S18A value 0,the card does not support 1.8v signaling
     *1.8 S18A value 0,alreay switched to 1.8V
     */
    if (r4_s18a) {
        ret = sdio_switch_voltage(host, MMC_IO_VOLTAGE_1P8);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return HAL_SDIO_VOLT_SWITCH_FAIL;
        }
    }
    /* Enable interrupts (RCA response has valid cmd indx and cmd crc) */
    sdhci_unmask_irqs(host, IRQ_CMD_CRC_ERR | IRQ_CMD_INDEX_ERR);

    ret = mmc_send_cmd_wait_resp(host, SET_RELATIVE_ADDR, 0, MMC_RSP_R6, CARD_RESPONSE_R6, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    host->card->rca = (host->cmd->resp[0] >> 16) & 0xffff;
    /* only use bit15-bit13,sdio does not support cmd13 */
    cmd3r6_com_crc_error = (host->cmd->resp[0] & 0x8000) >> 15;
    cmd3r6_illegal_command = (host->cmd->resp[0] & 0x4000) >> 14;
    cmd3r6_error = (host->cmd->resp[0] & 0x2000) >> 13;
    if ((cmd3r6_com_crc_error == 0) && (cmd3r6_illegal_command == 0) && (cmd3r6_error == 0)) {
        HAL_EMMC_TRACE(1, "R6->RCA=0x%X", host->card->rca);
        HAL_EMMC_TRACE(1, "R6->COM CRC ERROR=%d", cmd3r6_com_crc_error);
        HAL_EMMC_TRACE(1, "R6->ILLEGAL COMMAND=%d", cmd3r6_illegal_command);
        HAL_EMMC_TRACE(1, "R6->ERROR=%d", cmd3r6_error);
        HAL_EMMC_TRACE(0, "---SDIO INIT SUCCESS---");

        /* cmd7 select card*/
        ret = mmc_send_cmd_wait_resp(host, SELECT_CARD, (host->card->rca << 16), MMC_RSP_R1B, CARD_RESPONSE_R1B, 0, 0);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }

        /* bit12-bit9 CURRENT_STATE,Fh indicates I/O only card */
        if ((host->cmd->resp[0] & 0x1E00) == 0x1E00) {
            HAL_EMMC_TRACE(0, "CURRENT_STATE is sdio mode,ret:%d", ret);
        }
    }
    /* the device signal valtage is alreay 1.8v or the card does not support UHS-I*/
    if (!r4_s18a) {
        HAL_EMMC_TRACE(0, "voltage of device is already 1.8V or the device does not support UHS-I");

        /* Read cccr, check if the card supports UHS-I:cmd52 */
        ret = mmc_send_cmd_wait_resp(host, IO_RW_DIRECT, SDIO_CCCR_UHSI_SUPPORT << 9, MMC_RSP_R5, CARD_RESPONSE_R5, 0, 0);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
        if (host->card->r5 & 0x7) {
            HAL_EMMC_TRACE(0, "---device support UHS-I---");
        } else {
            HAL_EMMC_TRACE(0, "device does not support UHS-I, so the host does not do voltage switching");
        }

    }

    ret = sdio_get_cccr_version(host, &host_sdio_core.cccr_version);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    ret = sdio_get_sdio_version(host, &host_sdio_core.sdio_version);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    HAL_EMMC_TRACE(1, "cccr_version:%d,%s", host_sdio_core.cccr_version, (host_sdio_core.cccr_version == 3) ? "3.00" : "*.00");
    HAL_EMMC_TRACE(1, "sdio_version:%d,%s", host_sdio_core.sdio_version, (host_sdio_core.sdio_version == 4) ? "3.00" : "*.00");

    return ret;
}

static int sdio_set_device_bus_width(struct mmc_host *host, uint8_t bus_width)
{
    int ret = MMC_NO_ERR;
    uint8_t width = 0;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_BUS_CONTROL, 0, &width);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return MMC_INVALID_ERR;
    }

    width &= ~0x3;
    width |= bus_width;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_WRITE, SDIO_FUNC_0, SDIO_CCCR_BUS_CONTROL, width, NULL);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_SET_BUS_WIDTH_FAIL;
    }

    return ret;
}

static int sdio_set_bus_speed(struct mmc_host *host, unsigned int bus_speed)
{
    int ret = MMC_NO_ERR;
    uint8_t val = 0;

    /* BSS2 BSS1 BSS0 */
    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_BUS_SPEED_SEL, 0, &val);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return MMC_INVALID_ERR;
    }

    val &= 0xF1;
    val |= (uint8_t)(bus_speed << 1);

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_WRITE, SDIO_FUNC_0, SDIO_CCCR_BUS_SPEED_SEL, val, NULL);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_SET_BUS_SPEED_FAIL;
    }

    return ret;
}

static int sdio_get_cis_ptr(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num, uint32_t *ptr_addr)
{
    int ret = MMC_NO_ERR;
    uint8_t data = 0;
    uint8_t index;
    uint32_t temp = 0;

    if (func_num >= SDIO_FUNC_NUM || (!ptr_addr)) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_INVALID_PARAMETER;
    }

    /* common cis pointer:0x09,0x0A,0x0B ,24 bit*/
    for (index = 0; index < 3; index++) {
        ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_CIS_PTR + index, 0, &data);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
        temp |= data << (index * 8);
    }

    *ptr_addr = temp;

    return ret;
}

static int sdio_cis_parse(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num, uint32_t cis_ptr)
{
    int ret = MMC_NO_ERR;
    uint8_t data[255];
    uint8_t index = 0;
    int len = 0;
    uint8_t tpl_code = CISTPL_NULL;
    uint8_t tpl_link = 0;
    uint32_t temp_cis_ptr = cis_ptr;
    memset(data, 0, sizeof(data));

    /* 16.2 basic tuple format and tuple chain structure */
    while (1) {
        ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, temp_cis_ptr++, 0, &tpl_code);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
        HAL_EMMC_TRACE(2, "CIS[0x%x] tpl_code=0x%X", temp_cis_ptr - 1, (uint32_t)tpl_code);
        if ((tpl_code == CISTPL_NULL) || (tpl_code == CISTPL_END)) {
            break;
        }

        ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, temp_cis_ptr++, 0, &tpl_link);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
        HAL_EMMC_TRACE(2, "CIS[0x%x] tpl_link=%d", temp_cis_ptr - 1, tpl_link);

        for (index = 0; index < tpl_link; index ++) {
            ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, temp_cis_ptr + index, 0, &data[index]);
            if (ret != MMC_NO_ERR) {
                HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            HAL_EMMC_TRACE(2, "tpl data[%d]=0x%x", index, data[index]);
        }
        /* 16.5 SDIO CARD Metaformat */
        switch (tpl_code) {
            case CISTPL_VERS_1:
                HAL_EMMC_TRACE(0, "Product Information:");
                for (index = 2; data[index] != 0xff; index += len + 1) {
                    len = strlen((char *)data + index);
                    if (len)
                        HAL_EMMC_TRACE(1, " %s", data + index);
                }
                break;
            case CISTPL_MANFID:
                /* 16.6 CISTPL_MANFID: Manufacturer Identification String Tuple */
                HAL_EMMC_TRACE(1, "Manufacturer Code: 0x%04x", *(uint16_t *)data);
                HAL_EMMC_TRACE(1, "Manufacturer Information: 0x%04x", *(uint16_t *)(data + 2));
                host_sdio_core.manf_code = *(uint16_t *)data;
                host_sdio_core.manf_info = *(uint16_t *)(data + 2);
                break;
            case CISTPL_FUNCID:
                /* 16.7.1 CISTPL_FUNCID: Function Identification Tuple */
                HAL_EMMC_TRACE(1, "Card Function Code: 0x%02X", data[0]);
                HAL_EMMC_TRACE(1, "System Initialization Bit Mask: 0x%02X", data[1]);
                break;
            case CISTPL_FUNCE:
                /* 16.7.2 CISTPL_FUNCE: Function Extension Tuple */
                if (data[0] == 0) {
                    /* 16.7.3 CISTPL_FUNCE Tuple for Function 0 (Extended Data 00h) */
                    HAL_EMMC_TRACE(1, "Maximum Block Size case1: func: %d,size %d", func_num, *(uint16_t *)(data + 1));
                    HAL_EMMC_TRACE(1, "Maximum Transfer Rate Code: 0x%02X", data[3]);
                    if (host_sdio_core.func[func_num]) {
                        host_sdio_core.func[func_num]->max_blk_size = *(uint16_t *)(data + 1);
                    }
                } else if (data[0] == 1) {
                    /* 16.7.4 CISTPL_FUNCE Tuple for Function 1-7 (Extended Data 01h) */
                    HAL_EMMC_TRACE(2, "Maximum Block Size case2 func: %d,size %d", func_num, *(uint16_t *)(data + 0x0c));
                    if (host_sdio_core.func[func_num]) {
                        host_sdio_core.func[func_num]->max_blk_size = *(uint16_t *)(data + 0x0c);
                    }
                } else {
                    HAL_EMMC_TRACE(2, "TPLFE_PS func: %d,size %d", func_num, *(uint16_t *)(data + 0x02));
                }
                break;
            default:
                break;
        }

        if (tpl_link == CISTPL_END)
            break;
        else
            temp_cis_ptr += tpl_link;
    }

    return ret;
}

static int sdio_enable_func(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num)
{
    int ret = MMC_NO_ERR;
    uint8_t enable = 0;
    uint8_t ready = 0;

    if (func_num >= SDIO_FUNC_NUM) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_INVALID_PARAMETER;
    }
    /* 02H IOE7~IOE1*/
    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, 0, &enable);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_ENABLE_FUNCTION_FAIL;
    }

    enable |= (1 << func_num);

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_WRITE, SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, enable, NULL);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_ENABLE_FUNCTION_FAIL;
    }
    /* 03H IOR7~IOR1*/
    while (!(ready & (1 << func_num))) {
        sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_IO_READY, 0, &ready);
    }

    if (host_sdio_core.func[func_num]) {
        host_sdio_core.func[func_num]->func_status = FUNC_ENABLE;
    }
    return ret;
}

static int sdio_enable_func_int(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num)
{
    int ret = MMC_NO_ERR;
    uint8_t enable = 0;

    if (func_num >= SDIO_FUNC_NUM) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_INVALID_PARAMETER;
    }
    /* 04H IEN7-IEN1 */
    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_ENABLE_FUNCTION_INT_FAIL;
    }

    enable |= (1 << func_num);

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_WRITE, SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_ENABLE_FUNCTION_INT_FAIL;
    }

    return ret;
}

static int sdio_enable_master_int(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    uint8_t enable = 0;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_ENABLE_MASTER_INT_FAIL;
    }

    enable |= 0x1;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_WRITE, SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_ENABLE_MASTER_INT_FAIL;
    }

    return ret;
}

static int sdio_enable_asyn_int(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    uint8_t int_ext = 0;
    /* 16H EAT*/
    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_INT_EXTERN, 0, &int_ext);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_ENABLE_INT_EXT_FAIL;
    }

    int_ext |= 0x2;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_WRITE, SDIO_FUNC_0, SDIO_CCCR_INT_EXTERN, int_ext, NULL);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_ENABLE_INT_EXT_FAIL;
    }

    return ret;
}

/* use CMD53 */
int sdio_io_rw_extended(struct mmc_host *host, enum SDIO_DEVICE_RW read0_write1, enum SDIO_FUNC_NUM_T func_num, uint32_t addr, uint8_t *rsp, uint8_t incr_addr, uint8_t *data_buff,
                        uint32_t data_size, enum BLOCKING_MODE blocking_mode, bool dma_en)
{
    int ret = MMC_NO_ERR;
    uint16_t func_cur_blk_size;
    uint16_t blk_count;
    uint32_t argument;
    uint32_t flags;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    if (host_sdio_core.func[func_num]) {
        func_cur_blk_size = host_sdio_core.func[func_num]->cur_blk_size;
        if (!func_cur_blk_size) {
            return HAL_SDIO_BLOCK_SIZE_ZERO;
        }
    } else {
        return HAL_SDIO_INVALID_FUNCTION_NUM;
    }

    mmc_card_set_blocking_mode(host, blocking_mode);

    HAL_EMMC_TRACE(0, "write card blocking mode:%d, irq_en:%d, irq_en_cp:%d", blocking_mode, host->irq_en, host->irq_en_cp);
    /* Automatically select byte mode or block mode, block_count is at least 1 */
    if (data_size < func_cur_blk_size) {
        blk_count = 1;
    } else {
        if (data_size % func_cur_blk_size) {
            blk_count = data_size / func_cur_blk_size + 1;
        } else {
            blk_count = data_size / func_cur_blk_size;
        }

        if (blk_count > 511) {
            HAL_EMMC_ERROR(0, "blk_count %d is over 511", blk_count);
            return HAL_SDIO_INVALID_PARAMETER;
        }
    }

    argument  = read0_write1 ? 0x80000000 : 0x00000000;   //R/W flag
    argument |= ((uint32_t)(func_num & 0x7)) << 28;       //funcion number
    argument |= incr_addr ? (1 << 26) : 0;                //op code
    argument |= addr << 9;                                //register address

    if (read0_write1) {       // 0 read 1write
        flags = MMC_DATA_PRESENT | MMC_DATA_WRITE;
    } else {
        flags = MMC_DATA_PRESENT | MMC_DATA_READ;
    }

    if (blk_count > 1) {
        /* block mode */
        argument |= (1 << 27) | blk_count;
        flags |= MMC_DATA_AUTOCMD_DIS | MMC_DATA_MB | MMC_DATA_BC_EN;
    } else {
        /* bytes mode */
        argument |= (data_size == 512) ? 0 : data_size;
    }
    if (dma_en) {
        flags |= MMC_DATA_DMA;
    }
    host->dma_status = true;
    mmc_set_tran_mode(host, blocking_mode);

    if (blk_count == 1) {
        if (read0_write1) {
            mmc_prepare_transfer(host, data_buff, CARD_WRITE, data_size, data_size, dma_en);
        } else {
            mmc_prepare_transfer(host, data_buff, CARD_READ, 1, data_size, dma_en);
        }
    } else if (read0_write1) {
        mmc_prepare_transfer(host, data_buff, CARD_WRITE, func_cur_blk_size, data_size, dma_en);
    } else {
        mmc_prepare_transfer(host, data_buff, CARD_READ, func_cur_blk_size, data_size, dma_en);
    }

    sdhci_mask_irqs(host, IRQ_CMD_COMP | EMMC_IRQ_CMD_ERR_MASK);

    ret = mmc_send_cmd_wait_resp(host, IO_RW_EXTENDED, argument, MMC_RSP_R5 | flags, CARD_RESPONSE_R5, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        goto error_out;
    }
    sdhci_unmask_irqs(host, IRQ_CMD_COMP | EMMC_IRQ_CMD_ERR_MASK);

    if (rsp) {
        *rsp = host->cmd->resp[0];
    }

    if (blocking_mode) {
        ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
        if (ret != MMC_NO_ERR) {
            goto error_out;
        }
    } else {
        if (host->dma_status && host->emmc_host_dma_done) {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
            return MMC_NO_ERR;
        } else {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
            if (ret != MMC_NO_ERR) {
                goto error_out;
            }
        }
    }

error_out:
    host->tran_status = 0;

    card_controller_clk_pause(host);
    host->in_use = 0;

    return ret;
}

static int sdio_set_block_size(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num, uint16_t block_size)
{
    int ret = MMC_NO_ERR;
    /* 10H-11H set block size */
    ret = sdio_io_rw_direct(host, SDIO_DEVICE_WRITE, SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_BLK_SIZE, block_size & 0xff, NULL);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_WRITE, SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_BLK_SIZE + 1, (block_size >> 8) & 0xff, NULL);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    if (host_sdio_core.func[func_num]) {
        host_sdio_core.func[func_num]->cur_blk_size = block_size;
    } else {
        return HAL_SDIO_INVALID_FUNCTION_NUM;
    }

    return ret;
}

int sdio_set_func_abort(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num)
{
    int ret = MMC_NO_ERR;
    uint8_t abort = 0;
    /* only byte mode, equivalent to CMD12 */
    if (func_num >= SDIO_FUNC_NUM) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_INVALID_PARAMETER;
    }

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_IO_ABORT, 0, &abort);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_SET_ABORT_FAIL;
    }

    abort |= func_num;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_WRITE, SDIO_FUNC_0, SDIO_CCCR_IO_ABORT, abort, NULL);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_SET_ABORT_FAIL;
    }

    return ret;
}

int mmc_config_sdio_card(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width)
{
    int ret = MMC_NO_ERR;
    POSSIBLY_UNUSED uint32_t bus_speed = 0;
    uint32_t bus_width = 0;
    uint8_t func_index;
    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);

    /* sdio use cmd52 write regiset instead of cmd6 */
    switch (width) {
        case MMC_1BIT_WIDTH:
            bus_width = 0x0;
            break;
        case MMC_4BIT_WIDTH:
            bus_width = 0x2;
            break;
        case MMC_8BIT_WIDTH:
            bus_width = 0x3;
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unsupported bus width", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }

    ret = sdio_set_device_bus_width(host, bus_width);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    sdhci_set_bus_width(host, width);
    /* 3.3V only use BSS0,1.8V use BBS[2:0] support to SDR104
     * SDR must set SDR104(Otherwise write error),max clock frequency 208M
     * DDR must set DDR50
     */
    switch (timing) {
        case DS:
        case SDR12:
        case HS:
        case SDR25:
        case SDR50:
        case SDR104:
            bus_speed = 0x3;
            break;
        case DDR50:
        case DDR52:
            bus_speed = 0x4;
            break;
        default:
            HAL_EMMC_ERROR(0, "%s:%d, Unsupported timing mode", __FUNCTION__, __LINE__);
            return MMC_INVALID_ERR;
            break;
    }

    ret = sdio_set_bus_speed(host, bus_speed);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    sdhci_set_bus_timing(host, timing);

    sdhci_set_bus_clk(host, khz);
    for (func_index = 0; func_index < 1; func_index++) {
        uint32_t cis_ptr;
        ret = sdio_get_cis_ptr(host, (enum SDIO_FUNC_NUM_T)func_index, &cis_ptr);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
        HAL_EMMC_TRACE(0, "------cis_ptr[%d]:0x%X", func_index, cis_ptr);
        sdio_cis_parse(host, (enum SDIO_FUNC_NUM_T)func_index, cis_ptr);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }

    }
    /* function enable */

    for (func_index = SDIO_FUNC_1; func_index <= host_sdio_core.func_total_num; func_index++) {
        /* enable Func */
        ret = sdio_enable_func(host, (enum SDIO_FUNC_NUM_T)func_index);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
        HAL_EMMC_TRACE(2, "sdio device func%d has been turned on,error:%d", func_index, ret);

        /* Enable interrupt of each FUNC */
        ret = sdio_enable_func_int(host, (enum SDIO_FUNC_NUM_T)func_index);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
        HAL_EMMC_TRACE(2, "sdio device func%d int has been turned on,error:%d", func_index, ret);
    }
    HAL_EMMC_TRACE(0, "sdio:enable function ok");
    HAL_EMMC_TRACE(1, "sdio:enable function int ok");

    ret = sdio_enable_master_int(host);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    HAL_EMMC_TRACE(1, "sdio:enable master int ok");

    sdio_enable_asyn_int(host);

    /* set block size*/
    uint32_t block_size = 512;
    for (func_index = SDIO_FUNC_0; func_index <= host_sdio_core.func_total_num; func_index++) {
        ret = sdio_set_block_size(host, (enum SDIO_FUNC_NUM_T)func_index, block_size);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }
    HAL_EMMC_TRACE(1, "sdio:enable function block size ok, function number %d size %d", host_sdio_core.func_total_num, block_size);

    sdhci_mask_irqs(host, IRQ_RD_DATA_CRC_ERR);

    uint8_t cis_data;
    uint32_t cis_ptr;
    ret = sdio_get_cis_ptr(host, (enum SDIO_FUNC_NUM_T)0, &cis_ptr);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    ret = sdio_io_rw_extended(host, SDIO_DEVICE_READ, (enum SDIO_FUNC_NUM_T)0, cis_ptr, NULL, 1, &cis_data, 1, BLOCKING_MODE_IRQ_ON, 1);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    HAL_EMMC_TRACE(1, "cis data XXXXX %#X", cis_data);
    hal_sys_timer_delay_us(1);
    ret = sdio_set_func_abort(host, (enum SDIO_FUNC_NUM_T)0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    return ret;
}

int sdio_write_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd)
{
    return MMC_NO_ERR;
}

int sdio_read_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd)
{
    return MMC_NO_ERR;
}
/*
 * get same device information
 */
int sdio_get_block_size(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num, uint16_t *block_size)
{
    int ret = MMC_NO_ERR;

    if (!block_size) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (host_sdio_core.func[func_num]) {
        *block_size = host_sdio_core.func[func_num]->cur_blk_size;
    } else {
        return HAL_SDIO_INVALID_FUNCTION_NUM;
    }

    return ret;
}

int sdio_get_bus_speed(struct mmc_host *host, uint8_t *bus_speed)
{
    int ret = MMC_NO_ERR;
    uint8_t val = 0;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_BUS_SPEED_SEL, 0, &val);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_GET_BUS_SPEED_FAIL;
    }

    if (bus_speed) {
        *bus_speed = (val & 0x0E) >> 1;
    }

    return ret;
}

int sdio_get_capability(struct mmc_host *host, uint8_t *cap)
{
    int ret = MMC_NO_ERR;
    uint8_t val = 0;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_CARD_CAP, 0, &val);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_GET_BUS_SPEED_FAIL;
    }

    if (cap) {
        *cap = val;
    }

    return ret;
}
/*
 *********************************************
 *********************************************
 **Special purpose use, for some sdio device**
 *********************************************
 *********************************************
 */
int sdio_host_gen_int_to_device(struct mmc_host *host)
{
    uint8_t flag = 0;
    int ret = MMC_NO_ERR;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_1, SDIO_FUNC1_INT_TO_DEVICE, 0, &flag);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_ENABLE_FUNC1_INT_TO_DEVICE_FAIL;
    }
    /* set bit 0 */
    flag |= 0x1;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_WRITE, SDIO_FUNC_1, SDIO_FUNC1_INT_TO_DEVICE, flag, NULL);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_ENABLE_FUNC1_INT_TO_DEVICE_FAIL;
    }

    return  ret;
}

int sdio_host_get_func1_rx_buf_cfg(struct mmc_host *host, uint8_t *buf_cnt, uint16_t *buf_len)
{
    int ret = MMC_NO_ERR;
    uint8_t u8temp = 0;
    uint16_t u16temp;

    if ((!buf_cnt) && (!buf_len)) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_INVALID_PARAMETER;
    }

    /* read buf cnt */
    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_1, SDIO_FUNC1_RX_BUF_CFG, 0, &u8temp);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_GET_FUNC1_RX_BUF_CFG_FAIL;
    }

    if (buf_cnt) {
        *buf_cnt = u8temp;
    }

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_1, SDIO_FUNC1_RX_BUF_CFG + 1, 0, &u8temp);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_GET_FUNC1_RX_BUF_CFG_FAIL;
    }

    u16temp = u8temp;

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_1, SDIO_FUNC1_RX_BUF_CFG + 2, 0, &u8temp);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return HAL_SDIO_GET_FUNC1_RX_BUF_CFG_FAIL;
    }

    if (buf_len) {
        u16temp |= (uint16_t)(u8temp << 8);
        *buf_len = u16temp;
    }

    return ret;
}

//from device
int sdio_get_int_pending(struct mmc_host *host, uint8_t *int_pending)
{
    int ret = MMC_NO_ERR;
    if (!int_pending) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    ret = sdio_io_rw_direct(host, SDIO_DEVICE_READ, SDIO_FUNC_0, SDIO_CCCR_INT_PENDING, 0, int_pending);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    return ret;
}

/* read need to received device fun1 int */
void sdio_host_func_int(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num)
{
    uint8_t int_data_lsb = 0;
    uint8_t int_data_hsb = 0;
#define SDIO_FUNC1_INT_VALUE1       0x0
#define SDIO_FUNC1_INT_VALUE2       0x1

    sdio_io_rw_direct(host, SDIO_DEVICE_READ, func_num, SDIO_FUNC1_INT_VALUE1, 0, &int_data_lsb);
    sdio_io_rw_direct(host, SDIO_DEVICE_READ, func_num, SDIO_FUNC1_INT_VALUE2, 0, &int_data_hsb);
}
