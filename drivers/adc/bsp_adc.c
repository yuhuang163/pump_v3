/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "hal_gpadc.h"
#include "bsp_adc.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef struct {
    uint8_t conv_times;
    uint8_t volid_count;
    uint32_t sum_volt;
} adc_result_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static os_mutex_id s_adc_mux = NULL;
static os_message_id s_adc_msg_id = NULL;
static adc_result_t s_adc_result = {0};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
POSSIBLY_UNUSED static void gpadc_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    if (s_adc_result.conv_times == 0) {
        return;
    }

    s_adc_result.sum_volt += volt;
    s_adc_result.volid_count++;
    s_adc_result.conv_times--;
    if (s_adc_result.conv_times == 0) {
        MSG_QUEUE_SEND(s_adc_msg_id, &s_adc_result.volid_count, 0);
        // LOGI("raw_val = %d volt = %d ", irq_val, volt);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_adc_single_conversion(uint8_t channel_id, uint16_t conv_times, uint16_t *voltage)
{
    int ret = 0;
    uint8_t msg = 0;

    if (channel_id >= HAL_GPADC_CHAN_QTY) {
        LOGE("channel_id out of range: %d", channel_id);
        return -1;
    }

    MUTEX_LOCK(s_adc_mux);
    memset(&s_adc_result, 0, sizeof(adc_result_t));
    s_adc_result.conv_times = conv_times;
    ret = hal_gpadc_open(channel_id, HAL_GPADC_ATP_125US, gpadc_irq_handler);
    if (ret != 0) {
        LOGE("gpadc open failed: %d", ret);
        MUTEX_UNLOCK(s_adc_mux);
        return ret;
    }
    MSG_QUEUE_RECV(s_adc_msg_id, &msg, conv_times + 100);
    if (s_adc_result.volid_count > 0) {
        *voltage = s_adc_result.sum_volt / s_adc_result.volid_count;
    }
    hal_gpadc_close(channel_id);
    MUTEX_UNLOCK(s_adc_mux);

    return BSP_OK;
}

void bsp_adc_init(void)
{
    if (s_adc_mux == NULL) {
        MUTEX_INIT(&s_adc_mux, dev_adc_mutex);
    }

    if (s_adc_msg_id == NULL) {
        CREATE_MSG_TYPE_QUEUE(&s_adc_msg_id, 1, 1);
    }
}

void bsp_adc_deinit(uint8_t channel_id)
{
    MUTEX_LOCK(s_adc_mux);
    hal_gpadc_close(channel_id);
    MUTEX_UNLOCK(s_adc_mux);
}