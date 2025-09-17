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
#include <string.h>
#include <stdlib.h>
#include "hal_sdmmc.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#ifdef RTOS
    #include "cmsis_os.h"
#endif

#ifdef  CHIP_FUNCTION_TEST_SDMMC

#ifndef SDMMC_TEST_BLOCK_COUNT
#define SDMMC_TEST_BLOCK_COUNT 64
#endif
/*--------------Test Case Switch-----------------*/
#define SDMMC_TUNING_TEST_ON                     (0x1 << 0)
#define SDMMC_GET_INFO_TEST_ON                   (0x1 << 1)
#define SDMMC_SINGLE_BLOCK_WRITE_TEST_ON         (0x1 << 2)
#define SDMMC_SINGLE_BLOCK_READ_TEST_ON          (0x1 << 3)
#define SDMMC_MULT_BLOCK_WRITE_TEST_ON           (0x1 << 4)
#define SDMMC_MULT_BLOCK_READ_TEST_ON            (0x1 << 5)

#define SDMMC_BLOCK_SIZE       512
#define SDMMC_TEST_BUFFER_SIZE (SDMMC_TEST_BLOCK_COUNT*SDMMC_BLOCK_SIZE)
typedef struct {
#ifdef MMC_SUPPORTS_TUNING
    bool tuning_pass;
#endif
    bool get_info_pass;
    bool single_block_write_pass;
    bool single_block_read_pass;
    bool mult_block_write_pass;
    bool mult_block_read_pass;
    uint32_t single_block_write_speed;
    uint32_t single_block_read_speed;
    uint32_t mult_block_write_speed;
    uint32_t mult_block_read_speed;
    uint32_t sector_count;
    uint32_t sector_size;
}SDMMC_TEST_RESULT;

static SDMMC_TEST_RESULT sdmmc_test_result[HAL_SDMMC_ID_NUM];
static uint32_t sdmmc_test_case_switch[HAL_SDMMC_ID_NUM];

static uint8_t sdmmc_test_write_buffer[SDMMC_TEST_BUFFER_SIZE];
static uint8_t sdmmc_test_read_buffer[SDMMC_TEST_BUFFER_SIZE];
#ifdef SDMMC_TEST_DATA_RESTORE
static uint8_t sdmmc_test_data_buffer[SDMMC_TEST_BUFFER_SIZE];
#endif

#define SDMMC_ID        HAL_SDMMC_ID_1
#define SD_DETECT_PIN   HAL_GPIO_PIN_P0_2

enum HAL_CARD_LAST_STATUS_T {
    HAL_CARD_LAST_STATUS_UNKNOWN = 0,
    HAL_CARD_LAST_STATUS_INSERTED,
    HAL_CARD_LAST_STATUS_NOT_INSERTED,
};

static enum HAL_CARD_LAST_STATUS_T open_sd_card = HAL_CARD_LAST_STATUS_UNKNOWN;

static void hal_sd_detect_pin_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    bool gpio_lvl;
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;

    gpio_lvl = hal_gpio_pin_get_val(pin);
    TRACE(2, "%s, io=%d val=%d", __func__, pin, gpio_lvl);
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

void hal_enable_sd_detect_pin(void)
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


void rand_generator(uint32_t *dest, uint32_t size)
{
    while (size--) {
        *dest++ = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * 0xFFFFFFFF);
    }
}

POSSIBLY_UNUSED static void hal_sdmmc_host_error(enum HAL_SDMMC_HOST_ERR error)
{
    TRACE(1, "******sdmmc host error:%d", error);
}

#if defined(SDMMC_TX_RX_USE_CALLBACK)
static void hal_sdmmc_txrx_done(void)
{
    sdmmc_txrx_done = 1;
    TRACE(0, "------sdmmc dma txrx done");
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


static void _sdmmc_test_result_init(void)
{
    uint32_t i;

    for(i = 0; i < sizeof(sdmmc_test_result)/sizeof(sdmmc_test_result[0]); i++)
    {
#ifdef MMC_SUPPORTS_TUNING
        sdmmc_test_result[i].tuning_pass = false;
#endif
        sdmmc_test_result[i].get_info_pass = false;
        sdmmc_test_result[i].single_block_write_pass = false;
        sdmmc_test_result[i].single_block_read_pass = false;
        sdmmc_test_result[i].mult_block_write_pass = false;
        sdmmc_test_result[i].mult_block_read_pass = false;
        sdmmc_test_result[i].single_block_write_speed = 0;
        sdmmc_test_result[i].single_block_read_speed = 0;
        sdmmc_test_result[i].mult_block_write_speed = 0;
        sdmmc_test_result[i].mult_block_read_speed = 0;
        sdmmc_test_result[i].sector_count = 0;
        sdmmc_test_result[i].sector_size = 0;
    }

    for(i = 0; i < sizeof(sdmmc_test_case_switch)/sizeof(sdmmc_test_case_switch[0]); i++)
    {
        sdmmc_test_case_switch[i] = 0x0;
#ifdef MMC_SUPPORTS_TUNING
        sdmmc_test_case_switch[i] |= SDMMC_TUNING_TEST_ON;
#endif
        sdmmc_test_case_switch[i] |= SDMMC_GET_INFO_TEST_ON;
        sdmmc_test_case_switch[i] |= SDMMC_SINGLE_BLOCK_WRITE_TEST_ON;
        sdmmc_test_case_switch[i] |= SDMMC_SINGLE_BLOCK_READ_TEST_ON;
        sdmmc_test_case_switch[i] |= SDMMC_MULT_BLOCK_WRITE_TEST_ON;
        sdmmc_test_case_switch[i] |= SDMMC_MULT_BLOCK_READ_TEST_ON;
    }

}

static void _sdmmc_test_result_print(enum HAL_FLASH_ID_T id)
{
    uint32_t test_case_switch = sdmmc_test_case_switch[id];
    SDMMC_TEST_RESULT *test_result = &sdmmc_test_result[id];

    TRACE(1, "\n----------------SDMMC %d TEST RESULT---------------------\n", id);
#ifdef MMC_SUPPORTS_TUNING
    if(test_case_switch & SDMMC_TUNING_TEST_ON)
    {
        if(test_result->tuning_pass)
        {
            TRACE(1, "SDMMC TUNING TEST:               PASS");
        }
        else
        {
            TRACE(1, "SDMMC TUNING TEST:               NG!");
        }
    }
#endif
    if(test_case_switch & SDMMC_GET_INFO_TEST_ON)
    {
        if(test_result->get_info_pass)
        {
            TRACE(1, "SDMMC GET INFO TEST:             PASS");
        }
        else
        {
            TRACE(1, "SDMMC GET INFO TEST:             NG!");
        }
    }

    if(test_case_switch & SDMMC_SINGLE_BLOCK_WRITE_TEST_ON)
    {
        if(test_result->single_block_write_pass)
        {
            TRACE(1, "SDMMC SINGLE BLOCK WRITE TEST:   PASS");
        }
        else
        {
            TRACE(1, "SDMMC SINGLE BLOCK WRITE TEST:   NG!");
        }
    }

    if(test_case_switch & SDMMC_SINGLE_BLOCK_READ_TEST_ON)
    {
        if(test_result->single_block_read_pass)
        {
            TRACE(1, "SDMMC SINGLE BLOCK READ TEST:    PASS");
        }
        else
        {
            TRACE(1, "SDMMC SINGLE BLOCK READ TEST:    NG!");
        }
    }

    if(test_case_switch & SDMMC_MULT_BLOCK_WRITE_TEST_ON)
    {
        if(test_result->mult_block_write_pass)
        {
            TRACE(1, "SDMMC MULT BLOCK WRITE TEST:     PASS");
        }
        else
        {
            TRACE(1, "SDMMC MULT BLOCK WRITE TEST:     NG!");
        }
    }

    if(test_case_switch & SDMMC_MULT_BLOCK_READ_TEST_ON)
    {
        if(test_result->mult_block_read_pass)
        {
            TRACE(1, "SDMMC MULT BLOCK READ TEST:      PASS");
        }
        else
        {
            TRACE(1, "SDMMC MULT BLOCK READ TEST:      NG!");
        }
    }

    if(test_case_switch & SDMMC_GET_INFO_TEST_ON)
    {
        if(test_result->get_info_pass)
        {
            TRACE(1, "SDMMC INFO: SECTOR COUNT:        %08d ", test_result->sector_count);
            TRACE(1, "SDMMC INFO: SECOTR SIZE:         %08d(Byte)",  test_result->sector_size);
        }
    }

    if(test_case_switch & SDMMC_SINGLE_BLOCK_WRITE_TEST_ON)
    {
        if(test_result->single_block_write_pass)
        {
            TRACE(1, "SINGLE BLOCK WRITE SPEED:        %08d(KB/s)", test_result->single_block_write_speed/1024);
        }
    }

    if(test_case_switch & SDMMC_SINGLE_BLOCK_READ_TEST_ON)
    {
        if(test_result->single_block_read_pass)
        {
            TRACE(1, "SINGLE BLOCK READ SPEED:         %08d(KB/s)", test_result->single_block_read_speed/1024);
        }
    }

    if(test_case_switch & SDMMC_MULT_BLOCK_WRITE_TEST_ON)
    {
        if(test_result->mult_block_write_pass)
        {
            TRACE(1, "MULT BLOCK WRITE SPEED:          %08d(KB/s)", test_result->mult_block_write_speed/1024);
        }
    }

    if(test_case_switch & SDMMC_MULT_BLOCK_READ_TEST_ON)
    {
        if(test_result->mult_block_read_pass)
        {
            TRACE(1, "MULT BLOCK READ SPEED:           %08d(KB/s)", test_result->mult_block_read_speed/1024);
        }
    }

    TRACE(1, "\n-----------------------------------------------------------\n");
}

#ifdef MMC_SUPPORTS_TUNING
int _sdmmc_test_tuning(enum HAL_SDMMC_ID_T id)
{
    return 0;
}
#endif

int _sdmmc_test_get_info(enum HAL_SDMMC_ID_T id)
{
    uint32_t sector_count = 0;
    uint32_t sector_size = 0;

    hal_sdmmc_info(id, &sector_count, &sector_size);
    if(sector_count > 0 && sector_size > 0)
    {
        sdmmc_test_result[id].sector_count = sector_count;
        sdmmc_test_result[id].sector_size = sector_size;
        sdmmc_test_result[id].get_info_pass = true;
        TRACE(1, "%s: sector count: %d, sector size:%d", __func__, sector_count, sector_size);
        TRACE(1, "%s: done", __func__);
        return 0;
    }
    else
    {
        TRACE(1, "%s: failed", __func__);
        return -1;
    }
}


int _sdmmc_test_wr(enum HAL_SDMMC_ID_T id, uint32_t block_count)
{
    uint32_t start_block;
    uint32_t test_block_count;
    uint32_t i,blk;
    uint8_t* wbuff = (uint8_t*)sdmmc_test_write_buffer;
    uint8_t* rbuff = (uint8_t*)sdmmc_test_read_buffer;
#ifdef SDMMC_TEST_DATA_RESTORE
    uint8_t* bbuff = (uint8_t*)sdmmc_test_data_buffer;
#endif
    int ret;
    int err_code = 0;
    uint32_t start_time, end_time;
    uint32_t write_time_ms = 0;
    uint32_t read_time_ms = 0;
    uint32_t write_speed;
    uint32_t read_speed;

#ifdef SDMMC_FULL_TEST
    test_block_count = sdmmc_test_result[id].sector_count;
#else
    test_block_count = SDMMC_TEST_BLOCK_COUNT*block_count;
#endif

    for(i = 0; i < sdmmc_test_result[id].sector_size*block_count; i++)
    {
        wbuff[i] = i;
    }

    for(blk = 0; blk < test_block_count/block_count; blk++)
    {
        start_block = blk*block_count;
#ifdef SDMMC_TEST_DATA_RESTORE
        ret = hal_sdmmc_read_blocks(id, start_block, block_count, bbuff);
        if(ret == 0)
        {
            TRACE(1, "%s: read original failed, ret = %d", __func__, ret);
            err_code =  -1;
            goto _func_fail;
        }
#endif
        //TRACE(1, "%s: hal_sdmmc_write_blocks(%d,%d,%d)", __func__, id, start_block, block_count);
        start_time = hal_fast_sys_timer_get();
        ret = hal_sdmmc_write_blocks(id, start_block, block_count, wbuff);
        end_time = hal_fast_sys_timer_get();
        if(ret == 0)
        {
            TRACE(1, "%s: write failed, ret = %d", __func__, ret);
            err_code =  -2;
            goto _func_fail;
        }
        write_time_ms += FAST_TICKS_TO_MS(end_time - start_time);

        start_time = hal_fast_sys_timer_get();
        ret = hal_sdmmc_read_blocks(id, start_block, block_count, rbuff);
        end_time = hal_fast_sys_timer_get();
        if(ret == 0)
        {
            TRACE(1, "%s: read failed, ret = %d", __func__, ret);
            err_code =  -3;
            goto _func_fail;
        }
        for(i = 0; i < sdmmc_test_result[i].sector_size*block_count; i++)
        {
            if(rbuff[i] != wbuff[i])
            {
                TRACE(1, "%s: write verify failed, 0x%x(@%d) != 0x%x",
                      __func__, rbuff[i], start_block, wbuff[i]);
                err_code =  -4;
                goto _func_fail;
            }
        }
        read_time_ms += FAST_TICKS_TO_MS(end_time - start_time);
#ifdef SDMMC_TEST_DATA_RESTORE
        ret = hal_sdmmc_write_blocks(id, start_block, block_count, bbuff);
        if(ret == 0)
        {
            TRACE(1, "%s: restore failed, ret = %d", __func__, ret);
            err_code =  -5;
            goto _func_fail;
        }
#endif
        TRACE(1, "write_blocks(%d, %08d, %08d) done", id, start_block, block_count);

    }
    write_speed =
        (uint32_t)((((uint64_t)test_block_count*sdmmc_test_result[id].sector_size)/(write_time_ms))*1000);
    read_speed =
        (uint32_t)((((uint64_t)test_block_count*sdmmc_test_result[id].sector_size)/(read_time_ms))*1000);

    if(block_count > 1)
    {
        sdmmc_test_result[id].mult_block_write_speed = write_speed;
        sdmmc_test_result[id].mult_block_read_speed = read_speed;
    }
    else
    {
        sdmmc_test_result[id].single_block_write_speed = write_speed;
        sdmmc_test_result[id].single_block_read_speed = read_speed;
    }

    TRACE(1, "WRITE SPEED: %08d(KB/s), READ SPEED: %08d(KB/s)", write_speed/1024, read_speed/1024);
    TRACE(1, "%s: done", __func__);
    return 0;
_func_fail:
    TRACE(1, "%s: failed, err code = %d", __func__, err_code);
    return err_code;

}

int _sdmmc_test_single_block_write(enum HAL_SDMMC_ID_T id)
{
    int ret;

    ret =_sdmmc_test_wr(id, 1);
    if(ret)
    {
        TRACE(1, "%s: failed, ret = %d", __func__, ret);
    }
    else
    {
        sdmmc_test_result[id].single_block_write_pass = true;
        sdmmc_test_result[id].single_block_read_pass = true;
        TRACE(1, "%s: done", __func__);
    }
    return ret;
}

int _sdmmc_test_mult_block_write(enum HAL_SDMMC_ID_T id)
{
    int ret;

    ret =_sdmmc_test_wr(id, SDMMC_TEST_BLOCK_COUNT);
    if(ret)
    {
        TRACE(1, "%s: failed, ret = %d", __func__, ret);
    }
    else
    {
        sdmmc_test_result[id].mult_block_write_pass = true;
        sdmmc_test_result[id].mult_block_read_pass = true;
        TRACE(1, "%s: done", __func__);
    }
    return ret;
}

int _sdmmc_test_do(enum HAL_SDMMC_ID_T id)
{
    int32_t ret = 0;

#ifdef MMC_SUPPORTS_TUNING
    ret = _sdmmc_test_tuning(id);
    if(ret)
    {
        goto func_fail;
    }
#endif

    ret = _sdmmc_test_get_info(id);
    if(ret)
    {
        goto func_fail;
    }

    ret = _sdmmc_test_single_block_write(id);
    if(ret)
    {
        goto func_fail;
    }

    ret = _sdmmc_test_mult_block_write(id);
    if(ret)
    {
        goto func_fail;
    }

    return 0;

func_fail:
    return -1;
}

void function_test_sdmmc(void)
{
    enum HAL_SDMMC_ID_T id;
    int ret;
    int test_fail_num = 0;
    POSSIBLY_UNUSED struct HAL_SDMMC_CONFIG_T emmc_cfg;
    struct HAL_SDMMC_CARD_INFO_T card_info;

    TRACE(1, "SDMMC TEST BEGIN.");
    //hal_sdmmc_set_delay_func((HAL_SDMMC_DELAY_FUNC)osDelay);
    //hal_enable_sd_detect_pin();
    emmc_cfg.ddr_mode = false;
    emmc_cfg.bus_speed = 24000000;
    emmc_cfg.bus_width = HAL_SDMMC_BUS_WIDTH_4;
    emmc_cfg.device_type = HAL_SDMMC_DEVICE_TYPE_EMMC;
    emmc_cfg.callback = &sdemmc_test_callback;
    _sdmmc_test_result_init();
    for(id = SDMMC_ID; id < HAL_SDMMC_ID_NUM; id ++)
    {
        id = HAL_SDMMC_ID_1;
        ret = hal_sdmmc_open(id, &emmc_cfg);
        if(ret)
        {
            TRACE(1, "%s: sdmmc(%d) open faile, ret = %d", __func__, id, ret);
            test_fail_num ++;
            //continue;
        }
        TRACE(1, "%s: sdmmc(%d) open done, ret = %d", __func__, id, ret);

        hal_sdmmc_get_card_info(id, &card_info);
        hal_sdmmc_dump_card_info(&card_info);

        ret = _sdmmc_test_do(id);
        if(ret)
        {
            TRACE(1, "%s: sdmmc_test_do(%d) fail. ret = %d", __func__, id, ret);
            test_fail_num ++;
            //continue;
        }

        hal_sdmmc_close(id);
    }

    for(id = SDMMC_ID; id < HAL_SDMMC_ID_NUM; id ++)
    {
        hal_sdmmc_dump(id);
        _sdmmc_test_result_print(id);
    }

    if(test_fail_num == 0)
    {
        TRACE(1, "SDMMC TEST DONE.");
    }
    else
    {
        TRACE(1, "SDMMC TEST FAIL.");
    }
    while(1);
}

/*
void function_test_sdmmc(void)
{
    enum HAL_SDMMC_ID_T id;
    int ret;
    int test_fail_num = 0;
    bool sdmmc_is_opened = false;

    TRACE(1, "SDMMC TEST BEGIN.");
    hal_sdmmc_set_delay_func((HAL_SDMMC_DELAY_FUNC)osDelay);
    hal_enable_sd_detect_pin();

    _sdmmc_test_result_init();
    id = SDMMC_ID;
    //for(id = SDMMC_ID; id < HAL_SDMMC_ID_NUM; id ++)
    while(1)
    {
        if(!sdmmc_is_opened)
        {
            if(open_sd_card !=  HAL_CARD_LAST_STATUS_UNKNOWN)
            {
                if(open_sd_card ==  HAL_CARD_LAST_STATUS_INSERTED)
                {
                    ret = hal_sdmmc_open(SDMMC_ID, NULL);
                    if(ret)
                    {
                        TRACE(1, "%s: sdmmc(%d) open faile, ret = %d", __func__, id, ret);
                        test_fail_num ++;
                        //continue;
                    }
                    sdmmc_is_opened = true;
                }
                else if(open_sd_card ==  HAL_CARD_LAST_STATUS_NOT_INSERTED)
                {
                    hal_sdmmc_close(SDMMC_ID);
                    sdmmc_is_opened = false;
                }
            }
        }

        if(sdmmc_is_opened)
        {
            ret = _sdmmc_test_do(SDMMC_ID);
            if(ret)
            {
                TRACE(1, "%s: sdmmc_test_do(%d) fail. ret = %d", __func__, id, ret);
                test_fail_num ++;
                continue;
            }
            hal_sdmmc_close(SDMMC_ID);
            sdmmc_is_opened = false;
        }

    }

    for(id = 0; id < HAL_SDMMC_ID_NUM; id ++)
    {
        hal_sdmmc_dump(SDMMC_ID);
        _sdmmc_test_result_print(SDMMC_ID);
    }

    if(test_fail_num == 0)
    {
        TRACE(1, "SDMMC TEST DONE.");
    }
    else
    {
        TRACE(1, "SDMMC TEST FAIL.");
    }
}
*/
#endif // CHIP_FUNCTION_TEST_SDMMC
