#ifndef __HAL_GPIO_CAPSPI_H__
#define __HAL_GPIO_CAPSPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_gpio.h"

#define DURATION_HIGH       900
#define DURATION_LOW        600
#define HAL_GPIO_I2C_DELAY(duration)    hal_sys_timer_delay_ns(duration);

struct HAL_GPIO_CAPSPI_CONFIG_T {
    enum HAL_GPIO_PIN_T clk;
    enum HAL_GPIO_PIN_T cs;
    enum HAL_GPIO_PIN_T tx;
    enum HAL_GPIO_PIN_T rx;
    uint32_t speed;
    uint8_t isOpen;  // initial value is 0
};

int hal_gpio_capspi_initialize(struct HAL_GPIO_CAPSPI_CONFIG_T *cfg);
uint8_t hal_gpio_capspi_send(struct HAL_GPIO_CAPSPI_CONFIG_T *cfg, uint32_t data);
uint8_t hal_gpio_capspi_recv(struct HAL_GPIO_CAPSPI_CONFIG_T *cfg, uint32_t cmd, uint32_t *data);

#endif

