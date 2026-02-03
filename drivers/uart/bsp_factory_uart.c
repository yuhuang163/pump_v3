/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "hal_uart.h" // IWYU pragma: keep
#include "hal_timer.h" // IWYU pragma: keep
#include "hal_iomux.h" // IWYU pragma: keep
#include "bsp_uart_internal.h" // IWYU pragma: keep
#include "hal_trace.h" // IWYU pragma: keep
#include "cmsis_os_api.h" // IWYU pragma: keep

//工厂测试共用日志uart接口
#define FACTORY_UART_USE_TRACE                    1

#if FACTORY_UART_USE_TRACE
/*
* DEFINES
*****************************************************************************************
*/
#define FACTORY_UART_RX_BUF_SIZE            (128)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
typedef struct factory_uart_msg_data {
    uint32_t msg_length;
    uint8_t  *msg_data;
} factory_uart_msg_data_t;

static uint8_t factory_uart_rx_buf[FACTORY_UART_RX_BUF_SIZE];
static uart_recv_event_cb_func s_factory_uart_event_cb = NULL;
static thread_context_t s_factory_uart_thd = {0};
static rt_device_node_t *node = NULL;
static os_message_id factory_uart_recv_msg_id = NULL;
/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static unsigned int factory_uart_recv_handler(unsigned char *rx_buff, unsigned int len)
{
    // 忽略无意义数据
    // 协议最小帧长度：
    // - comm_protocol (新协议): 16字节 (COMM_PROTOCOL_FRAME_SIZE_MIN)
    // - app_old_protocol (旧协议): 6字节 (Header(2) + CT(1) + CID(1) + CAL(1) + Checksum(1))
    // 使用 len < 6 可以过滤掉所有小于最小协议帧长度的噪声数据
    if (len < 6) {
        LOGW("factory uart rx: filtered short data, len=%d (min frame size: 6 bytes)", len);
        return len;
    }
    
    // 检查缓冲区指针
    if (rx_buff == NULL) {
        LOGE("factory uart rx: NULL buffer pointer");
        return len;
    }
    
    if (len > 0) {
        BSP_LOG_HEX("factory uart rx", rx_buff, len);
    }

    if (len > FACTORY_UART_RX_BUF_SIZE) {
        LOGE("factory uart rx buffer overflow: len=%d", len);
        return len;
    }
 
    factory_uart_msg_data_t msg = {0};
    msg.msg_data = u_malloz(len);
    if (msg.msg_data == NULL) {
        LOGE("factory uart rx buffer malloc failed: len=%d", len);
        return len;
    }
    memcpy(msg.msg_data, rx_buff, len);
    msg.msg_length = len;
    MSG_QUEUE_SEND(factory_uart_recv_msg_id, &msg, 0);
    return len;
}

static void factory_uart_event_thread(const void *args)
{
    factory_uart_msg_data_t msg = {0};
    CREATE_MSG_TYPE_QUEUE(&factory_uart_recv_msg_id, 3, sizeof(factory_uart_msg_data_t));
    while (1) {
        MSG_QUEUE_RECV(factory_uart_recv_msg_id, &msg, OS_WAIT_FOREVER);
    
        if (s_factory_uart_event_cb) {
            s_factory_uart_event_cb(node, msg.msg_data, msg.msg_length);
        }
        if (msg.msg_data) {
            u_free(msg.msg_data);
            msg.msg_data = NULL;
            msg.msg_length = 0;
        }
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void bsp_factory_uart_register_event_cb(uart_recv_event_cb_func cb, rt_device_node_t *dev)
{
    s_factory_uart_event_cb = cb;
    node = dev;
}

int bsp_factory_uart_send_data(uint8_t *buf, uint32_t len)
{
    hal_trace_output(buf, len);
    return 0;
}

int bsp_factory_uart_init(void)
{
    int ret = 0;
    ret = hal_trace_rx_open(
        factory_uart_rx_buf, sizeof(factory_uart_rx_buf),
        (PLATFORM_BT_TRACE_RX_CALLBACK_T)factory_uart_recv_handler);
    if (ret != 0) {
        LOGE("platform_bt_trace_rx_register failed: %d", ret);
    }
    RUN_NEW_THREAD(&s_factory_uart_thd, "factory_uart",
                   factory_uart_event_thread, RT_PRIO_FACTORY_UART_THREAD,
                   (FACTORY_UART_THREAD_STACKSIZE));
    return 0;
}

void bsp_factory_uart_close(void)   
{
    hal_trace_rx_close();
}
#else

/*
* DEFINES
*****************************************************************************************
*/
#define FACTORY_UART_BAUDRATE               115200
#define FACTORY_UART_ID                     HAL_UART_ID_1
#define FACTORY_UART_IOMUX_SET()            hal_iomux_set_uart1()
#define FACTORY_UART_RX_BUF_SIZE            (256)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;
static volatile uint8_t factory_uart_rx_done = 0;
static volatile uint8_t factory_uart_tx_done = 0;
static volatile uint16_t factory_uart_rx_len = 0;
static thread_context_t s_factory_uart_thd = {0};
static uart_recv_event_cb_func s_factory_uart_event_cb = NULL;
static os_message_id factory_uart_recv_msg_id = NULL;
static os_message_id factory_uart_send_msg_id = NULL;    
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
POSSIBLY_UNUSED static int factory_uart_write(uint8_t *buf, uint32_t len)
{
    uint8_t msg = 0;
    int ret = 0;
    ret = hal_uart_dma_send((enum HAL_UART_ID_T)FACTORY_UART_ID, (uint8_t *)buf, len, NULL, NULL);
    if (ret) {
        LOGW("uart tx dma error: ret=%d", ret);
        return ret;
    }
    ret = MSG_QUEUE_RECV(factory_uart_send_msg_id, &msg, 100);
    return ret;
}

POSSIBLY_UNUSED static int factory_uart_read(uint8_t *buf, uint32_t len)
{
    uint16_t msg = 0;
    union HAL_UART_IRQ_T mask = {
        .reg = 0,   // Clear all interruptions
        .RT  = 1,   // 1: Open the interrupt, 0 closed interrupt
        .FE  = 1,
        .PE  = 0,
        .BE  = 1,
        .OE  = 1,
    };
    int ret = hal_uart_dma_recv_mask((enum HAL_UART_ID_T)FACTORY_UART_ID, buf, len, NULL, NULL, &mask);
    if (ret) {
        LOGW("uart rx dma error: ret=%d", ret);
        msleep(100);
        return msg;
    }
    MSG_QUEUE_RECV(factory_uart_recv_msg_id, &msg, OS_WAIT_FOREVER);
    return msg;
}

POSSIBLY_UNUSED static void factory_uart_rx(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status)
{
    factory_uart_rx_len = 0;
    if (dma_error) {
        LOGE("uart rx dma error: xfer_size=%d", xfer_size);
    } else if (status.BE || status.FE || status.OE || status.PE) {
        LOGE("uart rx uart error: xfer_size=%d, status=0x%08x", xfer_size, status.reg);
    } else {
        factory_uart_rx_len = xfer_size;
    }
    factory_uart_rx_done = 1;
    MSG_QUEUE_SEND(factory_uart_recv_msg_id, (void *)&factory_uart_rx_len, 0);
}

POSSIBLY_UNUSED static void factory_uart_tx(uint32_t xfer_size, int dma_error)
{
    uint8_t msg = 0;
    if (dma_error) {
        LOGE("uart tx dma error: xfer_size=%d", xfer_size);
    }
    factory_uart_tx_done = 1;
    MSG_QUEUE_SEND(factory_uart_send_msg_id, &msg, 0);
}

static void factory_uart_recv_handler(uint8_t *buf, uint16_t len)
{
    if (len > 0) {
        BSP_LOG_HEX("lcd rx", buf, len);
    }

    if (s_factory_uart_event_cb) {
        s_factory_uart_event_cb(node, buf, len);
    }
}

static void factory_uart_event_thread(const void *args)
{
    uint16_t recv_len = 0;
    uint8_t *recv_buf = u_malloz(FACTORY_UART_RX_BUF_SIZE);
    while (1) {
        recv_len = factory_uart_read(recv_buf, FACTORY_UART_RX_BUF_SIZE);
        if (recv_len > 0) {
            factory_uart_recv_handler(recv_buf, recv_len); 
            factory_uart_write(recv_buf, recv_len); // 调试用，实际使用时不需要
        }
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void bsp_factory_uart_register_event_cb(uart_recv_event_cb_func cb, rt_device_node_t *dev)
{
    s_factory_uart_event_cb = cb;
    node = dev;
}

int bsp_factory_uart_send_data(uint8_t *buf, uint32_t len)
{
    return factory_uart_write(buf, len);
}

int bsp_factory_uart_init(void)
{
    int ret = 0;
    struct HAL_UART_CFG_T uart_cfg = {
        .parity = HAL_UART_PARITY_NONE,
        .stop = HAL_UART_STOP_BITS_1,
        .data = HAL_UART_DATA_BITS_8,
        .flow = HAL_UART_FLOW_CONTROL_NONE,
        .tx_level = HAL_UART_FIFO_LEVEL_1_2,
        .rx_level = HAL_UART_FIFO_LEVEL_1_4,
        .baud = FACTORY_UART_BAUDRATE,
        .dma_rx_stop_on_err = false,
        .dma_rx = true,
        .dma_tx = true,
    };

    FACTORY_UART_IOMUX_SET();
    CREATE_MSG_TYPE_QUEUE(&factory_uart_recv_msg_id, 1, sizeof(uint16_t));
    CREATE_MSG_TYPE_QUEUE(&factory_uart_send_msg_id, 1, 1);
    ret = hal_uart_open((enum HAL_UART_ID_T)FACTORY_UART_ID, &uart_cfg);
    if (ret) {
        LOGW("uart open error: ret=%d", ret);
        return ret;
    }
    hal_uart_irq_set_dma_handler((enum HAL_UART_ID_T)FACTORY_UART_ID, factory_uart_rx, factory_uart_tx);
    RUN_NEW_THREAD(&s_factory_uart_thd, "factory_uart", factory_uart_event_thread, RT_PRIO_FACTORY_UART_THREAD, (FACTORY_UART_THREAD_STACKSIZE));
    LOG_PBUS("lcd uart init success");

    return 0;
}

void bsp_factory_uart_close(void)
{
    uint32_t lock = int_lock();
    hal_uart_close((enum HAL_UART_ID_T)FACTORY_UART_ID);
    int_unlock(lock);
}
#endif
