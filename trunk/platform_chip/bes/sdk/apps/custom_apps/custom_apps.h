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

#ifndef __CUSTOM_APPS_H__
#define __CUSTOM_APPS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CUSTOM_APPS_MSG_DEMO = 0,
    // TODO: add new message id here and add the entry via CUSTOM_APPS_MESSAGE_HANDLER_TO_ADD

    CUSTOM_APPS_MSG_CNT,
} CUSTOM_APPS_MSG_ID_E;


// custom message handler management
typedef void (*CustomAppsMessageHandler_t)(uint16_t message_id, uint8_t* ptrParam, uint32_t paramLen);

typedef struct
{
    uint16_t                    message_id;
    CustomAppsMessageHandler_t  messageHandler;
} CUSTOM_APPS_MESSAGE_HANDLER_INSTANCE_T;

#define CUSTOM_APPS_MESSAGE_HANDLER_TO_ADD(message_id, messageHandler)    \
    static const CUSTOM_APPS_MESSAGE_HANDLER_INSTANCE_T msgCode##_entry __attribute__((used, section(".custom_apps_message_handler_table"))) =     \
        {(message_id), (messageHandler)};


void* custom_apps_heap_allocate(uint32_t length);
void custom_apps_heap_free(void* ptr);
uint32_t custom_apps_heap_get_free_size(void);
int32_t custom_apps_mailbox_put(uint16_t       message_id, uint8_t* msg_param_ptr, uint32_t msg_param_len);


#ifdef __cplusplus
}
#endif
#endif // __CUSTOM_APPS_H__

