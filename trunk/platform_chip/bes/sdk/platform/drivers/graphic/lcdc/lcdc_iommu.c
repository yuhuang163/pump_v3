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

#include <plat_types.h>
#include "lcdc_iommu.h"

#ifdef LCDC_IOMMU
static iommu_handle_t lcdc_iommu_handle;
static iommu_handle_t tfbc_iommu_handle;
static uint8_t lcdc_handler_ref;
static uint8_t tfbc_handler_ref;

void lcdc_iommu_init(void)
{
    if (lcdc_iommu_handle == NULL)
        lcdc_iommu_handle = bes_iommu_init(IOMMU_DISP_BASE);
    lcdc_handler_ref++;
}

void lcdc_iommu_deinit(void)
{
    lcdc_handler_ref--;
    if (lcdc_handler_ref == 0) {
        bes_iommu_deinit(lcdc_iommu_handle);
        lcdc_iommu_handle = NULL;
    }
}

uint32_t *lcdc_fb_mmap(uint32_t *buffer, uint32_t size, uint16_t tbu_type, bool mmu_on)
{
    if (mmu_on) {
        return iommu_addr_map(lcdc_iommu_handle, tbu_type, buffer, size);
    }
    else
        return buffer;
}

void lcdc_fb_unmmap(uint32_t *buffer, uint16_t tbu_type, bool mmu_on)
{
    if (mmu_on) {
        iommu_addr_unmap(lcdc_iommu_handle, tbu_type, (uint32_t)buffer);
    }
}

void tfbc_iommu_init(void) {
    if (tfbc_iommu_handle == NULL)
        tfbc_iommu_handle = bes_iommu_init(IOMMU_TFBC_BASE);
    tfbc_handler_ref++;
}

void tfbc_iommu_deinit(void)
{
    tfbc_handler_ref--;
    if (tfbc_handler_ref == 0) {
        bes_iommu_deinit(tfbc_iommu_handle);
        tfbc_iommu_handle = NULL;
    }
}

uint32_t *tfbc_fb_mmap(uint32_t *buffer, uint32_t size, uint16_t tbu_type, bool mmu_on)
{
    if (mmu_on) {
        return iommu_addr_map(tfbc_iommu_handle, tbu_type, buffer, size);
    }
    else
        return buffer;
}

void tfbc_fb_unmmap(uint32_t *buffer, bool mmu_on)
{
    if (mmu_on) {
        uint16_t tbu_type = TBU_TYPE_SEQ|TBU_TYPE_READ;
        iommu_addr_unmap(tfbc_iommu_handle, tbu_type, (uint32_t)buffer);
    }
}

void dump_iommu_status(void)
{

}

#endif
