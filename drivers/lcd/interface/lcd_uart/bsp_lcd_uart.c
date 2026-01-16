/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "hal_uart.h" // IWYU pragma: keep
#include "hal_timer.h" // IWYU pragma: keep
#include "hal_iomux.h" // IWYU pragma: keep
#include "bsp_internal.h" // IWYU pragma: keep
#include "bsp_lcd_uart.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define LCD_UART_BAUDRATE               115200
#define LCD_UART_ID                     HAL_UART_ID_3
#define LCD_UART_IOMUX_SET()            hal_iomux_set_uart3()
#define LCD_UART_RX_BUF_SIZE            (256)
#define IS_SUPPORT_LCD_UART_ASYN_RECV   (0)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static volatile uint8_t lcd_uart_rx_done = 0;
static volatile uint8_t lcd_uart_tx_done = 0;
static volatile uint16_t lcd_uart_rx_len = 0;
static thread_context_t s_lcd_uart_thd = {0};

static os_message_id lcd_recv_msg_id = NULL;
static os_message_id lcd_send_msg_id = NULL;
static os_message_id lcd_rsp_msg_id  = NULL;
static panel_recv_event_cb_func s_lcd_display_event_cb = NULL;
static panel_recv_event_cb_func s_knob_button_event_cb = NULL;
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
POSSIBLY_UNUSED static int lcd_uart_write(uint8_t *buf, uint32_t len)
{
    uint8_t msg = 0;
    int ret = 0;
    BSP_LOG_HEX("lcd tx", buf, len);
    ret = hal_uart_dma_send((enum HAL_UART_ID_T)LCD_UART_ID, (uint8_t *)buf, len, NULL, NULL);
    if (ret) {
        LOGW("uart tx dma error: ret=%d", ret);
        return ret;
    }
    ret = MSG_QUEUE_RECV(lcd_send_msg_id, &msg, 100);
    if (ret != BSP_OK) {
        LOGW("uart tx timeout");
    }
    return ret;
}

POSSIBLY_UNUSED static int lcd_uart_read(uint8_t *buf, uint32_t len)
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

    int ret = hal_uart_dma_recv_mask((enum HAL_UART_ID_T)LCD_UART_ID, buf, len, NULL, NULL, &mask);
    if (ret) {
        return msg;
    }
    MSG_QUEUE_RECV(lcd_recv_msg_id, &msg, OS_WAIT_FOREVER);

    return msg;
}

POSSIBLY_UNUSED static void lcd_uart_rx(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status)
{
    if (dma_error) {
        lcd_uart_rx_len = 0;
        LOGE("uart rx dma error: xfer_size=%d", xfer_size);
    } else if (status.BE || status.FE || status.OE || status.PE) {
        lcd_uart_rx_len = 0;
        LOGE("uart rx uart error: xfer_size=%d, status=0x%08x", xfer_size, status.reg);
    } else {
        lcd_uart_rx_len = xfer_size;
    }
    lcd_uart_rx_done = 1;
    MSG_QUEUE_SEND(lcd_recv_msg_id, (void *)&lcd_uart_rx_len, 0);
}

POSSIBLY_UNUSED static void lcd_uart_tx(uint32_t xfer_size, int dma_error)
{
    uint8_t msg = 0;
    if (dma_error) {
        LOGE("uart tx dma error: xfer_size=%d", xfer_size);
    }
    lcd_uart_tx_done = 1;
    MSG_QUEUE_SEND(lcd_send_msg_id, &msg, 0);
}

static void lcd_uart_recv_handler(uint8_t *buf, uint16_t len)
{
    EXCEPTION_CHECK_VOID(buf);
    BSP_LOG_HEX("lcd rx", buf, len);
    if (len < 5) {
        LOGE("uart recv error");
        return;
    }

    lcd_protocol_data_t *recv = (lcd_protocol_data_t *)buf;
    lcd_tlvs_t *tlvs = (lcd_tlvs_t *)recv->payload;
    if (recv->header.sof != LCD_UART_SOF) {
        LOGE("uart recv sof error");
        return;
    }

    if (recv->header.srv_id == LCD_KNOB_BUTTON_EVENT_SRV) {
        if (s_knob_button_event_cb) {
            s_knob_button_event_cb(tlvs->data[0], tlvs->data[1], NULL);
        } else {
            LOGE("knob button event callback is not registered");
        }
    }

    if (tlvs->type == LCD_RSP_ERROR_CODE && tlvs->data_size >= 2) {
        uint16_t error_code = ((tlvs->data[0] & 0xFFFF) << 8) | tlvs->data[1];
        MSG_QUEUE_SEND(lcd_rsp_msg_id, &error_code, 50);
        return;
    }
}

static void lcd_uart_event_thread(const void *args)
{
    uint16_t recv_len = 0;
    uint8_t *recv_buf = u_malloz(LCD_UART_RX_BUF_SIZE);
    while (1) {
        recv_len = lcd_uart_read(recv_buf, LCD_UART_RX_BUF_SIZE);
        if (recv_len > 0) {
            lcd_uart_recv_handler(recv_buf, recv_len);
        }
        msleep(20);
    }
}

static int bsp_lcd_data_send(lcd_tlvs_msg_t *msg) 
{
    EXCEPTION_CHECK_INT(msg);
    EXCEPTION_CHECK_INT(msg->tlvs);
    uint8_t *tlvs_data = NULL;
    uint16_t tlvs_size = 0, payload_size = 0, error_code = 0, checksum = 0;
    lcd_protocol_data_t *lcd = u_malloz(sizeof(lcd_protocol_data_t));
    for (uint8_t i = 0; i < msg->tlvs_num; i++) {
        tlvs_size = msg->tlvs[i].data_size + LCD_TLVS_HEADER_SIZE;
        if (payload_size + tlvs_size > LCD_PROTOCOL_DATA_SIZE) {
            LOGE("lcd data size is too large");
            break;
        }
        tlvs_data = (uint8_t *)&msg->tlvs[i];
        // BSP_LOG_HEX("tlvs", tlvs_data, tlvs_size);
        memcpy(lcd->payload + payload_size, tlvs_data, tlvs_size);
        payload_size += tlvs_size;
    }
    lcd->header.sof = LCD_UART_SOF;
    lcd->header.srv_id = msg->srv_id;
    uint16_t data_length = payload_size + 1;
    lcd->header.length[1] = data_length & 0xFF;
    lcd->header.length[0] = data_length >> 8;
    checksum = CRC16(lcd, payload_size + LCD_PROTOCOL_HEADER_SIZE);
    // BSP_LOG("lcd", "srv:%d, size:%d, checksum:%x", srv_id, payload_size, checksum);
    lcd->payload[payload_size] = checksum >> 8;
    lcd->payload[payload_size + 1] = checksum & 0xFF;
    payload_size += LCD_PROTOCOL_CHECKSUM_SIZE;
    int ret = lcd_uart_write((uint8_t *)lcd, payload_size + LCD_PROTOCOL_HEADER_SIZE);
    if (ret != BSP_OK) {
        LOGE("lcd uart send failed");
        goto ERROR_EXIT;
    }

    ret = MSG_QUEUE_RECV(lcd_rsp_msg_id, &error_code, 20);
    if (ret != BSP_OK) {
        LOGE("lcd rsp timeout");
        goto ERROR_EXIT;
    }

    if (error_code != LCD_RSP_SUCCESS) {
        LOGE("lcd rsp error:%x", error_code);
        ret = BSP_ERROR;
    }

ERROR_EXIT:
    u_free(lcd);
    return ret;
}

static void lcd_backlight_control(uint8_t level)
{
    lcd_tlvs_msg_t msg = {
        .srv_id = LCD_UART_DISPLAY_EVENT,
        .tlvs = u_malloz(sizeof(lcd_tlvs_t) * 1),
        .tlvs_num = 1,
    };
    EXCEPTION_CHECK_VOID(msg.tlvs);
    msg.tlvs[0].type = LCD_BACKLIGHT_LEVEL;
    msg.tlvs[0].data_size = 1;
    msg.tlvs[0].data[0] = level > 100 ? 100 : level;
    bsp_lcd_data_send(&msg);
    u_free(msg.tlvs);
}

static void bsp_lcd_uart_register_event_cb(lcd_uart_event_type_e event_type, panel_recv_event_cb_func cb)
{
    if (event_type == LCD_UART_DISPLAY_EVENT) {
        s_lcd_display_event_cb = cb;
    } else if (event_type == LCD_UART_KNOB_BUTTON_EVENT) {
        s_knob_button_event_cb = cb;
    }
}

static int bsp_lcd_uart_init(void)
{
    int ret = 0;
    struct HAL_UART_CFG_T uart_cfg = {
        .parity = HAL_UART_PARITY_NONE,
        .stop = HAL_UART_STOP_BITS_1,
        .data = HAL_UART_DATA_BITS_8,
        .flow = HAL_UART_FLOW_CONTROL_NONE,
        .tx_level = HAL_UART_FIFO_LEVEL_1_2,
        .rx_level = HAL_UART_FIFO_LEVEL_1_4,
        .baud = LCD_UART_BAUDRATE,
        .dma_rx_stop_on_err = false,
        .dma_rx = true,
        .dma_tx = true,
    };
    LCD_UART_IOMUX_SET();
    CREATE_MSG_TYPE_QUEUE(&lcd_recv_msg_id, 1, sizeof(uint16_t));
    CREATE_MSG_TYPE_QUEUE(&lcd_rsp_msg_id, 1, sizeof(uint16_t));
    CREATE_MSG_TYPE_QUEUE(&lcd_send_msg_id, 1, 1);
    ret = hal_uart_open((enum HAL_UART_ID_T)LCD_UART_ID, &uart_cfg);
    if (ret) {
        LOGW("uart open error: ret=%d", ret);
        return ret;
    }
    hal_uart_irq_set_dma_handler((enum HAL_UART_ID_T)LCD_UART_ID, lcd_uart_rx, lcd_uart_tx);
    RUN_NEW_THREAD(&s_lcd_uart_thd, "lcd_uart", lcd_uart_event_thread, RT_PRIO_LCD_THREAD, (LCD_THREAD_STACKSIZE));
    LOG_PBUS("lcd uart init success");

    return 0;
}

static void bsp_lcd_uart_close(void)
{
    uint32_t lock = int_lock();
    hal_uart_close((enum HAL_UART_ID_T)LCD_UART_ID);
    int_unlock(lock);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
__WEAK void knob_button_event_handler(int arg1, int arg2, void *param)
{

}

int bsp_lcd_init(void) 
{
    bsp_lcd_uart_register_event_cb(LCD_UART_KNOB_BUTTON_EVENT, knob_button_event_handler);
    return bsp_lcd_uart_init();
}

int bsp_lcd_deinit(void) 
{
    return BSP_OK;
}

int bsp_lcd_uart_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    uint32_t *tmp = NULL;
    switch (op_code) {
        case OP_CODE_PANEL_SET_BL:
            tmp = (uint32_t *)args;
            lcd_backlight_control(tmp[0]);
            return 0;
        case OP_CODE_PANEL_DATA_SEND:
            return bsp_lcd_data_send((lcd_tlvs_msg_t *)args);
        default:
            // LOGE("lcd err op_code: %lx", op_code);
            return -1;
    }
}

int bsp_lcd_refresh(const void *buf, uint32_t n_bytes)
{
    return 0;
}