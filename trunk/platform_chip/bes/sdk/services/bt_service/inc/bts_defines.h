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
#ifndef __BTS_DEFINES_H__
#define __BTS_DEFINES_H__

typedef struct {
    unsigned char addr[6];
} __attribute__ ((packed)) bts_dev_addr_t;

typedef unsigned char bts_accessible_mode_t;
#define BTS_ACCESS_MODE_NOT_ACCESSIBLE         0x00    /* Non-discoverable or connectable */
#define BTS_ACCESS_MODE_GENERAL_ACCESSIBLE     0x03    /* General discoverable and connectable */
#define BTS_ACCESS_MODE_LIMITED_ACCESSIBLE     0x13    /* Limited discoverable and connectable */
#define BTS_ACCESS_MODE_CONNECTABLE_ONLY       0x02    /* Connectable but not discoverable */
#define BTS_ACCESS_MODE_DISCOVERABLE_ONLY      0x01    /* Discoverable but not connectable */
#define BTS_ACCESS_MODE_INVALID_ACCESS_MODE    0xFF

#endif /* __BTS_DEFINES_H__ */