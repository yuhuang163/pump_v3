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
#ifdef TEST_TMA525C
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_i2c.h"

#ifdef CHIP_BEST1600
#define TMA525C_I2C_BUS     HAL_I2C_ID_0
#define TMA525C_I2C_ADDR    0x24
#define TMA525C_RST_GPIO    HAL_GPIO_PIN_P4_3
#define TMA525C_IRQ_GPIO    HAL_GPIO_PIN_P4_1
#endif

#ifdef CHIP_BEST1700
#define TMA525C_I2C_BUS     HAL_I2C_ID_0
#define TMA525C_I2C_ADDR    0x24
#define TMA525C_RST_GPIO    HAL_GPIO_PIN_P4_3
#define TMA525C_IRQ_GPIO    HAL_GPIO_PIN_P4_1
#endif

#ifdef CHIP_BEST1502X
#define TMA525C_I2C_BUS     HAL_I2C_ID_2
#define TMA525C_I2C_ADDR    0x24
#define TMA525C_RST_GPIO    HAL_GPIO_PIN_P6_2
#define TMA525C_IRQ_GPIO    HAL_GPIO_PIN_P3_2
#endif

#ifdef CHIP_BEST1503
#define TMA525C_I2C_BUS     HAL_I2C_ID_2
#define TMA525C_I2C_ADDR    0x24
#define TMA525C_RST_GPIO    HAL_GPIO_PIN_P6_2
#define TMA525C_IRQ_GPIO    HAL_GPIO_PIN_P3_2
#endif

#define CHIP_ON_DELAY       200
#define CHIP_OFF_DELAY      70

#define PT_MAX_PIP_MSG_SIZE 264
#define PT_MAX_INPUT PT_MAX_PIP_MSG_SIZE

typedef  struct {
    uint8_t reg[2];
    uint16_t len;
    uint8_t report_id;
    uint8_t rsvd;
    uint8_t sop;
    uint8_t cmd_id;
    uint16_t data_len;
    uint16_t crc;
    uint8_t eop;
} __attribute__ ((packed)) PIP1_BL_CMD_HEADER;

typedef  struct {
    uint8_t reg[2];
    uint16_t len;
    uint8_t report_id;
    uint8_t rsvd;
    uint8_t cmd_id;
} __attribute__ ((packed)) PIP1_CMD_HEADER;

struct pt_core_data {
    uint8_t debug_level;
    uint8_t write_buf[PT_MAX_INPUT];
    uint8_t read_buf[PT_MAX_INPUT];
    uint8_t mode;
    uint8_t system_mode;
    uint8_t hid_cmd_state;
    uint8_t pip2_cmd_tag_seq;
    uint8_t pip2_prot_active;
    uint8_t pip2_send_user_cmd;
    uint8_t pip_ver_major;
    uint8_t pip_ver_minor;
    uint8_t bl_ver_major;
    uint8_t bl_ver_minor;
    uint8_t fw_ver_major;
    uint8_t fw_ver_minor;
    uint8_t elec_x; /* tx number when tx == column */
    uint8_t elec_y; /* rx number when rx == row*/
    uint8_t mfg_id[8];
    uint16_t post_code;
    uint16_t fw_ver_conf;
    uint32_t revctrl;
};

typedef  struct {
    uint16_t wideband_noise;
    uint16_t inject_noise;
    uint16_t NMF_noise;
    uint8_t touch_type;
    uint8_t water_level;
    uint32_t raw_count_variation;
    uint32_t diff_count_variation;
    uint16_t CDC_overflow;
    uint16_t temperature;
}__attribute__ ((packed)) PT_NOISE_METRIC;

#define PT_CMD_OUTPUT_REGISTER  0x0004
#define HIGH_BYTE(x) (uint8_t)(((x) >> 8) & 0xFF)
#define LOW_BYTE(x) (uint8_t)((x)&0xFF)
#define PIP1_RESP_REPORT_ID_OFFSET 2
#define TOUCH_COUNT_BYTE_OFFSET 5
#define IS_LARGE_AREA(x) ((x)&0x20)
#define PR_BUF_SIZE (1024)
#define GET_NUM_TOUCHES(x) ((x)&0x1F)

#define MAX_BUF_LEN (10*1024)

#define CHECK_PIP1_RESP_ID(rsp_buf, cmd_id) \
    (((rsp_buf)[2] == 0x1F) && (((rsp_buf)[4] & 0x7f) == (cmd_id)))

#define PIP1_CMD_ID_GET_NOISE_METRICS 0x07

typedef enum {
    EVENT_NONE = 0,
    EVENT_DOWN,
    EVENT_MOVE,
    EVENT_UP,
} tma525c_event;

struct tma525c_touch_msg {
    uint8_t   event;
    uint16_t  x;
    uint16_t  y;
};

#define PRINT_PIP_MSG

#ifdef PRINT_PIP_MSG
static char print_buf[PR_BUF_SIZE];
static char mfg_print_buffer[MAX_BUF_LEN]; // To store formated mfg test result
#endif
static struct pt_core_data cd = { 0 };

static int pt_parse_cmd_and_interrupt(struct tma525c_touch_msg *msg);

static void tma525c_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    TRACE(0, "TMA525C IRQ GOT!");
    struct tma525c_touch_msg msg;
    memset(&msg, 0, sizeof(msg));
    if (pt_parse_cmd_and_interrupt(&msg) == 0) {
        TRACE(0, "event: %d, x: %d, y: %d", msg.event, msg.x, msg.y);
    }
}

static void tma525c_irq_init(bool enable, uint32_t int_pin, uint32_t int_mode)
{
    uint32_t lock;
    struct HAL_GPIO_IRQ_CFG_T gpiocfg = {0};
    lock = int_lock_global();

    gpiocfg.irq_polarity = int_mode ;
    gpiocfg.irq_type     = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;

    if (enable) {
        gpiocfg.irq_enable   = true;
        gpiocfg.irq_debounce = true;
        gpiocfg.irq_handler  = tma525c_irq_handler;
    } else {
        gpiocfg.irq_enable = false;
        gpiocfg.irq_debounce = false;
        gpiocfg.irq_handler = NULL;
    }

    hal_gpio_setup_irq(int_pin, &gpiocfg);

    int_unlock_global(lock);
}

static int tma525c_init(void)
{
    int ret = 0;
    struct HAL_IOMUX_PIN_FUNCTION_MAP iomux_cfg[] = {
         {TMA525C_RST_GPIO, HAL_IOMUX_FUNC_AS_GPIO,
             HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
         {TMA525C_IRQ_GPIO, HAL_IOMUX_FUNC_AS_GPIO,
             HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    struct HAL_I2C_CONFIG_T cfg = {0};

    hal_iomux_init(iomux_cfg, sizeof(iomux_cfg)/sizeof(iomux_cfg[0]));
    hal_gpio_pin_set_dir(TMA525C_RST_GPIO, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir(TMA525C_IRQ_GPIO, HAL_GPIO_DIR_IN, 0);

    osDelay(1);
    hal_gpio_pin_set(TMA525C_RST_GPIO);
    osDelay(10);
    hal_gpio_pin_clr(TMA525C_RST_GPIO);
    osDelay(100);
    hal_gpio_pin_set(TMA525C_RST_GPIO);
    osDelay(200);

    tma525c_irq_init(true, TMA525C_IRQ_GPIO, HAL_GPIO_IRQ_POLARITY_LOW_FALLING);

    memset(&cfg, 0, sizeof(cfg));
    cfg.mode      = HAL_I2C_API_MODE_SIMPLE;
    cfg.use_dma   = 1;
    cfg.use_sync  = 1;
    cfg.as_master = 1;
    cfg.speed     = 400000;
    ret = hal_i2c_open(TMA525C_I2C_BUS, &cfg);

    return ret;
}

static int tma525c_i2c_write(uint8_t *buf, uint32_t size)
{
    int ret = hal_i2c_simple_send(TMA525C_I2C_BUS, TMA525C_I2C_ADDR, buf, size);
    TRACE(0, "%s: ret %d, buf %p, size %u", __func__, ret, buf, size);
    return ret;
}

static int tma525c_i2c_read(uint8_t *buf, uint32_t size)
{
    int ret = hal_i2c_simple_recv(TMA525C_I2C_BUS, TMA525C_I2C_ADDR, NULL, 0, buf, size);
    TRACE(0, "%s: ret %d, buf %p, size %u", __func__, ret, buf, size);
    return ret;
}

POSSIBLY_UNUSED
static int pt_wait_and_clear_int(uint16_t max_wait_ms, uint16_t *buf16, uint16_t size)
{
    uint16_t time_ms = 0;
    uint8_t *buf = (uint8_t *)buf16;
    int ret = 0;

    memset(buf, 0, size);

    while (time_ms < max_wait_ms) {
        if (hal_gpio_pin_get_val(TMA525C_IRQ_GPIO) == 0) {
            ret = tma525c_i2c_read(buf, size);
            if (ret == 0) {
                TRACE(0, "%s: receive buf 0x%02x 0x%02x\n", __func__, buf[0], buf[1]);
                if (buf[0] == 0x02) {
                    TRACE(0, "%s: receive application mode!\n", __func__);
                    return 1;
                }
                continue;
            } else {
                TRACE(0, "%s: I2C error\n", __func__);
                break;
            }
        }
        osDelay(1);
        time_ms += 1;
    }

    return ret;
}

static void print_pip_msg(uint8_t *buffer, uint8_t len, const char *header)
{
#ifdef PRINT_PIP_MSG
    int i = 0;
    int index = 0;

    memset(print_buf, 0, sizeof(print_buf));
    if (header)
        index += sprintf(&print_buf[index], "%s:", header);
    for (i = 0; i < len && index < (PR_BUF_SIZE-3); i++) {
        index += sprintf(&print_buf[index], " %02X", buffer[i]);
    }
    TRACE(0, "%s\n", print_buf);
#endif
}

static uint16_t get_unaligned_le16(const void *p)
{
    const uint8_t *_p = p;
    return _p[0] | _p[1] << 8;
}

static uint32_t get_unaligned_le32(const void *p)
{
    const uint8_t *_p = p;
    return _p[0] | _p[1] << 8 | _p[2] << 16 | _p[3] << 24;
}

static int pt_read_default_nosize(uint8_t *buf, uint32_t max)
{
    int ret = 0;
    uint32_t size;

    if (!buf)
        return -1;

    ret = tma525c_i2c_read(buf, 2);
    size = get_unaligned_le16(&buf[0]);
    if (!size) {
        TRACE(0, "%s: Sentinel\n", __func__);
        return ret;
    } else if ((size > max) || (size > PT_MAX_PIP_MSG_SIZE)) {
        TRACE(0, "%s: Empty buffer\n", __func__);
        return ret;
    }

    ret = tma525c_i2c_read(buf, size);
    print_pip_msg(buf, size, "READ");
    return ret;
}

static int pt_wait_and_check_cmd_id(uint16_t max_wait_ms, uint8_t cmd_id)
{
    uint16_t time_ms = 0;
    int rc = 0;

    while (time_ms < max_wait_ms) {
        if (hal_gpio_pin_get_val(TMA525C_IRQ_GPIO) == 0) {
            rc = pt_read_default_nosize(cd.read_buf, PT_MAX_PIP_MSG_SIZE);
            if (!rc) {
                if (CHECK_PIP1_RESP_ID(cd.read_buf, cmd_id)) {
                    return 1;
                } else {
                    TRACE(0, "%s: Unexpected msg report_id = %d, retry\n",
                        __func__, cd.read_buf[2]);
                }
            } else {
                TRACE(0, "%s: I2C error\n", __func__);
                break;
            }
        }
        osDelay(1);
        time_ms += 1;
    }

    TRACE(0, "%s: Time Out\n", __func__);
    return 0;
}

static int pt_write_read_specific(uint8_t write_len, uint8_t *write_buf, uint8_t *read_buf)
{
    int rc = 0;

    tma525c_i2c_write(write_buf, write_len);
    print_pip_msg(write_buf, write_len, "WRITE");

    if (read_buf)
        rc = pt_read_default_nosize(read_buf, PT_MAX_PIP_MSG_SIZE);

    return rc;
}

static int pt_send_pip1_cmd(int cmd_id, uint8_t *buf, uint8_t data_len)
{
    int rc;
    uint16_t index = 0;

    PIP1_CMD_HEADER pip1_header = {0};
    pip1_header.reg[0] = LOW_BYTE(PT_CMD_OUTPUT_REGISTER);
    pip1_header.reg[1] = HIGH_BYTE(PT_CMD_OUTPUT_REGISTER);
    /* head.len(2)report id(1) rsvd(1) cmd_id(1) */
    pip1_header.len = data_len + 0x05;
    pip1_header.report_id = 0x2f;
    pip1_header.rsvd = 0x00;
    pip1_header.cmd_id = cmd_id;

    index = 0;
    memcpy(&cd.write_buf[index], &pip1_header, sizeof(pip1_header));
    index += sizeof(pip1_header);

    if (buf && data_len) {
        memcpy(&cd.write_buf[index], buf, data_len);
        index += data_len;
    }

    rc = pt_write_read_specific(index, cd.write_buf, NULL);
    return rc;
}

static int pt_get_sysinfo(void)
{
    int int_state;
    uint8_t cmd_id = 0x02;//PIP1_CMD_ID_GET_SYSINFO = 0x02,

    pt_send_pip1_cmd(cmd_id, NULL, 0);
    int_state = pt_wait_and_check_cmd_id(1000, cmd_id);

    if (int_state) {
        cd.fw_ver_major = cd.read_buf[9];
        cd.fw_ver_minor = cd.read_buf[10];
        cd.revctrl = get_unaligned_le32(&cd.read_buf[11]);
        cd.fw_ver_conf = get_unaligned_le16(&cd.read_buf[15]);
        cd.post_code = get_unaligned_le16(&cd.read_buf[31]);
        cd.elec_x = cd.read_buf[33];
        cd.elec_y = cd.read_buf[34];
        memcpy(cd.mfg_id, &cd.read_buf[23],8);

        TRACE(0, "FW ver=0x%X.%X, rev_ctrl=0x%08X, conf_ver=0x%02X, x=%d, y=%d\n",
            cd.fw_ver_major, cd.fw_ver_minor,
            cd.revctrl, cd.fw_ver_conf, cd.elec_x, cd.elec_y);
        return 0;
    }

    TRACE(0, "%s: error\n", __func__);
    return -1;
}

static void pt_parse_touch_report(struct tma525c_touch_msg *msg)
{
    uint8_t report_id = 0;
    uint8_t report_reg = 0;
    uint8_t num_cur_tch = 0;
    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t i = 0, touch_id = 0, event_id = 0;

    report_id = cd.read_buf[PIP1_RESP_REPORT_ID_OFFSET];
    if (0x01 == report_id) {
        report_reg = cd.read_buf[TOUCH_COUNT_BYTE_OFFSET];
        if (IS_LARGE_AREA(report_reg))
            TRACE(0, "Large Object detected\n");

        num_cur_tch = GET_NUM_TOUCHES(report_reg);
        if (num_cur_tch) {
            for (i = 0; i < num_cur_tch; i ++) {
                touch_id = cd.read_buf[7+ i*10 + 1] & 0x1f;
                event_id = (cd.read_buf[7+ i*10 + 1] >> 5) & 0x03;
                x = cd.read_buf[7+ i*10 + 2] | cd.read_buf[7+ i*10 + 3] << 8;
                y = cd.read_buf[7+ i*10 + 4] | cd.read_buf[7+ i*10 + 5] << 8;
                if (event_id == 1)
                    msg->event = EVENT_DOWN;
                else if (event_id == 2)
                    msg->event = EVENT_MOVE;
                else if(event_id == 3)
                    msg->event = EVENT_UP;
                msg->x = (454 - x);
                msg->y = (454 - y);
                if (event_id == 3)
                    TRACE(0, "(%d): event %d Lift, ID=%d, X=%d, Y=%d\n", i,
                                event_id, touch_id, x, y);
                else
                    TRACE(0, "(%d): event %d Down, ID=%d, X=%d, Y=%d\n", i,
                                event_id, touch_id, x, y);
            }
        } else {
            TRACE(0, "All Lift-off\n");
        }
    }
}

static void pt_parse_noise_metric(void)
{
    uint8_t report_id = 0;
    uint8_t cmd_id = PIP1_CMD_ID_GET_NOISE_METRICS;
    PT_NOISE_METRIC *noise_metric;

    report_id = cd.read_buf[PIP1_RESP_REPORT_ID_OFFSET];

    if (CHECK_PIP1_RESP_ID(cd.read_buf, cmd_id) && (report_id == 0x1f)) {
        noise_metric = (PT_NOISE_METRIC *)&cd.read_buf[5];
        TRACE(0, "water_level=%d, inject_noise=%d, "
               "temperature=%d,touch_type=%d\n",
               noise_metric->water_level, noise_metric->inject_noise,
               noise_metric->temperature, noise_metric->touch_type);
    }
}

#ifdef PRINT_PIP_MSG
static void pt_pip1_cmd_send_noise_metric(void)
{
    uint8_t cmd_id = PIP1_CMD_ID_GET_NOISE_METRICS;
    pt_send_pip1_cmd(cmd_id, NULL, 0);
}

static uint32_t pt_format_hex(uint8_t* input_buf, uint32_t input_len,
                        char* out_buf, uint32_t out_buf_size)
{
    uint32_t input_index = 0;
    uint32_t output_index = 0;

    while (input_index < input_len) {
        if ((output_index + 3) > out_buf_size)
            break;

        sprintf(&out_buf[output_index], "%02X,", input_buf[input_index]);
        output_index += 3;
        input_index += 1;
    }

    if ((output_index + 1) > out_buf_size)
        sprintf(&out_buf[output_index-1], "\n");
    else {
        sprintf(&out_buf[output_index], "\n");
        output_index ++;
    }

    return output_index;
}

static void pt_format_debug_info(void)
{
    uint8_t cmd_id = PIP1_CMD_ID_GET_NOISE_METRICS;
    uint8_t noise_metrix = 0;
    uint8_t report_id = 0;
    uint8_t event_id = 0;
    uint8_t touch_report = 0;
    uint8_t num_cur_tch = 0;
    uint8_t report_reg = 0;
    uint8_t static first_touch;
    char *out_buf = &mfg_print_buffer[0];
    uint16_t size = 0;
    int i = 0;

    if (cd.debug_level) {
        memset(out_buf, 0, MAX_BUF_LEN);
        size = get_unaligned_le16(&cd.read_buf[0]);
        report_id = cd.read_buf[PIP1_RESP_REPORT_ID_OFFSET];
        if (size >= 7 && (0x01 == report_id)) {
            touch_report = 1;
        } else if (CHECK_PIP1_RESP_ID(cd.read_buf, cmd_id) &&
               (report_id == 0x1f)) {
            noise_metrix = 1;
        }
        if (cd.debug_level == 2 && touch_report) {
            report_reg = cd.read_buf[TOUCH_COUNT_BYTE_OFFSET];
            num_cur_tch = GET_NUM_TOUCHES(report_reg);
            for (i = 0; i < num_cur_tch; i++) {
                event_id =
                    (cd.read_buf[7 + i * 10 + 1] >> 5) & 0x03;
                if (event_id == 3)
                    break;
            }
            if (num_cur_tch && (first_touch == 0)) {
                first_touch = 1;
                pt_pip1_cmd_send_noise_metric();
            }
            if (num_cur_tch && (event_id == 3)) {
                first_touch = 0;
                pt_pip1_cmd_send_noise_metric();
            }
        }

        if (cd.debug_level == 3 && touch_report) {
            pt_pip1_cmd_send_noise_metric();
        }

        if (noise_metrix || touch_report)
            pt_format_hex(cd.read_buf, size, out_buf, MAX_BUF_LEN);
    }

    TRACE(0, "%d,%u,%s", cd.debug_level, hal_fast_sys_timer_get(), out_buf);
}
#endif /* PRINT_PIP_MSG */

static int pt_parse_cmd_and_interrupt(struct tma525c_touch_msg *msg)
{
    uint8_t report_id = 0;
    int rc = 0;
    uint16_t size;

    rc = pt_read_default_nosize(cd.read_buf, PT_MAX_PIP_MSG_SIZE);
    if (!rc) {
        size = get_unaligned_le16(&cd.read_buf[0]);
        if (!size || (size == 2) || size > PT_MAX_PIP_MSG_SIZE) {
            return -1;
        }

        report_id = cd.read_buf[PIP1_RESP_REPORT_ID_OFFSET];
        if (size >= 7 && (0x01 == report_id)) {
            pt_parse_touch_report(msg);
            rc = 0;
        } else if (0x04 == report_id) {
            TRACE(0, "%s: wakeup event\n", __func__);
            rc = 1;
        } else {
            pt_parse_noise_metric();
            rc = 1;
        }

#ifdef PRINT_PIP_MSG
        pt_format_debug_info();
#endif
    } else {
        TRACE(0, "%s: I2C error\n", __func__);
    }
    return rc;
}

void test_tma525c(void)
{
    hal_iomux_set_i2c0();

    tma525c_init();

    pt_get_sysinfo();

    while (1) {
        osDelay(10000);
    }
}

#endif
