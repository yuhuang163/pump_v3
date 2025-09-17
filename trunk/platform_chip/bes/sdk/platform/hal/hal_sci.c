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
#include "plat_addr_map.h"

#ifdef SCI_BASE

#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_sci.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "reg_sci.h"
#include "string.h"

#define SCI_DEBUG

#ifdef SCI_DEBUG
#define HAL_SCI_TRACE(n, s, ...)        TRACE(n, s, ##__VA_ARGS__)
#define HAL_SCI_TRACE_NOCRLF(n, s, ...) TRACE_NOCRLF(n, s, ##__VA_ARGS__)
#define HAL_SCI_DUMP8(n, s, ...)        DUMP8(n, s, ##__VA_ARGS__)
#else
#define HAL_SCI_TRACE(n, s, ...)
#define HAL_SCI_TRACE_NOCRLF(n, s, ...)
#define HAL_SCI_DUMP8(n, s, ...)
#endif

#define CLK_CYCLES_PER_MS        4000

#define SCI_CLK_DIV              6 // 24M/6 = 4M
#define SCI_ETU_RESET            372
#define SCI_ETU_WORKING          16

#define ATR_LEN                  33

#define SCARD_PROTOCOL_UNDEFINED 0x00000000 // There is no active protocol.
#define SCARD_PROTOCOL_T0        0x00000001 // T=0 is the active protocol.
#define SCARD_PROTOCOL_T1        0x00000002 // T=1 is the active protocol.
// Block waiting integer default value as definded by ISO
#define T1_BWI_DEFAULT           4
// Character waiting integer default value as definded by ISO
#define T1_CWI_DEFAULT           13

#define T0_WI_DEFAULT            10
#define SCARD_GT_N_DEFAULT       0
#define SCARD_FI_DI_DEFAULT      10

#define SCI_TX_FIFO_LEN          8
#define SCI_RX_FIFO_LEN          8

#define TX_BUFF_LEN              512
#define RX_BUFF_LEN              512

enum ISO7816_OP_MODE_T {
    ISO7816_OP_SPECIFIC,
    ISO7816_OP_NEGOTIABLE,
};

struct CLOCK_RATE_CONVERSION_T {

    const uint32_t F;
    const uint32_t fs;
};

struct BIT_RATE_ADJUSTMENT_T {

    const uint32_t DNumerator;
    const uint32_t DDivisor;
};

struct PTS_DATA_T {
#define PTS_TYPE_DEFAULT 0x00
#define PTS_TYPE_OPTIMAL 0x01
#define PTS_TYPE_USER    0x02

    uint8_t Type;

    uint8_t Fl; // Fl value for PTS

    uint8_t Dl; // Dl value for PTS

    uint32_t CLKFrequency; // New clock frequency

    uint32_t DataRate; // New baud rate to be used after pts

    uint8_t StopBits; // new stop bits to be used after pts
};

struct SCARD_CARD_CAPABILITIES_T {
    // Flag that indicates that the current card uses invers convention
    bool InversConvention;

    // Calculated etu
    uint32_t etu;

    // Answer To Reset string returned by card.
    struct
    {
        uint8_t Buffer[64];
        uint8_t Length;
    } ATR;

    struct
    {
        uint8_t Buffer[16];
        uint8_t Length;
    } HistoricalChars;

    struct CLOCK_RATE_CONVERSION_T *ClockRateConversion;
    struct BIT_RATE_ADJUSTMENT_T *BitRateAdjustment;

    uint8_t FI;  // Clock rate conversion
    uint8_t DI;  // Bit rate adjustment
    uint8_t II;  // Maximum programming current
    uint8_t P;   // Programming voltage in .1 Volts
    uint8_t N;   // Extra guard time in etu
    uint32_t GT; // Calculated guard time in micro seconds

    struct
    {
        uint32_t Supported; // This is a bit mask of the supported protocols
        uint32_t Selected;  // The currently selected protocol
    } Protocol;

    // T=0 specific data
    struct
    {
        uint8_t GT;  // Guard time,
        uint8_t WI;  // Waiting integer
        uint32_t WT; // Waiting time in milli seconds
    } T0;

    // T=1 specific data
    struct
    {
        uint8_t IFSC; // Information field size of card

        // Character waiting integer and block waiting integer
        uint8_t CWI;
        uint8_t BWI;

        uint8_t EDC; // Error detection code

        // Character and block waiting time in micro seconds
        uint32_t CWT;
        uint32_t BWT;

        uint32_t BGT; // Block guarding time in micro seconds
        int CGT;
    } T1;

    //
    //
    //
    uint8_t classSeleted; // 0, A, 1, B, 2,C
    uint8_t clockStop;    //
    uint8_t spu;

    struct PTS_DATA_T PtsData;

    uint8_t pps2Flag; // 0:invalid, 1:valid
    uint8_t pps2;

    uint8_t Reserved[100 - sizeof(struct PTS_DATA_T)];
};

static struct SCI_T *const sci = (struct SCI_T *)SCI_BASE;

static struct rxdata {
    uint8_t buf[RX_BUFF_LEN];
    uint32_t rxlen;
    uint8_t fifolen;
    uint32_t retlen;
} rxdata;

static struct txdata {
    uint8_t buf[TX_BUFF_LEN];
    int txlen;
    uint8_t fifolen;
    int leftlen;
} txdata;

#ifdef RTOS
static osSemaphoreId_t sem_sci_recv_done = NULL;
static osSemaphoreId_t sem_sci_send_done = NULL;
#endif

static enum ISO7816_OP_MODE_T OperationMode;
static struct SCARD_CARD_CAPABILITIES_T cardCapabilities;

#define RFU 0xffff
static const uint32_t Fi[16] = {372, 372, 558, 744, 1116, 1488, 1860, RFU, RFU, 512, 768, 1024, 1536, 2048, RFU, RFU};
static const uint32_t Di[16] = {RFU, 1, 2, 4, 8, 16, 32, 64, 12, 20, RFU, RFU, RFU, RFU, RFU, RFU};

static void hal_sci_send_data(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++) {
        sci->SCIBUFHW = buf[i];
    }
}

static void hal_sci_irq_handler(void)
{
    uint32_t sciint, scistat;
    sciint = sci->SCIINTIO1;
    scistat = sci->SCISTAT;

    sci->SCIINTIO1 = 0;

    (void)scistat;
    // HAL_SCI_TRACE(0, "SCIINTIO1:%08x SCISTAT:%08x", sciint, scistat);

    if (sciint & TX_INT) {
        // HAL_SCI_TRACE(0, "leftlen:%d", txdata.leftlen);
        if (0 == txdata.leftlen) {
#ifdef RTOS
            osSemaphoreRelease(sem_sci_send_done);
#endif
            return;
        }
        if (txdata.leftlen > SCI_TX_FIFO_LEN) {
            hal_sci_send_data(&txdata.buf[txdata.txlen - txdata.leftlen], SCI_TX_FIFO_LEN);
            txdata.leftlen -= SCI_TX_FIFO_LEN;
        } else {
            hal_sci_send_data(&txdata.buf[txdata.txlen - txdata.leftlen], txdata.leftlen);
            txdata.leftlen = 0;
        }
    }

    if (sciint & RX_INT) {
        int remainlen;
        while (sci->SCISTAT & RX_FIFO_EMPTY) {
            rxdata.buf[rxdata.retlen++] = sci->SCIBUFHW;
            remainlen = rxdata.rxlen - rxdata.retlen;

            if (remainlen == 0) {
#ifdef RTOS
                osSemaphoreRelease(sem_sci_recv_done);
#endif
            }
        }
        // HAL_SCI_TRACE(0, "remainlen:%d", remainlen);
    }
}

static void hal_sci_irq_enable(void)
{
    NVIC_EnableIRQ(SCI_IRQn);
}

static void hal_sci_irq_disable(void)
{
    NVIC_DisableIRQ(SCI_IRQn);
}

static uint32_t hal_sci_get_atr(uint8_t *atr)
{
    return hal_sci_get_response(atr, ATR_LEN);
}

static uint8_t inverse_byte(uint8_t byte)
{
    uint8_t temp = 0;
    uint8_t reverse_byte = 0;

    for (int i = 0; i < 8; i++) {
        temp = (byte >> i);
        reverse_byte = (reverse_byte << 1 | (temp & 1));
    }

    return ~reverse_byte;
}

static void inverse_ATR(uint8_t *atr, uint32_t len)
{
    uint8_t temp = 0;
    for (uint32_t i = 0; i < len; i++) {
        temp = inverse_byte(atr[i]);
        atr[i] = temp;
    }
}

static int hal_update_card_capabilities()
{
    uint8_t *atrString = cardCapabilities.ATR.Buffer;
    uint32_t atrLength = (uint32_t)cardCapabilities.ATR.Length;
    uint8_t Tck, TA[4] = {0}, TB[4] = {0}, TC[4] = {0}, TD[4] = {0}, Y;
    bool TA2Present = 0;
    uint32_t i, numProtocols = 0, protocolTypes = 0;
    int indexT1 = -1, indexT15 = -1;
    int status = 0;
    if (atrLength < 2) {
        HAL_SCI_TRACE(0, "ATR is too short (Min. length is 2)!");
        return 0;
    }

    if (atrString[0] != 0x3b && atrString[0] != 0x3f && atrString[0] != 0x03) {
        HAL_SCI_TRACE(0, "Initial character %02xh of ATR is invalid\n", atrString[0]);
        return 0;
    }

    if (*atrString != 0x3b) {
        cardCapabilities.InversConvention = true;
        HAL_SCI_TRACE(0, "Card uses Inverse Convention\r\n");
        inverse_ATR(atrString, atrLength);
        sci->SCIMODHW |= DIRC_SEL;
    }

    atrString += 1;
    atrLength -= 1;

    // Calculate check char, but do not test now since if only T=0  is present the ATR doesn't contain a check char
    for (i = 0, Tck = 0; i < atrLength; i++) {
        Tck ^= atrString[i]; // order to check sum, if T =0 and T=15 and other case , T0 - TCK should ble = 0
    }

    // Set default values as described in ISO 7816-3
    TA[0] = SCARD_FI_DI_DEFAULT; // TA1 codes FI in high-byte and Dl in low-byte;
    TB[0] = 0x25;                // TB1 codes II in bits b7/b6 and PI1 in b5-b1. b8 has to be 0
    TC[0] = SCARD_GT_N_DEFAULT;  // TC1 codes N in bits b8-b1,
    TC[1] = T0_WI_DEFAULT;       // TC2 codes T=0 WI

    // Translate ATR string to TA to TD values (See ISO)
    cardCapabilities.HistoricalChars.Length = *atrString & 0x0f;
    Y = *atrString & 0xf0; // Y is TD1

    atrString += 1; // Setp to TA1 ..... TK, TCK
    atrLength -= 1;

    cardCapabilities.pps2Flag = 0; // clear pps2 flag

    for (i = 0; i < 4; i++) {
        if (Y & 0x10) // TAi
        {
            if (i == 1) // TA2
            {
                TA2Present = 1;
            }

            TA[i] = *atrString++;
            atrLength--;
        }

        if (Y & 0x20) // TBi
        {
            TB[i] = *atrString++;
            atrLength--;

            if (cardCapabilities.pps2Flag != 1) {
                if ((i >= 2) && ((TD[i - 1] & 0xF) == 15)) // first TBi when T=15
                {
                    cardCapabilities.pps2 = (TB[i] & 0x90); // low impedence buffer supported only
                    cardCapabilities.pps2Flag = 1;
                }
            }
        }

        if (Y & 0x40) { // TCi
            TC[i] = *atrString++;
            atrLength--;
        }

        if (Y & 0x80) { // TDi
            // get the first T=1, and T=15
            if (1 == (*atrString & 0x0f)) {
                indexT1 = i;
            }
            if (15 == (*atrString & 0x0f)) {
                indexT15 = i;
            }

            Y = *atrString & 0xf0; // TDi + 1
            TD[i] = *atrString++ & 0x0f;
            atrLength--;

            // Check if the next parameters are for a new protocol.
            if (((1 << TD[i]) & protocolTypes) == 0) {
                // Count the number of protocols that the card supports
                numProtocols++;
            }
            protocolTypes |= 1 << TD[i];
        } else {
            break;
        }
    }

    // step to T1 .... TK, TCK

    // Check if the card supports a protocol other than T=0
    if (protocolTypes & ~1) {
        // The atr contains a checksum byte. Exclude that from the historical byte length check
        atrLength -= 1;

        // This card supports more than one protocol or a protocol other than T=0, so test if the checksum is correct
        if (Tck != 0) {
            HAL_SCI_TRACE(0, "ATR Checksum is invalid\r\n");
            status = 0;
            goto _leave_lab;
        }
    }

    if (atrLength < 0 || atrLength != cardCapabilities.HistoricalChars.Length) { //? TCK
        HAL_SCI_TRACE(0, "ATR length is inconsistent\r\n");
        status = 0;
        goto _leave_lab;
    }

_leave_lab:
    if (status != 0) {
        return status;
    }

    // store historical characters
    memcpy(cardCapabilities.HistoricalChars.Buffer, atrString, cardCapabilities.HistoricalChars.Length);

    // Now convert TA - TD values to global interface bytes
    if (!TA2Present || (TA2Present && ((TA[1] & 0X10) == 0))) { // if TA2 bit 5 is 1 fi & di shall apply, else implicit apply
        cardCapabilities.FI = (TA[0] & 0xf0) >> 4;              // Clock rate conversion
        cardCapabilities.DI = (TA[0] & 0x0f);                   // bit rate adjustment
    }

    cardCapabilities.N = TC[0]; // Extra guard time

    {
        HAL_SCI_TRACE(0, "indexT15 is %d\n", indexT15);
        if (cardCapabilities.N < 255) {
            cardCapabilities.T0.GT = 12 + cardCapabilities.N; // Unit is etu
            cardCapabilities.T1.CGT = cardCapabilities.T0.GT;
        } else {
            HAL_SCI_TRACE(0, "TC1 absent\n");
            cardCapabilities.T1.CGT = 11; // unit etu
            cardCapabilities.T0.GT = 12;
        }
    }

    // clock stop & class
    if (-1 != indexT15) {
        i = indexT15 + 1;
        cardCapabilities.clockStop = TA[i] & 0xC0;    // the first TA fot T=15 BIT 8, 7
        cardCapabilities.classSeleted = TA[i] & 0x3F; ////the first TA fot T=15 6 - 1
        cardCapabilities.spu = TB[i] ? TB[i] : 0x00;

        HAL_SCI_TRACE(0, "\t clockStop = %d :  \n", cardCapabilities.clockStop);
        HAL_SCI_TRACE(0, "\t classSeleted = %d :  \n", cardCapabilities.classSeleted);
        HAL_SCI_TRACE(0, "\t spu = %d :  \n", cardCapabilities.spu);
    }
    if (TA2Present) { // just want to check TA2 preset
        // If the card supports only one protocol (or T=0 as default)
        // and only standard paramters then PTS selection is not available
        OperationMode = ISO7816_OP_SPECIFIC;
    } else {
        OperationMode = ISO7816_OP_NEGOTIABLE;
    }

    // Now find protocol specific data
    if (TD[0] == 0) { // TD[0] == 0: TA2 absen, T=0, must
        cardCapabilities.Protocol.Supported |= SCARD_PROTOCOL_T0;
        cardCapabilities.T0.WI = TC[1];
        HAL_SCI_TRACE(0, "T=0 Values from ATR:  WI = %x\n", cardCapabilities.T0.WI);
    }

    if (protocolTypes & SCARD_PROTOCOL_T1) {
        // find the First T=1
        HAL_SCI_TRACE(0, "T=1 , supported :  \n");
        HAL_SCI_TRACE(0, "\t indexT1 = %d :  \n", indexT1);
        i = indexT1 + 1;

        cardCapabilities.Protocol.Supported |= SCARD_PROTOCOL_T1;

        cardCapabilities.T1.IFSC = (TA[i] ? TA[i] : 32);
        cardCapabilities.T1.CWI = ((TB[i] & 0x0f) ? (TB[i] & 0x0f) : T1_CWI_DEFAULT);
        cardCapabilities.T1.BWI = ((TB[i] & 0xf0) >> 4 ? (TB[i] & 0xf0) >> 4 : T1_BWI_DEFAULT);
        cardCapabilities.T1.EDC = (TC[i] & 0x01);

        HAL_SCI_TRACE(0, "\t T1.IFSC = %d :  \n", cardCapabilities.T1.IFSC);
        HAL_SCI_TRACE(0, "\t T1.CWI = %d :  \n", cardCapabilities.T1.CWI);
        HAL_SCI_TRACE(0, "\t T1.BWI = %d :  \n", cardCapabilities.T1.BWI);
        HAL_SCI_TRACE(0, "\t T1.EDC = %d :  \n", cardCapabilities.T1.EDC);
    }

    if (OperationMode == ISO7816_OP_SPECIFIC) {
        if (TA2Present) {
            // TA2 is present in the ATR, so use the protocol indicated in the ATR
            cardCapabilities.Protocol.Selected = 1 << TA[1];
        } else {
            // The card only supports one protocol So make that one protocol the current one to use
            cardCapabilities.Protocol.Selected = cardCapabilities.Protocol.Supported;
        }
    }

    return 1;
}

static void hal_sci_init_rx_data(uint32_t HaveReadLen)
{
    if (HaveReadLen >= rxdata.retlen || HaveReadLen == 0U) {
        rxdata.retlen = 0;
    } else {
        rxdata.retlen -= HaveReadLen;
        memmove(rxdata.buf, rxdata.buf + HaveReadLen, rxdata.retlen);
    }
    rxdata.rxlen = 0;
}

static int hal_sci_request_pps(uint8_t *pps_request, uint8_t len)
{
    uint8_t pps_reply[256] = {0};
    uint32_t retlen = 0;
    hal_sci_send_command(pps_request, len);
    retlen = hal_sci_get_response(pps_reply, len);
    HAL_SCI_TRACE(0, "PPS retlen %d", retlen);
    if (pps_request[0] == pps_reply[0] && pps_request[1] == pps_reply[1] && pps_request[2] == pps_reply[2] && pps_request[3] == pps_reply[3]) {
        HAL_SCI_TRACE(0, "PPS successful!");
        return 1;
    }
    return 0;
}

int32_t hal_sci_open(void)
{
    int ret = 0;
    uint32_t atr_len = 0;
    hal_cmu_clock_enable(HAL_CMU_MOD_P_SCI);
    hal_cmu_clock_enable(HAL_CMU_MOD_Q_SCI);
    hal_cmu_reset_clear(HAL_CMU_MOD_P_SCI);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_SCI);

    hal_cmu_sci_set_div(SCI_CLK_DIV);

    hal_sci_reset_card();

#ifdef RTOS
    sem_sci_recv_done = osSemaphoreNew(1, 0, NULL);
    sem_sci_send_done = osSemaphoreNew(1, 0, NULL);
#endif

    hal_sci_init_rx_data(0);

    NVIC_SetVector(SCI_IRQn, (uint32_t)hal_sci_irq_handler);
    NVIC_SetPriority(SCI_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(SCI_IRQn);
    NVIC_EnableIRQ(SCI_IRQn);

    atr_len = hal_sci_get_atr(rxdata.buf);
    HAL_SCI_TRACE(0, "atr_len:%d", atr_len);
    memcpy(cardCapabilities.ATR.Buffer, rxdata.buf, atr_len);
    cardCapabilities.ATR.Length = atr_len;
    ret = hal_update_card_capabilities();
    if (0 == ret) {
        hal_sci_reset_card();
    } else {
        uint8_t pps_request[] = {0xFF, 0x10, 0, 0, 0};
        uint8_t pps_len = 0;
        // if card can supported T0, seleted T0, if card only supported T1, seleted T1
        if (cardCapabilities.Protocol.Supported & SCARD_PROTOCOL_T0 || cardCapabilities.Protocol.Supported == SCARD_PROTOCOL_UNDEFINED) {
            HAL_SCI_TRACE(0, "Request PPS: T0");
            pps_request[1] = 0x10;
        } else if (cardCapabilities.Protocol.Supported & SCARD_PROTOCOL_T1) {
            HAL_SCI_TRACE(0, "Request PPS: T1");
            pps_request[1] = 0x11;
        }
        HAL_SCI_TRACE(0, "FI is %d  DI is %d", cardCapabilities.FI, cardCapabilities.DI);
        pps_request[2] = (cardCapabilities.FI << 4) | cardCapabilities.DI;
        if (cardCapabilities.pps2Flag) {
            pps_request[1] |= 0x20; // pps2 present
            pps_request[3] = cardCapabilities.pps2;
            pps_request[4] = pps_request[0] ^ pps_request[1] ^ pps_request[2] ^ pps_request[3];
            pps_len = 5;
        } else {
            pps_request[3] = pps_request[0] ^ pps_request[1] ^ pps_request[2];
            pps_len = 4;
        }
        if (!hal_sci_request_pps(pps_request, pps_len)) {
            HAL_SCI_TRACE(0, "pps error");
            return -1;
        }
    }

    if (cardCapabilities.FI >= 0 && cardCapabilities.FI < 16 &&
        cardCapabilities.DI >= 0 && cardCapabilities.DI < 16) {
        sci->ETUDATA = Fi[cardCapabilities.FI] / Di[cardCapabilities.DI];
        HAL_SCI_TRACE(0, "ETUDATA:%d", sci->ETUDATA);
    }

    cardCapabilities.T0.WT = cardCapabilities.T0.WI * 960 * Fi[cardCapabilities.FI] / CLK_CYCLES_PER_MS; // unit : milli second
    return 0;
}

void hal_sci_reset_card(void)
{
    // hal_cmu_sci_reset(true);
    // hal_sys_timer_delay(MS_TO_TICKS(100));
    hal_cmu_sci_reset(false);
}

void hal_sci_close(void)
{
    NVIC_DisableIRQ(SCI_IRQn);
    NVIC_ClearPendingIRQ(SCI_IRQn);
}

void hal_sci_vcc_start(void)
{
}

void hal_sci_vcc_stop(void)
{
}

uint32_t get_read_wt(uint32_t u32ReadLen) // UNIT: 1ms
{
    // 1 second timeout for ATR
    if (0 == cardCapabilities.T0.WT) {
        return 1000;
    }
    return cardCapabilities.T0.WT * u32ReadLen;
}

uint32_t hal_sci_get_response(uint8_t *buf, uint32_t rxlen)
{
    uint32_t retlen;

    hal_sci_irq_disable();

    if (rxdata.retlen >= rxlen) {
        memcpy(buf, rxdata.buf, rxlen);
        HAL_SCI_TRACE_NOCRLF(0, "sci response[%02x]:", rxlen);
        HAL_SCI_DUMP8("%02x ", buf, rxlen);
        hal_sci_init_rx_data(rxlen);
        hal_sci_irq_enable();
        return rxlen;
    }

    rxdata.rxlen = rxlen;
    hal_sci_irq_enable();

#ifdef RTOS
    uint32_t left_rx_len;
    uint32_t wt;

    left_rx_len = rxlen - rxdata.retlen;
    wt = get_read_wt(left_rx_len);
    HAL_SCI_TRACE(0, "rxlen %d left_rx_len %d wt %d", rxlen, left_rx_len, wt);
    osStatus_t stat = osSemaphoreAcquire(sem_sci_recv_done, wt);
    if (stat != osOK) {
        HAL_SCI_TRACE(0, "recv timeout");
        if (rxdata.retlen > 0) {
            memcpy(buf, rxdata.buf, rxdata.retlen);
        }
        retlen = rxdata.retlen;
    } else
#endif
    {
        retlen = rxlen;
        memcpy(buf, rxdata.buf, retlen);
    }

    HAL_SCI_TRACE_NOCRLF(0, "sci response[%02x]:", retlen);
    HAL_SCI_DUMP8("%02x ", buf, retlen);
    hal_sci_init_rx_data(retlen);
    return retlen;
}

uint8_t hal_sci_send_command(uint8_t *buf, uint8_t txlen)
{
    uint8_t len = 0;

    if (0 == txlen) {
        HAL_SCI_TRACE(0, "send length is 0");
        return txlen;
    }

    memcpy(txdata.buf, buf, txlen);
    txdata.txlen = txlen;
    txdata.leftlen = txlen;
    txdata.fifolen = SCI_TX_FIFO_LEN;

    HAL_SCI_TRACE_NOCRLF(0, "sci command [%02x]:", txlen);
    HAL_SCI_DUMP8("%02x ", buf, txlen);
    HAL_SCI_TRACE(0, "rxlen %d retlen %d", rxdata.rxlen, rxdata.retlen);

    if (txlen > SCI_TX_FIFO_LEN) {
        len = SCI_TX_FIFO_LEN;
    } else {
        len = txlen;
    }

    hal_sci_irq_disable();
    hal_sci_send_data(txdata.buf, len);
    txdata.leftlen -= len;
    hal_sci_irq_enable();

#ifdef RTOS
    osStatus_t stat = osSemaphoreAcquire(sem_sci_send_done, 500);
    if (stat != osOK) {
        TRACE(0, "%s wait sem ret %d", __func__, stat);
        return txlen - txdata.leftlen;
    }
#endif
    return txlen;
}

#endif
