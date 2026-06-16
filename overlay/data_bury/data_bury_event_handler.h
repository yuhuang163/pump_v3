#ifndef __DATA_BURY_EVENT_HANDLER_H__
#define __DATA_BURY_EVENT_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "data_bury_manager_api.h"

void data_bury_event_handler_init_params(void);
int data_bury_event_handler_on_ble_notify_broadcast(data_bury_manager_context_t *ctx, int arg1, int arg2);

int data_bury_event_process_ble_connected(data_bury_manager_context_t *ctx,
                                          const data_bury_trigger_args_t *args);
int data_bury_event_process_ble_disconnect(data_bury_manager_context_t *ctx,
                                           const data_bury_trigger_args_t *args);
int data_bury_event_process_pump_session_start(data_bury_manager_context_t *ctx,
                                               const data_bury_trigger_args_t *args);
int data_bury_event_process_pump_mode_switch(data_bury_manager_context_t *ctx,
                                             const data_bury_trigger_args_t *args);
int data_bury_event_process_pump_manual_adjust(data_bury_manager_context_t *ctx,
                                               const data_bury_trigger_args_t *args);
int data_bury_event_process_pump_user_event(data_bury_manager_context_t *ctx,
                                            const data_bury_trigger_args_t *args);
int data_bury_event_process_pump_session_end(data_bury_manager_context_t *ctx,
                                             const data_bury_trigger_args_t *args);
int data_bury_event_process_power_on(data_bury_manager_context_t *ctx,
                                     const data_bury_trigger_args_t *args);
int data_bury_event_process_power_off(data_bury_manager_context_t *ctx,
                                      const data_bury_trigger_args_t *args);
int data_bury_event_process_ota_start(data_bury_manager_context_t *ctx,
                                      const data_bury_trigger_args_t *args);
int data_bury_event_process_bat_charge_start(data_bury_manager_context_t *ctx,
                                             const data_bury_trigger_args_t *args);
int data_bury_event_process_bat_charge_end(data_bury_manager_context_t *ctx,
                                           const data_bury_trigger_args_t *args);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_EVENT_HANDLER_H__ */
