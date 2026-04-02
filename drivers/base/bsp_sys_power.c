/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_bootmode.h"
#include "pmu.h"
#include "bsp_config.h"
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void sys_power_gpio_init(void)
{
#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1)
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)SYS_POWER_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SYS_POWER_PIN, HAL_GPIO_DIR_OUT, 1);
    msleep(20);
    LOG_PBUS("peripheral_power_gpio_init");
#endif
}

static void peripheral_power_gpio_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)PERIPHERAL_POWER_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)PERIPHERAL_POWER_PIN, HAL_GPIO_DIR_OUT, 1);
    msleep(20);
    LOG_PBUS("peripheral_power_gpio_init");
}

static void sys_power_gpio_control(bool enable)
{
#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1)
    GPIO_PIN_CTRL(SYS_POWER_PIN, enable ? 1 : 0);
    LOG_PBUS("sys_power_gpio_control: %d", enable);
#elif (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)   
    if (enable == false) {
        LOG_PBUS("shutdown system");
        msleep(50);
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
        pmu_shutdown();
    }
#endif
}

static void peripheral_power_control(bool enable)
{
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
    GPIO_PIN_CTRL(PERIPHERAL_POWER_PIN, enable ? 1 : 0);
    LOG_PBUS("peripheral_power_control: %d", enable);
#endif
}

static int sys_power_dev_open(struct rt_device_node *dev)
{
    sys_power_gpio_init();
    peripheral_power_gpio_init();
    return 0;
}

static int sys_power_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int sys_power_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    return 0;
}

static int sys_power_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return 0;
}

static int sys_power_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    uint32_t *temp = (uint32_t *)args;
    switch (op_code) {
        case OP_CODE_POWER_CONTROL: 
#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1)
            EXCEPTION_CHECK_INT(temp);
            sys_power_gpio_control(temp[0] > 0 ? true : false);
#elif (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)   
            sys_power_gpio_control(temp[0] > 0 ? true : false);
#else
            return -1;
#endif
            break;
        default:
            break;
    }
    return 0;
}

static int sys_power_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int sys_power_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_power(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "sys_power";
    node->id = ID_DEVICE_POWER;
    node->open = sys_power_dev_open;
    node->close = sys_power_dev_close;
    node->suspend = sys_power_dev_suspend;
    node->resume = sys_power_dev_resume;
    node->read = sys_power_dev_read;
    node->write = sys_power_dev_write;
    node->ioctl = sys_power_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
