#include <stdio.h>
#include <stdarg.h>
#include "plat_types.h"
#include "string.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "app_utils.h"
#include "eshell.h"
#include "cqueue.h"
#include "app_eshell.h"
#include "app_eshell_core.h"

#define ESHELL_SEND_DATA_PAYLAOD_MAX_SIZE       (255)

#define ESHELL_OUTPUT_ONELINE_MAX_BYTES         (ONELINE_BUFF_SIZE-3)
#define ESHELL_CMD_ONELINE_MAX_BYTES            (ESHELL_SEND_DATA_PAYLAOD_MAX_SIZE)

#define MODULE_TRACE_LEVEL  TR_LEVEL_WARN

#define LOG_V(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_VERBOSE) TR_VERBOSE(TR_MOD(ESHELL), "[APP-ESHELL]" str, ##__VA_ARGS__)
#define LOG_D(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_DEBUG) TR_DEBUG(TR_MOD(ESHELL), "[APP-ESHELL]" str, ##__VA_ARGS__)
#define LOG_I(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_INFO) TR_INFO(TR_MOD(ESHELL), "[APP-ESHELL]" str, ##__VA_ARGS__)
#define LOG_W(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_WARN) TR_WARN(TR_MOD(ESHELL), "[APP-ESHELL]" str, ##__VA_ARGS__)
#define LOG_E(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_ERROR) TR_ERROR(TR_MOD(ESHELL), "[APP-ESHELL]" str, ##__VA_ARGS__)

typedef struct{
    uint16_t     cmd_code;
    uint8_t      sub_cmd_code;
    uint8_t      cmd_direction;
    uint16_t     data_len;
}__attribute__((aligned(1))) EXT_ESHELL_MSG_HEAD;

typedef struct{
    EXT_ESHELL_MSG_HEAD     head;
    uint8_t                 payload[ESHELL_SEND_DATA_PAYLAOD_MAX_SIZE];
}__attribute__((aligned(1))) EXT_ESHELL_MSG;


#if defined(APP_RPC_ENABLE)
#define APP_ESHELL_TASK_STACK_SZ ROUND_UP(2048, 8)
static osThreadId_t eshell_rpc_rx_thread_id = NULL;
static void app_eshell_rpc_rx_data_thread(const void *arg);
static uint64_t os_thread_def_stack_app_eshell_rpc_rx_data_thread [APP_ESHELL_TASK_STACK_SZ / sizeof(uint64_t)];
static const osThreadAttr_t ThreadAttr_Eshell_App = {
    .name = "eshell_rpc_rx_thread",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_mem = os_thread_def_stack_app_eshell_rpc_rx_data_thread,
    .stack_size = APP_ESHELL_TASK_STACK_SZ,
    .priority = osPriorityHigh,
    .tz_module = 1U,                  // indicate calls to secure mode
    .reserved = 0U,
};

#define APP_ESHELL_RPC_RX_CMD_SIGNAL                (1<<11)
#define APP_ESHELL_RPC_RX_RESULT_SIGNAL             (1<<12)

static uint8_t eshell_output_oneline_buff[ESHELL_OUTPUT_ONELINE_MAX_BYTES] = {0};
static osMutexId_t rx_result_queue_mutex_id = NULL;
static const osMutexAttr_t MutexAttrEshellAppRxResult = {
    .name = "rx_result_queue_mutex",
    .attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust,
    .cb_mem = NULL,
    .cb_size = 0U,
};
static CQueue rx_result_queue;
static uint8_t rx_result_queue_buff[ESHELL_SEND_DATA_PAYLAOD_MAX_SIZE*4] = {0};

static uint8_t eshell_cmd_oneshot_buff[ESHELL_CMD_ONELINE_MAX_BYTES] = {0};
static osMutexId_t rx_cmd_queue_mutex_id = NULL;
static const osMutexAttr_t MutexAttrEshellAppRxCmd = {
    .name = "rx_cmd_queue_mutex",
    .attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust,
    .cb_mem = NULL,
    .cb_size = 0U,
};
static CQueue rx_cmd_queue;
static uint8_t rx_cmd_queue_buff[ESHELL_SEND_DATA_PAYLAOD_MAX_SIZE] = {0};


#define ESHELL_WAIT_CMD_DONE_TIMEOUT  (5000)
static osSemaphoreId_t eshell_cmd_done_signal_id = NULL;

static int app_eshell_cmd_done_signal_wait(void)
{
    int ret = osOK;

    LOG_D("WAIT SIGNAL");
    ret = osSemaphoreAcquire(eshell_cmd_done_signal_id, ESHELL_WAIT_CMD_DONE_TIMEOUT);
    if(ret!= osOK)
    {
        LOG_W("%s %d",__func__,ret);
    }
    return ret;
}

static void app_eshell_cmd_done_signal_set(void)
{
    LOG_D("SET SIGNAL");
    osSemaphoreRelease(eshell_cmd_done_signal_id);
}

void app_eshell_rpc_send_op_cmd_func(char* buffer, unsigned int buff_size)
{
    LOG_I("%s buff_size=%d", __func__, buff_size);

    uint8_t send_data[sizeof(EXT_ESHELL_MSG)] = {0};
    EXT_ESHELL_MSG *p_eshell_msg = (EXT_ESHELL_MSG *)&send_data[0];

    p_eshell_msg->head.cmd_code = MCU_BTH_M55_TASK_CMD_ESHELL_TRANSMIT_DATA;
    p_eshell_msg->head.sub_cmd_code = EXT_ESHELL_MSG_SUB_CMD_OP_CMD;
    p_eshell_msg->head.cmd_direction = ESHELL_CMD_DIR_TRANSIMIT;
    p_eshell_msg->head.data_len = (uint16_t)buff_size;
    if (p_eshell_msg->head.data_len > ESHELL_SEND_DATA_PAYLAOD_MAX_SIZE)
    {
        return;
    }
    memcpy(p_eshell_msg->payload, buffer, p_eshell_msg->head.data_len);

    app_rpc_send_cmd(APP_RPC_CORE_REMOTE, MCU_BTH_M55_TASK_CMD_ESHELL_TRANSMIT_DATA, (uint8_t*)send_data, p_eshell_msg->head.data_len + sizeof(EXT_ESHELL_MSG_HEAD));
}

void app_eshell_rpc_send_op_result_func(char* buffer, unsigned int buff_size)
{
    LOG_I("%s buff_size=%d", __func__, buff_size);

    uint8_t send_data[sizeof(EXT_ESHELL_MSG)] = {0};
    EXT_ESHELL_MSG *p_eshell_msg = (EXT_ESHELL_MSG *)&send_data[0];

    p_eshell_msg->head.cmd_code = MCU_BTH_M55_TASK_CMD_ESHELL_TRANSMIT_DATA;
    p_eshell_msg->head.sub_cmd_code = EXT_ESHELL_MSG_SUB_CMD_OP_RESULT;
    p_eshell_msg->head.cmd_direction = ESHELL_CMD_DIR_TRANSIMIT;
    p_eshell_msg->head.data_len = (uint16_t)buff_size;
    if (p_eshell_msg->head.data_len > ESHELL_SEND_DATA_PAYLAOD_MAX_SIZE)
    {
        return;
    }
    memcpy(p_eshell_msg->payload, buffer, p_eshell_msg->head.data_len);

    app_rpc_send_cmd(APP_RPC_CORE_REMOTE, MCU_BTH_M55_TASK_CMD_ESHELL_TRANSMIT_DATA, (uint8_t*)send_data, p_eshell_msg->head.data_len + sizeof(EXT_ESHELL_MSG_HEAD));
    app_eshell_cmd_done_signal_wait();
}

static void app_eshell_rpc_recv_op_cmd_execute_handler(char* buffer, unsigned int buff_size)
{
    uint32_t data_len, left_len, output_len;
    do {
        osMutexAcquire(rx_cmd_queue_mutex_id, osWaitForever);
        data_len = LengthOfCQueue(&rx_cmd_queue);
        output_len = (data_len > ESHELL_CMD_ONELINE_MAX_BYTES)?ESHELL_CMD_ONELINE_MAX_BYTES:data_len;
        DeCQueue(&rx_cmd_queue, eshell_cmd_oneshot_buff, output_len);
        osMutexRelease(rx_cmd_queue_mutex_id);

        eshell_rpc_execute_command((char*)eshell_cmd_oneshot_buff, output_len);
        left_len = data_len - output_len;
    } while(left_len > 0);
}

static void app_eshell_rpc_recv_op_result_execute_handler(char* buffer, unsigned int buff_size)
{
    uint32_t data_len, left_len, output_len;
    do {
        osMutexAcquire(rx_result_queue_mutex_id, osWaitForever);
        data_len = LengthOfCQueue(&rx_result_queue);
        output_len = (data_len > ESHELL_OUTPUT_ONELINE_MAX_BYTES)?ESHELL_OUTPUT_ONELINE_MAX_BYTES:data_len;
        DeCQueue(&rx_result_queue, eshell_output_oneline_buff, output_len);
        osMutexRelease(rx_result_queue_mutex_id);

        eshell_output_rawdata((char*)eshell_output_oneline_buff, output_len);
        left_len = data_len - output_len;
    } while(left_len > 0);
}

static void app_eshell_rpc_recv_cmd_data_execute_handler(unsigned char* buffer, short buff_size)
{
    (void)buff_size;
    EXT_ESHELL_MSG* p_eshell_msg = (EXT_ESHELL_MSG*)buffer;
    LOG_I("ehsell rpc recv sub_code=0x%x len=%d dir=%d", p_eshell_msg->head.sub_cmd_code,
                                p_eshell_msg->head.data_len, p_eshell_msg->head.cmd_direction);
    uint8_t rsp_data[sizeof(EXT_ESHELL_MSG)] = {0};
    EXT_ESHELL_MSG *p_rsp_data = NULL;
    uint8_t str_end_mark[1] = {'\0'};

    switch (p_eshell_msg->head.sub_cmd_code)
    {
        case EXT_ESHELL_MSG_SUB_CMD_OP_CMD:
            if (p_eshell_msg->head.cmd_direction == ESHELL_CMD_DIR_TRANSIMIT)
            {
                osMutexAcquire(rx_cmd_queue_mutex_id, osWaitForever);
                uint32_t avaliable_len = AvailableOfCQueue(&rx_cmd_queue);
                uint32_t push_len = 0;
                if (avaliable_len < (p_eshell_msg->head.data_len + 1))
                {
                    LOG_E("ESHELL CMD LOST!!!");
                    push_len = avaliable_len - 1;
                }
                else
                {
                    push_len = p_eshell_msg->head.data_len;
                }
                EnCQueue(&rx_cmd_queue, p_eshell_msg->payload, push_len);
                osMutexRelease(rx_cmd_queue_mutex_id);
                osThreadFlagsSet(eshell_rpc_rx_thread_id, APP_ESHELL_RPC_RX_CMD_SIGNAL);
            }
            else if (p_eshell_msg->head.cmd_direction == ESHELL_CMD_DIR_RSP)
            {

            }
            break;
        case EXT_ESHELL_MSG_SUB_CMD_OP_RESULT:
            if (p_eshell_msg->head.cmd_direction == ESHELL_CMD_DIR_TRANSIMIT)
            {
                osMutexAcquire(rx_result_queue_mutex_id, osWaitForever);
                uint32_t avaliable_len = AvailableOfCQueue(&rx_result_queue);
                uint32_t push_len = 0;
                if (avaliable_len < (p_eshell_msg->head.data_len + 1))
                {
                    LOG_E("ESHELL OUTPUT LOST!!!");
                    push_len = avaliable_len - 1;
                }
                else
                {
                    push_len = p_eshell_msg->head.data_len;
                }
                EnCQueue(&rx_result_queue, p_eshell_msg->payload, push_len);
                EnCQueue(&rx_result_queue, str_end_mark, 1);
                osMutexRelease(rx_result_queue_mutex_id);
                osThreadFlagsSet(eshell_rpc_rx_thread_id, APP_ESHELL_RPC_RX_RESULT_SIGNAL);

                p_rsp_data = (EXT_ESHELL_MSG *)&rsp_data[0];
                p_rsp_data->head.cmd_code = p_eshell_msg->head.cmd_code;
                p_rsp_data->head.sub_cmd_code = EXT_ESHELL_MSG_SUB_CMD_OP_RESULT;
                p_rsp_data->head.cmd_direction = ESHELL_CMD_DIR_RSP;
                p_rsp_data->head.data_len = 0;
                app_rpc_send_cmd(APP_RPC_CORE_REMOTE, MCU_BTH_M55_TASK_CMD_ESHELL_TRANSMIT_DATA, (uint8_t*)rsp_data, p_rsp_data->head.data_len + sizeof(EXT_ESHELL_MSG_HEAD));
            }
            else if (p_eshell_msg->head.cmd_direction == ESHELL_CMD_DIR_RSP)
            {
                app_eshell_cmd_done_signal_set();
            }
            break;
        default:
            break;
    }
}

static void app_eshell_rpc_rx_data_thread(const void *arg)
{
    LOG_I("eshell_rpc_rx_data_thread...");
    uint32_t flags;
    while(1)
    {
        flags = osThreadFlagsWait(CMSIS_V1_SIGNAL_ANY, osFlagsWaitAny, osWaitForever);

        if((flags > 0U) && (flags < 0x80000000U))
        {
            LOG_D("eshll rpc rx flags=0x%04x", (int32_t)flags);
            app_sysfreq_req(APP_SYSFREQ_USER_ESHELL, APP_SYSFREQ_104M);
            if ((int32_t)flags & APP_ESHELL_RPC_RX_CMD_SIGNAL)
            {
                app_eshell_rpc_recv_op_cmd_execute_handler(NULL, 0);
            }
            if ((int32_t)flags & APP_ESHELL_RPC_RX_RESULT_SIGNAL)
            {
                app_eshell_rpc_recv_op_result_execute_handler(NULL, 0);
            }
            app_sysfreq_req(APP_SYSFREQ_USER_ESHELL, APP_SYSFREQ_32K);
        }
    }
}

void app_eshell_rpc_init(void)
{
    if (rx_result_queue_mutex_id == NULL)
    {
        rx_result_queue_mutex_id = osMutexNew(&MutexAttrEshellAppRxResult);
    }
    if (rx_cmd_queue_mutex_id == NULL)
    {
        rx_cmd_queue_mutex_id = osMutexNew(&MutexAttrEshellAppRxCmd);
    }
    InitCQueue(&rx_result_queue, sizeof(rx_result_queue_buff), rx_result_queue_buff);
    InitCQueue(&rx_cmd_queue, sizeof(rx_cmd_queue_buff), rx_cmd_queue_buff);
    app_esehll_rpc_bridge_cmd_recv_handler_cb_register(app_eshell_rpc_recv_cmd_data_execute_handler);
    if(eshell_cmd_done_signal_id == NULL)
    {
        eshell_cmd_done_signal_id = osSemaphoreNew(65535U, (uint32_t)0, NULL);
    }
    eshell_rpc_rx_thread_id = osThreadNew((osThreadFunc_t)app_eshell_rpc_rx_data_thread,
                                          NULL, &ThreadAttr_Eshell_App);
}
#endif
