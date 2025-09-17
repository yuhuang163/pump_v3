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
 * spi pin
 * master   slave[spi0]
 * clk ---- clk [gpio14]
 * cs ----- cs  [gpio15]
 * mosi --- miso[gpio16]
 * miso --- mosi[gpio17]
 ****************************************************************************/
#if defined(TEST_SPI_SLAVE) || defined (TEST_SYSVIEW)
#include "string.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_spi.h"
#include "hal_gpio.h"
#include "hal_sysfreq.h"
#include "test_spi.h"
#include "test_spi_slave.h"
#ifdef SPI_WAKEUP
#include "hal_sleep.h"
#endif

static volatile bes_spi_share_memory_struct_t _spi_shm;
static volatile bes_spi_share_memory_struct_t *spi_shm = &_spi_shm;

#ifndef MCU_SPI_SLAVE_SPECIFY_MEM
#define TEST_MTU_SIEZE 4096
static unsigned char master_tx_buff[TEST_MTU_SIEZE];
static unsigned char slave_tx_buff[TEST_MTU_SIEZE];
#else
#define TEST_MTU_SIEZE MCU_SPI_SLAVE_BUF_SIZE
extern uint8_t __spi_share_mem_master_tx_start[];
extern uint8_t __spi_share_mem_slave_tx_start[];
static uint8_t *master_tx_buff = __spi_share_mem_master_tx_start;
static uint8_t *slave_tx_buff  = __spi_share_mem_slave_tx_start;
#endif

#ifdef USE_BBK_SPI_IF
#define LEN_SIZE    4
#define DATA_SIZE   4096
#define CRC_SIZE    4
#define BUF_SIZE  (LEN_SIZE + DATA_SIZE + CRC_SIZE)
static uint8_t tx_buf[BUF_SIZE];
static uint8_t rx_buf[BUF_SIZE];
#else
#define CRC_SIZE    4
#define BUF_SIZE  (1024+CRC_SIZE)
static uint8_t tx_buf[BUF_SIZE];
static uint8_t rx_buf[BUF_SIZE];
#endif

#define SPI_MASTER_ACTION_RX_INDICATE       0
#define SPI_MASTER_ACTION_TX_OK_CONFIRMED   1

#define MASTER_TX_INDICATE_GPIO_PIN         (HAL_GPIO_PIN_P1_0)
#define MASTER_RX_OK_GPIO_PIN               (HAL_GPIO_PIN_P1_1)

#define SLAVE_TX_INDICATE_GPIO_PIN          (HAL_GPIO_PIN_P1_2)
#define SLAVE_RX_OK_GPIO_PIN                (HAL_GPIO_PIN_P1_3)

#ifdef SPI_WAKEUP
static int spi_slave_pm_notify_handler(enum HAL_PM_STATE_T state)
{
    if (state == HAL_PM_STATE_NORMAL_SLEEP || state == HAL_PM_STATE_POWER_DOWN_SLEEP) {
        hal_iomux_clear_spi_slave();
    } else if (state == HAL_PM_STATE_NORMAL_WAKEUP || state == HAL_PM_STATE_POWER_DOWN_WAKEUP) {
        hal_iomux_set_spi_slave();
    }
    return 0;
}

static void spi_slave_pm_notiy_set(bool on)
{
    uint32_t lock;

    lock = int_lock();
    if (on) {
        hal_pm_notif_register(HAL_PM_USER_DRV, spi_slave_pm_notify_handler);
    } else {
        hal_pm_notif_deregister(HAL_PM_USER_DRV, spi_slave_pm_notify_handler);
    }
    int_unlock(lock);
}
#endif

int spi_slave_device_open(void)
{
    unsigned int crc = 0;

    //hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    // spi
    hal_iomux_set_spi_slave();

    // share memory
    spi_shm->master_tx_address = (unsigned int)master_tx_buff;
    spi_shm->master_tx_len = TEST_MTU_SIEZE;
    spi_shm->slave_tx_address = (unsigned int)slave_tx_buff;
    spi_shm->slave_tx_len = TEST_MTU_SIEZE;
    crc = pal_crc32(0, (unsigned char *)spi_shm, sizeof(*spi_shm)-4);
    spi_shm->crc = crc;

    *((volatile unsigned int *)BEST_SHARE_MEMORY_ADDRESS_POINTER) = (unsigned int)spi_shm;

    TRACE(3,"bes_spi_slave_open:share_address_content=0x%x,crc=0x%x",
        *((volatile unsigned int *)BEST_SHARE_MEMORY_ADDRESS_POINTER), crc);
    TRACE(3,"m tx addr=0x%x,m tx_len=%d,s tx_addr=0x%x,s tx_len=%d",spi_shm->master_tx_address,spi_shm->master_tx_len\
    ,spi_shm->slave_tx_address,spi_shm->slave_tx_len);

#ifdef SPI_WAKEUP
    spi_slave_pm_notiy_set(true);
#endif

    return 0;
}

void spi_slave_device_read(unsigned char *buf, unsigned int buf_len)
{
    if((void *)spi_shm->master_tx_address == NULL) {
        TRACE(3, "SPI addr is null");
    }
	memcpy(buf, (void *)spi_shm->master_tx_address, buf_len);
}

void spi_slave_device_write(unsigned char *buf, unsigned int buf_len)
{
	memcpy((void *)spi_shm->slave_tx_address, buf, buf_len);
}

void spi_slave_tx_indicate(char set)
{
    if (set) {
        hal_gpio_pin_set(SLAVE_TX_INDICATE_GPIO_PIN);
    } else {
        hal_gpio_pin_clr(SLAVE_TX_INDICATE_GPIO_PIN);
    }
}

void spi_slave_rx_ok(char set)
{
    if (set) {
        hal_gpio_pin_set(SLAVE_RX_OK_GPIO_PIN);
    } else {
        hal_gpio_pin_clr(SLAVE_RX_OK_GPIO_PIN);
    }
}


/**
 * new:
 * #define LEN_SIZE    4
 * #define DATA_SIZE   1024
 * #define CRC_SIZE    4
 * #define BUF_SIZE  (LEN_SIZE + DATA_SIZE + CRC_SIZE)
 * static uint8_t tx_buf[BUF_SIZE];
 * static uint8_t rx_buf[BUF_SIZE];
 * 
 * len: sizeof(buf)
 * len + buf + crc
 * 
 */
#ifdef USE_BBK_SPI_IF
static void xtc_spi_slave_read(void)
{
    uint32_t tx_crc;
    uint32_t rx_crc;
    uint32_t data_len;

    //slave rx
    spi_slave_rx_ok(0);
    spi_slave_device_read(rx_buf, BUF_SIZE);
    rx_crc = pal_crc32(0, (const uint8_t *)rx_buf, (LEN_SIZE + DATA_SIZE));
    memcpy(&tx_crc, (rx_buf + LEN_SIZE + DATA_SIZE), CRC_SIZE);
    memcpy(&data_len, rx_buf, sizeof(uint32_t));
    if (rx_crc == tx_crc) {
        TRACE(5, "---[s rx success]crc32:0x%08X,rx size:%d data_len=%d", rx_crc, BUF_SIZE - CRC_SIZE, data_len);
    } else {
        TRACE(6, "***[s rx fail]tx crc32:0x%08X,rx crc32:0x%08X,rx size:%d", tx_crc, rx_crc, BUF_SIZE - CRC_SIZE);
    }
    spi_slave_rx_ok(1);
}

void xtc_spi_slave_write(uint8_t *buf, uint32_t len)
{
    uint32_t tx_crc;

    spi_slave_tx_indicate(0);
    *(uint32_t *)(tx_buf) = len;
    memcpy(tx_buf + LEN_SIZE, buf, len);
    tx_crc = pal_crc32(0, (const uint8_t *)tx_buf, (LEN_SIZE + DATA_SIZE));
    memcpy(tx_buf + LEN_SIZE + DATA_SIZE, &tx_crc, CRC_SIZE);
    spi_slave_device_write(tx_buf, BUF_SIZE);
    spi_slave_tx_indicate(1);
}

#define TRIGGER_LEN 4096
uint8_t spi_dump_buf[TRIGGER_LEN];

void xtc_spi_slave_dump_crash(uint8_t *buf, uint32_t len, uint8_t status)
{
    static uint32_t length = 0;
    static uint8_t *ptr = spi_dump_buf;
    uint32_t _len;

    if (status == DUMP_CRASH_END) {
#ifdef USE_SPI_DUMP_CRASH
        *ptr = '\0';
        length += 1;
#endif
        xtc_spi_slave_write(spi_dump_buf, length);
        ptr = spi_dump_buf;
        length = 0;
        return ;
    }

    while (length + len >= TRIGGER_LEN) {
        _len = TRIGGER_LEN - length;
        memcpy(ptr, buf, _len);
        xtc_spi_slave_write(spi_dump_buf, TRIGGER_LEN);
        hal_sys_timer_delay(MS_TO_TICKS(5));

        ptr = spi_dump_buf;
        buf += _len;
        len -= _len;
        length = 0;
    }

    memcpy(ptr, buf, len);
    ptr += len;
    length += len;
}

#else

static void spi_slave_read(void)
{
    uint32_t tx_crc;
    uint32_t rx_crc;

    //slave rx
    spi_slave_rx_ok(0);
    spi_slave_device_read(rx_buf, BUF_SIZE);
    rx_crc = pal_crc32(0, (const uint8_t *)rx_buf, (BUF_SIZE - CRC_SIZE));
    memcpy(&tx_crc, (rx_buf + BUF_SIZE - CRC_SIZE), CRC_SIZE);
    if (rx_crc == tx_crc) {
        TRACE(5, "---[s rx success]crc32:0x%08X,rx size:%d", rx_crc, BUF_SIZE - CRC_SIZE);
    } else {
        TRACE(6, "***[s rx fail]tx crc32:0x%08X,rx crc32:0x%08X,rx size:%d", tx_crc, rx_crc, BUF_SIZE - CRC_SIZE);
    }
    spi_slave_rx_ok(1);

    //slave tx case
    spi_slave_tx_indicate(0);
    spi_slave_device_write(rx_buf, BUF_SIZE);
    spi_slave_tx_indicate(1);
}
#endif

static void spi_master_irq_handle(uint8_t status)
{
    if(status == SPI_MASTER_ACTION_TX_OK_CONFIRMED) {
#ifdef USE_BBK_SPI_IF
        xtc_spi_slave_read();
#else
        spi_slave_read();
#endif
#ifdef SPI_WAKEUP
        hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
#endif
    }
    else if(status == SPI_MASTER_ACTION_RX_INDICATE){
    }
}

static void _spi_master_gpio_irq(enum HAL_GPIO_PIN_T pin)
{
    if (pin == MASTER_TX_INDICATE_GPIO_PIN) {
        spi_master_irq_handle(SPI_MASTER_ACTION_TX_OK_CONFIRMED);
    }
    else if (pin == MASTER_RX_OK_GPIO_PIN) {
        spi_master_irq_handle(SPI_MASTER_ACTION_RX_INDICATE);
    }
}

static void set_slave_iomux(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    // gpio
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_trigger_master_pinmux[] = {
        {MASTER_TX_INDICATE_GPIO_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
        {MASTER_RX_OK_GPIO_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
        {SLAVE_TX_INDICATE_GPIO_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
        {SLAVE_RX_OK_GPIO_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(gpio_trigger_master_pinmux, ARRAY_SIZE(gpio_trigger_master_pinmux));

    // output pin
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SLAVE_TX_INDICATE_GPIO_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SLAVE_RX_OK_GPIO_PIN, HAL_GPIO_DIR_OUT, 0);

    // input pin
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)MASTER_TX_INDICATE_GPIO_PIN, HAL_GPIO_DIR_IN, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)MASTER_RX_OK_GPIO_PIN, HAL_GPIO_DIR_IN, 1);

    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_handler = _spi_master_gpio_irq;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    hal_gpio_setup_irq(MASTER_TX_INDICATE_GPIO_PIN, &gpiocfg);
    hal_gpio_setup_irq(MASTER_RX_OK_GPIO_PIN, &gpiocfg);
}

#ifdef SPI_WAKEUP
int gpio_irq_count = 0;
static void wakeup_handler(enum HAL_GPIO_PIN_T pin)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TRACE(0, "%s: gpio_irq_count=%d ", __func__, gpio_irq_count);
    gpio_irq_count ++ ;
}

#define WAKEUP_PIN HAL_GPIO_PIN_P0_0
static void wakeup_gpio_init()
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
        {WAKEUP_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(WAKEUP_PIN, HAL_GPIO_DIR_IN, 0);
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg;
    gpio_irq_cfg.irq_enable = 1;
    gpio_irq_cfg.irq_debounce = 0;
    gpio_irq_cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpio_irq_cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpio_irq_cfg.irq_handler = wakeup_handler;
    hal_gpio_setup_irq(WAKEUP_PIN, &gpio_irq_cfg);
}
#endif

void spi_slave_test(void)
{
    TRACE(3,"SPI SLAVE TEST");
    spi_slave_device_open();
    set_slave_iomux();

#ifdef SPI_WAKEUP
    wakeup_gpio_init();
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
    TR_INFO(TR_MOD(MAIN), "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));
#endif

#ifdef TEST_SYSVIEW
    osDelay(5000);
#else
    while(1)
    {
        TRACE(3 , "test spi slave...");
        osDelay(1000);
    }
#endif
}
#endif
