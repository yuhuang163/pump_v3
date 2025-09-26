/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_pwm.h"
#include "hal_iomux.h"
#include "bsp_common.h" 
#include "bsp_motor_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
typedef struct {
    enum HAL_PWM_ID_T id;
    void (*iomux_set)(void);
} motor_pwm_cfg_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const motor_pwm_cfg_t motor_pwm_cfg[] = {
    {HAL_PWM_ID_0,  hal_iomux_set_pwm0},
    {HAL_PWM_ID_1,  hal_iomux_set_pwm1},
    {HAL_PWM1_ID_2, hal_iomux_set_pwm6}, // P36
    {HAL_PWM1_ID_3, hal_iomux_set_pwm7}, // P37
};

static uint8_t motor_pwm_status[PWM_ID_MAX];
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_motor_init(void)
{
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(motor_pwm_cfg); i++) {
        motor_pwm_cfg[i].iomux_set();
    }
    memset(motor_pwm_status, 0, sizeof(motor_pwm_status));
    return BSP_OK;
}

int bsp_motor_deinit(void)
{
    for (int i = 0; i < ARRAY_SIZE(motor_pwm_cfg); i++) {
        hal_pwm_disable(motor_pwm_cfg[i].id);
    }
    return 0;
}

int bsp_motor_parameter_set(void *args)
{
    uint32_t *tmp = (uint32_t *)args;
    pwm_control_id_e pwm_id = (pwm_control_id_e )tmp[0];
    if (pwm_id >= PWM_ID_MAX) {
        LOGE("pwm_id is out of range: %d", pwm_id);
        return -1;
    }

    motor_para_t *motor = (motor_para_t *)tmp[1];
    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = motor->un.pwm.freq,
        .ratio = motor->un.pwm.duty,
        .inv = false,
        .sleep_on = false,
    };
    int ret = hal_pwm_enable(motor_pwm_cfg[pwm_id].id, &pwm_cfg);
    motor_pwm_status[pwm_id] = 1;
    LOGI("parameter_set: pwm_id: %d, duty: %d, freq: %d, ret: %d", pwm_id, motor->un.pwm.duty, motor->un.pwm.freq, ret);
    return ret;
}

int bsp_motor_stop(void *args)
{
    pwm_control_id_e pwm_id = *(pwm_control_id_e *)args;
    if (pwm_id >= PWM_ID_MAX) {
        return -1;
    }
    int ret = hal_pwm_disable(motor_pwm_cfg[pwm_id].id);
    motor_pwm_status[pwm_id] = 0;
    return ret;
}

int bsp_motor_start(void *args)
{
    return BSP_OK;
}

int bsp_motor_is_running(void *args)
{
    pwm_control_id_e pwm_id = *(pwm_control_id_e *)args;
    if (pwm_id >= PWM_ID_MAX) {
        return -1;
    }
    return motor_pwm_status[pwm_id] > 0 ? 1 : 0;
}