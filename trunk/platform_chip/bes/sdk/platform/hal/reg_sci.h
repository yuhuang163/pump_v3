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
#ifndef __REG_SCI_H__
#define __REG_SCI_H__

#include "plat_types.h"

//SCI Registers
struct SCI_T {
    __IO uint32_t SCIMODHW;  //0x00
    __IO uint32_t SCIMODSW;  //0x04
    __IO uint32_t SCICTRL;   //0x08
    __I  uint32_t SCISTAT;   //0x0C
    __IO uint32_t SCIINTIO1; //0x10
    __IO uint32_t SCIINTIO2; //0x14
    __IO uint32_t EDCCTRL;   //0x18
    __IO uint32_t WTCTRL;    //0x1C
    __IO uint32_t SCIBUFHW;  //0x20
    __IO uint32_t SCIBUFSW;  //0x24
    __IO uint32_t ETUDATA;   //0x28
    __IO uint32_t BGTDATA;   //0x2C
    __IO uint32_t CWTDATA;   //0x30
    __IO uint32_t EDCDATA;   //0x34
};

//SCIMODHW
#define DIRC_SEL_INTERNAL (1 << 8)
#define DIRC_SEL (1 << 1)
#define TMODE_SEL (1 << 0)

//SCICTRL
#define TX_FIFO_CLR (1 << 5)
#define RX_FIFO_CLR (1 << 4)

//SCISTAT
#define TX_FIFO_EMPTY (1 << 3)
#define RX_FIFO_EMPTY (1 << 0)

//SCIINTIO
#define TX_INT (1 << 2)
#define RX_INT (1 << 1)

#endif
