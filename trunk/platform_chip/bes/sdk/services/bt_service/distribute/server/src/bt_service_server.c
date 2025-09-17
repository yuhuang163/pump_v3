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
#include <stdio.h>
#include <stdlib.h>

#include "bts_private.h"
#include "bt_service_server.h"
#include "transport_if.h"
#include "bt_service_server_internal.h"
#include "bts_pack.h"

bts_status_t bt_service_server_init(transport_if_t *transport_if)
{
    bt_service_server_internal_init(transport_if);
    return BTS_ST_SUCCESS;
}