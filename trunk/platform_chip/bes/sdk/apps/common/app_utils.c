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
#include "cmsis.h"
#include "hal_sleep.h"
#include "hal_trace.h"
#include "app_utils.h"

#define FREQ_FREE                       0UL
#define FREQ_26M                        26UL
#define FREQ_52M                        52UL
#define FREQ_78M                        78UL
#define FREQ_104M                       104UL
#ifdef HAL_CMU_FREQ_15M
#define FREQ_15M                        15UL
#define FREQ_30M                        30UL
#define FREQ_120M                       120UL
#define FREQ_144M                       144UL
#define FREQ_168M                       168UL
#endif
#define FREQ_208M                       208UL
#ifdef HAL_CMU_FREQ_390M
#define FREQ_390M                       390UL
#endif

/*
 * qos_users, quality of services users, this kind of user must run with the
 * minist frequency they requiested, if there are more users, the frequency
 * should be boost for these users
 * e.g. the key word recorgnithion function request 26M Hz, and the music
 * also need 26M, if both of them are running, the cpu freq should boost to
 * 26M + 26M = 52M Hz
 */

#define USER_OFFSET(a)                  ((a) - APP_SYSFREQ_USER_APP_0)

/*
 * NOTE:
 * The macro QOS_USERS works only when the APP_SYSFREQ_USER_APP_XXX is not large than
 * 32, currently this works, but if the are more user, another way needed
 */
#define QOS_USERS                       ((1ULL << USER_OFFSET(APP_SYSFREQ_USER_AI_VOICE))      | \
                                        (1ULL << USER_OFFSET(APP_SYSFREQ_USER_BT_A2DP))       | \
                                        (1ULL << USER_OFFSET(APP_SYSFREQ_USER_BT_SCO))        | \
                                        (1ULL << USER_OFFSET(APP_SYSFREQ_USER_VOICE_ASSIST))  | \
                                        (1ULL << USER_OFFSET(APP_SYSFREQ_USER_AOB_PLAYBACK))  | \
                                        (1ULL << USER_OFFSET(APP_SYSFREQ_USER_AOB_CAPTURE))   | \
                                        (1ULL << USER_OFFSET(APP_SYSFREQ_USER_PSAP_SW)))

static const uint32_t freq_map[] =  {
    [HAL_CMU_FREQ_32K] = FREQ_FREE,
    [HAL_CMU_FREQ_26M] = FREQ_26M,
    [HAL_CMU_FREQ_52M] = FREQ_52M,
    [HAL_CMU_FREQ_78M] = FREQ_78M,
    [HAL_CMU_FREQ_104M] = FREQ_104M,
#ifdef HAL_CMU_FREQ_15M
    [HAL_CMU_FREQ_15M] = FREQ_15M,
    [HAL_CMU_FREQ_30M] = FREQ_30M,
    [HAL_CMU_FREQ_120M] = FREQ_120M,
    [HAL_CMU_FREQ_144M] = FREQ_144M,
    [HAL_CMU_FREQ_168M] = FREQ_168M,
#endif
    [HAL_CMU_FREQ_208M] = FREQ_208M,
#ifdef HAL_CMU_FREQ_390M
    [HAL_CMU_FREQ_390M] = FREQ_390M,
#endif
};

// !!!CAUTION!!!: The number of data in user_map[] cannot exceed 64/5=12.8
static const uint32_t sysfreq_user_map[] =  {
    [0] = USER_OFFSET(APP_SYSFREQ_USER_AI_VOICE),
    [1] = USER_OFFSET(APP_SYSFREQ_USER_BT_A2DP),
    [2] = USER_OFFSET(APP_SYSFREQ_USER_BT_SCO),
    [3] = USER_OFFSET(APP_SYSFREQ_USER_VOICE_ASSIST),
    [4] = USER_OFFSET(APP_SYSFREQ_USER_AOB_PLAYBACK),
    [5] = USER_OFFSET(APP_SYSFREQ_USER_AOB_CAPTURE),
    [6] = USER_OFFSET(APP_SYSFREQ_USER_PSAP_SW),
};

/*
 * qos_freqs_map
 * filled with user's freq, one user's freq occupy 5bits,
 * that limit the frequecy number is not large than 64
 *
 * bit field structure:
  -----------------------------------------------------
  | user11 |..................|user1  | user 0|
  -----------------------------------------------------
  | 59~55  |..................|9~5    | 4~0   |
  -----------------------------------------------------
 *
 * Ok, this is so ugly, but there is not so much frequecy level,
 * and maybe work for a long time
 */
static uint64_t qos_freqs_map;

/*
 * qos_users_map
 * filled with user's number, one user's occupy 1 bit,
 *
 bit field structure:
  -----------------------------------------------------
  |user31~12(reseverd)| user11 |........| user 1| user 0|
  -----------------------------------------------------
 *
 */
static uint32_t qos_users_map;

static uint8_t sysfreq_bundle[USER_OFFSET(APP_SYSFREQ_USER_MAX)];

static uint8_t top_user_offset;
STATIC_ASSERT(USER_OFFSET(APP_SYSFREQ_USER_MAX) <= (1 << (sizeof(top_user_offset) * 8)), "Too many app sysfreq users");

static int app_qosfreq_req(uint32_t user_offset, enum APP_SYSFREQ_FREQ_T freq)
{
    int ret;
    int qos_freq_num = 0;
    uint32_t max_qos_freq = 0;
    int user_idx;
    int i;
    uint32_t lock;

    lock = int_lock();
    for (i = 0; i < ARRAY_SIZE(sysfreq_user_map); i++) {
        if (user_offset == sysfreq_user_map[i]) {
            break;
        }
    }

    if (i >= ARRAY_SIZE(sysfreq_user_map)) {
        int_unlock(lock);
        ASSERT(0, "can not find qos user %u (0x%X)", user_offset, user_offset);
        return 0;
    }

    user_idx = i;
    if ((int)freq != (int)HAL_CMU_FREQ_32K) { // require freq
        qos_freqs_map &= ~(((uint64_t)0x1f) << (5 * i));
        qos_freqs_map |= ((uint64_t)freq) << (5 * i);
        qos_users_map |= 1 << user_idx;
    } else { //release freq
        qos_freqs_map &= ~(((uint64_t)0x1f) << (5 * i));
        qos_users_map &= ~ (1 << user_idx);
    }

    //scan the qos_user_map and sum every user's request freq
    for(i = 0; i < ARRAY_SIZE(sysfreq_user_map); i++) {
        if ((qos_users_map >> i) & 0x1) {
            uint32_t real_freq;
            int freq_num;

            freq_num = (qos_freqs_map >> (5 * i )) & 0x1f;
            real_freq = freq_map[freq_num];
            max_qos_freq += real_freq;
        }
    }

    for (i = 0; i < ARRAY_SIZE(freq_map); i++) {
        if (i) {
            if ((max_qos_freq > freq_map[i-1]) && (max_qos_freq <= freq_map[i])) {
                qos_freq_num  = i;
                break;
            }
        } else {
            if (max_qos_freq == freq_map[i]) {
                qos_freq_num = i;
                break;
            }
        }
    }

    if (i >= ARRAY_SIZE(freq_map)) {
        qos_freq_num = (HAL_CMU_FREQ_QTY - 1);
        TRACE(0, "WARNING: required sysfreq exceed");
        // ASSERT(0, "can not find actual freq");
    }

    // TRACE(2, "User %d require sysfreq %d", user_offset, qos_freq_num);

    // // TODO: remove this when chipset is stable at 208M
    // if (qos_freq_num >= HAL_CMU_FREQ_104M)
    // {
    //     qos_freq_num = HAL_CMU_FREQ_104M;
    // }

    //TRACE(2, "User %d require sysfreq %d", user_offset, qos_freq_num);
    ret = hal_sysfreq_req(HAL_SYSFREQ_USER_APP_QOS, (enum HAL_CMU_FREQ_T)qos_freq_num);

    int_unlock(lock);
    return ret;
}

int app_sysfreq_req(enum APP_SYSFREQ_USER_T user, enum APP_SYSFREQ_FREQ_T freq)
{
    int ret = 0;
    uint32_t user_offset;
    enum APP_SYSFREQ_FREQ_T top_freq;
    enum APP_SYSFREQ_FREQ_T cur_freq;
    uint32_t i;
    uint32_t lock;
    bool update = false;

    if (user < APP_SYSFREQ_USER_APP_0 || user >= APP_SYSFREQ_USER_MAX) {
        ASSERT(false, "Bad app_sysfreq_req user: %u (0x%X)", user, user);
        return -1;
    }
    if (freq >= APP_SYSFREQ_FREQ_QTY) {
        ASSERT(false, "Bad app_sysfreq_req freq: %u", freq);
        return -2;
    }

    user_offset = USER_OFFSET(user);

    // if user is qos user
    if ((1ULL << user_offset) & QOS_USERS) {
        return app_qosfreq_req(user_offset, freq);
    }

    // if user is NOT qos user
    lock = int_lock();

    top_freq = sysfreq_bundle[top_user_offset];

    sysfreq_bundle[user_offset] = freq;

    if (freq > top_freq) {
        top_freq = freq;
        top_user_offset = user_offset;
        update = true;
    } else if (user_offset == top_user_offset && freq < top_freq) {
        cur_freq = top_freq;
        top_freq = sysfreq_bundle[0];
        top_user_offset = 0;
        for (i = 1; i < ARRAY_SIZE(sysfreq_bundle); i++) {
            if (top_freq < sysfreq_bundle[i]) {
                top_freq = sysfreq_bundle[i];
                top_user_offset = i;
            }
        }
        if (cur_freq != top_freq) {
            update = true;
        }
    }

    if (update) {
        ret = hal_sysfreq_req(HAL_SYSFREQ_USER_APP_COMMON, (enum HAL_CMU_FREQ_T)top_freq);
    }

    int_unlock(lock);

    return ret;
}

uint32_t app_sysfreq_get_value(enum APP_SYSFREQ_FREQ_T freq)
{
    if (freq < ARRAY_SIZE(freq_map))
        return freq_map[freq];
    else
        return 0;
}

// ------------------------------------------------------------------------------
// APP WAKE LOCK USERS
// ------------------------------------------------------------------------------

#define WAKE_USER_OFFSET(a)             ((a) - APP_WAKE_LOCK_USER_APP_0)

static uint32_t app_wake_map;
STATIC_ASSERT(WAKE_USER_OFFSET(APP_WAKE_LOCK_USER_MAX) <= (sizeof(app_wake_map) * 8), "Too many app wake lock users");

int app_wake_lock(enum APP_WAKE_LOCK_USER_T user)
{
    uint32_t lock;

    if (user < APP_WAKE_LOCK_USER_APP_0 || user >= APP_WAKE_LOCK_USER_MAX) {
        ASSERT(false, "Bad app_wake_lock user: %u (0x%X)", user, user);
        return -1;
    }

    lock = int_lock();
    if (app_wake_map == 0) {
        hal_bus_wake_lock(HAL_BUS_WAKE_LOCK_USER_APP_COMMON);
    }
    app_wake_map |= (1 << WAKE_USER_OFFSET(user));
    int_unlock(lock);

    return 0;
}

int app_wake_unlock(enum APP_WAKE_LOCK_USER_T user)
{
    uint32_t lock;

    if (user < APP_WAKE_LOCK_USER_APP_0 || user >= APP_WAKE_LOCK_USER_MAX) {
        ASSERT(false, "Bad app_wake_unlock user: %u (0x%X)", user, user);
        return -1;
    }

    lock = int_lock();
    if (app_wake_map) {
        app_wake_map &= ~(1 << WAKE_USER_OFFSET(user));
        if (app_wake_map == 0) {
            hal_bus_wake_unlock(HAL_BUS_WAKE_LOCK_USER_APP_COMMON);
        }
    }
    int_unlock(lock);

    return 0;
}
