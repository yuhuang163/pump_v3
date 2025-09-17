/***************************************************************************
 *
 * Copyright 2015-2023 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "plat_addr_map.h"

#ifdef I3C0_BASE

#include "hal_location.h"
#include "reg_i3c_hci.h"
#include "cmsis_nvic.h"
#include "hal_sleep.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_i3c.h"
#include <string.h>
#ifdef RTOS
#include "cmsis_os.h"
#endif

#ifdef I3C_DEBUG
#define HAL_I3C_TRACE(s, ...)       TRACE_IMM(0, s , ##__VA_ARGS__)
#else
#define HAL_I3C_TRACE(s, ...)       TRACE_DUMMY(0, s , ##__VA_ARGS__)
#endif

#if 1
#define HAL_I3C_ERROR(s, ...)       TRACE_IMM(0, s , ##__VA_ARGS__)
#else
#define HAL_I3C_ERROR(s, ...)       TRACE_DUMMY(0, s , ##__VA_ARGS__)
#endif

/*******************************************************************************
*************************Chip-related modification area*************************
*******************************************************************************/
typedef void (*I3C_IOMUX_SET_FUNC)(void);
typedef void (*I3C_CLK_OPEN_FUNC)(void);
typedef void (*I3C_CLK_CLOSE_FUNC)(void);
struct HAL_I3C_HW_DESC_T {
    uint32_t base;
    IRQn_Type irq;
    I3C_IOMUX_SET_FUNC iomux_func;
    I3C_CLK_OPEN_FUNC clk_open_func;
    I3C_CLK_CLOSE_FUNC clk_close_func;
};

static const struct HAL_I3C_HW_DESC_T i3c_desc[HAL_I3C_ID_NUM] = {
    {
        .base = I3C0_BASE,
        .irq = I3C0_IRQn,
        .iomux_func = hal_iomux_set_i3c0,
        .clk_open_func = hal_cmu_i3c0_clock_enable,
        .clk_close_func = hal_cmu_i3c0_clock_disable,
    },
#ifdef I3C1_BASE
    {
        .base = I3C1_BASE,
        .irq = I3C1_IRQn,
        .iomux_func = hal_iomux_set_i3c1,
        .clk_open_func = hal_cmu_i3c1_clock_enable,
        .clk_close_func = hal_cmu_i3c1_clock_disable,
    },
#endif
};

#define MAX_DEVICES                 8 //[1-32], ip cfg:8
#define DAT_TABLE_SIZE              MAX_DEVICES
#define DCT_TABLE_SIZE              MAX_DEVICES
#define PIO_CRQUEUE_SIZE            2
#define MAX_RECV_BUF_SIZE           128
/*******************************************************************************
*****************************Type definition area*******************************
*******************************************************************************/
#define CMD_QUE_RDY_TIMEOUT         2000 // ms
#define RESP_RDY_TIMEOUT            2000 // ms
#define RX_THLD_TIMEOUT             1000 // ms
#define TX_THLD_TIMEOUT             1000 // ms

// Register Values start
// Device Control Register
#define IBA_INCLUDE                 (1 << 0) //include I3C broadcast address
#define I2C_SLAVE_PRESERNT          (1 << 7) //set this bit, HC shall use HDR-TSL protocol
#define HOT_JOIN_CTRL               (1 << 8) //ACK/NACK Hot-Join request
#define HCI_ABORT                   (1 << 29)
#define HCI_RESUME                  (1 << 30)
#define HCI_ENABLE                  (1 << 31)
#define HCI_DISABLE                 (~(1 << 31))
#define HCI_DISABLE_MASK            0x7FFFFFFF

// Device Cap Fields -- indicate whether or not those mode are supported by HC
#define DAA_CAP_EN                  (1 << 0)
#define EXT_COMBO_CAP               (1 << 1)
#define HDR_DDR_EN                  (1 << 6)
#define HDR_TS_EN                   (1 << 7)

// Reset control reg fields -- only valid in PIO except SW_RST
#define SW_RST                      (1 << 0)
#define CMD_QUE_RST                 (1 << 1)
#define RSP_QUE_RST                 (1 << 2)
#define TX_FIFO_RST                 (1 << 3)
#define RX_FIFO_RST                 (1 << 4)
#define IBI_QUE_RST                 (1 << 5)

// Interrupt Status reg fields
#define INTR_TX_THLD_STS            (1 << 0) //when the number of entries in the Rx Data Queue is bigger than the Receive Buffer threshold
#define INTR_RX_THLD_STS            (1 << 1)
#define INTR_IBI_THLD_STS           (1 << 2)
#define INTR_CMD_QUE_RDY_STS        (1 << 3) //when the number of empty Command Queue entries is bigger than the CMD_EMPTY_BUF_THLD threshold
#define INTR_RSP_RDY_STS            (1 << 4)
#define INTR_TX_ABORT_STS           (1 << 5)
#define INTR_TX_ERR_STS             (1 << 9)
#define INTR_HC_INTRNL_ERR_STS      (1 << 10)

#define I3C_PIO_EXCEPTION_INTR (INTR_TX_ABORT_STS | \
                                INTR_TX_ERR_STS |   \
                                INTR_HC_INTRNL_ERR_STS)

// Interrupt Status Enable reg fields
#define INTR_STS_EN_TX_THLD         (1 << 0)
#define INTR_STS_EN_RX_THLD         (1 << 1)
#define INTR_STS_EN_IBI_THLD        (1 << 2)
#define INTR_STS_EN_CMD_QUE_RDY     (1 << 3)
#define INTR_STS_EN_RSP_RDY         (1 << 4)
#define INTR_STS_EN_TX_ABORT        (1 << 5)
#define INTR_STS_EN_TX_ERR          (1 << 9)
#define INTR_STS_EN_HC_INTRNL_ERR   (1 << 10)

#define I3C_PIO_INTR_STS_EN (INTR_STS_EN_TX_ERR |      \
                             INTR_STS_EN_TX_ABORT |    \
                             INTR_STS_EN_RSP_RDY |     \
                             INTR_STS_EN_CMD_QUE_RDY | \
                             INTR_STS_EN_IBI_THLD |    \
                             INTR_STS_EN_RX_THLD |     \
                             INTR_STS_EN_TX_THLD)

#define I3C_PIO_INTR_STS_DIS (INTR_STS_EN_TX_ERR | \
                              INTR_STS_EN_TX_ABORT)

#define I3C_PIO_INTR_STS_TX_RX_EN (INTR_STS_EN_TX_ERR |      \
                                   INTR_STS_EN_TX_ABORT |    \
                                   INTR_STS_EN_CMD_QUE_RDY | \
                                   INTR_STS_EN_RX_THLD |     \
                                   INTR_STS_EN_TX_THLD)

#define I3C_PIO_INTR_STS_TX_EN (INTR_STS_EN_TX_ERR |      \
                                INTR_STS_EN_TX_ABORT |    \
                                INTR_STS_EN_CMD_QUE_RDY | \
                                INTR_STS_EN_TX_THLD)

#define I3C_PIO_INTR_STS_RX_EN (INTR_STS_EN_TX_ERR |      \
                                INTR_STS_EN_TX_ABORT |    \
                                INTR_STS_EN_CMD_QUE_RDY | \
                                INTR_STS_EN_RX_THLD)

#define I3C_PIO_INTR_STS_RSP_EN (INTR_STS_EN_TX_ERR |   \
                                 INTR_STS_EN_TX_ABORT | \
                                 INTR_STS_EN_RSP_RDY)

// Interrupt Signal Enable reg fields
#define INTR_SIG_EN_TX_THLD         (1 << 0)
#define INTR_SIG_EN_RX_THLD         (1 << 1)
#define INTR_SIG_EN_IBI_THLD        (1 << 2)
#define INTR_SIG_EN_CMD_QUE_RDY     (1 << 3)
#define INTR_SIG_EN_RSP_RDY         (1 << 4)
#define INTR_SIG_EN_TX_ABORT        (1 << 5)
#define INTR_SIG_EN_TX_ERR          (1 << 9)
#define INTR_SIG_EN_HC_INTRNL_ERR   (1 << 10)

#define I3C_PIO_INTR_SIG_EN (INTR_SIG_EN_TX_ERR |      \
                             INTR_SIG_EN_TX_ABORT |    \
                             INTR_SIG_EN_RSP_RDY |     \
                             INTR_SIG_EN_CMD_QUE_RDY | \
                             INTR_SIG_EN_IBI_THLD |    \
                             INTR_SIG_EN_RX_THLD |     \
                             INTR_SIG_EN_TX_THLD)

#define I3C_PIO_INTR_SIG_DIS (INTR_SIG_EN_TX_ERR | \
                              INTR_SIG_EN_TX_ABORT)

#define I3C_PIO_INTR_SIG_TX_RX_EN (INTR_SIG_EN_TX_ERR |      \
                                   INTR_SIG_EN_TX_ABORT |    \
                                   INTR_SIG_EN_CMD_QUE_RDY | \
                                   INTR_SIG_EN_RX_THLD |     \
                                   INTR_SIG_EN_TX_THLD)

#define I3C_PIO_INTR_SIG_TX_EN (INTR_SIG_EN_TX_ERR |      \
                                INTR_SIG_EN_TX_ABORT |    \
                                INTR_SIG_EN_CMD_QUE_RDY | \
                                INTR_SIG_EN_TX_THLD)

#define I3C_PIO_INTR_SIG_RX_EN (INTR_SIG_EN_TX_ERR |      \
                                INTR_SIG_EN_TX_ABORT |    \
                                INTR_SIG_EN_CMD_QUE_RDY | \
                                INTR_SIG_EN_RX_THLD)

#define I3C_PIO_INTR_SIG_RSP_EN (INTR_SIG_EN_TX_ERR |   \
                                 INTR_SIG_EN_TX_ABORT | \
                                 INTR_SIG_EN_RSP_RDY)

// Interrupt Force reg fields
#define INTR_FORCE_HC_INTRNL_ERR    (1 << 10)

#define INTR_FORCE_TX_THLD          (1 << 0)
#define INTR_FORCE_RX_THLD          (1 << 1)
#define INTR_FORCE_IBI_THLD         (1 << 2)
#define INTR_FORCE_CMD_QUE_RDY      (1 << 3)
#define INTR_FORCE_RSP_RDY          (1 << 4)
#define INTR_FORCE_TX_ABORT         (1 << 5)
#define INTR_FORCE_TX_ERR           (1 << 9)

// IBI Queue control fields
#define NOTIFY_HJ_RJCT              (1 << 0) //0:disabled  1:disabled
#define NOTIFY_SIR_RJCT             (1 << 3) //en/dia rejection of IBI request

// RH Interupt Status Reg Fields
#define RH_INTR_TX_ABORT_STS        (1 << 5)
#define RH_INTR_IBI_RING_FULL_STS   (1 << 6) //write 1 indicate lack of space on IBI status or IBI data ring
#define RH_INTR_TX_ERR_STS          (1 << 9)
#define RH_INTR_RING_OP_STS         (1 << 10)
#define RH_INTR_TX_DONE_STS         (1 << 11)
#define RH_INTR_IBI_READY_STS       (1 << 12)

#define I3C_DMA_EXCEPTION_INTR (RH_INTR_TX_ABORT_STS | \
                                RH_INTR_TX_ERR_STS)

// RH Interrupt status enable register fields
#define RH_INTR_STS_EN_TX_ABORT         (1 << 5)
#define RH_INTR_STS_EN_IBI_RING_FULL    (1 << 6)
#define RH_INTR_STS_EN_TX_ERR           (1 << 9)
#define RH_INTR_STS_EN_RING_OP          (1 << 10)
#define RH_INTR_STS_EN_TX_DONE          (1 << 11)
#define RH_INTR_STS_EN_IBI_READY        (1 << 12)

// RH Interrupt signal enable register fields
#define RH_INTR_SIG_EN_TX_ABORT         (1 << 5)
#define RH_INTR_SIG_EN_IBI_RING_FULL    (1 << 6)
#define RH_INTR_SIG_EN_TX_ERR           (1 << 9)
#define RH_INTR_SIG_EN_RING_OP          (1 << 10)
#define RH_INTR_SIG_EN_TX_DONE          (1 << 11)
#define RH_INTR_SIG_EN_IBI_READY        (1 << 12)

// RH Interrupt force register fields
#define RH_INTR_FORCE_TX_ABORT_STS      (1 << 5)
#define RH_INTR_FORCE_IBI_RING_FULL_STS (1 << 6)
#define RH_INTR_FORCE_TX_ERR_STS        (1 << 9)
#define RH_INTR_FORCE_RING_OP_STS       (1 << 10)
#define RH_INTR_FORCE_TX_DONE_STS       (1 << 11)
#define RH_INTR_FORCE_IBI_READY_STS     (1 << 12)

#define I3C_DMA_INTR_STS_EN (RH_INTR_STS_EN_TX_ABORT |      \
                             RH_INTR_STS_EN_IBI_RING_FULL | \
                             RH_INTR_STS_EN_TX_ERR |        \
                             RH_INTR_STS_EN_RING_OP |       \
                             RH_INTR_STS_EN_TX_DONE |       \
                             RH_INTR_STS_EN_IBI_READY)

#define I3C_DMA_INTR_SIG_EN (RH_INTR_SIG_EN_TX_ABORT |      \
                             RH_INTR_SIG_EN_IBI_RING_FULL | \
                             RH_INTR_SIG_EN_TX_ERR |        \
                             RH_INTR_SIG_EN_RING_OP |       \
                             RH_INTR_SIG_EN_TX_DONE |       \
                             RH_INTR_SIG_EN_IBI_READY)

// Ring status fields
#define RH_ENABLED_MASK                 (1 << 0)
#define RH_RUNNING_MASK                 (1 << 1)
#define RH_ABORTED_MASK                 (1 << 2)
#define RH_LOCKED_MASK                  (1 << 3)

// Ring control fields
#define RH_ENABLE_REQ                   (1 << 0)
#define RH_RS_REQ                       (1 << 1) // Run/Stop request
#define RH_ABORT_REQ                    (1 << 2)

// Ring Operation1 Mask
#define CR_ENQ_PTR_MASK                 0xFF
#define CR_SW_DQ_PTR_MASK               0xFF00
#define IBI_DQ_PTR_MASK                 0xFF0000

// Ring Operation2 MAsk
#define CR_DQ_PTR_MASK                  0xFF
#define IBI_ENQ_PTR_MASK                0xFF0000

//----------------Mask Values----------------
//(DCT_SECTION_OFFSET) (BASE + 0x34)
#define DCT_TBL_SIZE_MASK               (0x7F << 12) // 18:12
#define DCT_TBL_INDEX_MASK              (0xF8 << 19) // 23:19
//(HC_CONTROL) (BASE + 0x4)
#define IBA_INCLUDE_MASK                (1 << 0)
#define MODE_SELECTOR_PIO_MASK          (1 << 6)
#define I2C_SLAVE_PRE_MASK              (1 << 7)
//(HC_CAPABILITIES) (BASE + 0xC)
#define COMBO_CMD_EN_MASK               (1 << 2)
#define AUTO_CMD_EN_MASK                (1 << 3)
#define HDR_DDR_EN_MASK                 (1 << 6)
#define CMD_CCC_DEFBYTE_EN_MASK         (1 << 10)
// (CR_SETUP) (RH+ 0x0)
#define XFER_STRUCT_SIZE_MASK           0xFF000000
#define RSP_STRUCT_SIZE_MASK            0xFF0000

// Response Queue Port(+0x4)
#define RSP_DQPTR_MASK                  0x000000FF
#define RSP_SWDQ_PTR_MASK               0xFFFF00FF
#define RSP_PIO_TID_MASK                0x0000000F
#define RSP_PIO_LEN_MASK                0x0000FFFF

// Preamble Reg
#define RING_SIZE_MASK                  0xFF0000
#define RING_SUPPORT_MASK               0xF0
#define RING_PREAMBLE_MASK              0xFF000000

// DAT masks & defines
#define DAT_I3C_DEVICE                  0x0 //dev type
#define DAT_I2C_DEVICE                  0x1
#define NACK_RETRY_CNT                  0x3 // 0x03 is the max value to be assigned
#define NEXT_4B_OFFSET                  0x4

#define DAT_SA_MASK                     0x3F
#define DAT_DA_MASK                     0xFF0000
#define DAT_RING_ID_MASK                0x6000000
#define DAT_DEV_RETRY_CNT               0x60000000 //device retries on NACK
#define DAT_DEV_TYPE_MASK               0x10000000
#define DAT_AUTOCMD_MASK                0xFF
#define DAT_AUTOCMD_VAL_MASK            0xFF00

// DCT masks
#define DCT_DA_MASK                     0xFF
#define DCT_DCR_MASK                    0xFF   //indicate device type
#define DCT_BCR_MASK                    0xFF00 //device role��offline capable��IBI req capable��max speed limitation...
#define DCT_PID_LOW_MASK                0xFF

// Interrupt Return
#define INTR_BIT_SET                    1
#define INTR_BIT_RESET                  0

#define I3C_MAX_ENTRIES                 255

#define I2C_DEVICE                      0x1
#define I3C_DEVICE                      0x0

#define BC_DEVICES_ADDR                 0x7F

// CmdAttributes
#define CMD_ATRB_REGULAR_TX             0
#define CMD_ATRB_IMM_DATA_TX            1
#define CMD_ATRB_DAA                    2
#define CMD_ATRB_COMBO_TX               3
#define CMD_ATRB_INTERNAL_CTRL          7

#define ROC_ON                          1 //response on completion    always set 1 in DMA
#define ROC_OFF                         0

#define TOC_STOP                        1 //terminal on completion  control sent Sr/P after command complete
#define TOC_START                       0

#define IOC_COMPLETE                    0x1
#define IOC_CONTINUE                    0x0

// Tx Modes
#define DMA_MODE                        1
#define PIO_MODE                        0

// Tx Direction
#define RD_XFER                         1
#define WR_XFER                         0

// Ring IDs
#define MAX_RINGS                       2
#define RING_ID_0                       0
#define RING_ID_1                       1
#define RING_ID_2                       2
#define RING_ID_3                       3
#define RING_ID_4                       4
#define RING_ID_5                       5
#define RING_ID_6                       6
#define RING_ID_7                       7

#define RINGHDRS_RHx_OFFSET(n)         (RINGHDRS_RH_OFFSET + 0x80 * n)
#define RHx_REG(n, i)                  (RINGHDRS_RHx_OFFSET(n) + 0x04 * i)

// Structures
// 5.1.11 Device Address Table Section Offset (DAT_SECTION_OFFSET) (BASE + 0x30)
struct HAL_I3C_DATREG {
    uint32_t u12TableOffset : 12, // 11:0   Rd only--(200)   offset of DAT relative to HCI base
             u07TableSize : 7,    // 18:12  Rd only--(4)  size of DAT, in entries
             u09Rsvd27d19 : 9,    // 27:19  Rd only
             u04EntrySize : 4;    // 31:28  Rd only--(0) defauly:DAT table entry size is 2 DWORDS
};

// 5.2.1 Command Queue Port (COMMAND_QUEUE_PORT) (pio + 0x0)
// 5.2.1.1 Address Assignment Command
struct HAL_I3C_CMDDESC_DAA {
    uint32_t u03CmdAttr : 3,  // 2:0
             u04TID : 4,      // 6:3
             u08CMD : 8,      // 14:7
             u01Rsvd_AAC : 1, // 15
             u05DevIndex : 5, // 20:16   Indicates the DAT table index for the Target Device being assigned an address
             u05Rsvd_AAC : 5, // 25:21
             u04Dev_cnt : 4,  // 29:26   indicate the number of devices that a dy addr shall be assigned to; DAT entries are processing from DEV_INDEX  up to DEV_CNT
             u01ROC : 1,      // 30
             u01TOC : 1;      // 31
    uint32_t u32Rsvd_AAC;     // 63:32
};

// 5.2.1.5 Internal Control Command
struct HAL_I3C_CMDDESC_ICC {
    uint32_t u03CmdAttr : 3,             // 2:0
             u04TID : 4,                 // 6:3
             u01Vendor_info_present : 1, // 7
             u04Mipi_cmd : 4,            // 11:8
             u20Mipi_rsvd : 20;          // 31:12
    uint32_t u32Vendor_specific;         // 63:32
};

// 5.2.1.2 Immediate Data Transfer Command
struct HAL_I3C_CMDDESC_IDTC {
    uint32_t u03CmdAttr : 3,    // 2:0
             u04TID : 4,        // 6:3
             u08CMD : 8,        // 14:7
             u01CP : 1,         // 15
             u05DevIndex : 5,   // 20:16
             u02Rsvd_IDTC : 2,  // 22:21
             u03Byte_cnt : 3,   // 25:23
             u03Speed_mode : 3, // 28:26
             u01Dir_rnw : 1,    // 29
             u01ROC : 1,        // 30
             u01TOC : 1;        // 31
    uint32_t u8ImmByt1 : 8,     // 39:32
             u8ImmByt2 : 8,     // 47:40
             u8ImmByt3 : 8,     // 55:48
             u8ImmByt4 : 8;     // 63:56
};

// 5.2.1.3 Regular Data Transfer Command
struct HAL_I3C_CMDDESC_RDTC {
    uint32_t u03CmdAttr : 3,             // 2:0
             u04TID : 4,                 // 6:3
             u08CMD : 8,                 // 14:7  CCC/HDR command code value
             u01CP : 1,                  // 15    command present: indicate whether or not CMD field is valid
             u05DevIndex : 5,            // 20:16
             u03Rsvd_RDTC : 3,           // 23:21
             u01Short_read_err : 1,      // 24    valid for read only, shord_read: not return the num of bytes requested by DATA_LENGTH
             u01Define_byte_present : 1, // 25    if 1, field DEF_VYTE contains the Defining byte value
             u03Speed_mode : 3,          // 28:26
             u01Dir_rnw : 1,             // 29
             u01ROC : 1,                 // 30
             u01TOC : 1;                 // 31
    uint32_t u8Define_byte : 8,          // 39:32
             u8Rsvd_RDTC : 8,            // 47:40
             u16Data_len : 16;           // 63:48
};

// 5.2.1.4 Combo transfer Command
struct HAL_I3C_CMDDESC_CTC {
    uint32_t u03CmdAttr : 3,           // 2:0
             u04TID : 4,               // 6:3
             u08CMD : 8,               // 14:7
             u01CP : 1,                // 15
             u05DevIndex : 5,          // 20:16
             u01Rsvd_CTC : 1,          // 21
             u02Data_len_position : 2, // 23:22
             u01Fist_phase_mode : 1,   // 24
             u01Sub_offset : 1,        // 25
             u03Speed_mode : 3,        // 28:26
             u01Dir_rnw : 1,           // 29
             u01ROC : 1,               // 30
             u01TOC : 1;               // 31
    uint32_t u16Offset_suboffset : 16, // 47:32
             u16Data_len : 16;         // 63:48
};

// Structure to represent Response descriptor
// 5.2.2 Response Queue Port (RESPONSE_QUEUE_PORT) (pio + 0x4)
struct HAL_I3C_RESP_DESC {
    uint32_t u16DataLen : 16, // 15:0
             u08Rsvd : 8,     // 23:16
             u04TID : 4,      // 27:24
             u04ErrSts : 4;   // 31:28
};

// 5.2.4 IBI Port (IBI_PORT) (+ 0xC)
// 5.2.4.2 IBI Status Descriptor
struct HAL_I3C_IBISTS_DESC {
    uint32_t u08DataLen : 8,         // 7:0
             u08IBIId : 8,           // 15:8
             u08Chunks : 8,          // 23:16
             u01LastSts : 1,         // 24
             u01TimestampMarker : 1, // 25
             u03HwContext : 3,       // 28:26
             u01StsType : 1,         // 29
             u01Error : 1,           // 30
             u01IBISts : 1;          // 31
};

// 5.2.5 Queue Threshold Control (QUEUE_THLD_CTRL) (+ 0x10)
struct HAL_I3C_PIO_QUE_THLD_CTRL {
    uint32_t u8CmdEmptyBufThld : 8, // 7:0    1-255 entries                  set 1 in code
             u8RespBufThld : 8,     // 15:8   1-255 entries                  set 1 in code
             u8IBIDateSegSize : 8,  // 23:16  1-63 DWORDS = 4-252 bytes      set 1 in code
             u8IBIStatusThld : 8;   // 31:24  1-255 entries                  set 1 in code
};

// 5.2.6 Transfer Data Buffer Threshold Control (DATA_BUFFER_THLD_CTRL) (+ 0x14)
struct HAL_I3C_PIO_DATA_BUF_THLD_CTRL {
    uint32_t u3TxBufThld : 3,     // 2:0    minimum number of TX FIFO entries that will trigger THLD_INTR   2^(N+1) DWORD   N:0-7  set 0 in code
             u5Rsvd1_PIOCtrl : 5, // 7:3
             u3RxBufThld : 3,     // 10:8   minimum number of RX FIFO entries that will trigger THLD_INTR   2^(N+1) DWORD   N:0-7  set 0 in code
             u5Rsvd2_PIOCtrl : 5, // 15:11
             u3TxStartThld : 3,   // 18:16  wait until 2^(N+1) DWORD entries are available to initiate transfer   N:0-7    for TX buffer  set 1 in code
             u5Rsvd3_PIOCtrl : 5, // 23:19
             u3RxStartThld : 3,   // 26:24  wait until 2^(N+1) DWORD entries are available  initiate transfer     N:0-7    for RX buffer  set 1 in code
             u5Rsvd4_PIOCtrl : 5; // 31:27
};

// 5.2.7 Queue Size (QUEUE_SIZE_CTRL) (PIO + 0x18)
struct HAL_I3C_PIO_QUE_SIZE_CTRL {
    uint32_t u8CRQueSize : 8,  // 7:0,   Rd Only--(2) size of command/response queues, in entries. 2-255 entries (CD size: 2 DWORDS, RD SIZE: 1 WORD)
             u8IBIStsSize : 8, // 15:8,  Rd Only--(A) IBI queue size  2-255 DWORDS
             u8RxBufSize : 8,  // 23:16, Rd Only--(2) Rx Buffer size  2^(N+1) DWORD   N:0-7
             u8TxBufSize : 8;  // 31:24, Rd Only--(2) Tx Buffer size  2^(N+1) DWORD   N:0-7
};

// DMA Structures
// 5.3.1 Ring Headers Preamble (RHS_CONTROL) (RHS+ 0x0)
struct HAL_I3C_PREAMBLE {
    uint32_t u4Max_header_cnt : 4,     // 3:0    maximum header count by software  0-8
             u4Max_header_cnt_cap : 4, // 7:4    Rd only(8)--maximum header count by HC  0-8
             u8Rsvd : 8,               // 15:8
             u8HeaderSize : 8,         // 23:16  Rd only(5)--ring header size, per ring header descriptor strct, 2^N
             u8PreambleSize : 8;       // 31:24  Rd only(0x10)--preamble section size, 0x10 allows for 8 ring headers
};

// 5.4 Register Description- Device Address Table (DAT)
struct HAL_I3C_SLAVE_DAT {
    uint32_t u07StaticAddr : 7,      // 6:0
             u05Rsvd1 : 5,           // 11:7
             u01IBIPayload : 1,      // 12
             u01IBIReject : 1,       // 13
             u01MRReject : 1,        // 14
             u01TS : 1,              // 15
             u08DynAddr : 8,         // 23:16
             u02Rsvd2 : 2,           // 25:24
             u03RingID : 3,          // 28:26
             u02DevNackRetryCnt : 2, // 30:29
             u01Device : 1;          // 31, 0=>i3c, 1=>i2c
    uint32_t u08AutoCmdMask : 8,     // 39:32
             u08AutoCmdValue : 8,    // 47:40
             u03AutoCmdMode : 3,     // 50:48
             u08AutoCmdHdrCode : 8,  // 58:51
             u05Rsvd : 5;            // 63:59
};

// 5.5 Register Description - Device Characteristic Table (DCT)
struct HAL_I3C_SLAVE_DCT {
    uint32_t u32PID_LO;      // 31:0
    uint32_t u16PID_HI : 16, // 47:32
             u16Rsvd1 : 16;  // 63:48
    uint32_t u08DCR : 8,     // 71:64
             u08BCR : 8,     // 79:72
             u16Rsvd2 : 16;  // 95:80
    uint32_t u08DynAddr : 8, // 103:96
             u24Rsvd3 : 24;  // 127:104
};

// User str - to add CMD desc and data
struct HAL_I3C_CMD_DATA_DESC {
    struct HAL_I3C_CMDDESC_RDTC *pstCmdDesc;
    uint8_t *pu8Buf;
    struct HAL_I3C_RESP_DESC *pstRspDesc;
};

/********The following 2 types of definitions are from the I3C standard********/
// Slave Structure to represent BCR
struct HAL_I3C_SLAVE_BCR {
    uint8_t u1MaxSpdLimit : 1, // 0
            u1IBIReqCap : 1,   // 1
            u1IBIPayload : 1,  // 2
            u1OfflineCap : 1,  // 3
            u1VirtualDev : 1,  // 4
            u1AdvancedCap : 1, // 5
            u2DevRole : 2;     // 7:6
};
// Slave Structure to represent DCR
//be used to describe dev type, 0-255, 0: generic dev
struct HAL_I3C_SLAVE_DCR {
    uint8_t u8DevID0 : 1, // 0
            u8DevID1 : 1, // 1
            u8DevID2 : 1, // 2
            u8DevID3 : 1, // 3
            u8DevID4 : 1, // 4
            u8DevID5 : 1, // 5
            u8DevID6 : 1, // 6
            u8DevID7 : 1; // 7
};

// PIO Structures
// Structure to represent PIO Transfer - generic for Tx/Rx/IBI
struct HAL_I3C_PIO_XFER {
    uint32_t u32CmdFrameHi;
    uint32_t u32CmdFrameLo;
    uint16_t u16DataLenth;
    uint8_t *pstData;
    uint8_t u8DataDir;
    uint32_t u32NoOfEntries;
    uint8_t u08CmdAttr;
};

// Structure to represent Tx descriptor,The minimum size shall be 20 bytes
struct HAL_I3C_RING_TX_DESC {
    uint32_t u32CMDDescAddrLow;
    uint32_t u32CMDDescAddrHi;
    uint16_t u16DataLength;
    uint8_t u8IOC;
    uint8_t u8BLP;
    uint32_t u32DataAddrLow;
    uint32_t u32DataAddrHi;
};

// Structure to represent Ring in the DMA
struct HAL_I3C_DMA_RING_XFER {
    struct HAL_I3C_RING_TX_DESC *pstTxDesc;
    struct HAL_I3C_RESP_DESC *pstRsp;
    // IBI Rings to be added - TBD
};

// Structure to represent Ring Cap
// 5.3.10 CR Control (CR_SETUP) (RH+ 0x0)
struct HAL_I3C_RING_CAP {
    uint8_t u8RingSize;    // 7:0    R/W--(2)  2-255, expresses as a number of paired transfer/response descriptors
    uint8_t u8RespStrSize; // 23:16  Rd only--(0x4)
    uint8_t u8XferStrSize; // 31:24  Rd only--(0x14)
    // TBD - IBI Sizes
};

// DAT Structure
// Structure to represent I3C bus info
struct HAL_I3C_BUS_CTRL_INFO {
    uint8_t combo_cmd_en;
    uint8_t auto_cmd_en;
    uint8_t standby_cr_cap_en;
    uint8_t hdr_ddr_en;
    uint8_t hdr_ts_en;
    uint8_t cmd_ccc_def_byte_en;
    uint8_t sg_cap_cr_en;
    uint8_t sg_cap_ibi_en;
    uint8_t sg_cap_dc_en;

    uint8_t dynamic_addr;

    uint8_t u8HWMaxRings;
    uint16_t u16DATSize; // dev cnt - one entry for each dev

    uint8_t u8DCTIndex;
    uint16_t u16DCTSize;

    uint8_t cmdque_rdy_compln;
    uint8_t rsp_rdy_compln;
    uint8_t rx_thld_compln;
    uint8_t tx_thld_compln;
    uint8_t dma_xfer_done;
    uint8_t error_flag;
    uint8_t dat_update_en : 1; //for IBI
    uint8_t resp_rdy : 1;

    uint32_t read_index;
    uint32_t words_cnt; // number of 4bytes

    struct HAL_I3C_SLAVE_DAT *pstSlvDAT;
    struct HAL_I3C_SLAVE_DCT *pstSlvDCT;
    struct HAL_I3C_SLAVE_BCR *BCR;
    struct HAL_I3C_PREAMBLE *pstPreamble;
    struct HAL_I3C_PIO_QUE_SIZE_CTRL *pstPIOCtrl;
    struct HAL_I3C_DMA_RING_XFER *pstRingDesc[MAX_RINGS];
    struct HAL_I3C_PIO_XFER *pstPIOTx;
    struct HAL_I3C_PIO_XFER PIOTx;
    struct HAL_I3C_RING_CAP *pstRingCap[MAX_RINGS];
    struct HAL_I3C_CMD_DATA_DESC *glpstCmdData;
    struct HAL_I3C_HOST_T *priv;
};

enum HAL_I3C_DIV_FACTOR {
    SDR0_DIV = 4,
    SDR1_DIV = 6,
    SDR2_DIV = 8,
    SDR3_DIV = 12,
    SDR4_DIV = 24,
};

enum HAL_I3C_DMA_REG {
    CR_SETUP            = 1,
    IBI_SETUP           = 2,
    CHUNK_CTRL          = 3,
    RING_INTR_ST        = 4,
    RING_INTR_ST_EN     = 5,
    RING_INTR_SIG_EN    = 6,
    RING_INTR_FORCE     = 7,
    RING_STATUS         = 8,
    RING_CTRL           = 9,
    RING_OPR1           = 10,
    RING_OPR2           = 11,
    CMD_BASE_L          = 12,
    CMD_BASE_H          = 13,
    RSP_BASE_L          = 14,
    RSP_BASE_H          = 15,
    IBI_ST_BASE_L       = 16,
    IBI_ST_BASE_H       = 17,
    IBI_DAT_BASE_L      = 18,
    IBI_DAT_BASE_H      = 19,
    CMD_RING_SG         = 20,
    RESP_RING_SG        = 21,
    IBI_STATUS_RING_SG  = 22,
    IBI_DATA_RING_SG    = 23,
};

/*******************************************************************************
*************************Global variable definition area************************
*******************************************************************************/
#ifndef I3C_HCI_REG_OFFSET_READ_SKIP_EN
static uint16_t u16glValue;
static uint32_t u32glValue;
#endif

struct HAL_I3C_HOST_T {
    uint8_t id;
    uint32_t base;
    uint32_t src_clk;
    uint32_t bus_speed;
    uint8_t dev_cnt : 4;
    uint8_t init_cnt : 1;
    uint8_t use_dma : 1;
    uint8_t i2c_dev_present : 1;
    uint8_t inc_broadcast_addr : 1;
    uint8_t *static_addr;
    enum HAL_I3C_MODE_T mode;

    struct HAL_I3C_BUS_CTRL_INFO bus_ctrl_info;
    struct HAL_I3C_PIO_QUE_SIZE_CTRL pio_ctrl;
};

static const char *const err_invalid_id = "Invalid I3C ID: %d";
static HAL_I3C_IRQ_HANDLER_T irq_handler[HAL_I3C_ID_NUM] = { NULL };

static struct HAL_I3C_HOST_T i3c_host[HAL_I3C_ID_NUM] = {0};

SYNC_FLAGS_LOC static uint8_t i3c_pio_queue[HAL_I3C_ID_NUM][sizeof(struct HAL_I3C_PIO_XFER) * PIO_CRQUEUE_SIZE];
SYNC_FLAGS_LOC static uint8_t i3c_slv_dat[HAL_I3C_ID_NUM][sizeof(struct HAL_I3C_SLAVE_DAT) * DAT_TABLE_SIZE];
SYNC_FLAGS_LOC static uint8_t i3c_slv_dct[HAL_I3C_ID_NUM][sizeof(struct HAL_I3C_SLAVE_DCT) * DCT_TABLE_SIZE];
SYNC_FLAGS_LOC static uint8_t i3c_slv_bcr[HAL_I3C_ID_NUM][sizeof(struct HAL_I3C_SLAVE_BCR) * DCT_TABLE_SIZE];

SYNC_FLAGS_LOC static uint8_t i3c_ring_cap[HAL_I3C_ID_NUM][sizeof(struct HAL_I3C_RING_CAP) * MAX_RINGS];
SYNC_FLAGS_LOC static uint8_t i3c_ring_desc[HAL_I3C_ID_NUM][sizeof(struct HAL_I3C_DMA_RING_XFER) * MAX_RINGS];

SYNC_FLAGS_LOC static uint8_t Ring_TxDesc[HAL_I3C_ID_NUM][MAX_RINGS][sizeof(struct HAL_I3C_RING_TX_DESC) * 8];//RING_SIZE = 8
SYNC_FLAGS_LOC static uint8_t Ring_RespDesc[HAL_I3C_ID_NUM][MAX_RINGS][sizeof(struct HAL_I3C_RESP_DESC) * 8]; //RING_SIZE = 8
SYNC_FLAGS_LOC static uint8_t __attribute__((aligned(4))) aligned_recv_buf[MAX_RECV_BUF_SIZE];

static int32_t hal_i3c_pio_cmd_tx(enum HAL_I3C_ID_T id, struct HAL_I3C_PIO_XFER *pstPIOTx);
/*******************************************************************************
**************************Register operation interface**************************
*******************************************************************************/
static inline void i3c_reg_read32(enum HAL_I3C_ID_T id, uint32_t u32Offset, uint32_t *pu32Value)
{
    *pu32Value = *((volatile uint32_t *)(i3c_desc[id].base + u32Offset));
}

static inline void i3c_reg_write32(enum HAL_I3C_ID_T id, uint32_t u32Offset, uint32_t u32Value)
{
    *((volatile uint32_t *)(i3c_desc[id].base + u32Offset)) = u32Value;
}

POSSIBLY_UNUSED static inline void i3c_reg_read16(enum HAL_I3C_ID_T id, uint32_t u32Offset, uint16_t *pu16Value)
{
    *pu16Value = *((volatile uint16_t *)(i3c_desc[id].base + u32Offset));
}

POSSIBLY_UNUSED static inline void i3c_reg_write16(enum HAL_I3C_ID_T id, uint32_t u32Offset, uint16_t u16Value)
{
    *((volatile uint16_t *)(i3c_desc[id].base + u32Offset)) = u16Value;
}

POSSIBLY_UNUSED static inline void i3c_reg_read8(enum HAL_I3C_ID_T id, uint32_t u32Offset, uint8_t *pu8Value)
{
    *pu8Value = *((volatile uint8_t *)(i3c_desc[id].base + u32Offset));
}

POSSIBLY_UNUSED static inline void i3c_reg_write8(enum HAL_I3C_ID_T id, uint32_t u32Offset, uint8_t u8Value)
{
    *((volatile uint8_t *)(i3c_desc[id].base + u32Offset)) = u8Value;
}

/*******************************************************************************
****************************master operation interface**************************
*******************************************************************************/
/*********************************************************************
Argument     : None
Return Value : pointer to I3CBusCtrlInfo structure
Description  : This function returns pointer to I3CBusCtrlInfo structure.
*********************************************************************/
static struct HAL_I3C_BUS_CTRL_INFO *hal_i3c_get_bus_info(enum HAL_I3C_ID_T id)
{
    return &i3c_host[id].bus_ctrl_info;
}

static int32_t wait_for_completion_timeout(enum HAL_I3C_ID_T id, uint8_t *semaphore, uint8_t value, uint32_t timeout_ms, uint8_t yield)
{
    uint32_t timeout = MS_TO_FAST_TICKS(timeout_ms), busy_t;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    busy_t = hal_fast_sys_timer_get();
    while (hal_fast_sys_timer_get() - busy_t < timeout) {
        if (bus_info->error_flag) {
            bus_info->error_flag = 0;
            HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
            return I3C_ERR_TRANSFER;
        } else if (*semaphore == value) {
            return I3C_ERR_NONE;
        } else if (yield) {
            osDelay(1);
        }
    }

    HAL_I3C_ERROR("ERR %d, %s, time:%dms, yield:%d", I3C_ERR_TIMEOUT, __func__, timeout_ms, yield);
    return I3C_ERR_TIMEOUT;
}

/*********************************************************************
Return Value : None
Description  : This API is used to print the command descriptor.
*********************************************************************/
static void hal_i3c_print_cmd_desc(struct HAL_I3C_CMD_DATA_DESC *desc)
{
    HAL_I3C_TRACE("----------------------------------");
    HAL_I3C_TRACE("     Printing CmdDataDesc Values");
    HAL_I3C_TRACE("----------------------------------");
    HAL_I3C_TRACE("CmdAttr              :0x%X", desc->pstCmdDesc->u03CmdAttr);
    HAL_I3C_TRACE("TID                  :0x%X", desc->pstCmdDesc->u04TID);
    switch (desc->pstCmdDesc->u03CmdAttr) {
        case CMD_ATRB_DAA: {
            struct HAL_I3C_CMDDESC_DAA *pstCmdDesc;
            pstCmdDesc = (struct HAL_I3C_CMDDESC_DAA *)desc->pstCmdDesc;
            HAL_I3C_TRACE("CMD                  :0x%X", pstCmdDesc->u08CMD);
            HAL_I3C_TRACE("DevIndex             :0x%X", pstCmdDesc->u05DevIndex);
            HAL_I3C_TRACE("Device Count         :0x%X", pstCmdDesc->u04Dev_cnt);
            HAL_I3C_TRACE("ROC                  :0x%X", pstCmdDesc->u01ROC);
            HAL_I3C_TRACE("TOC                  :0x%X", pstCmdDesc->u01TOC);
            break;
        }
        case CMD_ATRB_IMM_DATA_TX: {
            struct HAL_I3C_CMDDESC_IDTC *pstCmdDesc;
            pstCmdDesc = (struct HAL_I3C_CMDDESC_IDTC *)desc->pstCmdDesc;
            HAL_I3C_TRACE("CMD                  :0x%X", pstCmdDesc->u08CMD);
            HAL_I3C_TRACE("CP                   :0x%X", pstCmdDesc->u01CP);
            HAL_I3C_TRACE("DevIndex             :0x%X", pstCmdDesc->u05DevIndex);
            HAL_I3C_TRACE("Type and byte cnt    :0x%X", pstCmdDesc->u03Byte_cnt);
            HAL_I3C_TRACE("Mode and speed       :0x%X", pstCmdDesc->u03Speed_mode);
            HAL_I3C_TRACE("RnW                  :%s",   pstCmdDesc->u01Dir_rnw == 0 ? "write" : "read");
            HAL_I3C_TRACE("ROC                  :0x%X", pstCmdDesc->u01ROC);
            HAL_I3C_TRACE("TOC                  :0x%X", pstCmdDesc->u01TOC);
            HAL_I3C_TRACE("ImmByt1              :0x%X", pstCmdDesc->u8ImmByt1);
            HAL_I3C_TRACE("ImmByt2              :0x%X", pstCmdDesc->u8ImmByt2);
            HAL_I3C_TRACE("ImmByt3              :0x%X", pstCmdDesc->u8ImmByt3);
            HAL_I3C_TRACE("ImmByt4              :0x%X", pstCmdDesc->u8ImmByt4);
            break;
        }
        case CMD_ATRB_REGULAR_TX: {
            struct HAL_I3C_CMDDESC_RDTC *pstCmdDesc;
            pstCmdDesc = (struct HAL_I3C_CMDDESC_RDTC *)desc->pstCmdDesc;
            HAL_I3C_TRACE("CMD                  :0x%X", pstCmdDesc->u08CMD);
            HAL_I3C_TRACE("CP                   :0x%X", pstCmdDesc->u01CP);
            HAL_I3C_TRACE("DevIndex             :0x%X", pstCmdDesc->u05DevIndex);
            HAL_I3C_TRACE("Short read err       :0x%X", pstCmdDesc->u01Short_read_err);
            HAL_I3C_TRACE("Define byte present  :0x%X", pstCmdDesc->u01Define_byte_present);
            HAL_I3C_TRACE("Mode and speed       :0x%X", pstCmdDesc->u03Speed_mode);
            HAL_I3C_TRACE("RnW                  :%s",   pstCmdDesc->u01Dir_rnw == 0 ? "write" : "read");
            HAL_I3C_TRACE("ROC                  :0x%X", pstCmdDesc->u01ROC);
            HAL_I3C_TRACE("TOC                  :0x%X", pstCmdDesc->u01TOC);
            HAL_I3C_TRACE("Define byte value    :0x%X", pstCmdDesc->u8Define_byte);
            HAL_I3C_TRACE("Data length          :0x%X", pstCmdDesc->u16Data_len);
            break;
        }
        case CMD_ATRB_COMBO_TX: {
            struct HAL_I3C_CMDDESC_CTC *pstCmdDesc;
            pstCmdDesc = (struct HAL_I3C_CMDDESC_CTC *)desc->pstCmdDesc;
            HAL_I3C_TRACE("CMD                  :0x%X", pstCmdDesc->u08CMD);
            HAL_I3C_TRACE("CP                   :0x%X", pstCmdDesc->u01CP);
            HAL_I3C_TRACE("DevIndex             :0x%X", pstCmdDesc->u05DevIndex);
            HAL_I3C_TRACE("Datalength position  :0x%X", pstCmdDesc->u02Data_len_position);
            HAL_I3C_TRACE("First phase mode     :0x%X", pstCmdDesc->u01Fist_phase_mode);
            HAL_I3C_TRACE("16B_SubOffset        :0x%X", pstCmdDesc->u01Sub_offset);
            HAL_I3C_TRACE("Mode and speed       :0x%X", pstCmdDesc->u03Speed_mode);
            HAL_I3C_TRACE("RnW                  :%s",   pstCmdDesc->u01Dir_rnw == 0 ? "write" : "read");
            HAL_I3C_TRACE("ROC                  :0x%X", pstCmdDesc->u01ROC);
            HAL_I3C_TRACE("TOC                  :0x%X", pstCmdDesc->u01TOC);
            HAL_I3C_TRACE("Offset/suboffset     :0x%X", pstCmdDesc->u16Offset_suboffset);
            HAL_I3C_TRACE("Data length          :0x%X", pstCmdDesc->u16Data_len);
            break;
        }
        case CMD_ATRB_INTERNAL_CTRL: {
            struct HAL_I3C_CMDDESC_ICC *pstCmdDesc;
            pstCmdDesc = (struct HAL_I3C_CMDDESC_ICC *)desc->pstCmdDesc;
            HAL_I3C_TRACE("Vendor info present  :0x%X", pstCmdDesc->u01Vendor_info_present);
            HAL_I3C_TRACE("MIPI cmd             :0x%X", pstCmdDesc->u04Mipi_cmd);
            HAL_I3C_TRACE("Vendor specific      :0x%X", pstCmdDesc->u32Vendor_specific);
            break;
        }
        default: {
            HAL_I3C_ERROR("ERR, Invalid Transfer Mode, line:%d", __LINE__);
            break;
        }
    }
    HAL_I3C_TRACE("----------------------------------");
}

/*********************************************************************
Return Value : 0 - Success
               Others - FAILURE
Description  : This API initializes the I3C Master Controller.
*********************************************************************/
static int32_t hal_i3c_print_base_offset(enum HAL_I3C_ID_T id)
{
    HAL_I3C_TRACE("DAT SECTION OFFSET       :0x%X", I3C_HCI_DAT_OFFSET);
    HAL_I3C_TRACE("DCT SECTION OFFSET       :0x%X", I3C_HCI_DCT_OFFSET);
    HAL_I3C_TRACE("PIO_BASE                 :0x%X", PIO_BASE);
    HAL_I3C_TRACE("RH_BASE                  :0x%X", RH_BASE);

    HAL_I3C_TRACE("EXT_CAPS_HEADER_BASE     :0x%X", EXT_CAPS_HEADER_BASE);
    HAL_I3C_TRACE("EXT_CAPS_HEADER_OFFSET   :0x%X", EXT_CAPS_HEADER_OFFSET);

    HAL_I3C_TRACE("I3C_HCI_RHS_CTRL         :0x%X", I3C_HCI_RHS_CTRL);
    HAL_I3C_TRACE("I3C_HCI_RH0_OFFSET       :0x%X", I3C_HCI_RH0_OFFSET);
    HAL_I3C_TRACE("I3C_HCI_RH1_OFFSET       :0x%X", I3C_HCI_RH1_OFFSET);
    HAL_I3C_TRACE("I3C_HCI_RH2_OFFSET       :0x%X", I3C_HCI_RH2_OFFSET);
    HAL_I3C_TRACE("I3C_HCI_RH3_OFFSET       :0x%X", I3C_HCI_RH3_OFFSET);
    HAL_I3C_TRACE("I3C_HCI_RH4_OFFSET       :0x%X", I3C_HCI_RH4_OFFSET);
    HAL_I3C_TRACE("I3C_HCI_RH5_OFFSET       :0x%X", I3C_HCI_RH5_OFFSET);
    HAL_I3C_TRACE("I3C_HCI_RH6_OFFSET       :0x%X", I3C_HCI_RH6_OFFSET);
    HAL_I3C_TRACE("I3C_HCI_RH7_OFFSET       :0x%X", I3C_HCI_RH7_OFFSET);

    HAL_I3C_TRACE("RH0_BASE                 :0x%X", RH0_BASE);
    HAL_I3C_TRACE("RH0_CR_SETUP             :0x%X", RH0_CR_SETUP);
    HAL_I3C_TRACE("RH0_IBI_SETUP            :0x%X", RH0_IBI_SETUP);
    HAL_I3C_TRACE("RH0_RING_INTR_ST         :0x%X", RH0_RING_INTR_ST);
    HAL_I3C_TRACE("RH0_RING_INTR_ST_EN      :0x%X", RH0_RING_INTR_ST_EN);
    HAL_I3C_TRACE("RH0_RING_INTR_SIG_EN     :0x%X", RH0_RING_INTR_SIG_EN);
    HAL_I3C_TRACE("RH0_RING_INTR_FORCE      :0x%X", RH0_RING_INTR_FORCE);
    HAL_I3C_TRACE("RH0_RING_STATUS          :0x%X", RH0_RING_STATUS);
    HAL_I3C_TRACE("RH0_RING_CTRL            :0x%X", RH0_RING_CTRL);
    HAL_I3C_TRACE("RH0_RING_OPR1            :0x%X", RH0_RING_OPR1);
    HAL_I3C_TRACE("RH0_RING_OPR2            :0x%X", RH0_RING_OPR2);
    HAL_I3C_TRACE("RH0_CMD_BASE_H           :0x%X", RH0_CMD_BASE_H);
    HAL_I3C_TRACE("RH0_CMD_BASE_L           :0x%X", RH0_CMD_BASE_L);
    HAL_I3C_TRACE("RH0_RSP_BASE_H           :0x%X", RH0_RSP_BASE_H);
    HAL_I3C_TRACE("RH0_RSP_BASE_L           :0x%X", RH0_RSP_BASE_L);
    HAL_I3C_TRACE("RH0_IBI_ST_BASE_H        :0x%X", RH0_IBI_ST_BASE_H);
    HAL_I3C_TRACE("RH0_IBI_ST_BASE_L        :0x%X", RH0_IBI_ST_BASE_L);
    HAL_I3C_TRACE("RH0_IBI_DAT_BASE_H       :0x%X", RH0_IBI_DAT_BASE_H);
    HAL_I3C_TRACE("RH0_IBI_DAT_BASE_L       :0x%X", RH0_IBI_DAT_BASE_L);

    HAL_I3C_TRACE("RH1_BASE                 :0x%X", RH1_BASE);
    HAL_I3C_TRACE("RH2_BASE                 :0x%X", RH2_BASE);
    HAL_I3C_TRACE("RH3_BASE                 :0x%X", RH3_BASE);

    HAL_I3C_TRACE("PIO_BASE                 :0x%X", PIO_BASE);
    HAL_I3C_TRACE("I3C_HCI_PIO_CMD_QUE_PORT :0x%X", I3C_HCI_PIO_CMD_QUE_PORT);
    HAL_I3C_TRACE("I3C_HCI_PIO_RSP_QUE_PORT :0x%X", I3C_HCI_PIO_RSP_QUE_PORT);
    HAL_I3C_TRACE("I3C_HCI_PIO_DATA_PORT    :0x%X", I3C_HCI_PIO_DATA_PORT);
    HAL_I3C_TRACE("I3C_HCI_PIO_IBI_PORT     :0x%X", I3C_HCI_PIO_IBI_PORT);
    HAL_I3C_TRACE("I3C_HCI_PIO_QUE_THLD_CTRL:0x%X", I3C_HCI_PIO_QUE_THLD_CTRL);
    HAL_I3C_TRACE("I3C_HCI_PIO_DAT_THLD_CTRL:0x%X", I3C_HCI_PIO_DAT_THLD_CTRL);
    HAL_I3C_TRACE("I3C_HCI_PIO_QUE_SIZE     :0x%X", I3C_HCI_PIO_QUE_SIZE);

    return I3C_ERR_NONE;
}

/*********************************************************************
Return Value : Dynamic Address
Description  : This function returns a valid DA
*********************************************************************/
static int8_t hal_i3c_get_dynamic_addr(enum HAL_I3C_ID_T id)
{
    uint8_t parity_bit[8], parity_check;
    static uint8_t u8DA = 0x7;

    while (1) {
        u8DA++;
        if ((u8DA >= 0x08) && (u8DA != 0x3E) && (u8DA != 0x5E) && \
            (u8DA != 0x6E) && (u8DA != 0x76) && (u8DA <= 0x77)) {
            parity_bit[0] = (u8DA >> 0) & 1;
            parity_bit[1] = (u8DA >> 1) & 1;
            parity_bit[2] = (u8DA >> 2) & 1;
            parity_bit[3] = (u8DA >> 3) & 1;
            parity_bit[4] = (u8DA >> 4) & 1;
            parity_bit[5] = (u8DA >> 5) & 1;
            parity_bit[6] = (u8DA >> 6) & 1;
            parity_bit[7] = (u8DA >> 7) & 1;

            parity_check = ((((((parity_bit[7] ^ parity_bit[6]) ^ parity_bit[5]) ^\
                               parity_bit[4]) ^ parity_bit[3]) ^ parity_bit[2]) ^ parity_bit[1]);

            if (((!parity_check) == parity_bit[0]) && (parity_bit[0] == 1)) {
                HAL_I3C_TRACE("dynamic addr:0x%X", u8DA);
                return u8DA;
            }
        }
        if (u8DA > 0x7F) {
            HAL_I3C_ERROR("ERR, %s, dynamic addr 0x%X exceeds 0x7F", __func__, u8DA);
            return I3C_ERR_DAA_EXCEED;
        }
    }
}

/*********************************************************************
Argument     : None
Return Value : Valid TID/TID Exceed Error code
Description  : This function returns a valid TID
*********************************************************************/
static int8_t hal_i3c_get_tid(void)
{
    static int8_t s8TID = -1;
    while (1) {
        s8TID++;
        if (s8TID >= 0x0 && s8TID <= 0xF) {
            HAL_I3C_TRACE("s8TID:0x%X", s8TID);
            return s8TID;
        } else {
            s8TID = 0;
            HAL_I3C_TRACE("s8TID:0x%X", s8TID);
            return s8TID;
        }
    }
}

/*********************************************************************
Argument     : pstRingCap ?RingCap structure pointer
               u8MaxRings ?No of rings supported in HW
Return Value : 0 - Success
               Others - FAILURE
Description  : This function collects the capability of all the rings supported by HW
*********************************************************************/
static int8_t hal_i3c_get_rings_cap(enum HAL_I3C_ID_T id, struct HAL_I3C_RING_CAP *pstRingCap, uint8_t u8MaxRings)
{
    uint32_t i, u32RdVal;
    struct HAL_I3C_RING_CAP ringCap;
    struct HAL_I3C_RING_CAP *pRingCap = pstRingCap;

    HAL_I3C_TRACE("  ");
    HAL_I3C_TRACE("%s", __func__);
    for (i = 0; i < u8MaxRings; i++) {
        i3c_reg_write32(id, RHx_REG(i, CR_SETUP), u8MaxRings);
        i3c_reg_read32(id, RHx_REG(i, CR_SETUP), &u32RdVal);

        ringCap.u8RingSize      = u32RdVal & 0xFF;
        ringCap.u8RespStrSize   = (u32RdVal & RSP_STRUCT_SIZE_MASK) >> 16;
        ringCap.u8XferStrSize   = (u32RdVal & XFER_STRUCT_SIZE_MASK) >> 24;

        memcpy(pRingCap, &ringCap, sizeof(struct HAL_I3C_RING_CAP));

        HAL_I3C_TRACE("RING_ID[%d] pRingCap addr :0x%X", i, (uint32_t)pRingCap);
        HAL_I3C_TRACE("RING_ID[%d] u8RingSize    :0x%X", i, pRingCap->u8RingSize);
        HAL_I3C_TRACE("RING_ID[%d] u8RespStrSize :0x%X", i, pRingCap->u8RespStrSize);
        HAL_I3C_TRACE("RING_ID[%d] u8XferStrSize :0x%X", i, pRingCap->u8XferStrSize);
        HAL_I3C_TRACE("  ");

        pRingCap++;
    }
    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : pstTxDesc ?pointer to CmdDataDesc structure
               bus_info ?pointer to I3CBusCtrlInfo structure
Return Value : 0 - Success
               Others - FAILURE
Description  : This function frames the transfer descriptor.
*********************************************************************/
static int32_t hal_i3c_frame_pio_cmd_queue(enum HAL_I3C_ID_T id, struct HAL_I3C_CMD_DATA_DESC *pstTxDesc,
        struct HAL_I3C_BUS_CTRL_INFO *bus_info)
{
    uint16_t u16DataLen;
    uint32_t u32CmdFmtHi, u32CmdFmtLow;
    uint32_t u32RdValue;
    int32_t s32Status;

    memcpy((uint8_t *)&u32CmdFmtLow, (uint8_t *)pstTxDesc->pstCmdDesc, sizeof(u32CmdFmtLow));
    memcpy((uint8_t *)&u32CmdFmtHi, (uint8_t *)((uint32_t)pstTxDesc->pstCmdDesc + sizeof(u32CmdFmtLow)), sizeof(u32CmdFmtHi));
    HAL_I3C_TRACE("CmdFmtLow:0x%X", u32CmdFmtLow);
    HAL_I3C_TRACE("CmdFmtHi :0x%X", u32CmdFmtHi);

    //for Immediate Data Transfer
    if (pstTxDesc->pstCmdDesc->u03CmdAttr == CMD_ATRB_DAA) {
        bus_info->cmdque_rdy_compln = 0;

        i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32RdValue);
        i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, INTR_CMD_QUE_RDY_STS | u32RdValue);

        s32Status = wait_for_completion_timeout(id, &bus_info->cmdque_rdy_compln, 1, CMD_QUE_RDY_TIMEOUT, 0);
        if (bus_info->error_flag) {
            bus_info->error_flag = 0;
            HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
            return I3C_ERR_TRANSFER;
        }

        if (s32Status == 0) {
            HAL_I3C_TRACE("COMMAND QUEUE READY STATUS INTERRUPT BIT SET, line:%d", __LINE__);
            //Free CMD Queue
            i3c_reg_write32(id, I3C_HCI_PIO_CMD_QUE_PORT, u32CmdFmtLow);
            i3c_reg_write32(id, I3C_HCI_PIO_CMD_QUE_PORT, u32CmdFmtHi);
        } else {
            HAL_I3C_ERROR("ERR %d, line:%d", s32Status, __LINE__);
            return s32Status;
        }
        return I3C_ERR_NONE;
    }
    u16DataLen = pstTxDesc->pstCmdDesc->u16Data_len;

    //EnQ the request
    bus_info->PIOTx.u32CmdFrameLo = u32CmdFmtLow;
    bus_info->PIOTx.u32CmdFrameHi = u32CmdFmtHi;
    bus_info->PIOTx.u8DataDir     = pstTxDesc->pstCmdDesc->u01Dir_rnw;
    bus_info->PIOTx.u32NoOfEntries = bus_info->pstPIOTx->u32NoOfEntries;
    bus_info->PIOTx.u16DataLenth  = u16DataLen;
    bus_info->PIOTx.pstData       = NULL;
    bus_info->PIOTx.u08CmdAttr    = pstTxDesc->pstCmdDesc->u03CmdAttr;
    HAL_I3C_TRACE("u16DataLen:0x%X", u16DataLen);
    if (pstTxDesc->pu8Buf != NULL) {
        bus_info->PIOTx.pstData = pstTxDesc->pu8Buf;

#ifdef I3C_DEBUG
        uint16_t u16maxprint;
        u16maxprint = u16DataLen > 32 ? 32 : u16DataLen;
        for (int i = 0; i < u16maxprint; i++) {
            HAL_I3C_TRACE("PIOTx.pstData[%d]:0x%X", i, bus_info->PIOTx.pstData[i]);
        }
#endif
    }

#ifdef I3C_DEBUG
    if (bus_info->pstPIOTx->pstData != NULL) {
        for (int i = 0; i < 8; i++)
            HAL_I3C_TRACE("bus_info->pstPIOTx->pstData[%d]:0x%X", i, bus_info->pstPIOTx->pstData[i]);
    }
#endif

    bus_info->pstPIOTx->u32CmdFrameLo = u32CmdFmtLow;
    bus_info->pstPIOTx->u32CmdFrameHi = u32CmdFmtHi;
    s32Status = hal_i3c_pio_cmd_tx(id, &bus_info->PIOTx);
    if (s32Status < 0) {
        return s32Status;
    }

    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : uint32_t NoOfEntries
               uint8_t u8RingID- Ring ID
Return       : 0 - Success
               Others - Error
Description  : This function is used to door bell the controller to let it know that transfer
               descriptors are ready to be processed
*********************************************************************/
static int32_t hal_i3c_ring_enqueue_ptr(enum HAL_I3C_ID_T id, uint32_t NoOfEntries, uint8_t u8RingID)
{
    uint32_t u32RdValue;

    HAL_I3C_TRACE("%s, NoOfEntries:%d, u8RingID:%d", __func__, NoOfEntries, u8RingID);

    i3c_reg_write32(id, RHx_REG(u8RingID, RING_INTR_ST_EN), I3C_DMA_INTR_STS_EN);
    i3c_reg_write32(id, RHx_REG(u8RingID, RING_INTR_SIG_EN), I3C_DMA_INTR_STS_EN);

    //Write CMD
    //Chk if the queue is empty
    //i3c_reg_read32(id, RH0_RING_INTR_ST, &u32RdValue);
    //if((u32RdValue & RH_INTR_TX_DONE_STS)) {//Free CMD Queue, that means its fine to enQ controller abt the cmd Q.
    //Read RH_STATUS to know if Ring is busy or not before updating EnQ ptr
    i3c_reg_read32(id, RHx_REG(u8RingID, RING_STATUS), &u32RdValue);
    if (!(u32RdValue & RH_RUNNING_MASK)) { // Free Ring
        i3c_reg_write32(id, RHx_REG(u8RingID, RING_OPR1), NoOfEntries & CR_ENQ_PTR_MASK);
    }
    //}
    i3c_reg_read32(id, RHx_REG(u8RingID, RING_OPR1), &u32RdValue);
    //Set CR_SW_DQ_PTR = 1
    i3c_reg_write32(id, RHx_REG(u8RingID, RING_OPR1), ((NoOfEntries << 8) & CR_SW_DQ_PTR_MASK) | u32RdValue);
    //Enable current Ring and set ring start request
    i3c_reg_write32(id, RHx_REG(u8RingID, RING_CTRL), RH_ENABLE_REQ | RH_RS_REQ);

    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : pstTxDesc ?pointer to CmdDataDesc structure
               EntryNum ?Index number of the transfer descriptor.
               bus_info ?pointer to I3CBusCtrlInfo structure
Return Value : 0 - Success
               Others - FAILURE
Description  : This function frames the transfer descriptor for the DMA mode.
*********************************************************************/
static int32_t hal_i3c_frame_dma_cmd_desc(enum HAL_I3C_ID_T id, struct HAL_I3C_CMD_DATA_DESC *pstTxDesc,
        uint8_t u8RingID, uint32_t EntryNum, struct HAL_I3C_BUS_CTRL_INFO *bus_info)
{
    uint32_t u32TxDescIndex;
    struct HAL_I3C_RING_TX_DESC DMATx;

    DMATx.u32CMDDescAddrLow = *(uint32_t *)pstTxDesc->pstCmdDesc;
    DMATx.u32CMDDescAddrHi = *(uint32_t *)((uint32_t)pstTxDesc->pstCmdDesc + 4);
    if (pstTxDesc->pstCmdDesc->u03CmdAttr == CMD_ATRB_DAA) {
        DMATx.u32DataAddrLow = 0;
        DMATx.u32DataAddrHi = 0;
    } else {
        DMATx.u32DataAddrLow = ADDR_CPU_TO_DEV((uint32_t)(pstTxDesc->pu8Buf));
        DMATx.u32DataAddrHi = 0;
    }
    DMATx.u8BLP = 0;
    DMATx.u8IOC = 1;
    DMATx.u16DataLength = pstTxDesc->pstCmdDesc->u16Data_len;

    u32TxDescIndex = EntryNum * bus_info->pstRingCap[u8RingID]->u8XferStrSize;

    HAL_I3C_TRACE("%s, u32TxDescIndex:0x%X", __func__, u32TxDescIndex);
    HAL_I3C_TRACE("pstRingCap[%d]->u8XferStrSize        :%d", u8RingID, bus_info->pstRingCap[u8RingID]->u8XferStrSize);
    HAL_I3C_TRACE("pstRingDesc[%d] Address              :0x%X", u8RingID, (uint32_t)bus_info->pstRingDesc[u8RingID]);
    HAL_I3C_TRACE("pstRingDesc[%d]->pstTxDesc Address   :0x%X", u8RingID, (uint32_t)bus_info->pstRingDesc[u8RingID]->pstTxDesc);

    memcpy((void *)((uint32_t)(bus_info->pstRingDesc[u8RingID]->pstTxDesc) + u32TxDescIndex), (void *)&DMATx, sizeof(struct HAL_I3C_RING_TX_DESC));

    HAL_I3C_TRACE("memcopy desc Address = 0x%x", ((uint32_t)(bus_info->pstRingDesc[u8RingID]->pstTxDesc) + u32TxDescIndex));
    HAL_I3C_TRACE("memcopy desc Context = 0x%x", *(uint32_t *)((uint32_t)(bus_info->pstRingDesc[u8RingID]->pstTxDesc) + u32TxDescIndex));

    hal_i3c_ring_enqueue_ptr(id, EntryNum + 1, u8RingID);

    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : TxDescAddr ?Tx Descriptor Address
               RspDescAddr ?Response Descriptor Address
               u8RingID ?Ring ID
Return       : 0 - Success
               Others - Error
Description  : This function is used to program the set of Tx Desc base addresses
               for all rings in DMA
*********************************************************************/
static int32_t hal_i3c_program_ring_base_addr(enum HAL_I3C_ID_T id, uint64_t TxDescAddr,
        uint64_t RspDescAddr, uint8_t u8RingID)
{
    if(u8RingID < RING_ID_0 || RING_ID_0 > RING_ID_7){
        HAL_I3C_ERROR("ERR, %s, ringid:%d", __func__, u8RingID);
        return I3C_ERR_INVALID_RING_ID;
    }

    i3c_reg_write32(id, RHx_REG(u8RingID, CMD_BASE_L), TxDescAddr);
    i3c_reg_write32(id, RHx_REG(u8RingID, CMD_BASE_H), TxDescAddr >> 32);
    i3c_reg_write32(id, RHx_REG(u8RingID, RSP_BASE_L), RspDescAddr);
    i3c_reg_write32(id, RHx_REG(u8RingID, RSP_BASE_H), RspDescAddr >> 32);

    return I3C_ERR_NONE;
}

/*********************************************************************
Return      : None
Description : This API is used to handle PIO Data Write as a seperate
              Worker thread in a Work Queue
*********************************************************************/
static void hal_i3c_pio_write(enum HAL_I3C_ID_T id)
{
    uint8_t u8TxThld;
    uint16_t i = 0, j, u16TxIntrInterval = 1, roundoff;
    uint32_t u32Data, u32RdValue, u32Thld = 0, u32noofdwords = 0, u32Retrycnt = 0;
    int32_t s32Status;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    bus_info->tx_thld_compln = 0;
    u32noofdwords = bus_info->PIOTx.u16DataLenth / (sizeof(u32RdValue));
    if (bus_info->PIOTx.u16DataLenth % (sizeof(u32RdValue)))
        u32noofdwords++;

    i3c_reg_read32(id, I3C_HCI_PIO_DAT_THLD_CTRL, &u32Thld);
    u8TxThld = u32Thld & 0x7;

    u16TxIntrInterval = 2 << u8TxThld;
    roundoff = u32noofdwords / u16TxIntrInterval;
    if (u32noofdwords % u16TxIntrInterval)
        roundoff++;

    u32noofdwords = roundoff * u16TxIntrInterval;

    while (u32noofdwords > 0) {
        i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32RdValue);
        i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, INTR_TX_THLD_STS | u32RdValue);

        s32Status = wait_for_completion_timeout(id, &bus_info->tx_thld_compln, 1, TX_THLD_TIMEOUT, 0);
        if (bus_info->error_flag) {
            bus_info->error_flag = 0;
            HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
            break;
        }
        if (s32Status == 0) {
            for (j = 0; j < u16TxIntrInterval; j++) {
                u32Data = bus_info->PIOTx.pstData[0 + i] | \
                          bus_info->PIOTx.pstData[1 + i] << 8 | \
                          bus_info->PIOTx.pstData[2 + i] << 16 | \
                          bus_info->PIOTx.pstData[3 + i] << 24;
                i3c_reg_write32(id, I3C_HCI_PIO_DATA_PORT, u32Data);
                i = i + 4;
                u32noofdwords--;
            }
        } else {
            HAL_I3C_TRACE("Tx Threshold Status Interrupt Not Set");
            u32Retrycnt++;
            if (u32Retrycnt >= 5) {
                HAL_I3C_TRACE("Retried 5 times");
                break;
            }
        }
    }
}

/*******************************************************************************
Return      : None
Description : This API is used to handle PIO Data Read as a seperate
              Worker thread in a Work Queue
********************************************************************************/
static void hal_i3c_pio_read(enum HAL_I3C_ID_T id)
{
    uint8_t u8RxThld, j;
    uint16_t u16CmdLen = 0, u16RxIntrInterval = 1, roundoff;
    uint32_t u32RdValue = 0, u32Retrycnt = 0, u32Thld;
    int32_t s32Status;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    u16CmdLen = bus_info->glpstCmdData->pstCmdDesc->u16Data_len;

    bus_info->rx_thld_compln = 0;
    bus_info->words_cnt = u16CmdLen / (sizeof(u32RdValue));
    if (u16CmdLen % (sizeof(u32RdValue)))
        bus_info->words_cnt++;

    i3c_reg_read32(id, I3C_HCI_PIO_DAT_THLD_CTRL, &u32Thld);
    u8RxThld = (u32Thld & 0x700) >> 8;
    u16RxIntrInterval = 2 << u8RxThld;
    roundoff = bus_info->words_cnt / u16RxIntrInterval;
    if (bus_info->words_cnt % u16RxIntrInterval) {
        roundoff++;
    }

    bus_info->words_cnt = roundoff * u16RxIntrInterval;
    bus_info->read_index = 0;
    while (bus_info->words_cnt > 0) {
        if (bus_info->resp_rdy) {
            break;
        } else {
            i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32RdValue);
            i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, INTR_RX_THLD_STS | u32RdValue);

            s32Status = wait_for_completion_timeout(id, &bus_info->rx_thld_compln, 1, RX_THLD_TIMEOUT, 0);
            if (bus_info->error_flag) {
                bus_info->error_flag = 0;
                HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
                break;
            }
            if (s32Status == 0) {
                for (j = 0; j < u16RxIntrInterval; j++) {
                    u32RdValue = 0;
                    i3c_reg_read32(id, I3C_HCI_PIO_DATA_PORT, &u32RdValue);
                    if (u32RdValue == 0xffffffff) {
                        HAL_I3C_TRACE("Not able to Read DATA Port");
                        break;
                    } else {
                        memcpy(bus_info->glpstCmdData->pu8Buf + (bus_info->read_index * sizeof(u32RdValue)), &u32RdValue, sizeof(u32RdValue));
                        bus_info->words_cnt--;
                        bus_info->read_index++;
                    }
                }
            } else {
                HAL_I3C_TRACE("Rx Threshold Status Interrupt Not Set");
                u32Retrycnt++;
                if (u32Retrycnt >= 5) {
                    HAL_I3C_TRACE("Retried 5 times");
                    break;
                }
            }
        }
    }
}

/*********************************************************************
Argument     : struct HAL_I3C_PIO_XFER *pstPIOTx
Return       : 0 - Success
               Others - Error
Description  : This function is used to door bell the controller to let it know that transfer
               queue is ready to be processed in PIO method
*********************************************************************/
static int32_t hal_i3c_pio_cmd_tx(enum HAL_I3C_ID_T id, struct HAL_I3C_PIO_XFER *pstPIOTx)
{
    int32_t s32Status = -1;
    uint32_t u32RdValue, i, u32Data = 0;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    if (pstPIOTx->pstData != NULL) {
        if (pstPIOTx->u8DataDir == WR_XFER) {//Write
            HAL_I3C_TRACE("PIO REGULAR WRITE");
            if (pstPIOTx->u16DataLenth < 32) {
                for (i = 0; i < pstPIOTx->u16DataLenth; i += 4) {
                    u32Data = pstPIOTx->pstData[0 + i] | \
                              pstPIOTx->pstData[1 + i] << 8 | \
                              pstPIOTx->pstData[2 + i] << 16 | \
                              pstPIOTx->pstData[3 + i] << 24;
                    i3c_reg_write32(id, I3C_HCI_PIO_DATA_PORT, u32Data);
                }
            } else {
                hal_i3c_pio_write(id);
            }
        } else {
            HAL_I3C_TRACE("PIO COMMAND READ");
        }
    }
    bus_info->cmdque_rdy_compln = 0;

    i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32RdValue);
    i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, INTR_CMD_QUE_RDY_STS | u32RdValue);

    s32Status = wait_for_completion_timeout(id, &bus_info->cmdque_rdy_compln, 1, CMD_QUE_RDY_TIMEOUT, 0);
    if (bus_info->error_flag) {
        bus_info->error_flag = 0;
        HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
        return I3C_ERR_TRANSFER;
    }
    if (s32Status == 0) {
        HAL_I3C_TRACE("COMMAND QUEUE READY STATUS INTERRUPT BIT SET, line:%d", __LINE__);
        //Free CMD Queue
        i3c_reg_write32(id, I3C_HCI_PIO_CMD_QUE_PORT, pstPIOTx->u32CmdFrameLo);
        i3c_reg_write32(id, I3C_HCI_PIO_CMD_QUE_PORT, pstPIOTx->u32CmdFrameHi);
    } else {
        HAL_I3C_ERROR("ERR %d, line:%d", s32Status, __LINE__);
        return s32Status;
    }

    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : struct HAL_I3C_SLAVE_DAT *ptrstSlvDAT, uint32_t index
Return Value : None
Description  : This function writes the Device Address table
               based on device index
*********************************************************************/
static void hal_i3c_write_dev_addr_table(enum HAL_I3C_ID_T id, struct HAL_I3C_SLAVE_DAT *pstSlvDAT, uint32_t index)
{
    uint32_t dat_low32, dat_high32;
    uint8_t u8IncOffset = 0;

    u8IncOffset = u8IncOffset + (0x8 * index);

    memcpy((uint8_t *)&dat_low32, (uint8_t *)pstSlvDAT, sizeof(dat_low32));
    memcpy((uint8_t *)&dat_high32, (uint8_t *)((uint32_t)pstSlvDAT + sizeof(dat_low32)), sizeof(dat_high32));
    HAL_I3C_TRACE("DAT Value LO[0x%X]:0x%X", I3C_HCI_DAT_OFFSET, dat_low32);
    HAL_I3C_TRACE("DAT Value HI[0x%X]:0x%X", I3C_HCI_DAT_OFFSET + NEXT_4B_OFFSET, dat_high32);

    i3c_reg_write32(id, I3C_HCI_DAT_OFFSET + u8IncOffset, dat_low32);
    i3c_reg_write32(id, I3C_HCI_DAT_OFFSET + u8IncOffset + NEXT_4B_OFFSET, dat_high32);
}

/*********************************************************************
Argument     : struct HAL_I3C_BUS_CTRL_INFO *pstbusInfo
Return Value : 0 - Success
               Others - FAILURE
Description  : This function populates the Device Address table
**********************************************************************/
static int32_t hal_i3c_populate_dev_addr_table(enum HAL_I3C_ID_T id, struct HAL_I3C_BUS_CTRL_INFO *pstbusInfo)
{
    uint8_t i;
    uint32_t u32RdVal;
    uint16_t u16DATSize, u16DCTSize;
    uint8_t u8DCTIndex;
    struct HAL_I3C_SLAVE_DCT  *pstSlvDCT;
    struct HAL_I3C_SLAVE_BCR *BCR;
    struct HAL_I3C_SLAVE_DAT *pstSlvDAT;
    struct HAL_I3C_DATREG datreg;
    struct HAL_I3C_DATREG *pstDATReg;
    struct HAL_I3C_HOST_T *host = &i3c_host[id];

    pstDATReg = &datreg;
    memset(&datreg, 0, sizeof(struct HAL_I3C_DATREG));

    //Check size of DAT table
    i3c_reg_read32(id, I3C_HCI_DAT_SEC_OFFSET, &u32RdVal);
    memcpy(pstDATReg, &u32RdVal, sizeof(u32RdVal));

    //Single device has 2 dwords of DAT
    HAL_I3C_TRACE("DAT TableOffset:0x%X", pstDATReg->u12TableOffset);
    HAL_I3C_TRACE("DAT TableSize  :0x%X", pstDATReg->u07TableSize);
    u16DATSize = (pstDATReg->u07TableSize);
    pstbusInfo->u16DATSize = u16DATSize;

    if (u16DATSize > MAX_DEVICES) {
        HAL_I3C_ERROR("ERR, No of devices in the bus exceeds max count");
        return I3C_ERR_DEVICES_EXCEED;
    }

    pstSlvDAT = (struct HAL_I3C_SLAVE_DAT *)i3c_slv_dat[id];
    memset(pstSlvDAT, 0, sizeof(struct HAL_I3C_SLAVE_DAT) * u16DATSize);
    pstbusInfo->pstSlvDAT = pstSlvDAT;

    //Check size of DCT table
    i3c_reg_read32(id, I3C_HCI_DCT_SEC_OFFSET, &u32RdVal);
    u16DCTSize = (u32RdVal & DCT_TBL_SIZE_MASK) >> 12;//18:12
    u8DCTIndex = (u32RdVal & DCT_TBL_INDEX_MASK) >> 19;//23:19

    pstbusInfo->u16DCTSize = u16DCTSize;
    pstbusInfo->u8DCTIndex = u8DCTIndex;

    if (pstbusInfo->u16DCTSize > MAX_DEVICES) {
        HAL_I3C_ERROR("ERR, No of devices in the bus exceeds max count");
        return I3C_ERR_DEVICES_EXCEED;
    }

    //Single device has 4 dwords of DCT
    HAL_I3C_TRACE("DCT Table Size :0x%X", pstbusInfo->u16DCTSize);
    HAL_I3C_TRACE("DCT Table Index:0x%X", pstbusInfo->u8DCTIndex);

    pstSlvDCT = (struct HAL_I3C_SLAVE_DCT *)i3c_slv_dct[id];
    memset(pstSlvDCT, 0, (sizeof(struct HAL_I3C_SLAVE_DCT) * u16DCTSize));
    pstbusInfo->pstSlvDCT = pstSlvDCT;

    //Allocate memory for BCR/
    BCR = (struct HAL_I3C_SLAVE_BCR *)i3c_slv_bcr[id];
    memset(i3c_slv_bcr[id], 0, sizeof(i3c_slv_bcr[id]));
    pstbusInfo->BCR = BCR;

    //Read DATs
    for (i = 0; i < host->dev_cnt; i++) {
        pstSlvDAT->u02DevNackRetryCnt = NACK_RETRY_CNT;
        if (host->static_addr != NULL) {
            pstSlvDAT->u07StaticAddr = host->static_addr[i];
        } else {
            pstSlvDAT->u07StaticAddr = i;
        }
        pstSlvDAT->u08DynAddr = hal_i3c_get_dynamic_addr(id);

        if (host->bus_ctrl_info.auto_cmd_en) {
            pstSlvDAT->u08AutoCmdValue = 0x62;
            pstSlvDAT->u08AutoCmdMask = 0xff;
        } else {
            pstSlvDAT->u08AutoCmdValue = 0xff;
            pstSlvDAT->u08AutoCmdMask = 0x0;
        }

        HAL_I3C_TRACE("pstSlvDAT->u07StaticAddr     :0x%X", pstSlvDAT->u07StaticAddr);
        HAL_I3C_TRACE("pstSlvDAT->u08DynAddr        :0x%X", pstSlvDAT->u08DynAddr);
        HAL_I3C_TRACE("pstSlvDAT->u03RingID         :0x%X", pstSlvDAT->u03RingID);
        HAL_I3C_TRACE("pstSlvDAT->u02DevNackRetryCnt:0x%X", pstSlvDAT->u02DevNackRetryCnt);
        HAL_I3C_TRACE("pstSlvDAT->u01Device         :0x%X", pstSlvDAT->u01Device);
        HAL_I3C_TRACE("pstSlvDAT->u08AutoCmdMask    :0x%X", pstSlvDAT->u08AutoCmdMask);
        HAL_I3C_TRACE("pstSlvDAT->u08AutoCmdValue   :0x%X", pstSlvDAT->u08AutoCmdValue);

        pstSlvDAT++;
    }

    pstSlvDAT = pstbusInfo->pstSlvDAT;//Pointer back to index 0

    //Populate/Write DAT for all devices with DAA
    for (i = 0; i < host->dev_cnt; i++) {
        HAL_I3C_TRACE("dynamic Address:0x%X", pstSlvDAT->u08DynAddr);
        hal_i3c_write_dev_addr_table(id, pstSlvDAT, i);
        pstSlvDAT++;
    }
    pstSlvDAT = pstbusInfo->pstSlvDAT;//Pointer back to index 0

    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : struct HAL_I3C_BUS_CTRL_INFO *pstbusInfo
Return Value : 0 - Success
               Others - FAILURE
Description  : This function populates the DMA Rings
*********************************************************************/
static int32_t hal_i3c_populate_dma_rings(enum HAL_I3C_ID_T id, struct HAL_I3C_BUS_CTRL_INFO *pstbusInfo)
{
    uint16_t i;
    uint32_t u32RdVal;
    //uint32_t u32TxDescSize, u32RspDescSize;
    uint64_t u64TxDescAddr, u64RspDescAddr;
    struct HAL_I3C_PREAMBLE Preamble;// TODO: Change this to a global variable when needed
    struct HAL_I3C_PREAMBLE *pstPreamble;
    struct HAL_I3C_DMA_RING_XFER *pstRingDesc;
    struct HAL_I3C_RING_CAP *pstRingCap;

    HAL_I3C_TRACE("  ");
    HAL_I3C_TRACE("%s", __func__);
    pstPreamble = &Preamble;
    memset(&Preamble, 0, sizeof(struct HAL_I3C_PREAMBLE));
    pstbusInfo->pstPreamble = pstPreamble;

    //DMA Ring formation
    i3c_reg_read32(id, I3C_HCI_RHS_CTRL, &u32RdVal);
    pstPreamble->u8PreambleSize = (u32RdVal & RING_PREAMBLE_MASK) >> 24;
    pstPreamble->u8HeaderSize = (u32RdVal & RING_SIZE_MASK) >> 16;
    pstPreamble->u4Max_header_cnt_cap = (u32RdVal & RING_SUPPORT_MASK) >> 4;
    pstPreamble->u4Max_header_cnt = MAX_RINGS;
    pstbusInfo->u8HWMaxRings = pstPreamble->u4Max_header_cnt;
    HAL_I3C_TRACE("u4Max_header_cnt     :0x%X", pstPreamble->u4Max_header_cnt);
    HAL_I3C_TRACE("u4Max_header_cnt_cap :0x%X", pstPreamble->u4Max_header_cnt_cap);
    HAL_I3C_TRACE("u8HeaderSize         :0x%X", pstPreamble->u8HeaderSize);
    HAL_I3C_TRACE("u8PreambleSize       :0x%X", pstPreamble->u8PreambleSize);
    HAL_I3C_TRACE("u8HWMaxRings         :0x%X", pstbusInfo->u8HWMaxRings);

    i3c_reg_write32(id, I3C_HCI_RHS_CTRL, pstPreamble->u4Max_header_cnt);

    pstRingCap = (struct HAL_I3C_RING_CAP *)i3c_ring_cap[id];
    memset(i3c_ring_cap[id], 0, sizeof(i3c_ring_cap[id]));
    HAL_I3C_TRACE("pstRingCap Address   :0x%X", (uint32_t)pstRingCap);

    hal_i3c_get_rings_cap(id, pstRingCap, pstbusInfo->u8HWMaxRings);

    HAL_I3C_TRACE("  ");
    HAL_I3C_TRACE("pstRingCap Address before assigining :0x%X", (uint32_t)pstRingCap);
    for (i = 0; i < pstbusInfo->u8HWMaxRings; i++) {
        pstbusInfo->pstRingCap[i] = pstRingCap;
        HAL_I3C_TRACE("pstRingCap + %d Address               :0x%X", i, (uint32_t)pstRingCap);
        HAL_I3C_TRACE("pstbusInfo->pstRingCap[%d] Address    :0x%X", i, (uint32_t)pstbusInfo->pstRingCap[i]);
        pstRingCap++;
    }
    HAL_I3C_TRACE("pstRingCap Address after assigining  :0x%X", (uint32_t)pstRingCap);

    pstRingDesc = (struct HAL_I3C_DMA_RING_XFER *)i3c_ring_desc[id];
    memset(pstRingDesc, 0x0, sizeof(struct HAL_I3C_DMA_RING_XFER) * MAX_RINGS);
    HAL_I3C_TRACE("  ");
    HAL_I3C_TRACE("pstRingDesc Address before assigining:0x%X", (uint32_t)pstRingDesc);
    for (i = 0; i < pstbusInfo->u8HWMaxRings; i++) {
        pstbusInfo->pstRingDesc[i] = pstRingDesc;
        HAL_I3C_TRACE("pstRingDesc + %d Address              :0x%X", i, (uint32_t)pstRingDesc);
        HAL_I3C_TRACE("pstbusInfo->pstRingDesc[%d] Address   :0x%X", i, (uint32_t)pstbusInfo->pstRingDesc[i]);
        pstRingDesc++;
    }
    HAL_I3C_TRACE("pstRingDesc Address after assigining :0x%X", (uint32_t)pstRingDesc);

    //Ring Formation
    for (i = 0; i < pstbusInfo->u8HWMaxRings; i++) {
        //Framing Transfer Descriptor
        //u32TxDescSize = sizeof(struct HAL_I3C_RING_TX_DESC) * pstbusInfo->pstRingCap[i]->u8XferStrSize * pstbusInfo->pstRingCap[i]->u8RingSize;

        //No Of Entries for each Ring
        pstbusInfo->pstRingDesc[i]->pstTxDesc = (struct HAL_I3C_RING_TX_DESC *)Ring_TxDesc[id][i];
        u64TxDescAddr = ADDR_CPU_TO_DEV((uint32_t)pstbusInfo->pstRingDesc[i]->pstTxDesc);
        if (pstbusInfo->pstRingDesc[i]->pstTxDesc == NULL) {
            HAL_I3C_ERROR("ERR, ERRpstbusInfo->pstRingDesc[i]->pstRsp: Cannot Allocate Memory for Transfer Descriptor");
            return I3C_ERR_NULL_POINTER;
        }

        HAL_I3C_TRACE("  ");
        HAL_I3C_TRACE("pstRingDesc[%d]->pstTxDesc addr  :0x%X", i, (uint32_t)pstbusInfo->pstRingDesc[i]->pstTxDesc);

        //u32RspDescSize = sizeof(struct HAL_I3C_RESP_DESC) * pstbusInfo->pstRingCap[i]->u8RingSize;
        //HAL_I3C_TRACE("u32RspDescSize:0x%X", u32RspDescSize);

        //Framing Resp Descriptor
        pstbusInfo->pstRingDesc[i]->pstRsp = (struct HAL_I3C_RESP_DESC *)Ring_RespDesc[id][i];
        u64RspDescAddr = ADDR_CPU_TO_DEV((uint32_t)pstbusInfo->pstRingDesc[i]->pstRsp);
        if (pstbusInfo->pstRingDesc[i]->pstRsp == NULL) {
            HAL_I3C_ERROR("ERR, Cannot Allocate Memory for Response Descriptor");
            return I3C_ERR_NULL_POINTER;
        }

        HAL_I3C_TRACE("pstRingDesc[%d]->pstRsp addr     :0x%X", i, (uint32_t)pstbusInfo->pstRingDesc[i]->pstRsp);

        hal_i3c_program_ring_base_addr(id, u64TxDescAddr, u64RspDescAddr, i);

    }
    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : struct HAL_I3C_BUS_CTRL_INFO *pstbusInfo
Return Value : 0 - Success
               Others - FAILURE
Description  : This function populates the PIO Queue
*********************************************************************/
static int32_t hal_i3c_populate_pio_queue(enum HAL_I3C_ID_T id, struct HAL_I3C_BUS_CTRL_INFO *pstbusInfo)
{
    uint32_t u32RdVal;
    struct HAL_I3C_PIO_XFER *pstPIOTx;
    struct HAL_I3C_PIO_QUE_SIZE_CTRL *pstPIOCtrl;
    struct HAL_I3C_HOST_T *host = &i3c_host[id];

    pstPIOCtrl = &host->pio_ctrl;
    memset(pstPIOCtrl, 0, sizeof(struct HAL_I3C_PIO_QUE_SIZE_CTRL));
    pstbusInfo->pstPIOCtrl = pstPIOCtrl;

    //PIO Queue Size
    i3c_reg_read32(id, I3C_HCI_PIO_QUE_SIZE, &u32RdVal);
    memcpy((uint8_t *)pstPIOCtrl, (uint8_t *)&u32RdVal, sizeof(u32RdVal));

    HAL_I3C_TRACE("------------------------------");
    HAL_I3C_TRACE("CRQueSize :0x%X", pstPIOCtrl->u8CRQueSize);
    HAL_I3C_TRACE("IBIStsSize:0x%X", pstPIOCtrl->u8IBIStsSize);
    HAL_I3C_TRACE("RxBufSize :0x%X", pstPIOCtrl->u8RxBufSize);
    HAL_I3C_TRACE("TxBufSize :0x%X", pstPIOCtrl->u8TxBufSize);
    HAL_I3C_TRACE("------------------------------");

    //PIO Queue Formation
    pstPIOTx = (struct HAL_I3C_PIO_XFER *)i3c_pio_queue[id];
    memset(pstPIOTx, 0x0, sizeof(struct HAL_I3C_PIO_XFER) * pstPIOCtrl->u8CRQueSize);
    pstbusInfo->pstPIOTx = pstPIOTx;
    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : pstTxDesc ?pointer to CmdDataDesc structure
               bus_info ?pointer to I3CBusCtrlInfo structure
Return Value : 0 - Success
               Others - FAILURE
Description  : This function processes the response in DMA once response interrupt is set.
*********************************************************************/
static int32_t hal_i3c_process_dma_resp(enum HAL_I3C_ID_T id, struct HAL_I3C_CMD_DATA_DESC *pstTxDesc, uint8_t u8RingID,
                                        struct HAL_I3C_BUS_CTRL_INFO *bus_info)
{
    uint8_t u8ResTID = 0, u8CmdTID = 0, i, j = 0;
    uint32_t DeQptr = 0, SwDQptr = 0, u32RdValue = 0;

    struct HAL_I3C_CMD_DATA_DESC *TempTxDesc = NULL;
    struct HAL_I3C_DMA_RING_XFER *pstRingDesc = NULL;
    TempTxDesc = pstTxDesc;

    HAL_I3C_TRACE("%s: ", __func__);
    HAL_I3C_TRACE("     u8RingID                :0x%X", u8RingID);
    HAL_I3C_TRACE("     Address of TempTxDesc   :0x%X", (uint32_t)TempTxDesc);
    HAL_I3C_TRACE("     Address of pstTxDesc    :0x%X", (uint32_t)pstTxDesc);

    HAL_I3C_TRACE("RIND ID %X", RING_ID_0);
    pstRingDesc = bus_info->pstRingDesc[u8RingID];
    while (!(u32RdValue & RSP_DQPTR_MASK)) {
        i3c_reg_read32(id, RHx_REG(u8RingID, RING_OPR2), &u32RdValue);
    }
    DeQptr = u32RdValue & RSP_DQPTR_MASK;
    HAL_I3C_TRACE("DeQptr: %d", DeQptr);

    if (DeQptr == 0) {
        return I3C_ERR_INVALID_VALUE;
    }
    while (j < DeQptr) {
        for (i = 0; i < DeQptr; i++) {
            u8ResTID = pstRingDesc->pstRsp->u04TID;//chk the pointer increment
            u8CmdTID = (uint8_t) TempTxDesc->pstCmdDesc->u04TID;
            HAL_I3C_TRACE("u8ResTID:0x%X", u8ResTID);
            HAL_I3C_TRACE("u8CmdTID:0x%X", u8CmdTID);

            if (u8ResTID == u8CmdTID) {
                memcpy(TempTxDesc->pstRspDesc, pstRingDesc->pstRsp, sizeof(struct HAL_I3C_RESP_DESC));
                SwDQptr++;
                break;
            }
            pstRingDesc++;
        }
        pstRingDesc = bus_info->pstRingDesc[u8RingID];//bringing the pointer back to home position
        TempTxDesc++;
        HAL_I3C_TRACE("     Address of TempTxDesc after increment   :0x%X", (uint32_t)TempTxDesc);
        HAL_I3C_TRACE("     Address of pstTxDesc after increment    :0x%X", (uint32_t)pstTxDesc);

        j++;
    }
    i3c_reg_read32(id, RHx_REG(u8RingID, RING_OPR1), &u32RdValue);
    u32RdValue &= RSP_SWDQ_PTR_MASK;//make a #define
    u32RdValue |= SwDQptr << 8;
    i3c_reg_write32(id, RHx_REG(u8RingID, RING_OPR1), u32RdValue);
    i3c_reg_read32(id, RHx_REG(u8RingID, RING_STATUS), &u32RdValue);
    //In order to pull down the RH_CONTROL_RS so that the OP1_ENQ_PTR can be writen in next communication
    i3c_reg_read32(id, RHx_REG(u8RingID, RING_CTRL), &u32RdValue);
    i3c_reg_write32(id, RHx_REG(u8RingID, RING_CTRL), u32RdValue & (~(RH_ENABLE_REQ | RH_RS_REQ)));
    i3c_reg_read32(id, RHx_REG(u8RingID, RING_STATUS), &u32RdValue);

    return I3C_ERR_NONE;
}

static void hal_i3c_resp_status(uint8_t sts)
{
    switch (sts) {
        case 0x0:
            HAL_I3C_TRACE("I3C_ERR_NONE: Transfer successful, no error");
            break;
        case 0x1:
            HAL_I3C_ERROR("ERR, CRC: CRC Error");
            break;
        case 0x2:
            HAL_I3C_ERROR("ERR, PARITY: Parity Error");
            break;
        case 0x3:
            HAL_I3C_ERROR("ERR, FRAME: Frame Error");
            break;
        case 0x4:
            HAL_I3C_ERROR("ERR, ADDR_HEADER: Address Header Error");
            break;
        case 0x5:
            HAL_I3C_ERROR("ERR, NACK: Address Nacked or Dynamic Address Assignment Nacked");
            break;
        case 0x6:
            HAL_I3C_ERROR("ERR, OVL: Receive Overflow or Transfer Underflow Error");
            break;
        case 0x7:
            HAL_I3C_ERROR("ERR, RESERVED");
            break;
        case 0x8:
            HAL_I3C_ERROR("ERR, ABORTED: Aborted");
            break;
        case 0x9:
            HAL_I3C_ERROR("ERR, RESERVED");
            break;
        case 0xA:
            HAL_I3C_ERROR("ERR, NOT_SUPPORTED");
            break;
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
            HAL_I3C_ERROR("ERR, RESERVED");
            break;
        default:
            break;
    }
}

static void hal_i3c_ring_status(uint8_t sts)
{
    if (!(sts & RH_ENABLED_MASK)) {
        HAL_I3C_ERROR("ERR, RING disabled");
    } else if (sts & RH_RUNNING_MASK) {
        HAL_I3C_TRACE("RING running");
    } else if (sts & RH_ABORTED_MASK) {
        HAL_I3C_ERROR("ERR, RING aborted");
    } else if (sts & RH_LOCKED_MASK) {
        HAL_I3C_ERROR("ERR, RING locked");
    }
}

/*********************************************************************
Argument     : pstTxDesc ?pointer to CmdDataDesc structure
Return Value : 0 - Success
               Others - FAILURE
Description  : This function processes the response in PIO once response interrupt is set.
*********************************************************************/
static int32_t hal_i3c_process_pio_resp(enum HAL_I3C_ID_T id, struct HAL_I3C_CMD_DATA_DESC *pstTxDesc)
{
    uint32_t u32RdValue = 0, u32noofdwords;
    uint8_t i = 0, j = 0, u8ResTID = 0, u8CmdTID = 0;
    uint16_t u16ResLen = 0, u16CmdLen = 0;
    struct HAL_I3C_CMD_DATA_DESC *TempTxDesc = pstTxDesc;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    HAL_I3C_TRACE("enter:%s", __func__);
    HAL_I3C_TRACE("NoOfEntries:%d", bus_info->pstPIOTx->u32NoOfEntries);
    while (j < bus_info->pstPIOTx->u32NoOfEntries) {
        i3c_reg_read32(id, I3C_HCI_PIO_RSP_QUE_PORT, &u32RdValue);
        u8ResTID = (uint8_t)(u32RdValue >> 24) & RSP_PIO_TID_MASK;
        u16ResLen = (uint16_t)(u32RdValue) & RSP_PIO_LEN_MASK;
        HAL_I3C_TRACE("pio resp queue port:0x%X", u32RdValue);
        HAL_I3C_TRACE("Resp TID:0x%X", u8ResTID);
        HAL_I3C_TRACE("Resp Len:0x%X", u16ResLen);

        for (i = 0; i < bus_info->pstPIOTx->u32NoOfEntries; i++) {
            u8CmdTID = (uint8_t)pstTxDesc->pstCmdDesc->u04TID;
            HAL_I3C_TRACE("CmdTID:0x%X", u8CmdTID);

            if (u8ResTID == u8CmdTID) {
                memcpy(pstTxDesc->pstRspDesc, &u32RdValue, sizeof(u32RdValue));

                HAL_I3C_TRACE("Error Status:0x%X", pstTxDesc->pstRspDesc->u04ErrSts);
                HAL_I3C_TRACE("TID         :0x%X", pstTxDesc->pstRspDesc->u04TID);

                hal_i3c_resp_status(pstTxDesc->pstRspDesc->u04ErrSts);

                if (pstTxDesc->pstCmdDesc->u08CMD == CCC_BC_ENTDAA) {
                    HAL_I3C_TRACE("Remaining Device Count:0x%X", pstTxDesc->pstRspDesc->u16DataLen);
                    if (pstTxDesc->pstRspDesc->u16DataLen > 0)
                        HAL_I3C_TRACE("%d Device not yet participated in dynamic addressing", pstTxDesc->pstRspDesc->u16DataLen);
                } else if (pstTxDesc->pstCmdDesc->u01Dir_rnw == RD_XFER) {
                    HAL_I3C_TRACE("Received data length in bytes:0x%X", pstTxDesc->pstRspDesc->u16DataLen);

                    if (pstTxDesc->pu8Buf != NULL) {
                        u16CmdLen = pstTxDesc->pstCmdDesc->u16Data_len;
                        HAL_I3C_TRACE("u16CmdLen:0x%X", u16CmdLen);
                        HAL_I3C_TRACE("u16ResLen:0x%X", u16ResLen);

                        if (u16CmdLen == u16ResLen) {

                            HAL_I3C_TRACE("Data Length matched");
                            u32noofdwords = u16ResLen / (sizeof(u32RdValue));

                            if (u16ResLen % (sizeof(u32RdValue)))
                                u32noofdwords++;

                            u32RdValue = 0;
                            while ((bus_info->words_cnt > 0) && (bus_info->words_cnt <= u32noofdwords)) {

                                i3c_reg_read32(id, I3C_HCI_PIO_DATA_PORT, &u32RdValue);

                                if (u32RdValue == 0xffffffff) {
                                    HAL_I3C_TRACE("Not able to Read DATA Port");
                                    break;
                                } else {
                                    memcpy(bus_info->glpstCmdData->pu8Buf + (bus_info->read_index * sizeof(u32RdValue)), \
                                           &u32RdValue, sizeof(u32RdValue));
                                    bus_info->words_cnt--;
                                    bus_info->read_index++;
                                }
                            }
                        } else {
                            HAL_I3C_ERROR("ERR, Data Length Mismatch");
                            return I3C_ERR_INVALID_VALUE;
                        }
                        break;
                    }
                } else {
                    HAL_I3C_TRACE("Remaining data length in bytes:0x%X", pstTxDesc->pstRspDesc->u16DataLen);
                }
            } else {
                HAL_I3C_ERROR("resp TID:%d != cmd TID:%d", u8ResTID, u8CmdTID);
            }
            pstTxDesc++;
        }

        pstTxDesc = TempTxDesc;
        j++;
    }
    return I3C_ERR_NONE;
}

/*******************************************************************************
*****************************HCI operation interface****************************
*******************************************************************************/

/*********************************************************************
Argument     : Threshold values for CMD/Rsp/IBI bufferes in both DMA/PIO modes
Return       : 0 - Success
               Others - Error
Description  : This API is used to set threshold value for  CMD/Rsp/IBI buffers
               in both DMA/PIO modes
*********************************************************************/
static int32_t hal_i3c_set_cmd_resp_threshold(enum HAL_I3C_ID_T id, uint8_t u8TxMode,
        uint8_t u8CmdThld, uint8_t u8RspThld, uint8_t u8IBIThld, uint8_t u8IBIStsThld)
{
    uint32_t u32Thld;
    struct HAL_I3C_PIO_QUE_SIZE_CTRL *QCtrl;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    if (bus_info == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s", __func__);
        return I3C_ERR_NULL_POINTER;
    }

    QCtrl = bus_info->pstPIOCtrl;

    HAL_I3C_TRACE("Trasfer Mode                 :%s", (u8TxMode == PIO_MODE) ? "PIO" : "DMA");
    HAL_I3C_TRACE("Command Empty Buffer Thresold:0x%X", u8CmdThld);
    HAL_I3C_TRACE("Response Buffer Thresold     :0x%X", u8RspThld);
    HAL_I3C_TRACE("IBI Data Segment Size        :0x%X", u8IBIThld);
    HAL_I3C_TRACE("IBI Status Thresold          :0x%X", u8IBIStsThld);

    if (u8CmdThld > QCtrl->u8CRQueSize  || u8RspThld > QCtrl->u8CRQueSize || \
        u8IBIThld > QCtrl->u8IBIStsSize || u8IBIStsThld > QCtrl->u8IBIStsSize) {
        HAL_I3C_ERROR("ERR, PIO: Threshold is greater than buffer size, line:%d", __LINE__);
        return I3C_ERR_THLD_EXCEED;
    }

    u32Thld = (u8IBIStsThld << 24) | (u8IBIThld << 16) | (u8RspThld << 8) | u8CmdThld;

    switch (u8TxMode) {
        case DMA_MODE: {
            // TODO: 2023-05-08
            //i3c_reg_write32(id, RH0_RING_QUE_THLD, u32Thld);
            break;
        }
        case PIO_MODE: {
            i3c_reg_write32(id, I3C_HCI_PIO_QUE_THLD_CTRL, u32Thld);
            break;
        }
        default: {
            HAL_I3C_ERROR("ERR, Invalid Transfer Mode");
            return I3C_ERR_INVALID_MODE;
        }
    }
    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : Threshold values for data bufferes in both DMA/PIO modes
Return       : 0 - Success
               Others - Error
Description  : This API is used to set threshold value for  Rd/Write data  buffers
               in both DMA/PIO modes
*********************************************************************/
static int32_t hal_i3c_set_data_threshold(enum HAL_I3C_ID_T id, uint8_t u8TxMode, uint8_t u8TxThld,
        uint8_t u8TxStThld, uint8_t u8RxThld, uint8_t u8RxStThld)
{
    uint32_t u32Thld;
    struct HAL_I3C_PIO_QUE_SIZE_CTRL *QCtrl;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    if (bus_info == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s", __func__);
        return I3C_ERR_NULL_POINTER;
    }

    QCtrl = bus_info->pstPIOCtrl;

    HAL_I3C_TRACE("Trasfer Mode                 :%s", (u8TxMode == PIO_MODE) ? "PIO" : "DMA");
    HAL_I3C_TRACE("Tx Buf Thresold              :0x%X", u8TxThld);
    HAL_I3C_TRACE("Rx Buf Thresold              :0x%X", u8RxThld);
    HAL_I3C_TRACE("Tx Start Thresold            :0x%X", u8TxStThld);
    HAL_I3C_TRACE("Rx Start Thresold            :0x%X", u8RxStThld);

    if (u8TxThld > QCtrl->u8TxBufSize || u8TxStThld > QCtrl->u8TxBufSize || \
        u8RxThld > QCtrl->u8RxBufSize || u8RxStThld > QCtrl->u8RxBufSize) {
        HAL_I3C_ERROR("ERR, PIO: Threshold is greater than buffer size, line:%d", __LINE__);
        return I3C_ERR_THLD_EXCEED;
    }

    u32Thld = (u8RxStThld << 24) | (u8TxStThld << 16) | (u8RxThld << 8) | u8TxThld;

    switch (u8TxMode) {
        case DMA_MODE: {
            // TODO: 2023-05-08
            //i3c_reg_write32(id, RH0_RING_DAT_THLD, u32Thld);
            break;
        }
        case PIO_MODE: {
            i3c_reg_write32(id, I3C_HCI_PIO_DAT_THLD_CTRL, u32Thld);
            break;
        }
        default: {
            HAL_I3C_ERROR("ERR, Invalid Transfer Mode");
            return I3C_ERR_INVALID_MODE;
        }
    }
    return I3C_ERR_NONE;
}

static int32_t hal_i3c_pio_thld_init(enum HAL_I3C_ID_T id)
{
    enum HAL_I3C_ERR ret;
    struct HAL_I3C_PIO_QUE_THLD_CTRL pio_que_thld_ctrl;
    struct HAL_I3C_PIO_DATA_BUF_THLD_CTRL pio_data_thld_ctrl;

    //Set Queue thresholds and DATA_BUFFER_THLD_CTRL
    pio_que_thld_ctrl.u8CmdEmptyBufThld = 1;
    pio_que_thld_ctrl.u8RespBufThld = 1;
    pio_que_thld_ctrl.u8IBIDateSegSize = 1;
    pio_que_thld_ctrl.u8IBIStatusThld = 1;
    ret = hal_i3c_set_cmd_resp_threshold(id, PIO_MODE, pio_que_thld_ctrl.u8CmdEmptyBufThld,
                                         pio_que_thld_ctrl.u8RespBufThld,
                                         pio_que_thld_ctrl.u8IBIDateSegSize,
                                         pio_que_thld_ctrl.u8IBIStatusThld);
    if (ret) {
        HAL_I3C_ERROR("ERR, hal_i3c_set_cmd_resp_threshold fail");
        return ret;
    }

    HAL_I3C_TRACE(" ");
    pio_data_thld_ctrl.u3TxBufThld = 0;
    pio_data_thld_ctrl.u3RxBufThld = 0;
    pio_data_thld_ctrl.u3TxStartThld = 1;
    pio_data_thld_ctrl.u3RxStartThld = 1;
    ret = hal_i3c_set_data_threshold(id, PIO_MODE, pio_data_thld_ctrl.u3TxBufThld,
                                     pio_data_thld_ctrl.u3TxStartThld,
                                     pio_data_thld_ctrl.u3RxBufThld,
                                     pio_data_thld_ctrl.u3RxStartThld);
    if (ret) {
        HAL_I3C_ERROR("ERR, hal_i3c_set_data_threshold fail");
        return ret;
    }
    HAL_I3C_TRACE("------------------------------");

    //enable interrupt
    i3c_reg_write32(id, I3C_HCI_PIO_INTR_EN, I3C_PIO_INTR_STS_EN);
    i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, I3C_PIO_INTR_SIG_EN);

    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : struct TxCmdData *pstTxCmdData - Queue of CMDs
               NoOfEntries - no Of entries in the queue
Return       : 0 - Success
               Others - Error
Description  : This API is used to send I3C CMDs/Data to/from the devices.
*********************************************************************/
static int32_t hal_i3c_send_cmd_data_dma(enum HAL_I3C_ID_T id, struct HAL_I3C_CMD_DATA_DESC *pstTxCmdData, uint32_t NoOfEntries)
{
    uint8_t i = 0;
    uint8_t u8RingID = 0;
    int32_t s32Status;
    struct HAL_I3C_CMD_DATA_DESC *TempTxCmdData = pstTxCmdData;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    if (bus_info->error_flag) {
        bus_info->error_flag = 0;
        HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
        return I3C_ERR_TRANSFER;
    }

    if (bus_info == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    if (NoOfEntries > I3C_MAX_ENTRIES) {
        HAL_I3C_ERROR("ERR, No Of Entries Exceed");
        return I3C_ERR_ENTRIES_EXCEED;
    }

    if (pstTxCmdData == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    if (pstTxCmdData->pstCmdDesc == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    if (pstTxCmdData->pstRspDesc == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    if ((pstTxCmdData->pu8Buf == NULL) && (pstTxCmdData->pstCmdDesc->u08CMD != CCC_BC_ENTDAA && \
                                           pstTxCmdData->pstCmdDesc->u08CMD != CC_DS_SETDASA)) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    bus_info->dma_xfer_done = 0;
    while (1) {
        hal_i3c_frame_dma_cmd_desc(id, pstTxCmdData, u8RingID, i, bus_info);

        if (pstTxCmdData->pstCmdDesc->u01TOC == TOC_STOP) {
            s32Status = wait_for_completion_timeout(id, &bus_info->dma_xfer_done, 1, RESP_RDY_TIMEOUT, 0);
            if (bus_info->error_flag) {
                bus_info->error_flag = 0;
                HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
                return I3C_ERR_TRANSFER;
            }
            if (s32Status == 0) {
                HAL_I3C_TRACE("RESPONSE READY STATUS INTERRUPT SET");

                pstTxCmdData = TempTxCmdData;
                s32Status = hal_i3c_process_dma_resp(id, pstTxCmdData, u8RingID, bus_info);
                if (s32Status < 0) {
                    HAL_I3C_ERROR("ERR %d, line:%d", s32Status, __LINE__);
                    return s32Status;
                }
            } else {
                HAL_I3C_ERROR("ERR %d, line:%d", s32Status, __LINE__);
                return s32Status;
            }
            break;
        }

        pstTxCmdData++;
        i++;
    }
    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : struct TxCmdData *pstTxCmdData - Queue of CMDs
               NoOfEntries - no Of entries in the queue
Return       : 0 - Success
               Others - Error
Description  : This API is used to send I3C CMDs/Data to/from the devices.
*********************************************************************/
static int32_t hal_i3c_send_cmd_data_pio(enum HAL_I3C_ID_T id, struct HAL_I3C_CMD_DATA_DESC *pstTxCmdData,
        uint32_t NoOfEntries)
{
    uint8_t i;
    int32_t s32Status;
    uint16_t u16DataLen;
    uint32_t u32RdValue;
    struct HAL_I3C_CMD_DATA_DESC *TempTxCmdData = pstTxCmdData;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    if (bus_info->error_flag) {
        bus_info->error_flag = 0;
        HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
        return I3C_ERR_TRANSFER;
    }

    if (bus_info == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    if (NoOfEntries > I3C_MAX_ENTRIES) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_ENTRIES_EXCEED;
    }

    if (pstTxCmdData == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    if (pstTxCmdData->pstCmdDesc == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    if (pstTxCmdData->pstRspDesc == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    if (pstTxCmdData->pstCmdDesc->u03CmdAttr == CMD_ATRB_REGULAR_TX && pstTxCmdData->pu8Buf == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    for (i = 0; i < NoOfEntries; i++) {
        HAL_I3C_TRACE("TID Assigned :0x%X", pstTxCmdData->pstCmdDesc->u04TID);
        hal_i3c_print_cmd_desc(pstTxCmdData);

        bus_info->pstPIOTx->u32NoOfEntries = NoOfEntries;
        hal_i3c_frame_pio_cmd_queue(id, pstTxCmdData, bus_info);
        HAL_I3C_TRACE("pstTxCmdData->pstCmdDesc->u01TOC = %d", pstTxCmdData->pstCmdDesc->u01TOC);

        if (pstTxCmdData->pstCmdDesc->u01TOC == TOC_STOP) {
            bus_info->resp_rdy = 0;
            bus_info->rsp_rdy_compln = 0;

            i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32RdValue);
            i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, INTR_RSP_RDY_STS | u32RdValue);

            bus_info->glpstCmdData = TempTxCmdData;
            u16DataLen = bus_info->glpstCmdData->pstCmdDesc->u16Data_len;

            if ((pstTxCmdData->pstCmdDesc->u01Dir_rnw == RD_XFER) && (u16DataLen >= 32)) {
                hal_i3c_pio_read(id);
            } else {
                bus_info->read_index = 0;
                bus_info->words_cnt = u16DataLen / (sizeof(u32RdValue));
                if (u16DataLen % (sizeof(u32RdValue))) {
                    bus_info->words_cnt++;
                }
            }

            s32Status = wait_for_completion_timeout(id, &bus_info->rsp_rdy_compln, 1, RESP_RDY_TIMEOUT, 0);
            if (bus_info->error_flag) {
                bus_info->error_flag = 0;
                HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
                return I3C_ERR_TRANSFER;
            }
            if (s32Status == 0) {
                bus_info->resp_rdy = 1;
                HAL_I3C_TRACE("RESPONSE READY STATUS INTERRUPT SET");
                pstTxCmdData = TempTxCmdData;
                s32Status = hal_i3c_process_pio_resp(id, pstTxCmdData);
                if (s32Status < 0) {
                    HAL_I3C_ERROR("ERR %d, line:%d", s32Status, __LINE__);
                    return s32Status;
                }
            } else {
                HAL_I3C_ERROR("ERR %d, line:%d", s32Status, __LINE__);
                return s32Status;
            }
            break;
        } else {
            /*
                Update the Response Queue buffer pointer, otherwise the next read
                data will still be the previous value
            */
            i3c_reg_read32(id, I3C_HCI_PIO_RSP_QUE_PORT, &u32RdValue);
            HAL_I3C_TRACE("I3C_HCI_PIO_RSP_QUE_PORT:0x%X", u32RdValue);
            hal_sys_timer_delay_us(10);// TODO:The delay to be removed
        }

        pstTxCmdData++;
    }
    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : uint8_t u8SlaveAddr- The slave?s dynamic addres
               struct HAL_I3C_SLAVE_BCR *BCR- Pointer to the BCR structure
               uint8_t *p8DCR- Pointer to the DCR value
Return Value : 0 - Success
               Others - FAILURE
Description  : This API is used to get the information about all the I3C/I2C slave devices
               characteristics connected on the I3C bus.
*********************************************************************/
int32_t hal_i3c_get_dev_characteristics(enum HAL_I3C_ID_T id, uint8_t u8SlaveAddr, uint8_t *BCR, uint8_t *p8DCR)
{
    struct HAL_I3C_SLAVE_BCR *ptrBCR;
    struct HAL_I3C_SLAVE_DCT *pstSlvDCT;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    if (bus_info == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    ptrBCR = bus_info->BCR;
    pstSlvDCT = bus_info->pstSlvDCT;

    if (BCR == NULL || p8DCR == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s:%d", __func__, __LINE__);
        return I3C_ERR_NULL_POINTER;
    }

    if (u8SlaveAddr == BC_DEVICES_ADDR) {
        memcpy(BCR, bus_info->BCR, sizeof(struct HAL_I3C_SLAVE_BCR) * bus_info->u16DATSize);
        *p8DCR = bus_info->pstSlvDCT->u08DCR;
    } else {
        for (int i = 0; i < bus_info->u16DATSize; i++) {
            if (u8SlaveAddr == bus_info->pstSlvDAT->u08DynAddr) {
                memcpy(BCR, bus_info->BCR, sizeof(struct HAL_I3C_SLAVE_BCR));
                *p8DCR = bus_info->pstSlvDCT->u08DCR;

                bus_info->BCR = ptrBCR;
                bus_info->pstSlvDCT = pstSlvDCT;
                return I3C_ERR_NONE;
            } else {
                bus_info->BCR++;
                bus_info->pstSlvDCT++;
            }
        }
        bus_info->BCR = ptrBCR;
        bus_info->pstSlvDCT = pstSlvDCT;
        return I3C_ERR_DEV_NOT_FOUND;
    }
    return I3C_ERR_DEV_NOT_FOUND;
}

/*********************************************************************
Return       : 0 - Success
               Others - Error
*********************************************************************/
enum HAL_I3C_ERR hal_i3c_ccc_read(enum HAL_I3C_ID_T id, uint8_t dev_index, enum HAL_I3C_CCC_CMD_T cmd, uint16_t data_len, uint8_t *data)
{
    struct HAL_I3C_RESP_DESC resp_desc;
    struct HAL_I3C_CMD_DATA_DESC data_desc;
    struct HAL_I3C_CMDDESC_RDTC cmd_desc;
    struct HAL_I3C_HOST_T *host = &i3c_host[id];

    memset(&resp_desc, 0x0, sizeof(struct HAL_I3C_RESP_DESC));
    memset(&cmd_desc, 0x0, sizeof(struct HAL_I3C_CMDDESC_RDTC));
    memset(&data_desc, 0x0, sizeof(struct HAL_I3C_CMD_DATA_DESC));
    data_desc.pstCmdDesc = &cmd_desc;
    data_desc.pstRspDesc = &resp_desc;

    data_desc.pstCmdDesc->u03CmdAttr       = CMD_ATRB_REGULAR_TX;
    data_desc.pstCmdDesc->u04TID           = hal_i3c_get_tid();
    data_desc.pstCmdDesc->u08CMD           = cmd;
    data_desc.pstCmdDesc->u01CP            = 1;//0=>data, 1=>CMD
    data_desc.pstCmdDesc->u05DevIndex      = dev_index;
    data_desc.pstCmdDesc->u01Short_read_err = 0;
    data_desc.pstCmdDesc->u01Define_byte_present = 0;
    data_desc.pstCmdDesc->u03Speed_mode    = host->mode;
    data_desc.pstCmdDesc->u01Dir_rnw       = RD_XFER;
    data_desc.pstCmdDesc->u01ROC           = ROC_ON;
    data_desc.pstCmdDesc->u01TOC           = TOC_STOP;
    data_desc.pstCmdDesc->u8Define_byte    = 0;
    data_desc.pstCmdDesc->u16Data_len      = data_len;
    data_desc.pu8Buf = data;

    if (host->use_dma) {
        return hal_i3c_send_cmd_data_dma(id, &data_desc, 1);
    } else {
        return hal_i3c_send_cmd_data_pio(id, &data_desc, 1);
    }
}

/*********************************************************************
Argument     : u32Devindex - Device Index
Return Value : 0 - Success
               Others - FAILURE
Description  : This API is used to enumerate the devices in the bus using Direct cmd.
*********************************************************************/
static int32_t hal_i3c_set_da_sa_cmd(enum HAL_I3C_ID_T id, uint32_t u32Devindex)
{
    uint32_t u32RdValue;
    int32_t s32Status;
    struct HAL_I3C_CMDDESC_RDTC cmd_desc;
    struct HAL_I3C_RESP_DESC resp_desc;
    struct HAL_I3C_CMD_DATA_DESC data_desc;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    if (u32Devindex >= bus_info->priv->dev_cnt) {
        HAL_I3C_ERROR("ERR, Invalid Device Index, %s", __func__);
        return I3C_ERR_INVALID_PARA;
    }

    memset(&data_desc, 0x0, sizeof(struct HAL_I3C_CMD_DATA_DESC));
    memset(&cmd_desc, 0x0, sizeof(struct HAL_I3C_CMDDESC_RDTC));
    memset(&resp_desc, 0x0, sizeof(struct HAL_I3C_RESP_DESC));
    data_desc.pstCmdDesc = &cmd_desc;
    data_desc.pstRspDesc = &resp_desc;
    data_desc.pu8Buf = NULL;

    //Sending SETDASA Command
    data_desc.pstCmdDesc->u03CmdAttr = CMD_ATRB_DAA;
    data_desc.pstCmdDesc->u04TID = hal_i3c_get_tid();
    data_desc.pstCmdDesc->u08CMD = CC_DS_SETDASA;
    data_desc.pstCmdDesc->u05DevIndex = u32Devindex;
    data_desc.pstCmdDesc->u03Speed_mode = bus_info->priv->dev_cnt & 0x7;      //CmdDesc_RDTC replaces CmdDesc_DAA
    data_desc.pstCmdDesc->u01Dir_rnw = (bus_info->priv->dev_cnt & 0x80) >> 3; //u01Dir_rnw replaces bit29 of u04Dev_cnt
    data_desc.pstCmdDesc->u01ROC = ROC_ON;
    data_desc.pstCmdDesc->u01TOC = TOC_STOP;
    hal_i3c_print_cmd_desc(&data_desc);

    if (bus_info->priv->use_dma) {
        s32Status = hal_i3c_send_cmd_data_dma(id, &data_desc, 1);
        if (s32Status < 0) {
            HAL_I3C_ERROR("ERR %d, line:%d", s32Status, __LINE__);
            return s32Status;
        }
    } else {
        bus_info->pstPIOTx->u32NoOfEntries = bus_info->priv->dev_cnt;
        hal_i3c_frame_pio_cmd_queue(id, &data_desc, bus_info);
        if (bus_info->error_flag) {
            bus_info->error_flag = 0;
            HAL_I3C_ERROR("ERR, %s:%d", __func__, __LINE__);
            return I3C_ERR_INVALID_VALUE;
        }
        bus_info->rsp_rdy_compln = 0;

        i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32RdValue);
        i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, INTR_RSP_RDY_STS | u32RdValue);

        s32Status = wait_for_completion_timeout(id, &bus_info->rsp_rdy_compln, 1, RESP_RDY_TIMEOUT, 0);
        if (bus_info->error_flag) {
            bus_info->error_flag = 0;
            HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
            return I3C_ERR_TRANSFER;
        }
        if (s32Status == 0) {
            HAL_I3C_TRACE("RESPONSE READY STATUS INTERRUPT BIT SET");
            s32Status = hal_i3c_process_pio_resp(id, &data_desc);
            if (s32Status < 0) {
                HAL_I3C_ERROR("ERR %d, line:%d", s32Status, __LINE__);
                return s32Status;
            }
        } else {
            HAL_I3C_ERROR("ERR %d, RESPONSE READY STATUS INTERRUPT BIT NOT SET, line:%d", s32Status, __LINE__);
            return s32Status;
        }
    }

    return I3C_ERR_NONE;
}

/*********************************************************************
Return Value : No of Devices present in the bus
Description  : This API is used to enumerate the devices in the bus using broadcast cmd.
*********************************************************************/
enum HAL_I3C_ERR hal_i3c_bus_enumeration(enum HAL_I3C_ID_T id, enum HAL_I3C_ENUM_CMD_T enum_cmd)
{
    uint32_t i, u32RdVal;
    uint32_t u32RdValue;
    int32_t s32Status;
    uint8_t u8IncOffset = 0;
    struct HAL_I3C_CMD_DATA_DESC tx_desc;
    struct HAL_I3C_CMDDESC_RDTC cmd_desc;
    struct HAL_I3C_RESP_DESC resp_desc;
    struct HAL_I3C_SLAVE_BCR *ptrBCR;
    struct HAL_I3C_SLAVE_DCT *pstSlvDCT;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    HAL_I3C_TRACE("==============================");
    HAL_I3C_TRACE("Bus Enumeration");
    HAL_I3C_TRACE("==============================");

    if (bus_info == NULL) {
        HAL_I3C_ERROR("NULL Pointer, %s", __func__);
        return I3C_ERR_NULL_POINTER;
    }

    hal_i3c_populate_dev_addr_table(id, bus_info);
    ptrBCR = bus_info->BCR;
    pstSlvDCT = bus_info->pstSlvDCT;

    memset(&tx_desc, 0x0, sizeof(struct HAL_I3C_CMD_DATA_DESC));
    memset(&cmd_desc, 0x0, sizeof(struct HAL_I3C_CMDDESC_RDTC));
    memset(&resp_desc, 0x0, sizeof(struct HAL_I3C_RESP_DESC));
    tx_desc.pstCmdDesc = &cmd_desc;
    tx_desc.pstRspDesc = &resp_desc;
    tx_desc.pu8Buf = NULL;

    if (enum_cmd == HAL_I3C_ENUM_CMD_DAA) {
        //Sending ENTDAA Command
        tx_desc.pstCmdDesc->u03CmdAttr = CMD_ATRB_DAA;
        tx_desc.pstCmdDesc->u04TID = hal_i3c_get_tid();
        tx_desc.pstCmdDesc->u08CMD = CCC_BC_ENTDAA;
        tx_desc.pstCmdDesc->u05DevIndex = 0x0; //from the starting index
        tx_desc.pstCmdDesc->u03Speed_mode = bus_info->priv->dev_cnt & 0x7;      //CmdDesc_RDTC replaces CmdDesc_DAA
        tx_desc.pstCmdDesc->u01Dir_rnw = (bus_info->priv->dev_cnt & 0x80) >> 3; //u01Dir_rnw replaces bit29 of u04Dev_cnt
        tx_desc.pstCmdDesc->u01ROC = ROC_ON;
        tx_desc.pstCmdDesc->u01TOC = TOC_STOP;
        hal_i3c_print_cmd_desc(&tx_desc);

        if (bus_info->priv->use_dma) {
            s32Status = hal_i3c_send_cmd_data_dma(id, &tx_desc, 1);
            if (s32Status < 0) {
                HAL_I3C_ERROR("ERR %d, line:%d", s32Status, __LINE__);
                return s32Status;
            }
        } else {
            bus_info->pstPIOTx->u32NoOfEntries = bus_info->priv->dev_cnt;
            hal_i3c_frame_pio_cmd_queue(id, &tx_desc, bus_info);
            bus_info->rsp_rdy_compln = 0;
#ifdef FPGA
#ifndef I3C_DEBUG
            HAL_I3C_ERROR("Necessary delay...");
            HAL_I3C_ERROR(" ");
#endif
#endif

            i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32RdValue);
            i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, INTR_RSP_RDY_STS | u32RdValue);

            s32Status = wait_for_completion_timeout(id, &bus_info->rsp_rdy_compln, 1, RESP_RDY_TIMEOUT, 0);
            if (bus_info->error_flag) {
                bus_info->error_flag = 0;
                HAL_I3C_ERROR("ERR %d, line:%d", I3C_ERR_TRANSFER, __LINE__);
                return I3C_ERR_TRANSFER;
            }

            if (s32Status == 0) {
                HAL_I3C_TRACE("RESPONSE READY STATUS INTERRUPT BIT SET");
                s32Status = hal_i3c_process_pio_resp(id, &tx_desc);
                if (s32Status < 0) {
                    HAL_I3C_ERROR("ERR %d, line:%d", s32Status, __LINE__);
                    return s32Status;
                }
            } else {
                HAL_I3C_ERROR("ERR %d, RESPONSE READY STATUS INTERRUPT BIT NOT SET, line:%d", s32Status, __LINE__);
                return s32Status;
            }
        }

        //Read DCTs
        for (i = 0; i < bus_info->priv->dev_cnt; i++) {
            HAL_I3C_TRACE("  ");
            HAL_I3C_TRACE("bus_info->u16DCTSize:0x%X", bus_info->u16DCTSize);

            //First Dword DCT Read
            i3c_reg_read32(id, I3C_HCI_DCT_OFFSET + u8IncOffset, &u32RdVal);
            HAL_I3C_TRACE("I3C_HCI_DCT_OFFSET_00B:0x%X", u32RdVal);
            bus_info->pstSlvDCT->u32PID_LO = u32RdVal;

            //Second Dword DCT Read
            i3c_reg_read32(id, I3C_HCI_DCT_OFFSET + u8IncOffset + NEXT_4B_OFFSET, &u32RdVal);
            HAL_I3C_TRACE("I3C_HCI_DCT_OFFSET_04B:0x%X", u32RdVal);
            bus_info->pstSlvDCT->u16PID_HI = u32RdVal;

            //Third Dword DCT Read
            i3c_reg_read32(id, I3C_HCI_DCT_OFFSET + u8IncOffset + (2 * NEXT_4B_OFFSET), &u32RdVal);
            HAL_I3C_TRACE("I3C_HCI_DCT_OFFSET_08B:0x%X", u32RdVal);
            bus_info->pstSlvDCT->u08DCR = u32RdVal;
            bus_info->pstSlvDCT->u08BCR = u32RdVal >> 8;

            //Fourth Dword DCT Read
            i3c_reg_read32(id, I3C_HCI_DCT_OFFSET + u8IncOffset + (3 * NEXT_4B_OFFSET), &u32RdVal);
            HAL_I3C_TRACE("I3C_HCI_DCT_OFFSET_12B:0x%X", u32RdVal);
            bus_info->pstSlvDCT->u08DynAddr = u32RdVal;

            bus_info->BCR->u1MaxSpdLimit = bus_info->pstSlvDCT->u08BCR;
            bus_info->BCR->u1IBIReqCap = bus_info->pstSlvDCT->u08BCR >> 1 ;
            bus_info->BCR->u1IBIPayload = bus_info->pstSlvDCT->u08BCR >> 2;
            bus_info->BCR->u1OfflineCap = bus_info->pstSlvDCT->u08BCR >> 3;
            bus_info->BCR->u1VirtualDev = bus_info->pstSlvDCT->u08BCR >> 4;
            bus_info->BCR->u1AdvancedCap = bus_info->pstSlvDCT->u08BCR >> 5;
            bus_info->BCR->u2DevRole = bus_info->pstSlvDCT->u08BCR >> 6;
            HAL_I3C_TRACE("------------------------------");
            HAL_I3C_TRACE("BCR");
            HAL_I3C_TRACE("BridgeDev    :0x%X", bus_info->BCR->u1VirtualDev);
            HAL_I3C_TRACE("IBIPayload   :0x%X", bus_info->BCR->u1IBIPayload);
            HAL_I3C_TRACE("IBIReqCap    :0x%X", bus_info->BCR->u1IBIReqCap);
            HAL_I3C_TRACE("MaxSpdLimit  :0x%X", bus_info->BCR->u1MaxSpdLimit);
            HAL_I3C_TRACE("OfflineCap   :0x%X", bus_info->BCR->u1OfflineCap);
            HAL_I3C_TRACE("SDRHDRCap    :0x%X", bus_info->BCR->u1AdvancedCap);
            HAL_I3C_TRACE("DevRole      :0x%X", bus_info->BCR->u2DevRole);
            HAL_I3C_TRACE("------------------------------");
            HAL_I3C_TRACE("DCT");
            HAL_I3C_TRACE("DynAddr      :0x%X", bus_info->pstSlvDCT->u08DynAddr);
            HAL_I3C_TRACE("DCR          :0x%X", bus_info->pstSlvDCT->u08DCR);
            HAL_I3C_TRACE("BCR          :0x%X", bus_info->pstSlvDCT->u08BCR);
            HAL_I3C_TRACE("PID_HI       :0x%X", bus_info->pstSlvDCT->u16PID_HI);
            HAL_I3C_TRACE("PID_LO       :0x%X", bus_info->pstSlvDCT->u32PID_LO);
            HAL_I3C_TRACE("------------------------------");
            u8IncOffset += 0x10;
            bus_info->pstSlvDCT++;
            bus_info->BCR++;
        }
        bus_info->pstSlvDCT = pstSlvDCT;
        bus_info->BCR = ptrBCR;
    } else {
        for (i = 0; i < bus_info->priv->dev_cnt; i++) {
            s32Status = hal_i3c_set_da_sa_cmd(id, i);
            if (s32Status < 0) {
                HAL_I3C_ERROR("ERR, set da sa cmd err:%d", s32Status);
                return I3C_ERR_INVALID_VALUE;
            }
        }
    }

    return bus_info->priv->dev_cnt;
}

/*********************************************************************
Argument     : None
Return Value : 0 - Success
               Others - FAILURE
Description  : This API initializes the I3C Master Controller and populates all the structures
               and initializes all sturctures.
*********************************************************************/
static int32_t hal_i3c_hci_init(enum HAL_I3C_ID_T id)
{
    enum HAL_I3C_ERR ret;
    uint32_t u32RdVal;
    struct HAL_I3C_BUS_CTRL_INFO *pstbusInfo;
    struct HAL_I3C_HOST_T *host = &i3c_host[id];

    pstbusInfo = &host->bus_ctrl_info;
    memset(pstbusInfo, 0, sizeof(struct HAL_I3C_BUS_CTRL_INFO));
    pstbusInfo->priv = host;
    pstbusInfo->dynamic_addr = 0x07;
    pstbusInfo->dat_update_en = 0x0;

    HAL_I3C_TRACE(" ");
    HAL_I3C_TRACE("==============================");
    HAL_I3C_TRACE("I3C Host Controller Initialization");
    HAL_I3C_TRACE("ip base:0x%X", host->base);
    HAL_I3C_TRACE("==============================");

    //HCI Version
    i3c_reg_read32(id, I3C_HCI_VERSION, &u32RdVal);
    HAL_I3C_TRACE("I3C_HC_VERSION is: %X.%x rev %X (0x%X)", ((u32RdVal >> 8) & 0xf), ((u32RdVal >> 4) & 0xf), ((u32RdVal) & 0xf), u32RdVal);
    HAL_I3C_TRACE("------------------------------");
    //Base offsets
    hal_i3c_print_base_offset(id);
    HAL_I3C_TRACE("------------------------------");

    //Host Controller Capabilities
    i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32RdVal);
    HAL_I3C_TRACE("I3C_HCI_HC_CTRL-before   :0x%X", u32RdVal);
    if (host->inc_broadcast_addr) {
        u32RdVal |= IBA_INCLUDE_MASK;
    }
    if (host->use_dma) {
        u32RdVal &= ~MODE_SELECTOR_PIO_MASK;
    }
    if (host->i2c_dev_present) {
        u32RdVal |= I2C_SLAVE_PRE_MASK;
    }
    i3c_reg_write32(id, I3C_HCI_HC_CTRL, u32RdVal);
    i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32RdVal);
    HAL_I3C_TRACE("I3C_HCI_HC_CTRL-after    :0x%X", u32RdVal);

    i3c_reg_read32(id, I3C_HCI_HC_CAPS, &u32RdVal);
    HAL_I3C_TRACE("I3C_HCI_HC_CAPS          :0x%X", u32RdVal);
    if (u32RdVal & COMBO_CMD_EN_MASK)
        pstbusInfo->combo_cmd_en = 1;
    if (u32RdVal & AUTO_CMD_EN_MASK)
        pstbusInfo->auto_cmd_en = 1;
    if (u32RdVal & HDR_DDR_EN_MASK)
        pstbusInfo->hdr_ddr_en = 1;
    if (u32RdVal & CMD_CCC_DEFBYTE_EN_MASK)
        pstbusInfo->cmd_ccc_def_byte_en = 1;

    //capability print
    HAL_I3C_TRACE(" ");
    HAL_I3C_TRACE("HOST CONTROLLER CAPABILTY");
    HAL_I3C_TRACE("COMBO_COMMAND            :0x%X", (u32RdVal & COMBO_CMD_EN_MASK) >> 2);           //2nd bit
    HAL_I3C_TRACE("AUTO_COMMAND             :0x%X", (u32RdVal & AUTO_CMD_EN_MASK) >> 3);            //3nd bit
    HAL_I3C_TRACE("HDR_DDR_EN               :0x%X", (u32RdVal & HDR_DDR_EN_MASK) >> 6);             //6nd bit
    HAL_I3C_TRACE("CMD_CCC_DEFBYTE          :0x%X", (u32RdVal & CMD_CCC_DEFBYTE_EN_MASK) >> 10);    //10nd bit
    HAL_I3C_TRACE("------------------------------");

    if (host->use_dma) {
        ret = hal_i3c_populate_dma_rings(id, pstbusInfo);
        if (ret < 0) {
            HAL_I3C_ERROR("ERR %d, line:%d", ret, __LINE__);
            return ret;
        }
    } else {
        ret = hal_i3c_populate_pio_queue(id, pstbusInfo);
        if (ret < 0) {
            HAL_I3C_ERROR("ERR %d, line:%d", ret, __LINE__);
            return ret;
        }
        ret = hal_i3c_pio_thld_init(id);
        if (ret < 0) {
            HAL_I3C_ERROR("ERR %d, line:%d", ret, __LINE__);
            return ret;
        }
    }

    //Enable Host controller
    i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32RdVal);
    i3c_reg_write32(id, I3C_HCI_HC_CTRL, HCI_ENABLE | u32RdVal);
    i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32RdVal);
    HAL_I3C_TRACE("  ");
    HAL_I3C_TRACE("I3C_HCI_HC_CTRL-final:0x%X", u32RdVal);

    i3c_reg_write32(id, I3C_HCI_INTR_EN, INTR_STS_EN_HC_INTRNL_ERR);
    i3c_reg_write32(id, I3C_HCI_INT_SIG_EN, INTR_SIG_EN_HC_INTRNL_ERR);

    return I3C_ERR_NONE;
}

/*********************************************************************
Argument     : None
Return Value : None
Description  : This API deinitializes the host controller
*********************************************************************/
static void hal_i3c_hci_deinit(enum HAL_I3C_ID_T id)
{
    uint32_t u32RdVal;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    HAL_I3C_TRACE(" ");
    //Disable Host controller
    i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32RdVal);
    i3c_reg_write32(id, I3C_HCI_HC_CTRL, HCI_DISABLE & u32RdVal);
    i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32RdVal);
    HAL_I3C_TRACE("HCI Host controller Control Register :0x%X", u32RdVal);
    i3c_reg_write32(id, I3C_HCI_RESET_CTRL, SW_RST);

    bus_info->error_flag = 0;
    HAL_I3C_TRACE(" ");
}

/*********************************************************************
Description : This API is used to define a bottom half technique,
              called to handle IBI interrupts in a Work Queue
*********************************************************************/
static void i3c_ibi_handler(enum HAL_I3C_ID_T id)
{
    uint32_t u32Devindex = 0;
    uint32_t u32PIORdVal = 0;
    uint32_t u32IBIRdVal, u32IntrSigEn, ibidataindwords, i;
    int32_t s32Status;
    struct HAL_I3C_IBISTS_DESC *pstIBIStsDesc;
    struct HAL_I3C_IBISTS_DESC ibi_status_desc;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    pstIBIStsDesc = &ibi_status_desc;
    memset(&ibi_status_desc, 0, sizeof(struct HAL_I3C_IBISTS_DESC));
    HAL_I3C_TRACE("IBI Thresold Status interrupt Bit Set");

    i3c_reg_read32(id, I3C_HCI_PIO_IBI_PORT, &u32IBIRdVal);
    memcpy(pstIBIStsDesc, &u32IBIRdVal, sizeof(u32IBIRdVal));
    HAL_I3C_TRACE("IBI Status:0x%X", u32IBIRdVal);

    HAL_I3C_TRACE("IBISts         :0x%X", pstIBIStsDesc->u01IBISts);
    HAL_I3C_TRACE("Status type    :0x%X", pstIBIStsDesc->u01StsType);
    HAL_I3C_TRACE("Error          :0x%X", pstIBIStsDesc->u01Error);
    HAL_I3C_TRACE("HwContext      :0x%X", pstIBIStsDesc->u03HwContext);
    HAL_I3C_TRACE("TimestampMarker:0x%X", pstIBIStsDesc->u01TimestampMarker);
    HAL_I3C_TRACE("LastSts        :0x%X", pstIBIStsDesc->u01LastSts);
    HAL_I3C_TRACE("Chunks         :0x%X", pstIBIStsDesc->u08Chunks);
    HAL_I3C_TRACE("IBIId          :0x%X", pstIBIStsDesc->u08IBIId);
    HAL_I3C_TRACE("DataLen        :0x%X", pstIBIStsDesc->u08DataLen);

    if (pstIBIStsDesc->u08DataLen > 0) {
        ibidataindwords = pstIBIStsDesc->u08DataLen / 4;
        if (pstIBIStsDesc->u08DataLen % 4)
            ibidataindwords++;

        for (i = 0; i < ibidataindwords; i++) {
            i3c_reg_read32(id, I3C_HCI_PIO_IBI_PORT, &u32IBIRdVal);
            HAL_I3C_TRACE("IBI DATA:0x%X", u32IBIRdVal);
        }
    }

    if (pstIBIStsDesc->u08IBIId == 0x2) {
        if (bus_info->dat_update_en == 0) {
            hal_i3c_populate_dev_addr_table(id, bus_info); // need to Populate DAT One time only, after that we can update it
            bus_info->dat_update_en = 1;
        }

        s32Status = hal_i3c_set_da_sa_cmd(id, u32Devindex);
        if (s32Status < 0) {
            HAL_I3C_TRACE("SETDASA FAILED");
        } else {
            HAL_I3C_TRACE("SETDASA PASSED");
            u32Devindex++;
        }
    }

    i3c_reg_read32(id, I3C_HCI_PIO_INTR_STATUS, &u32PIORdVal);
    if (u32PIORdVal & INTR_IBI_THLD_STS) {}
    else {
        i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32IntrSigEn);
        i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, (INTR_IBI_THLD_STS) | u32IntrSigEn);
    }
}

/*********************************************************************
Description  : This API is used to define the interrupt request
               handler for the I3C Master controller.
*********************************************************************/
SRAM_TEXT_LOC static void i3c_pio_irq_hander(enum HAL_I3C_ID_T id)
{
    uint32_t u32IntrSts, u32PIOIntrSts, u32value, u32errsts;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    i3c_reg_read32(id, I3C_HCI_INTR_STATUS, &u32IntrSts);
    i3c_reg_read32(id, I3C_HCI_PIO_INTR_STATUS, &u32PIOIntrSts);
    HAL_I3C_TRACE("I3C_HCI_INTR_STATUS      :0x%X", u32IntrSts);
    HAL_I3C_TRACE("I3C_HCI_PIO_INTR_STATUS  :0x%X", u32PIOIntrSts);

    if (!u32PIOIntrSts) {
        HAL_I3C_ERROR("ERR, %s:%d", __func__, __LINE__);
        goto pio_irq_done;
    }

    if (u32PIOIntrSts == 0xffffffff) {
        HAL_I3C_ERROR("ERR, %s:%d", __func__, __LINE__);
        goto pio_irq_done;
    }

    if (u32IntrSts & INTR_HC_INTRNL_ERR_STS) { //10thbit, non recoverable error
        HAL_I3C_ERROR("ERR, HC INTERNAL ERROR INTERRUPT SET");
        bus_info->error_flag = 1;

        i3c_reg_write32(id, I3C_HCI_INTR_STATUS, INTR_HC_INTRNL_ERR_STS);
        i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32value);
        i3c_reg_write32(id, I3C_HCI_HC_CTRL, u32value & (HCI_DISABLE & HCI_DISABLE_MASK));//HC Disable and Enable - Controller Reset
    }

    if (u32PIOIntrSts & INTR_TX_ERR_STS) { //9th bit
        HAL_I3C_ERROR("ERR, TRANSFER ERROR INTERRUPT SET");
        bus_info->error_flag = 1;

        i3c_reg_read32(id, I3C_HCI_PIO_RSP_QUE_PORT, &u32errsts);
        u32errsts = u32errsts >> 28; //31:28 => err sts
        hal_i3c_resp_status(u32errsts); //error type examined in response structure

        i3c_reg_write32(id, I3C_HCI_RESET_CTRL, RSP_QUE_RST); //Reset Response
        i3c_reg_write32(id, I3C_HCI_PIO_INTR_STATUS, INTR_TX_ERR_STS); //Write 1 to Intr sts reg
        i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32value);
        i3c_reg_write32(id, I3C_HCI_HC_CTRL, u32value | HCI_RESUME); //Resume HCI
    }

    if (u32PIOIntrSts & INTR_TX_ABORT_STS) {
        HAL_I3C_ERROR("ERR, TRANSFER ABORT ERROR INTERRUPT SET");
        bus_info->error_flag = 1;

        i3c_reg_write32(id, I3C_HCI_PIO_INTR_STATUS, INTR_TX_ABORT_STS);
        i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32value);
        i3c_reg_write32(id, I3C_HCI_HC_CTRL, u32value & (HCI_DISABLE & HCI_DISABLE_MASK)); //HC Disable and Enable - Controller Reset
    }

    if ((irq_handler[id] != NULL) && ((u32IntrSts & INTR_HC_INTRNL_ERR_STS) || (u32PIOIntrSts & I3C_PIO_EXCEPTION_INTR))) {
        irq_handler[id](u32IntrSts, u32PIOIntrSts);
    }

    if (bus_info->error_flag == 1) {
        HAL_I3C_ERROR("ERR, line:%d, error_flag:%d", __LINE__, bus_info->error_flag);
        goto pio_irq_done;
    }

    if (u32PIOIntrSts & INTR_IBI_THLD_STS) {
        i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32value);
        i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, ~(INTR_IBI_THLD_STS) & u32value);
        i3c_ibi_handler(id);
    }

    if (u32PIOIntrSts & INTR_CMD_QUE_RDY_STS) {
        i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32value);
        i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, ~(INTR_CMD_QUE_RDY_STS) & u32value);
        bus_info->cmdque_rdy_compln = 1;
    }

    if (u32PIOIntrSts & INTR_RSP_RDY_STS) {
        i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32value);
        i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, ~(INTR_RSP_RDY_STS) & u32value);
        bus_info->rsp_rdy_compln = 1;
    }

    if (u32PIOIntrSts & INTR_RX_THLD_STS) {
        i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32value);
        i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, ~(INTR_RX_THLD_STS) & u32value);
        bus_info->rx_thld_compln = 1;
    }

    if (u32PIOIntrSts & INTR_TX_THLD_STS) {
        i3c_reg_read32(id, I3C_HCI_PIO_INT_SIG_EN, &u32value);
        i3c_reg_write32(id, I3C_HCI_PIO_INT_SIG_EN, ~(INTR_TX_THLD_STS) & u32value);
        bus_info->tx_thld_compln = 1;
    }

pio_irq_done:
    return;
}

/*********************************************************************
Argument     : u32IntrMask - Intr bit
               u8RingID ?Ring ID
Return Value : 0 - Success
               Others - FAILURE
Description  : This function checks whether interrupt is set or not in DMA mode
*********************************************************************/
SRAM_TEXT_LOC static void i3c_pio_dma_hander(enum HAL_I3C_ID_T id)
{
    uint8_t u8RingID = 0;
    uint32_t u32value;
    uint32_t u32IntrSts;
    uint32_t u32DMAIntrSts;
    uint32_t status_addr;
    uint32_t int_sig_addr;
    uint32_t int_status_addr;
    struct HAL_I3C_BUS_CTRL_INFO *bus_info = hal_i3c_get_bus_info(id);

    switch (u8RingID) {
        case RING_ID_0: {
            status_addr = RH0_RING_STATUS;
            int_sig_addr = RH0_RING_INTR_SIG_EN;
            int_status_addr = RH0_RING_INTR_ST;
            break;
        }
        case RING_ID_1: {
            status_addr = RH1_RING_STATUS;
            int_sig_addr = RH1_RING_INTR_SIG_EN;
            int_status_addr = RH1_RING_INTR_ST;
            break;
        }
        case RING_ID_2: {
            status_addr = RH2_RING_STATUS;
            int_sig_addr = RH2_RING_INTR_SIG_EN;
            int_status_addr = RH2_RING_INTR_ST;
            break;
        }
        case RING_ID_3: {
            status_addr = RH3_RING_STATUS;
            int_sig_addr = RH3_RING_INTR_SIG_EN;
            int_status_addr = RH3_RING_INTR_ST;
            break;
        }
        case RING_ID_4: {
            status_addr = RH4_RING_STATUS;
            int_sig_addr = RH4_RING_INTR_SIG_EN;
            int_status_addr = RH4_RING_INTR_ST;
            break;
        }
        case RING_ID_5: {
            status_addr = RH5_RING_STATUS;
            int_sig_addr = RH5_RING_INTR_SIG_EN;
            int_status_addr = RH5_RING_INTR_ST;
            break;
        }
        case RING_ID_6: {
            status_addr = RH6_RING_STATUS;
            int_sig_addr = RH6_RING_INTR_SIG_EN;
            int_status_addr = RH6_RING_INTR_ST;
            break;
        }
        case RING_ID_7: {
            status_addr = RH7_RING_STATUS;
            int_sig_addr = RH7_RING_INTR_SIG_EN;
            int_status_addr = RH7_RING_INTR_ST;
            break;
        }
        default: {
            HAL_I3C_ERROR("ERR, %s, ringid:%d", __func__, u8RingID);
            goto dma_irq_done;
            break;
        }
    }

    i3c_reg_read32(id, int_status_addr, &u32DMAIntrSts);
    i3c_reg_read32(id, I3C_HCI_INTR_STATUS, &u32IntrSts);
    HAL_I3C_TRACE("I3C_HCI_INTR_STATUS  :0x%X", u32IntrSts);
    HAL_I3C_TRACE("RHx_RING_INTR_ST     :0x%X", u32DMAIntrSts);

    if (!u32DMAIntrSts) {
        HAL_I3C_ERROR("ERR, %s:%d", __func__, __LINE__);
        goto dma_irq_done;
    }

    if (u32DMAIntrSts == 0xffffffff) {
        HAL_I3C_ERROR("ERR, %s:%d", __func__, __LINE__);
        goto dma_irq_done;
    }

    if (u32IntrSts & INTR_HC_INTRNL_ERR_STS) { //10thbit, non recoverable error
        HAL_I3C_ERROR("ERR, HC INTERNAL ERROR INTERRUPT SET");
        bus_info->error_flag = 1;

        i3c_reg_write32(id, I3C_HCI_INTR_STATUS, INTR_HC_INTRNL_ERR_STS);
        i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32value);
        i3c_reg_write32(id, I3C_HCI_HC_CTRL, u32value & (HCI_DISABLE & HCI_DISABLE_MASK));//HC Disable and Enable - Controller Reset
    }

    if (u32DMAIntrSts & RH_INTR_TX_ERR_STS) { //9th bit
        HAL_I3C_ERROR("ERR, TRANSFER ERROR INTERRUPT SET");
        bus_info->error_flag = 1;

        i3c_reg_read32(id, status_addr, &u32value);
        hal_i3c_ring_status(u32value);

        // i3c_reg_write32(id, I3C_HCI_RESET_CTRL, RSP_QUE_RST);//Reset Response
        i3c_reg_write32(id, int_status_addr, RH_INTR_TX_ERR_STS); //Write 1 to Intr sts reg
        i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32value);
        i3c_reg_write32(id, I3C_HCI_HC_CTRL, u32value | HCI_RESUME); //Resume HCI
    }

    if (u32DMAIntrSts & RH_INTR_TX_ABORT_STS) {
        HAL_I3C_ERROR("ERR, TRANSFER ABORT ERROR INTERRUPT SET");
        bus_info->error_flag = 1;

        i3c_reg_write32(id, int_status_addr, RH_INTR_TX_ABORT_STS);
        i3c_reg_read32(id, I3C_HCI_HC_CTRL, &u32value);
        i3c_reg_write32(id, I3C_HCI_HC_CTRL, u32value & (HCI_DISABLE & HCI_DISABLE_MASK)); //HC Disable and Enable - Controller Reset
    }

    if ((irq_handler[id] != NULL) && ((u32IntrSts & INTR_HC_INTRNL_ERR_STS) || (u32DMAIntrSts & I3C_DMA_EXCEPTION_INTR))) {
        irq_handler[id](u32IntrSts, u32DMAIntrSts);
    }

    if (bus_info->error_flag == 1) {
        HAL_I3C_ERROR("ERR, line:%d, error_flag:%d", __LINE__, bus_info->error_flag);
        goto dma_irq_done;
    }

    if (u32DMAIntrSts & RH_INTR_IBI_READY_STS) {
        i3c_reg_read32(id, int_sig_addr, &u32value);
        i3c_reg_write32(id, int_sig_addr, ~(RH_INTR_IBI_READY_STS) & u32value);
        i3c_ibi_handler(id);
    }

    if (u32DMAIntrSts & RH_INTR_TX_DONE_STS) {
        i3c_reg_read32(id, int_sig_addr, &u32value);
        i3c_reg_write32(id, int_sig_addr, ~(RH_INTR_TX_DONE_STS) & u32value);
        bus_info->dma_xfer_done = 1;
    }

    if (u32DMAIntrSts & RH_INTR_RING_OP_STS) {
        i3c_reg_read32(id, int_sig_addr, &u32value);
        i3c_reg_write32(id, int_sig_addr, ~(RH_INTR_RING_OP_STS) & u32value);
    }

    if (u32DMAIntrSts & RH_INTR_IBI_RING_FULL_STS) {
        i3c_reg_read32(id, int_sig_addr, &u32value);
        i3c_reg_write32(id, int_sig_addr, ~(RH_INTR_IBI_RING_FULL_STS) & u32value);
    }

dma_irq_done:
    return;
}

/*******************************************************************************
**************************Integrated operation interface************************
*******************************************************************************/
static int8_t hal_i3c_nvic_setup_irq(enum HAL_I3C_ID_T id, uint32_t handler)
{
    IRQn_Type irqt = i3c_desc[id].irq;

    NVIC_SetVector(irqt, handler);
    NVIC_SetPriority(irqt, IRQ_PRIORITY_NORMAL);
    return I3C_ERR_NONE;
}

static int8_t hal_i3c_nvic_enable_irq(enum HAL_I3C_ID_T id, uint8_t enabled)
{
    IRQn_Type irqt = i3c_desc[id].irq;

    if (irqt >= INVALID_IRQn) {
        HAL_I3C_ERROR("ERR, Invalid IRQ number");
        return I3C_ERR_INVALID_PARA;
    }

    if (enabled) {
        NVIC_ClearPendingIRQ(irqt);
        NVIC_EnableIRQ(irqt);
    } else {
        NVIC_DisableIRQ(irqt);
        NVIC_ClearPendingIRQ(irqt);
    }
    return I3C_ERR_NONE;
}

static void hal_i3c_irq_handler(void)
{
    enum HAL_I3C_ID_T id;
    struct HAL_I3C_HOST_T *host = NULL;

    for (id = HAL_I3C_ID_0; id < HAL_I3C_ID_NUM; id++) {
        if (NVIC_GetActive(i3c_desc[id].irq)) {
            host = &i3c_host[id];
            if (host->use_dma) {
                i3c_pio_dma_hander(id);
            } else {
                i3c_pio_irq_hander(id);
            }
        }
    }
}

void hal_i3c_irq_set_callback(enum HAL_I3C_ID_T id, HAL_I3C_IRQ_HANDLER_T i3c_cb)
{
    ASSERT(id < HAL_I3C_ID_NUM, err_invalid_id, id);
    irq_handler[id] = i3c_cb;
}

static enum HAL_I3C_ERR hal_i3c_bus_clk_cfg(enum HAL_I3C_ID_T id, const struct HAL_I3C_CONFIG_T *cfg)
{
    uint32_t src_clk = 0;
    uint32_t i3c_speed;
    enum HAL_I3C_ERR ret = I3C_ERR_NONE;
    struct HAL_I3C_HOST_T *host = NULL;
#ifndef FPGA
    enum HAL_I3C_DIV_FACTOR sdr_mode_div[5] = {SDR0_DIV, SDR1_DIV, SDR2_DIV, SDR3_DIV, SDR4_DIV};
#endif

    if (cfg && cfg->speed) {
        i3c_speed = cfg->speed;
    } else {
        i3c_speed = 12 * 1000 * 1000;
    }

#ifdef FPGA
    switch (id) {
        case HAL_I3C_ID_0:
            hal_cmu_i3c_set_freq(HAL_CMU_PERIPH_FREQ_26M);
            break;
        default:
            ret = I3C_ERR_HOST_ID;
            HAL_I3C_ERROR("%s:%d, ERR id:%d", __func__, __LINE__, id);
            goto cfg_error_out;
            break;
    }

    src_clk = hal_cmu_get_crystal_freq();
    if (src_clk < i3c_speed) {
        ret = I3C_ERR_INPUT_CFG;
        HAL_I3C_ERROR("ERR, %s:%d, i3c_speed %d > src clk %d", __func__, __LINE__, i3c_speed, src_clk);
        goto cfg_error_out;
    }
#else
    if (0) {
#ifdef PERIPH_PLL_FREQ
    } else if (i3c_speed > 2 * hal_cmu_get_crystal_freq()) {
        uint32_t div;//division is one step in place, the sdmmc ip is no longer divided(Support even division)

        div = PERIPH_PLL_FREQ / i3c_speed;
        if (PERIPH_PLL_FREQ % i3c_speed) {
            div += 1;
        }

        src_clk = PERIPH_PLL_FREQ / div;
        switch (id) {
            case HAL_I3C_ID_0:
                ret = hal_cmu_i3c_set_div(div);
                break;
#ifdef I3C1_BASE
            case HAL_I3C_ID_1:
                break;
#endif
            default:
                ret = I3C_ERR_HOST_ID;
                HAL_I3C_ERROR("%s:%d, ERR id:%d", __func__, __LINE__, id);
                goto cfg_error_out;
                break;
        }
        if (ret) {
            ret = I3C_ERR_INPUT_CFG;
            HAL_I3C_ERROR("ERR, The i3c_speed value is invalid, causing the div to be out of range, ret %d, div is %d", ret, div);
            goto cfg_error_out;
        }
        HAL_I3C_TRACE("PERIPH_PLL_FREQ is %d, the final PLL div is %d", PERIPH_PLL_FREQ, div);
#endif
    } else {
        switch (id) {
            case HAL_I3C_ID_0:
                if (i3c_speed <= hal_cmu_get_crystal_freq() / sdr_mode_div[cfg->mode]) {
                    hal_cmu_i3c_set_freq(HAL_CMU_PERIPH_FREQ_26M);
                    src_clk = hal_cmu_get_crystal_freq();
                    break;
                } else {
                    hal_cmu_i3c_set_freq(HAL_CMU_PERIPH_FREQ_52M);
                    src_clk = 2 * hal_cmu_get_crystal_freq();
                }
                break;
#ifdef I3C1_BASE
            case HAL_I3C_ID_1:
                if (i3c_speed <= hal_cmu_get_crystal_freq() / sdr_mode_div[cfg->mode]) {
                    hal_cmu_i3c_set_freq(HAL_CMU_PERIPH_FREQ_26M);
                    src_clk = hal_cmu_get_crystal_freq();
                    break;
                } else {
                    hal_cmu_i3c_set_freq(HAL_CMU_PERIPH_FREQ_52M);
                    src_clk = 2 * hal_cmu_get_crystal_freq();
                }
                break;
#endif
            default:
                ret = I3C_ERR_HOST_ID;
                HAL_I3C_ERROR("%s:%d, ERR id:%d", __func__, __LINE__, id);
                goto cfg_error_out;
                break;
        }

        if (src_clk < i3c_speed) {
            ret = I3C_ERR_INPUT_CFG;
            HAL_I3C_ERROR("ERR, %s:%d, i3c_speed %d > src clk %d", __func__, __LINE__, i3c_speed, src_clk);
            goto cfg_error_out;
        }
    }
#endif  /* end of FPGA*/
    HAL_I3C_TRACE("i3c_speed %d, src clk %d", i3c_speed, src_clk);

    host = &i3c_host[id];
    host->id    = id;
    host->src_clk = src_clk;
    host->base  = i3c_desc[id].base;
    host->bus_speed = i3c_speed;
    if (cfg) {
        host->dev_cnt = cfg->dev_cnt;
        host->use_dma = cfg->use_dma;
        host->i2c_dev_present = cfg->i2c_dev_present;
        host->inc_broadcast_addr = cfg->inc_broadcast_addr;
        host->mode = cfg->mode;
        host->static_addr = cfg->static_addr;
    } else {
        host->dev_cnt = 1;
        host->use_dma = 0;
        host->i2c_dev_present = 0;
        host->inc_broadcast_addr = 0;
        host->mode = HAL_I3C_MODE_SDR0;
        host->static_addr = NULL;
    }

cfg_error_out:
    return ret;
}

enum HAL_I3C_ERR hal_i3c_open(enum HAL_I3C_ID_T id, const struct HAL_I3C_CONFIG_T *cfg)
{
    enum HAL_I3C_ERR ret;
    struct HAL_I3C_HOST_T *host = &i3c_host[id];

    if (host->init_cnt == 0) {
        /* i3c bus clk config */
        ret = hal_i3c_bus_clk_cfg(id, cfg);
        if (ret) {
            return ret;
        }

        /* i3c iomux config */
        if (i3c_desc[id].iomux_func) {
            i3c_desc[id].iomux_func();
        }

        /* i3c host clock enable */
        if (i3c_desc[id].clk_open_func) {
            i3c_desc[id].clk_open_func();
        }

        /* i3c host controller config */
        ret = hal_i3c_hci_init(id);
        if (ret) {
            return ret;
        }

        /* NVIC config */
        hal_i3c_nvic_setup_irq(id, (uint32_t)hal_i3c_irq_handler);
        hal_i3c_nvic_enable_irq(id, 1);

        host->init_cnt = 1;
        HAL_I3C_TRACE("%s init Done", __func__);
    } else {
        HAL_I3C_TRACE("%s already Done", __func__);
    }
    HAL_I3C_TRACE("  ");

    return I3C_ERR_NONE;
}

void hal_i3c_close(enum HAL_I3C_ID_T id)
{
    struct HAL_I3C_HOST_T *host = &i3c_host[id];

    hal_i3c_nvic_enable_irq(id, 0);
    hal_i3c_hci_deinit(id);

    /* i3c host clock disable */
    if (i3c_desc[id].clk_close_func) {
        i3c_desc[id].clk_close_func();
    }

    if (host->init_cnt == 1) {
        host->init_cnt = 0;
    }
}

enum HAL_I3C_ERR hal_i3c_send(enum HAL_I3C_ID_T id, uint8_t dev_index, const void *buf, uint16_t reg_len, uint16_t data_len)
{
    enum HAL_I3C_ERR ret;
    struct HAL_I3C_RESP_DESC resp_desc;
    struct HAL_I3C_CMDDESC_RDTC cmd_desc;
    struct HAL_I3C_CMD_DATA_DESC TxCmdData;
    struct HAL_I3C_HOST_T *host = &i3c_host[id];

    HAL_I3C_TRACE("  ");
    HAL_I3C_TRACE("%s", __func__);
    memset(&resp_desc, 0, sizeof(resp_desc));
    memset(&cmd_desc, 0, sizeof(cmd_desc));
    memset(&TxCmdData, 0, sizeof(TxCmdData));
    TxCmdData.pstCmdDesc = &cmd_desc;
    TxCmdData.pstRspDesc = &resp_desc;

    TxCmdData.pstCmdDesc->u03CmdAttr       = CMD_ATRB_REGULAR_TX;
    TxCmdData.pstCmdDesc->u04TID           = hal_i3c_get_tid();
    TxCmdData.pstCmdDesc->u08CMD           = 0;//useless
    TxCmdData.pstCmdDesc->u01CP            = 0;//0=>data, 1=>CMD
    TxCmdData.pstCmdDesc->u05DevIndex      = dev_index;
    TxCmdData.pstCmdDesc->u01Short_read_err = 0;
    TxCmdData.pstCmdDesc->u01Define_byte_present = 0;
    TxCmdData.pstCmdDesc->u03Speed_mode    = host->mode;
    TxCmdData.pstCmdDesc->u01Dir_rnw       = WR_XFER;
    TxCmdData.pstCmdDesc->u01ROC           = ROC_ON;
    TxCmdData.pstCmdDesc->u01TOC           = TOC_STOP;
    TxCmdData.pstCmdDesc->u8Define_byte    = 0;
    TxCmdData.pstCmdDesc->u16Data_len      = reg_len + data_len;
    TxCmdData.pu8Buf = (uint8_t *)buf;

    if (host->use_dma) {
        ret = hal_i3c_send_cmd_data_dma(id, &TxCmdData, 1);
    } else {
        ret = hal_i3c_send_cmd_data_pio(id, &TxCmdData, 1);
    }
    if (ret < 0) {
        HAL_I3C_ERROR("ERR %d, line:%d", ret, __LINE__);
    }

    return ret;
}

enum HAL_I3C_ERR hal_i3c_recv(enum HAL_I3C_ID_T id, uint8_t dev_index, uint8_t *buf, uint16_t reg_len, uint16_t data_len)
{
    enum HAL_I3C_ERR ret;
    struct HAL_I3C_RESP_DESC resp_desc;
    struct HAL_I3C_CMDDESC_RDTC cmd_desc;
    struct HAL_I3C_CMD_DATA_DESC TxCmdData;
    struct HAL_I3C_HOST_T *host = &i3c_host[id];

    HAL_I3C_TRACE("  ");
    HAL_I3C_TRACE("%s", __func__);
    memset(&resp_desc, 0, sizeof(resp_desc));
    memset(&cmd_desc, 0, sizeof(cmd_desc));
    memset(&TxCmdData, 0, sizeof(TxCmdData));
    TxCmdData.pstCmdDesc = &cmd_desc;
    TxCmdData.pstRspDesc = &resp_desc;
    memcpy(aligned_recv_buf, buf, data_len);

    TxCmdData.pstCmdDesc->u03CmdAttr       = CMD_ATRB_REGULAR_TX;
    TxCmdData.pstCmdDesc->u04TID           = hal_i3c_get_tid();
    TxCmdData.pstCmdDesc->u08CMD           = 0;//useless
    TxCmdData.pstCmdDesc->u01CP            = 0;//0=>data, 1=>CMD
    TxCmdData.pstCmdDesc->u05DevIndex      = dev_index;
    TxCmdData.pstCmdDesc->u01Short_read_err = 0;
    TxCmdData.pstCmdDesc->u01Define_byte_present = 0;
    TxCmdData.pstCmdDesc->u03Speed_mode    = host->mode;
    TxCmdData.pstCmdDesc->u01Dir_rnw       = WR_XFER;
    TxCmdData.pstCmdDesc->u01ROC           = ROC_ON;
    TxCmdData.pstCmdDesc->u01TOC           = TOC_STOP;
    TxCmdData.pstCmdDesc->u8Define_byte    = 0;
    TxCmdData.pstCmdDesc->u16Data_len      = reg_len;
    TxCmdData.pu8Buf = aligned_recv_buf;

    if (host->use_dma) {
        ret = hal_i3c_send_cmd_data_dma(id, &TxCmdData, 1);
    } else {
        ret = hal_i3c_send_cmd_data_pio(id, &TxCmdData, 1);
    }
    if (ret < 0) {
        HAL_I3C_ERROR("ERR %d, line:%d", ret, __LINE__);
        return ret;
    }

    TxCmdData.pstCmdDesc->u01Dir_rnw       = RD_XFER;
    TxCmdData.pstCmdDesc->u01TOC           = TOC_STOP;
    TxCmdData.pstCmdDesc->u16Data_len      = data_len;
    TxCmdData.pu8Buf = aligned_recv_buf;

    if (host->use_dma) {
        ret = hal_i3c_send_cmd_data_dma(id, &TxCmdData, 1);
    } else {
        ret = hal_i3c_send_cmd_data_pio(id, &TxCmdData, 1);
    }
    if (ret < 0) {
        HAL_I3C_ERROR("ERR %d, line:%d", ret, __LINE__);
    }

    memcpy(buf, aligned_recv_buf, data_len);

    return ret;
}

#endif

