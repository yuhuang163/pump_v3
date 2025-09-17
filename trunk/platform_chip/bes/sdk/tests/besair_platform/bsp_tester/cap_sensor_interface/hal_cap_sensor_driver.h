#ifndef __HAL_CAP_SENSOR_DRIVER_H__
#define __HAL_CAP_SENSOR_DRIVER_H__

struct cap_sense_sample {
    uint8_t ch;
    uint32_t sar;
    uint32_t sar_zheng;
    uint32_t sar_xiao;
    uint32_t sdm;
    uint32_t sdm_zheng;
    uint32_t sdm_xiao;
};

void pmu_capsense_vref_enable(void);
void cap_sense_reg_start_write(uint16_t reg, uint16_t val);
void cap_sense_reg_start_read(uint16_t reg, uint16_t *val);
void capsense_reg_init(void);

int get_sample(struct cap_sense_sample *sample);
int catpture_sample_start(void);
int catpture_sample_stop(void);
int catpture_base_line(void);

#endif