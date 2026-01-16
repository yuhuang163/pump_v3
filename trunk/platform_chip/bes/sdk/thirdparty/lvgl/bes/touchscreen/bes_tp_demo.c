/****************************************************************************
 * vendor/bes/boards/best1600_ep/src/bes_tp_zinitix.c
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
//#include "zinitix.h"

//#include <arch/chip/bes_gpio.h>
#include "hal_i2c.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define ZINITIX_FREQUENCY 100000  /* For now, will boost later */
#define ZINITIX_I2C_ADDRESS 0x20

#if 1//def CONFIG_BES_TP_ZINITIX

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bes_zinitix_config_s
{
  xcpt_t          handler;  /* The zinitix interrupt handler */
  FAR void       *arg;      /* Interrupt handler argument */
};

/****************************************************************************
 * Private Function Ptototypes
 ****************************************************************************/

#ifndef CONFIG_ZINITIX_POLLMODE
static int  bes_zinitix_attach(FAR const struct zinitix_config_s *config,
                                xcpt_t isr, FAR void *arg);
static void bes_zinitix_enable(FAR const struct zinitix_config_s *config,
                                bool enable);
static void bes_zinitix_clear(FAR const struct zinitix_config_s *config);
#endif
static void bes_zinitix_wakeup(FAR const struct zinitix_config_s *config);
static void bes_zinitix_nreset(FAR const struct zinitix_config_s *config,
                                bool state);
static void bes_zinitix_pendown(FAR const struct zinitix_config_s *config);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct zinitix_config_s g_zinitix_config =
{
  .address   = ZINITIX_I2C_ADDRESS,
  .frequency = ZINITIX_FREQUENCY,
  .gpio_rst_pin = GPIO_TP_RST,
  .gpio_irq_pin = GPIO_TP_INT,
#ifndef CONFIG_ZINITIX_POLLMODE
  .attach    = bes_zinitix_attach,
  .enable    = bes_zinitix_enable,
  .clear     = bes_zinitix_clear,
#endif
  //.pendown    = bes_zinitix_pendown,
  .wakeup    = bes_zinitix_wakeup,
  .nreset    = bes_zinitix_nreset
};

static struct bes_zinitix_config_s g_priv_config =
{
  .handler     = NULL,
  .arg         = NULL,
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bes_zinitix_attach
 *
 * Description:
 *   Attach an zinitix interrupt handler to a GPIO interrupt
 *
 ****************************************************************************/

#ifndef CONFIG_ZINITIX_POLLMODE
static int  bes_zinitix_attach(FAR const struct zinitix_config_s *config,
                                xcpt_t isr, FAR void *arg)
{
  iinfo("Saving handler %p\n", isr);

  /* Just save the handler.  We will use it when EXTI interruptsare enabled */
  if (isr)
    {
      /* Just save the address of the handler for now.  The new handler will
       * be attached when the interrupt is next enabled.
       */

      iinfo("Attaching %p\n", isr);
      g_priv_config.handler = isr;
      g_priv_config.arg     = arg;
    }
  else
    {
      iinfo("Detaching %p\n", g_priv_config.handler);
      bes_zinitix_enable(config, false);
      g_priv_config.handler = NULL;
      g_priv_config.arg     = NULL;
      //dev->ops->ioe_detach(dev,NULL);
    }

  return OK;
}
#endif

/****************************************************************************
 * Name: bes_zinitix_enable
 *
 * Description:
 *   Enable or disable a GPIO interrupt
 *
 ****************************************************************************/

#ifndef CONFIG_ZINITIX_POLLMODE
static void bes_zinitix_enable(FAR const struct zinitix_config_s *config,
                                bool enable)
{
  irqstate_t flags;

  /* Attach and enable, or detach and disable.  Enabling and disabling GPIO
   * interrupts is a multi-step process so the safest thing is to keep
   * interrupts disabled during the reconfiguration.
   */

  flags = enter_critical_section();
  int irq = bes_gpiopin(GPIO_TP_INT);
  if (enable)
    {
      /* Configure the EXTI interrupt using the SAVED handler */
      struct ioexpander_dev_s * dev = bes_io_ioexpander_initialize();
      dev->ops->ioe_attach(dev,1<<irq, g_priv_config.handler, g_priv_config.arg);
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
 * Name: bes_zinitix_clear
 *
 * Description:
 *   Acknowledge/clear any pending GPIO interrupt
 *
 ****************************************************************************/

#ifndef CONFIG_ZINITIX_POLLMODE
static void bes_zinitix_clear(FAR const struct zinitix_config_s *config)
{
  irqstate_t flags;
  int irq;

  flags = enter_critical_section();

#ifndef CONFIG_ZINITIX_MULTIPLE
  irq = bes_gpiopin(GPIO_TP_INT);
#else
  irq = config->irq;
#endif

  bes_clrpend(irq);
  leave_critical_section(flags);

  /* Does nothing */
}
#endif

/****************************************************************************
 * Name: bes_zinitix_wakeup
 *
 * Description:
 *   Issue WAKE interrupt to zinitix to change the zinitix from Hibernate to
 *   Active mode.
 *
 ****************************************************************************/

static void bes_zinitix_wakeup(FAR const struct zinitix_config_s *config)
{
  /* We do not have access to the WAKE pin in the implementation */
}


/****************************************************************************
 * Name: bes_zinitix_nreset
 *
 * Description:
 *   Control the chip reset pin
 *
 ****************************************************************************/

static void bes_zinitix_nreset(FAR const struct zinitix_config_s *config,
                                bool nstate)
{
  //irqstate_t flags;

  /* We do not have access to the RESET pin in the implementation */
  //flags = enter_critical_section();
  //struct ioexpander_dev_s * dev = bes_io_ioexpander_initialize();
  //dev->ops->ioe_writepin(dev, HAL_GPIO_PIN_P1_2, nstate);
  //leave_critical_section(flags);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bes_ts_zinitix_setup
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

int bes_ts_zinitix_setup(FAR const char *devname)
{
  FAR struct i2c_master_s *dev;
  int ret;

  iinfo("devname %s\n", devname);
  //DEBUGASSERT(minor == CONFIG_zinitix_DEVMINOR);

  /* Check if we are already initialized */

  iinfo("Initializing\n");

  /* Configure the zinitix interrupt pin as an input */

  bes_configgpio(GPIO_TP_INT);
  bes_configgpio(GPIO_TP_RST);

  /* Get an instance of the I2C interface */

  dev = bes_i2cbus_initialize(CONFIG_ZINITIX_I2CDEV);
  if (!dev)
    {
      ierr("ERROR: Failed to initialize I2C bus %d\n", CONFIG_ZINITIX_I2CDEV);
      return -ENODEV;
    }

  /* Initialize and register the I2C touchscreen device */

  ret = zinitix_register(dev, &g_zinitix_config, devname);
  if (ret < 0)
    {
      ierr("ERROR: Failed to register zinitix driver: %d\n", ret);
      bes_i2cbus_uninitialize(dev);
      return ret;
    }

  return OK;
}
#endif /* CONFIG_BES_TP_ZINITIX */
