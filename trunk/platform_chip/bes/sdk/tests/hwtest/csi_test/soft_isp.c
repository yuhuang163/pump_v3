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
#ifdef CAMERA_USE_SOFT_ISP

#include "cmsis.h"
#ifdef RTOS
#include "stdlib.h"
#include "cmsis_os.h"
#endif
#include "soft_isp.h"
#include "string.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "camera_drv.h"
#if defined(CHIP_ROLE_DSP)
#include "video_stream_rpc.h"
#endif

//#define CAMERA_SOFT_ISP_DEBUG

#ifdef CAMERA_SOFT_ISP_DEBUG
#define ISP_DBG     TRACE
#else
#define ISP_DBG(...) do{}while(0)
#endif

//AEC & AGC api
// Histogram bin
// raw [0, 1023]
// yuv [0,255]
#define AE_STATS_BIN                (1024)
#define AE_CALCULATION_INTERVAL     (3)

aecCoreHandleType aec_handle = NULL;

struct AEC_REG_T {
    uint16_t addr;
    uint16_t val;
};

static bool aec_opened = false;

static int bus_desc_id = -1;
static uint32_t aec_img_width = 0;
static uint32_t aec_img_height = 0;

typedef int (*isp_i2c_write_handler_t)(int id, uint32_t reg_addr, uint32_t val);

static isp_i2c_write_handler_t isp_i2c_write_handler = NULL;


static void reg_isp_i2c_write_handler(isp_i2c_write_handler_t handler)
{
    isp_i2c_write_handler = handler;
}

void isp_i2c_write(uint16_t addr, uint16_t val)
{
#if defined(CHIP_ROLE_DSP)
    ASSERT(false, "[%s]: not support i2c write in DSP system");
#else
    ASSERT(bus_desc_id >= 0, "[%s]: Invalid bus_desc_id=%d", __func__, bus_desc_id);
    if (isp_i2c_write_handler) {
        isp_i2c_write_handler(bus_desc_id, (uint32_t)addr, (uint32_t)val);
    }
#endif
}

void isp_i2c_write_array(struct AEC_REG_T *regs, uint32_t num)
{
    uint32_t i;

#if defined(CHIP_ROLE_DSP)
    struct RPC_REQ_MSG_T msg;

    ASSERT(num <= RPC_REQ_MSG_PARAM_NUM, "[%s]: Bad num=%d", __func__, num);
    memset((void *)&msg, 0, sizeof(struct RPC_REQ_MSG_T));

    msg.hdr.id = RPC_MSG_HDR_ID;
    msg.cmd = RPC_MSG_VP_CAMERA_I2C_WR;
    for (i = 0; i < num; i++) {
        msg.param[i] = (regs[i].addr << 16) | regs[i].val;
    }
    vplayer_fine_request_msg_send_no_ack(&msg);

#else
    for (i = 0; i < num; i++) {
        isp_i2c_write(regs[i].addr, regs[i].val);
    }
#endif
}

static void aec_update_sensor_param(drv_sensor_exp_param_t *param)
{
    uint32_t ana_gain = param->anaGain;
    uint32_t exp_time = param->expTime;
    uint32_t hdr_exp_time = param->expTime/2;
    uint8_t exp_h, exp_l, ana_gain_val, ana_fine_gain_val;
    // uint8_t hdr_exp_h, hdr_exp_l;

    exp_time <<= 4;
    exp_h = (exp_time >> 8) & 0xff;
    exp_l = exp_time & 0xff;

    hdr_exp_time <<= 4;
    // hdr_exp_h = (hdr_exp_time >> 8) & 0xff;
    // hdr_exp_l = hdr_exp_time & 0xff;

    ana_gain = ana_gain * 0x20 /1000;
    ana_gain_val = (ana_gain >> 8) & 0xff;
    ana_fine_gain_val = ana_gain & 0xff;

    struct AEC_REG_T regs[8];
    uint32_t j = 0;

    regs[j].addr = 0x3e01;
    regs[j].val  = exp_h;
    j++;
    regs[j].addr = 0x3e02;
    regs[j].val  = exp_l;
    j++;
#if 0
    regs[j].addr = 0x3e31;
    regs[j].val  = hdr_exp_h;
    j++;
    regs[j].addr = 0x3e32;
    regs[j].val  = hdr_exp_l;
    j++;
#endif
    regs[j].addr = 0x3e08;
    regs[j].val  = ana_gain_val;
    j++;
    regs[j].addr = 0x3e09;
    regs[j].val  = ana_fine_gain_val;
    j++;

    isp_i2c_write_array(regs, j);
}

static void aec_init_sensor_param(drv_sensor_exp_param_t *param)
{
    struct AEC_REG_T regs[8];
    uint32_t j = 0;

    regs[j].addr = 0x3e01;
    regs[j].val  = 0x0008;
    j++;
    regs[j].addr = 0x3e02;
    regs[j].val  = 0x0040;
    j++;
    regs[j].addr = 0x3e03;
    regs[j].val  = 0x0003;
    j++;
    regs[j].addr = 0x3e08;
    regs[j].val  = 0x0000;
    j++;
    regs[j].addr = 0x3e09;
    regs[j].addr = 0x0020;
    j++;

    // isp_i2c_write(0x3220, 0xc3); // bit[6]: 1 ~ HDR ON  0 ~ HDR OFF
    // isp_i2c_write(0x3e31, 0x04);
    // isp_i2c_write(0x3e32, 0x20);
    isp_i2c_write_array(regs, j);
}

#ifndef RTOS
#define SOFT_ISP_LOCAL_BUFF_SIZE 0x8000
static uint32_t soft_isp_local_buff[SOFT_ISP_LOCAL_BUFF_SIZE/4];
static uint32_t local_buff_size = SOFT_ISP_LOCAL_BUFF_SIZE;
static uint32_t local_buff_used = 0;
#endif

static void soft_isp_buff_init(int init)
{
#ifndef RTOS
    if (init) {
        local_buff_size = SOFT_ISP_LOCAL_BUFF_SIZE;
        local_buff_used = 0;
    } else {
        local_buff_used = 0;
    }
#endif
}

static void* Mem_OS_Malloc(size_t size)
{
    void *pNewMem = NULL;

#ifdef RTOS
    pNewMem = malloc(size);
#else
    if (local_buff_used < local_buff_size) {
        uint32_t free_size;
        free_size = local_buff_size - local_buff_used;
        if (size <= free_size) {
            pNewMem = (void *)((uint8_t *)soft_isp_local_buff + local_buff_used);
            local_buff_used += (uint32_t)size;
        }
    }
#endif
    //void* pNewMem = isp_malloc(size);
    if (pNewMem)
    {
        memset(pNewMem, 0, size);
    }
    if (pNewMem) {
        TRACE(1, "[%s]: WARNING: malloc buffer failed", __func__);
    }
    return pNewMem;
}

static void  Mem_OS_Free(void* pMem)
{
#ifdef RTOS
    free(pMem);
#endif
    //isp_free(pMem);
}


static void AE_Params_Init_Default(ae_ctrl_t *pAECtl)
{
    // 收敛相关参数: 和 AE target 共同决定 曝光表项
    pAECtl->aeConvParam.aeConvergeStep    = 6;
    pAECtl->aeConvParam.aeMaxConvSpeed    = 60;
    pAECtl->aeConvParam.aeStableInRange   = 4;
    pAECtl->aeConvParam.aeStableOutRange  = 64;
    pAECtl->aeConvParam.aeSaturationThr   = 960;
    pAECtl->aeConvParam.aeStatsYIdx       = 3;
    pAECtl->aeConvParam.aeChgFrameCnt     = 60;

    // AE target 相关参数： 计算 final target
    pAECtl->aeTargetParam.aeTarget            = 512;
    pAECtl->aeTargetParam.aeHighContrTarget   = 60;
    pAECtl->aeTargetParam.aeLowLevel          = 15000;
    pAECtl->aeTargetParam.aeHighLevel         = 95000;
    pAECtl->aeTargetParam.aeLowContrast       = 5;
    pAECtl->aeTargetParam.aeHighContrast      = 20;
    pAECtl->aeTargetParam.aeHighCutMin        = 128;
    pAECtl->aeTargetParam.aeLongTarget        = 40;
    pAECtl->aeTargetParam.aeLmRatioMax        = 32;
    pAECtl->aeTargetParam.aeMidTarget         = 40;
    pAECtl->aeTargetParam.aeMsRatioMax        = 32;
    pAECtl->aeTargetParam.aeShortTarget       = 40;
    pAECtl->aeTargetParam.aeSvsRatioMax       = 32;
    pAECtl->aeTargetParam.aeSaturationMaxPercent    = 10240;

    pAECtl->sensorExpParam.maxIntegrationTime = 0x500; //0x568
    pAECtl->sensorExpParam.minIntegrationTime = 0x005;
    pAECtl->sensorExpParam.maxAnalogGain = 28000;
    pAECtl->sensorExpParam.minAnalogGain = 1000;
    pAECtl->sensorExpParam.anaGain = 1000;
    pAECtl->sensorExpParam.expTime = 0x080; //exp lines 0x2b4
}

isp_ret_t AEC_Init(aecCoreHandleType *pHandle)
{
    isp_ret_t res = ISP_RET_SUCCESS;
    uint32_t len;

    if(pHandle == NULL) {
        TRACE(1, "invalid input %p", pHandle);
        res = ISP_RET_INVALID;
        return res;
    }
    len = AE_STATS_BIN*sizeof(uint32_t);
    uint32_t *pHist = (uint32_t*)Mem_OS_Malloc(len);

    if(pHist == NULL) {
        TRACE(1, "Memory allocation failure.");
        res = ISP_RET_NOMEM;
        return res;
    }
    TRACE(1,"pHist addr = %p, len=%d", pHist, len);

    len = sizeof(ae_ctrl_t);
    ae_ctrl_t *pAECtl = (ae_ctrl_t*) Mem_OS_Malloc(len);

    if (pAECtl != NULL) {
        pAECtl->eAEState                      = AE_STATE_WAIT;
        pAECtl->aeStatsInterruptCounter       = 0;

        pAECtl->imgWidth                    = 480;
        pAECtl->imgHeight                   = 640;

        pAECtl->aeCalInterval               = AE_CALCULATION_INTERVAL;
        pAECtl->stableCount                 = 0;

        pAECtl->pAEHist                     = pHist;

        *pHandle = (aecCoreHandleType)pAECtl;

        TRACE(1,"pAECtl addr = %p, len=%d", pAECtl, len);
    }
    else {
        TRACE(1, "Memory allocation failure.");
        Mem_OS_Free(pHist);
        res = ISP_RET_NOMEM;
    }

    return res;
}

isp_ret_t AEC_Deinit(aecCoreHandleType handle)
{
    isp_ret_t res = ISP_RET_SUCCESS;

    if (handle) {
        ae_ctrl_t *pAECtl = (ae_ctrl_t *)handle;
        if(pAECtl->pAEHist) {
            Mem_OS_Free(pAECtl->pAEHist);
            pAECtl->pAEHist = NULL;
        }
        Mem_OS_Free(pAECtl);
        pAECtl = NULL;
        TRACE(1, "Free memory successful!");
    }
    else {
        TRACE(1, "First init Not need free memory!");
        res = ISP_RET_INVALID;
    }

    return res;
}

isp_ret_t AEC_Set_Parameters(aecCoreHandleType handle)
{
    isp_ret_t res = ISP_RET_SUCCESS;

    if(handle) {
        ae_ctrl_t *pAECtl = (ae_ctrl_t *)handle;

        AE_Params_Init_Default(pAECtl);

        reg_isp_i2c_write_handler(camera_device_bus_write);
        aec_init_sensor_param(&pAECtl->sensorExpParam);

        pAECtl->imgWidth = aec_img_width;
        pAECtl->imgHeight = aec_img_height;
    }
    else {
         res = ISP_RET_INVALID;
    }
     return res;
}

static void fast_memset_u32(uint32_t *p, uint32_t v, uint32_t size)
{
#define BSIZE 8
    uint32_t bsize = (size >> 2) / BSIZE;
    uint32_t last = (size >> 2) % BSIZE;

    if (bsize > 0) {
        do {
            p[0] = v;
            p[1] = v;
            p[2] = v;
            p[3] = v;
            p[4] = v;
            p[5] = v;
            p[6] = v;
            p[7] = v;
            p += BSIZE;
            bsize--;
        } while (bsize > 0);
    }
    if (last) {
        do {
            *p++ = v;
            last--;
        } while(last > 0);
    }
}

static int32_t calc_hist_10bit_raw(uint32_t *p_hist, uint32_t img_addr, uint32_t height, uint32_t width)
{
    ASSERT(p_hist, "[%s] p_hist is null", __func__);

    uint16_t *img = (uint16_t*)img_addr;
    uint32_t pixel_num = width * height;

    fast_memset_u32(p_hist, 0, AE_STATS_BIN * sizeof(uint32_t));
    //memset(p_hist, 0, AE_STATS_BIN * sizeof(uint32_t));

    for(uint32_t i=0; i<pixel_num; i++) {
        p_hist[img[i] & (uint16_t)0x3ff] += 1;
    }

    return 0;
}

static int32_t calc_final_target(ae_ctrl_t *pAECtl)
{
#define RATIO   (1000)

    uint32_t low_cut = 0;
    uint32_t high_cut = 0;
    uint32_t contrast = 0; //1000X
    uint32_t pix_total = 0;
    uint32_t pixel_num = pAECtl->imgHeight * pAECtl->imgWidth;
    uint32_t low_cut_total = (uint64_t)pAECtl->aeTargetParam.aeLowLevel * pixel_num / 100000;
    uint32_t high_cut_total = (uint64_t)pAECtl->aeTargetParam.aeHighLevel * pixel_num / 100000;

    pAECtl->ullHistSum = 0;
    calc_hist_10bit_raw(pAECtl->pAEHist, (uint32_t)pAECtl->imgAddr, pAECtl->imgHeight, pAECtl->imgWidth);

    for(uint32_t i=0; i<AE_STATS_BIN; i++) {
        uint32_t n = pAECtl->pAEHist[i];
        pAECtl->ullHistSum += n * i;
        pix_total += n;
        if((0 == low_cut) && (pix_total >= low_cut_total)) {
            low_cut = i;
        }
        if((pix_total >= high_cut_total) && (0 == high_cut)) {
            high_cut = i;
        }
    }
    if (pix_total == 0) {
        return -1;
    }
    low_cut = (low_cut == 0) ? 1 : low_cut;
    high_cut = (high_cut==0) ? (AE_STATS_BIN-1) : high_cut;
    pAECtl->lumaAveLinear = pAECtl->ullHistSum / pix_total;
    pAECtl->lumaAveLinear = (pAECtl->lumaAveLinear == 0) ? 1 : pAECtl->lumaAveLinear;
    contrast = high_cut*RATIO / low_cut;
    /* ISP_DBG(1, "pixel_num: %d %d %d %d %d %d", pixel_num, pix_total, \
                high_cut, low_cut, high_cut_total, low_cut_total);  */
    if(contrast <= pAECtl->aeTargetParam.aeLowContrast*RATIO) {
        pAECtl->finalTarget = pAECtl->aeTargetParam.aeTarget;
        return 0;
    }
    uint32_t high_cut_tmp = (high_cut > pAECtl->aeTargetParam.aeHighCutMin) ? high_cut : pAECtl->aeTargetParam.aeHighCutMin;
    uint32_t hdrTarget = pAECtl->aeTargetParam.aeTarget * pAECtl->aeTargetParam.aeHighContrTarget * (AE_STATS_BIN-1) / 100 / high_cut_tmp;
    uint32_t blending = (contrast - pAECtl->aeTargetParam.aeLowContrast*RATIO) / (pAECtl->aeTargetParam.aeHighContrast
                         - pAECtl->aeTargetParam.aeLowContrast);
    pAECtl->finalTarget = (hdrTarget * blending + pAECtl->aeTargetParam.aeTarget * (RATIO - blending)) / RATIO;

    // ISP_DBG(1, "final_target: %d %d %d %d %d", pAECtl->finalTarget, blending, hdrTarget, high_cut_tmp, contrast);

    return 0;
}

static void AE_Update_Exposure_Parameter(ae_ctrl_t *pAECtl)
{
#define RATIO (1000)

    uint32_t adj_ratio = 0;
    uint32_t need_update = 0;
    int32_t delta_target = pAECtl->finalTarget - pAECtl->lumaAveLinear ;
    uint32_t abs_delta_target = (uint32_t)((delta_target >= 0) ? delta_target : (0 - delta_target));

    // ISP_DBG(1, "luma: %d %d", pAECtl->finalTarget, pAECtl->lumaAveLinear);
    if(abs_delta_target <= pAECtl->aeConvParam.aeStableInRange) {
        pAECtl->aeConvergeDir = 0;
        pAECtl->stableCount = 0;
        return;
    }
    else if(abs_delta_target <= pAECtl->aeConvParam.aeStableOutRange) {
        return;
    }
    pAECtl->aeConvergeDir = (delta_target > 0) ? 1 : 2; // 0-stable 1-under expo 2-over expo

    if(abs_delta_target > (AE_STATS_BIN >> 1)) {
        adj_ratio = abs_delta_target * RATIO / 240;
    }
    else if(abs_delta_target > (AE_STATS_BIN >> 2)) {
        adj_ratio = abs_delta_target * RATIO / 480;
    }
    else {
        adj_ratio = abs_delta_target * RATIO / 960;
    }

    uint32_t exp_time = pAECtl->sensorExpParam.expTime;
    uint32_t ana_gain = pAECtl->sensorExpParam.anaGain;

    if(pAECtl->aeConvergeDir == 1) {
        adj_ratio = abs_delta_target * RATIO / pAECtl->lumaAveLinear;
        if(exp_time >= pAECtl->sensorExpParam.maxIntegrationTime) {
            if(ana_gain < pAECtl->sensorExpParam.maxAnalogGain) {
                ana_gain += ana_gain * adj_ratio / RATIO;
                if(ana_gain > pAECtl->sensorExpParam.maxAnalogGain) {
                    ana_gain = pAECtl->sensorExpParam.maxAnalogGain;
                }
                pAECtl->sensorExpParam.anaGain = ana_gain;
                pAECtl->sensorExpParam.expTime = pAECtl->sensorExpParam.maxIntegrationTime;
                need_update = 1;
            }
        }
        else {
            exp_time += exp_time * adj_ratio / RATIO;
            if(exp_time > pAECtl->sensorExpParam.maxIntegrationTime) {
                exp_time = pAECtl->sensorExpParam.maxIntegrationTime;
            }
            pAECtl->sensorExpParam.expTime = exp_time;
            need_update = 1;
        }
    }
    else if(pAECtl->aeConvergeDir == 2) {
        if(ana_gain <= pAECtl->sensorExpParam.minAnalogGain) {
            if(exp_time > pAECtl->sensorExpParam.minIntegrationTime) {
                uint32_t adj_time = exp_time * adj_ratio / RATIO;
                exp_time = (exp_time > adj_time) ? (exp_time - adj_time) : pAECtl->sensorExpParam.minIntegrationTime;
                if(exp_time <= pAECtl->sensorExpParam.minIntegrationTime) {
                    exp_time = pAECtl->sensorExpParam.minIntegrationTime;
                }
                pAECtl->sensorExpParam.expTime = exp_time;
                pAECtl->sensorExpParam.anaGain = pAECtl->sensorExpParam.minAnalogGain;
                need_update = 1;
            }
        }
        else {
            uint32_t adj_gain = ana_gain * adj_ratio / RATIO;
            ana_gain = (ana_gain > adj_gain) ? (ana_gain - adj_gain) : pAECtl->sensorExpParam.minAnalogGain;
            if(ana_gain <= pAECtl->sensorExpParam.minAnalogGain) {
                ana_gain = pAECtl->sensorExpParam.minAnalogGain;
            }
            pAECtl->sensorExpParam.anaGain = ana_gain;
            need_update = 1;
        }
    }
    if(need_update) {
        need_update = 0;
        aec_update_sensor_param(&pAECtl->sensorExpParam);
    }
    ISP_DBG(1, "adjust:[gain:%d] [time:%d]", pAECtl->sensorExpParam.anaGain, pAECtl->sensorExpParam.expTime);
}

isp_ret_t AEC_Process(aecCoreHandleType handle, uint32_t img_addr)
{
    POSSIBLY_UNUSED uint32_t time;
    isp_ret_t res = ISP_RET_SUCCESS;
    static uint32_t frame_calc = 0;

    if (handle) {
        ae_ctrl_t *pAECtl = (ae_ctrl_t *)handle;

        pAECtl->imgAddr = img_addr;
        pAECtl->frameNum++;
        if((pAECtl->frameNum - frame_calc) > pAECtl->aeCalInterval) {
            time = hal_sys_timer_get();
            calc_final_target(pAECtl);
            AE_Update_Exposure_Parameter(pAECtl);
            frame_calc = pAECtl->frameNum;
            ISP_DBG(1,"[aec_cost:%d ms frm_n:%d.]", TICKS_TO_MS(hal_sys_timer_get()-time), pAECtl->frameNum);
        }
    }
    else {
        TRACE(1, "INVALID HANDLE!");
        res = ISP_RET_INVALID;
    }

    return res;
}

void aec_api_handler(uint8_t type, uint32_t img)
{
    switch (type)
    {
    case AEC_API_TYPE_INIT:
        AEC_Init(&aec_handle);
        AEC_Set_Parameters(aec_handle);
        break;
    case AEC_API_TYPE_PROC:
        AEC_Process(aec_handle, img);
        break;
    case AEC_API_TYPE_RELEASE:
        AEC_Deinit(aec_handle);
        break;
    default:
        break;
    }
}

int soft_isp_aec_open(void)
{
    TRACE(1, "[%s]:", __func__);
    if (!aec_opened) {
        soft_isp_buff_init(1);
        aec_api_handler(AEC_API_TYPE_INIT, 0);
        aec_opened = true;
    }
    return 0;
}

int soft_isp_aec_process_data(uint32_t img, uint32_t img_size)
{
    if (aec_opened) {
        aec_api_handler(AEC_API_TYPE_PROC, img);
    }
    return 0;
}

int soft_isp_aec_close(void)
{
    TRACE(1, "[%s]:", __func__);
    if (aec_opened) {
        aec_api_handler(AEC_API_TYPE_RELEASE, 0);
        aec_opened = false;
        soft_isp_buff_init(0);
    }
    return 0;
}

void soft_isp_setup_device(int desc_id, enum VP_DEVICE_T device, struct VP_DEVICE_CONFIG_T *cfg)
{
    aec_img_width = cfg->width;
    aec_img_height = cfg->height;
    bus_desc_id = desc_id;

    TRACE(1, "[%s]: device=%d, desc_id=%d", __func__, device, bus_desc_id);
    TRACE(1, "[%s]: width=%d, height=%d", __func__, cfg->width, cfg->height);
}

#endif  /*CAMERA_USE_SOFT_ISP*/
