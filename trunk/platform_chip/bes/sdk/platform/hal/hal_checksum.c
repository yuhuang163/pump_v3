/***************************************************************************
 *
 * Copyright 2015-2023 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary cfgrmation of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "plat_addr_map.h"

#if defined(CHECKSUM_BASE)

#include "hal_checksum.h"
#include "reg_checksum.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include <string.h>

#ifdef RTOS
#include "cmsis_os.h"
static osSemaphoreId_t g_checksum_sem;
#endif

#define FOLD_U32T(u)            ((uint32_t)(((u) >> 16) + ((u) & 0x0000ffffUL)))

#define IPV4                    0x04
#define IPV6                    0x06

#define SINGLE_MAX_TCP_LEN      8188

struct IP6_ADDR_T {
  uint32_t addr[4];
};

struct CHS_CTX_T {
    void *data;
    uint8_t type_tcp;
    uint16_t hdr_len;
    uint32_t hdr_src;
    uint32_t hdr_dst;
    uint8_t hdr_proto;
    bool last_flag;
    uint16_t total_len;
    uint16_t trans_len;
    uint16_t unprocess_len;
    uint8_t unprocess_data[4];
};

static struct CHS_CTX_T chs_ctx;

static MUTEX_FLAG_T checksum_active;

static HAL_CHS_RUN_CB_T run_cb;

static struct CHK_T * const checksum = (struct CHK_T *)CHECKSUM_BASE;

static uint16_t bswap_16(uint16_t x)
{
    return x << 8 | x >> 8;
}

static uint32_t bswap_32(uint32_t x)
{
    return x >> 24 | (x >> 8 & 0xff00) | (x << 8 & 0xff0000) | x << 24;
}

static uint16_t chk_htons(uint16_t n)
{
    union { int i; char c; } u = { 1 };
    return u.c ? bswap_16(n) : n;
}

static uint32_t chk_htonl(uint32_t n)
{
    union { int i; char c; } u = { 1 };
    return u.c ? bswap_32(n) : n;
}

static void ip6_addr_adapt(struct IP6_ADDR_T *p_src, struct IP6_ADDR_T *p_dest, uint32_t *src, uint32_t *dst)
{
    uint32_t addr_src =0;
    uint32_t addr_dst = 0;
    uint32_t tmp_src = 0;
    uint32_t tmp_dst = 0;
    for (uint8_t addr_part = 0; addr_part < 4; addr_part++) {
        addr_src = p_src->addr[addr_part];
        tmp_src = (uint32_t)(tmp_src + (addr_src & 0xffffUL));
        tmp_src = (uint32_t)(tmp_src + ((addr_src >> 16) & 0xffffUL));
        addr_dst = p_dest->addr[addr_part];
        tmp_dst = (uint32_t)(tmp_dst + (addr_dst & 0xffffUL));
        tmp_dst = (uint32_t)(tmp_dst + ((addr_dst >> 16) & 0xffffUL));
    }

    tmp_src = FOLD_U32T(tmp_src);
    tmp_src = FOLD_U32T(tmp_src);
    tmp_dst = FOLD_U32T(tmp_dst);
    tmp_dst = FOLD_U32T(tmp_dst);
    *src = tmp_src;
    *dst = tmp_dst;
}

static void tcp_psd_hdr_adapt(struct HAL_CHS_CFG_T *cfg, uint8_t *addr)
{
    uint8_t version;
    uint32_t ip6_src;
    uint32_t ip6_dst;

    version = ((*(uint8_t *)addr) >> 4) & 0xff;
    if (version == IPV4) {
        cfg->len = chk_htons(*((uint16_t *)&addr[2])) - 20;
        cfg->data = &addr[20];
        cfg->src = chk_htonl(*((uint32_t *)&addr[12]));
        cfg->dst = chk_htonl(*((uint32_t *)&addr[16]));
    } else if (version == IPV6) {
        ip6_addr_adapt((struct IP6_ADDR_T*)(&addr[8]), (struct IP6_ADDR_T*)(&addr[24]), &ip6_src, &ip6_dst);
        cfg->len = chk_htons(*((uint16_t *)&addr[4]));;
        cfg->data = &addr[40];
        cfg->src = chk_htons(ip6_src);
        cfg->dst = chk_htons(ip6_dst);
    }
    cfg->proto = 0x06;
}

static void hal_checksum_sem_init(void)
{
#ifdef RTOS
    if (g_checksum_sem == NULL)
        g_checksum_sem = osSemaphoreNew(1, 0, NULL);
#endif
}

static void hal_checksum_sem_release(void)
{
#ifdef RTOS
    if (g_checksum_sem)
        osSemaphoreRelease(g_checksum_sem);
#endif
}

static int hal_checksum_get_result(uint16_t *res)
{
    uint32_t val;
    uint32_t checksum_mode = 0;
    uint32_t state;

    state = checksum->DMA_STATUS;
    if (state & CHK_DMA_TRANS_ERR) {
        return -1;
    }

    checksum_mode = checksum->CHK_CFG;
    val = checksum->CHK_RES;
    if (checksum_mode & CHK_CHK_MODE)
        val = (val >> 16) & 0xffff;
    else
        val = val & 0xffff;

    *res = val;
    return 0;
}

static void hal_checksum_intr_clr(void)
{
    checksum->CHK_CFG |= CHK_INTR_CLEAR;
}

static void hal_checksum_reset(void)
{
    checksum->DMA_REQ &= ~CHK_DMA_EN;
    checksum->DMA_CFG &= ~CHK_DMA_RST;
    checksum->DMA_CFG |= CHK_DMA_RST;
    checksum->CHK_CFG |= CHK_CHK_CLEAR;
}

static void hal_checksum_config_tcp_param(void)
{
    uint32_t val;

    val = checksum->CHK_CFG;
    val |= CHK_CHK_MODE | CHK_INTR_MASK;
    val &= ~CHK_CHK_BIGD;
    val |= CHK_CHK_HEADER_BIGD;
    checksum->CHK_CFG = val;
}

static void hal_checksum_config_tcp_data(uint8_t *data, uint16_t len, uint32_t src, uint32_t dest, uint8_t proto)
{
    uint16_t trans_len = chs_ctx.trans_len;
    checksum->CHK_HEADER_LEN_PRO = CHK_HEADER_PROTOCOL(proto) | CHK_HEADER_LENGTH(len);
    checksum->CHK_HEADER_SRC = src;
    checksum->CHK_HEADER_DST = dest;
    checksum->DMA_ADDR = ADDR_CPU_TO_DEV((uint32_t)data);
    checksum->DMA_LEN_BST = CHK_DMA_LEN(trans_len);
}

static void hal_checksum_config_tcp(void *data, uint16_t len, uint32_t src, uint32_t dest, uint8_t proto)
{
    hal_checksum_config_tcp_param();
    hal_checksum_config_tcp_data(data, len, src, dest, proto);
}

static void hal_checksum_config_ip_param(void)
{
    uint32_t val;

    val = checksum->CHK_CFG;
    val |= CHK_INTR_MASK;
    val &= ~CHK_CHK_MODE;
    val &= ~CHK_CHK_BIGD;
    checksum->CHK_CFG = val;
}

static void hal_checksum_config_ip_data(uint32_t *data, uint16_t len)
{
    checksum->DMA_ADDR = ADDR_CPU_TO_DEV((uint32_t)data);
    checksum->DMA_LEN_BST = CHK_DMA_LEN((uint32_t)len);
}

static void hal_checksum_config_ip(uint32_t *data, uint16_t len)
{
    hal_checksum_config_ip_param();
    hal_checksum_config_ip_data(data, len);
}

static void hal_checksum_begin(void)
{
    checksum->DMA_REQ |= CHK_DMA_EN;
}

bool hal_checksum_busy(void)
{
    if ((checksum->DMA_STATUS & CHK_CHK_REQING) || (checksum->DMA_STATUS & CHK_CHK_WORKING))
        return true;
    return false;
}

static void hal_checksum_wait(void)
{
#ifdef RTOS
    if (g_checksum_sem)
        osSemaphoreAcquire(g_checksum_sem, osWaitForever);
#else
    while (hal_checksum_busy());
#endif
}

void hal_checksum_close(void)
{
    NVIC_DisableIRQ(CHKSUM_IRQn);
    hal_cmu_checksum_clock_disable();
}

static void tcp_checksum_soft(uint8_t *addr, uint8_t count, uint16_t *res)
{
    uint32_t sum = 0;

    while (count > 1) {
        sum += *(uint16_t*)addr;
        addr += 2;
        count -= 2;
    }

    if (count > 0) {
        uint16_t left_over;
        left_over = *addr;
        sum += left_over;
    }

    sum = ((uint16_t)(~sum) + *res);
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    *res = (sum == 0xffff) ? 0 : sum;
}

static void tcp_input_len_process(void)
{
    if (chs_ctx.total_len <= SINGLE_MAX_TCP_LEN) {
        chs_ctx.last_flag = true;
        chs_ctx.trans_len = chs_ctx.total_len;
    } else {
        chs_ctx.trans_len = SINGLE_MAX_TCP_LEN;
    }
}

static void tcp_inter_result_process(uint16_t *res)
{
    *res = chk_htons(~(*res));
    chs_ctx.hdr_len = 0;
    chs_ctx.hdr_dst = 0;
    chs_ctx.hdr_proto = 0;
    chs_ctx.hdr_src = *res;
    chs_ctx.data += chs_ctx.trans_len;
    chs_ctx.total_len -= SINGLE_MAX_TCP_LEN;
}

static void hal_checksum_update(uint16_t *res, HAL_CHS_RUN_CB_T cb)
{
    hal_checksum_config_tcp(chs_ctx.data, chs_ctx.hdr_len, chs_ctx.hdr_src, chs_ctx.hdr_dst, chs_ctx.hdr_proto);
    hal_checksum_begin();
    if (cb)
        return;
    hal_checksum_wait();
    hal_checksum_get_result(res);
}

static void hal_checksum_finish(uint16_t *res)
{
    tcp_input_len_process();
    hal_checksum_reset();
    hal_checksum_config_tcp(chs_ctx.data, chs_ctx.hdr_len, chs_ctx.hdr_src, chs_ctx.hdr_dst, chs_ctx.hdr_proto);
    hal_checksum_begin();
}

static void hal_checksum_get_info(const struct HAL_CHS_CFG_T *cfg)
{
    chs_ctx.data = cfg->data;
    chs_ctx.type_tcp = cfg->type_tcp;
    chs_ctx.hdr_len = cfg->len;
    chs_ctx.hdr_src = cfg->src;
    chs_ctx.hdr_dst = cfg->dst;
    chs_ctx.hdr_proto = cfg->proto;
    chs_ctx.total_len = cfg->len;
}

static void hal_checksum_irq_handler(void)
{
    uint32_t state;
    uint16_t res;
    uint8_t finish_flag = 0;

    state = checksum->DMA_STATUS;
    hal_checksum_intr_clr();

    if (run_cb) {
        if (chs_ctx.type_tcp) {
            if (chs_ctx.last_flag) {
                if (hal_checksum_get_result(&res) == 0) {
                    if (chs_ctx.unprocess_len){
                        tcp_checksum_soft(chs_ctx.unprocess_data, chs_ctx.unprocess_len, &res);
                    }
                    run_cb(res, 0);
                } else {
                    run_cb(state, 1);
                }
                finish_flag = 1;
            } else {
                if (hal_checksum_get_result(&res) == 0) {
                    tcp_inter_result_process(&res);
                    if (chs_ctx.total_len > SINGLE_MAX_TCP_LEN) {
                        tcp_input_len_process();
                        hal_checksum_reset();
                        hal_checksum_update(&res, run_cb);
                    } else {
                        hal_checksum_finish(&res);
                    }
                } else {
                    run_cb(state, 1);
                }
            }
        } else {
            if (hal_checksum_get_result(&res) == 0) {
                run_cb(res, 0);
            } else {
                run_cb(state, 1);
            }
            finish_flag = 1;
        }
        if (finish_flag)
            mutex_flag_unlock(&checksum_active);
    } else {
        hal_checksum_sem_release();
    }
}

static void hal_checksum_irq_enable(void)
{
    NVIC_SetVector(CHKSUM_IRQn, (uint32_t)hal_checksum_irq_handler);
    NVIC_SetPriority(CHKSUM_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(CHKSUM_IRQn);
    NVIC_EnableIRQ(CHKSUM_IRQn);
}

void hal_checksum_open(void)
{
    hal_checksum_sem_init();
    hal_cmu_checksum_clock_enable();
    hal_checksum_irq_enable();
}

int hal_checksum_start(const struct HAL_CHS_CFG_T *cfg, uint16_t *res, HAL_CHS_RUN_CB_T cb)
{
    if (mutex_flag_lock(&checksum_active))
        return -1;

    memset(&chs_ctx, 0, sizeof(chs_ctx));
    hal_checksum_get_info(cfg);
    chs_ctx.unprocess_len = chs_ctx.total_len % 4;
    if (chs_ctx.unprocess_len) {
        memcpy(chs_ctx.unprocess_data, chs_ctx.data + chs_ctx.total_len - chs_ctx.unprocess_len, chs_ctx.unprocess_len);
    }
    chs_ctx.total_len -= chs_ctx.unprocess_len;

    run_cb = cb;
    if (chs_ctx.type_tcp) {
        if (cb == NULL) {
            while(chs_ctx.total_len > SINGLE_MAX_TCP_LEN) {
                tcp_input_len_process();
                hal_checksum_reset();
                hal_checksum_update(res, cb);
                tcp_inter_result_process(res);
            }
            hal_checksum_finish(res);
        } else {
            if (chs_ctx.total_len > SINGLE_MAX_TCP_LEN) {
                tcp_input_len_process();
                hal_checksum_reset();
                hal_checksum_update(res, cb);
            } else {
                hal_checksum_finish(res);
            }
        }
    } else {
        hal_checksum_reset();
        hal_checksum_config_ip(cfg->data, cfg->len);
        hal_checksum_begin();
    }

    if (run_cb) {
        return 0;
    }

    hal_checksum_wait();
    ASSERT(hal_checksum_get_result(res) == 0, "checksum state error,error state = %#x\n", checksum->DMA_STATUS);
    if (chs_ctx.unprocess_len) {
        tcp_checksum_soft(chs_ctx.unprocess_data, chs_ctx.unprocess_len, res);
    }
    mutex_flag_unlock(&checksum_active);
    return 0;
}

int hal_checksum_tcp_start(uint8_t *addr, uint16_t *res, HAL_CHS_RUN_CB_T cb)
{
    struct HAL_CHS_CFG_T cfg;

    memset(&cfg, 0 ,sizeof(cfg));
    tcp_psd_hdr_adapt(&cfg, addr);
    cfg.type_tcp = 1;

    return hal_checksum_start(&cfg, res, cb);
}

int hal_checksum_ip_start(uint8_t *addr, uint16_t *res, HAL_CHS_RUN_CB_T cb)
{
    struct HAL_CHS_CFG_T cfg;
    cfg.type_tcp = 0;
    cfg.data = addr;
    cfg.len = 20;
    return hal_checksum_start(&cfg, res, cb);
}
#endif
