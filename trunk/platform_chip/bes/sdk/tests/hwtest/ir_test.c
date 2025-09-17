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
 *  2021-02-22     tanwenchen    New document.
 ****************************************************************************/
#include "plat_addr_map.h"

#ifdef IRDA_BASE

#include "hwtest.h"
#include "hal_ir.h"
#include "string.h"

//#define IR_DEBUG
#define ENABLE_IR_RX_DATA_DUMP

enum HAL_IR_NEC_FABRIC_T {
    HAL_IR_NEC_START_PULSE = 0,
    HAL_IR_NEC_START_PAUSE,
    HAL_IR_NEC_BIT_0,
    HAL_IR_NEC_BIT_1,
    HAL_IR_NEC_STOP,
    HAL_IR_NEC_REPEAT,
};

struct HAL_IR_NEC_TIMING_T {
    uint32_t tx_clk;
    uint32_t start_bit_pulse_time_us;
    uint32_t start_bit_pause_time_us;
    uint32_t repeat_start_bit_pause_time_us;
    uint32_t pulse_time_us;
    uint32_t pause_1_time_us;
    uint32_t pause_0_time_us;
    uint32_t frame_repeat_pause_time_us;
    uint8_t enable_repeat;
};

static uint8_t resend_flag=0;
static enum HAL_IR_MODE_T rx_tx_mark;

#ifdef IR_DONOT_USE_DMA
static uint32_t pulse_0_len=0;
static uint32_t pause_0_len=0;
static uint32_t pulse_1_len=0;
static uint32_t pause_1_len=0;

void nec_protocol_tx_bit(const struct HAL_IR_NEC_TIMING_T *nec_timing_cfg, enum HAL_IR_NEC_FABRIC_T fabric)
{
    uint32_t pulse_len=0;
    uint32_t pause_len=0;

    switch(fabric)
    {
        case HAL_IR_NEC_START_PULSE:
        {
            pulse_len = (nec_timing_cfg->tx_clk * nec_timing_cfg->start_bit_pulse_time_us) / 1000000;
            pause_len=0;

            pulse_0_len = (nec_timing_cfg->tx_clk * nec_timing_cfg->pulse_time_us) / 1000000;
            pause_0_len = (nec_timing_cfg->tx_clk * nec_timing_cfg->pause_0_time_us) / 1000000;
            pulse_1_len = (nec_timing_cfg->tx_clk * nec_timing_cfg->pulse_time_us) / 1000000;
            pause_1_len = (nec_timing_cfg->tx_clk * nec_timing_cfg->pause_1_time_us) / 1000000;
#ifdef IR_DEBUG
            TRACE(0,"start pulse_len=%d",pulse_len);
            TRACE(0,"bit 0 pulse_0_len=%d",pulse_0_len);
            TRACE(0,"bit 0 pause_0_len=%d",pause_0_len);
            TRACE(0,"bit 1 pulse_1_len=%d",pulse_1_len);
            TRACE(0,"bit 1 pause_1_len=%d",pause_1_len);
#endif
            break;
        }
        case HAL_IR_NEC_START_PAUSE:
        {
            pulse_len=0;
            pause_len = (nec_timing_cfg->tx_clk * nec_timing_cfg->start_bit_pause_time_us) / 1000000;
#ifdef IR_DEBUG
            TRACE(0,"start pause_len=%d",pause_len);
#endif
            break;
        }
        case HAL_IR_NEC_BIT_0:
        case HAL_IR_NEC_STOP://0 or 1 is fine
        {
            pulse_len = (nec_timing_cfg->tx_clk * nec_timing_cfg->pulse_time_us) / 1000000;
            pause_len = (nec_timing_cfg->tx_clk * nec_timing_cfg->pause_0_time_us) / 1000000;
#ifdef IR_DEBUG
            TRACE(0,"bit 0 pulse_len=%d",pulse_len);
            TRACE(0,"bit 0 pause_len=%d",pause_len);
#endif
            break;
        }
        case HAL_IR_NEC_BIT_1:
        {
            pulse_len = (nec_timing_cfg->tx_clk * nec_timing_cfg->pulse_time_us) / 1000000;
            pause_len = (nec_timing_cfg->tx_clk * nec_timing_cfg->pause_1_time_us) / 1000000;
#ifdef IR_DEBUG
            TRACE(0,"bit 1 pulse_len=%d",pulse_len);
            TRACE(0,"bit 1 pause_len=%d",pause_len);
#endif
            break;
        }
        case HAL_IR_NEC_REPEAT:
        {
            //TODO :Not currently supported,tanwenchen,2021-02-24
            break;
        }
        default:
        {
            ASSERT(0, "%s:ir nec fabric error,line=%d",__func__,__LINE__);
            break;
        }
    }

    //tx pulse,Send carrier (high level)
    while(pulse_len)
    {
        if(pulse_len > 127)
        {
            hal_ir_tx_byte(0xFF);
            pulse_len -=127;
        }
        else
        {
            hal_ir_tx_byte(pulse_len|0x80);
            pulse_len =0;
        }
    }

    //tx pause,No carrier (low level)
    while(pause_len)
    {
        if(pause_len > 127)
        {
            hal_ir_tx_byte(0x7F);
            pause_len -=127;
        }
        else
        {
            hal_ir_tx_byte(pause_len);
            pause_len =0;
        }
    }
}

void nec_protocol_tx_0_1(bool bit)
{
    uint32_t pulse_len=0;
    uint32_t pause_len=0;

    if(bit == 0)
    {
        pulse_len = pulse_0_len;
        pause_len = pause_0_len;
    }
    else
    {
        pulse_len = pulse_1_len;
        pause_len = pause_1_len;
    }

    //tx pulseSend carrier (high level)
    while(pulse_len)
    {
        if(pulse_len > 127)
        {
            hal_ir_tx_byte(0xFF);
            pulse_len -=127;
        }
        else
        {
            hal_ir_tx_byte(pulse_len|0x80);
            pulse_len =0;
        }
    }

    //tx pause,No carrier (low level)
    while(pause_len)
    {
        if(pause_len > 127)
        {
            hal_ir_tx_byte(0x7F);
            pause_len -=127;
        }
        else
        {
            hal_ir_tx_byte(pause_len);
            pause_len =0;
        }
    }
}

void nec_protocol_tx_data(const uint8_t *data, uint32_t length)
{
    uint32_t i;
    uint8_t j;
    uint8_t temp;

    for(i=0;i<length;i++)
    {
        temp = data[i];
#ifdef IR_DEBUG
        TRACE(0,"---send data=0x%X",data[i]);
#endif
        for(j=0;j<8;j++)//Sending 1 byte takes up 16 bytes of fifo space
        {
            if(temp & 0x01)//NEC is LSB
            {
                nec_protocol_tx_0_1(1);
            }
            else
            {
                nec_protocol_tx_0_1(0);
            }
            temp >>=1;
        }
    }
}
#else

#define BUFFER_LEN_RX 512
static uint8_t buf_rx[BUFFER_LEN_RX];

#endif

//dma complete interrupt(fifo is almost empty or almost full)
void ir_dma_complete_handler(int error)
{
    //Do things after dma data is sent or received
}

//Peripheral data processing complete interrupt(The data has been completely sent or received from gpio)
void ir_peri_complete_handler(const uint8_t *buf, uint32_t len, bool rxtx_complete)
{
#ifdef IR_DONOT_USE_DMA
    if(rx_tx_mark == HAL_IR_MODE_TX)
    {
        #define TX_REC_DATA

#ifndef TX_REC_DATA
        #define DATA_LEN    4
#else
        #define DATA_LEN    96
#endif
        #define FIFO_LEN    2   //max is 2(send 1byte take up 16byte fifo), other values will not work,2021-02-25
#ifndef TX_REC_DATA
        static uint8_t step=0;
        static uint8_t data[DATA_LEN]={0x80,0x7F,0x02,0xFD};
#else
        static uint8_t step=2;
        static uint8_t data[DATA_LEN]=
        {
            //38K Sampling Rate
            0xFF,0xFF,0xD9,0x7F,0x2C,0x96,0x14,0x95,0x15,0x95,0x15,0x95,0x15,0x96,0x13,0x97,0x13,0x96,0x14,0x97,0x3F,0x95,0x40,0x97,
            0x3F,0x96,0x3F,0x96,0x40,0x97,0x3F,0x96,0x3F,0x96,0x40,0x95,0x14,0x97,0x13,0x96,0x40,0x95,0x15,0x95,0x15,0x95,0x14,0x96,
            0x14,0x96,0x14,0x96,0x14,0x97,0x3F,0x94,0x15,0x97,0x3F,0x96,0x40,0x95,0x40,0x96,0x40,0x95,0x40,0x97,0x3F,0x95,0x7F,0x7F,
            0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        };
#endif

        static struct HAL_IR_NEC_TIMING_T nec_timing_cfg =
        {
            .tx_clk                           = 38000,
            .start_bit_pulse_time_us          = 9000,//9ms
            .start_bit_pause_time_us          = 4500,//4.5ms
            .repeat_start_bit_pause_time_us   = 2250,//2.25ms
            .pulse_time_us                    = 560, //0.56ms
            .pause_1_time_us                  = 1690,//1.69ms
            .pause_0_time_us                  = 560, //0.56ms
            .frame_repeat_pause_time_us       = 98750,//110-9-2.25ms
            .enable_repeat                    = false,
        };

#ifdef IR_DEBUG
        TRACE(0,"---step=%d",step);
#endif

        if(step == 0)
        {
            nec_protocol_tx_bit(&nec_timing_cfg, HAL_IR_NEC_START_PULSE);
            step++;
        }
        else if(step == 1)
        {
            nec_protocol_tx_bit(&nec_timing_cfg, HAL_IR_NEC_START_PAUSE);
            step++;
        }
        else if(step == 2)
        {
            static uint16_t i=0;
#ifndef TX_REC_DATA
            if(DATA_LEN > FIFO_LEN)
            {
                nec_protocol_tx_data(&data[i],FIFO_LEN);
                i += FIFO_LEN;
            }
            else
            {
                nec_protocol_tx_data(&data[i],DATA_LEN-i);
                i += DATA_LEN-i;
            }

            if(i >= DATA_LEN)
            {
                i=0;
                step++;
            }
#else
            if(DATA_LEN > 32)
            {
                hal_ir_tx(&data[i],32);
                i +=32;
            }
            else
            {
                hal_ir_tx(&data[i],DATA_LEN-i);
                i +=DATA_LEN-i;
            }

            if(i >= DATA_LEN)
            {
                i=0;
                hal_ir_set_tx_done_mask();
                step=4;
            }
#endif
        }
        else if(step == 3)
        {
            hal_ir_set_tx_done_mask();
            nec_protocol_tx_bit(&nec_timing_cfg, HAL_IR_NEC_STOP);
            step++;
        }
        else if((step == 4)&&(rxtx_complete))
        {
#ifndef TX_REC_DATA
            step=0;
#else
            step=2;
#endif
            resend_flag=1;
            hal_ir_close();
            TRACE(0,"IR send ok...");
        }
    }
    else
    {
        uint8_t len;
        static uint32_t index=0;
        static uint8_t data[2048];

        hal_ir_get_rx_length(&len);
        hal_ir_rx(data+index,len);
#ifdef ENABLE_IR_RX_DATA_DUMP
        TRACE(0,"--len=%d,index=%d",len,index);
        DUMP8("%02X ",data+index,len);
        TRACE(TR_ATTR_NO_TS|TR_ATTR_NO_ID,"\r\n");
#endif
        index +=len;
        if(index > 2048)
        {
            index=0;
            TRACE(0,"------rx buffer flipped");
        }
        else if(rxtx_complete)
        {
            //Not needed
            //resend_flag=1;
            //hal_ir_close();
            TRACE(0,"------rx complete,total len=%d",index);
            index=0;
        }
    }
#else
    if(rx_tx_mark == HAL_IR_MODE_TX)
    {
        resend_flag=1;
        hal_ir_close();
        TRACE(0,"IR send ok...");
    }
    else
    {
        hal_ir_stop_dma_rx();
        TRACE(0,"------rx complete");
#ifdef ENABLE_IR_RX_DATA_DUMP
        uint16_t i;
        uint16_t integer = BUFFER_LEN_RX/64;
        uint16_t remainder = BUFFER_LEN_RX%64;

        TRACE(0,"--len=%d",BUFFER_LEN_RX);
        for(i=0;i<integer;i++)
        {
            DUMP8("%02X ",&buf_rx[i*64],64);
        }
        DUMP8("%02X ",&buf_rx[i*64],remainder);
        TRACE(TR_ATTR_NO_TS|TR_ATTR_NO_ID,"\r\n");
#endif
        hal_ir_dma_rx(&buf_rx[0], BUFFER_LEN_RX, ir_dma_complete_handler);
    }
#endif  //IR_DONOT_USE_DMA

}

void ir_test(void)
{
    struct HAL_IR_CFG_T cfg;

    TRACE(0,"IR test has started...");

    memset(&cfg, 0, sizeof(cfg));
    //common config
    cfg.ir_mode = HAL_IR_MODE_TX;
#ifdef IR_DONOT_USE_DMA
    cfg.use_dma = false;
#else
    cfg.use_dma = true;
#endif
    if(cfg.ir_mode == HAL_IR_MODE_TX)
    {
        cfg.auto_sleep_cnt = 0x80;//the number of clk
    }
    else
    {
        /*
        n*127*1/clk > Longest duration level time,But it can’t be too long,
        too long will result in failure to quickly identify to the end, and a
        large amount of end data will be generated,tanwenchen,2021-02-26
        */
        cfg.auto_sleep_cnt = 0x4;//4*127*26.3us=13.36ms>>9ms
    }
    cfg.clear_fifo = false;
    cfg.fifo_ae_level = 0x10;//tx use,Generally do not modify, if you modify, you need to change the dma related configuration
    cfg.fifo_af_level = 0x10;//rx use
    cfg.handler = ir_peri_complete_handler;

    //tx config
    cfg.tx_clk = 38*1000;//38k=26.3us
    cfg.tx_clk_duty = 25;
    cfg.tx_level = HAL_IR_TX_LEVEL_0_LOW_1_CARRIER;

    //rx config
    cfg.rx_clk = 38*1000;//100k=10us
    cfg.rec_period_single = 0;  //No need to
    cfg.rec_duty_single = 0;    //No need to
    cfg.rec_clk_cnt_idle = 0;   //No need to
    cfg.rx_default_level = HAL_IR_RX_DEFAULT_LEVEL_LOW;
    cfg.rx_invert = HAL_IR_RX_INVERT_ENABLE;                //When level flipping is enabled, the idle level must be low
    cfg.rx_edge = HAL_IR_RX_BOTH_CAPTURE_WITHOUT_MODULATION;//Note: The level needs to match with invert,2021-02-25

    rx_tx_mark = cfg.ir_mode;
    TRACE(0,"IR NEC protocol tx/rx testing------");

#ifndef IR_DONOT_USE_DMA
    #define BUFFER_LEN_TX 96
    static uint8_t buf_tx[BUFFER_LEN_TX]=
    {
        //38K Sampling Rate
        0xFF,0xFF,0xD9,0x7F,0x2C,0x96,0x14,0x95,0x15,0x95,0x15,0x95,0x15,0x96,0x13,0x97,0x13,0x96,0x14,0x97,0x3F,0x95,0x40,0x97,
        0x3F,0x96,0x3F,0x96,0x40,0x97,0x3F,0x96,0x3F,0x96,0x40,0x95,0x14,0x97,0x13,0x96,0x40,0x95,0x15,0x95,0x15,0x95,0x14,0x96,
        0x14,0x96,0x14,0x96,0x14,0x97,0x3F,0x94,0x15,0x97,0x3F,0x96,0x40,0x95,0x40,0x96,0x40,0x95,0x40,0x97,0x3F,0x95,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
    };
#endif

    hal_ir_open(&cfg);

#ifndef IR_DONOT_USE_DMA
    uint32_t start_ticks;
    uint32_t stop_ticks;

    if(rx_tx_mark == HAL_IR_MODE_TX)
    {
        start_ticks = hal_fast_sys_timer_get();
        hal_ir_dma_tx(buf_tx, BUFFER_LEN_TX, ir_dma_complete_handler);
        stop_ticks = hal_fast_sys_timer_get();
        TRACE(0,"------ir dma tx time=%dus",FAST_TICKS_TO_US(stop_ticks-start_ticks));
    }
    else
    {
        start_ticks = hal_fast_sys_timer_get();
        hal_ir_dma_rx(&buf_rx[0], BUFFER_LEN_RX, ir_dma_complete_handler);
        stop_ticks = hal_fast_sys_timer_get();
        TRACE(0,"------ir dma rx time=%dus",FAST_TICKS_TO_US(stop_ticks-start_ticks));
    }
#endif
    while(1)
    {
        if(resend_flag)
        {
            resend_flag=0;
            hal_sys_timer_delay(MS_TO_TICKS(1000));
            hal_ir_open(&cfg);
#ifndef IR_DONOT_USE_DMA
            if(rx_tx_mark == HAL_IR_MODE_TX)
            {
                start_ticks = hal_fast_sys_timer_get();
                hal_ir_dma_tx(buf_tx, BUFFER_LEN_TX, ir_dma_complete_handler);
                stop_ticks = hal_fast_sys_timer_get();
                TRACE(0,"------------ir dma tx time=%dus",FAST_TICKS_TO_US(stop_ticks-start_ticks));
            }
#endif
        }
        hal_sys_timer_delay(MS_TO_TICKS(50));
    }
}

#endif

