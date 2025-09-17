#ifndef __TEST_HAL_API_H__
#define __TEST_HAL_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/* gpio api */
void
test_gpio(void);

/* timer api*/
void
test_timer(void);

/* rtc api*/
void 
test_rtc(void);

/* wdt api */
void test_wdt(void);

/* ostick api */
void
test_ostick_and_timestamp(void);

/* i2c api */
void
test_i2c(void);

/* trng api */
void test_trng(void);

/* sec_eng api */
void sec_eng_test(void);

/* PWM */
int
test_pwm(void);

/* spi api */
void test_spi(void);

/* uart api */
int test_uart(void);

/*openamp*/
void test_openamp(void);

/* gpadc */
void test_gpadc(void);

/* norflash */
void test_norflash(void);

/* vmic */
void test_vmic(void);

/* dma */
void test_dma(void);

/* i2s */
void test_i2s(void);

/* sdio_host_signal */
void test_sdio_host_signal(void);

/* sdio_host */
void test_sdio_host(void);

/* psram */
void test_psram(void);

/*emmc */
void test_emmc(void);

/*display */
void test_display(void);
void test_gpu(void);
void test_fb(void);
void test_gpu_draw(void);
void test_lcdc(void);
void test_arm2d(void);

/*sensor_engine */
void test_sensor_engine(void);

/*spi slave*/
void spi_slave_test(void);

/*freq switch time cost*/
void test_freq_sw_time(void);

/* mbedtls library test */
void test_mbedtls(void);

void test_dma2d(void);

/* blend */
void test_blend(void);

/* tma525c touchscreen test */
void test_tma525c(void);

/*shanhai mbedtls*/
void test_shanhai_mbedtls(void);

void run_fc_test(void);

void test_user_gps_to_flash(void);

void test_dwt(void);

void test_mpu(void);

void test_sysview(void);

void beco_test(void);

void cmsis_test(void);

#ifdef __cplusplus
}
#endif

#endif
