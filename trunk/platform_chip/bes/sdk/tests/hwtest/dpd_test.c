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
#include "hwtest.h"
#include "hal_dma.h"
#include "hal_cmu.h"
#include "string.h"
#ifndef NOSTD
#include "math.h"
#endif

#ifdef CHIP_BEST1000

// DPD_SIMU
#define DPD_FRAME_SIZE              (4000)
#ifdef NOSTD
#define DPD_FRAME_NUM               2
#else
#define DPD_FRAME_NUM               1
#endif
#define DPD_BUF_ALIGN               __attribute__((aligned(0x100)))

static uint32_t DPD_BUF_ALIGN dpd_rx_buf[DPD_FRAME_NUM][DPD_FRAME_SIZE / 2];
static uint32_t DPD_BUF_ALIGN dpd_tx_buf[DPD_FRAME_NUM][DPD_FRAME_SIZE / 2];
static struct HAL_DMA_DESC_T dpd_rx_dma_desc[DPD_FRAME_NUM];
static struct HAL_DMA_DESC_T dpd_tx_dma_desc[DPD_FRAME_NUM];

static volatile bool dpd_rx_done = false;
static volatile bool dpd_tx_done = false;

#ifndef NOSTD

#define Len (DPD_FRAME_SIZE / 2)
#define K 3
#define Q 1

typedef struct{
    float real;
    float imag;
} complex;

static complex P[36];
static complex para[6];
static complex Rx[Len],Tx[Len];

static void initConvariance(complex p[])
{
  int i,j;
  for(i=0;i<6;i++)
  {
     for(j=0;j<6;j++)
     {
         if(i==j)
         {
           p[i*6+j].real = 0.001;
           p[i*6+j].imag = 0.001;
         }
         else
         {
           p[i*6+j].real = 0.0;
           p[i*6+j].imag = 0.0;
         }
     }
  }
}

static void initPara(complex *para)
{
   int i;
   for (i=0;i<6;i++)
   {
      para[i].real = 0.0;
      para[i].imag = 0.0;
    }
}

static float EnergyCalc(complex *Rx,int length)
{
  int i;
  float  sum=0.0;
  for (i=0;i<length;i++)
  {
    sum = sum + (Rx[i].real*Rx[i].real);
    sum = sum + (Rx[i].imag*Rx[i].imag);
  }
  sum = sum/length;
  return sqrt(sum);
}

static int DelayCalc(complex *Rx,complex *Tx,int length,int *type)
{
    int i,delay,ind1=0,ind2=0;
    float sumRcorr1,sumRcorr2,maxRcorr1=0,maxRcorr2=0;
    for (delay = 0;delay<200;delay++)
    {
        sumRcorr1=0.0;
        sumRcorr2=0.0;
        for (i=0;i<length;i++)
        {
            sumRcorr1 = sumRcorr1+Rx[i+delay].real*Tx[i].imag;
            sumRcorr2 = sumRcorr2-Rx[i+delay].imag*Tx[i].imag;
        }
       if(maxRcorr1<sumRcorr1)
       {
           maxRcorr1 = sumRcorr1;
           ind1 = delay;
       }
       if(maxRcorr2<sumRcorr2)
       {
           maxRcorr2 = sumRcorr2;
           ind2 = delay;
       }
    }
    if(maxRcorr1>maxRcorr2)
    {
        *type = 1;
        return ind1;
    }
    else
    {
        *type = 2;
        return ind2;
    }

}

static void ComplexMul(complex a,complex b,complex *c)
{
  c->real = a.real*b.real - a.imag*b.imag;
  c->imag = a.real*b.imag + a.imag*b.real;
}

static void MatrixMul( complex a[],complex b[],complex c[],int adim1,int adim2,int bdim2)
{
  int i,j,k;
  complex d,cum;
  for(k=0;k<bdim2;k++)
  {
    for (i=0;i<adim1;i++)
    {
      cum.real = 0.0;
      cum.imag = 0.0;
      for(j=0;j<adim2;j++)
      {
        ComplexMul(a[i*adim2+j],b[j*bdim2+k],&d);
        cum.real = cum.real + d.real;
        cum.imag = cum.imag + d.imag;
      }
      c[i*bdim2+k].real = cum.real;
      c[i*bdim2+k].imag = cum.imag;
    }
  }
}

static void ParaCalc(complex *Rx,complex *Tx,complex *para, int delay,int type)
{
 int n,k,q,i,j;
 float gain,mag,mag2;
 complex Phy[6],Res[6],PhyT[6];
 complex uu[3][2],Kn[6],tmp1[36];
 complex tmp[1],den[1],x,tmp2[1];
 float lamda = 0.9921875;
 float TxEn=EnergyCalc(Tx,2000);
 gain = EnergyCalc(Rx,2000)/TxEn;
 initConvariance(P);
 initPara(para);
 for (n = Q;n<Len-delay;n++)
 {
    for (k=0;k<K;k++)
    {
        for(q=0;q<=Q;q++)
        {
          if(type==1)
          {
            x.real = Rx[n-q+delay].imag/gain;
            x.imag = Rx[n-q+delay].real/gain;
          }
          else
          {
            x.real = Rx[n-q+delay].real/gain;
            x.imag = -Rx[n-q+delay].imag/gain;
          }
           mag = x.real*x.real+ x.imag*x.imag;
           mag = sqrt(mag);
           uu[k][q].real = x.real*pow(mag,(2*(k+1)-2));
           uu[k][q].imag = x.imag*pow(mag,(2*(k+1)-2));
         }
     }
    for(i =0;i<6;i++)
    {
       if(i<3)
       {
         Phy[i].real = uu[i][0].real;
         Phy[i].imag = uu[i][0].imag;
         PhyT[i].real = Phy[i].real;
         PhyT[i].imag = -Phy[i].imag;
       }
       else
       {
         Phy[i].real = uu[i-3][1].real;
         Phy[i].imag = uu[i-3][1].imag;
         PhyT[i].real = Phy[i].real;
         PhyT[i].imag = -Phy[i].imag;
       }
    }
    MatrixMul(Phy,P,Res,1,6,6);
    MatrixMul(Res,PhyT,tmp,1,6,1);
    den[0].real = lamda + tmp[0].real;
    den[0].imag = tmp[0].imag;
    mag2 = den[0].real*den[0].real + den[0].imag*den[0].imag;
    den[0].real = den[0].real/mag2;
    den[0].imag = -den[0].imag/mag2;
    MatrixMul(P,PhyT,Kn,6,6,1);
    MatrixMul(Kn,den,Kn,6,1,1);
    MatrixMul(Phy,para,tmp,1,6,1);
    tmp[0].real = Tx[n].real - tmp[0].real;
    tmp[0].imag = Tx[n].imag - tmp[0].imag;
    for (i=0;i<6;i++)
    {
     ComplexMul(Kn[i],tmp[0],&tmp2[0]);
     para[i].real += tmp2[0].real;
     para[i].imag += tmp2[0].imag;
    }
    MatrixMul(Kn,Res,tmp1,6,1,6);
    for (i=0;i<6;i++)
      for(j=0;j<6;j++)
      {
        P[i*6+j].real-= tmp1[i*6+j].real;
        P[i*6+j].imag-= tmp1[i*6+j].imag;
        P[i*6+j].real = P[i*6+j].real/lamda;
        P[i*6+j].imag = P[i*6+j].imag/lamda;
      }
   }
}

void start_calc(void)
{
    uint32_t time;
    int i,k,tmp,delay,type;
    short para_reg[12];

    TRACE(0,"Start to calc dpd pkt ...");
    time = hal_sys_timer_get();

    for (i=0;i<Len;i++)
    {
        Tx[i].real = (float)((short)dpd_tx_buf[0][i] >> 6)/512;
        Tx[i].imag = (float)((short)(dpd_tx_buf[0][i] >> 16) >> 6)/512;
        Rx[i].real = (float)((short)dpd_rx_buf[0][i] >> 2)/8192;
        Rx[i].imag = (float)((short)(dpd_rx_buf[0][i] >> 16) >> 2)/8192;
    }
    delay = DelayCalc(Rx,Tx,Len-200,&type);
    ParaCalc(Rx,Tx,para,delay,type);
    k = 0;
    for (i=0;i<6;i++)
    {
        tmp = (int)(512*para[i].real);
        if(tmp>32767)
            tmp = 32767;
        if(tmp< -32768)
            tmp = -32768;
        para_reg[k++] = tmp;
        tmp = (int)(512*para[i].imag);
        if(tmp>32767)
            tmp = 32767;
        if(tmp<-32768)
            tmp = -32768;
        para_reg[k++] = tmp;

        REG(0xD0330058 + i * 4) = ((uint16_t)para_reg[k - 1] << 16) | (uint16_t)para_reg[k - 2];
        //TRACE(3,"[%d] { %d, %d }", i, para_reg[k - 2], para_reg[k - 1]);
    }

    TRACE(1,"Calc finished: %u ms", TICKS_TO_MS(hal_sys_timer_get() - time));
    TRACE(6,"0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x",
        REG(0xD0330058 + 0 * 4),
        REG(0xD0330058 + 1 * 4),
        REG(0xD0330058 + 2 * 4),
        REG(0xD0330058 + 3 * 4),
        REG(0xD0330058 + 4 * 4),
        REG(0xD0330058 + 5 * 4));
}
#endif // !NOSTD

static void dpd_rx_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    ++cnt;
    dpd_rx_done = true;
    TRACE(3,"DPD-RX: remains=%d, error=%d, cnt=%d", remains, error, cnt);
}

static void dpd_tx_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    static int cnt = 0;

    ++cnt;
    dpd_tx_done = true;
    TRACE(3,"DPD-TX: remains=%d, error=%d, cnt=%d", remains, error, cnt);
}

void dpd_test(void)
{
    int i;
    volatile int k;
    uint32_t time;
    struct HAL_DMA_CH_CFG_T dma_cfg;

    hal_cmu_pll_enable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_AUD);

    hal_cmu_sys_set_freq(HAL_CMU_FREQ_26M);
    hal_cmu_sys_select_pll(HAL_CMU_PLL_AUD);
    hal_cmu_sys_set_freq(HAL_CMU_FREQ_104M);

    REG(0xd0350220) = (1 << 1);
    for (k = 0; k < 20; k++);
    REG(0xd0350224) = (1 << 1);

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_16;
    //dma_cfg.dst_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.handler = dpd_rx_handler;
    dma_cfg.src = 0; // useless
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_AUDMA_DPD_RX;
    dma_cfg.src_tsize = sizeof(dpd_rx_buf[0]) / 4;
    dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    for (i = 0; i < DPD_FRAME_NUM; i++) {
        dma_cfg.dst = (uint32_t)&dpd_rx_buf[i][0];
#if (DPD_FRAME_NUM > 2)
        hal_audma_init_desc(&dpd_rx_dma_desc[i], &dma_cfg, &dpd_rx_dma_desc[(i + 1) % DPD_FRAME_NUM], (i + 1) % (DPD_FRAME_NUM / 2) == 0);
#else
        hal_audma_init_desc(&dpd_rx_dma_desc[i], &dma_cfg, NULL, 1);
#endif
    }
    hal_audma_sg_start(&dpd_rx_dma_desc[0], &dma_cfg);

    dma_cfg.handler = dpd_tx_handler;
    dma_cfg.src_periph = HAL_AUDMA_DPD_TX;
    dma_cfg.src_tsize = sizeof(dpd_tx_buf[0]) / 4;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);
    for (i = 0; i < DPD_FRAME_NUM; i++) {
        dma_cfg.dst = (uint32_t)&dpd_tx_buf[i][0];
#if (DPD_FRAME_NUM > 2)
        hal_audma_init_desc(&dpd_tx_dma_desc[i], &dma_cfg, &dpd_tx_dma_desc[(i + 1) % DPD_FRAME_NUM], (i + 1) % (DPD_FRAME_NUM / 2) == 0);
#else
        hal_audma_init_desc(&dpd_tx_dma_desc[i], &dma_cfg, NULL, 1);
#endif
    }
    hal_audma_sg_start(&dpd_tx_dma_desc[0], &dma_cfg);

    REG(0xD0330038) |= (1 << 12);

    REG(DPD_RX_BASE + 0x30) = 1;
    REG(DPD_RX_BASE) = 3;
    REG(DPD_TX_BASE + 0x30) = 1;
    REG(DPD_TX_BASE) = 3;

    TRACE(0,"Start dpd rx/tx ...");
    time = hal_sys_timer_get();

    while (!dpd_rx_done || !dpd_tx_done);

    TRACE(1,"Recv first dpd rx/tx pkt: %u ms", TICKS_TO_MS(hal_sys_timer_get() - time));

#ifndef NOSTD
    REG(DPD_RX_BASE) = 0;
    REG(DPD_TX_BASE) = 0;
    start_calc();
#endif

    SAFE_PROGRAM_STOP();
}

#endif // CHIP_BEST1000
