/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "drv_uart.h"
#include "los_event.h"
#include "los_task_pri.h"
#include "hal_uart.h"
#include "hal_trace.h"

//#define LOS_UART_DBG

extern void a7_uart_trace_open(void);
extern void a7_trace_output_handler(const unsigned char* data, unsigned int size);
extern void a7_trace_flush(void);
extern void panicHandler_pre_hook(void);
VOID UartPuts(const CHAR *s, UINT32 len, BOOL isLock)
{
    static bool exception = false;
#ifndef LOS_UART_DBG
    if (strstr(s, "#excFrom:") != NULL) {
        exception = true;
        panicHandler_pre_hook();
    }

    if (exception) {
        if (hal_trace_get_transport() < HAL_TRACE_TRANSPORT_QTY) {
            TRACE_OUTPUT((const unsigned char *)s, len);
            TRACE_FLUSH();
        } else {
            a7_trace_output_handler((const unsigned char*)s,len);
            a7_trace_flush();
        }
    } else
        TRACE_OUTPUT((const unsigned char *)s, len);
#else
    hal_uart_printf_output((const uint8_t *)s, len);
#endif
}

VOID uart_init(VOID)
{
#ifdef LOS_UART_DBG
    hal_uart_printf_init();
#else
    a7_uart_trace_open();
#endif
}
