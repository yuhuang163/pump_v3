#ifdef TEST_CAPSENSE_SPI
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
#include "hal_cap_sensor_driver.h"

struct cap_sense_sample capsense_sample_buff[128];
//capsense_tbl_point_callback_t capsense_init_tbl_p = NULL;

void test_capsense_spi(void)
{
    TRACE_IMM(2,"%s used id: %s", __FUNCTION__, "SPI_ID_SLCD");
    int ret = 0;
    uint16_t value = 0;

    pmu_capsense_vref_enable();  //pmu init
    //ret = hal_capsenseif_open();
    TRACE(2,"%s hal_spilcd_open ret[%d]", __FUNCTION__, ret);
    capsense_reg_init();    //capsensor reg init 384过采样率，200K采样频率 ， full fifo timer：1/200k*384*128

    // hal_iomux_set_analog_i2c();  //I2C read
    osDelay(500);
    while(1) {
        catpture_base_line();
        catpture_sample_start();  //switch clock read--sample start
        for(int i=0; i< 128; i++){
            get_sample(&capsense_sample_buff[i]); //fifo deep-128 
            osDelay(1);
        }
        catpture_sample_stop();  //switch work clock mode
        osDelay(100);
        
        TRACE(0 , "====start sample==== ");
        TRACE(0 , "ch\tsar\tsar_count\tsdm\tsdm_count\tsar_sdm_count");
        for (int i=0; i<128; i++)
        {
            TRACE(0 ,"%d\t%x\t%x.%x\t%x\t%x.%x", capsense_sample_buff[i].ch, capsense_sample_buff[i].sar, 
                capsense_sample_buff[i].sar_zheng, capsense_sample_buff[i].sar_xiao, 
                capsense_sample_buff[i].sdm, capsense_sample_buff[i].sdm_zheng, capsense_sample_buff[i].sdm_xiao);
            osDelay(1);
        }
        TRACE(0 , "========end========\n\n ");

        osDelay(500);
    }

}

#endif
