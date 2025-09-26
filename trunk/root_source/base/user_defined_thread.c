/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "hal_motor.h"
#include "peripheral_bus_api.h"
#include "root_common_api.h" // IWYU pragma: keep

/*
 * DEFINES
 *****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
#if IS_SUPPORT_PRESSURE_TEST  
static void hdr_run_pressure_test_cb(hdr_post_args_t *args)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *motor_dev = pbus->find_device(ID_DEVICE_MOTOR);
    motor_para_t motor_cfg = {
        .un.pwm.duty =  rand() % 100,
        .un.pwm.freq =  rand() % 1000,
    };
    pwm_control_id_e pwm_id = rand() % PWM_ID_MAX;
    uint32_t args1[] = {pwm_id, (uint32_t) &motor_cfg};
    int ret = DEVICE_IOCTL_ARGS(motor_dev, OP_CODE_MOTOR_SET_PARAMETER, args1);
    LOGI("pressure_test: pwm_id: %d, duty: %d, freq: %d", pwm_id, motor_cfg.un.pwm.duty, motor_cfg.un.pwm.freq);
    POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, 1000, "pressure_test:continue");
}
#endif

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int run_user_thread_dispatch(void)
{
#if IS_SUPPORT_PRESSURE_TEST  
    POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, 1000, "pressure_test");
#endif
    // window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    // if (wm) {
    //     wm->play_boot_animation();
    // }
    return 0;
}
