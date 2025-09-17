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
 ****************************************************************************/
#include "plat_addr_map.h"

#ifdef SENSOR_ENGINE_TEST

#include "string.h"
#include "hal_spi.h"
#include "hal_i2c.h"
#include "hal_gpio.h"
#include "hal_sleep.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "hal_sensor_eng_v2.h"

#if 1
    #define SENSOR_ENGINE_TRACE(n, s, ...)      TRACE(n, s, ##__VA_ARGS__)
    #define SENSOR_ENGINE_DUMP8(n, s, ...)      DUMP8(n, s, ##__VA_ARGS__)
#else
    #define SENSOR_ENGINE_TRACE(n, s, ...)
    #define SENSOR_ENGINE_DUMP8(n, s, ...)
#endif

#if 1
    #define SENSOR_ENGINE_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
    #define SENSOR_ENGINE_DELAY_US(us)          hal_sys_timer_delay_us(us)
#else
    #define SENSOR_ENGINE_DELAY_MS(ms)
    #define SENSOR_ENGINE_DELAY_US(us)
#endif


#define TEST_MODE_PERIODIC_TIMER

#define     SPI_ID                              0   //At 1600, spi can only be spi0,2021-10-01
#define     SENSOR_ENG_MASTER_ID                HAL_SENSOR_ENGINE_MASTER_0
#define     __HAL_ISPI__                        0
#define     __HAL_SPI_ID_0__                    1
#define     __HAL_SPI_ID_SLCD__                 2

#define     SENSOR_ENGINE_DATA_LOG
#define     VCNL36826S_ENABLE
//#define     VCNL36826S_WRITE_ENABLE
//#define     LIS3DH_WRITE_ENABLE

/*-------------------i2c address config--------------------*/
#define     LSM6DSOX_I2C_ADD_L                  0xD5
#define     LSM6DSOX_I2C_ADD_H                  0xD7
#define     LIS3DSH_I2C_ADD_L                   0x3D
#define     LIS3DSH_I2C_ADD_H                   0x3B
#define     BMP280_I2C_ADDR_PRIM                0x76
#define     BMP280_I2C_ADDR_SEC                 0x77
#define     BMA2x2_I2C_ADDR1                    0x18
#define     BMA2x2_I2C_ADDR2                    0x19
#define     BMA2x2_I2C_ADDR3                    0x10
#define     BMA2x2_I2C_ADDR4                    0x11
#define     LSM6DSOX_WHO_AM_I                   0x0F
#define     LIS3DSH_INFO1                       0x0D
#define     LIS3DSH_INFO2                       0x0E
#define     LIS3DSH_WHO_AM_I                    0x0F
#define     LIS3DH_I2C_ADD_L                    0x31
#define     LIS3DH_I2C_ADD_H                    0x33
#define     LIS3DH_ID                           0x33
#define     LIS3DH_WHO_AM_I                     0x0F
#define     LIS3DH_CTRL_REG0                    0x1E
#define     LIS3DH_TIME_LIMIT                   0x3B
#define     BMP280_CHIP_ID_ADDR                 0xD0
#define     BMP280_SOFT_RESET_ADDR              0xE0
#define     BMP280_STATUS_ADDR                  0xF3
#define     BMP280_CTRL_MEAS_ADDR               0xF4
#define     BMP280_CONFIG_ADDR                  0xF5
#define     BMP280_PRES_MSB_ADDR                0xF7

#define     lsm6dsox_address                    ((LSM6DSOX_I2C_ADD_H & 0xFE)>>1)
#define     lis3dsh_address                     ((LIS3DSH_I2C_ADD_L & 0xFE)>>1)
#define     bmp280_address                      (BMP280_I2C_ADDR_PRIM)
#define     bmx055_accel_addr                   (BMA2x2_I2C_ADDR1)
#define     bmx055_gyro_addr                    (0x68)
#define     bmx055_magnet_addr                  (BMA2x2_I2C_ADDR3)
#define     bmx055_accel_id_cmd                 (0x00)
#define     bmx055_gyro_id_cmd                  (0x00)
#define     bmx055_magnet_id_cmd                (0x40)
#define     max30102_address                    (0x57)
#define     max30102_id_cmd                     (0xFF)
#define     vcnl36826s_address                  (0x60)
#define     vcnl36826s_id_cmd                   (0xFA)
#define     vcnl36826s_ps_thdL                  (0x05)
#define     vcnl36826s_ps_thdH                  (0x06)
#define     bes1501_address                     (0x27)
#define     BES1501_WHOAMI                      (0x00)

/** I2C Device Address 8 bit format. SA0 = not(I2C_A0 pin) **/
#define     LIS25BA_I2C_ADD_L                   0x33
#define     LIS25BA_I2C_ADD_H                   0x31
#define     LIS25BA_WHO_AM_I                    0x0FU   /*id=0x20*/
#define     LIS25BA_ADDRESS                     ((LIS25BA_I2C_ADD_L & 0xFE)>>1)

/*----------------user config,2021-01-06-------------------*/
#define     sensor_engine_i2c_master_cnt        1
#define     sensor_engine_spi_master_cnt        1
#define     sensor_engine_i2c_slave_cnt         7   /*The maximum value is less than SENSOR_NUM*/
#define     sensor_engine_spi_slave_cnt         1   /*The maximum value is less than SENSOR_NUM*/
#define     SENSOR_NUM                          8   /*total Number of sensors*/

#define     SENSOR0_SENSOR_ENGINE_ID            0
#define     SENSOR1_SENSOR_ENGINE_ID            1
#define     SENSOR2_SENSOR_ENGINE_ID            2
#define     SENSOR3_SENSOR_ENGINE_ID            3
#define     SENSOR4_SENSOR_ENGINE_ID            4
#define     SENSOR5_SENSOR_ENGINE_ID            5
#define     SENSOR6_SENSOR_ENGINE_ID            6
#define     SENSOR7_SENSOR_ENGINE_ID            7

#define     SENSOR0_SENSOR_ENGINE_DEVICE        HAL_SENSOR_ENGINE_DEVICE_I2C4
#define     SENSOR1_SENSOR_ENGINE_DEVICE        HAL_SENSOR_ENGINE_DEVICE_I2C4
#define     SENSOR2_SENSOR_ENGINE_DEVICE        HAL_SENSOR_ENGINE_DEVICE_I2C4
#define     SENSOR3_SENSOR_ENGINE_DEVICE        HAL_SENSOR_ENGINE_DEVICE_I2C4
#define     SENSOR4_SENSOR_ENGINE_DEVICE        HAL_SENSOR_ENGINE_DEVICE_I2C4
#define     SENSOR5_SENSOR_ENGINE_DEVICE        HAL_SENSOR_ENGINE_DEVICE_I2C4
#define     SENSOR6_SENSOR_ENGINE_DEVICE        HAL_SENSOR_ENGINE_DEVICE_I2C4
#if (sensor_engine_spi_slave_cnt == 0)
#define     SENSOR7_SENSOR_ENGINE_DEVICE        HAL_SENSOR_ENGINE_DEVICE_I2C4
#else
#define     SENSOR7_SENSOR_ENGINE_DEVICE        HAL_SENSOR_ENGINE_DEVICE_SPI
#endif

#define     DEVICE_CONTROLLER_0                 HAL_I2C_ID_4    /* i2c first,only config i2c/spi use*/
#define     DEVICE_CONTROLLER_1                 SPI_ID          /* i2c first,only config i2c/spi use*/
#define     DEVICE_CONTROLLER_2                 HAL_I2C_ID_4    /* i2c first,only config i2c/spi use*/
#define     DEVICE_CONTROLLER_3                 HAL_I2C_ID_4    /* i2c first,only config i2c/spi use*/
#define     DEVICE_CONTROLLER_4                 HAL_I2C_ID_4    /* i2c first,only config i2c/spi use*/
#define     DEVICE_CONTROLLER_5                 HAL_I2C_ID_4    /* i2c first,only config i2c/spi use*/
#define     DEVICE_CONTROLLER_6                 HAL_I2C_ID_4    /* i2c first,only config i2c/spi use*/
#define     DEVICE_CONTROLLER_7                 HAL_I2C_ID_4    /* i2c first,only config i2c/spi use*/

#define     SENSOR0_DEVICE_SPI_CS               0
#define     SENSOR1_DEVICE_SPI_CS               1
#define     SENSOR2_DEVICE_SPI_CS               2
#define     SENSOR3_DEVICE_SPI_CS               3
#define     SENSOR4_DEVICE_SPI_CS               4
#define     SENSOR5_DEVICE_SPI_CS               5
#define     SENSOR6_DEVICE_SPI_CS               6
#define     SENSOR7_DEVICE_SPI_CS               0

#define     SENSOR0_WRITE_BUF_SIZE              1   /*write buffer size*/
#define     SENSOR1_WRITE_BUF_SIZE              1   /*write buffer size*/
#define     SENSOR2_WRITE_BUF_SIZE              1   /*write buffer size*/
#define     SENSOR3_WRITE_BUF_SIZE              1   /*write buffer size*/
#define     SENSOR4_WRITE_BUF_SIZE              1   /*write buffer size*/
#define     SENSOR5_WRITE_BUF_SIZE              1   /*write buffer size*/
#ifdef VCNL36826S_ENABLE
#ifndef VCNL36826S_WRITE_ENABLE
#define     SENSOR6_WRITE_BUF_SIZE              1   /*write buffer size*/
#else
#define     SENSOR6_WRITE_BUF_SIZE              3   /*write buffer size*/
#endif
#else
#define     SENSOR6_WRITE_BUF_SIZE              1   /*write buffer size*/
#endif
#if (sensor_engine_spi_slave_cnt == 0)
#define     SENSOR7_WRITE_BUF_SIZE              1   /*write buffer size*/
#else
#ifndef LIS3DH_WRITE_ENABLE
#define     SENSOR7_WRITE_BUF_SIZE              1   /*write buffer size*/
#else
#define     SENSOR7_WRITE_BUF_SIZE              2   /*write buffer size*/
#endif
#endif

#define     SENSOR0_WRITE_BUF_DATA              LSM6DSOX_WHO_AM_I       /*id=0x6C*/
#define     SENSOR1_WRITE_BUF_DATA              LIS3DSH_INFO1           /*info=0x21 00 3F*/
#define     SENSOR2_WRITE_BUF_DATA              BMP280_PRES_MSB_ADDR    /*press=0x80 00 00,temp=0x80 00 00*/
#define     SENSOR3_WRITE_BUF_DATA              LIS25BA_WHO_AM_I        /*lis25ba id=0x20*/
#define     SENSOR4_WRITE_BUF_DATA              bmx055_accel_id_cmd     /*bmx055 accle  id=0xFA ,bes1501(1.8v) id=0x1510*/
#define     SENSOR5_WRITE_BUF_DATA              bmx055_magnet_id_cmd    /*bmx055 magnet id=0x32*/
#ifdef VCNL36826S_ENABLE
#ifndef VCNL36826S_WRITE_ENABLE
//#define     SENSOR6_WRITE_BUF_DATA              vcnl36826s_id_cmd       /*vcnl36826s id=0x0026,id_l=0x26,id_h=0x00*/
#define     SENSOR6_WRITE_BUF_DATA              vcnl36826s_ps_thdL      /*[read write data test],vcnl36826s data=0xA5 0A*/
#else
#define     SENSOR6_WRITE_BUF_DATA              vcnl36826s_ps_thdL
#endif
#else
#define     SENSOR6_WRITE_BUF_DATA              bmx055_gyro_id_cmd      /*bmx055 gyro   id=0x0F*/
#endif

#if (sensor_engine_spi_slave_cnt == 0)
#define     SENSOR7_WRITE_BUF_DATA              max30102_id_cmd         /*max30102 id=0x15*/
#else
#ifndef LIS3DH_WRITE_ENABLE
//#define     SENSOR7_WRITE_BUF_DATA              (BMP280_PRES_MSB_ADDR | 0x80)   /*press=0x80 00 00,temp=0x80 00 00*/
//The register of bmp280 cannot be written directly, so the spi write test of bmp280 is abandoned,2021-03-16
//#define     SENSOR7_WRITE_BUF_DATA              (BMP280_CTRL_MEAS_ADDR | 0x80)   /*[read write data test],data=0xAA 5C 00 80 00 00*/
//#define     SENSOR7_WRITE_BUF_DATA              (LIS3DH_WHO_AM_I | 0x80) /*lis3dh id=0x33*/
#define     SENSOR7_WRITE_BUF_DATA              (LIS3DH_TIME_LIMIT | 0x80) /*lis3dh TIME_LIMIT=0xAA*/
#else
//#define     SENSOR7_WRITE_BUF_DATA              (BMP280_CTRL_MEAS_ADDR)
#define     SENSOR7_WRITE_BUF_DATA              (LIS3DH_TIME_LIMIT)
#endif
#endif

#define     SENSOR0_I2C_ADDR                    lsm6dsox_address
#define     SENSOR1_I2C_ADDR                    lis3dsh_address
#define     SENSOR2_I2C_ADDR                    bmp280_address
#define     SENSOR3_I2C_ADDR                    LIS25BA_ADDRESS
#define     SENSOR4_I2C_ADDR                    bmx055_accel_addr
#define     SENSOR5_I2C_ADDR                    bmx055_magnet_addr
#ifdef VCNL36826S_ENABLE
#define     SENSOR6_I2C_ADDR                    vcnl36826s_address
#else
#define     SENSOR6_I2C_ADDR                    bmx055_gyro_addr
#endif
#define     SENSOR7_I2C_ADDR                    max30102_address    //Useless when spi

#define     SENSOR0_READ_BURST_CNT              66  /*read buffer size,Least common multiple*/
#define     SENSOR1_READ_BURST_CNT              66  /*read buffer size,Least common multiple*/
#define     SENSOR2_READ_BURST_CNT              66  /*read buffer size,Least common multiple*/
#define     SENSOR3_READ_BURST_CNT              66  /*read buffer size,Least common multiple*/
#define     SENSOR4_READ_BURST_CNT              66  /*read buffer size,Least common multiple*/
#define     SENSOR5_READ_BURST_CNT              66  /*read buffer size,Least common multiple*/
#define     SENSOR6_READ_BURST_CNT              66  /*read buffer size,Least common multiple*/
#define     SENSOR7_READ_BURST_CNT              66  /*read buffer size,Least common multiple*/

#define     SENSOR0_RX_BURST_LEN                1   /*lsm6dsox i2c=1,spi=2,The length of data received after each reading of the sensor */
#define     SENSOR1_RX_BURST_LEN                3   /*lis3dsh  i2c=3,spi=4,The length of data received after each reading of the sensor*/
#define     SENSOR2_RX_BURST_LEN                6   /*bmp280   i2c=6,spi=7,The length of data received after each reading of the sensor*/
#define     SENSOR3_RX_BURST_LEN                1   /*lis25ba=1,The length of data received after each reading of the sensor*/
#define     SENSOR4_RX_BURST_LEN                1   /*bes1501(1.8v)  i2c=2,lis25ba i2c=1,The length of data received after each reading of the sensor*/
#define     SENSOR5_RX_BURST_LEN                1
#ifdef VCNL36826S_ENABLE
#ifndef VCNL36826S_WRITE_ENABLE
#define     SENSOR6_RX_BURST_LEN                2
#else
#define     SENSOR6_RX_BURST_LEN                2
#endif
#else
#define     SENSOR6_RX_BURST_LEN                1
#endif
#if (sensor_engine_spi_slave_cnt == 0)
#define     SENSOR7_RX_BURST_LEN                1
#else
#ifndef LIS3DH_WRITE_ENABLE
#define     SENSOR7_RX_BURST_LEN                1   /* LIS3DSH=3,LIS3DH=1,bmp280=6*/
#else
#define     SENSOR7_RX_BURST_LEN                1
#endif
#endif

#define     SENSOR0_TRIGGER_TIMER_MS            500
#define     SENSOR1_TRIGGER_TIMER_MS            500
#define     SENSOR2_TRIGGER_TIMER_MS            500
#define     SENSOR3_TRIGGER_TIMER_MS            500
#define     SENSOR4_TRIGGER_TIMER_MS            500
#define     SENSOR5_TRIGGER_TIMER_MS            500
#define     SENSOR6_TRIGGER_TIMER_MS            500
#define     SENSOR7_TRIGGER_TIMER_MS            500

#define     SENSOR0_TRIGGER_GPIO                HAL_GPIO_PIN_P0_0
#define     SENSOR1_TRIGGER_GPIO                HAL_GPIO_PIN_P0_1
#define     SENSOR2_TRIGGER_GPIO                HAL_GPIO_PIN_P0_2
#define     SENSOR3_TRIGGER_GPIO                HAL_GPIO_PIN_P0_3
#define     SENSOR4_TRIGGER_GPIO                HAL_GPIO_PIN_P1_4
#define     SENSOR5_TRIGGER_GPIO                HAL_GPIO_PIN_P1_5
#define     SENSOR6_TRIGGER_GPIO                HAL_GPIO_PIN_P1_6
#define     SENSOR7_TRIGGER_GPIO                HAL_GPIO_PIN_P1_7

#define     SENSOR0_TRIGGERS_CNT                2   /*Interrupt after several triggers*/
#define     SENSOR1_TRIGGERS_CNT                2   /*Interrupt after several triggers*/
#define     SENSOR2_TRIGGERS_CNT                2   /*Interrupt after several triggers*/
#define     SENSOR3_TRIGGERS_CNT                2   /*Interrupt after several triggers*/
#define     SENSOR4_TRIGGERS_CNT                2   /*Interrupt after several triggers*/
#define     SENSOR5_TRIGGERS_CNT                2   /*Interrupt after several triggers*/
#define     SENSOR6_TRIGGERS_CNT                2   /*Interrupt after several triggers*/
#define     SENSOR7_TRIGGERS_CNT                2   /*Interrupt after several triggers*/

#define     SENSORX_CODEC_SEL                   HAL_SENSOR_ENGINE_CODEC_SEL_SENSOR0

#if (sensor_engine_i2c_master_cnt > 5)
#error The maximum value of "sensor_engine_i2c_master_cnt" is 5,2021-01-07!
#endif
#if (sensor_engine_spi_master_cnt > 1)
#error The maximum value of "sensor_engine_spi_master_cnt" is 1,2021-01-07!
#endif
#if (((sensor_engine_i2c_slave_cnt + sensor_engine_spi_slave_cnt) > 8)||(SENSOR_NUM > 8))
#error The maximum value of "SENSOR_NUM" is 8,2021-01-07!
#endif

static uint8_t sensor_eng_device_controller_list[] = {DEVICE_CONTROLLER_0,
                                                      DEVICE_CONTROLLER_1,
                                                      DEVICE_CONTROLLER_2,
                                                      DEVICE_CONTROLLER_3,
                                                      DEVICE_CONTROLLER_4,
                                                      DEVICE_CONTROLLER_5,
                                                      DEVICE_CONTROLLER_6,
                                                      DEVICE_CONTROLLER_7
                                                     };
static uint8_t sensor_sensor_engine_id[] = {  SENSOR0_SENSOR_ENGINE_ID,
                                              SENSOR1_SENSOR_ENGINE_ID,
                                              SENSOR2_SENSOR_ENGINE_ID,
                                              SENSOR3_SENSOR_ENGINE_ID,
                                              SENSOR4_SENSOR_ENGINE_ID,
                                              SENSOR5_SENSOR_ENGINE_ID,
                                              SENSOR6_SENSOR_ENGINE_ID,
                                              SENSOR7_SENSOR_ENGINE_ID
                                           };
static uint8_t sensor_sensor_engine_device[] = {  SENSOR0_SENSOR_ENGINE_DEVICE,
                                                  SENSOR1_SENSOR_ENGINE_DEVICE,
                                                  SENSOR2_SENSOR_ENGINE_DEVICE,
                                                  SENSOR3_SENSOR_ENGINE_DEVICE,
                                                  SENSOR4_SENSOR_ENGINE_DEVICE,
                                                  SENSOR5_SENSOR_ENGINE_DEVICE,
                                                  SENSOR6_SENSOR_ENGINE_DEVICE,
                                                  SENSOR7_SENSOR_ENGINE_DEVICE
                                               };
static uint8_t trigger_gpio_pin[] = { SENSOR0_TRIGGER_GPIO,
                                      SENSOR1_TRIGGER_GPIO,
                                      SENSOR2_TRIGGER_GPIO,
                                      SENSOR3_TRIGGER_GPIO,
                                      SENSOR4_TRIGGER_GPIO,
                                      SENSOR5_TRIGGER_GPIO,
                                      SENSOR6_TRIGGER_GPIO,
                                      SENSOR7_TRIGGER_GPIO
                                    };
static uint32_t trigger_timer[] = {   SENSOR0_TRIGGER_TIMER_MS,
                                      SENSOR1_TRIGGER_TIMER_MS,
                                      SENSOR2_TRIGGER_TIMER_MS,
                                      SENSOR3_TRIGGER_TIMER_MS,
                                      SENSOR4_TRIGGER_TIMER_MS,
                                      SENSOR5_TRIGGER_TIMER_MS,
                                      SENSOR6_TRIGGER_TIMER_MS,
                                      SENSOR7_TRIGGER_TIMER_MS
                                  };
static uint8_t trigger_count[] = {    SENSOR0_TRIGGERS_CNT,
                                      SENSOR1_TRIGGERS_CNT,
                                      SENSOR2_TRIGGERS_CNT,
                                      SENSOR3_TRIGGERS_CNT,
                                      SENSOR4_TRIGGERS_CNT,
                                      SENSOR5_TRIGGERS_CNT,
                                      SENSOR6_TRIGGERS_CNT,
                                      SENSOR7_TRIGGERS_CNT
                                 };
static uint8_t sensor_i2c_addr[] = {  SENSOR0_I2C_ADDR,
                                      SENSOR1_I2C_ADDR,
                                      SENSOR2_I2C_ADDR,
                                      SENSOR3_I2C_ADDR,
                                      SENSOR4_I2C_ADDR,
                                      SENSOR5_I2C_ADDR,
                                      SENSOR6_I2C_ADDR,
                                      SENSOR7_I2C_ADDR
                                   };
static uint8_t sensor_spi_cs[] = {    SENSOR0_DEVICE_SPI_CS,
                                      SENSOR1_DEVICE_SPI_CS,
                                      SENSOR2_DEVICE_SPI_CS,
                                      SENSOR3_DEVICE_SPI_CS,
                                      SENSOR4_DEVICE_SPI_CS,
                                      SENSOR5_DEVICE_SPI_CS,
                                      SENSOR6_DEVICE_SPI_CS,
                                      SENSOR7_DEVICE_SPI_CS
                                 };
static uint8_t sensor_write_buffer_size[] = { SENSOR0_WRITE_BUF_SIZE,
                                              SENSOR1_WRITE_BUF_SIZE,
                                              SENSOR2_WRITE_BUF_SIZE,
                                              SENSOR3_WRITE_BUF_SIZE,
                                              SENSOR4_WRITE_BUF_SIZE,
                                              SENSOR5_WRITE_BUF_SIZE,
                                              SENSOR6_WRITE_BUF_SIZE,
                                              SENSOR7_WRITE_BUF_SIZE
                                            };
static uint32_t sensor_rx_burst_size[] = {    SENSOR0_RX_BURST_LEN,
                                              SENSOR1_RX_BURST_LEN,
                                              SENSOR2_RX_BURST_LEN,
                                              SENSOR3_RX_BURST_LEN,
                                              SENSOR4_RX_BURST_LEN,
                                              SENSOR5_RX_BURST_LEN,
                                              SENSOR6_RX_BURST_LEN,
                                              SENSOR7_RX_BURST_LEN
                                         };
static uint32_t sensor_read_burst_cnt[] = {   SENSOR0_READ_BURST_CNT,
                                              SENSOR1_READ_BURST_CNT,
                                              SENSOR2_READ_BURST_CNT,
                                              SENSOR3_READ_BURST_CNT,
                                              SENSOR4_READ_BURST_CNT,
                                              SENSOR5_READ_BURST_CNT,
                                              SENSOR6_READ_BURST_CNT,
                                              SENSOR7_READ_BURST_CNT
                                          };

struct HAL_SENSOR_ENGINE_CFG_T sensor_cfg[HAL_SENSOR_ENGINE_MASTER_QTY][SENSOR_NUM];

static void hal_sensor_eng_external_input_pin_irqhandler(enum HAL_GPIO_PIN_T pin)
{
#ifdef SENSOR_ENGINE_DATA_LOG
    POSSIBLY_UNUSED bool gpio_lvl = 0;
    gpio_lvl = hal_gpio_pin_get_val(pin);

    SENSOR_ENGINE_TRACE(2, "hal_sensor_eng_external_irqhandler io=%d val=%d", pin, gpio_lvl);
#endif
}

static void sensor_timer_test_handler(enum HAL_SENSOR_ENGINE_MASTER_T master, enum HAL_SENSOR_ENGINE_ID_T id, enum HAL_SENSOR_ENGINE_DEVICE_T device, const uint8_t *buf, uint32_t len,
                                      enum HAL_SENSOR_ENGINE_READ_WRITE_T read_write)
{
    if (read_write == HAL_SENSOR_ENGINE_WRITE) {
        SENSOR_ENGINE_TRACE(5, "%s: master=%d sensor enging id=%d device=%d ***write complete***", __FUNCTION__, master, id, device);
    } else {
#ifdef SENSOR_ENGINE_DATA_LOG
        SENSOR_ENGINE_TRACE(TR_ATTR_NO_LF, "%s: master=%d sensor enging id=%d device=%d len=%u data=", __FUNCTION__, master, id, device, len);

        uint8_t *addr;

        if (SENSOR_NUM == 1) {
            id = 0;
        }
        addr = sensor_cfg[master][id].rx_buffer_addr + sensor_cfg[master][id].rx_buffer_size;
#if 0
        TRACE_IMM("data=0x%x len=%d", *buf, len);
        TRACE_IMM("addr=%p,rx_buffer_addr=%p,buf=%p", addr, sensor_cfg[master][id].rx_buffer_addr, buf);
#endif
        if (buf + len > addr) {
            SENSOR_ENGINE_DUMP8("%02x ", buf, addr - buf);
            SENSOR_ENGINE_DUMP8("%02x ", sensor_cfg[master][id].rx_buffer_addr, len - (addr - buf));
        } else {
            SENSOR_ENGINE_DUMP8("%02x ", buf, len);
        }
#endif
    }
}

static void sensor_gpio_test_handler(enum HAL_SENSOR_ENGINE_MASTER_T master, enum HAL_SENSOR_ENGINE_ID_T id, enum HAL_SENSOR_ENGINE_DEVICE_T device, const uint8_t *buf, uint32_t len,
                                     enum HAL_SENSOR_ENGINE_READ_WRITE_T read_write)
{
    if (read_write == HAL_SENSOR_ENGINE_WRITE) {
        SENSOR_ENGINE_TRACE(5, "%s: master=%d sensor enging id=%d device=%d ***write complete***", __FUNCTION__, master, id, device);
    } else {
#ifdef SENSOR_ENGINE_DATA_LOG
        SENSOR_ENGINE_TRACE(TR_ATTR_NO_LF, "%s: master=%d sensor enging id=%d device=%d len=%u data=", __FUNCTION__, master, id, device, len);

        uint8_t *addr;

        if (SENSOR_NUM == 1) {
            id = 0;
        }
        addr = sensor_cfg[master][id].rx_buffer_addr + sensor_cfg[master][id].rx_buffer_size;
        if (buf + len > addr) {
            SENSOR_ENGINE_DUMP8("%02x ", buf, addr - buf);
            SENSOR_ENGINE_DUMP8("%02x ", sensor_cfg[master][id].rx_buffer_addr, len - (addr - buf));
        } else {
            SENSOR_ENGINE_DUMP8("%02x ", buf, len);
        }
#endif
    }
}

static int sensor_eng_cfg(enum HAL_SENSOR_ENGINE_MASTER_T master, enum HAL_SENSOR_ENGINE_TRIGGER_T trigger_type)
{
    uint8_t i;
    int ret = 1;

#if (SENSOR_NUM >= 1 )
    static uint8_t write_buf0[SENSOR0_WRITE_BUF_SIZE];
    static uint8_t read_buf0[SENSOR0_READ_BURST_CNT][sizeof(write_buf0)*SENSOR0_RX_BURST_LEN];

    write_buf0[0] = SENSOR0_WRITE_BUF_DATA;
#endif
#if (SENSOR_NUM >= 2 )
    static uint8_t write_buf1[SENSOR1_WRITE_BUF_SIZE];
    static uint8_t read_buf1[SENSOR1_READ_BURST_CNT][sizeof(write_buf1)*SENSOR1_RX_BURST_LEN];

    write_buf1[0] = SENSOR1_WRITE_BUF_DATA;
#endif
#if (SENSOR_NUM >= 3 )
    static uint8_t write_buf2[SENSOR2_WRITE_BUF_SIZE];
    static uint8_t read_buf2[SENSOR2_READ_BURST_CNT][sizeof(write_buf2)*SENSOR2_RX_BURST_LEN];

    write_buf2[0] = SENSOR2_WRITE_BUF_DATA;
#endif
#if (SENSOR_NUM >= 4 )
    static uint8_t write_buf3[SENSOR3_WRITE_BUF_SIZE];
    static uint8_t read_buf3[SENSOR3_READ_BURST_CNT][sizeof(write_buf3)*SENSOR3_RX_BURST_LEN];

    write_buf3[0] = SENSOR3_WRITE_BUF_DATA;
#endif
#if (SENSOR_NUM >= 5 )
    static uint8_t write_buf4[SENSOR4_WRITE_BUF_SIZE];
    static uint8_t read_buf4[SENSOR4_READ_BURST_CNT][sizeof(write_buf4)*SENSOR4_RX_BURST_LEN];

    write_buf4[0] = SENSOR4_WRITE_BUF_DATA;
#endif
#if (SENSOR_NUM >= 6 )
    static uint8_t write_buf5[SENSOR5_WRITE_BUF_SIZE];
    static uint8_t read_buf5[SENSOR5_READ_BURST_CNT][sizeof(write_buf5)*SENSOR5_RX_BURST_LEN];

    write_buf5[0] = SENSOR5_WRITE_BUF_DATA;
#endif
#if (SENSOR_NUM >= 7 )
    static uint8_t write_buf6[SENSOR6_WRITE_BUF_SIZE];
#ifndef VCNL36826S_WRITE_ENABLE
    static uint8_t read_buf6[SENSOR6_READ_BURST_CNT][sizeof(write_buf6)*SENSOR6_RX_BURST_LEN];

    write_buf6[0] = SENSOR6_WRITE_BUF_DATA;
#else
    write_buf6[0] = SENSOR6_WRITE_BUF_DATA;
    write_buf6[1] = 0xA5;
    write_buf6[2] = 0x0A;
#endif
#endif
#if (SENSOR_NUM >= 8 )
    static uint8_t write_buf7[SENSOR7_WRITE_BUF_SIZE];
#ifndef LIS3DH_WRITE_ENABLE
    static uint8_t read_buf7[SENSOR7_READ_BURST_CNT][sizeof(write_buf7)*SENSOR7_RX_BURST_LEN];

    write_buf7[0] = SENSOR7_WRITE_BUF_DATA;
#else
    write_buf7[0] = SENSOR7_WRITE_BUF_DATA;
    write_buf7[1] = 0xAA;//user write data
#endif
#endif

    /*-------------------------config i2c,but donot enable------------------------*/
#if (sensor_engine_i2c_slave_cnt > 0)
    struct HAL_I2C_CONFIG_T cfg_i2c;
    enum   HAL_I2C_ID_T id_i2c[sensor_engine_i2c_master_cnt];
    for (i = 0; i < sensor_engine_i2c_master_cnt; i++) {
        id_i2c[i] = sensor_eng_device_controller_list[i];
    }

    memset(&cfg_i2c, 0, sizeof(cfg_i2c));
    cfg_i2c.mode = HAL_I2C_API_MODE_SENSOR_ENGINE;
    cfg_i2c.as_master = 1;
    cfg_i2c.speed = 400 * 1000;

    for (i = 0; i < sensor_engine_i2c_master_cnt; i++) {
#ifdef FPGA
        if (id_i2c[i] == 0) {
            hal_iomux_set_i2c0();
        }
#ifdef I2C1_BASE
        else if (id_i2c[i] == 1) {
            hal_iomux_set_i2c1();
        }
#endif
#ifdef I2C2_BASE
        else if (id_i2c[i] == 2) {
            hal_iomux_set_i2c2();
        }
#endif
#ifdef I2C3_BASE
        else if (id_i2c[i] == 3) {
            hal_iomux_set_i2c3();
        }
#endif
#ifdef I2C4_BASE
        else if (id_i2c[i] == 4) {
            hal_iomux_set_i2c4();
        }
#endif
#ifdef I2C5_BASE
        else if (id_i2c[i] == 5) {
            hal_iomux_set_i2c5();
        }
#endif
#endif
        ret = hal_i2c_open(id_i2c[i], &cfg_i2c);
        if (ret) {
            ASSERT(false, "%d,%s: i2c open failed:%d", __LINE__, __FUNCTION__, ret);
            return ret;
        }
    }
#endif

    /*-------------------------config spi,but donot enable------------------------*/
#if (sensor_engine_spi_slave_cnt > 0)
    struct HAL_SPI_CFG_T cfg_spi;
#if 0   //multi spi controller use,2021-01-08
    enum   HAL_SPI_ID_T id_spi[sensor_engine_spi_master_cnt];
    for (i = 0; i < sensor_engine_spi_master_cnt; i++) {
        id_spi[i] = sensor_eng_device_controller_list[i + sensor_engine_i2c_master_cnt];
    }
#endif

    memset(&cfg_spi, 0, sizeof(cfg_spi));
    cfg_spi.clk_delay_half = true;
    cfg_spi.clk_polarity = true;
    cfg_spi.skip_en = true;
    cfg_spi.slave = false;
    cfg_spi.dma_rx = true;
    cfg_spi.dma_tx = true;
    cfg_spi.rx_sep_line = false;
    cfg_spi.cs = sensor_spi_cs[sensor_engine_i2c_slave_cnt];
    cfg_spi.rate = 6 * 1000 * 1000;
    cfg_spi.rx_bits = 8;
    cfg_spi.tx_bits = 8;
    cfg_spi.rx_frame_bits = 0;

    switch (sensor_eng_device_controller_list[sensor_engine_i2c_master_cnt]) {
        case __HAL_ISPI__: {
            hal_iomux_set_spi();
            ret = hal_ispi_open(&cfg_spi);
            if (ret) {
                ASSERT(false, "%s:%d, ispi open failed:%d", __FUNCTION__, __LINE__ , ret);
                return ret;
            }
            break;
        }
#ifdef SPI_BASE
        case __HAL_SPI_ID_0__: {
            hal_iomux_set_spi();
            ret = hal_spi_open(&cfg_spi);
            if (ret) {
                ASSERT(false, "%s:%d, spi open failed:%d", __FUNCTION__, __LINE__ , ret);
                return ret;
            }
            break;
        }
#endif
#ifdef SPILCD_BASE
        case __HAL_SPI_ID_SLCD__: {
            hal_iomux_set_spilcd();
            ret = hal_spilcd_open(&cfg_spi);
            if (ret) {
                ASSERT(false, "%s:%d, spilcd open failed:%d", __FUNCTION__, __LINE__ , ret);
                return ret;
            }
            break;
        }
#endif
        default: {
            ASSERT(false, "%s:%d,spi id error", __FUNCTION__, __LINE__);
            break;
        }
    }
#endif

#if (SENSOR_NUM >= 1 )
    i = 0;
    memset(&sensor_cfg[master][i], 0, sizeof(sensor_cfg[master][i]));
    sensor_cfg[master][i].id = sensor_sensor_engine_id[i];
    sensor_cfg[master][i].device = sensor_sensor_engine_device[i];
    sensor_cfg[master][i].trigger_type = trigger_type;
    if (trigger_type == HAL_SENSOR_ENGINE_TRIGGER_TIMER) {
        sensor_cfg[master][i].timer_period_ms = trigger_timer[i];
        sensor_cfg[master][i].handler = sensor_timer_test_handler;
    } else {
        sensor_cfg[master][i].trigger_gpio = trigger_gpio_pin[i];
        sensor_cfg[master][i].handler = sensor_gpio_test_handler;
    }
    sensor_cfg[master][i].interrupt_count = trigger_count[i];
    sensor_cfg[master][i].data_to_vad = SENSORX_CODEC_SEL;
    sensor_cfg[master][i].i2c_device_address = sensor_i2c_addr[i];
    sensor_cfg[master][i].spi_device_cs = sensor_spi_cs[i];
    sensor_cfg[master][i].spi_device_rxds = 1;
    sensor_cfg[master][i].spi_id = sensor_eng_device_controller_list[sensor_engine_i2c_master_cnt];
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_READ;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = HAL_SENSOR_ENGINE_DATA_BYTE;
    sensor_cfg[master][i].tx_buffer_addr = write_buf0;
    sensor_cfg[master][i].rx_buffer_addr = read_buf0;
    sensor_cfg[master][i].tx_buffer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_buffer_size = sensor_read_burst_cnt[i] * sensor_rx_burst_size[i] * 1;
    sensor_cfg[master][i].tx_transfer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_transfer_size = sensor_rx_burst_size[i];
#endif
#if (SENSOR_NUM >= 2 )
    i = 1;
    memset(&sensor_cfg[master][i], 0, sizeof(sensor_cfg[master][i]));
    sensor_cfg[master][i].id = sensor_sensor_engine_id[i];
    sensor_cfg[master][i].device = sensor_sensor_engine_device[i];
    sensor_cfg[master][i].trigger_type = trigger_type;
    if (trigger_type == HAL_SENSOR_ENGINE_TRIGGER_TIMER) {
        sensor_cfg[master][i].timer_period_ms = trigger_timer[i];
        sensor_cfg[master][i].handler = sensor_timer_test_handler;
    } else {
        sensor_cfg[master][i].trigger_gpio = trigger_gpio_pin[i];
        sensor_cfg[master][i].handler = sensor_gpio_test_handler;
    }
    sensor_cfg[master][i].interrupt_count = trigger_count[i];
    sensor_cfg[master][i].data_to_vad = SENSORX_CODEC_SEL;
    sensor_cfg[master][i].i2c_device_address = sensor_i2c_addr[i];
    sensor_cfg[master][i].spi_device_cs = sensor_spi_cs[i];
    sensor_cfg[master][i].spi_device_rxds = 1;
    sensor_cfg[master][i].spi_id = sensor_eng_device_controller_list[sensor_engine_i2c_master_cnt];
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_READ;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = HAL_SENSOR_ENGINE_DATA_BYTE;
    sensor_cfg[master][i].tx_buffer_addr = write_buf1;
    sensor_cfg[master][i].rx_buffer_addr = read_buf1;
    sensor_cfg[master][i].tx_buffer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_buffer_size = sensor_read_burst_cnt[i] * sensor_rx_burst_size[i] * 1;
    sensor_cfg[master][i].tx_transfer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_transfer_size = sensor_rx_burst_size[i];
#endif
#if (SENSOR_NUM >= 3 )
    i = 2;
    memset(&sensor_cfg[master][i], 0, sizeof(sensor_cfg[master][i]));
    sensor_cfg[master][i].id = sensor_sensor_engine_id[i];
    sensor_cfg[master][i].device = sensor_sensor_engine_device[i];
    sensor_cfg[master][i].trigger_type = trigger_type;
    if (trigger_type == HAL_SENSOR_ENGINE_TRIGGER_TIMER) {
        sensor_cfg[master][i].timer_period_ms = trigger_timer[i];
        sensor_cfg[master][i].handler = sensor_timer_test_handler;
    } else {
        sensor_cfg[master][i].trigger_gpio = trigger_gpio_pin[i];
        sensor_cfg[master][i].handler = sensor_gpio_test_handler;
    }
    sensor_cfg[master][i].interrupt_count = trigger_count[i];
    sensor_cfg[master][i].data_to_vad = SENSORX_CODEC_SEL;
    sensor_cfg[master][i].i2c_device_address = sensor_i2c_addr[i];
    sensor_cfg[master][i].spi_device_cs = sensor_spi_cs[i];
    sensor_cfg[master][i].spi_device_rxds = 1;
    sensor_cfg[master][i].spi_id = sensor_eng_device_controller_list[sensor_engine_i2c_master_cnt];
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_READ;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = HAL_SENSOR_ENGINE_DATA_BYTE;
    sensor_cfg[master][i].tx_buffer_addr = write_buf2;
    sensor_cfg[master][i].rx_buffer_addr = read_buf2;
    sensor_cfg[master][i].tx_buffer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_buffer_size = sensor_read_burst_cnt[i] * sensor_rx_burst_size[i] * 1;
    sensor_cfg[master][i].tx_transfer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_transfer_size = sensor_rx_burst_size[i];
#endif
#if (SENSOR_NUM >= 4 )
    i = 3;
    memset(&sensor_cfg[master][i], 0, sizeof(sensor_cfg[master][i]));
    sensor_cfg[master][i].id = sensor_sensor_engine_id[i];
    sensor_cfg[master][i].device = sensor_sensor_engine_device[i];
    sensor_cfg[master][i].trigger_type = trigger_type;
    if (trigger_type == HAL_SENSOR_ENGINE_TRIGGER_TIMER) {
        sensor_cfg[master][i].timer_period_ms = trigger_timer[i];
        sensor_cfg[master][i].handler = sensor_timer_test_handler;
    } else {
        sensor_cfg[master][i].trigger_gpio = trigger_gpio_pin[i];
        sensor_cfg[master][i].handler = sensor_gpio_test_handler;
    }
    sensor_cfg[master][i].interrupt_count = trigger_count[i];
    sensor_cfg[master][i].data_to_vad = SENSORX_CODEC_SEL;
    sensor_cfg[master][i].i2c_device_address = sensor_i2c_addr[i];
    sensor_cfg[master][i].spi_device_cs = sensor_spi_cs[i];
    sensor_cfg[master][i].spi_device_rxds = 1;
    sensor_cfg[master][i].spi_id = sensor_eng_device_controller_list[sensor_engine_i2c_master_cnt];
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_READ;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = HAL_SENSOR_ENGINE_DATA_BYTE;
    sensor_cfg[master][i].tx_buffer_addr = write_buf3;
    sensor_cfg[master][i].rx_buffer_addr = read_buf3;
    sensor_cfg[master][i].tx_buffer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_buffer_size = sensor_read_burst_cnt[i] * sensor_rx_burst_size[i] * 1;
    sensor_cfg[master][i].tx_transfer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_transfer_size = sensor_rx_burst_size[i];
#endif
#if (SENSOR_NUM >= 5 )
    i = 4;
    memset(&sensor_cfg[master][i], 0, sizeof(sensor_cfg[master][i]));
    sensor_cfg[master][i].id = sensor_sensor_engine_id[i];
    sensor_cfg[master][i].device = sensor_sensor_engine_device[i];
    sensor_cfg[master][i].trigger_type = trigger_type;
    if (trigger_type == HAL_SENSOR_ENGINE_TRIGGER_TIMER) {
        sensor_cfg[master][i].timer_period_ms = trigger_timer[i];
        sensor_cfg[master][i].handler = sensor_timer_test_handler;
    } else {
        sensor_cfg[master][i].trigger_gpio = trigger_gpio_pin[i];
        sensor_cfg[master][i].handler = sensor_gpio_test_handler;
    }
    sensor_cfg[master][i].interrupt_count = trigger_count[i];
    sensor_cfg[master][i].data_to_vad = SENSORX_CODEC_SEL;
    sensor_cfg[master][i].i2c_device_address = sensor_i2c_addr[i];
    sensor_cfg[master][i].spi_device_cs = sensor_spi_cs[i];
    sensor_cfg[master][i].spi_device_rxds = 1;
    sensor_cfg[master][i].spi_id = sensor_eng_device_controller_list[sensor_engine_i2c_master_cnt];
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_READ;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = HAL_SENSOR_ENGINE_DATA_BYTE;
    sensor_cfg[master][i].tx_buffer_addr = write_buf4;
    sensor_cfg[master][i].rx_buffer_addr = read_buf4;
    sensor_cfg[master][i].tx_buffer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_buffer_size = sensor_read_burst_cnt[i] * sensor_rx_burst_size[i] * 1;
    sensor_cfg[master][i].tx_transfer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_transfer_size = sensor_rx_burst_size[i];
#endif
#if (SENSOR_NUM >= 6 )
    i = 5;
    memset(&sensor_cfg[master][i], 0, sizeof(sensor_cfg[master][i]));
    sensor_cfg[master][i].id = sensor_sensor_engine_id[i];
    sensor_cfg[master][i].device = sensor_sensor_engine_device[i];
    sensor_cfg[master][i].trigger_type = trigger_type;
    if (trigger_type == HAL_SENSOR_ENGINE_TRIGGER_TIMER) {
        sensor_cfg[master][i].timer_period_ms = trigger_timer[i];
        sensor_cfg[master][i].handler = sensor_timer_test_handler;
    } else {
        sensor_cfg[master][i].trigger_gpio = trigger_gpio_pin[i];
        sensor_cfg[master][i].handler = sensor_gpio_test_handler;
    }
    sensor_cfg[master][i].interrupt_count = trigger_count[i];
    sensor_cfg[master][i].data_to_vad = SENSORX_CODEC_SEL;
    sensor_cfg[master][i].i2c_device_address = sensor_i2c_addr[i];
    sensor_cfg[master][i].spi_device_cs = sensor_spi_cs[i];
    sensor_cfg[master][i].spi_device_rxds = 1;
    sensor_cfg[master][i].spi_id = sensor_eng_device_controller_list[sensor_engine_i2c_master_cnt];
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_READ;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = HAL_SENSOR_ENGINE_DATA_BYTE;
    sensor_cfg[master][i].tx_buffer_addr = write_buf5;
    sensor_cfg[master][i].rx_buffer_addr = read_buf5;
    sensor_cfg[master][i].tx_buffer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_buffer_size = sensor_read_burst_cnt[i] * sensor_rx_burst_size[i] * 1;
    sensor_cfg[master][i].tx_transfer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_transfer_size = sensor_rx_burst_size[i];
#endif
#if (SENSOR_NUM >= 7 )
    i = 6;
    memset(&sensor_cfg[master][i], 0, sizeof(sensor_cfg[master][i]));
    sensor_cfg[master][i].id = sensor_sensor_engine_id[i];
    sensor_cfg[master][i].device = sensor_sensor_engine_device[i];
    sensor_cfg[master][i].trigger_type = trigger_type;
    if (trigger_type == HAL_SENSOR_ENGINE_TRIGGER_TIMER) {
        sensor_cfg[master][i].timer_period_ms = trigger_timer[i];
        sensor_cfg[master][i].handler = sensor_timer_test_handler;
    } else {
        sensor_cfg[master][i].trigger_gpio = trigger_gpio_pin[i];
        sensor_cfg[master][i].handler = sensor_gpio_test_handler;
    }
    sensor_cfg[master][i].interrupt_count = trigger_count[i];
    sensor_cfg[master][i].data_to_vad = SENSORX_CODEC_SEL;
    sensor_cfg[master][i].i2c_device_address = sensor_i2c_addr[i];
    sensor_cfg[master][i].spi_device_cs = sensor_spi_cs[i];
    sensor_cfg[master][i].spi_device_rxds = 1;
    sensor_cfg[master][i].spi_id = sensor_eng_device_controller_list[sensor_engine_i2c_master_cnt];
#ifndef VCNL36826S_WRITE_ENABLE
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_READ;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = HAL_SENSOR_ENGINE_DATA_BYTE;
    sensor_cfg[master][i].tx_buffer_addr = write_buf6;
    sensor_cfg[master][i].rx_buffer_addr = read_buf6;
    sensor_cfg[master][i].tx_buffer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_buffer_size = sensor_read_burst_cnt[i] * sensor_rx_burst_size[i] * 1;
    sensor_cfg[master][i].tx_transfer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_transfer_size = sensor_rx_burst_size[i];
#else
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_WRITE;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = 0;
    sensor_cfg[master][i].tx_buffer_addr = write_buf6;
    sensor_cfg[master][i].rx_buffer_addr = 0;
    sensor_cfg[master][i].tx_buffer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_buffer_size = 0;
    sensor_cfg[master][i].tx_transfer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_transfer_size = 0;
#endif
#endif
#if (SENSOR_NUM >= 8 )
    i = 7;
    memset(&sensor_cfg[master][i], 0, sizeof(sensor_cfg[master][i]));
    sensor_cfg[master][i].id = sensor_sensor_engine_id[i];
    sensor_cfg[master][i].device = sensor_sensor_engine_device[i];
    sensor_cfg[master][i].trigger_type = trigger_type;
    if (trigger_type == HAL_SENSOR_ENGINE_TRIGGER_TIMER) {
        sensor_cfg[master][i].timer_period_ms = trigger_timer[i];
        sensor_cfg[master][i].handler = sensor_timer_test_handler;
    } else {
        sensor_cfg[master][i].trigger_gpio = trigger_gpio_pin[i];
        sensor_cfg[master][i].handler = sensor_gpio_test_handler;
    }
    sensor_cfg[master][i].interrupt_count = trigger_count[i];
    sensor_cfg[master][i].data_to_vad = SENSORX_CODEC_SEL;
    sensor_cfg[master][i].i2c_device_address = sensor_i2c_addr[i];
    sensor_cfg[master][i].spi_device_cs = sensor_spi_cs[i];
    sensor_cfg[master][i].spi_device_rxds = 1;
    sensor_cfg[master][i].spi_id = sensor_eng_device_controller_list[sensor_engine_i2c_master_cnt];

#ifndef LIS3DH_WRITE_ENABLE
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_READ;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = HAL_SENSOR_ENGINE_DATA_BYTE;
    sensor_cfg[master][i].tx_buffer_addr = write_buf7;
    sensor_cfg[master][i].rx_buffer_addr = read_buf7;
    sensor_cfg[master][i].tx_buffer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_buffer_size = sensor_read_burst_cnt[i] * sensor_rx_burst_size[i] * 1;
    sensor_cfg[master][i].tx_transfer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_transfer_size = sensor_rx_burst_size[i];
#else
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_WRITE;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = 0;
    sensor_cfg[master][i].tx_buffer_addr = write_buf7;
    sensor_cfg[master][i].rx_buffer_addr = 0;
    sensor_cfg[master][i].tx_buffer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_buffer_size = 0;
    sensor_cfg[master][i].tx_transfer_size = sensor_write_buffer_size[i];
    sensor_cfg[master][i].rx_transfer_size = 0;
#endif
#endif

    for (i = 0; i < SENSOR_NUM; i++) {
        hal_sensor_engine_cfg(master, &sensor_cfg[master][i]);
    }
    hal_sensor_engine_open(master);
    SENSOR_ENGINE_TRACE(0, "----sensor engine cfg and opening has been completed----");
    SENSOR_ENGINE_TRACE(0, "---Please wait for sensor engine's completion interrupt---");
    SENSOR_ENGINE_TRACE(0, "  ");

    return 0;
}

int hal_sensor_eng_periodic_timer_test(void)
{
    sensor_eng_cfg(SENSOR_ENG_MASTER_ID, HAL_SENSOR_ENGINE_TRIGGER_TIMER);
    while(1) {
        hal_sleep_enter_sleep();
    }

    return 0;
}
int hal_sensor_eng_external_input_test(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP ext_pin[] = {
        {SENSOR0_TRIGGER_GPIO, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#if (SENSOR_NUM >= 2)
        {SENSOR1_TRIGGER_GPIO, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#endif
#if (SENSOR_NUM >= 3)
        {SENSOR2_TRIGGER_GPIO, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#endif
#if (SENSOR_NUM >= 4)
        {SENSOR3_TRIGGER_GPIO, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#endif
#if (SENSOR_NUM >= 5)
        {SENSOR4_TRIGGER_GPIO, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#endif
#if (SENSOR_NUM >= 6)
        {SENSOR5_TRIGGER_GPIO, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#endif
#if (SENSOR_NUM >= 7)
        {SENSOR6_TRIGGER_GPIO, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#endif
#if (SENSOR_NUM >= 8)
        {SENSOR7_TRIGGER_GPIO, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#endif
    };

    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    uint8_t i;

    sensor_eng_cfg(SENSOR_ENG_MASTER_ID, HAL_SENSOR_ENGINE_TRIGGER_GPIO);

    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)ext_pin, sizeof(ext_pin) / sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP));
    for (i = 0; i < SENSOR_NUM; i++) {
        hal_gpio_pin_set_dir(trigger_gpio_pin[i], HAL_GPIO_DIR_IN, 1);
    }
    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_handler = hal_sensor_eng_external_input_pin_irqhandler;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    for (i = 0; i < SENSOR_NUM; i++) {
        hal_gpio_setup_irq(trigger_gpio_pin[i], &gpiocfg);
    }
    while (1) {
        hal_sleep_enter_sleep();
    }

    return 0;
}

void sensor_engine_test(void)
{
    SENSOR_ENGINE_TRACE(0, "   ");
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    SENSOR_ENGINE_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SENSOR_ENGINE_TRACE(0, "------sensor engine v2 test started------");
    SENSOR_ENGINE_TRACE(1, "CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SENSOR_ENGINE_TRACE(0, "   ");

#ifdef TEST_MODE_PERIODIC_TIMER
    SENSOR_ENGINE_TRACE(0, "------periodic timer mode------");
    hal_sensor_eng_periodic_timer_test();
#else
    SENSOR_ENGINE_TRACE(0, "------external input mode------");
    hal_sensor_eng_external_input_test();
#endif
}

#endif

