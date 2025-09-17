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
#ifdef SDMMC_TEST

#include <string.h>
#include <stdlib.h>
#include "hal_gpio.h"
#include "hal_sdmmc.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_location.h"
#ifdef RTOS
    #include "cmsis_os.h"
#endif

#if 1
    #define SDMMC_TRACE(n, s, ...)      TRACE(n, s, ##__VA_ARGS__)
    #define SDMMC_DUMP8(n, s, ...)      DUMP8(n, s, ##__VA_ARGS__)
    #define SDMMC_DUMP16(n, s, ...)     DUMP16(n, s, ##__VA_ARGS__)
    #define SDMMC_DUMP32(n, s, ...)     DUMP32(n, s, ##__VA_ARGS__)
#else
    #define SDMMC_TRACE(n, s, ...)      TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define SDMMC_DUMP8(n, s, ...)
    #define SDMMC_DUMP16(n, s, ...)
    #define SDMMC_DUMP32(n, s, ...)
#endif

#if 1
    #define SDMMC_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
    #define SDMMC_DELAY_US(us)          hal_sys_timer_delay_us(us)
#else
    #define SDMMC_DELAY_MS(ms)
    #define SDMMC_DELAY_US(us)
#endif

/*************************Function configuration area**************************/
#define SDMMC_ID        HAL_SDMMC_ID_0
#define SD_DETECT_EN
#define SD_DETECT_PIN   HAL_GPIO_PIN_P0_2
//#define SD_OPEN_WITH_INT_LOCK
//#define SD_POLLING_MODE //default dma mode
/******************************************************************************/

enum HAL_CARD_LAST_STATUS_T {
    HAL_CARD_LAST_STATUS_UNKNOWN = 0,
    HAL_CARD_LAST_STATUS_INSERTED,
    HAL_CARD_LAST_STATUS_NOT_INSERTED,
};

static enum HAL_CARD_LAST_STATUS_T open_sd_card = HAL_CARD_LAST_STATUS_UNKNOWN;

#ifdef SD_DETECT_EN
static void hal_sd_detect_pin_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    bool gpio_lvl;
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;

    gpio_lvl = hal_gpio_pin_get_val(pin);
    SDMMC_TRACE(2, "%s, io=%d val=%d", __func__, pin, gpio_lvl);
    if (gpio_lvl) {
        open_sd_card = HAL_CARD_LAST_STATUS_NOT_INSERTED; //close card
    } else {
        open_sd_card = HAL_CARD_LAST_STATUS_INSERTED;   //open card
    }

    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_polarity = gpio_lvl ? HAL_GPIO_IRQ_POLARITY_LOW_FALLING : HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_handler = hal_sd_detect_pin_irq_handler;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;
    hal_gpio_setup_irq(SD_DETECT_PIN, &gpiocfg);
}

static void hal_enable_sd_detect_pin(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP ext_pin[] = {
        {SD_DETECT_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;

    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)ext_pin, sizeof(ext_pin) / sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP));
    hal_gpio_pin_set_dir(SD_DETECT_PIN, HAL_GPIO_DIR_IN, 1);

    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
    gpiocfg.irq_handler = hal_sd_detect_pin_irq_handler;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;
    hal_gpio_setup_irq(SD_DETECT_PIN, &gpiocfg);
}
#endif

static void rand_generator(uint32_t *dest, uint32_t size)
{
    while (size--) {
        *dest++ = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * 0xFFFFFFFF);
    }
}

POSSIBLY_UNUSED static void hal_sdmmc_signal_voltage_switch(enum mmc_voltage voltage)
{
#if defined(CHIP_BEST2002) || defined(CHIP_BEST2005)
    if (voltage == MMC_SIGNAL_VOLTAGE_180) {
        hal_iomux_set_sdio_voltage(HAL_IOMUX_PIN_VOLTAGE_MEM);
    } else if (voltage == MMC_SIGNAL_VOLTAGE_330) {
        hal_iomux_set_sdio_voltage(HAL_IOMUX_PIN_VOLTAGE_VIO);
    } else {
        SDMMC_TRACE(0, "******error voltage: %d", voltage);
    }
#endif

    //Complete signal voltage switching within 5ms
    SDMMC_TRACE(0, "------sdmmc signal voltage switching completed, %s", (voltage == 2) ? "1.8v" : (voltage == 4) ? "3.3v" : "unsupported voltage");
}

POSSIBLY_UNUSED static struct HAL_SDMMC_CB_T sdmmc_test_cb = {
    hal_sdmmc_signal_voltage_switch,
    NULL,
    NULL,
};

#define TEST_SIZE_MAX   (128 * 1024 *1024 / 512)//128MB
#if (RAM_SIZE <= 128 * 1024)
    #define BUF_SIZE 2048
#else
    #define BUF_SIZE (32 * 1024)
#endif
SYNC_FLAGS_LOC static uint32_t txbuf[BUF_SIZE / 4];
SYNC_FLAGS_LOC static uint32_t rxbuf[BUF_SIZE / 4];

void sdmmc_test(void)
{
    uint32_t i;
    uint32_t *pbuf_tx;
    uint32_t *pbuf_rx;
    uint32_t sector_index = 0;
    uint8_t enable_read_card = 0;
    int32_t init_result;
    uint32_t sector_size;
    uint32_t sector_count;
    uint32_t raw_status = 0;
    uint32_t current_state = 0;
    uint32_t time_start, time_end[2];
    enum HAL_SDMMC_ERR err;
    struct HAL_SDMMC_CARD_INFO_T card_info;
    POSSIBLY_UNUSED struct HAL_SDMMC_CONFIG_T sdmmc_cfg;

    SDMMC_TRACE(0, "      ");
    SDMMC_TRACE(2, "%s:%d", __func__, __LINE__);
    SDMMC_TRACE(0, "------sdmmc test started,2021-05-28------");
#ifdef SD_POLLING_MODE
    SDMMC_TRACE(0, "--------------polling mode---------------");
#else
    SDMMC_TRACE(0, "----------------dma mode-----------------");
#endif
    SDMMC_TRACE(1, "CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);

#if defined(CHIP_BEST2002) || defined(CHIP_BEST2005)
    enum HAL_IOMUX_PIN_VOLTAGE_DOMAINS_T vol;
    vol = hal_iomux_get_sdio_voltage();
    SDMMC_TRACE(2, "sdio port voltage:%s", (vol == HAL_IOMUX_PIN_VOLTAGE_MEM) ? "VMEM[1]" : "VIO[0]");
    if (vol == HAL_IOMUX_PIN_VOLTAGE_MEM) {
        hal_iomux_set_sdio_voltage(HAL_IOMUX_PIN_VOLTAGE_VIO);
        vol = hal_iomux_get_sdio_voltage();
        SDMMC_TRACE(2, "new sdio port voltage:%s", (vol == HAL_IOMUX_PIN_VOLTAGE_MEM) ? "VMEM[1]" : "VIO[0]");
    }
#endif

    hal_sdmmc_set_delay_func((HAL_SDMMC_DELAY_FUNC)osDelay);
    srand(hal_sys_timer_get());
#ifdef SD_DETECT_EN
    hal_enable_sd_detect_pin();
#else
    open_sd_card = HAL_CARD_LAST_STATUS_INSERTED;
#endif
    while (1) {
        if (open_sd_card != HAL_CARD_LAST_STATUS_UNKNOWN) {
            if (open_sd_card == HAL_CARD_LAST_STATUS_INSERTED) {
                memset(&sdmmc_cfg, 0, sizeof(struct HAL_SDMMC_CONFIG_T));
                sdmmc_cfg.yield = false;
                sdmmc_cfg.ddr_mode = false;
                sdmmc_cfg.volt_switch = false;//speed > 48M, the voltage must be switched
                sdmmc_cfg.bus_speed = 48 * 1000 * 1000;
                sdmmc_cfg.bus_width = HAL_SDMMC_BUS_WIDTH_4;
                sdmmc_cfg.device_type = HAL_SDMMC_DEVICE_TYPE_SD;
                if (sdmmc_cfg.volt_switch) {
                    sdmmc_cfg.callback = &sdmmc_test_cb;
                } else {
                    sdmmc_cfg.callback = NULL;
                }
#ifdef SD_OPEN_WITH_INT_LOCK
#ifdef RTOS
                #error "Interrupts cannot be turned off when RTOS=1"
#endif
                uint32_t lock = int_lock_global();
#endif
                init_result = hal_sdmmc_open(SDMMC_ID, &sdmmc_cfg);
#ifdef SD_OPEN_WITH_INT_LOCK
                int_unlock_global(lock);
#endif
                SDMMC_TRACE(0, "------sdmmc clk final speed:%d", hal_sdmmc_get_bus_speed(SDMMC_ID));
                SDMMC_TRACE(0, "  ");
                SDMMC_TRACE(0, "------sd card is inserted,init result=%d", init_result);
                if (!init_result) {
                    enable_read_card = 1;

                    hal_sdmmc_info(SDMMC_ID, &sector_count, &sector_size);
                    hal_sdmmc_dump(SDMMC_ID);
                    SDMMC_TRACE(2, "------card info:sector_count=0x%X,sector_size=%d", sector_count, sector_size);
                    SDMMC_TRACE(0, "    ");

                    //card info read
                    hal_sdmmc_get_card_info(SDMMC_ID, &card_info);
                    hal_sdmmc_dump_card_info(&card_info);
                } else {
                    enable_read_card = 0;
                    ASSERT(0, "%s:%d,card initialization failed,result=%d", __func__, __LINE__, init_result);
                }
            } else {
                enable_read_card = 0;
                hal_sdmmc_close(SDMMC_ID);
                SDMMC_TRACE(0, "******sd card has been pulled out");
            }
            open_sd_card = HAL_CARD_LAST_STATUS_UNKNOWN;
        }
        if (enable_read_card) {
            rand_generator((uint32_t *)txbuf, BUF_SIZE / sizeof(uint32_t));

            time_start = hal_fast_sys_timer_get();
#ifdef SD_POLLING_MODE
            hal_sdmmc_write_blocks_polling(SDMMC_ID, sector_index, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#else
            hal_sdmmc_write_blocks(SDMMC_ID, sector_index, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#endif
            time_end[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
            time_start = hal_fast_sys_timer_get();
#ifdef SD_POLLING_MODE
            hal_sdmmc_read_blocks_polling(SDMMC_ID, sector_index, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#else
            hal_sdmmc_read_blocks(SDMMC_ID, sector_index, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#endif
            time_end[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

            SDMMC_DUMP32("0x%08X ", (uint32_t *)txbuf, 8);
            SDMMC_DUMP32("0x%08X ", (uint32_t *)rxbuf, 8);
            SDMMC_TRACE(0, "------write time: %uus", time_end[0]);
            SDMMC_TRACE(0, "------read  time: %uus", time_end[1]);

            err = hal_sdmmc_get_card_raw_status(SDMMC_ID, &raw_status);
            if (err) {
                SDMMC_TRACE(0, "------hal_sdmmc_get_card_raw_status error: %d", err);
            } else {
                SDMMC_TRACE(0, "------raw status: 0x%X", raw_status);
            }
            err = hal_sdmmc_get_card_current_state(SDMMC_ID, &current_state);
            if (err) {
                SDMMC_TRACE(0, "------hal_sdmmc_get_card_current_state error: %d", err);
            } else {
                SDMMC_TRACE(0, "------current state: 0x%X", current_state);
            }

            if (BUF_SIZE < 32 * 1024) {
                SDMMC_TRACE(0, "******Warning: Data size is %d, the read and write speed will become lower", BUF_SIZE);
                SDMMC_TRACE(0, "******Warning: LARGE_RAM=1 can be solved");
            }
            time_start = (uint32_t)((float)BUF_SIZE * 954 / 1000.0);//10^6/1024/1024=0.95367431640625
#ifdef NOSTD
            SDMMC_TRACE(2, "------write speed: %dMB/s, read speed: %dMB/s", (uint32_t)time_start / time_end[0], (uint32_t)time_start / time_end[1]);
#else
            SDMMC_TRACE(2, "------write speed: %4.2fMB/s, read speed: %4.2fMB/s", (double)time_start / time_end[0], (double)time_start / time_end[1]);
#endif

            pbuf_tx = (uint32_t *)txbuf;
            pbuf_rx = (uint32_t *)rxbuf;
            for (i = 0; i < BUF_SIZE / sizeof(uint32_t); i++) {
                if (pbuf_tx[i] != pbuf_rx[i]) {
                    SDMMC_TRACE(4, "******sector %d write error,index:%d,[tx]0x%X != [rx]0x%X", sector_index, i, pbuf_tx[i], pbuf_rx[i]);
                }
            }
            SDMMC_TRACE(3, "------write/read size: %dbyte, sector count: %d, start sector: %d", BUF_SIZE, BUF_SIZE / sector_size, sector_index);

            memset(rxbuf, 0, sizeof(rxbuf));
            sector_index += BUF_SIZE / sector_size;
            if (sector_index >= TEST_SIZE_MAX) {
                sector_index = 0;
            }
        } else {
            SDMMC_TRACE(1, "******Card not inserted or removed,card stats=%d", enable_read_card);
        }

        osDelay(1000);
    }
}

#endif

