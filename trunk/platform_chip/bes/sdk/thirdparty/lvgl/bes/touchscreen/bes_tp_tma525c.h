#ifndef __BES_TP_TMA525C_H__
#define __BES_TP_TMA525C_H__

#include "bes_tp_utils.h"

#ifdef __cplusplus
extern "C"{
#endif

#define TMA525C_I2C_ADDR    0x24
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
} __attribute__ ((packed)) PIP1_BL_CMD_HEADER;
typedef  struct {
    uint8_t reg[2];
    uint16_t len;
    uint8_t report_id;
    uint8_t rsvd;
    uint8_t cmd_id;
} __attribute__ ((packed)) PIP1_CMD_HEADER;
struct pt_core_data {
    u8 debug_level;
    u8 write_buf[PT_MAX_INPUT];
    u8 read_buf[PT_MAX_INPUT];
    u8 mode;
    u8 system_mode;
    u8 hid_cmd_state;
    u8 pip2_cmd_tag_seq;
    u8 pip2_prot_active;
    u8 pip2_send_user_cmd;
    u8 pip_ver_major;
    u8 pip_ver_minor;
    u8 bl_ver_major;
    u8 bl_ver_minor;
    u8 fw_ver_major;
    u8 fw_ver_minor;
    u8 elec_x; /* tx number when tx == column */
    u8 elec_y; /* rx number when rx == row*/
    u8 mfg_id[8];
    u16 post_code;
    u16 fw_ver_conf;
    u32 revctrl;
};

typedef  struct {
    u16 wideband_noise;
    u16 inject_noise;
    u16 NMF_noise;
    u8 touch_type;
    u8 water_level;
    u32 raw_count_variation;
    u32 diff_count_variation;
    u16 CDC_overflow;
    u16 temperature;
}__attribute__ ((packed)) PT_NOISE_METRIC;

#define PT_CMD_OUTPUT_REGISTER  0x0004
#define HIGH_BYTE(x) (u8)(((x) >> 8) & 0xFF)
#define LOW_BYTE(x) (u8)((x)&0xFF)
#define PIP1_RESP_REPORT_ID_OFFSET 2
#define TOUCH_COUNT_BYTE_OFFSET 5
#define IS_LARGE_AREA(x) ((x)&0x20)
#define PR_BUF_SIZE (1024)
#define GET_NUM_TOUCHES(x) ((x)&0x1F)

#define MAX_BUF_LEN (10*1024)

#define CHECK_PIP1_RESP_ID(rsp_buf, cmd_id) \
    (((rsp_buf)[2] == 0x1F) && (((rsp_buf)[4] & 0x7f) == (cmd_id)))

#define PIP1_CMD_ID_GET_NOISE_METRICS 0x07

typedef int (*dev_write)(uint32_t i2c_bus, uint32_t slave,uint8_t *pd, uint32_t dlen);
typedef int (*dev_read)(uint32_t i2c_bus, uint32_t slave, uint8_t *reg_addr, uint32_t reg_len, uint8_t *pd, uint32_t dlen);

void tma525c_register_i2c_rw(dev_write fn_write, dev_read fn_read);
int32_t tma525c_read_coord(tp_sample_t* touch_sample);
int32_t tma525c_probe(uint32_t i2c_bus);
int32_t tma525c_init(uint32_t dev_addr);

#ifdef __cplusplus
}
#endif
#endif



