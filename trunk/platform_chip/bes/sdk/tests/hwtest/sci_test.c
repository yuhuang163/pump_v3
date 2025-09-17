/***************************************************************************
 *
 * Copyright 2021-2023 BES.
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
#ifdef SCI_TEST

#include "hal_iomux.h"
#include "hal_sci.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include <stdlib.h>

#define SCI_SYSFREQ HAL_CMU_FREQ_26M

typedef struct apdu {
    uint8_t apdu_size;
#define _NO_LC 0
#define _HAS_LC 1
    uint8_t has_Lc;
#define _NO_LE 0
#define _HAS_LE 1
    uint8_t has_Le;

    uint8_t CLA;
    uint8_t INC;
    uint8_t P1;
    uint8_t P2;
#define INVALID_LC 0x0u
#define INVALID_LE 0xFFu
    uint8_t Lc_data_Le[];
} apdu_t;

#define GTE_APDU_HEAD(t) ((uint8_t *)&(t->CLA))
#define GTE_APDU_LE(t) (t->has_Le == _HAS_LE ? *(GTE_APDU_HEAD(t) + t->apdu_size - 1) : 0x100u)

#define UICC_INS_SELECT 0xA4
#define UICC_INS_READ_BINARY 0xB0
#define UICC_INS_READ_RECORD 0xB2

typedef struct r_apdu_s {
    uint8_t *r_apdu;
    uint16_t r_apdu_size;
    uint8_t sw1;
    uint8_t sw2;
} r_apdu_t;

static apdu_t *_new_apdu_command(
    uint8_t cla,
    uint8_t ins,
    uint8_t p1,
    uint8_t p2,
    uint8_t lc,
    uint8_t *data,
    uint8_t le)
{
    if (data != NULL && lc == INVALID_LC) {// lc should equal to data size
        return NULL;
    }
    uint8_t _index = 0;
    apdu_t *_apdu_p = (apdu_t *)malloc(sizeof(apdu_t) + lc + (le == INVALID_LE ? 0 : 1));
    assert(_apdu_p != NULL);

    _apdu_p->CLA = cla;
    _apdu_p->INC = ins;
    _apdu_p->P1 = p1;
    _apdu_p->P2 = p2;
    _apdu_p->has_Lc = (lc == INVALID_LC ? _NO_LC : _HAS_LC);
    _apdu_p->has_Le = (le == INVALID_LE ? _NO_LE : _HAS_LE);
    if (data != NULL) {
        _apdu_p->Lc_data_Le[_index++] = lc;
        memcpy(_apdu_p->Lc_data_Le + 1, data, lc);
        _index += lc;
    }
    if (le != INVALID_LE) {
        _apdu_p->Lc_data_Le[_index++] = le;
    }
    _apdu_p->apdu_size = _index + 4; // add cla,ins,p1,p2

    return _apdu_p;
}

static void _delete_apdu_command(apdu_t *_apdu_p)
{
    free(_apdu_p);
}

static uint8_t response[256] = {0};

static void sci_send_select_apdu(uint8_t cla,
                                 uint8_t ins,
                                 uint8_t p1,
                                 uint8_t p2,
                                 uint8_t lc,
                                 uint8_t *data,
                                 uint8_t le)
{
    uint8_t procedure, sw1, sw2;
    (void)sw1;
    apdu_t *_apdu_p = _new_apdu_command(cla, ins, p1, p2, lc, data, le);
    hal_sci_send_command(GTE_APDU_HEAD(_apdu_p), T0_TPDU_COMMAND_HEADER_LEN);
    hal_sci_get_response(&procedure, 1);
    if (procedure == ins) {
        hal_sci_send_command(GTE_APDU_HEAD(_apdu_p) + T0_TPDU_COMMAND_HEADER_LEN, lc);
        hal_sci_get_response(&procedure, 1);
        if (procedure == 0x61) {
            sw1 = procedure;
            hal_sci_get_response(&sw2, 1);
            uint8_t get_response_command[T0_TPDU_COMMAND_HEADER_LEN] = {0x00, 0xC0, 0x00, 0x00, 0x00};
            get_response_command[4] = sw2;
            hal_sci_send_command(get_response_command, T0_TPDU_COMMAND_HEADER_LEN);
            hal_sci_get_response(&procedure, 1);
            hal_sci_get_response(response, sw2 + T0_APDU_SW1SW2_LEN);
        }
    }
    _delete_apdu_command(_apdu_p);
}

static void sci_send_read_apdu(uint8_t cla,
                               uint8_t ins,
                               uint8_t p1,
                               uint8_t p2,
                               uint8_t lc,
                               uint8_t *data,
                               uint8_t le)
{
    uint8_t procedure, sw1, sw2;
    (void)sw1;
    (void)sw2;
    apdu_t *_apdu_p = _new_apdu_command(cla, ins, p1, p2, lc, data, le);
    hal_sci_send_command(GTE_APDU_HEAD(_apdu_p), T0_TPDU_COMMAND_HEADER_LEN);
    hal_sci_get_response(&procedure, 1);
    if (procedure == ins) {
        hal_sci_get_response(response, le + T0_APDU_SW1SW2_LEN);
    }
    _delete_apdu_command(_apdu_p);
}

void sci_test(void)
{
    TRACE(0, "  ");
    TRACE(0, "------sci test started------");
    TRACE(0, "  ");

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, SCI_SYSFREQ);
    hal_iomux_set_sci();
    hal_sci_open();
    hal_sys_timer_delay(200);

    TRACE(0, "  ");
    TRACE(0, "------sci test SELECT MF------");
    TRACE(0, "  ");
    uint8_t mf[2] = {0x3F, 0x00};
    sci_send_select_apdu(0x00, UICC_INS_SELECT, 0x00, 0x04, 0x02, mf, 0x00);
    hal_sys_timer_delay(200);

    TRACE(0, "  ");
    TRACE(0, "------sci test SELECT EF ICCID------");
    TRACE(0, "  ");
    uint8_t iccid[2] = {0x2F, 0xE2};
    sci_send_select_apdu(0x00, UICC_INS_SELECT, 0x00, 0x04, 0x02, iccid, 0x00);
    hal_sys_timer_delay(200);

    TRACE(0, "  ");
    TRACE(0, "------sci test READ ICCID------");
    TRACE(0, "  ");
    sci_send_read_apdu(0x00, UICC_INS_READ_BINARY, 0x00, 0x00, INVALID_LC, NULL, 0x0A);
    hal_sys_timer_delay(200);

    TRACE(0, "  ");
    TRACE(0, "------sci test SELECT EF_DIR------");
    TRACE(0, "  ");
    uint8_t efdir[2] = {0x2F, 0x00};
    sci_send_select_apdu(0x00, UICC_INS_SELECT, 0x00, 0x04, 0x02, efdir, 0x00);
    hal_sys_timer_delay(200);

    TRACE(0, "  ");
    TRACE(0, "------sci test READ EF_DIR------");
    TRACE(0, "  ");
    sci_send_read_apdu(0x00, UICC_INS_READ_RECORD, 0x01, 0x04, INVALID_LC, NULL, 0x20);
    hal_sys_timer_delay(200);

    TRACE(0, "  ");
    TRACE(0, "------sci test SELECT AID USIM------");
    TRACE(0, "  ");
    sci_send_select_apdu(0x00, UICC_INS_SELECT, 0x04, 0x04, response[3], &response[4], 0x00);
    hal_sys_timer_delay(200);

    TRACE(0, "  ");
    TRACE(0, "------sci test SELECT IMSI------");
    TRACE(0, "  ");
    uint8_t imsi[2] = {0x6F, 0x07};
    sci_send_select_apdu(0x00, UICC_INS_SELECT, 0x00, 0x04, 0x02, imsi, 0x00);
    hal_sys_timer_delay(200);

    TRACE(0, "  ");
    TRACE(0, "------sci test READ IMSI------");
    TRACE(0, "  ");
    sci_send_read_apdu(0x00, UICC_INS_READ_BINARY, 0x00, 0x00, INVALID_LC, NULL, 0x09);
    hal_sys_timer_delay(200);
}

#endif
