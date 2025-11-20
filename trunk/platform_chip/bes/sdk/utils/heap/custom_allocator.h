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
#ifndef CUSTOM_ALLOCATOR
#define CUSTOM_ALLOCATOR

#include "stddef.h"
#include <stdint.h>

typedef struct
{
    void* (*malloc)(size_t);
    void* (*calloc)(size_t, size_t);
    void (*free)(void*);
    int (*log)(uint32_t attr, const char *fmt, ...);
    int (*beco_init)(void);
    int (*beco_exit)(void);
} custom_allocator;

#ifdef __cplusplus
extern "C" {
#endif

custom_allocator *default_allocator(void);

custom_allocator *audio_allocator(void);

custom_allocator *pool_allocator(void);

#ifdef __cplusplus
}
#endif

#endif
