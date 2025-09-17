#ifdef TEST_SENSOR_ENGINE

#include "string.h"
#include "hal_spi.h"
#include "hal_i2c.h"
#include "hal_gpio.h"
#include "hal_sleep.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "hal_sensor_eng_v2.h"
#include <stdint.h>
/*      ----------------- I2C SPI CFG ---------------------------          */

#define     BES1501_I2C_ADDRESS                 (0x27)
#define     BES1501_WHOAMI                      (0x00)
#define     LIS3DH_I2C_ADDRESS_L                ((0x30 & 0xFE)>>1)

#define     LIS3DSH_INFO1                       (0x80 | 0x0D)
#define     LIS3DSH_INFO2                       (0x80 | 0x0E)
#define     LIS3DSH_WHO_AM_I                    (0x80 | 0x0F)
#define     LIS3DH_WHO_AM_I                     (0x80 | 0x0F)


#define     SENS_ENG_I2C_TEST
#define     I2C_CLK_FREQ                        (1 * 100 * 1000)
#define     I2C_SLAVE_ADDR                      LIS3DH_I2C_ADDRESS_L

// #define     SENS_ENG_SPI_TEST
#define     SPI_CLK_FREQ                        (6 * 100 * 1000)

/*      ----------------- SENSOR ENGINE CFG ---------------------------          */

#define     TEST_MODE_PERIODIC_TIMER
#define     SENSOR_ENGINE_DATA_LOG
#define     SENSOR_ENG_MASTER_ID                HAL_SENSOR_ENGINE_MASTER_0
#define     SENSOR_TRIGGER_TIMER_MS             1000
#define     SENS_ENG_RX_BUF_SIZE                100

#define     SENSOR0_TRIGGER_GPIO                HAL_GPIO_PIN_P0_0
#define     SENSOR1_TRIGGER_GPIO                HAL_GPIO_PIN_P0_1
#define     SENSOR2_TRIGGER_GPIO                HAL_GPIO_PIN_P0_2
#define     SENSOR3_TRIGGER_GPIO                HAL_GPIO_PIN_P0_3
#define     SENSOR4_TRIGGER_GPIO                HAL_GPIO_PIN_P1_4
#define     SENSOR5_TRIGGER_GPIO                HAL_GPIO_PIN_P1_5
#define     SENSOR6_TRIGGER_GPIO                HAL_GPIO_PIN_P1_6
#define     SENSOR7_TRIGGER_GPIO                HAL_GPIO_PIN_P1_7

#if 1
    #define SENSOR_ENGINE_TRACE(n, s, ...)      TRACE(n, s, ##__VA_ARGS__)
    #define SENSOR_ENGINE_DUMP8(n, s, ...)      DUMP8(n, s, ##__VA_ARGS__)
#else
    #define SENSOR_ENGINE_TRACE(n, s, ...)
    #define SENSOR_ENGINE_DUMP8(n, s, ...)
#endif

static struct HAL_SENSOR_ENGINE_CFG_T sensor_cfg[HAL_SENSOR_ENGINE_MASTER_QTY][HAL_SENSOR_ENGINE_ID_QTY];

/*      ----------------- CallBack Function ---------------------------          */

POSSIBLY_UNUSED static void hal_sensor_eng_external_input_pin_irqhandler(enum HAL_GPIO_PIN_T pin)
{
#ifdef SENSOR_ENGINE_DATA_LOG
    POSSIBLY_UNUSED bool gpio_lvl = 0;
    gpio_lvl = hal_gpio_pin_get_val(pin);

    SENSOR_ENGINE_TRACE(2, "hal_sensor_eng_external_irqhandler io=%d val=%d", pin, gpio_lvl);
#endif
}

static void sensor_test_handler(enum HAL_SENSOR_ENGINE_MASTER_T master, enum HAL_SENSOR_ENGINE_ID_T id, enum HAL_SENSOR_ENGINE_DEVICE_T device, const uint8_t *buf, uint32_t len,
                                      enum HAL_SENSOR_ENGINE_READ_WRITE_T read_write)
{
    if (read_write == HAL_SENSOR_ENGINE_WRITE) {
        SENSOR_ENGINE_TRACE(5, "%s: master=%d sensor enging id=%d device=%d ***write complete***", __FUNCTION__, master, id, device);
    } else {
#ifdef SENSOR_ENGINE_DATA_LOG
        SENSOR_ENGINE_TRACE(0, "%s: master=%d sensor enging id=%d device=%d", __FUNCTION__, master, id, device);
        SENSOR_ENGINE_TRACE(TR_ATTR_NO_LF, "array pointer at %p  len=%u data=", buf, len);

        uint8_t *addr;
        addr = sensor_cfg[master][id].rx_buffer_addr + sensor_cfg[master][id].rx_buffer_size;

        if (buf + len > addr) {
            SENSOR_ENGINE_DUMP8("%02x ", buf, addr - buf);
            SENSOR_ENGINE_DUMP8("%02x ", sensor_cfg[master][id].rx_buffer_addr, len - (addr - buf));
        } else {
            SENSOR_ENGINE_DUMP8("%02x ", buf, len);
        }
#endif
    }
}

/*      ----------------- SENSOR ENGINE CFG FUNCTION---------------------------          */

static uint8_t sens_eng_w_buf[HAL_SENSOR_ENGINE_ID_QTY] = {LIS3DH_WHO_AM_I, LIS3DH_WHO_AM_I};
static uint8_t sens_eng_r_buf[HAL_SENSOR_ENGINE_ID_QTY][SENS_ENG_RX_BUF_SIZE];
static int sensor_eng_cfg(enum HAL_SENSOR_ENGINE_MASTER_T master, enum HAL_SENSOR_ENGINE_TRIGGER_T trigger_type)
{
    int ret;
    int i;
#ifdef SENS_ENG_I2C_TEST
    SENSOR_ENGINE_TRACE(0, "------i2c init started------");

    struct HAL_I2C_CONFIG_T cfg_i2c;
    enum HAL_I2C_ID_T i2c_id;

    for (i = 0; i < HAL_I2C_ID_NUM; i++) {
        memset(&cfg_i2c, 0, sizeof(cfg_i2c));
        i2c_id = (enum HAL_I2C_ID_T)i;
        cfg_i2c.mode = HAL_I2C_API_MODE_SENSOR_ENGINE;
        cfg_i2c.use_dma = 1;
        cfg_i2c.as_master = 1;
        cfg_i2c.speed = I2C_CLK_FREQ;
        ret = hal_i2c_open(i2c_id, &cfg_i2c);
        if (ret) {
            ASSERT(false, "%d,%s: i2c open failed:%d", __LINE__, __FUNCTION__, ret);
            return ret;
        }
    }

    i = 0;
    memset(&sensor_cfg[master][i], 0, sizeof(sensor_cfg[master][i]));
    sensor_cfg[master][i].id = i;
    sensor_cfg[master][i].device = HAL_SENSOR_ENGINE_DEVICE_I2C1;
    sensor_cfg[master][i].trigger_type = trigger_type;
    if (trigger_type == HAL_SENSOR_ENGINE_TRIGGER_TIMER) {
        sensor_cfg[master][i].timer_period_ms = SENSOR_TRIGGER_TIMER_MS;
        sensor_cfg[master][i].handler = sensor_test_handler;
    } else {
        sensor_cfg[master][i].trigger_gpio = SENSOR0_TRIGGER_GPIO;
        sensor_cfg[master][i].handler = sensor_test_handler;
    }
    sensor_cfg[master][i].interrupt_count = 1;
    // sensor_cfg[master][i].data_to_vad = 0;      // no use
    sensor_cfg[master][i].i2c_device_address = I2C_SLAVE_ADDR;
    // sensor_cfg[master][i].spi_device_cs = 0;
    // sensor_cfg[master][i].spi_device_rxds = 1;
    // sensor_cfg[master][i].spi_id = 0;
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_READ;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = HAL_SENSOR_ENGINE_DATA_BYTE;
    sensor_cfg[master][i].tx_buffer_addr = &sens_eng_w_buf[i];
    sensor_cfg[master][i].rx_buffer_addr = sens_eng_r_buf[i];
    sensor_cfg[master][i].tx_buffer_size = 1;
    sensor_cfg[master][i].rx_buffer_size = SENS_ENG_RX_BUF_SIZE;
    sensor_cfg[master][i].tx_transfer_size = 1;
    sensor_cfg[master][i].rx_transfer_size = 1;
#endif

#ifdef SENS_ENG_SPI_TEST
    SENSOR_ENGINE_TRACE(0, "------spi init started------");
    struct HAL_SPI_CFG_T cfg_spi;

    memset(&cfg_spi, 0, sizeof(cfg_spi));
    cfg_spi.clk_delay_half = true;
    cfg_spi.clk_polarity = true;
    cfg_spi.skip_en = true;
    cfg_spi.slave = false;
    cfg_spi.dma_rx = true;
    cfg_spi.dma_tx = true;
    cfg_spi.rx_sep_line = false;
    cfg_spi.cs = 0;
    cfg_spi.rate = SPI_CLK_FREQ;
    cfg_spi.rx_bits = 8;
    cfg_spi.tx_bits = 8;
    cfg_spi.rx_frame_bits = 0;
    hal_iomux_set_spi();
    ret = hal_spi_open(&cfg_spi);
    if (ret) {
        ASSERT(false, "%d,%s: spi open failed:%d", __LINE__, __FUNCTION__, ret);
        return ret;
    }

    i = 1;
    memset(&sensor_cfg[master][i], 0, sizeof(sensor_cfg[master][i]));
    sensor_cfg[master][i].id = i;
    sensor_cfg[master][i].device = HAL_SENSOR_ENGINE_DEVICE_SPI;
    sensor_cfg[master][i].trigger_type = trigger_type;
    if (trigger_type == HAL_SENSOR_ENGINE_TRIGGER_TIMER) {
        sensor_cfg[master][i].timer_period_ms = SENSOR_TRIGGER_TIMER_MS;
        sensor_cfg[master][i].handler = sensor_test_handler;
    } else {
        sensor_cfg[master][i].trigger_gpio = SENSOR0_TRIGGER_GPIO;
        sensor_cfg[master][i].handler = sensor_test_handler;
    }
    sensor_cfg[master][i].interrupt_count = 1;
    // sensor_cfg[master][i].data_to_vad = 0;      // no use
    // sensor_cfg[master][i].i2c_device_address = I2C_SLAVE_ADDR;
    sensor_cfg[master][i].spi_device_cs = 0;
    sensor_cfg[master][i].spi_device_rxds = 1;
    sensor_cfg[master][i].spi_id = 1;
    sensor_cfg[master][i].read_write = HAL_SENSOR_ENGINE_READ;
    sensor_cfg[master][i].tx_data_width = HAL_SENSOR_ENGINE_DATA_HALF_WORD;
    sensor_cfg[master][i].rx_data_width = HAL_SENSOR_ENGINE_DATA_BYTE;
    sensor_cfg[master][i].tx_buffer_addr = &sens_eng_w_buf[i];
    sensor_cfg[master][i].rx_buffer_addr = sens_eng_r_buf[i];
    sensor_cfg[master][i].tx_buffer_size = 1;
    sensor_cfg[master][i].rx_buffer_size = SENS_ENG_RX_BUF_SIZE;
    sensor_cfg[master][i].tx_transfer_size = 1;
    sensor_cfg[master][i].rx_transfer_size = 1;

#endif

#ifdef  SENS_ENG_I2C_TEST
    hal_sensor_engine_cfg(master, &sensor_cfg[master][0]);
#endif

#ifdef  SENS_ENG_SPI_TEST
    hal_sensor_engine_cfg(master, &sensor_cfg[master][1]);
#endif

    hal_sensor_engine_open(master);
    SENSOR_ENGINE_TRACE(0, "----sensor engine cfg and opening has been completed----");
    SENSOR_ENGINE_TRACE(0, "---Please wait for sensor engine's completion interrupt---");
    SENSOR_ENGINE_TRACE(0, "  ");

    return 0;
}

POSSIBLY_UNUSED static int hal_sensor_eng_external_input_test(void)
{
    SENSOR_ENGINE_TRACE(0, "----gpio irq init----");
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP ext_pin[] = {
        {SENSOR0_TRIGGER_GPIO, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},

    };

    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    uint8_t i;


    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)ext_pin, sizeof(ext_pin) / sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP));
    for (i = 0; i < ARRAY_SIZE(ext_pin); i++) {
        hal_gpio_pin_set_dir(ext_pin[i].pin, HAL_GPIO_DIR_IN, 1);
    }
    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_handler = hal_sensor_eng_external_input_pin_irqhandler;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    for (i = 0; i < ARRAY_SIZE(ext_pin); i++) {
        hal_gpio_setup_irq(ext_pin[i].pin, &gpiocfg);
    }
    sensor_eng_cfg(SENSOR_ENG_MASTER_ID, HAL_SENSOR_ENGINE_TRIGGER_GPIO);

    while (1) {
        hal_sleep_enter_sleep();
    }

    return 0;
}

POSSIBLY_UNUSED static int hal_sensor_eng_periodic_timer_test(void)
{
    sensor_eng_cfg(SENSOR_ENG_MASTER_ID, HAL_SENSOR_ENGINE_TRIGGER_TIMER);
    while(1) {
        hal_sleep_enter_sleep();
    }
    return 0;
}


void test_sensor_engine(void)
{
    SENSOR_ENGINE_TRACE(0, "   ");
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
    SENSOR_ENGINE_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SENSOR_ENGINE_TRACE(0, "------sensor engine v2 test started------");
    SENSOR_ENGINE_TRACE(1, "CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SENSOR_ENGINE_TRACE(0, "   ");

#ifdef TEST_MODE_PERIODIC_TIMER
    SENSOR_ENGINE_TRACE(0, "------periodic timer mode------");
    hal_sensor_eng_periodic_timer_test();
#else
    SENSOR_ENGINE_TRACE(0, "------external input mode------");
    hal_sensor_eng_external_input_test();
#endif
}
#endif
