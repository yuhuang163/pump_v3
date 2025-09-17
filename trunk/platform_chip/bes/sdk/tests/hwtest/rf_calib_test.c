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
#include "hal_spi.h"
#include "hal_cmu.h"
#include "string.h"

#define PMU_ROM_READ_CMD(r)             ((1 << 31) | (((r) & 0x7F) << 24))
#define PMU_ROM_WRITE_CMD(r, v)         ((((r) & 0x7F) << 24) | (((v) & 0xFFFF) << 8))
#define PMU_ROM_READ_VAL(v)             (((v) >> 8) & 0xFFFF)

static int rf_write_reg(unsigned short reg, unsigned short val)
{
    unsigned int cmd;

    cmd = PMU_ROM_WRITE_CMD(reg, val);
    return hal_ispi_send(&cmd, 4);
}

static int rf_read_reg(unsigned short reg, unsigned short *val)
{
    unsigned int cmd, data;
    int ret;

    cmd = PMU_ROM_READ_CMD(reg);
    ret = hal_ispi_recv(&cmd, &data, 4);
    if (ret) {
        return ret;
    }
    *val = PMU_ROM_READ_VAL(data);
    return 0;
}

static void delay_us(unsigned int us)
{
    volatile int i;
    // 6 instructions needed for each loop
    // SYS freq 78M
    // 1 us = 78 instructions = 13 loops
    us *= 13;
    for (i = 0; i < us; i++);
}

static void rx_dc(void)
{
    unsigned short read_value = 0;

    rf_write_reg(0x2c, 0x705c); // dig_vtoi_en    
    rf_write_reg(0x04, 0x53fc); // power on lna   
    rf_write_reg(0x18, 0x0680); // power on vco   
    rf_write_reg(0x17, 0x1c00); // vco cap bank   
    rf_write_reg(0x19, 0xe440); // vco cap bank   
    rf_write_reg(0x0a, 0x0a2f); // vco cap bank   
    rf_write_reg(0x2d, 0x0782); // bb ldo on 

    rf_write_reg(0xc0, 0x0088); // page 1 
    rf_write_reg(0xf6, 0xff32); // clk 0.5mhz 
    rf_write_reg(0xc0, 0x0008); // page 0 

    rf_write_reg(0x08, 0x0202); // cal wait time setup    
	
// rxdc cal 
// 07
//reg_btfm_flt_cap_block[15]
//reg_btfm_flt_cal1_en[14]
//reg_btfm_flt_cal2_en[13]
//reg_btfm_flt_cac1_dr[12]
//reg_btfm_flt_cac2_dr[11]              
//reg_btfm_flt_cal1en_dr[10]
//reg_btfm_flt_cal1en[9] 
//reg_btfm_flt_cal2en_dr[8] 
//reg_btfm_flt_cal2en[7] 
//reg_btfm_flt_calen[6] 
//reg_btfm_flt_pu_dr[5] 
//reg_btfm_flt_pu[4] 
//reg_btfm_flt_rstn[3] 
//reg_btfm_flt_dac_en[2] 
//reg_btfm_flt_dpd_en[1] 
//reg_bt_rxlowifdc_cal_resetn[0] 
    
    delay_us(1); // wait 1us   
    rf_write_reg(0x07, 0x0030); // power on    
    delay_us(1); // wait for 1us
// ================================ index 0 =========================================== //        
    rf_write_reg(0x07, 0x0038); // cal dig resetn  
    delay_us(1); // wait for 1us
    rf_write_reg(0x02, 0x0694); // rx flt gain dr set
    rf_write_reg(0x0b, 0x4000); // rx flt gain for ==================== index 0

    rf_write_reg(0x07, 0x0039); // cal dig resetn finish 
    rf_write_reg(0x07, 0x0079); // cal clk on     
    rf_write_reg(0x07, 0x4079); // cal1 en
    delay_us(110); // wait for 60us 
    rf_write_reg(0xc0, 0x0080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xd0, &read_value); // read cal value
    rf_write_reg(0xd0, read_value); // write cal value =================================== d0  
    rf_write_reg(0xc0, 0x0000); // chip page0    
    delay_us(1); // wait for 1us
    rf_write_reg(0x09, 0x0010); // set rx gain ====================  index 0 
    rf_write_reg(0x07, 0x2079); // cal2 en
    delay_us(110); // wait for 60us 
    rf_write_reg(0xc0, 0x0080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xd8, &read_value); // read cal value
    rf_write_reg(0xd8, read_value); // write cal value =================================== d8 
    rf_write_reg(0xc0, 0x0000); // chip page0    
 
// ================================ index 1 =========================================== //    
    rf_write_reg(0x07, 0x0038); // cal dig resetn  
    delay_us(1); // wait for 1us
    rf_write_reg(0x0b, 0x402d); // rx flt gain for ====================  index 1
    
    rf_write_reg(0x07, 0x0039); // cal dig resetn finish 
    rf_write_reg(0x07, 0x0079); // cal clk on 
    rf_write_reg(0x07, 0x4079); // cal1 en
    delay_us(110); // wait for 60us 
    rf_write_reg(0xc0, 0x0080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xd0, &read_value); // read cal value
    rf_write_reg(0xd1, read_value); // write cal value =================================== d1 
    rf_write_reg(0xc0, 0x0000); // chip page0    
    delay_us(1); // wait for 2us
    rf_write_reg(0x09, 0x0011); // set rx gain ====================  index 1
    rf_write_reg(0x07, 0x2079); // cal2 en
    delay_us(110); // wait for 60us 
    rf_write_reg(0xc0, 0x0080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xd8, &read_value); // read cal value
    rf_write_reg(0xd9, read_value); // write cal value =================================== d9 
    rf_write_reg(0xc0, 0x0000); // chip page0    

// ================================ index 8 =========================================== //        
    rf_write_reg(0x07, 0x0038); // cal dig resetn  
    delay_us(1); // wait for 1us
    rf_write_reg(0x0b, 0x4000); // rx flt gain fpr ====================  index 8

    rf_write_reg(0x07, 0x0039); // cal dig resetn finish 
    rf_write_reg(0x07, 0x0079); // cal clk on     
    rf_write_reg(0x07, 0x4079); // cal1 en
    delay_us(110); // wait for 60us 
    rf_write_reg(0xc0, 0x0080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xd0, &read_value); // read cal value
    rf_write_reg(0xc0, 0x8080); // chip page1+1        
    rf_write_reg(0xd0, read_value); // write cal value =================================== 1d0
    rf_write_reg(0xc0, 0x0000); // chip page0    
    delay_us(1); // wait for 1us
    rf_write_reg(0x09, 0x0018); // set rx gain ====================  index 8 
    rf_write_reg(0x07, 0x2079); // cal2 en
    delay_us(110); // wait for 60us 
    rf_write_reg(0xc0, 0x8080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xd8, &read_value); // read cal value
    rf_write_reg(0xc0, 0x8080); // chip page1+1        
    rf_write_reg(0xd8, read_value); // write cal value =================================== 1d8  
    rf_write_reg(0xc0, 0x0000); // chip page0    

// ================================ index 9 =========================================== //        
    rf_write_reg(0x07, 0x0038); // cal dig resetn  
    delay_us(1); // wait for 1us
    rf_write_reg(0x0b, 0x4000); // rx flt gain fpr ==================== index 9

    rf_write_reg(0x07, 0x0039); // cal dig resetn finish 
    rf_write_reg(0x07, 0x0079); // cal clk on     
    rf_write_reg(0x07, 0x4079); // cal1 en
    delay_us(110); // wait for 60us 
    rf_write_reg(0xc0, 0x0080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xd0, &read_value); // read cal value
    rf_write_reg(0xc0, 0x8080); // chip page1+1        
    rf_write_reg(0xd1, read_value); // write cal value =================================== 1d1  
    rf_write_reg(0xc0, 0x0000); // chip page0    
    delay_us(1); // wait for 1us
    rf_write_reg(0x09, 0x0019); // set rx gain ====================  index 9 
    rf_write_reg(0x07, 0x2079); // cal2 en
    delay_us(110); // wait for 60us 
    rf_write_reg(0xc0, 0x8080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xd8, &read_value); // read cal value
    rf_write_reg(0xc0, 0x8080); // chip page1+1        
    rf_write_reg(0xd9, read_value); // write cal value =================================== 1d9  
    rf_write_reg(0xc0, 0x0000); // chip page0    
    
    
    delay_us(1); // wait for 1us
// ================================ index end finish ==================================== //    
    rf_write_reg(0x07, 0x0038); // cal dig resetn  
    delay_us(1); // wait for 1us    
    rf_write_reg(0x07, 0x0039); // cal dig resetn finish 
    delay_us(1); // wait for 1us reg recover default value
    rf_write_reg(0x2c, 0x305c); // dig_vtoi_en    
    rf_write_reg(0x04, 0x5154); // power on lna   
    rf_write_reg(0x18, 0x0280); // power on vco   
    rf_write_reg(0x17, 0x0000); // vco cap bank   
    rf_write_reg(0x19, 0x6440); // vco cap bank   
    rf_write_reg(0x0a, 0x022f); // vco cap bank   
    rf_write_reg(0x2d, 0x0202); // bb ldo on 
    rf_write_reg(0x02, 0x0094); // rx flt gain dr set
    rf_write_reg(0x09, 0x0000); // set rx gain  
    
    delay_us(200); 

}

static void tx_dc(void)
{
    unsigned short read_value;

    rf_write_reg(0x2c, 0x705c); // dig_vtoi_en    
    rf_write_reg(0x0f, 0x1e18); // pu_dac    
    rf_write_reg(0x2d, 0x061a); // dig_bb_ldo_pu dig_bb_ldo_pu_vddr15c    
    rf_write_reg(0x10, 0x00b0); // reg_bt_dac_mux_en == 0000    
    rf_write_reg(0x11, 0x03ff); // reg_bt_txdc_cal_resetn[15]reg_bt_tmx_en[14]reg_bt_tmx_init_delay[13:12]reg_bt_tmx_polarity[11]reg_bt_tmx_dc_cal_en_dr[9]reg_bt_tmx_dc_cal_en[8]reg_bt_tmx_pu_dr[7]reg_bt_tmx_pu[6]reg_bt_tmx_pu_dc_cal_dr[5]reg_bt_tmx_pu_dc_cal[4]reg_bt_tmx_pu_ldo_dr[3]reg_bt_tmx_pu_ldo[2]reg_bt_tmx_pu_lpf_dr[1]reg_bt_tmx_pu_lpf[0]    
    delay_us(10);
    rf_write_reg(0x11, 0x83ff); // reg_bt_txdc_cal_resetn[15]reg_bt_tmx_en[14]reg_bt_tmx_init_delay[13:12]reg_bt_tmx_polarity[11]reg_bt_tmx_dc_cal_en_dr[9]reg_bt_tmx_dc_cal_en[8]reg_bt_tmx_pu_dr[7]reg_bt_tmx_pu[6]reg_bt_tmx_pu_dc_cal_dr[5]reg_bt_tmx_pu_dc_cal[4]reg_bt_tmx_pu_ldo_dr[3]reg_bt_tmx_pu_ldo[2]reg_bt_tmx_pu_lpf_dr[1]reg_bt_tmx_pu_lpf[0]    
    delay_us(1);
    rf_write_reg(0x0e, 0x00ff); // txmx gain    
    rf_write_reg(0x11, 0xc3ff); // reg_bt_txdc_cal_resetn[15]reg_bt_tmx_en[14]reg_bt_tmx_init_delay[13:12]reg_bt_tmx_polarity[11]reg_bt_tmx_dc_cal_en_dr[9]reg_bt_tmx_dc_cal_en[8]reg_bt_tmx_pu_dr[7]reg_bt_tmx_pu[6]reg_bt_tmx_pu_dc_cal_dr[5]reg_bt_tmx_pu_dc_cal[4]reg_bt_tmx_pu_ldo_dr[3]reg_bt_tmx_pu_ldo[2]reg_bt_tmx_pu_lpf_dr[1]reg_bt_tmx_pu_lpf[0]    
    delay_us(200); // 200us
    rf_write_reg(0xc0, 0x0080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xc5, &read_value); // read cal value
    rf_write_reg(0xc8, 0xc0c0); // write cal value   
    rf_write_reg(0xc0, 0x0000); // chip page0    
    delay_us(60);

    rf_write_reg(0x11, 0x03ff); // reg_bt_txdc_cal_resetn[15]reg_bt_tmx_en[14]reg_bt_tmx_init_delay[13:12]reg_bt_tmx_polarity[11]reg_bt_tmx_dc_cal_en_dr[9]reg_bt_tmx_dc_cal_en[8]reg_bt_tmx_pu_dr[7]reg_bt_tmx_pu[6]reg_bt_tmx_pu_dc_cal_dr[5]reg_bt_tmx_pu_dc_cal[4]reg_bt_tmx_pu_ldo_dr[3]reg_bt_tmx_pu_ldo[2]reg_bt_tmx_pu_lpf_dr[1]reg_bt_tmx_pu_lpf[0]    
    delay_us(10);
    rf_write_reg(0x11, 0x83ff); // reg_bt_txdc_cal_resetn[15]reg_bt_tmx_en[14]reg_bt_tmx_init_delay[13:12]reg_bt_tmx_polarity[11]reg_bt_tmx_dc_cal_en_dr[9]reg_bt_tmx_dc_cal_en[8]reg_bt_tmx_pu_dr[7]reg_bt_tmx_pu[6]reg_bt_tmx_pu_dc_cal_dr[5]reg_bt_tmx_pu_dc_cal[4]reg_bt_tmx_pu_ldo_dr[3]reg_bt_tmx_pu_ldo[2]reg_bt_tmx_pu_lpf_dr[1]reg_bt_tmx_pu_lpf[0]    
    delay_us(1);
    rf_write_reg(0x0e, 0x0000); // txmx gain    
    rf_write_reg(0x11, 0xc3ff); // reg_bt_txdc_cal_resetn[15]reg_bt_tmx_en[14]reg_bt_tmx_init_delay[13:12]reg_bt_tmx_polarity[11]reg_bt_tmx_dc_cal_en_dr[9]reg_bt_tmx_dc_cal_en[8]reg_bt_tmx_pu_dr[7]reg_bt_tmx_pu[6]reg_bt_tmx_pu_dc_cal_dr[5]reg_bt_tmx_pu_dc_cal[4]reg_bt_tmx_pu_ldo_dr[3]reg_bt_tmx_pu_ldo[2]reg_bt_tmx_pu_lpf_dr[1]reg_bt_tmx_pu_lpf[0]    
    delay_us(200); // 200us
    rf_write_reg(0xc0, 0x0080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xc5, &read_value); // read cal value
    rf_write_reg(0xc9, 0x8080); // write cal value   
    rf_write_reg(0xc0, 0x0000); // chip page0    
    delay_us(60);

    rf_write_reg(0x11, 0x03ff); // reg_bt_txdc_cal_resetn[15]reg_bt_tmx_en[14]reg_bt_tmx_init_delay[13:12]reg_bt_tmx_polarity[11]reg_bt_tmx_dc_cal_en_dr[9]reg_bt_tmx_dc_cal_en[8]reg_bt_tmx_pu_dr[7]reg_bt_tmx_pu[6]reg_bt_tmx_pu_dc_cal_dr[5]reg_bt_tmx_pu_dc_cal[4]reg_bt_tmx_pu_ldo_dr[3]reg_bt_tmx_pu_ldo[2]reg_bt_tmx_pu_lpf_dr[1]reg_bt_tmx_pu_lpf[0]    
    delay_us(10);
    rf_write_reg(0x11, 0x83ff); // reg_bt_txdc_cal_resetn[15]reg_bt_tmx_en[14]reg_bt_tmx_init_delay[13:12]reg_bt_tmx_polarity[11]reg_bt_tmx_dc_cal_en_dr[9]reg_bt_tmx_dc_cal_en[8]reg_bt_tmx_pu_dr[7]reg_bt_tmx_pu[6]reg_bt_tmx_pu_dc_cal_dr[5]reg_bt_tmx_pu_dc_cal[4]reg_bt_tmx_pu_ldo_dr[3]reg_bt_tmx_pu_ldo[2]reg_bt_tmx_pu_lpf_dr[1]reg_bt_tmx_pu_lpf[0]    
    delay_us(1);
    rf_write_reg(0x11, 0xc3ff); // reg_bt_txdc_cal_resetn[15]reg_bt_tmx_en[14]reg_bt_tmx_init_delay[13:12]reg_bt_tmx_polarity[11]reg_bt_tmx_dc_cal_en_dr[9]reg_bt_tmx_dc_cal_en[8]reg_bt_tmx_pu_dr[7]reg_bt_tmx_pu[6]reg_bt_tmx_pu_dc_cal_dr[5]reg_bt_tmx_pu_dc_cal[4]reg_bt_tmx_pu_ldo_dr[3]reg_bt_tmx_pu_ldo[2]reg_bt_tmx_pu_lpf_dr[1]reg_bt_tmx_pu_lpf[0]    
    delay_us(200); // 200us
    rf_write_reg(0xc0, 0x0080); // chip page1    
    rf_read_reg(0xc0, &read_value); // det cal done
    rf_read_reg(0xc5, &read_value); // read cal value
    rf_write_reg(0xca, 0x8080); // write cal value   
    
    rf_write_reg(0xcb, 0xc0c0); // write cal value   
    rf_write_reg(0xcc, 0xa0a0); // write cal value   
    rf_write_reg(0xcd, 0x9090); // write cal value   
    rf_write_reg(0xce, 0x8888); // write cal value   
    rf_write_reg(0xcf, 0x8484); // write cal value   
    rf_write_reg(0xc0, 0x0000); // chip page0    
    rf_write_reg(0x11, 0x0000); // reg_bt_txdc_cal_resetn[15]reg_bt_tmx_en[14]reg_bt_tmx_init_delay[13:12]reg_bt_tmx_polarity[11]reg_bt_tmx_dc_cal_en_dr[9]reg_bt_tmx_dc_cal_en[8]reg_bt_tmx_pu_dr[7]reg_bt_tmx_pu[6]reg_bt_tmx_pu_dc_cal_dr[5]reg_bt_tmx_pu_dc_cal[4]reg_bt_tmx_pu_ldo_dr[3]reg_bt_tmx_pu_ldo[2]reg_bt_tmx_pu_lpf_dr[1]reg_bt_tmx_pu_lpf[0]    
    delay_us(60);
}

void rf_calib_test(void)
{
    struct HAL_SPI_CFG_T spi_cfg;

    memset(&spi_cfg, 0, sizeof(spi_cfg));
    spi_cfg.clk_delay_half = false;
    spi_cfg.clk_polarity = false;
    spi_cfg.slave = false;
    spi_cfg.rate = 1300000;
    spi_cfg.dma_rx = false;
    spi_cfg.dma_tx = false;
    spi_cfg.rx_sep_line = false;
    spi_cfg.cs = 0;
    spi_cfg.tx_bits = 32;
    spi_cfg.rx_bits = 32;
    spi_cfg.rx_frame_bits = 0;

    hal_ispi_open(&spi_cfg);

    rx_dc();

    tx_dc();

    TRACE(0,"RF calib passed");
    hal_cmu_simu_pass();
}


