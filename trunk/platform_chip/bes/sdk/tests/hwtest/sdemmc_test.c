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
#ifdef SDEMMC_TEST

#include <string.h>
#include <stdlib.h>
#include "hal_wdt.h"
#include "hal_gpio.h"
#include "hal_sdmmc.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "hal_location.h"
#ifdef RTOS
    #include "cmsis_os.h"
#endif
#ifdef UTILS_ESHELL_EN
    #include "eshell.h"
#endif

#if 1
    #define SDEMMC_TRACE(n, s, ...)      TRACE(n, s, ##__VA_ARGS__)
    #define SDEMMC_DUMP8(n, s, ...)      DUMP8(n, s, ##__VA_ARGS__)
    #define SDEMMC_DUMP16(n, s, ...)     DUMP16(n, s, ##__VA_ARGS__)
    #define SDEMMC_DUMP32(n, s, ...)     DUMP32(n, s, ##__VA_ARGS__)
#else
    #define SDEMMC_TRACE(n, s, ...)      TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define SDEMMC_DUMP8(n, s, ...)
    #define SDEMMC_DUMP16(n, s, ...)
    #define SDEMMC_DUMP32(n, s, ...)
#endif

#if 0
    #define SDEMMC_TRACE_CB(n, s, ...)  TRACE(n, s, ##__VA_ARGS__)
#else
    #define SDEMMC_TRACE_CB(n, s, ...)  TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

#if 1
    #define SDEMMC_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
    #define SDEMMC_DELAY_US(us)          hal_sys_timer_delay_us(us)
#else
    #define SDEMMC_DELAY_MS(ms)
    #define SDEMMC_DELAY_US(us)
#endif

/*************************Function configuration area**************************/
#define SDEMMC_ID        HAL_SDMMC_ID_1

#define EMMC_SLEEP_ENABLE
//#define EMMC_CACHE_ENABLE
//#define EMMC_RANDOM_WRITE_ENABLE
#define EMMC_PMU_VIO_3V3_ENABLE //used for BES2700IB EVB_Core Board_V1.2/1.3......
//#define EMMC_POLLING_MODE     //default dma mode
//#define EMMC_ERAES_ENABLE
//#define EMMC_POWER_ON         //only used for the boards of QianChen
//#define SDMMC_TX_RX_USE_CALLBACK

//#define EMMC_OPEN_CLOSE_TEST

//#define EMMC_OPEN_WITH_INT_LOCK_TEST

//#define EMMC_RST_OPEN_CLOSE_TEST
//#define EMMC_RST_OPEN_CLOSE_SIMPLE
//#define EMMC_RST_OPEN_CLOSE_RANDOM
/******************************************************************************/
#if defined(EMMC_RST_OPEN_CLOSE_SIMPLE) && defined(EMMC_RST_OPEN_CLOSE_RANDOM)
    #error "EMMC_RST_OPEN_CLOSE_SIMPLE and EMMC_RST_OPEN_CLOSE_RANDOM, the 2 macros can only open one of them"
#endif
#if defined(SDMMC_TX_RX_USE_CALLBACK) && defined(EMMC_POLLING_MODE)
    #error "SDMMC_TX_RX_USE_CALLBACK and EMMC_POLLING_MODE, the 2 macros can only open one of them"
#endif

static void rand_generator(uint32_t *dest, uint32_t size)
{
    while (size--) {
        *dest++ = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * 0xFFFFFFFF);
    }
}

#if defined(SDMMC_TX_RX_USE_CALLBACK)
    static volatile uint8_t sdmmc_txrx_done = 0;
#endif

POSSIBLY_UNUSED static void hal_sdmmc_host_error(enum HAL_SDMMC_HOST_ERR error)
{
    SDEMMC_TRACE(1, "******sdmmc host error:%d", error);

#ifdef EMMC_RST_OPEN_CLOSE_RANDOM
    hal_wdt_stop(HAL_WDT_ID_0);
    while (1) {
        SDEMMC_TRACE(1, "******sdmmc host error:%d", error);
        osDelay(1000);
    }
#endif
}

#if defined(SDMMC_TX_RX_USE_CALLBACK)
static void hal_sdmmc_txrx_done(void)
{
    sdmmc_txrx_done = 1;
    SDEMMC_TRACE_CB(0, "------sdmmc dma txrx done");
}
#endif

POSSIBLY_UNUSED static struct HAL_SDMMC_CB_T sdemmc_test_callback = {
    NULL,//emmc is useless
    hal_sdmmc_host_error,
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    hal_sdmmc_txrx_done,
#else
    NULL,
#endif
};

static void sdemmc_dump32(const char *str, uint32_t *buf, uint32_t size)
{
    uint32_t i;

    TRACE_FLUSH();
    if (str) {
        SDEMMC_TRACE(1, "%s, buf=%x, size=%d", str, (uint32_t)buf, size);
    }
    SDEMMC_TRACE(0, " ");
    for (i = 0; i < size; i += 8) {
        SDEMMC_TRACE(1, "%08X %08X %08X %08X %08X %08X %08X %08X",
                     buf[i + 0], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7]);
    }
    SDEMMC_TRACE(1, " ");
    TRACE_FLUSH();
}

#define MULTI_TEST_SIZE             (2 * 1024 * 1024)//30MBytes
#define MULTI_TEST_CNT              (MULTI_TEST_SIZE / BUF_SIZE)

#if (RAM_SIZE <= 256 * 1024)
    #define BUF_SIZE 2048
#else
    #define BUF_SIZE (64 * 1024)
#endif
#if (BUF_SIZE < 512)
    #error "BUF_SIZE cannot be less than 512"
#endif

SYNC_FLAGS_LOC static uint32_t txbuf[BUF_SIZE / 4];
SYNC_FLAGS_LOC static uint32_t rxbuf[BUF_SIZE / 4];
static struct HAL_SDMMC_CARD_INFO_T card_info;
static void emmc_read_write_test(uint32_t sector_size, uint32_t sector_count)
{
    int ret;
    uint32_t i;
    uint64_t val;
    uint32_t blks;
    uint32_t start_addr;
    uint32_t card_offset = 0;
    uint32_t time_start, time[5];
    uint8_t sleep_status;

#ifdef EMMC_SLEEP_ENABLE
    ret = hal_sdmmc_sleep_card(SDEMMC_ID);
    SDEMMC_TRACE(0, "======card sleep, ret:%d", ret);
    hal_sdmmc_get_card_sleep_status(SDEMMC_ID, &sleep_status);
    SDEMMC_TRACE(0, "======card sleep status: %d(%s)", sleep_status, sleep_status ? "sleep" : "awake");
#endif
    rand_generator((uint32_t *)txbuf, BUF_SIZE / sizeof(uint32_t));

    SDEMMC_TRACE(0, "  ");
#ifdef EMMC_RANDOM_WRITE_ENABLE
    card_offset = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * sector_count);
    SDEMMC_TRACE(0, "------random card offset:0x%X", card_offset);
    if ((uint64_t)card_offset * 512 + MULTI_TEST_SIZE > (uint64_t)sector_count * 512) {
        card_offset = (uint32_t)((uint64_t)sector_count * 512 - MULTI_TEST_SIZE) / 512;
    }
    SDEMMC_TRACE(0, "------final card offset :0x%X", card_offset);
#endif

    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(0, "=================> writing card");
    SDEMMC_TRACE(1, "------card offset=0x%x, buf=0x%x, buf sz=%u", card_offset, (uint32_t)txbuf, BUF_SIZE);
#ifdef EMMC_SLEEP_ENABLE
    ret = hal_sdmmc_awake_card(SDEMMC_ID);
    SDEMMC_TRACE(0, "======card awake, ret:%d", ret);
    hal_sdmmc_get_card_sleep_status(SDEMMC_ID, &sleep_status);
    SDEMMC_TRACE(0, "======card sleep status: %d(%s)", sleep_status, sleep_status ? "sleep" : "awake");
#endif

    //single write test
    SDEMMC_TRACE(1, "------single write start, total size:%u, write size:%u, cnt:1", BUF_SIZE, BUF_SIZE);
    time_start = hal_fast_sys_timer_get();
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
    blks = hal_sdmmc_write_blocks_polling(SDEMMC_ID, card_offset, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#else
    blks = hal_sdmmc_write_blocks(SDEMMC_ID, card_offset, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#endif
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    while (!sdmmc_txrx_done);
    SDEMMC_TRACE_CB(2, "------write done");
#endif
    time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

    //multi write test
    SDEMMC_TRACE(1, "------multi write start,  total size:%uMB, write size:%u, cnt:%u", MULTI_TEST_SIZE / 1024 / 1024, BUF_SIZE, MULTI_TEST_CNT);
    time_start = hal_fast_sys_timer_get();
    for (i = 0; i < MULTI_TEST_CNT - 1; i++) {
        start_addr = card_offset + i * BUF_SIZE / sector_size;

#if defined(SDMMC_TX_RX_USE_CALLBACK)
        sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
        blks = hal_sdmmc_write_blocks_polling(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#else
        blks = hal_sdmmc_write_blocks(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#endif

#if defined(SDMMC_TX_RX_USE_CALLBACK)
        while (!sdmmc_txrx_done);
        SDEMMC_TRACE_CB(2, "------write done");
#endif
    }
    time[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    if (blks == BUF_SIZE / sector_size) {
        SDEMMC_TRACE(1, "------write card complete");
    } else {
        SDEMMC_TRACE(1, "******write card failed, blks=%d", blks);
    }
#ifdef EMMC_SLEEP_ENABLE
    ret = hal_sdmmc_sleep_card(SDEMMC_ID);
    SDEMMC_TRACE(0, "======card sleep, ret:%d", ret);
    hal_sdmmc_get_card_sleep_status(SDEMMC_ID, &sleep_status);
    SDEMMC_TRACE(0, "======card sleep status: %d(%s)", sleep_status, sleep_status ? "sleep" : "awake");
#endif

    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(1, "=================> reading card");
    SDEMMC_TRACE(1, "------card offset=0x%x, buf=0x%x, buf sz=%u", card_offset, (uint32_t)rxbuf, BUF_SIZE);
#ifdef EMMC_SLEEP_ENABLE
    ret = hal_sdmmc_awake_card(SDEMMC_ID);
    SDEMMC_TRACE(0, "======card awake, ret:%d", ret);
    hal_sdmmc_get_card_sleep_status(SDEMMC_ID, &sleep_status);
    SDEMMC_TRACE(0, "======card sleep status: %d(%s)", sleep_status, sleep_status ? "sleep" : "awake");
#endif

    //single read test
    SDEMMC_TRACE(1, "------single read start, total size:%u, read size:%u, cnt:1", BUF_SIZE, BUF_SIZE);
    time_start = hal_fast_sys_timer_get();
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
    blks = hal_sdmmc_read_blocks_polling(SDEMMC_ID, card_offset, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#else
    blks = hal_sdmmc_read_blocks(SDEMMC_ID, card_offset, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#endif
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    while (!sdmmc_txrx_done);
    SDEMMC_TRACE_CB(2, "------read done");
#endif
    time[2] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

    //multi read test
    SDEMMC_TRACE(1, "------multi read start,  total size:%uMB, read size:%d, cnt:%d", MULTI_TEST_SIZE / 1024 / 1024, BUF_SIZE, MULTI_TEST_CNT);
    time_start = hal_fast_sys_timer_get();
    for (i = 0; i < MULTI_TEST_CNT - 1; i++) {
        start_addr = card_offset + i * BUF_SIZE / sector_size;

#if defined(SDMMC_TX_RX_USE_CALLBACK)
        sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
        blks = hal_sdmmc_read_blocks_polling(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#else
        blks = hal_sdmmc_read_blocks(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#endif
#if defined(SDMMC_TX_RX_USE_CALLBACK)
        while (!sdmmc_txrx_done);
        SDEMMC_TRACE_CB(2, "------read done");
#endif
    }
    time[3] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    if (blks == BUF_SIZE / sector_size) {
        SDEMMC_TRACE(1, "------read card complete");
    } else {
        SDEMMC_TRACE(1, "******read card failed, blks=%d", blks);
    }

    //compare data
    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(1, "=================> checking card data");
    time_start = hal_fast_sys_timer_get();
    for (i = 0; i < MULTI_TEST_CNT - 1; i++) {
        start_addr = card_offset + i * BUF_SIZE / sector_size;

#if defined(SDMMC_TX_RX_USE_CALLBACK)
        sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
        hal_sdmmc_read_blocks_polling(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#else
        hal_sdmmc_read_blocks(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#endif
#if defined(SDMMC_TX_RX_USE_CALLBACK)
        while (!sdmmc_txrx_done);
        SDEMMC_TRACE_CB(2, "------read done");
#endif
        ret = memcmp((uint32_t *)txbuf, (uint32_t *)rxbuf, BUF_SIZE / sector_size);
        if (ret) {
            SDEMMC_TRACE(1, "check data failed, ret=%d, line=%d", ret, __LINE__);
            sdemmc_dump32("dump src", txbuf, BUF_SIZE / sector_size);
            sdemmc_dump32("dump dst", rxbuf, BUF_SIZE / sector_size);
            SDEMMC_TRACE(1, "halt for check card data error");
            while (1);
        }
    }
    time[4] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    SDEMMC_TRACE(1, "=================$ data check is OK!!!");
#ifdef EMMC_SLEEP_ENABLE
    ret = hal_sdmmc_sleep_card(SDEMMC_ID);
    SDEMMC_TRACE(0, "======card sleep, ret:%d", ret);
    hal_sdmmc_get_card_sleep_status(SDEMMC_ID, &sleep_status);
    SDEMMC_TRACE(0, "======card sleep status: %d(%s)", sleep_status, sleep_status ? "sleep" : "awake");
#endif

    //calc speed
    SDEMMC_TRACE(1, "  ");
    SDEMMC_TRACE(1, "==========read and write success==========");
    SDEMMC_TRACE(1, "******************************************");
    SDEMMC_TRACE(1, "TIME COST        : read       [%uus]", time[2]);
    SDEMMC_TRACE(1, "TIME COST        : multi read [%uus]", time[3]);
    SDEMMC_TRACE(1, "TIME COST        : write      [%uus]", time[0]);
    SDEMMC_TRACE(1, "TIME COST        : multi write[%uus]", time[1]);
    SDEMMC_TRACE(1, "TIME COST        : multi cmp  [%uus]", time[4]);
    SDEMMC_TRACE(1, "  ");
    SDEMMC_TRACE(1, "******************************************");
    SDEMMC_TRACE(1, "read size        :%ubytes", BUF_SIZE);
    val = (uint64_t)BUF_SIZE * MULTI_TEST_CNT / 1024;
    SDEMMC_TRACE(1, "multi read size  :%uKbytes=%uMbytes", (uint32_t)val, (uint32_t)(val / 1024));
    SDEMMC_TRACE(1, "write size       :%ubytes", BUF_SIZE);
    SDEMMC_TRACE(1, "multi write size :%uKbytes=%uMbytes", (uint32_t)val, (uint32_t)(val / 1024));
    SDEMMC_TRACE(1, "cmp size         :%ubytes", BUF_SIZE);
    SDEMMC_TRACE(1, "multi cmp size   :%uKbytes=%uMbytes", (uint32_t)val, (uint32_t)(val / 1024));
    SDEMMC_TRACE(1, "  ");

    //speed=MB/s
    //     =size/1024/1024MB/(time*10^-6)s
    //     =size/1024/1024*1000000/time (MB/s)
    //     =size*1000000/1024/1024/time (MB/s)
    //     =size*0.954/time (MB/s)
    //     =size*954/1000/time (MB/s)
    //     =size*95400/1000/time (MB/s) //The result is magnified 100 times and used to display 2 decimal places
    time[2] = (uint32_t)((uint64_t)BUF_SIZE * 95400 / 1000 / time[2]);                  //read
    time[3] = (uint32_t)((uint64_t)BUF_SIZE * 95400 * MULTI_TEST_CNT / 1000 / time[3]); //multi read
    time[0] = (uint32_t)((uint64_t)BUF_SIZE * 95400 / 1000 / time[0]);                  //write
    time[1] = (uint32_t)((uint64_t)BUF_SIZE * 95400 * MULTI_TEST_CNT / 1000 / time[1]); //multi write

    SDEMMC_TRACE(1, "emmc cache       :%s", card_info.cache_ctrl ? "ON" : "OFF");
    SDEMMC_TRACE(1, "  ");
    SDEMMC_TRACE(1, "read speed       :%u.%uMB/s", time[2] / 100, time[2] % 100);
    SDEMMC_TRACE(1, "multi read speed :%u.%uMB/s", time[3] / 100, time[3] % 100);
    SDEMMC_TRACE(1, "write speed      :%u.%uMB/s", time[0] / 100, time[0] % 100);
    SDEMMC_TRACE(1, "multi write speed:%u.%uMB/s", time[1] / 100, time[1] % 100);
    SDEMMC_TRACE(1, "******************************************");
    SDEMMC_TRACE(1, "  ");
}

void sdemmc_test(void)
{
    int32_t run_cnt;
    enum HAL_SDMMC_ERR ret;
    uint32_t sector_size = 0;
    uint32_t sector_count = 0;
    POSSIBLY_UNUSED struct HAL_SDMMC_CONFIG_T emmc_cfg;

    SDEMMC_TRACE(0, "  ");
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    SDEMMC_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SDEMMC_TRACE(0, "-------emmc test started,2022-06-01------");
#ifdef EMMC_POLLING_MODE
    SDEMMC_TRACE(0, "--------------polling mode---------------");
#else
    SDEMMC_TRACE(0, "----------------dma mode-----------------");
#endif
    SDEMMC_TRACE(0, "  ");

#ifdef EMMC_POWER_ON
#define USER_GPIO1    HAL_GPIO_PIN_P9_2
#define USER_GPIO2    HAL_GPIO_PIN_P9_4
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
        {USER_GPIO1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {USER_GPIO2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(USER_GPIO1, HAL_GPIO_DIR_OUT, 1);//output high
    hal_gpio_pin_set_dir(USER_GPIO2, HAL_GPIO_DIR_OUT, 1);//output high
#endif

#ifdef EMMC_PMU_VIO_3V3_ENABLE
    extern void pmu_vio_3v3(bool en);
    pmu_vio_3v3(true);
#endif

#ifdef EMMC_RST_OPEN_CLOSE_TEST
#define EMMC_RST_PIN                    HAL_GPIO_PIN_P0_6
#define CLEAR_FLAG_ADDR                 0x20120000
#define PASS_RESULT_ADDR                0x20121000
#define ERROR_RESULT_ADDR               0x20122000
#define HW_WRITE_REG(_Reg,_Value)       (*((volatile uint32_t*)(_Reg)) = (uint32_t)(_Value))
#define HW_READ_REG(_Reg)               (*((volatile uint32_t*)(_Reg)))

    if (HW_READ_REG(CLEAR_FLAG_ADDR)) {
        HW_WRITE_REG(CLEAR_FLAG_ADDR, 0);
        HW_WRITE_REG(PASS_RESULT_ADDR, 0);
        HW_WRITE_REG(ERROR_RESULT_ADDR, 0);
    }

    uint32_t err_cnt;
    uint32_t pass_cnt;
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP rst_gpio[] = {
        {EMMC_RST_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(rst_gpio, ARRAY_SIZE(rst_gpio));
    hal_gpio_pin_set_dir(EMMC_RST_PIN, HAL_GPIO_DIR_OUT, 0);//output high

    //reset emmc
    hal_gpio_pin_clr(EMMC_RST_PIN);
    SDEMMC_TRACE(0, "---set emmc rst pin low");
    osDelay(5);
    hal_gpio_pin_set(EMMC_RST_PIN);
    SDEMMC_TRACE(0, "---set emmc rst pin high");
    SDEMMC_TRACE(0, "    ");
    osDelay(50);
#endif

#ifdef EMMC_RST_OPEN_CLOSE_RANDOM
    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(0, "===wdt reset/sdemmc success cnt:%d, err cnt:%d", HW_READ_REG(PASS_RESULT_ADDR), HW_READ_REG(ERROR_RESULT_ADDR));
    SDEMMC_TRACE(0, "  ");

    uint8_t random_time;
    //gen [5,22]ms random time
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());
    random_time = (uint8_t)(rand() % (22 - 5 + 1) + 5);
    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(0, "===random_time:%d", random_time);
    SDEMMC_TRACE(0, "  ");

    //wdt reset
    hal_wdt_set_timeout_ms(HAL_WDT_ID_0, random_time);
    hal_wdt_start(HAL_WDT_ID_0);
    SDEMMC_TRACE(0, "Wait DIG WDT reset start...");
    SDEMMC_TRACE(0, "  ");
#endif

    memset(&emmc_cfg, 0, sizeof(struct HAL_SDMMC_CONFIG_T));
    emmc_cfg.yield = false;
    emmc_cfg.ddr_mode = false;
    emmc_cfg.volt_switch = false;//useless
    emmc_cfg.bus_speed = 48 * 1000 * 1000;//sdr52
    emmc_cfg.bus_width = HAL_SDMMC_BUS_WIDTH_8;
    emmc_cfg.device_type = HAL_SDMMC_DEVICE_TYPE_EMMC;
    emmc_cfg.callback = &sdemmc_test_callback;

#ifdef EMMC_OPEN_WITH_INT_LOCK_TEST
#ifdef RTOS
    #error "Interrupts cannot be turned off when RTOS=1"
#endif
    uint32_t lock = int_lock_global();
#endif

    ret = hal_sdmmc_open(SDEMMC_ID, &emmc_cfg);

#ifdef EMMC_OPEN_WITH_INT_LOCK_TEST
    int_unlock_global(lock);
#endif

#ifdef EMMC_RST_OPEN_CLOSE_TEST
    if (ret) {
        err_cnt = HW_READ_REG(ERROR_RESULT_ADDR);
        err_cnt++;
        HW_WRITE_REG(ERROR_RESULT_ADDR, err_cnt);

        SDEMMC_TRACE(0, "    ");
        SDEMMC_TRACE(0, "***hal_sdmmc_open fail:%d", ret);
        hal_sdmmc_close(SDEMMC_ID);
        SDEMMC_TRACE(0, "---hal_sdmmc_close");
        SDEMMC_TRACE(0, "    ");
    } else {
        pass_cnt = HW_READ_REG(PASS_RESULT_ADDR);
        pass_cnt++;
        HW_WRITE_REG(PASS_RESULT_ADDR, pass_cnt);

        SDEMMC_TRACE(0, "    ");
        SDEMMC_TRACE(0, "---hal_sdmmc_open success:%d", ret);
        SDEMMC_TRACE(0, "    ");
    }
#ifdef EMMC_RST_OPEN_CLOSE_RANDOM
    while (1) {
        SDEMMC_TRACE(0, "Wait DIG WDT reset end...");
    }
#endif
#else
    SDEMMC_TRACE(0, "-------sdmmc clk final speed:%d", hal_sdmmc_get_bus_speed(SDEMMC_ID));
    SDEMMC_TRACE(0, "  ");
    if (!ret) {
        hal_sdmmc_info(SDEMMC_ID, &sector_count, &sector_size);
        hal_sdmmc_dump(SDEMMC_ID);
        SDEMMC_TRACE(2, "------sdmmc info:sector_count=0x%X,sector_size=%d", sector_count, sector_size);
        SDEMMC_TRACE(0, "    ");
    } else {
        ASSERT(0, "%s:%d,sdmmc initialization failed,result=%d", __func__, __LINE__, ret);
    }
#endif

#ifdef EMMC_RST_OPEN_CLOSE_SIMPLE
    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(0, "===wdt reset/sdemmc success cnt:%d, err cnt:%d", HW_READ_REG(PASS_RESULT_ADDR), HW_READ_REG(ERROR_RESULT_ADDR));
    SDEMMC_TRACE(0, "  ");

    //wdt reset
    hal_wdt_set_timeout_ms(HAL_WDT_ID_0, 100);
    hal_wdt_start(HAL_WDT_ID_0);
    while (1) {
        SDEMMC_TRACE(0, "Wait DIG WDT reset...");
        osDelay(100);
    }
#endif

#ifdef EMMC_CACHE_ENABLE
    ret = hal_sdmmc_cache_control(SDEMMC_ID, 1);
    SDEMMC_TRACE(0, "emmc cache open result: %d(%s)", ret, ret ? "failed" : "success");
#endif

    //card info read
    hal_sdmmc_get_card_info(SDEMMC_ID, &card_info);
    hal_sdmmc_dump_card_info(&card_info);
    hal_sdmmc_dump_reg(SDEMMC_ID);

#ifdef EMMC_ERAES_ENABLE
    uint32_t blks;

    //erase test
    SDEMMC_TRACE(0, "-------------erase test-------------");
    blks = hal_sdmmc_read_blocks(SDEMMC_ID, sector_index, 1, (uint8_t *)rxbuf);
    SDEMMC_TRACE(0, "read blocks:%d", blks);
    SDEMMC_TRACE(0, "before erase data: ");
    SDEMMC_DUMP32("0x%08X ", (uint32_t *)rxbuf, 8);
    blks = hal_sdmmc_erase_dma(SDEMMC_ID, sector_index, 1);
    SDEMMC_TRACE(0, "erase blocks:%d", blks);
    hal_sdmmc_read_blocks(SDEMMC_ID, sector_index, 1, (uint8_t *)rxbuf);
    SDEMMC_TRACE(0, "read blocks:%d", blks);
    SDEMMC_TRACE(0, "after erase data: ");
    SDEMMC_DUMP32("0x%08X ", (uint32_t *)rxbuf, 8);
    SDEMMC_TRACE(0, "------------------------------------");
    SDEMMC_TRACE(0, "  ");
#endif

    run_cnt = 1;
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());
    while (run_cnt--) {
        emmc_read_write_test(sector_size, sector_count);

#ifdef EMMC_OPEN_CLOSE_TEST
        hal_sdmmc_close(SDEMMC_ID);
        SDEMMC_TRACE(0, "  ");
        SDEMMC_TRACE(0, "------------------------------------");
        SDEMMC_TRACE(0, "---hal_sdmmc_close");
        SDEMMC_TRACE(0, "------------------------------------");
        ret = hal_sdmmc_open(SDEMMC_ID, &emmc_cfg);
        SDEMMC_TRACE(0, "------------------------------------");
        if (ret) {
            SDEMMC_TRACE(0, "***hal_sdmmc_open fail:%d", ret);
        } else {
            SDEMMC_TRACE(0, "---hal_sdmmc_open success:%d", ret);
        }
        SDEMMC_TRACE(0, "------------------------------------");
        SDEMMC_TRACE(0, "  ");
        run_cnt = 100;
#endif
        if (run_cnt) {
            osDelay(500);
        }
    }
    SDEMMC_TRACE(0, "---------emmc read and write test ends---------");
}

#ifdef UTILS_ESHELL_EN
static void watch_sdemmc_test_event(int argc, char *argv[])
{
    eshell_putstring_nl("Auto Test command:sdemmc_test\n");
    sdemmc_test();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_AUTO_TEST, "sdemmc_test", "sdemmc_test", watch_sdemmc_test_event);
#endif

#endif
