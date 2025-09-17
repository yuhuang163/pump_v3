/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#ifndef __AIOT_GENERIC_H__
#define __AIOT_GENERIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PLAT_INIT_FLAG_WDG_EN      (1 << 0)
#define PLAT_INIT_FLAG_BT_EN       (1 << 1)
#define PLAT_INIT_FLAG_NET_EN      (1 << 2)
#define PLAT_INIT_FLAG_A7_EN       (1 << 3)
#define PLAT_INIT_FLAG_ESHELL_EN   (1 << 4)
#define PLAT_INIT_FLAG_ALL_EN      (0xFFFF)

int platform_driver_init(int flags);
int platform_framework_init(int flags);
void platform_deamon_start(void);
int platform_trace_disable(void);
int platform_trace_enable(void);

typedef void (*InitCall)(void);
#define USED_ATTR __attribute__((used))

#define APP_BEGIN(name)                                         \
    ({  extern InitCall __besinitcall_app_##name##_start;       \
        InitCall *initCall = &__besinitcall_app_##name##_start; \
        (initCall);                                             \
    })

#define APP_END(name)                                         \
    ({  extern InitCall __besinitcall_app_##name##_end;       \
        InitCall *initCall = &__besinitcall_app_##name##_end; \
        (initCall);                                           \
    })

#define INIT_APP_CALL(name)                                      \
    do {                                                    \
        InitCall *initcall = (InitCall *)(APP_BEGIN(name)); \
        InitCall *initend = (InitCall *)(APP_END(name));    \
        for (; initcall < initend; initcall++) {            \
            (*initcall)();                                  \
        }                                                   \
    } while (0)


#define LAYER_INITCALL(func, layer, clayer)                 \
    static const InitCall USED_ATTR __besinitcall_##layer##_##func \
        __attribute__((section(".besinitcall." clayer ".init"))) = func

#define APP_FEATURE_INIT(func) LAYER_INITCALL(func, app_feature, "app.feature")

void aiot_platform_pre_initializing_callback(void);
void aiot_platform_initialized_callback(void);

#ifdef __cplusplus
}
#endif
#endif // __AIOT_GENERIC_H__

