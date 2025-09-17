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
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __REG_I3C_HCI_H__
#define __REG_I3C_HCI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

/******************************************************************************/
/* Chip related base address */
#define DAT_TABLE_OFFSET            0x200
#define DCT_TABLE_OFFSET            0x300
#define PIO_SECTION_OFFSET          0x500
#define RINGHDRS_SECTION_OFFSET     0x600
#define EXTCAPS_SECTION_OFFSET      0x700
#define RINGHDRS_RH_OFFSET          0x800
#define RINGHDRS_RH0_OFFSET         (RINGHDRS_RH_OFFSET + 0x000)
#define RINGHDRS_RH1_OFFSET         (RINGHDRS_RH_OFFSET + 0x080)
#define RINGHDRS_RH2_OFFSET         (RINGHDRS_RH_OFFSET + 0x100)
#define RINGHDRS_RH3_OFFSET         (RINGHDRS_RH_OFFSET + 0x180)
#define RINGHDRS_RH4_OFFSET         (RINGHDRS_RH_OFFSET + 0x200)
#define RINGHDRS_RH5_OFFSET         (RINGHDRS_RH_OFFSET + 0x280)
#define RINGHDRS_RH6_OFFSET         (RINGHDRS_RH_OFFSET + 0x300)
#define RINGHDRS_RH7_OFFSET         (RINGHDRS_RH_OFFSET + 0x380)

/* In order to improve efficiency, the values in the registers were directly
    used instead of being read
*/
#define I3C_HCI_REG_OFFSET_READ_SKIP_EN
/******************************************************************************/

/* I3C HCI Direct Register set definition start */
#define I3C_HCI_VERSION             0x00
#define I3C_HCI_HC_CTRL             0x04
#define I3C_HCI_MASTER_DEVICE_ADDR  0x08
#define I3C_HCI_HC_CAPS             0x0C

#define I3C_HCI_RESET_CTRL          0x10
#define I3C_HCI_PRE_STATE           0x14
#define I3C_HCI_INTR_STATUS         0x20
#define I3C_HCI_INTR_EN             0x24
#define I3C_HCI_INT_SIG_EN          0x28
#define I3C_HCI_INTR_FORCE          0x2C

#define I3C_HCI_DAT_SEC_OFFSET      0x30
#define I3C_HCI_DCT_SEC_OFFSET      0x34
#define I3C_HCI_RH_SEC_OFFSET       0x38
#define I3C_HCI_PIO_SEC_OFFSET      0x3C

#define I3C_HCI_EXT_CAPS_OFFSET     0x40
#define I3C_HCI_INT_CTRL_CMDS_EN    0x4C

#define IBI_NOTIFY_CTRL             0x58

#define DEV_CTX_BASE_LO             0x60
#define DEV_CTX_BASE_HI             0x64
#define DEV_CTX_SG                  0x68
/* I3C HCI Direct Register set definition end */

/* I3C HCI InDirect Register set definition */
//PIO Registers - Start
#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define PIO_BASE PIO_SECTION_OFFSET
#else
#define PIO_BASE (i3c_reg_read32(id, I3C_HCI_PIO_SEC_OFFSET, &u32glValue) ? 0 : u32glValue)
#endif
#define I3C_HCI_PIO_CMD_QUE_PORT        PIO_BASE + 0x00
#define I3C_HCI_PIO_RSP_QUE_PORT        PIO_BASE + 0x04
#define I3C_HCI_PIO_DATA_PORT           PIO_BASE + 0x08
#define I3C_HCI_PIO_IBI_PORT            PIO_BASE + 0x0C
#define I3C_HCI_PIO_QUE_THLD_CTRL       PIO_BASE + 0x10
#define I3C_HCI_PIO_DAT_THLD_CTRL       PIO_BASE + 0x14
#define I3C_HCI_PIO_QUE_SIZE            PIO_BASE + 0x18
#define I3C_HCI_PIO_INTR_STATUS         PIO_BASE + 0x20
#define I3C_HCI_PIO_INTR_EN             PIO_BASE + 0x24
#define I3C_HCI_PIO_INT_SIG_EN          PIO_BASE + 0x28
#define I3C_HCI_PIO_INTR_FORCE          PIO_BASE + 0x2C
//PIO Registers - End

//DMA Registers - Start
#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define RH_BASE RINGHDRS_SECTION_OFFSET
#else
#define RH_BASE (i3c_reg_read16(id, I3C_HCI_RH_SEC_OFFSET, &u16glValue) ? 0 : u16glValue)
#endif
#define I3C_HCI_RHS_CTRL        RH_BASE + 0x00
#define I3C_HCI_RH0_OFFSET      RH_BASE + 0x04
#define I3C_HCI_RH1_OFFSET      RH_BASE + 0x08
#define I3C_HCI_RH2_OFFSET      RH_BASE + 0x0C
#define I3C_HCI_RH3_OFFSET      RH_BASE + 0x10
#define I3C_HCI_RH4_OFFSET      RH_BASE + 0x14
#define I3C_HCI_RH5_OFFSET      RH_BASE + 0x18
#define I3C_HCI_RH6_OFFSET      RH_BASE + 0x1C
#define I3C_HCI_RH7_OFFSET      RH_BASE + 0x20

#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define RH0_BASE RINGHDRS_RH0_OFFSET
#else
#define RH0_BASE (i3c_reg_read32(id, I3C_HCI_RH0_OFFSET, &u32glValue) ? 0 : u32glValue)
#endif
#define RH0_CR_SETUP            RH0_BASE + 0x00
#define RH0_IBI_SETUP           RH0_BASE + 0x04
#define RH0_CHUNK_CTRL          RH0_BASE + 0x08

#define RH0_RING_INTR_ST        RH0_BASE + 0x10
#define RH0_RING_INTR_ST_EN     RH0_BASE + 0x14
#define RH0_RING_INTR_SIG_EN    RH0_BASE + 0x18
#define RH0_RING_INTR_FORCE     RH0_BASE + 0x1C

#define RH0_RING_STATUS         RH0_BASE + 0x20
#define RH0_RING_CTRL           RH0_BASE + 0x24
#define RH0_RING_OPR1           RH0_BASE + 0x28
#define RH0_RING_OPR2           RH0_BASE + 0x2C

#define RH0_CMD_BASE_L          RH0_BASE + 0x30
#define RH0_CMD_BASE_H          RH0_BASE + 0x34
#define RH0_RSP_BASE_L          RH0_BASE + 0x38
#define RH0_RSP_BASE_H          RH0_BASE + 0x3C

#define RH0_IBI_ST_BASE_L       RH0_BASE + 0x40
#define RH0_IBI_ST_BASE_H       RH0_BASE + 0x44
#define RH0_IBI_DAT_BASE_L      RH0_BASE + 0x48
#define RH0_IBI_DAT_BASE_H      RH0_BASE + 0x4C

#define RH0_CMD_RING_SG         RH0_BASE + 0x50
#define RH0_RESP_RING_SG        RH0_BASE + 0x54
#define RH0_IBI_STATUS_RING_SG  RH0_BASE + 0x58
#define RH0_IBI_DATA_RING_SG    RH0_BASE + 0x5C

#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define RH1_BASE RINGHDRS_RH1_OFFSET
#else
#define RH1_BASE (i3c_reg_read32(id, I3C_HCI_RH1_OFFSET, &u32glValue) ? 0 : u32glValue)
#endif
#define RH1_CR_SETUP            RH1_BASE + 0x00
#define RH1_IBI_SETUP           RH1_BASE + 0x04
#define RH1_CHUNK_CTRL          RH1_BASE + 0x08

#define RH1_RING_INTR_ST        RH1_BASE + 0x10
#define RH1_RING_INTR_ST_EN     RH1_BASE + 0x14
#define RH1_RING_INTR_SIG_EN    RH1_BASE + 0x18
#define RH1_RING_INTR_FORCE     RH1_BASE + 0x1C

#define RH1_RING_STATUS         RH1_BASE + 0x20
#define RH1_RING_CTRL           RH1_BASE + 0x24
#define RH1_RING_OPR1           RH1_BASE + 0x28
#define RH1_RING_OPR2           RH1_BASE + 0x2C

#define RH1_CMD_BASE_L          RH1_BASE + 0x30
#define RH1_CMD_BASE_H          RH1_BASE + 0x34
#define RH1_RSP_BASE_L          RH1_BASE + 0x38
#define RH1_RSP_BASE_H          RH1_BASE + 0x3C

#define RH1_IBI_ST_BASE_L       RH1_BASE + 0x40
#define RH1_IBI_ST_BASE_H       RH1_BASE + 0x44
#define RH1_IBI_DAT_BASE_L      RH1_BASE + 0x48
#define RH1_IBI_DAT_BASE_H      RH1_BASE + 0x4C

#define RH1_CMD_RING_SG         RH1_BASE + 0x50
#define RH1_RESP_RING_SG        RH1_BASE + 0x54
#define RH1_IBI_STATUS_RING_SG  RH1_BASE + 0x58
#define RH1_IBI_DATA_RING_SG    RH1_BASE + 0x5C

#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define RH2_BASE RINGHDRS_RH2_OFFSET
#else
#define RH2_BASE (i3c_reg_read32(id, I3C_HCI_RH2_OFFSET, &u32glValue) ? 0 : u32glValue)
#endif
#define RH2_CR_SETUP            RH2_BASE + 0x00
#define RH2_IBI_SETUP           RH2_BASE + 0x04
#define RH2_CHUNK_CTRL          RH2_BASE + 0x08

#define RH2_RING_INTR_ST        RH2_BASE + 0x10
#define RH2_RING_INTR_ST_EN     RH2_BASE + 0x14
#define RH2_RING_INTR_SIG_EN    RH2_BASE + 0x18
#define RH2_RING_INTR_FORCE     RH2_BASE + 0x1C

#define RH2_RING_STATUS         RH2_BASE + 0x20
#define RH2_RING_CTRL           RH2_BASE + 0x24
#define RH2_RING_OPR1           RH2_BASE + 0x28
#define RH2_RING_OPR2           RH2_BASE + 0x2C

#define RH2_CMD_BASE_L          RH2_BASE + 0x30
#define RH2_CMD_BASE_H          RH2_BASE + 0x34
#define RH2_RSP_BASE_L          RH2_BASE + 0x38
#define RH2_RSP_BASE_H          RH2_BASE + 0x3C

#define RH2_IBI_ST_BASE_L       RH2_BASE + 0x40
#define RH2_IBI_ST_BASE_H       RH2_BASE + 0x44
#define RH2_IBI_DAT_BASE_L      RH2_BASE + 0x48
#define RH2_IBI_DAT_BASE_H      RH2_BASE + 0x4C

#define RH2_CMD_RING_SG         RH2_BASE + 0x50
#define RH2_RESP_RING_SG        RH2_BASE + 0x54
#define RH2_IBI_STATUS_RING_SG  RH2_BASE + 0x58
#define RH2_IBI_DATA_RING_SG    RH2_BASE + 0x5C

#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define RH3_BASE RINGHDRS_RH3_OFFSET
#else
#define RH3_BASE (i3c_reg_read32(id, I3C_HCI_RH3_OFFSET, &u32glValue) ? 0 : u32glValue)
#endif
#define RH3_CR_SETUP            RH3_BASE + 0x00
#define RH3_IBI_SETUP           RH3_BASE + 0x04
#define RH3_CHUNK_CTRL          RH3_BASE + 0x08

#define RH3_RING_INTR_ST        RH3_BASE + 0x10
#define RH3_RING_INTR_ST_EN     RH3_BASE + 0x14
#define RH3_RING_INTR_SIG_EN    RH3_BASE + 0x18
#define RH3_RING_INTR_FORCE     RH3_BASE + 0x1C

#define RH3_RING_STATUS         RH3_BASE + 0x20
#define RH3_RING_CTRL           RH3_BASE + 0x24
#define RH3_RING_OPR1           RH3_BASE + 0x28
#define RH3_RING_OPR2           RH3_BASE + 0x2C

#define RH3_CMD_BASE_L          RH3_BASE + 0x30
#define RH3_CMD_BASE_H          RH3_BASE + 0x34
#define RH3_RSP_BASE_L          RH3_BASE + 0x38
#define RH3_RSP_BASE_H          RH3_BASE + 0x3C

#define RH3_IBI_ST_BASE_L       RH3_BASE + 0x40
#define RH3_IBI_ST_BASE_H       RH3_BASE + 0x44
#define RH3_IBI_DAT_BASE_L      RH3_BASE + 0x48
#define RH3_IBI_DAT_BASE_H      RH3_BASE + 0x4C

#define RH3_CMD_RING_SG         RH3_BASE + 0x50
#define RH3_RESP_RING_SG        RH3_BASE + 0x54
#define RH3_IBI_STATUS_RING_SG  RH3_BASE + 0x58
#define RH3_IBI_DATA_RING_SG    RH3_BASE + 0x5C

#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define RH4_BASE RINGHDRS_RH4_OFFSET
#else
#define RH4_BASE (i3c_reg_read32(id, I3C_HCI_RH4_OFFSET, &u32glValue) ? 0 : u32glValue)
#endif
#define RH4_CR_SETUP            RH4_BASE + 0x00
#define RH4_IBI_SETUP           RH4_BASE + 0x04
#define RH4_CHUNK_CTRL          RH4_BASE + 0x08

#define RH4_RING_INTR_ST        RH4_BASE + 0x10
#define RH4_RING_INTR_ST_EN     RH4_BASE + 0x14
#define RH4_RING_INTR_SIG_EN    RH4_BASE + 0x18
#define RH4_RING_INTR_FORCE     RH4_BASE + 0x1C

#define RH4_RING_STATUS         RH4_BASE + 0x20
#define RH4_RING_CTRL           RH4_BASE + 0x24
#define RH4_RING_OPR1           RH4_BASE + 0x28
#define RH4_RING_OPR2           RH4_BASE + 0x2C

#define RH4_CMD_BASE_L          RH4_BASE + 0x30
#define RH4_CMD_BASE_H          RH4_BASE + 0x34
#define RH4_RSP_BASE_L          RH4_BASE + 0x38
#define RH4_RSP_BASE_H          RH4_BASE + 0x3C

#define RH4_IBI_ST_BASE_L       RH4_BASE + 0x40
#define RH4_IBI_ST_BASE_H       RH4_BASE + 0x44
#define RH4_IBI_DAT_BASE_L      RH4_BASE + 0x48
#define RH4_IBI_DAT_BASE_H      RH4_BASE + 0x4C

#define RH4_CMD_RING_SG         RH4_BASE + 0x50
#define RH4_RESP_RING_SG        RH4_BASE + 0x54
#define RH4_IBI_STATUS_RING_SG  RH4_BASE + 0x58
#define RH4_IBI_DATA_RING_SG    RH4_BASE + 0x5C

#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define RH5_BASE RINGHDRS_RH5_OFFSET
#else
#define RH5_BASE (i3c_reg_read32(id, I3C_HCI_RH5_OFFSET, &u32glValue) ? 0 : u32glValue)
#endif
#define RH5_CR_SETUP            RH5_BASE + 0x00
#define RH5_IBI_SETUP           RH5_BASE + 0x04
#define RH5_CHUNK_CTRL          RH5_BASE + 0x08

#define RH5_RING_INTR_ST        RH5_BASE + 0x10
#define RH5_RING_INTR_ST_EN     RH5_BASE + 0x14
#define RH5_RING_INTR_SIG_EN    RH5_BASE + 0x18
#define RH5_RING_INTR_FORCE     RH5_BASE + 0x1C

#define RH5_RING_STATUS         RH5_BASE + 0x20
#define RH5_RING_CTRL           RH5_BASE + 0x24
#define RH5_RING_OPR1           RH5_BASE + 0x28
#define RH5_RING_OPR2           RH5_BASE + 0x2C

#define RH5_CMD_BASE_L          RH5_BASE + 0x30
#define RH5_CMD_BASE_H          RH5_BASE + 0x34
#define RH5_RSP_BASE_L          RH5_BASE + 0x38
#define RH5_RSP_BASE_H          RH5_BASE + 0x3C

#define RH5_IBI_ST_BASE_L       RH5_BASE + 0x40
#define RH5_IBI_ST_BASE_H       RH5_BASE + 0x44
#define RH5_IBI_DAT_BASE_L      RH5_BASE + 0x48
#define RH5_IBI_DAT_BASE_H      RH5_BASE + 0x4C

#define RH5_CMD_RING_SG         RH5_BASE + 0x50
#define RH5_RESP_RING_SG        RH5_BASE + 0x54
#define RH5_IBI_STATUS_RING_SG  RH5_BASE + 0x58
#define RH5_IBI_DATA_RING_SG    RH5_BASE + 0x5C

#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define RH6_BASE RINGHDRS_RH6_OFFSET
#else
#define RH6_BASE (i3c_reg_read32(id, I3C_HCI_RH6_OFFSET, &u32glValue) ? 0 : u32glValue)
#endif
#define RH6_CR_SETUP            RH6_BASE + 0x00
#define RH6_IBI_SETUP           RH6_BASE + 0x04
#define RH6_CHUNK_CTRL          RH6_BASE + 0x08

#define RH6_RING_INTR_ST        RH6_BASE + 0x10
#define RH6_RING_INTR_ST_EN     RH6_BASE + 0x14
#define RH6_RING_INTR_SIG_EN    RH6_BASE + 0x18
#define RH6_RING_INTR_FORCE     RH6_BASE + 0x1C

#define RH6_RING_STATUS         RH6_BASE + 0x20
#define RH6_RING_CTRL           RH6_BASE + 0x24
#define RH6_RING_OPR1           RH6_BASE + 0x28
#define RH6_RING_OPR2           RH6_BASE + 0x2C

#define RH6_CMD_BASE_L          RH6_BASE + 0x30
#define RH6_CMD_BASE_H          RH6_BASE + 0x34
#define RH6_RSP_BASE_L          RH6_BASE + 0x38
#define RH6_RSP_BASE_H          RH6_BASE + 0x3C

#define RH6_IBI_ST_BASE_L       RH6_BASE + 0x40
#define RH6_IBI_ST_BASE_H       RH6_BASE + 0x44
#define RH6_IBI_DAT_BASE_L      RH6_BASE + 0x48
#define RH6_IBI_DAT_BASE_H      RH6_BASE + 0x4C

#define RH6_CMD_RING_SG         RH6_BASE + 0x50
#define RH6_RESP_RING_SG        RH6_BASE + 0x54
#define RH6_IBI_STATUS_RING_SG  RH6_BASE + 0x58
#define RH6_IBI_DATA_RING_SG    RH6_BASE + 0x5C

#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define RH7_BASE RINGHDRS_RH7_OFFSET
#else
#define RH7_BASE (i3c_reg_read32(id, I3C_HCI_RH7_OFFSET, &u32glValue) ? 0 : u32glValue)
#endif
#define RH7_CR_SETUP            RH7_BASE + 0x00
#define RH7_IBI_SETUP           RH7_BASE + 0x04
#define RH7_CHUNK_CTRL          RH7_BASE + 0x08

#define RH7_RING_INTR_ST        RH7_BASE + 0x10
#define RH7_RING_INTR_ST_EN     RH7_BASE + 0x14
#define RH7_RING_INTR_SIG_EN    RH7_BASE + 0x18
#define RH7_RING_INTR_FORCE     RH7_BASE + 0x1C

#define RH7_RING_STATUS         RH7_BASE + 0x20
#define RH7_RING_CTRL           RH7_BASE + 0x24
#define RH7_RING_OPR1           RH7_BASE + 0x28
#define RH7_RING_OPR2           RH7_BASE + 0x2C

#define RH7_CMD_BASE_L          RH7_BASE + 0x30
#define RH7_CMD_BASE_H          RH7_BASE + 0x34
#define RH7_RSP_BASE_L          RH7_BASE + 0x38
#define RH7_RSP_BASE_H          RH7_BASE + 0x3C

#define RH7_IBI_ST_BASE_L       RH7_BASE + 0x40
#define RH7_IBI_ST_BASE_H       RH7_BASE + 0x44
#define RH7_IBI_DAT_BASE_L      RH7_BASE + 0x48
#define RH7_IBI_DAT_BASE_H      RH7_BASE + 0x4C

#define RH7_CMD_RING_SG         RH7_BASE + 0x50
#define RH7_RESP_RING_SG        RH7_BASE + 0x54
#define RH7_IBI_STATUS_RING_SG  RH7_BASE + 0x58
#define RH7_IBI_DATA_RING_SG    RH7_BASE + 0x5C
//DMA Registers - End

//DAT/DCT offset
#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define I3C_HCI_DAT_OFFSET DAT_TABLE_OFFSET
#define I3C_HCI_DCT_OFFSET DCT_TABLE_OFFSET
#else
#define I3C_HCI_DAT_OFFSET (i3c_reg_read16(id, I3C_HCI_DAT_SEC_OFFSET, &u16glValue) ? 0 : (u16glValue & DAT_OFFSET_MASK))
#define I3C_HCI_DCT_OFFSET (i3c_reg_read16(id, I3C_HCI_DCT_SEC_OFFSET, &u16glValue) ? 0 : (u16glValue & DCT_OFFSET_MASK))
#endif

//Extended Capability Header (EXTCAP_HEADER)
#ifdef I3C_HCI_REG_OFFSET_READ_SKIP_EN
#define EXT_CAPS_HEADER_BASE EXTCAPS_SECTION_OFFSET
#else
#define EXT_CAPS_HEADER_BASE (i3c_reg_read16(id, I3C_HCI_EXT_CAPS_OFFSET, &u16glValue) ? 0 : u16glValue)
#endif
#define EXT_CAPS_HEADER_OFFSET EXT_CAPS_HEADER_BASE + 0x00
#define EXT_CAP_ID  (i3c_reg_read8(id, EXT_CAPS_HEADER_OFFSET, &u8glValue) ? 0 : u8glValue)

//CAP ID fields
#define CAP_ID_HW_IDENTITY       0x01
#define CAP_ID_MASTER_EXT_CONFIG 0x02
#define CAP_ID_BUS_TIMING        0x0B
#define CAP_ID_DEBUG_INFO        0x0C
/* I3C HCI InDirect Register set definition end */

#ifdef __cplusplus
}
#endif

#endif

