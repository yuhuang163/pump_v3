#include "hal_trace.h"
#include <stdbool.h>
#include "hal_ipc.h"
#include "hal_ipc_notify_api.h"

WEAK int hal_ipc_notify_interrupt_core(enum HAL_IPC_NOTIFY_CORE_TYPE_T core, enum HAL_IPC_NOTIFY_SUB_ID_T id){return 0;}
WEAK int hal_ipc_notify_open(enum HAL_IPC_NOTIFY_CORE_TYPE_T core, enum HAL_IPC_NOTIFY_SUB_ID_T id, struct HAL_IPC_NOTIFY_IRQ_CB_T irq_cb){return 0;}

void sys_bth_irq_cb_tx_handler(uint32_t core, uint32_t id)
{
    TRACE(0,"%s core:%d id:%d", __func__, core, id);
}

void sys_bth_irq_cb_rx_handler(uint32_t core, uint32_t id)
{
    TRACE(0,"%s core:%d id:%d", __func__, core, id);
}

void hal_ipc_notify_sys_bth(void)
{
    struct HAL_IPC_NOTIFY_IRQ_CB_T irq_cb;

    irq_cb.tx_handler = sys_bth_irq_cb_tx_handler;
    irq_cb.rx_handler = sys_bth_irq_cb_rx_handler;

    hal_ipc_notify_open(HAL_IPC_NOTIFY_CORE_SYS_BTH, HAL_IPC_NOTIFY_SUB_ID_0, irq_cb);
}

void bth_btc_irq_cb_tx_handler(uint32_t core, uint32_t id)
{
    TRACE(0,"%s core:%d id:%d", __func__, core, id);
}

void bth_btc_irq_cb_rx_handler(uint32_t core, uint32_t id)
{
    TRACE(0,"%s core:%d id:%d", __func__, core, id);
}

void hal_ipc_notify_bth_btc(void)
{
    struct HAL_IPC_NOTIFY_IRQ_CB_T irq_cb;

    irq_cb.tx_handler = bth_btc_irq_cb_tx_handler;
    irq_cb.rx_handler = bth_btc_irq_cb_rx_handler;

    hal_ipc_notify_open(HAL_IPC_NOTIFY_CORE_BTH_BTC, HAL_IPC_NOTIFY_SUB_ID_0, irq_cb);
    hal_ipc_notify_open(HAL_IPC_NOTIFY_CORE_BTH_BTC, HAL_IPC_NOTIFY_SUB_ID_1, irq_cb);
}

