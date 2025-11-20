
/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

/****************************************************************************
 *
 * File Description: this file defines the UI calls client interface which needs
 * data struct.
 *
 ****************************************************************************/

#ifndef __BTH_BLE_CUSTOM_API_H__
#define __BTH_BLE_CUSTOM_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************header include********************************/
#include "plat_types.h"

/******************************macro defination*****************************/


/******************************type defination******************************/
typedef struct {
    ///Connection interval value
    uint16_t            con_interval;
    ///Connection latency value
    uint16_t            con_latency;
    ///Supervision timeout
    uint16_t            sup_to;
    uint8_t conidx;
} __attribute__((packed)) ble_connection_context_t;

/****************************function declaration***************************/

/**
 ****************************************************************************************
 * @brief Register the customer callback function and enable the BLE information transfer
 *  between M55 and BTH
 *
 * @param[in] custom_ble_ui_cb      ble event callback
 *
 ****************************************************************************************
 */
void bth_ble_enable(void* custom_ble_ui_cb);

/**
 ****************************************************************************************
 * @brief Disable the BLE information transfer between M55 and BTH, unregister the customer
 *  callback function
 *
 ****************************************************************************************
 */
void bth_ble_disable(void);

/**
 ****************************************************************************************
 * @brief Start ble adv
 *
 ****************************************************************************************
 */
void bth_ble_start_adv(void);

/**
 ****************************************************************************************
 * @brief Updated ble connection parameters
 *
 * @param[in] condix                ble connection index
 * @param[in] con_interval          ble connection interval
 *                                      Range: 0x0006 to 0x0C80
 *                                      Time = N * 1.25 ms
 *                                      Time Range: 7.5 ms to 4 s.
 * @param[in] con_latency           the link supervision timeout for the LE link
 *                                      Range: 0x000A to 0x0C80
 *                                      Time = N * 10 ms
 *                                      Time Range: 100 ms to 32 s
 *
 ****************************************************************************************
 */
void bth_ble_connection_params_update(uint8_t condix, uint16_t con_interval, uint16_t con_latency, uint16_t sup_to);

#ifdef __cplusplus
}
#endif
#endif // end of __BTH_IF_CMD_DEFINITION_H__
