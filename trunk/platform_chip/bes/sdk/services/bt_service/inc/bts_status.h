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
#ifndef __BTS_STATUS_H__
#define __BTS_STATUS_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef int bts_status_t;

#define BTS_ST_SUCCESS         (0)
#define BTS_ST_FAILED          (-1)
#define BTS_ST_PENDING         (-2)
#define BTS_ST_BUSY            (-3)
#define BTS_ST_NO_RESOURCES    (-4)

#if defined(__cplusplus)
}
#endif

#endif /* __BTS_STATUS_H__ */