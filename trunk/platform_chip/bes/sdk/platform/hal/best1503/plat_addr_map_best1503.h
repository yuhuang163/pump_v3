/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __PLAT_ADDR_MAP_BEST1503_H__
#define __PLAT_ADDR_MAP_BEST1503_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ROM_BUILD)
#define ROM_BASE                                0x26000000
#define ROMX_BASE                               0x04020000
#else
#define ROM_BASE                                0x22000000
#define ROMX_BASE                               0x00020000
#endif

#ifndef ROM_SIZE
#define ROM_SIZE                                0x00010000
#endif
#define ROM_EXT_SIZE                            0x00050000

#define BTH_ROM_BASE                            0x22100000
#define BTH_ROMX_BASE                           0x00100000

#ifndef BTH_ROM_SIZE
#define BTH_ROM_SIZE                            0x000A0000
#endif

#define PATCH_ENTRY_NUM                         8
#define PATCH_CTRL_BASE                         0x0008F000
#define PATCH_DATA_BASE                         0x0008F100

#define PATCH1_ENTRY_NUM                        8
#define PATCH1_CTRL_BASE                        0x001CF000
#define PATCH1_DATA_BASE                        0x001CF100

#define RAM_SECURITY_FLAG                       0x04000000

#define SHR_RAM_BLK_SIZE                        0x00040000

#define RAM0_BASE                               0x20000000
#define RAMX0_BASE                              0x00200000
#define RAM1_BASE                               0x20020000
#define RAMX1_BASE                              0x00220000
#define RAM2_BASE                               0x20040000
#define RAMX2_BASE                              0x00240000
#define RAM3_BASE                               0x20060000
#define RAMX3_BASE                              0x00260000
#define RAM4_BASE                               0x200A0000
#define RAMX4_BASE                              0x002A0000
#define RAM5_BASE                               0x200E0000
#define RAMX5_BASE                              0x002E0000
#define RAM6_BASE                               0x20120000
#define RAMX6_BASE                              0x00320000

#define RAM0_SIZE                               0x00020000
#define RAM1_SIZE                               0x00020000
#define RAM2_SIZE                               0x00020000
#define RAM3_SIZE                               0x00040000
#define RAM4_SIZE                               0x00040000
#define RAM5_SIZE                               0x00040000
#ifdef BTH_IN_ROM
#define RAM6_SIZE                               0x00030000
#else
#define RAM6_SIZE                               0x00040000
#endif

#define RAM_TOTAL_SIZE                          (RAM6_BASE + RAM6_SIZE - RAM0_BASE)

#ifdef __NuttX__
#define AP_MAILBOX_SIZE                         0x18000
#endif

#ifndef AP_MAILBOX_SIZE
#define AP_MAILBOX_SIZE                         0
#endif

#ifdef CP_IN_SAME_EE
#ifndef RAMCPX_SIZE
#define RAMCPX_SIZE                             (RAMX1_BASE - RAMX0_BASE)
#endif
#define RAMCPX_BASE                             RAMX0_BASE

#ifndef RAMCP_SIZE
#define RAMCP_SIZE                              (RAM2_BASE - RAM1_BASE)
#endif
#define RAMCP_BASE                              (RAMCPX_BASE + RAMCPX_SIZE - RAMX0_BASE + RAM0_BASE)

#define RAMCP_TOP                               (RAMCP_BASE + RAMCP_SIZE)
#elif defined(CP_AS_SUBSYS) || defined(CHIP_ROLE_CP)
#define RAMCP_SUBSYS_BASE                       RAM0_BASE
#if defined(NO_SUBSYS_RAM)
#define RAMCP_SUBSYS_SIZE                       (0)
#else
#define RAMCP_SUBSYS_SIZE                       (RAM1_BASE - RAM0_BASE)
#endif
#define CP_SUBSYS_MAILBOX_SIZE                  0x60

#if RAMCP_SUBSYS_SIZE != 0
#define CP_SUBSYS_MAILBOX_BASE                  (RAMCP_SUBSYS_BASE + RAMCP_SUBSYS_SIZE - CP_SUBSYS_MAILBOX_SIZE)
#else
#define CP_SUBSYS_MAILBOX_BASE                  (RAM6_BASE + RAM6_SIZE - CP_SUBSYS_MAILBOX_SIZE)
#endif

#define RAMCP_TOP                               (RAMCP_SUBSYS_BASE + RAMCP_SUBSYS_SIZE)

#if defined(CHIP_ROLE_CP) && defined(NO_SUBSYS_RAM) && (PSRAM_REGION_SIZE > 0)
#define RAM_NC_SIZE                             (0x2000)
#define RAM_NC_BASE                             (PSRAM_NC_REGION_BASE + PSRAM_REGION_SIZE - RAM_NC_SIZE)
#endif

#else
#define RAMCP_TOP                               RAM0_BASE
#endif


#ifdef CORE_SLEEP_POWER_DOWN

#ifdef BLE_ONLY_ENABLED
#define RAM_NORM_BASE RAM6_BASE
#define MEM_POOL_SIZE                           (RAM1_BASE - MEM_POOL_BASE)
#endif

#ifdef __BT_RAMRUN_NEW__
/* RAM0-RAM3: 640K MCU volatile memory
 * RAM4-RAM6: 768K BT share memory
 */
#error "New LDS configuration is needed"
#endif

#ifdef MEM_POOL_BASE
#if (MEM_POOL_BASE < RAMCP_TOP)
#error "Bad MEM_POOL_BASE with CORE_SLEEP_POWER_DOWN"
#endif
#else
#define MEM_POOL_BASE                           RAMCP_TOP
#endif

#ifdef MEM_POOL_SIZE
#if (MEM_POOL_BASE + MEM_POOL_SIZE > RAM_NORM_BASE)
#error "Bad MEM_POOL_SIZE with CORE_SLEEP_POWER_DOWN"
#endif
#else
#define MEM_POOL_SIZE                           0
#endif

#ifdef CHIP_ROLE_CP
#define RAM_NORM_BASE                           RAMCP_SUBSYS_BASE
#elif defined(RAM_NORM_BASE)
#if (RAM_NORM_BASE < MEM_POOL_BASE)
#error "Bad RAM_BASE with CORE_SLEEP_POWER_DOWN"
#endif
#else
#define RAM_NORM_BASE                           (MEM_POOL_BASE + MEM_POOL_SIZE)
#endif
#define RAMX_NORM_BASE                          (RAM_NORM_BASE - RAM0_BASE + RAMX0_BASE)

#else /* !CORE_SLEEP_POWER_DOWN */

#ifdef CHIP_ROLE_CP
#define RAM_NORM_BASE                           RAMCP_SUBSYS_BASE
#elif defined(RAM_NORM_BASE)
#if (RAM_NORM_BASE < RAMCP_TOP)
#error "Bad RAM_BASE"
#endif
#else
#define RAM_NORM_BASE                           RAMCP_TOP
#endif
#define RAMX_NORM_BASE                          (RAM_NORM_BASE - RAM0_BASE + RAMX0_BASE)

#ifdef __BT_RAMRUN_NEW__
#if (RAM_NORM_BASE >= RAM4_BASE)
#error "Bad RAM_BASE with BT ramrun"
#endif
#define RAM_NORM_SIZE                           (RAM4_BASE - RAM_NORM_BASE - AP_MAILBOX_SIZE)
#endif

#endif /* !CORE_SLEEP_POWER_DOWN */

#ifdef CHIP_ROLE_CP
#define RAM_NORM_SIZE                           (CP_SUBSYS_MAILBOX_BASE - RAMCP_SUBSYS_BASE)
#elif defined(RAM_NORM_SIZE)
#if (RAM_NORM_BASE + RAM_NORM_SIZE) > (RAM6_BASE + RAM6_SIZE)
#error "Bad RAM_SIZE"
#endif
#else
#define RAM_NORM_SIZE                           (RAM6_BASE + RAM6_SIZE - RAM_NORM_BASE - AP_MAILBOX_SIZE)
#endif

#if defined(ARM_CMSE) || defined(ARM_CMNS)
/*MPC: SRAM block size: 0x8000, FLASH block size 0x40000*/
#ifdef LARGE_SE_RAM
#define RAM_S_SIZE                              0x00038000
#else
#define RAM_S_SIZE                              0x00018000
#endif
#define RAM_NSC_SIZE                            0
#ifndef FLASH_S_SIZE
#define FLASH_S_SIZE                            0x00040000
#endif

#undef RAM_BASE
#undef RAMX_BASE
#undef RAM_SIZE
#undef RAMCP_BASE
#undef RAMCPX_BASE
#undef RAMCP_SIZE
#undef RAMCPX_SIZE
#undef RAM_NORM_BASE
#undef RAMX_NORM_BASE
#undef RAM_NORM_SIZE

#define RAMCPX_BASE                             (RAMX0_BASE + RAM_S_SIZE + RAM_NSC_SIZE)
#define RAMCPX_SIZE                             0x20000
#define RAMCP_BASE                              (RAM0_BASE + RAM_S_SIZE + RAM_NSC_SIZE + RAMCPX_SIZE)
#define RAMCP_SIZE                              0x20000
#define RAM_NS_BASE                             (RAMCP_BASE + RAMCP_SIZE)
#define RAMX_NS_BASE                            (RAMCPX_BASE + RAMCPX_SIZE + RAMCP_SIZE)
#define RAM_NS_SIZE                             (RAM6_BASE + RAM6_SIZE - RAM_NORM_BASE)
#define RAM_S_BASE                              (RAM_NORM_BASE)
#define RAMX_S_BASE                             (RAMX_NORM_BASE)

#if defined(ARM_CMNS)
#define RAM_NORM_BASE                           RAM_NS_BASE
#define RAMX_NORM_BASE                          RAMX_NS_BASE
#define RAM_NORM_SIZE                           RAM_NS_SIZE
#else
#if ((RAM_S_SIZE + RAM_NSC_SIZE) & (0x8000-1))
#error "RAM_S_SIZE should be 0x8000 aligned"
#endif
#if (FLASH_S_SIZE & (0x40000-1))
#error "FLASH_S_SIZE should be 0x40000 aligned"
#endif

#define RAM_NORM_BASE                           RAM0_BASE
#define RAMX_NORM_BASE                          RAMX0_BASE
#define RAM_NORM_SIZE                           RAM_S_SIZE
#ifndef NS_APP_START_OFFSET
#define NS_APP_START_OFFSET                     (FLASH_S_SIZE)
#endif
#ifndef FLASH_REGION_SIZE
#define FLASH_REGION_SIZE                       FLASH_S_SIZE
#endif
#endif
#else
#define RAM_S_BASE                              RAM_NS_TO_S(RAM_NORM_BASE)
#define RAM_S_SIZE                              RAM_NORM_SIZE
#define RAMX_S_BASE                             RAM_NS_TO_S(RAMX_NORM_BASE)

#define RAM_NS_BASE                             RAM_S_TO_NS(RAM_NORM_BASE)
#define RAM_NS_SIZE                             RAM_NORM_SIZE
#define RAMX_NS_BASE                            RAM_S_TO_NS(RAMX_NORM_BASE)
#endif /* defined(ARM_CMSE) || defined(ARM_CMNS) */

#if defined(NO_TRUSTZONE) || defined(ARM_CMNS) || \
        !(defined(ARM_CMSE) || defined(ROM_BUILD) || defined(PROGRAMMER))
#define RAM_BASE                                RAM_NS_BASE
#define RAM_SIZE                                RAM_NS_SIZE
#define RAMX_BASE                               RAMX_NS_BASE
#else
#define RAM_BASE                                RAM_S_BASE
#define RAM_SIZE                                RAM_S_SIZE
#define RAMX_BASE                               RAMX_S_BASE
#endif


#define REAL_FLASH_BASE                         0x2C000000
#define REAL_FLASH_NC_BASE                      0x28000000
#define REAL_FLASHX_BASE                        0x0C000000
#define REAL_FLASHX_NC_BASE                     0x08000000

#define REAL_FLASH1_BASE                        0x34000000
#define REAL_FLASH1_NC_BASE                     0x30000000
#define REAL_FLASH1X_BASE                       0x14000000
#define REAL_FLASH1X_NC_BASE                    0x10000000

#define PSRAM_BASE                              0x3C000000
#define PSRAM_NC_BASE                           0x38000000
#define PSRAMX_BASE                             0x1C000000
#define PSRAMX_NC_BASE                          0x18000000

#ifndef CHIP_ROLE_CP
#define ICACHE_CTRL_BASE                        0x07FFA000
#define ICACHE_SIZE                             0x00008000
#define DCACHE_CTRL_BASE                        0x07FFC000
#define DCACHE_SIZE                             0x00008000
#endif

#define CMU_BASE                                0x40000000
#define MCU_WDT_BASE                            0x40001000
#define MCU_TIMER0_BASE                         0x40002000
#define MCU_TIMER1_BASE                         0x40003000
#define MCU_TIMER2_BASE                         0x40004000

#ifndef CHIP_ROLE_CP
#define I2C0_BASE                               0x40005000
#define I2C1_BASE                               0x40006000
#define SPI_BASE                                0x40007000
#define TRNG_BASE                               0x40008000
#define ISPI_BASE                               0x40009000
#define UART5_BASE                              0x4000A000
#define UART0_BASE                              0x4000B000
#define UART1_BASE                              0x4000C000
#define UART2_BASE                              0x4000D000
#define I2S0_BASE                               0x4000F000
#define SPDIF0_BASE                             0x40010000
#define I2S1_BASE                               0x40011000
#define SEC_ENG_BASE                            0x40020000
#define SEC_CTRL_BASE                           0x40030000
#define MPC_PSRAM0_BASE                         0x40031000
#define REAL_MPC_FLASH0_BASE                    0x40032000
#define REAL_MPC_FLASH1_BASE                    0x40033000

#ifdef ALT_BOOT_FLASH
#define MPC_FLASH0_BASE                         REAL_MPC_FLASH1_BASE
#define MPC_FLASH1_BASE                         REAL_MPC_FLASH0_BASE
#else
#define MPC_FLASH0_BASE                         REAL_MPC_FLASH0_BASE
#define MPC_FLASH1_BASE                         REAL_MPC_FLASH1_BASE
#endif

#define MPC_SRAM0_BASE                          0x40034000
#define SPY_SRAM0_BASE                          0x40050020
#define MPC_SRAM1_BASE                          0x40035000
#define SPY_SRAM1_BASE                          0x40050040
#define MPC_SRAM2_BASE                          0x40036000
#define SPY_SRAM2_BASE                          0x40050060
#define MPC_SRAM3_BASE                          0x40037000
#define SPY_SRAM3_BASE                          0x40050080
#define MPC_SRAM4_BASE                          0x40038000
#define SPY_SRAM4_BASE                          0x400500A0
#define MPC_SRAM5_BASE                          0x40039000
#define SPY_SRAM5_BASE                          0x400500C0
#define MPC_SRAM6_BASE                          0x4003A000
#define SPY_SRAM6_BASE                          0x400500E0

#define PPI_BASE                                0x40040000
#define PAGE_SPY_BASE                           0x40050000
#define I2C2_BASE                               0x40060000
#define I2C4_BASE                               0x40062000
#define I2C5_BASE                               0x40063000
#define I2C6_BASE                               0x40064000
#define I2C7_BASE                               0x40065000
#define UART3_BASE                              0x40066000
#define SPIDPD_BASE                             0x40068000
#define UART4_BASE                              0x40069000
#elif defined(TRACE_CPU_IMM_OUTPUT)
#define UART0_BASE                              0x4000B000
#define UART1_BASE                              0x4000C000
#define UART2_BASE                              0x4000D000
#elif defined(CHIP_ROLE_CP)
#define SPILCD_BASE                             0x40067000
#define I2C3_BASE                               0x40061000
#define I2C0_BASE                               I2C3_BASE
#endif

#define AON_CMU_BASE                            0x40080000
#define AON_GPIO_BASE                           0x40081000
#define AON_WDT_BASE                            0x40082000
#define AON_PWM_BASE                            0x40083000
#define AON_TIMER0_BASE                         0x40084000
#define AON_PSC_BASE                            0x40085000
#define AON_IOMUX_BASE                          0x40086000
#define I2C_SLAVE_BASE                          0x40087000
#define AON_SEC_CTRL_BASE                       0x40088000
#define AON_GPIO1_BASE                          0x40089000
#define AON_GPIO2_BASE                          0x4008A000
#define AON_TIMER1_BASE                         0x4008B000
#define AON_PWM1_BASE                           0x4008C000


#if defined(CHIP_DMA_CFG_IDX) && (CHIP_DMA_CFG_IDX == 1)
#ifndef CHIP_ROLE_CP
/*ap use gpdma*/
#define GPDMA_BASE                              0x40130000
#define DMA0_BASE                               GPDMA_BASE
#else
/*cp use audma*/
#define AUDMA_BASE                              0x40120000
#define DMA0_BASE                               AUDMA_BASE
#define BTPCM_BASE                              0x4000E000
#endif/*CHIP_ROLE_CP*/
#else
#ifndef CHIP_ROLE_CP
#define AUDMA_BASE                              0x40120000
#define GPDMA_BASE                              0x40130000
#define DMA0_BASE                               AUDMA_BASE
#define DMA1_BASE                               GPDMA_BASE
#define BTPCM_BASE                              0x4000E000
#endif/*CHIP_ROLE_CP*/
#endif


#ifndef CHIP_ROLE_CP
#define REAL_FLASH1_CTRL_BASE                   0x40500000
#define SDMMC0_BASE                             0x40110000
#define REAL_FLASH_CTRL_BASE                    0x40140000
#define BTDUMP_BASE                             0x40150000
#define PSRAM_CTRL_BASE                         0x40160000
#define CRC_BASE                                0x40180000
#define SEDMA_BASE                              0x401D0000
#define CPUDUMP_BASE                            0x401E0000
#define JPEG_BASE                               0x401F0000

#define DUMPPC_TOTAL_NUM                        256

#define LCDC_BASE                               0x40200000
#define GPU_BASE                                0x40220000
#define GAMU_BASE                               0x40240000
#define GAMU_VMEM_BASE                          0x60000000
#endif

#define CODEC_BASE                              0x40300000

#ifndef BTC_ROM_RAM_BASE
#define BTC_ROM_RAM_BASE                        0xA0000000
#endif

#ifndef BTC_ROM_RAM_SIZE
#define BTC_ROM_RAM_SIZE                        0x000C0000
#endif

#define BTC_MIN_ROM_RAM_BASE                    0xA0000000
#define BT_RAM_BASE                             0xC0000000
#define BT_RAM_SIZE                             0x00010000
#define BT_EXCH_MEM_BASE                        0xD0200000
#define BT_EXCH_MEM_SIZE                        0x00010000
#define BT_UART_BASE                            0xD0300000
#define BT_CMU_BASE                             0xD0330000
#define BTC_CPUDUMP_BASE                        0xD0700000

#ifndef CP_INTR_RELAY
#if defined(CP_AS_SUBSYS) || defined(CHIP_ROLE_CP)
#define CP_SUBSYS_TIMER0_BASE                   MCU_TIMER0_BASE
#define CP_SUBSYS_TIMER1_BASE                   MCU_TIMER2_BASE
#endif

#ifdef CHIP_ROLE_CP
#define TIMER0_BASE                             CP_SUBSYS_TIMER0_BASE
#define TIMER1_BASE                             CP_SUBSYS_TIMER1_BASE
#else
#ifdef FPGA
#define TIMER0_BASE                             MCU_TIMER0_BASE
#else
#define TIMER0_BASE                             AON_TIMER0_BASE
#endif
#define TIMER1_BASE                             MCU_TIMER1_BASE
#ifndef CP_AS_SUBSYS
#define TIMER2_BASE                             MCU_TIMER2_BASE
#endif
#endif
#else
#define SLOW_FREE_TIMER_ID                      2
#if defined(CP_AS_SUBSYS) || defined(CHIP_ROLE_CP)
#define CP_SUBSYS_TIMER0_BASE                   AON_TIMER0_BASE
#define CP_SUBSYS_TIMER1_BASE                   MCU_TIMER1_BASE
#endif

#ifdef CHIP_ROLE_CP
#define SLOW_TIMER_SUB_ID_WITH_IRQ              1
#define TIMER0_BASE                             CP_SUBSYS_TIMER0_BASE
#define TIMER1_BASE                             CP_SUBSYS_TIMER1_BASE
#define TIMER2_BASE                             AON_TIMER1_BASE
#else
#define SLOW_TIMER_SUB_ID_WITH_IRQ              0
#ifdef FPGA
#define TIMER0_BASE                             MCU_TIMER0_BASE
#else
#define TIMER0_BASE                             AON_TIMER0_BASE
#endif
#define TIMER1_BASE                             MCU_TIMER1_BASE
#define TIMER2_BASE                             AON_TIMER1_BASE
#endif
#endif

#ifndef CHIP_ROLE_CP
#ifdef ALT_BOOT_FLASH
#define FLASH_BASE                              REAL_FLASH1_BASE
#define FLASH_NC_BASE                           REAL_FLASH1_NC_BASE
#define FLASHX_BASE                             REAL_FLASH1X_BASE
#define FLASHX_NC_BASE                          REAL_FLASH1X_NC_BASE

#define FLASH1_BASE                             REAL_FLASH_BASE
#define FLASH1_NC_BASE                          REAL_FLASH_NC_BASE
#define FLASH1X_BASE                            REAL_FLASHX_BASE
#define FLASH1X_NC_BASE                         REAL_FLASHX_NC_BASE

#define FLASH_CTRL_BASE                         REAL_FLASH1_CTRL_BASE
#define FLASH1_CTRL_BASE                        REAL_FLASH_CTRL_BASE
#else
#define FLASH_BASE                              REAL_FLASH_BASE
#define FLASH_NC_BASE                           REAL_FLASH_NC_BASE
#define FLASHX_BASE                             REAL_FLASHX_BASE
#define FLASHX_NC_BASE                          REAL_FLASHX_NC_BASE

#define FLASH1_BASE                             REAL_FLASH1_BASE
#define FLASH1_NC_BASE                          REAL_FLASH1_NC_BASE
#define FLASH1X_BASE                            REAL_FLASH1X_BASE
#define FLASH1X_NC_BASE                         REAL_FLASH1X_NC_BASE

#define FLASH_CTRL_BASE                         REAL_FLASH_CTRL_BASE
#define FLASH1_CTRL_BASE                        REAL_FLASH1_CTRL_BASE
#endif

#else // !CHIP_ROLE_CP

#define NO_FLASH_BASE_ACCESS
#endif // CHIP_ROLE_CP

/*resize fpga ram*/
#ifdef FPGA
#ifdef RAM_BASE
#undef RAM_BASE
#define RAM_BASE                                0x20000000
#endif

#ifdef RAMX_BASE
#undef RAMX_BASE
#define RAMX_BASE                               (RAM_BASE - 0x20000000 + 0x200000)
#endif

#ifndef FPGA_RAM_TOTAL_SIZE
#define FPGA_RAM_TOTAL_SIZE                     0xc0000
#endif

#ifdef RAM_SIZE
#undef RAM_SIZE
#define RAM_SIZE                                FPGA_RAM_TOTAL_SIZE
#endif

#ifdef RAMCP_SIZE
#undef RAMCP_SIZE
#define RAMCP_SIZE                              0
#endif
#endif

#define IOMUX_BASE                              AON_IOMUX_BASE
#define GPIO_BASE                               AON_GPIO_BASE
#define GPIO1_BASE                              AON_GPIO1_BASE
#define GPIO2_BASE                              AON_GPIO2_BASE

#define PWM_BASE                                AON_PWM_BASE
#define PWM1_BASE                               AON_PWM1_BASE
#define WDT_BASE                                AON_WDT_BASE

/* For linker scripts */
#define VECTOR_SECTION_SIZE                     380
#define REBOOT_PARAM_SECTION_SIZE               64
#define ROM_BUILD_INFO_SECTION_SIZE             40
#define ROM_EXPORT_FN_SECTION_SIZE              128
#define BT_INTESYS_MEM_OFFSET                   0x00004000

/* For module features */
#define CODEC_FREQ_CRYSTAL                      CODEC_FREQ_24M
#define CODEC_FREQ_EXTRA_DIV                    2
#define CODEC_PLL_DIV                           16
#define CODEC_CMU_DIV                           8
#define CODEC_PLAYBACK_BIT_DEPTH                24
#define CODEC_HAS_FIR
#define GPADC_CTRL_VER                          3
#define GPADC_VALUE_BITS                        16
#define GPADC_HAS_VSYS_DIV
#define GPADC_HAS_EXT_SLOPE_CAL
#define GPIO_HAS_BOTH_EDGE_IRQ
#define SEC_ENG_HAS_HASH
#define DCDC_CLOCK_CONTROL
#define PWRKEY_IRQ_IN_PMU
#ifndef AUD_SECTION_STRUCT_VERSION
#define AUD_SECTION_STRUCT_VERSION              3
#endif
#define PAGE_SPY_VER                            2
#define NO_SUBSYS_FLASH_ADDR_ACCESS
#define UARTCOM_USE_LED2

/* TODO: MCU has 64M-byte space but SENS only has 16M-byte space */
#define HAL_NORFLASH_ADDR_MASK                  0x00FFFFFF
#define RMT_IPC_API_ENABLE

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
#define NO_FLASH_S_ACCESS
#endif

#define PROGRAMMER_HAL_CHIP_INIT

/* For boot struct version */
#ifndef SECURE_BOOT_VER
#define SECURE_BOOT_VER                         5
#endif

/* For ROM export functions */
#define NO_MEMMOVE
#define NO_EXPORT_QSORT
#define NO_EXPORT_BSEARCH
#define NO_EXPORT_VSSCANF

#ifdef __cplusplus
}
#endif

#endif
