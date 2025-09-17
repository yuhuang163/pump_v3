#include "plat_types.h"
#include "plat_addr_map.h"
#include "cmsis_nvic.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_i2c.h"
#include "hal_i2cip.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "hal_gpio_capspi.h"
#include "hal_iomux.h"

static struct HAL_GPIO_CAPSPI_CONFIG_T gpio_spi_cfg = {
    .clk = HAL_IOMUX_PIN_P1_0,
    .cs = HAL_IOMUX_PIN_P1_1,
    .tx = HAL_IOMUX_PIN_P1_2,
    .rx = HAL_IOMUX_PIN_P1_3,
    .speed = 0,
    .isOpen = 0,
};


__STATIC_FORCEINLINE void GPIO_InitIO(uint8_t port, uint8_t direction, uint8_t val_for_out)
{
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)port, (enum HAL_GPIO_DIR_T)direction, val_for_out);
}

__STATIC_FORCEINLINE void GPIO_WriteIO(uint8_t port, uint8_t data)
{
    if(data){
        hal_gpio_pin_set((enum HAL_GPIO_PIN_T)port);
    }else{
        hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)port);
    }
}
__STATIC_FORCEINLINE uint8_t GPIO_ReadIO(uint8_t port)
{
    uint8_t level = 0;
    level = hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)port);
    return level;
}

int hal_gpio_capspi_initialize(struct HAL_GPIO_CAPSPI_CONFIG_T *cfg)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP hal_gpio_capspi_iomux_cfg =
    {HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}; 
    
    cfg = &gpio_spi_cfg;

    // Set the GPIO pin to output status
    hal_gpio_capspi_iomux_cfg.pin = cfg->clk;
    hal_iomux_init(&hal_gpio_capspi_iomux_cfg, 1);
    GPIO_InitIO(cfg->clk, 1, 0);

    hal_gpio_capspi_iomux_cfg.pin = cfg->tx;
    hal_iomux_init(&hal_gpio_capspi_iomux_cfg, 1);
    GPIO_InitIO(cfg->tx, 1, 1);

    hal_gpio_capspi_iomux_cfg.pin = cfg->cs;
    hal_iomux_init(&hal_gpio_capspi_iomux_cfg, 1);
    GPIO_InitIO(cfg->cs, 1, 1);

    hal_gpio_capspi_iomux_cfg.pin = cfg->rx;
    hal_iomux_init(&hal_gpio_capspi_iomux_cfg, 1);
    GPIO_InitIO(cfg->rx, 0, 1);

    return 0;
}

uint8_t hal_gpio_capspi_send(struct HAL_GPIO_CAPSPI_CONFIG_T *cfg, uint32_t data) /* return 0 --> ack */
{
    uint32_t i;
    cfg = &gpio_spi_cfg; 
    //TRACE(0, "data = %x", data);

	//add cs L
    GPIO_WriteIO(cfg->cs,0);

    HAL_GPIO_I2C_DELAY(DURATION_LOW);
    for(i=24; (i>=0)&&(i<=24); i--)
    {
        GPIO_WriteIO(cfg->clk,0); /* low */
        HAL_GPIO_I2C_DELAY(DURATION_LOW);
        GPIO_WriteIO(cfg->tx,((data>>i)&0x01));
        HAL_GPIO_I2C_DELAY(DURATION_LOW/2);
        GPIO_WriteIO(cfg->clk, 1); /* high */
        HAL_GPIO_I2C_DELAY(DURATION_HIGH);
    }
    GPIO_WriteIO(cfg->clk,0); /* low */
	HAL_GPIO_I2C_DELAY(DURATION_LOW);

	//add cs H
    GPIO_WriteIO(cfg->cs,1);
    return 0;
}

uint8_t hal_gpio_capspi_recv(struct HAL_GPIO_CAPSPI_CONFIG_T *cfg, uint32_t cmd, uint32_t *data) /* return 0 --> ack */
{
    uint32_t i;
    cfg = &gpio_spi_cfg; 
    uint32_t dataCache = 0;

	//add cs L
    GPIO_WriteIO(cfg->cs,0);

    HAL_GPIO_I2C_DELAY(DURATION_LOW);
    for(i=24; (i>=16)&&(i<=24); i--)
    {
        GPIO_WriteIO(cfg->clk,0); /* low */
        HAL_GPIO_I2C_DELAY(DURATION_LOW);
        GPIO_WriteIO(cfg->tx,((cmd>>i)&0x01));
        HAL_GPIO_I2C_DELAY(DURATION_LOW/2);
        GPIO_WriteIO(cfg->clk, 1); /* high */
        HAL_GPIO_I2C_DELAY(DURATION_HIGH);
    }

    for(i=15; (i>=0)&&(i<=15); i--)
    {
        GPIO_WriteIO(cfg->clk,0); /* low */
        HAL_GPIO_I2C_DELAY(DURATION_LOW);
        GPIO_WriteIO(cfg->tx,((cmd>>i)&0x01));
        HAL_GPIO_I2C_DELAY(DURATION_LOW/2);
        GPIO_WriteIO(cfg->clk, 1); /* high */
        HAL_GPIO_I2C_DELAY(DURATION_HIGH/2);
        dataCache |= (GPIO_ReadIO(cfg->rx )<<i);
        HAL_GPIO_I2C_DELAY(DURATION_HIGH/2);
    }
    GPIO_WriteIO(cfg->clk,0); /* low */
	HAL_GPIO_I2C_DELAY(DURATION_LOW);

	//add cs H
    GPIO_WriteIO(cfg->cs,1);
    *data = dataCache;
    //TRACE(0, "dataCache = %04x", dataCache);
    return 0;
}


