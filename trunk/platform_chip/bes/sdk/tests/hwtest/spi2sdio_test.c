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
#ifdef SPI2SDIO_TEST
/*
*****************************************************************************
WARNING: This example is tested that spi master <-> sdio-spi mode slave.
*****************************************************************************
*/
#include "string.h"
#include "hal_spi.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "cmsis.h"

//#define SPI_ENABLE_DMA
#define     SPI_ID_SELECT           (1)  //0:spi0,1:spilcd,2021-06-28
#define     SPI2SDIO_DEBUG

#ifdef SPI2SDIO_DEBUG
#ifdef FPGA
    #define HAL_SPI2SDIO_TRACE(n, s, ...)    TRACE(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
#else
    #define HAL_SPI2SDIO_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#endif
#else
    #define HAL_SPI2SDIO_TRACE(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

/*refer to 7.3.3 chapter in Part1_Physical_Layer_Simplified_Specification_Ver7.10.pdf*/
#define     TOKEN_DATA_RSP          (0xE5)
#define     TOKEN_START             (0xFE)
#define     TOKEN_START_MUL         (0xFC)
#define     TOKEN_STOP_MUL          (0xFD)
#define     SDIO_CMD_TRANSFLAG      (0x40)
#define     CMD_MR1_ERR             (0x5C)
#define     SPI_CMD_DUMMY           (0xFF)
#define     BUFF_LEN                (2048+32*2)
enum HAL_SDIO_HOST_ID_T {
    HAL_SDIO_HOST_ID_0 = 0,
    HAL_SDIO_HOST_ID_NUM,
};
enum HAL_SDIO_FUNC_NUM_T {
    HAL_SDIO_FUNC_0 = 0,
    HAL_SDIO_FUNC_1,
    HAL_SDIO_FUNC_NUM,
};
enum HAL_SDIO_DEVICE_RW {
    HAL_SDIO_DEVICE_READ = 0,
    HAL_SDIO_DEVICE_WRITE,
};
enum HAL_SDIO_ERR {
    HAL_SDIO_ERR_NONE = 0,
    HAL_SDIO_CMD5_FAIL,
    HAL_SDIO_CMD52_FAIL,
    HAL_SDIO_CMD53_FAIL,
    HAL_SDIO_INVALID_PARAMETER,
    HAL_SDIO_FAIL,
};
/* sdio cmd list */
#define SDIO_CMD_GO_IDLE_STATE          0   /* RSP:R1 */
#define SDIO_CMD_IO_SEND_OP_COND        5   /* RSP:R4 */
#define SDIO_CMD_IO_RW_DIRECT           52  /* RSP:R5 */
#define SDIO_CMD_IO_RW_EXTENDED         53  /* RSP:R5 */
/* CCCR regiter */
#define SDIO_CCCR_IO_ENABLE             0x2
#define SDIO_CCCR_INT_ENABLE            0x4
#define SDIO_CCCR_BLK_SIZE              0x10
#define SDIO_DEFAULT_BLOCK_SIZE         512//Can not exceed 1024 when multiple blocks
struct SDIO_FUNC_T {
    uint8_t func_num;       /* func id */
    uint8_t func_status;    /* func enable status 0:disable 1:enable  */
    uint8_t func_int_status;/* func int status 0:disable 1:enable */
    uint16_t cur_blk_size;  /* func block size */
    uint16_t max_blk_size;  /* func max block size */
};
struct SDIO_CORE_T {
    uint8_t func_total_num;
    uint8_t version;
    uint16_t real_read_len;
    uint16_t real_write_len;
    struct SDIO_FUNC_T *func[HAL_SDIO_FUNC_NUM];
};
POSSIBLY_UNUSED uint8_t data_buff[BUFF_LEN] = {SPI_CMD_DUMMY};
POSSIBLY_UNUSED uint8_t recvformat[BUFF_LEN]= {SPI_CMD_DUMMY};
static struct HAL_SPI_CFG_T spi_cfg;
static struct SDIO_FUNC_T host_sdio_func[HAL_SDIO_FUNC_NUM];
static struct SDIO_CORE_T host_sdio_core;
static struct SDIO_CORE_T *phost_sdio_core = &host_sdio_core;
void spi_set_speed(struct HAL_SPI_CFG_T* local_spi_cfg, uint32_t val)
{
     //maybe spi_cfg.rate
     local_spi_cfg->rate = val * 1000;
}
/******************************************************************************
 * Name:    CRC-7/MMC           x7+x3+1
 * Poly:    0x09
 * Init:    0x00
 * Refin:   False
 * Refout:  False
 * Xorout:  0x00
 * Use:     MultiMediaCard,SD,ect.
 *****************************************************************************/
uint8_t crc7_mmc(uint8_t *data, uint32_t length)
{
    uint8_t i;
    uint8_t crc = 0;        // Initial value
    while(length--)
    {
        crc ^= *data++;        // crc ^= *data; data++;
        for ( i = 0; i < 8; i++ )
        {
            if ( crc & 0x80 )
                crc = (crc << 1) ^ 0x12;        // 0x12 = 0x09<<(8-7)
            else
                crc <<= 1;
        }
    }
    return crc >> 1;
}
int32_t spi2sdio_write(uint8_t *pbuf, uint32_t len, uint8_t enable_dma)
{
    int32_t ret;
#if (SPI_ID_SELECT == 0)
    if (enable_dma) {
        ret = hal_spi_dma_send(pbuf, len, NULL);
    } else {
        ret = hal_spi_send(pbuf, len);
    }
#elif (SPI_ID_SELECT == 1)
    if (enable_dma) {
        ret = hal_spilcd_dma_send(pbuf, len, NULL);
    } else {
        ret = hal_spilcd_send(pbuf, len);
    }
#endif
    return ret;
}
/*
params: format, 
can be register address of SPI
if not, fill it with 0xFF
*/
int32_t spi2sdio_read(uint8_t *format, uint8_t *pbuf, uint32_t len, uint8_t enable_dma)
{
    int32_t ret;
#if (SPI_ID_SELECT == 0)
    if (enable_dma) {
        ret = hal_spi_dma_recv(format, pbuf, len, NULL);
    } else {
        ret = hal_spi_recv(format, pbuf, len);
    }
#elif (SPI_ID_SELECT == 1)
    if (enable_dma) {
        ret = hal_spilcd_dma_recv(format, pbuf, len, NULL);
    } else {
        ret = hal_spilcd_recv(format, pbuf, len);
    }
#endif
    return ret;
}
int32_t hal_spi2sdio_write(uint8_t *pbuf, uint32_t len)
{
    int32_t ret;
    ret = spi2sdio_write(pbuf, len, spi_cfg.dma_tx);
    return ret;
}
int32_t hal_spi2sdio_read(uint8_t *pbuf, uint32_t len)
{
    int32_t ret;
    ret = spi2sdio_read(recvformat, pbuf, len, spi_cfg.dma_rx);
    return ret;
}
uint32_t hal_spi2sdio_sendcmd(uint8_t cmdId, uint32_t arg)
{
    int32_t ret;
    uint8_t crc;
    uint8_t cmdbuffer[8];
    HAL_SPI2SDIO_TRACE(0, "%s, cmd =0x%x, arg=0x%x", __func__, cmdId, arg);
    memset(cmdbuffer, 0xFF, sizeof(cmdbuffer));
    cmdId |= SDIO_CMD_TRANSFLAG;
    memcpy(cmdbuffer + 1, (uint8_t *)&cmdId, sizeof(cmdId));
    memcpy(cmdbuffer + 2, (uint8_t *)&arg+3, 1);
    memcpy(cmdbuffer + 3, (uint8_t *)&arg+2, 1);
    memcpy(cmdbuffer + 4, (uint8_t *)&arg+1, 1);
    memcpy(cmdbuffer + 5, (uint8_t *)&arg+0, 1);
    crc = (uint8_t)((crc7_mmc(cmdbuffer+1, 5) << 1) | 0x1);
    memcpy(cmdbuffer + 6, (uint8_t *)&crc, sizeof(crc));
    do
    {
        ret = hal_spi2sdio_write(cmdbuffer, sizeof(cmdbuffer));
    }
    while(ret != 0);
#ifdef SPI2SDIO_DEBUG
    DUMP8("%02x ",cmdbuffer, sizeof(cmdbuffer));
#endif
    return ret;
}
uint32_t hal_spi2sdio_recvrsp(uint8_t* expect_resp, uint32_t resp_len)
{
    int32_t ret=0;
    do
    {
        ret = hal_spi2sdio_read(expect_resp, resp_len);
#ifdef SPI2SDIO_DEBUG
        DUMP8("%02x ",expect_resp, resp_len);
#endif
    }
    while(ret != 0);
    return ret;
}
int32_t hal_spi2sdio_data_read(uint8_t *pbuf, uint32_t data_size, uint16_t blk_count, uint16_t blk_size)
{
    uint8_t loc=0, index=0, *p=NULL, *pstart=NULL;
    uint32_t len_read_all=0;
    //len_read_all include start token + data + crc-16 +dummy, so 4 is Align len of token + crc-16
    len_read_all = (blk_count ? (blk_count * (blk_size+4)): (((data_size + 3) & (~3))+4));
    hal_spi2sdio_read(pbuf, len_read_all);
    for(loc=0; loc<len_read_all; loc++)
    {
        if(pbuf[loc] == TOKEN_START){
            HAL_SPI2SDIO_TRACE(0, "%s, index=%d", __func__, loc);
            hal_spi2sdio_read(pbuf+len_read_all, loc);
            len_read_all+=loc;
            break;
        }
    }
    p = pbuf+loc;//point to location of real data
    pstart = pbuf;
    //get the real data from all data recved
    if(!blk_count)
    {
        memcpy(pbuf, p+1, data_size);
    }
    else
    {
        for(index=0; index<blk_count; index++)
        {
            memcpy(pbuf, p+4*index+1, blk_size);
            pbuf+=blk_size;
            p+=blk_size;
        }
        if(data_size % blk_size)
            memcpy(pbuf, p+4*index+1, data_size % blk_size);
    }
    memset(pstart+data_size, 0xFF, len_read_all-data_size);
#ifdef SPI2SDIO_DEBUG
    DUMP8("%02x ",pstart, data_size);
#endif
    return 0;
}
int32_t hal_spi2sdio_data_write(uint8_t *pbuf, uint32_t len_4align, uint16_t blk_cnt, uint16_t blk_size)
{
    uint8_t data_format = 0xFF, rsp=0;
    uint16_t crc16=0xFFFF;//crc-16 of data, device ignore it, so fill it with 0xFFFF
    HAL_SPI2SDIO_TRACE(0, "%s", __func__);
    if(!blk_cnt)
    {/*send content:start token+ data + crc-16, then read the device rsp*/
        data_format = TOKEN_START;//start token
        hal_spi2sdio_write((uint8_t*)&data_format, sizeof(data_format));
        hal_spi2sdio_write(pbuf, len_4align);
        hal_spi2sdio_write((uint8_t*)&crc16, sizeof(crc16));
        //recv device rsp when a block send, device rsp should be 0xE5
        hal_spi2sdio_recvrsp(&rsp, sizeof(rsp));
    }
    else
    {/*send content, and then send stop token*/
        for(uint16_t i=0; i<blk_cnt; i++)
        {
            data_format = TOKEN_START_MUL;//multiple block start token
            hal_spi2sdio_write((uint8_t*)&data_format, sizeof(data_format));
            hal_spi2sdio_write(pbuf+i*blk_size, blk_size);
            hal_spi2sdio_write((uint8_t*)&crc16, sizeof(crc16));
            //recv rsp when a block send, rsp should be 0xE5
            hal_spi2sdio_recvrsp(&rsp, sizeof(rsp));
        }
        data_format = TOKEN_STOP_MUL;//multiple block stop token
        hal_spi2sdio_write((uint8_t*)&data_format, sizeof(data_format));
    }
    return 0;
}
/*Modified R1 description
|start bit(always 0)| parameter error| RFU(always 0)| Function Number Error|
|       bit7        |       bit6     |       bit5   |       bit4           |
|COM CRC error      | illegal command| RFU(always 0)|    idle state        |
|       bit3        |       bit2     |       bit1   |       bit0           |
*/
//sdio device change to spi mode, success if rsp is 0x01
int hal_spi2sdio_changeToSPIMode()
{
    uint8_t rsp;
    do{
        hal_spi2sdio_sendcmd(SDIO_CMD_GO_IDLE_STATE, 0);
        hal_spi2sdio_recvrsp(&rsp, sizeof(rsp));
    }while(rsp != 0x01);
    HAL_SPI2SDIO_TRACE(1, "%s, success!!!", __func__);
    return 0;
}
/*R4 description
|Modified Response-1|    other       |       I/O OCR        |
|       8bits       |    8bits       |       24bits         |
*/
int hal_spi2sdio_get_OCR()
{
    uint8_t rsp[5];
    hal_spi2sdio_sendcmd(SDIO_CMD_IO_SEND_OP_COND, 0);
    hal_spi2sdio_recvrsp(rsp, sizeof(rsp));
    if(rsp[0] &(CMD_MR1_ERR))
    {
        return -HAL_SDIO_CMD5_FAIL;
    }
    HAL_SPI2SDIO_TRACE(1, "%s, rsp.status=0x%02x, OCR=0x%08x!!!", __func__, rsp[0], *(uint32_t*)(&rsp[1]));
    return 0;
}
/*R5 description
|Modified Response-1| r/w data(cmd53, always 0) |
|       8bits       |       8bits               |
*/
int hal_spi2sdio_io_rw_direct(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_DEVICE_RW read0_write1,
                              enum HAL_SDIO_FUNC_NUM_T func_num, uint32_t addr, uint8_t data, uint8_t *resp)
{
    uint32_t cmdarg=0;
    uint8_t rsp[2];
    HAL_SPI2SDIO_TRACE(0, "%s,rw=%d, func %d, addr=0x%x, data=0x%x",
                      __func__, read0_write1, func_num, addr, data);
    cmdarg  = read0_write1 ? 0x80000000 : 0x00000000;   //R/W flag
    cmdarg |= ((uint32_t)(func_num & 0x7)) << 28;       //funcion number
    cmdarg |= (read0_write1 && (uint32_t)resp) ? 0x08000000 : 0x00000000; //RAW flag
    cmdarg |= addr << 9;    //register address
    cmdarg |= data;
    hal_spi2sdio_sendcmd(SDIO_CMD_IO_RW_DIRECT, cmdarg);
    hal_spi2sdio_recvrsp(rsp, sizeof(rsp));
    if(rsp[0] &(CMD_MR1_ERR))
    {
        return -HAL_SDIO_CMD52_FAIL;
    }
    if(!read0_write1)
    {
        *resp=rsp[1];
        HAL_SPI2SDIO_TRACE(0, "%s, rsp=0x%x,0x%x", __func__, rsp[1], *resp);
    }
    return 0;
}
/*R5 description
|Modified Response-1| r/w data(cmd53, always 0) |
|       8bits       |       8bits               |
*/
int hal_spi2sdio_io_rw_extended(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_DEVICE_RW read0_write1, enum HAL_SDIO_FUNC_NUM_T func_num,
                                uint32_t addr, uint8_t *resp, uint8_t incr_addr, uint8_t* local_data_buff, uint32_t data_size)
{
    POSSIBLY_UNUSED uint32_t cmdarg=0, real_len=0;
    uint16_t blk_count=0, blk_size=0;
    uint8_t rsp[2];
    HAL_SPI2SDIO_TRACE(0, "%s,rw=%d, func %d, addr=0x%x, data_size=%d",
                        __func__, read0_write1, func_num, addr, data_size);
    cmdarg  = read0_write1 ? 0x80000000 : 0x00000000;   //R/W flag
    cmdarg |= ((uint32_t)(func_num & 0x7)) << 28;       //funcion number
    cmdarg |= incr_addr ? (1 << 26) : 0; //op code
    cmdarg |= addr << 9;    //register address
    real_len = data_size;
    blk_size = phost_sdio_core->func[func_num]->cur_blk_size;
    if (data_size < blk_size) {
        blk_count = 0;
        data_size = (data_size + 3) & (~3);//Round up to a multiple of 4 bytes
    } else {
        if (data_size % blk_size) {
            blk_count = data_size / blk_size + 1;  //byte block count
        } else {
            blk_count = data_size / blk_size;      //byte block count
        }
        if (blk_count > 511) {
            return -1;
        }
    }
    if (blk_count) {
        //block mode
        cmdarg |= (1 << 27);
        cmdarg |= blk_count;
    } else {
        //bytes mode
        cmdarg |= (data_size == 512) ? 0 : data_size;
    }
    hal_spi2sdio_sendcmd(SDIO_CMD_IO_RW_EXTENDED, cmdarg);
    hal_spi2sdio_recvrsp(rsp, sizeof(rsp));
    if(rsp[0] &(CMD_MR1_ERR))
    {
        return -HAL_SDIO_CMD53_FAIL;
    }
    if(!read0_write1)
    {
        *resp=rsp[1];
        HAL_SPI2SDIO_TRACE(0, "%s, rsp=0x%x,0x%x", __func__, rsp[1], *resp);
    }
    if(!read0_write1)
    {
        hal_spi2sdio_data_read(local_data_buff, real_len, blk_count, blk_size);
    }
    else
    {
        hal_spi2sdio_data_write(local_data_buff, data_size, blk_count, blk_size);
    }
    return 0;
}
enum HAL_SDIO_ERR hal_spi2sdio_enable_func(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t enable=0;
    HAL_SPI2SDIO_TRACE(0, "%s", __func__);
    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }
#ifdef NEED_READ
    if (!hal_spi2sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, 0, &enable)) {
        enable |= (1 << func_num);
    } else {
        return HAL_SDIO_FAIL;
    }
#endif
    for(int i=(HAL_SDIO_FUNC_0+1); i<= func_num; i++)
    {
        enable |= (1 << func_num);
    }
    if (hal_spi2sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, enable, NULL)) {
        return HAL_SDIO_FAIL;
    }
    return  HAL_SDIO_ERR_NONE;
}
enum HAL_SDIO_ERR hal_spi2sdio_enable_func_int(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t enable=0;
    HAL_SPI2SDIO_TRACE(0, "%s", __func__);
    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }
#ifdef NEED_READ
    if (!hal_spi2sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable)) {
        enable |= (1 << func_num);
    } else {
        return HAL_SDIO_FAIL;
    }
#endif
    for(int i=HAL_SDIO_FUNC_0; i<= func_num; i++)
    {
        enable |= (1 << func_num);
    }
    if (hal_spi2sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL)) {
        return HAL_SDIO_FAIL;
    }
    return  HAL_SDIO_ERR_NONE;
}
enum HAL_SDIO_ERR hal_spi2sdio_set_block_size(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num, uint16_t block_size)
{
    HAL_SPI2SDIO_TRACE(0, "%s", __func__);
    #define SDIO_FBR_BASE(x)            ((x) * 0x100)
    /* set block size */
    hal_spi2sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_BLK_SIZE, block_size & 0xff, NULL);
    hal_spi2sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_BLK_SIZE + 1, (block_size >> 8) & 0xff, NULL);
    if (phost_sdio_core->func[func_num]) {
        phost_sdio_core->func[func_num]->cur_blk_size = block_size;
        HAL_SPI2SDIO_TRACE(0, "%s, funcNum=%d, blk_size=0x%x, 0x%x", __func__, func_num, block_size, phost_sdio_core->func[func_num]->cur_blk_size);
    } else {
        return HAL_SDIO_FAIL;
    }
    return  HAL_SDIO_ERR_NONE;
}
enum HAL_SDIO_ERR hal_spi2sdio_get_block_size(enum HAL_SDIO_FUNC_NUM_T func_num, uint16_t *block_size)
{
    HAL_SPI2SDIO_TRACE(0, "%s", __func__);
    if (!block_size) {
        return HAL_SDIO_INVALID_PARAMETER;
    }
    if (phost_sdio_core->func[func_num]) {
        *block_size = phost_sdio_core->func[func_num]->cur_blk_size;
    } else {
        return HAL_SDIO_FAIL;
    }
    return  HAL_SDIO_ERR_NONE;
}
enum HAL_SDIO_ERR hal_spi2sdio_host_gen_int_to_device(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t flag=0;
    HAL_SPI2SDIO_TRACE(0, "%s", __func__);
    #define SDIO_FUNC1_INT_TO_DEVICE    0x120
#ifdef NEED_READ
    if (!hal_spi2sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_INT_TO_DEVICE, 0, &flag)) {
        flag |= 0x1;//set bit0
    } else {
        return HAL_SDIO_FAIL;
    }
#endif
    flag |= 0x1;
    if (hal_spi2sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_1, SDIO_FUNC1_INT_TO_DEVICE, flag, NULL)) {
        return HAL_SDIO_FAIL;
    }
    return  HAL_SDIO_ERR_NONE;
}
void hal_spi2sdio_host_func_int(enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t int_data_lsb = 0;
    uint8_t int_data_hsb = 0;
    HAL_SPI2SDIO_TRACE(1, "%s, func%d int", __func__, func_num);
    #define SDIO_FUNC1_INT_VALUE1       0x0
    #define SDIO_FUNC1_INT_VALUE2       0x1
    hal_spi2sdio_io_rw_direct(HAL_SDIO_HOST_ID_0, HAL_SDIO_DEVICE_READ, func_num, SDIO_FUNC1_INT_VALUE1, 0, &int_data_lsb);
    hal_spi2sdio_io_rw_direct(HAL_SDIO_HOST_ID_0, HAL_SDIO_DEVICE_READ, func_num, SDIO_FUNC1_INT_VALUE2, 0, &int_data_hsb);
    HAL_SPI2SDIO_TRACE(1, "%s, hsb=0x%x,lsb=0x%x", __func__, int_data_hsb, int_data_lsb);
    phost_sdio_core->real_read_len = ((uint16_t)(int_data_hsb << 8)) + int_data_lsb;
    HAL_SPI2SDIO_TRACE(1, "read real len=%d", phost_sdio_core->real_read_len);
}
int32_t hal_spi2sdio_host_open(enum HAL_SDIO_HOST_ID_T id)
{
    uint32_t i=0;
    uint8_t cmdbuffer[8];
    HAL_SPI2SDIO_TRACE(0, "%s", __func__);
    memset(&host_sdio_func, 0, sizeof(struct SDIO_FUNC_T)*HAL_SDIO_FUNC_NUM);
    memset(&host_sdio_core, 0, sizeof(struct SDIO_CORE_T));
    phost_sdio_core->func_total_num = HAL_SDIO_FUNC_NUM;
    for (uint8_t index = 0; index <= phost_sdio_core->func_total_num; index++) {
        phost_sdio_core->func[index] = &host_sdio_func[index];
        host_sdio_func[index].func_num = index;
    }
    memset(cmdbuffer, SPI_CMD_DUMMY, sizeof(cmdbuffer));
    memset(recvformat, SPI_CMD_DUMMY, sizeof(recvformat));
    memset(data_buff, SPI_CMD_DUMMY, sizeof(data_buff));
    //wait sdio/sd card powerup
    for(i=0; i<0xf00; i++);
    //send 0xff to wait sdio/sd card inited done
    hal_spi2sdio_write((uint8_t *)&cmdbuffer, sizeof(cmdbuffer));
    //change to spi mode
    hal_spi2sdio_changeToSPIMode();
    //get ocr
    hal_spi2sdio_get_OCR();
    //func IO enable
    hal_spi2sdio_enable_func(id, HAL_SDIO_FUNC_1);
    //func IO INT enable
    hal_spi2sdio_enable_func_int(id, HAL_SDIO_FUNC_1);
    //set block size: cmd53 multiple block mode may error if not setting block size
    for(i=HAL_SDIO_FUNC_0; i<=HAL_SDIO_FUNC_1; i++)
    {
        hal_spi2sdio_set_block_size(id, i, SDIO_DEFAULT_BLOCK_SIZE);
    }
    return 0;
}
void spi2sdio_test(void)
{
    uint32_t error = 0;
    uint8_t rsp = 0;
    HAL_SPI2SDIO_TRACE(0, "   ");
#if defined(CHIP_BEST1501) || defined(CHIP_BEST1600)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_390M);
#endif
    HAL_SPI2SDIO_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    HAL_SPI2SDIO_TRACE(0, "------SPI master test started------");
    HAL_SPI2SDIO_TRACE(1, "CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    memset(&spi_cfg, 0, sizeof(spi_cfg));
    spi_cfg.clk_delay_half = true;
    spi_cfg.clk_polarity = true;
    spi_cfg.slave = false;
#ifdef SPI_ENABLE_DMA
    spi_cfg.dma_rx = true;
    spi_cfg.dma_tx = true;
#else
    spi_cfg.dma_rx = false;
    spi_cfg.dma_tx = false;
#endif
    spi_cfg.rx_sep_line = false;
    spi_cfg.cs = 0;
    //1.crystal clock source: maximum frequency 48M (OSCx2=48M/2=24, finally get 24M)
    //2.pll clock source     : maximum frequency 52M (PLL/2=52,finally get 52M)
    spi_cfg.rate = 12 * 1000 * 1000;
    spi_cfg.rx_bits = 8;
    spi_cfg.tx_bits = 8;
    spi_cfg.rx_frame_bits = 0;
    //the clock of sd card initialed, don't more than 400KHZ
    spi_set_speed(&spi_cfg, 400);
    HAL_SPI2SDIO_TRACE(0, "  ");
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.clk_delay_half:%d", spi_cfg.clk_delay_half);
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.clk_polarity  :%d", spi_cfg.clk_polarity);
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.slave         :%d", spi_cfg.slave);
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.dma_rx        :%d", spi_cfg.dma_rx);
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.dma_tx        :%d", spi_cfg.dma_tx);
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.rx_sep_line   :%d", spi_cfg.rx_sep_line);
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.cs            :%d", spi_cfg.cs);
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.rate          :%d", spi_cfg.rate);
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.rx_bits       :%d", spi_cfg.rx_bits);
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.tx_bits       :%d", spi_cfg.tx_bits);
    HAL_SPI2SDIO_TRACE(1, "spi_cfg.rx_frame_bits :%d", spi_cfg.rx_frame_bits);
    HAL_SPI2SDIO_TRACE(0, "  ");
#if (SPI_ID_SELECT == 0)
    hal_iomux_set_spi();
    error = hal_spi_open(&spi_cfg);
    ASSERT(!error, "%s:%d,spi open error:%d", __func__, __LINE__, error);
#elif (SPI_ID_SELECT == 1)
    hal_iomux_set_spilcd();
    error = hal_spilcd_open(&spi_cfg);
    ASSERT(!error, "%s:%d,spilcd open error:%d", __func__, __LINE__, error);
#endif
    hal_sys_timer_delay(MS_TO_TICKS(3));
    hal_spi2sdio_host_open(HAL_SDIO_HOST_ID_0);
    //recv data to data_buff
    hal_spi2sdio_host_gen_int_to_device(HAL_SDIO_HOST_ID_0);
    hal_spi2sdio_host_func_int(HAL_SDIO_FUNC_1);
    hal_spi2sdio_io_rw_extended(HAL_SDIO_HOST_ID_0, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, phost_sdio_core->real_read_len,
                                    &rsp, 1, (uint8_t *)data_buff, phost_sdio_core->real_read_len);
    //send data from data_buff
    phost_sdio_core->real_write_len = 21;
    for(uint16_t i=0; i<phost_sdio_core->real_write_len; i++)
    {
        data_buff[i] = (uint8_t)(i);
    }
    hal_spi2sdio_io_rw_extended(HAL_SDIO_HOST_ID_0, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_1, phost_sdio_core->real_write_len,
                                    &rsp, 1, (uint8_t *)data_buff, phost_sdio_core->real_write_len);
    HAL_SPI2SDIO_TRACE(0, " idle");
}
#endif
