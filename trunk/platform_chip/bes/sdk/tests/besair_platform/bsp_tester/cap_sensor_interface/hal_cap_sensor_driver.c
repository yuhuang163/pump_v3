#include "string.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "plat_types.h"
#include "cmsis.h"
#include "hal_location.h"
#include "hal_spi.h"
#include "hal_trace.h"
#include "pmu.h"
#include "hal_psc.h"
#include "hal_capsenseif.h"
#include "hal_cap_sensor_driver.h"

void pmu_capsense_vref_enable(void)
{
    uint16_t val;

    pmu_write(EXTPMU_REG(0x6C), 0x8000);
    pmu_read(EXTPMU_REG(0x6C), &val);
    TRACE(0 , "val = %x", val);

    pmu_write(EXTPMU_REG(0x40), 0xF014);
    pmu_read(EXTPMU_REG(0x40), &val);
    TRACE(0 , "val = %x", val);

    pmu_write(EXTPMU_REG(0x3F), 0x108A);
    pmu_read(EXTPMU_REG(0x3F), &val);
    TRACE(0 , "val = %x", val);

#if 1
    //pmu_write(EXTPMU_REG(0x7A), 0xDF1C);
    pmu_write(EXTPMU_REG(0x7A), 0xD31C); //rc
#else
    hal_cmu_dcdc_clock_enable(HAL_CMU_DCDC_CLOCK_USER_GPADC);
    pmu_write(EXTPMU_REG(0x7A), 0xDF33); //osc 24M
#endif
    pmu_read(EXTPMU_REG(0x7A), &val);
    TRACE(0 , "val = %x", val);
    NVIC_DisableIRQ(PMU_IRQn);
}

const uint16_t capsense_init_tbl_1[][3] =
{
    {0x55,0x0001,1},
    {0x56,0x0002,1},
    {0x57,0x0003,1},
    {0x58,0x0004,1},
    {0x59,0x0005,1},
    {0x5a,0x0005,1},
    {0x5b,0x0006,1},
    {0x5c,0x0007,1},
    {0x02,0xc240,1},    //ch number [6~4]    0:1ch  ~ 7:8ch   bit7~8:operation mode(c300) (00:sar-per sdm-per(c200),10:sar-once sdm-per(c300))
    {0x4f,0x28cf,1},    //dither_byp
    {0x3c,0x000f,1},
    {0x45,0x07c0,1},
    {0x46,0xfd26,1},
    {0x47,0x036c,1},
    {0x48,0x22d1,1},
    {0x49,0x3939,1},
    {0x4a,0x22d1,1},
    {0x4b,0x036c,1},
    {0x4c,0xfd26,1},
    //{0x50,0x004F,1},
    {0x4f,0x28cf,1},
    //{0x02,0xc670,1},
    {0x3e,0x12ff,1},
    {0x80,0x0001,1},  //fifo target en --ch message
    {0x7f,0x0902,1},  //  11bits-[1] low power(average)(0102) ,[0] original put  (0902)
    {0x04,0x0808,1},  //  sdm sample_timer   //8~15:ch0   0-7:ch1
    {0x05,0x0808,1},  //  sdm sample_timer   //8~15:ch2   0-7:ch3
    {0x06,0x0808,1},  //  sdm sample_timer   //8~15:ch4   0-7:ch5
    {0x07,0x0808,1},  //  sdm sample_timer   //8~15:ch6   0-7:ch7
    {0x01,0xf000,1},
    {0x01,0xf001,1},

};

void count_sar_value(struct cap_sense_sample **sample_value)
{
    double sar_value = 0;
    int sar_int_zheng_value;
    int sar_int_xiao_value;

    sar_value += (double)((((*(sample_value))->sar)>>9&0x1)*3.9);
    sar_value += (double)((((*(sample_value))->sar)>>8&0x1)*3.9/2);
    sar_value += (double)((((*(sample_value))->sar)>>7&0x1)*3.9/4);
    sar_value += (double)((((*(sample_value))->sar)>>6&0x1)*3.9/4);
    sar_value += (double)((((*(sample_value))->sar)>>5&0x1)*3.9/8);
    sar_value += (double)((((*(sample_value))->sar)>>4&0x1)*3.9/16);
    sar_value += (double)((((*(sample_value))->sar)>>3&0x1)*3.9/32);
    sar_value += (double)((((*(sample_value))->sar)>>2&0x1)*3.9/32);
    sar_value += (double)((((*(sample_value))->sar)>>1&0x1)*3.9/64);
    sar_value += (double)((((*(sample_value))->sar)>>0&0x1)*3.9/128);
    //TRACE(0 , "sar_value1 = %f", sar_value);
    sar_int_zheng_value = (int)sar_value;
    sar_int_xiao_value = (int) ((sar_value-sar_int_zheng_value)*1000000);
    TRACE(0 , "sar_value1 = %d.%d", sar_int_zheng_value,sar_int_xiao_value); 
    (*(sample_value))->sar_zheng = sar_int_zheng_value;
    (*(sample_value))->sar_xiao = sar_int_xiao_value;
    TRACE(0 , "sar_value2 = %d.%d", (*(sample_value))->sar_zheng, (*(sample_value))->sar_xiao); 
}

void count_sdm_value(struct cap_sense_sample **sample_value)
{
    double sdm_value = 0;
    int sdm_int_zheng_value;
    int sdm_int_xiao_value;

    sdm_value += (double)(((*(sample_value))->sdm)/131071.0);  //131072
    //TRACE(0 , "sdm_value1 = %f", sdm_value);
    sdm_int_zheng_value = (int)sdm_value;
    sdm_int_xiao_value = (int) ((sdm_value-sdm_int_zheng_value)*1000000);
    TRACE(0 , "sdm_value1 = %d.%d", sdm_int_zheng_value,sdm_int_xiao_value); 
    (*(sample_value))->sdm_zheng = sdm_int_zheng_value;
    (*(sample_value))->sdm_xiao = sdm_int_xiao_value;
    TRACE(0 , "sdm_value2 = %d.%d", (*(sample_value))->sdm_zheng, (*(sample_value))->sdm_xiao); 
}


int get_sample(struct cap_sense_sample *sample)
{
    uint32_t sdm_low;
    uint32_t sdm_high;
    uint16_t value = 0;
    
    hal_capsenseif_reg_read(0x63, &value);
    hal_capsenseif_reg_read(0x43, &value);
    sample->sar = (value >> 4) & 0x3ff;     //sar
    count_sar_value(&sample);               //sar_zheng_xiao
    sdm_high = value&(~0xfff0);             //sdm_hifh
    hal_capsenseif_reg_read(0x44, &value);
    sdm_low = (value >> 3) & 0x1fff;        //sdm_low
    sample->sdm = (sdm_high<<13) | sdm_low; //sdm(high,low)
    count_sdm_value(&sample);               //sdm_zheng_xiao
    sample->ch = value&(~0xfff8);           //channel
    
    return 0;
}

int catpture_sample_start(void)
{
    hal_capsenseif_reg_write(0x54,0x0010);
    return 0;
}

int catpture_sample_stop(void)
{
    hal_capsenseif_reg_write(0x54,0x0000);
    return 0;
}

int catpture_base_line(void)
{
    uint16_t value = 0;
    double base_value = 0;
    int base_int_zheng_value;
    int base_int_xiao_value;
    hal_capsenseif_reg_read(0x14, &value);  //ch 0
    TRACE(0 ,"0x14 val = %x", value);
    base_value += (double)((value>>9&0x1)*98.0);
    base_value += (double)((value>>8&0x1)*49.0);
    base_value += (double)((value>>7&0x1)*49.0/2);
    base_value += (double)((value>>6&0x1)*49.0/2);
    base_value += (double)((value>>5&0x1)*49.0/4);
    base_value += (double)((value>>4&0x1)*49.0/8);
    base_value += (double)((value>>3&0x1)*49.0/16);
    base_value += (double)((value>>2&0x1)*49.0/16);
    base_value += (double)((value>>1&0x1)*49.0/32);
    base_value += (double)((value>>0&0x1)*49.0/32);
    //TRACE(0 , "base_value1 = %f", base_value);
    base_int_zheng_value = (int)base_value;
    base_int_xiao_value = (int) ((base_value-base_int_zheng_value)*1000000);
    TRACE(0 , "base_value = %d.%d", base_int_zheng_value,base_int_xiao_value);
    hal_capsenseif_reg_read(0x15, &value);  //ch 1
    TRACE(0 ,"0x15 val = %x", value);
    return 0;
}

void cap_sense_reg_start_write(uint16_t reg, uint16_t val)
{
    hal_capsenseif_reg_write(reg, val);
}

void cap_sense_reg_start_read(uint16_t reg, uint16_t *val)
{
    hal_capsenseif_reg_read(reg, val);
}

void capsense_reg_init(void)
{
    const uint16_t (*capsense_init_tbl_p)[3] = NULL;
    uint16_t value = 0;
    uint32_t tbl_size = 0;
    capsense_init_tbl_p = &capsense_init_tbl_1[0];
    tbl_size = sizeof(capsense_init_tbl_1)/sizeof(capsense_init_tbl_1[0]);
    for(int i=0; i< tbl_size; i++){
        hal_capsenseif_reg_write(capsense_init_tbl_p[i][0],capsense_init_tbl_p[i][1]);
        osDelay(1);
        hal_capsenseif_reg_read(capsense_init_tbl_p[i][0],&value);
        TRACE_IMM(2,"reg=%x,v=%x",capsense_init_tbl_p[i][0],value);
        osDelay(10);
    }
}



