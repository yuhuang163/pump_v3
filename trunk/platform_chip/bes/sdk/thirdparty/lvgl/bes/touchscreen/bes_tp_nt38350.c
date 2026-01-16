/****************************************************************************
 * vendor/bes/boards/best1600_ep/src/bes_tp_nt38350.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <debug.h>
#include <errno.h>
#include <arch/board/board.h>
#include <nuttx/kmalloc.h>
#include <nuttx/input/nt38350.h>
#include <nuttx/ioexpander/ioexpander.h>

#include <arch/chip/bes_gpio.h>
#include "bes_i2c.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define NT38350_FREQUENCY 100000  /* For now, will boost later */
#define NT38350_I2C_ADDRESS 0x62

#ifdef CONFIG_INPUT_NT38350
#ifndef CONFIG_INPUT
#  error "NT38350 support requires CONFIG_INPUT"
#endif

#ifndef CONFIG_NT38350_I2CDEV
#  define CONFIG_NT38350_I2CDEV 1
#endif

#ifndef CONFIG_NT38350_DEVMINOR
#  define CONFIG_NT38350_DEVMINOR 0
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Ptototypes
 ****************************************************************************/

#ifndef CONFIG_NT38350_POLLMODE
static int  bes_nt38350_attach(FAR struct nt38350_config_s *config,
                                ioe_callback_t isr, FAR void *arg);
static int  bes_nt38350_detach(FAR struct nt38350_config_s *config,
                                ioe_callback_t isr);
static void bes_nt38350_enable(FAR const struct nt38350_config_s *config,
                                bool enable);
static void bes_nt38350_clear(FAR const struct nt38350_config_s *config);
#endif
static void bes_nt38350_wakeup(FAR const struct nt38350_config_s *config);
static void bes_nt38350_nreset(FAR const struct nt38350_config_s *config,
                                bool state);
static void bes_nt38350_pendown(FAR const struct nt38350_config_s *config);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bes_nt38350_attach
 *
 * Description:
 *   Attach an nt38350 interrupt handler to a GPIO interrupt
 *
 ****************************************************************************/

#ifndef CONFIG_NT38350_POLLMODE
static int  bes_nt38350_attach(FAR struct nt38350_config_s *config,
                                ioe_callback_t isr, FAR void *arg)
{
  /* Just save the handler.  We will use it when EXTI interruptsare enabled */

  if (isr)
    {
      /* Just save the address of the handler for now.  The new handler will
       * be attached when the interrupt is next enabled.
       */

      config->handler = isr;
      config->arg     = arg;
    }
  else
    {
      bes_nt38350_enable(config, false);
      config->handler = NULL;
      config->arg     = NULL;
    }

  return OK;
}

/****************************************************************************
 * Name: bes_nt38350_detach
 *
 * Description:
 *   Detach the nt38350 interrupt handler to a GPIO interrupt
 *
 ****************************************************************************/

static int  bes_nt38350_detach(FAR struct nt38350_config_s *config,
                                ioe_callback_t isr)
{
  if(isr)
    {
      config->ioe_dev->ops->ioe_detach(config->ioe_dev, isr);
      bes_nt38350_enable(config, false);
    }
}
#endif

/****************************************************************************
 * Name: bes_nt38350_enable
 *
 * Description:
 *   Enable or disable a GPIO interrupt
 *
 ****************************************************************************/

#ifndef CONFIG_NT38350_POLLMODE
static void bes_nt38350_enable(FAR const struct nt38350_config_s *config,
                                bool enable)
{
  irqstate_t flags;

  /* Attach and enable, or detach and disable.  Enabling and disabling GPIO
   * interrupts is a multi-step process so the safest thing is to keep
   * interrupts disabled during the reconfiguration.
   */

  flags = enter_critical_section();
  int irq = bes_gpiopin(config->gpio_irq_pin);
  if (enable)
    {
      /* Configure the EXTI interrupt using the SAVED handler */

      config->ioe_dev->ops->ioe_attach(config->ioe_dev, irq,
                                       config->handler, config->arg);
      bes_gpioirqenable(irq);
    }
  else
    {
      /* Configure the EXTI interrupt with a NULL handler to disable it */

     bes_gpioirqdisable(irq);
    }

  leave_critical_section(flags);
}
#endif

/****************************************************************************
 * Name: bes_nt38350_clear
 *
 * Description:
 *   Acknowledge/clear any pending GPIO interrupt
 *
 ****************************************************************************/

#ifndef CONFIG_NT38350_POLLMODE
static void bes_nt38350_clear(FAR const struct nt38350_config_s *config)
{
  irqstate_t flags;
  int irq;

  flags = enter_critical_section();

  irq = bes_gpiopin(config->gpio_irq_pin);

  bes_clrpend(irq);
  leave_critical_section(flags);

  /* Does nothing */
}
#endif

/****************************************************************************
 * Name: bes_nt38350_wakeup
 *
 * Description:
 *   Issue WAKE interrupt to nt38350 to change the nt38350 from Hibernate to
 *   Active mode.
 *
 ****************************************************************************/

static void bes_nt38350_wakeup(FAR const struct nt38350_config_s *config)
{
  /* We do not have access to the WAKE pin in the implementation */
}

/****************************************************************************
 * Name: bes_nt38350_nreset
 *
 * Description:
 *   Control the chip reset pin
 *
 ****************************************************************************/

static void bes_nt38350_nreset(FAR const struct nt38350_config_s *config,
                                bool nstate)
{
#if 0
  irqstate_t flags;

  /* We do not have access to the RESET pin in the implementation */

  flags = enter_critical_section();
  struct ioexpander_dev_s * dev = bes_io_ioexpander_initialize();
  dev->ops->ioe_writepin(dev, HAL_GPIO_PIN_P1_2, nstate);
  leave_critical_section(flags);
#endif
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bes_ts_nt38350_setup
 *
 * Description:
 *   This function is called by board-bringup logic to configure the
 *   touchscreen device.  This function will register the driver as
 *   /dev/inputN where N is the minor device number.
 *
 * Input Parameters:
 *   minor   - The input device minor number
 *
 * Returned Value:
 *   Zero is returned on success.  Otherwise, a negated errno value is
 *   returned to indicate the nature of the failure.
 *
 ****************************************************************************/

int bes_ts_nt38350_setup(FAR const char *devname)
{
  FAR struct i2c_master_s *i2c;
  FAR struct ioexpander_dev_s *ioe_dev;
  FAR struct nt38350_config_s *config;
  int ret;

  config = kmm_zalloc(sizeof(struct nt38350_config_s));
  if (config == NULL)
    {
      ierr("Failed to allocate instance\n");
      return -ENOMEM;
    }

  ioe_dev = bes_io_ioexpander_initialize();
  if (!ioe_dev)
    {
      ierr("bes io ioexpander initialize failed!\n");
      ret = -ENODEV;
      goto err;
    }

  config->address      = NT38350_I2C_ADDRESS;
  config->frequency    = NT38350_FREQUENCY;
  config->gpio_irq_pin = GPIO_TP_INT;
#ifndef CONFIG_NT38350_POLLMODE
  config->attach       = bes_nt38350_attach;
  config->enable       = bes_nt38350_enable;
  config->clear        = bes_nt38350_clear;
  config->detach       = bes_nt38350_detach;
#endif
  config->wakeup       = bes_nt38350_wakeup;
  config->nreset       = bes_nt38350_nreset;
  config->ioe_dev      = ioe_dev;

  /* Configure the nt38350 interrupt pin as an input */

  bes_configgpio(config->gpio_irq_pin);

  /* Get an instance of the I2C interface */

  i2c = bes_i2cbus_initialize(CONFIG_NT38350_I2CDEV);
  if (!i2c)
    {
      ierr("ERROR: Failed to initialize I2C bus %d\n",
           CONFIG_NT38350_I2CDEV);
      ret = -ENODEV;
      goto err;
    }

  config->i2c      = i2c;

  /* Initialize and register the I2C touchscreen device */

  ret = nt38350_register(config, devname);
  if (ret < 0)
    {
      ierr("ERROR: Failed to register nt38350 driver: %d\n", ret);
      bes_i2cbus_uninitialize(i2c);
      goto err;
    }

  return OK;
err:
  kmm_free(config);
  return ret;
}
#endif /* CONFIG_INPUT_NT38350 */

