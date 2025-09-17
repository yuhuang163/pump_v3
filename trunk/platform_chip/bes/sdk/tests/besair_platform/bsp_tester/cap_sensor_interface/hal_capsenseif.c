#include "string.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "plat_types.h"
#include "cmsis.h"
#include "hal_location.h"
#include "hal_spi.h"
#include "hal_trace.h"
#include "pmu.h"
#include "hal_psc.h"
#include "hal_capsenseif.h"
#include "hal_gpio_capspi.h"

#define SENS_SPI_SENS_CS                     1
#define SENS_SPI_RF_CS                       2

#define SENS_SPI_SENS_PAGE_REG               0x00
#define SENS_SPI_RF_PAGE_REG                 0x00
#define SENS_SPI_NORMAL_PAGE_REG             0x00

#define CAPSENSE_CHIP_ID_SHIFT               (4)
#define CAPSENSE_CHIP_ID_MASK                (0xFFF << CAPSENSE_CHIP_ID_SHIFT)
#define CAPSENSE_CHIP_ID(n)                  BITFIELD_VAL(SENS_CHIP_ID, n)
#define CAPSENSE_VAL_CHIP_ID                 0x160

// SENS_SPI_ARBITRATOR_ENABLE should be defined when:
// 1) BT and MCU will access RF register at the same time; or
// 2) BT can access PMU/CAPSENSE, and BT will access RF register at the same time
//    when MCU is accessing PMU/CAPSENSE register

#ifdef SENS_SPI_ARBITRATOR_ENABLE
// Min padding OSC cycles needed: BT=0 MCU=6
// When OSC=26M and SPI=6.5M, min padding SPI cycles is BT=0 MCU=2
#define PADDING_CYCLES                  2
#else
#define PADDING_CYCLES                  0
#endif

#define CAPSENSE_READ_CMD(r)                 (((1 << 24) | (((r) & 0xFF) << 16)) << PADDING_CYCLES)
#define CAPSENSE_WRITE_CMD(r, v)             (((((r) & 0xFF) << 16) | ((v) & 0xFFFF)) << PADDING_CYCLES)
#define CAPSENSE_READ_VAL(v)                 (((v) >> PADDING_CYCLES) & 0xFFFF)
#define CAPSENSE_REG_CHIP_ID                 ITNPMU_REG(0x0A)

#define SENS_PAGE_0                      (0xA000)
#define SENS_PAGE_1                      (0xA010)
#define SENS_PAGE_2                      (0xA020)
#define SENS_PAGE_3                      (0xA030)
#define SENS_PAGE_4                      (0xA040)
#define SENS_PAGE_QTY                    5

#define SENS_SPI_REG_CS(r)                  ((r) >> 12)
#define SENS_SPI_REG_PAGE(r)                (((r) >> 8) & 0xF)
#define SENS_SPI_REG_OFFSET(r)              ((r) & 0xFF) 
 //((r) & 0xFF)

static uint8_t BOOT_BSS_LOC sens_cs;
#ifdef SENS_SPI_CHECK_RF_ACCESS
static enum HAL_CAPSENSEIF_RF_ACCESS_T BOOT_BSS_LOC rf_access;
#endif

static const struct HAL_SPI_CFG_T spi_cfg = {
    .clk_delay_half = false,
    .clk_polarity = false,
    .slave = false,
    .dma_rx = false,
    .dma_tx = false,
    .rx_sep_line = true,
    .cs = 0,
    .rate = 12000000 / 2,
    .tx_bits = 25 + PADDING_CYCLES,
    .rx_bits = 25 + PADDING_CYCLES,
    .rx_frame_bits = 0,
};

// Max page number is 4
static const uint16_t page_val[SENS_PAGE_QTY] = { SENS_PAGE_0, SENS_PAGE_1, SENS_PAGE_2, SENS_PAGE_3, SENS_PAGE_4, };

enum HAL_CAPSENSEIF_RF_ACCESS_T hal_capsenseif_rf_access_config(enum HAL_CAPSENSEIF_RF_ACCESS_T type)
{
    enum HAL_CAPSENSEIF_RF_ACCESS_T old_val = HAL_CAPSENSEIF_RF_ACCESS_ALL;

#ifdef SENS_SPI_CHECK_RF_ACCESS
    old_val = rf_access;
    rf_access = type;
#endif

    return old_val;
}

__STATIC_FORCEINLINE void hal_capsenseif_check_rf_access(uint8_t cs, uint8_t page, uint8_t reg)
{
#ifdef SENS_SPI_CHECK_RF_ACCESS
    if (cs == SENS_SPI_RF_CS) {
        ASSERT(rf_access != HAL_CAPSENSEIF_RF_ACCESS_NONE,
            "RF-SENS_SPI: page=%u reg=0x%X caller=%p",
            page, reg, __builtin_return_address(0));
        ASSERT(rf_access != HAL_CAPSENSEIF_RF_ACCESS_NO_PAGE || page == 0,
            "RF-SENS_SPI P: page=%u reg=0x%X caller=%p",
            page, reg, __builtin_return_address(0));
    }
#endif
}

static int hal_capsenseif_rawread(unsigned short reg, unsigned short *val)
{
    int ret;
    unsigned int data;
    unsigned int cmd;

    data = 0;
    cmd = CAPSENSE_READ_CMD(reg);
#if 0
    ret = hal_spilcd_recv(&cmd, &data, 4);
#else 
    hal_gpio_capspi_initialize(NULL);
    ret = hal_gpio_capspi_recv(NULL, cmd, &data);
#endif
    if (ret) {
        return ret;
    }
    *val = CAPSENSE_READ_VAL(data);
    return 0;
}

static int hal_capsenseif_rawwrite(unsigned short reg, unsigned short val)
{
    int ret;

    unsigned int cmd;
    TRACE(0, "reg = %x, val = %x", reg, val);
    cmd = CAPSENSE_WRITE_CMD(reg, val);
#if 0
    ret = hal_spilcd_send(&cmd, 4);
#else
    hal_gpio_capspi_initialize(NULL);
    ret = hal_gpio_capspi_send(NULL, cmd);
#endif
    if (ret) {
        return ret;
    }
    return 0;
}

static uint8_t hal_capsenseif_page_switch(uint8_t cs, uint8_t page)
{
    uint8_t page_reg = 0;

    page_reg = SENS_SPI_NORMAL_PAGE_REG;
    hal_capsenseif_rawwrite(page_reg, page_val[page]);

    return page_reg;
}

static void hal_capsenseif_page_restore(uint8_t page_reg)
{
    hal_capsenseif_rawwrite(page_reg, page_val[0]);
}

int hal_capsenseif_reg_read(unsigned short reg, unsigned short *val)
{
    int ret;
    uint32_t lock;
    uint8_t cs = 0;
    uint8_t page;
    uint8_t page_reg = 0;

    page = SENS_SPI_REG_PAGE(reg);
    reg = SENS_SPI_REG_OFFSET(reg);

    hal_capsenseif_check_rf_access(cs, page, reg);

    if (page >= SENS_PAGE_QTY) {
        return -1;
    }

    lock = int_lock();
    if (cs != sens_cs) {
        hal_spilcd_activate_cs(cs);
        sens_cs = cs;
    }

    if (page) {
        page_reg = hal_capsenseif_page_switch(cs, page);
    }
    ret = hal_capsenseif_rawread(reg, val);
    if (page) {
        hal_capsenseif_page_restore(page_reg);
    }
    int_unlock(lock);

    return ret;
}

int hal_capsenseif_reg_write(unsigned short reg, unsigned short val)
{
    int ret;
    uint32_t lock;
    uint8_t cs = 0;
    uint8_t page;
    uint8_t page_reg = 0;

    page = SENS_SPI_REG_PAGE(reg);
    reg = SENS_SPI_REG_OFFSET(reg);

    hal_capsenseif_check_rf_access(cs, page, reg);

    if (page >= SENS_PAGE_QTY) {
        return -1;
    }

    lock = int_lock();
    if (cs != sens_cs) {
        hal_spilcd_activate_cs(cs);
        sens_cs = cs;
    }
    if (page) {
        page_reg = hal_capsenseif_page_switch(cs, page);
    }
    ret = hal_capsenseif_rawwrite(reg, val);
    if (page) {
        hal_capsenseif_page_restore(page_reg);
    }
    int_unlock(lock);

    return ret;
}

int BOOT_TEXT_FLASH_LOC hal_capsenseif_open(void)
{
    int ret;

    hal_psc_sens_enable();
    hal_cmu_sens_clock_enable();
    hal_cmu_sens_reset_clear();
    osDelay(100);
    hal_iomux_set_spi();

    ret = hal_spilcd_open(&spi_cfg);
    if (ret) {
        return ret;
    }
    
    sens_cs = 0;
    hal_spilcd_activate_cs(sens_cs);
    
    return 0;
}











