#ifndef __APP_HCI_BRIDGE_H__
#define __APP_HCI_BRIDGE_H__

#include <stdint.h>
#include <stddef.h>

typedef void (*app_hci_bridge_data_handler)(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
extern "C" {
#endif
    // mode, true: connectivity side false: BTH side
    void app_hci_bridge_init(bool mode);

    void app_hci_bridge_register_data_handler(app_hci_bridge_data_handler func);
    int app_hci_bridge_send_data(const uint8_t *data, uint16_t len);
    bool app_uart_start_receiving_data(void);
    void app_hci_bridge_close(void);
    void app_uart_stop_uart_rx(void);

#ifdef __cplusplus
}
#endif

#endif//__APP_HCI_BRIDGE_H__
