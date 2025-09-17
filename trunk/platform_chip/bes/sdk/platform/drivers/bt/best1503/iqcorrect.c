#ifndef NOSTD
#if defined (TX_IQ_CAL) || defined (RX_IQ_CAL)
#include <math.h>
#include <stdio.h>

#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_trace.h"

#include "string.h"
#include "besbt_string.h"
#include "bt_drv.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_btdump.h"
#include "iqcorrect.h"
#include "bt_drv_internal.h"
#include "bt_drv_1503_internal.h"
#include "bt_1503_reg_map.h"
#include CHIP_SPECIFIC_HDR(bt_drv_modem_reg_map)

#define FFTSIZE 1024

#define IQ_CALIB
#define DC_CALIB
//#define IQ_DEBUG
//#define DC_DEBUG
#define DC_ENERGY_THD 10
#define DC_RANGE_MIN -200
#define DC_RANGE_MAX 200
#define IQ_ENERGY_THD 10
#define IQ_RANGE_MIN -200
#define IQ_RANGE_MAX 200

#define SWEEP_STEP 50

static float get_DC_energy(uint32_t addr, short dc_i, short dc_q,int num)
{
    float E_ave = 0;
    for(int cc = 0; cc < num; cc++) {
        int32_t i_data, q_data = 0;

        BTDIGITAL_REG_SET_FIELD(addr, 0x3FFF, 0,  (dc_i & 0x3FFF));
        BTDIGITAL_REG_SET_FIELD(addr, 0x3FFF, 14, (dc_q & 0x3FFF));

        BTDIGITAL_REG_WR(BT_BES_TESTMODE_ADDR, 0x800A0000);

        hal_sys_timer_delay(US_TO_TICKS(100));

        BTDIGITAL_REG_SET_FIELD(BESMDM_MIX_MODE_CONFIG_ADDR, 0x1, 30, 0);    //reset start mix
        hal_sys_timer_delay(US_TO_TICKS(100));
        BTDIGITAL_REG_SET_FIELD(BESMDM_MIX_MODE_CONFIG_ADDR, 0x1, 30, 1);    //start mix
        hal_sys_timer_delay(US_TO_TICKS(150));
        i_data = BTDIGITAL_REG(BESMDM_ONE_TONE_ENERGY_GET_I_ADDR) & 0x3FFFFF;
        q_data = BTDIGITAL_REG(BESMDM_ONE_TONE_ENERGY_GET_Q_ADDR) & 0x3FFFFF;

        if(0x200000 <= i_data)
            i_data -= 0x400000;
        if(0x200000 <= q_data)
            q_data -= 0x400000;
        i_data = i_data/8;
        q_data = q_data/8;
        float E = sqrtf(i_data*i_data + q_data*q_data);

#ifdef IQ_DEBUG
        BT_DRV_TRACE(3,"geti=%d,q=%d,e_tmp=%d",dc_i,dc_q,(int)E);
#endif
        E_ave += E;
    }
    E_ave = E_ave / num;
    BTDIGITAL_REG_WR(BT_BES_TESTMODE_ADDR, 0);
    return E_ave;
}

static void set_gain_phase(uint32_t addr, short gain, short phase)
{
    BTDIGITAL_REG_SET_FIELD(BESMDM_IQMCNTL_ADDR_CT_ADDR, 0x1, 31, 0);


    BTDIGITAL_REG_SET_FIELD(addr, 0x3FFF, 0,  (gain & 0x3FFF));
    BTDIGITAL_REG_SET_FIELD(addr, 0x3FFF, 16, (phase & 0x3FFF));
}

static float get_iq_energy(uint32_t addr, short gain,short phase,int num, uint32_t freq_addr)
{
    float E_ave = 0;
    for(int cc = 0; cc < num; cc++) {
        int32_t i_data, q_data = 0;

        BTDIGITAL_REG_WR(CMU_REG_58_ADDR, 0x00800030);
        BTDIGITAL_REG_WR(CMU_REG_60_ADDR, 0xFFFFFFFF);       //rst

        set_gain_phase(addr, gain, phase);
        hal_sys_timer_delay(US_TO_TICKS(1));

        BTDIGITAL_REG_WR(CMU_REG_64_ADDR, 0xFFFFFFFF);       //clr rst
        BTDIGITAL_REG_SET_FIELD(BESMDM_IQMCNTL_ADDR_CT_ADDR, 0x1, 31, 1);

        BTDIGITAL_REG_WR(BT_BES_TESTMODE_ADDR, freq_addr);

        hal_sys_timer_delay(US_TO_TICKS(100));

        BTDIGITAL_REG_SET_FIELD(BESMDM_MIX_MODE_CONFIG_ADDR, 0x1, 30, 0);    //reset start mix
        hal_sys_timer_delay(US_TO_TICKS(100));
        BTDIGITAL_REG_SET_FIELD(BESMDM_MIX_MODE_CONFIG_ADDR, 0x1, 30, 1);    //start mix
        hal_sys_timer_delay(US_TO_TICKS(150));

        i_data = BTDIGITAL_REG(BESMDM_ONE_TONE_ENERGY_GET_I_ADDR) & 0x3FFFFF;
        q_data = BTDIGITAL_REG(BESMDM_ONE_TONE_ENERGY_GET_Q_ADDR) & 0x3FFFFF;
        //BT_DRV_TRACE(2,"i=0x%x,q=0x%x",i_data,q_data);
        if(0x200000 <= i_data)
            i_data -= 0x400000;
        if(0x200000 <= q_data)
            q_data -= 0x400000;
        i_data = i_data/8;
        q_data = q_data/8;
        float E = sqrtf(i_data*i_data + q_data*q_data);

#ifdef IQ_DEBUG
        BT_DRV_TRACE(3,"n=%d,g=%d,p=%d,e=%d",cc,gain,phase,(int)E);
#endif
        E_ave += E;
    }
    E_ave = E_ave / num;
    BTDIGITAL_REG_WR(BT_BES_TESTMODE_ADDR, 0);
    return E_ave;
}


static int get_best_i(int min_i,float min_energy,int base_q,int step,int* im, float* em)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t e_addr = BESMDM_IQMCNTL_ADDR_CT_ADDR;
    int i_left = 0;
    int i_right = 0;
    int i_middle = 0;
    int base_i = 0;
    i_left = min_i - step;
    energy_left = get_DC_energy(e_addr, i_left, base_q, 1);
    i_right = min_i + step;
    energy_right = get_DC_energy(e_addr, i_right, base_q, 1);
    while(1) {
        i_middle = (i_left + i_right) / 2;
        if(ABS(i_left - i_right) < 17) {
            energy_middle = get_DC_energy(e_addr, i_middle, base_q, 3);

        } else {
            energy_middle = get_DC_energy(e_addr, i_middle, base_q, 2);
        }
#ifdef DC_DEBUG
        BT_DRV_TRACE(4,"Mi=%d,q=%d,e=%d,DC_Emin=%d",i_middle,base_q,(int)energy_middle,(int)*em);
        BT_DRV_TRACE(4,"Li=%d,ri=%d,le=%d,re=%d",i_left,i_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right) {
            i_left = i_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_i = i_right;
        } else {
            i_right = i_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_i = i_left;
        }
        if(min_energy < *em) {
            *im = min_i;
            *em = min_energy;
        }
#ifdef DC_DEBUG
        //BT_DRV_TRACE(4,"DICHOTOMY_best_i=%d,best_q=%d,energy_min=%d,DC_Emin=%d",min_i,base_q,(int)min_energy,(int)*em);
#endif
        if(ABS(i_left - i_right) < 2)
            break;
    }

    if(energy_left > energy_right) {
        min_energy = energy_right;
        min_i = i_right;
    } else {
        min_energy = energy_left;
        min_i = i_left;
    }
    if(min_energy < *em) {
        *im = min_i;
        *em = min_energy;
    }
#ifdef DC_DEBUG
    BT_DRV_TRACE(4,"LAST_i=%d,q=%d,e=%d,DC_Emin=%d",min_i,base_q,(int)min_energy,(int)*em);
#endif
    if(min_i != *im) {
        *em = get_DC_energy(e_addr, *im, base_q, 3);
        if(*em > min_energy) {
            *im = min_i;
            *em = min_energy;
        }
    }
    base_i = *im;
    return base_i;
}

static int get_best_q(int min_q,float min_energy,int base_i,int step,int* qm, float* em)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t e_addr = BESMDM_IQMCNTL_ADDR_CT_ADDR;
    int q_left = 0;
    int q_right = 0;
    int q_middle = 0;
    int base_q = 0;
    q_left = min_q - step;
    energy_left = get_DC_energy(e_addr, base_i, q_left, 1);
    q_right = min_q + step;
    energy_right = get_DC_energy(e_addr, base_i, q_right, 1);

    while(1) {
        q_middle = (q_left+q_right) / 2;
        if(ABS(q_left - q_right) < 17) {
            energy_middle = get_DC_energy(e_addr, base_i, q_middle, 3);
        } else {
            energy_middle = get_DC_energy(e_addr, base_i, q_middle, 2);
        }
#ifdef DC_DEBUG
        BT_DRV_TRACE(4,"Mi=%d,q=%d,e=%d,DC_Emin=%d",base_i,q_middle,(int)energy_middle,(int)*em);
#endif
#ifdef DC_DEBUG
        BT_DRV_TRACE(4,"Lq=%d,rq=%d,le=%d,re=%d",q_left,q_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right) {
            q_left = q_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_q = q_right;
        } else {
            q_right = q_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_q = q_left;
        }
        if(min_energy < *em) {
            *qm = min_q;
            *em = min_energy;
        }
#ifdef DC_DEBUG
        //BT_DRV_TRACE(4,"DICHOTOMY_best_i=%d,best_q=%d,energy_min=%d,DC_Emin=%d",base_i,min_q,(int)min_energy,(int)*em);
#endif
        if(ABS(q_left - q_right) < 2)
            break;
    }

    if(energy_left > energy_right) {
        min_energy = energy_right;
        min_q = q_right;
    } else {
        min_energy = energy_left;
        min_q = q_left;
    }
    if(min_energy < *em) {
        *qm = min_q;
        *em = min_energy;
    }
#ifdef DC_DEBUG
    BT_DRV_TRACE(4,"LAST_i=%d,q=%d,e=%d,DC_Emin=%d",base_i,min_q,(int)min_energy,(int)*em);
#endif
    if(min_q != *qm) {
        *em = get_DC_energy(e_addr, base_i, *qm, 3);
        if(*em > min_energy) {
            *qm = min_q;
            *em = min_energy;
        }
    }
    base_q = *qm;
    return base_q;
}

static int get_best_gain(uint32_t addr,int min_gain,float min_energy,int base_phase,int step,int* gm, float* em, uint32_t freq_addr)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t en_addr = addr;
    int gain_left = 0;
    int gain_right = 0;
    int gain_middle = 0;
    int base_gain = 0;
    gain_left = min_gain - step;
    energy_left = get_iq_energy(en_addr, gain_left, base_phase, 2, freq_addr);
    gain_right = min_gain + step;
    energy_right = get_iq_energy(en_addr, gain_right, base_phase, 2, freq_addr);
    while(1) {
        gain_middle = (gain_left+gain_right) / 2;
        if(ABS(gain_left - gain_right) < 8) {
            energy_middle = get_iq_energy(en_addr, gain_middle, base_phase, 4, freq_addr);
        } else {
            energy_middle = get_iq_energy(en_addr, gain_middle, base_phase, 2, freq_addr);
        }
#ifdef IQ_DEBUG
        BT_DRV_TRACE(4,"Mg=%d,p=%d,e=%d,IQ_Emin=%d",gain_middle,base_phase,(int)energy_middle,(int)*em);
        BT_DRV_TRACE(4,"Lg=%d,rg=%d,le=%d,re=%d",gain_left,gain_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right) {
            gain_left = gain_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_gain = gain_right;
        } else {
            gain_right = gain_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_gain = gain_left;
        }
        if(min_energy < *em) {
            *gm = min_gain;
            *em = min_energy;
        }

#ifdef IQ_DEBUG
        //BT_DRV_TRACE(4,"DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",min_gain,base_phase,(int)min_energy,(int)*em);
#endif
        if(ABS(gain_left-gain_right) < 2)
            break;
    }

    if(energy_left > energy_right) {
        min_energy = energy_right;
        min_gain = gain_right;
    } else {
        min_energy = energy_left;
        min_gain = gain_left;
    }
    if(min_energy < *em) {
        *gm = min_gain;
        *em = min_energy;
    }
#ifdef IQ_DEBUG
    //BT_DRV_TRACE(4,"LAST_DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",min_gain,base_phase,(int)min_energy,(int)*em);
#endif
    if(min_gain != *gm) {
        *em = get_DC_energy(en_addr, *gm, base_phase, 3);
        if(*em > min_energy) {
            *gm = min_gain;
            *em = min_energy;
        }
    }
    base_gain = *gm;
    return base_gain;
}

static int get_best_phase(uint32_t addr,int min_phase,float min_energy,int base_gain,int step,int* pm, float* em, uint32_t freq_addr)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t en_addr = addr;
    int phase_left = 0;
    int phase_right = 0;
    int phase_middle = 0;
    int base_phase = 0;
    phase_left = min_phase - step;
    energy_left = get_iq_energy(en_addr, base_gain, phase_left, 2, freq_addr);
    phase_right = min_phase + step;
    energy_right = get_iq_energy(en_addr, base_gain, phase_right, 2, freq_addr);
    while(1) {
        phase_middle = (phase_left + phase_right) / 2;
        if(ABS(phase_left - phase_right) < 8) {
            energy_middle = get_iq_energy(en_addr,base_gain,phase_middle,4, freq_addr);
        } else {
            energy_middle = get_iq_energy(en_addr,base_gain,phase_middle,2, freq_addr);
        }
#ifdef IQ_DEBUG
        BT_DRV_TRACE(4,"Mg=%d,p=%d,e=%d,IQ_Emin=%d",base_gain,phase_middle,(int)energy_middle,(int)*em);
        BT_DRV_TRACE(4,"Lp=%d,rp=%d,le=%d,re=%d",phase_left,phase_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right) {
            phase_left = phase_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_phase = phase_right;
        } else {
            phase_right = phase_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_phase = phase_left;
        }
        if(min_energy < *em) {
            *pm = min_phase;
            *em = min_energy;
        }
#ifdef IQ_DEBUG
        //BT_DRV_TRACE(4,"DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",base_gain,min_phase,(int)min_energy,(int)*em);
#endif
        if(ABS(phase_left - phase_right) < 2)
            break;
    }

    if(energy_left > energy_right) {
        min_energy = energy_right;
        min_phase = phase_right;
    } else {
        min_energy = energy_left;
        min_phase = phase_left;
    }
    if(min_energy < *em) {
        *pm = min_phase;
        *em = min_energy;
    }
#ifdef IQ_DEBUG
    //BT_DRV_TRACE(4,"LAST_DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",base_gain,min_phase,(int)min_energy,(int)*em);
#endif
    if(min_phase != *pm) {
        *em = get_DC_energy(en_addr, base_gain, *pm, 3);
        if(*em > min_energy) {
            *pm = min_phase;
            *em = min_energy;
        }
    }
    base_phase = *pm;
    return base_phase;
}

void dc_iq_calib_1503(void)
{
    uint32_t time_start = hal_sys_timer_get();
    BTDIGITAL_REG_WR(BESMDM_ONE_TONE_SET_GAIN_ADDR, 0x02200012);	//one tone set gain
    BTDIGITAL_REG_WR(BESMDM_ONE_TONE_SET_FREQ_ADDR, 0x7);	        //187.5k tone	bit7:0 0x3
#ifdef DC_CALIB
    int DC_i_min = 0;
    int DC_q_min = 0;
    uint32_t e_addr = BESMDM_IQMCNTL_ADDR_CT_ADDR;
    float DC_energy_min = 1000000;

    int base_i = 0;
    int base_q = 0;
    int min_i = 0;
    int min_q = 0;
    int step = SWEEP_STEP;
    float min_energy = 1000000;

    for(int q1=DC_RANGE_MIN; q1<=DC_RANGE_MAX; q1+=step) {
        float e_tmp=get_DC_energy(e_addr,base_i,q1,1);
        if(min_energy > e_tmp) {
            min_q = q1;
            min_energy = e_tmp;
            DC_q_min = min_q;
            DC_energy_min = min_energy;
        }
#ifdef DC_DEBUG
    //BT_DRV_TRACE(4,"i=%d,q=%d,e_tmp=%d,DC_Emin=%d",base_i,q1,(int)e_tmp,(int)DC_energy_min);
#endif
    }
#ifdef DC_DEBUG
    BT_DRV_TRACE(4,"--------------SWEEP q=%d,e_tmp=%d--------------",min_q,(int)min_energy);
#endif

    base_q=get_best_q(min_q,min_energy,base_i,step,&DC_q_min,&DC_energy_min);

#ifdef DC_DEBUG
    BT_DRV_TRACE(2,"Step1:best_q=%d,energy_min=%d",base_q,(int)DC_energy_min);
#endif

    if(DC_ENERGY_THD < DC_energy_min) {
        min_energy = 1000000;
        for(int i1=DC_RANGE_MIN; i1<=DC_RANGE_MAX; i1+=step){
            float e_tmp=get_DC_energy(e_addr,i1,base_q,1);
            if(min_energy > e_tmp) {
                min_i = i1;
                min_energy = e_tmp;
                DC_i_min = min_i;
                DC_energy_min = min_energy;
            }
#ifdef DC_DEBUG
            //BT_DRV_TRACE(4,"i=%d,q=%d,e_tmp=%d,DC_Emin=%d",i1,base_q,(int)e_tmp,(int)DC_energy_min);
#endif
        }
#ifdef DC_DEBUG
        BT_DRV_TRACE(4,"--------------SWEEP i=%d,e_tmp=%d--------------",min_i,(int)min_energy);
#endif
        base_i=get_best_i(min_i,min_energy,base_q,step,&DC_i_min,&DC_energy_min);
#ifdef DC_DEBUG
        BT_DRV_TRACE(3,"Step2:abest_i=%d,best_q=%d,energy_min=%d",base_i,base_q,(int)DC_energy_min);
#endif
        if(DC_ENERGY_THD < DC_energy_min) {
            base_q=get_best_q(min_q,min_energy,base_i,step,&DC_q_min,&DC_energy_min);
#ifdef DC_DEBUG
            BT_DRV_TRACE(3,"Step3:abest_i=%d,best_q=%d,energy_min=%d",base_i,base_q,(int)DC_energy_min);
#endif
        }
    }
#endif

    BT_DRV_TRACE(3,"FINAL DC best_i=%d,best_q=%d", base_i,base_q);
    float teste=get_DC_energy(e_addr,base_i,base_q,1);

    BT_DRV_TRACE(3,"FINAL limit DC best_i=%d,best_q=%d,e=%d,energy_min=%d", base_i,base_q,(int)teste,(int)DC_energy_min);

    //IQ
    BTDIGITAL_REG_WR(BESMDM_ONE_TONE_SET_FREQ_ADDR, 0x3);          //160K
#ifdef IQ_CALIB
    uint32_t en_addr_base = BESMDM_INT_TX_IQCOMP_BASE;
    uint32_t en_addr = 0x0;
    for (int k = 0; k<3; k++) {
        uint32_t freq_add = 0x800A0000;
        if(k==0) {
            freq_add += 13;
            en_addr = en_addr_base + 13 * 4;
        } else if(k==1) {
            freq_add += 39;
            en_addr = en_addr_base + 39 * 4;
        } else {
            freq_add += 65;
            en_addr = en_addr_base + 65 * 4;
        }
        int iq_gain_min = 0;
        int iq_phase_min = 0;
        float iq_energy_min = 1000000;
        int base_phase = 0;
        int base_gain = 0;
        int min_phase = 0;
        int min_gain = 0;
        step = SWEEP_STEP;
        min_energy = 1000000;
        for(int gain1=IQ_RANGE_MIN; gain1<=IQ_RANGE_MAX; gain1+=step) {
            float e_tmp=get_iq_energy(en_addr,gain1,base_phase,1, freq_add);
            if(min_energy > e_tmp) {
                min_gain = gain1;
                min_energy = e_tmp;
                iq_gain_min = min_gain;
                iq_energy_min = min_energy;
            }
#ifdef IQ_DEBUG
        //BT_DRV_TRACE(4,"gain=%d,phase=%d,e_tmp=%d,IQ_Emin=%d",gain1,base_phase,(int)e_tmp,(int)iq_energy_min);
#endif
        }
#ifdef IQ_DEBUG
        BT_DRV_TRACE(4,"--------------SWEEP gain=%d,e_tmp=%d--------------",min_gain,(int)min_energy);
#endif
        base_gain=get_best_gain(en_addr,min_gain,min_energy,base_phase,step,&iq_gain_min,&iq_energy_min, freq_add);
#ifdef IQ_DEBUG
        BT_DRV_TRACE(2,"Step1:best_gain=%d,energy_min=%d",base_gain,(int)iq_energy_min);
#endif

        if(IQ_ENERGY_THD < iq_energy_min) {
            min_energy = 1000000;
            for(int phase1=IQ_RANGE_MIN; phase1<=IQ_RANGE_MAX; phase1+=step) {
                float e_tmp=get_iq_energy(en_addr,base_gain,phase1,1, freq_add);
                if(min_energy > e_tmp) {
                    min_phase = phase1;
                    min_energy = e_tmp;
                    iq_phase_min = min_phase;
                    iq_energy_min = min_energy;
                }
#ifdef IQ_DEBUG
            //BT_DRV_TRACE(4,"gain=%d,phase=%d,e_tmp=%d,IQ_Emin=%d",base_gain,phase1,(int)e_tmp,(int)iq_energy_min);
#endif
            }
#ifdef IQ_DEBUG
            BT_DRV_TRACE(4,"--------------SWEEP phase=%d,e_tmp=%d--------------",min_phase,(int)min_energy);
#endif
            base_phase=get_best_phase(en_addr,min_phase,min_energy,base_gain,step,&iq_phase_min,&iq_energy_min, freq_add);
#ifdef IQ_DEBUG
            BT_DRV_TRACE(3,"Step2:best_gain=%d,best_phase=%d,energy_min=%d",base_gain,base_phase,(int)iq_energy_min);
#endif

            if(IQ_ENERGY_THD < iq_energy_min) {
                base_gain=get_best_gain(en_addr,min_gain,min_energy,base_phase,step,&iq_gain_min,&iq_energy_min, freq_add);
#ifdef IQ_DEBUG
                BT_DRV_TRACE(3,"Step3:best_gain=%d,best_phase=%d,energy_min=%d",base_gain,base_phase,(int)iq_energy_min);
#endif
            }
        }

        float testen=get_iq_energy(en_addr,base_gain,base_phase,1, freq_add);
        BT_DRV_TRACE(3,"FINAL %d,best_g=%d,best_p=%d,e=%d,energy_min=%d", k,base_gain,base_phase,(int)testen,(int)iq_energy_min);

        int ch1 = 0;
        int ch2 = 0;
        if(k==0) {
            ch1= 0;
            ch2= 25;
        } else if(k==1) {
            ch1= 26;
            ch2= 51;
        } else {
            ch1= 52;
            ch2= 78;
        }
        for (int ch = ch1; ch <= ch2; ch++) {
            uint32_t addre=en_addr_base+ch*4;
            set_gain_phase(addre,base_gain,base_phase);
        }
#ifdef IQ_DEBUG
        BT_DRV_TRACE(3,"aaaaaaa--FINAL %d,best_g=%d,best_p=%d--aaaaaaa", k,base_gain,base_phase);
#endif
    }
#endif
    //common RF register init
    BTDIGITAL_REG_SET_FIELD(BESMDM_IQMCNTL_ADDR_CT_ADDR, 0x1, 31, 1);
    BT_DRV_TRACE(1,"use time: %d ms", __TICKS_TO_MS(hal_sys_timer_get()-time_start));
}

#define RX_AGC_IDX      7

void rx_dc_cal_1503(void)
{
    uint16_t calib_flag = 0;
    uint16_t i_value = 0;
    uint16_t q_value = 0;
    uint16_t i2v_dciq_addr_base = 0;
    uint32_t time_start = hal_sys_timer_get();

#ifdef __HW_AGC__
    i2v_dciq_addr_base = 0xD5;
#else
    i2v_dciq_addr_base = 0x31;
#endif

    for (uint8_t i = 0; i <= RX_AGC_IDX; i++) {
#ifdef __HW_AGC__
        // dr rx gain
        BTRF_REG_SET_FIELD(0x309, 0x1, 9, 1);
        // dr rx gain idx
        BTRF_REG_SET_FIELD(0x309, 0x7, 10, i);
#else
        // dr rx gain
        BTRF_REG_SET_FIELD(0x24, 0x1, 0, 1);
        // dr rx gain idx
        BTRF_REG_SET_FIELD(0x24, 0x7, 1, i);
#endif
        // rx on
        BTDIGITAL_REG_WR(0xD0220c00, 0x000A0080);
        btdrv_delay(1);

        BTRF_REG_SET_FIELD(0x2A, 0x1, 8, 0);
        btdrv_delay(1);
        BTRF_REG_SET_FIELD(0x2A, 0x1, 8, 1);
        btdrv_delay(1);

        BTRF_REG_GET_FIELD(0x55, 0x1, 12, calib_flag);

        if (calib_flag) {
            BTRF_REG_GET_FIELD(0x55, 0x3F, 0, q_value);
            BTRF_REG_GET_FIELD(0x55, 0x3F, 6, i_value);
            BT_DRV_TRACE(0,"%s, agc gain%d, i2v_dc_i:0x%x, i2v_dc_q:0x%x", __func__, i, i_value, q_value);
        } else {
            // use default value
            q_value = 0x20;
            i_value = 0x20;
            BT_DRV_TRACE(0,"%s, agc gain%d, use default value:0x%x", __func__, i, q_value);
        }

        BTRF_REG_SET_FIELD(i2v_dciq_addr_base + i, 0x3F, 0, i_value);
        BTRF_REG_SET_FIELD(i2v_dciq_addr_base + i, 0x3F, 6, q_value);

        // rx off
        BTDIGITAL_REG_WR(0xD0220c00, 0x0);

        //dccal_dac_en
        BTRF_REG_SET_FIELD(0x409, 0x3, 6, 3);
    }
    BT_DRV_TRACE(1,"use time: %d ms", __TICKS_TO_MS(hal_sys_timer_get()-time_start));
}
#endif
#endif

