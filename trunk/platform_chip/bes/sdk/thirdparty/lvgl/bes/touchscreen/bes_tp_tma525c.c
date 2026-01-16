
#include "hal_timer.h"
#include "hal_trace.h"
#include "stdio.h"
#include "string.h"
#include "bes_tp_tma525c.h"
#include "hal_gpio.h"
#include "bes_tp_hal.h"

#define TMA_DEBUG

#ifdef TMA_DEBUG
#define TMA_TRACE(n, s, ...) TRACE(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
#else
#define TMA_TRACE(n, s, ...)
#endif

typedef struct this_i2c_dev_ {
    uint32_t          bus;
    uint32_t          addr; //device address
    dev_write         fn_write;
    dev_read          fn_read;
}this_i2c_t;

static this_i2c_t tma525c_dev = {
    .addr = TMA525C_I2C_ADDR,
};

static uint16_t get_chip_code(uint16_t regaddr);

char print_buf[PR_BUF_SIZE];
char mfg_print_buffer[MAX_BUF_LEN]; // To store formated mfg test result
struct pt_core_data cd = {0};
static volatile u8 noise_wait_count;

void tma525c_register_i2c_rw(dev_write fn_write, dev_read fn_read)
{
    tma525c_dev.fn_write = fn_write;
    tma525c_dev.fn_read  = fn_read;
}

static int tma525c_write_reg(uint8_t* data, uint32_t size)
{
    this_i2c_t *pdev = &tma525c_dev;

    TMA_TRACE(0,"i2c addr: 0x%02X value: 0x%02X size %u\n", pdev->addr, data[0], size);

    uint8_t msg_buf[32] = {0};

    memcpy(msg_buf, data, size);

    int ret = pdev->fn_write(pdev->bus, pdev->addr,(uint8_t*)msg_buf, size);

    return ret;
}

int pt_wait_and_clear_int(uint16_t max_wait_ms,uint16_t *data, uint16_t size)
{
    u16 time_ms = 0;

    int ret = 0;
    u8 int_flag = 0;
    while (time_ms < max_wait_ms) {
        if (hal_gpio_pin_get_val(TP_INT_PIN) == 0) {
            int_flag = 1;
        }

        if (int_flag == 1) {
            ret = tma525c_dev.fn_read(tma525c_dev.bus, tma525c_dev.addr, NULL, 0, (uint8_t*)data, size);
            if (ret == 0) {
                TMA_TRACE(0, "%s: receive data 0x%02x 0x%02x\n", __func__, data[0],data[1]);
                if(data[0] == 0x02)
                {
                    TMA_TRACE(0, "%s: receive application mode!", __func__);
                    return 1;
                }
                continue;
            } else {
                TMA_TRACE(0, "%s: I2C error\n", __func__);
                break;
            }
        }
        hal_sys_timer_delay(MS_TO_TICKS(1));
        time_ms += 1;
    }

    return ret;
}

static void print_pip_msg(u8 *buffer, u8 len, const char *header)
{
    int i = 0;
    int index = 0;

    memset(print_buf, 0, sizeof(print_buf));
    if (header)
        index += sprintf(&print_buf[index], "%s:", header);
    for (i = 0; i < len && index < (PR_BUF_SIZE-3); i++) {
        index += sprintf(&print_buf[index], " %02X", buffer[i]);
    }
    TMA_TRACE(0,"%s\n",print_buf);

}
static u16 get_unaligned_le16(const void *p)
{
    const u8 *_p = p;
    return _p[0] | _p[1] << 8;
}
static u32 get_unaligned_le32(const void *p)
{
    const u8 *_p = p;
    return _p[0] | _p[1] << 8 | _p[2] << 16 | _p[3] << 24;
}
static int32_t pt_read_default_nosize(u8 *buf, u32 max)
{
    u32 size;

    if (!buf)
        return -1;

    int ret = tma525c_dev.fn_read(tma525c_dev.bus, tma525c_dev.addr, NULL, 0, (uint8_t*)buf, 2);
    size = get_unaligned_le16(&buf[0]);
    if (!size) {
        TMA_TRACE(0, "%s: Sentinel\n", __func__);
        return ret;
    } else if ((size > max) || (size > PT_MAX_PIP_MSG_SIZE)) {
        TMA_TRACE(0,"%s: Empty buffer\n", __func__);
        return ret;
    }

    tma525c_dev.fn_read(tma525c_dev.bus, tma525c_dev.addr, NULL, 0, (uint8_t*)buf, size);
    print_pip_msg(buf, size, "READ");
    return 0;
}
int pt_wait_and_check_cmd_id(uint16_t max_wait_ms, u8 cmd_id)
{
    u16 time_ms = 0;
    u8 int_flag = 0;
    int rc = 0;

    while (time_ms < max_wait_ms) {
        if (hal_gpio_pin_get_val(TP_INT_PIN) == 0) {
            int_flag = 1;
        }

        if (int_flag == 1) {
            rc = pt_read_default_nosize(cd.read_buf, PT_MAX_PIP_MSG_SIZE);
            if (!rc) {
                if (CHECK_PIP1_RESP_ID(cd.read_buf, cmd_id))
                    return 1;
                else {
                    TMA_TRACE(0, "%s: Unexpected msg report_id = %d, retry\n",
                        __func__, cd.read_buf[2]);
                    int_flag = 0;
                }
            } else {
                TMA_TRACE(0,"%s: I2C error\n", __func__);
                break;
            }
        }
        hal_sys_timer_delay(MS_TO_TICKS(1));
        time_ms += 1;
    }

    TMA_TRACE(0,"%s: Time Out\n", __func__);
    return 0;
}
static int32_t pt_write_read_specific(u8 write_len, u8 *write_buf, u8 *read_buf)
{
    int32_t rc = 0;

    tma525c_write_reg(write_buf, write_len);
    print_pip_msg(write_buf, write_len, "WRITE");

    if (read_buf)
        rc = pt_read_default_nosize(read_buf, PT_MAX_PIP_MSG_SIZE);

    return rc;
}
static int32_t pt_send_pip1_cmd(int cmd_id, u8 *data, u8 data_len)
{
    int32_t rc;
    u16 index = 0;

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

    if (data && data_len) {
        memcpy(&cd.write_buf[index], data, data_len);
        index += data_len;
    }

    rc = pt_write_read_specific(index, cd.write_buf, NULL);
    return rc;
}
static int32_t pt_pip1_cmd_get_sysinfo(void)
{
    int int_state;
    u8 cmd_id = 0x02;//PIP1_CMD_ID_GET_SYSINFO = 0x02,

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

        TMA_TRACE(0, "FW ver=0x%X.%X, rev_ctrl=0x%08X, conf_ver=0x%02X, x=%d, y=%d\n",
            cd.fw_ver_major, cd.fw_ver_minor,
            cd.revctrl, cd.fw_ver_conf, cd.elec_x, cd.elec_y);
        return 0;
    }
    
    TMA_TRACE(0, "%s: error\n", __func__);
    return -1;
}

static void pt_parse_touch_report(tp_sample_t* touch_sample)
{
    u8 report_id = 0;
    u8 report_reg = 0;
    u8 num_cur_tch = 0;
    u16 x = 0;
    u16 y = 0;
    u8 i = 0, touch_id = 0, event_id = 0;

    report_id = cd.read_buf[PIP1_RESP_REPORT_ID_OFFSET];
    if (0x01 == report_id)
    {
        report_reg = cd.read_buf[TOUCH_COUNT_BYTE_OFFSET];
        if (IS_LARGE_AREA(report_reg)) {
            TMA_TRACE(0, "Large Object detected\n");
        } 
            
        num_cur_tch = GET_NUM_TOUCHES(report_reg);
        if (num_cur_tch) {
            for (i = 0; i < num_cur_tch; i ++) {
                touch_id = cd.read_buf[7+ i*10 + 1] & 0x1f;
                event_id = (cd.read_buf[7+ i*10 + 1] >> 5) & 0x03;
                x = cd.read_buf[7+ i*10 + 2] | cd.read_buf[7+ i*10 + 3] << 8;
                y = cd.read_buf[7+ i*10 + 4] | cd.read_buf[7+ i*10 + 5] << 8;
                if (event_id == 1)
                {
                    touch_sample->contact = CONTACT_DOWN;
                }
                else if (event_id == 2)
                {
                    touch_sample->contact = CONTACT_MOVE;
                }
                else if(event_id == 3)
                {
                    touch_sample->contact = CONTACT_UP;
                }
                touch_sample->x = (454-x);
                touch_sample->y = (454-y);
                if (event_id == 3)
                    TMA_TRACE(0, "(%d): event %d Lift, ID=%d, X=%d, Y=%d\n", i, event_id, touch_id, x, y);
                else
                    TMA_TRACE(0, "(%d): event %d Down, ID=%d, X=%d, Y=%d\n", i, event_id, touch_id, x, y);
            }
        } else {
            TMA_TRACE(0,"All Lift-off\n");
        }
    }
}

void pt_pip1_cmd_send_noise_metric(void)
{
    u8 cmd_id = PIP1_CMD_ID_GET_NOISE_METRICS;
    pt_send_pip1_cmd(cmd_id, NULL, 0);
}

void pt_parse_noise_metric(void)
{
    u8 report_id = 0;
    u8 cmd_id = PIP1_CMD_ID_GET_NOISE_METRICS;
    PT_NOISE_METRIC *noise_metric;

    report_id = cd.read_buf[PIP1_RESP_REPORT_ID_OFFSET];

    if (CHECK_PIP1_RESP_ID(cd.read_buf, cmd_id) && (report_id == 0x1f)) {
        noise_metric = (PT_NOISE_METRIC *)&cd.read_buf[5];
        TMA_TRACE(0,"water_level=%d, inject_noise=%d, "
               "temperature=%d,touch_type=%d\n",
               noise_metric->water_level, noise_metric->inject_noise,
               noise_metric->temperature, noise_metric->touch_type);
    }
}
static u32 pt_format_hex(u8* input_buf, u32 input_len, char* out_buf, u32 out_buf_size)
{
    u32 input_index = 0;
    u32 output_index = 0;

    while (input_index < input_len)
    {

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
void pt_format_debug_info(void)
{
    u8 cmd_id = PIP1_CMD_ID_GET_NOISE_METRICS;
    u8 noise_metrix = 0;
    u8 report_id = 0;
    u8 event_id = 0;
    u8 touch_report = 0;
    u8 num_cur_tch = 0;
    u8 report_reg = 0;
    u8 static first_touch;
    char *out_buf = &mfg_print_buffer[0];
    u16 size = 0;
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

    TMA_TRACE(0, "%d,%u,%s", cd.debug_level, hal_fast_sys_timer_get(), out_buf);
}
int pt_parse_cmd_and_interrupt(tp_sample_t* touch_sample)
{
    u8 report_id = 0;
    int rc = 0;
    u16 size;

    rc = pt_read_default_nosize(cd.read_buf, PT_MAX_PIP_MSG_SIZE);
    if (!rc) {
        size = get_unaligned_le16(&cd.read_buf[0]);
        if (!size || (size == 2) || size > PT_MAX_PIP_MSG_SIZE) {
            return 0;
        }
        report_id = cd.read_buf[PIP1_RESP_REPORT_ID_OFFSET];
        if (size >= 7 && (0x01 == report_id)) {
            pt_parse_touch_report(touch_sample);
            noise_wait_count = 0;
        } else if (0x04 == report_id){
            TMA_TRACE(0,"%s: wakeup event\n", __func__);
        } else {
            pt_parse_noise_metric();
        }
        pt_format_debug_info();
    } else {
        TMA_TRACE(0, "%s: I2C error\n", __func__);
    }
    return 0;
}

static uint16_t get_chip_code(uint16_t regaddr)
{
    uint16_t chip_code = 0;

    //uint8_t pip[] = {0x01,0x00};

    //tma525c_write_reg(pip, sizeof(pip));

    //tma525c_read_reg(&chip_code, sizeof(chip_code));
    //pt_wait_and_clear_int(500, &chip_code, sizeof(chip_code));

    pt_wait_and_clear_int(1000, &chip_code, sizeof(chip_code));

    pt_pip1_cmd_get_sysinfo();

    TMA_TRACE(0," %s chip_code %x \n",__func__,chip_code);
    return chip_code;
}


int32_t tma525c_init(uint32_t dev_addr)
{
    return 0;
}

int32_t tma525c_read_coord(tp_sample_t* touch_sample)
{
    pt_parse_cmd_and_interrupt(touch_sample);
    return 0;
}
int32_t tma525c_probe(uint32_t i2c_bus)
{
    tma525c_dev.bus = i2c_bus;
    if(get_chip_code(0) != 0)
    {
        return 0;
    }
    return 0;
}
