
/***************************************************************************
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

#ifndef __HAL_LCDC_QSPI_H
#define __HAL_LCDC_QSPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "plat_addr_map.h"

enum HAL_LCDC_QSPI_ID_E {
    LCDC_QSPI_ID_0 = 0,
#ifdef LCDC_HAS_QSPI1
    LCDC_QSPI_ID_1 = 1,
#endif

    LCDC_QSPI_ID_QTY,
};

/**
 * enum HAL_LCDC_QSPI_LINE_MODE_E
 */
enum HAL_LCDC_QSPI_LINE_MODE_E {
    LCDC_QSPI_1LINE = 1, /* 1 data line  */
    LCDC_QSPI_2LINE = 2, /* 2 data lines */
    LCDC_QSPI_4LINE = 4, /* 4 data lines */
    LCDC_QSPI_MIP   = 5, /* QSPI 4 lines -> MIP */
};

/**
 * enum HAL_LCDC_QSPI_CFMT_E - lcdc qspi out pixel color format
 */

enum HAL_LCDC_QSPI_CFMT_E {
    LCDC_QSPI_RGB888,
    LCDC_QSPI_RGB666,
    LCDC_QSPI_RGB565,
};

struct HAL_LCDC_QSPI_CONFIG_T {
    uint16_t pix_mhz;
    uint16_t bus_mhz;

    enum HAL_LCDC_QSPI_CFMT_E color_fmt;
    enum HAL_LCDC_QSPI_LINE_MODE_E line_mode;

    bool dtr_enable;
    bool dual_enable;
};

/**
 * @brief hal_lcdc_qspi_init - initialize qspi sub-module, after lcdc & qspi color enable.
 *
 * @param id     : LCDC QSPI bus id
 * @param config : LCDC QSPI configuration
 */
int hal_lcdc_qspi_init(enum HAL_LCDC_QSPI_ID_E id, struct HAL_LCDC_QSPI_CONFIG_T *config);

/**
 * @brief hal_lcdc_qspi_exit - exit qspi sub-module, diable qspi clocks
 */
void hal_lcdc_qspi_exit(void);

/**
 * @brief hal_lcdc_qspi_get_mhz - get QSPI bus freq in Mhz.
 */
uint32_t hal_lcdc_qspi_get_mhz(void);

/**
 * @brief hal_lcdc_qspi_get_pixmhz - get QSPI pixel clock freq in Mhz.
 */
uint32_t hal_lcdc_qspi_get_pixmhz(void);

/**
 * @brief hal_lcdc_qspi_sleep - qspi enter low power mode
 */
void hal_lcdc_qspi_sleep(void);

/**
 * @brief hal_lcdc_qspi_wakeup - qspi exit low power mode
 */
void hal_lcdc_qspi_wakeup(void);

/**
 * @brief hal_lcdc_qspi_swaprb_enable - enable red & blue order swap
 */
void hal_lcdc_qspi_swaprb_enable(bool enable);

/**
 * @brief hal_lcdc_qspi_set_command_freq - set command transfer frequncy in Mhz
 */
void hal_lcdc_qspi_set_command_freq(uint32_t freqmhz);

/**
 * @brief hal_lcdc_qspi_send_command - send one command
 *
 * @param id  : LCDC QSPI bus id
 * @param cmd : Command data
 */
int hal_lcdc_qspi_send_command(enum HAL_LCDC_QSPI_ID_E id, uint8_t cmd);

/**
 * @brief hal_lcdc_qspi_read_command - read data from qspi
 *
 * @param id       : LCDC QSPI bus id
 * @param cmd      : Command data
 * @param read_buf : Read buffer
 * @param len      : Read length
 */
int hal_lcdc_qspi_read_command(enum HAL_LCDC_QSPI_ID_E id, uint8_t cmd, uint8_t *read_buf,
                               uint8_t len);

/**
 * @brief hal_lcdc_qspi_send_command_data - send command with 4 parameters
 *
 * @param id  : LCDC QSPI bus id
 * @param cmd : Command data
 * @param len : Param count
 * @param p0  : Param byte0
 * @param p1  : Param byte1
 * @param p2  : Param byte2
 * @param p3  : Param byte3
 */
int hal_lcdc_qspi_send_command_data(enum HAL_LCDC_QSPI_ID_E id, uint8_t cmd, uint32_t len,
                                    uint8_t p0, uint8_t p1, uint8_t p2, uint8_t p3);

/**
 * @brief hal_lcdc_qspi_send_command_list - send command with parameter list
 *
 * @param id          : LCDC QSPI bus id
 * @param cmd         : Command data
 * @param param_count : Param count
 * @param param_list  : Param data arrays
 */
int hal_lcdc_qspi_send_command_list(enum HAL_LCDC_QSPI_ID_E id, uint8_t cmd, uint8_t param_count,
                                    uint8_t *param_list);


/**
 * @brief hal_lcdc_qspi_gen_start - start qspi frame transfer
 */
int hal_lcdc_qspi_gen_start(void);


/**
 * @brief hal_lcdc_qspi_cs_sel - active or deactive spi cs pin
 * Only for best1502x.
 */
void hal_lcdc_qspi_cs_sel(bool active);


#ifdef __cplusplus
}
#endif
#endif /* __HAL_QSPI_H */
