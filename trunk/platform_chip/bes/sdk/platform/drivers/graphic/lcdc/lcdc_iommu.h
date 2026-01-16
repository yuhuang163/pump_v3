/***************************************************************************
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
#ifndef LCDC_IOMMU_H
#define LCDC_IOMMU_H

#include "plat_addr_map.h"

#ifdef LCDC_IOMMU
#include "bes_iommu.h"

#define IOMMU_INIT()  do { lcdc_iommu_init(); tfbc_iommu_init(); } while (0);
#define IOMMU_DEINIT()  do { lcdc_iommu_deinit(); tfbc_iommu_deinit(); } while (0);
#define FB_MMAP(buffer, size, tbu_type, mmu_on) \
    lcdc_fb_mmap((buffer), (size), (tbu_type), (mmu_on))
#define FB_UNMAP(iova, tbu_type, mmu_on) \
    lcdc_fb_unmmap((iova), (tbu_type), (mmu_on))

#define TFBC_MMAP(buffer, size, tbu_type, mmu_on) \
    tfbc_fb_mmap((buffer), (size), (tbu_type), (mmu_on))
#define TFBC_UNMAP(iova, mmu_on) \
    tfbc_fb_unmmap((iova), (mmu_on))

void lcdc_iommu_init(void);

void lcdc_iommu_deinit(void);

uint32_t *lcdc_fb_mmap(uint32_t *buffer, uint32_t size, uint16_t tbu_type, bool mmu_on);

void lcdc_fb_unmmap(uint32_t *buffer, uint16_t tbu_type, bool mmu_on);

void tfbc_iommu_init(void);

void tfbc_iommu_deinit(void);

uint32_t *tfbc_fb_mmap(uint32_t *buffer, uint32_t size, uint16_t tbu_type, bool mmu_on);

void tfbc_fb_unmmap(uint32_t *buffer, bool mmu_on);

void dump_iommu_status(void);
#else

#define IOMMU_INIT()
#define IOMMU_DEINIT()
#define FB_MMAP(buffer, size, tbu_type, mmu_on) (buffer)
#define FB_UNMAP(iova, tbu_type, mmu_on) ((void)0)
#define TFBC_MMAP(buffer, size, tbu_type, mmu_on) (buffer)
#define TFBC_UNMAP(iova, mmu_on) ((void)0)
#endif

#endif
