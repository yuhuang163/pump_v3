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
#ifndef __TEST_SPI_H__
#define __TEST_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

//////// BES SPI slave operation
#define MST_SPI_WR (0<<30)
#define MST_SPI_RD (1<<30)

#define MST_SPI_DUMMY_DATA (0xDEADBEEF)

#define MST_SPI_ADDR_WR(a) (((a)>>2)|MST_SPI_WR)
#define MST_SPI_ADDR_RD(a) (((a)>>2)|MST_SPI_RD)
#ifndef PLATFORM_BES2710IMP
#define BEST_SHARE_MEMORY_ADDRESS_POINTER     (0x20000000)
#else
#define BEST_SHARE_MEMORY_ADDRESS_POINTER     (0x20040000)
#endif

typedef struct bes_api_share_memory_struct
{
    unsigned int master_tx_address;
    unsigned int master_tx_len;
    unsigned int slave_tx_address;
    unsigned int slave_tx_len;
    unsigned int crc;
} bes_spi_share_memory_struct_t;
unsigned long pal_crc32(unsigned long crc, const unsigned char *buf, unsigned int len);
#ifdef __cplusplus
}
#endif


#endif


