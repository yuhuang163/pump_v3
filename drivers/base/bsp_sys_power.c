/*
* INCLUDE FILES
*****************************************************************************************
*/
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
#if IS_SUPPORT_HARDWARE_VERSION_ADAPTIVE
static uint8_t s_hardware_version = PRODUCT_INFO_V3PRO;
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#ifdef IS_SUPPORT_HARDWARE_VERSION_ADAPTIVE
static int hardware_version_check(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)HW_VERSION_DETECT_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(HW_VERSION_DETECT_PIN,  HAL_GPIO_DIR_IN, 1);
    if (hal_gpio_pin_get_val(HW_VERSION_DETECT_PIN)) {
        LOG_PBUS("hardware version is v3pro");
        return 1;
    }
    LOG_PBUS("hardware version is v3");
    return 0;
}
#endif

static void sys_power_gpio_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)SYS_POWER_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL,
    };

    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SYS_POWER_PIN, HAL_GPIO_DIR_OUT, 1);
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1)
    gpio_map.pin = (enum HAL_IOMUX_PIN_T)SYS_3V3_EN_PIN;
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SYS_3V3_EN_PIN, HAL_GPIO_DIR_OUT, 0);
#endif
    msleep(20);
    LOG_PBUS("sys_power_gpio_init");
}

static void sys_power_gpio_control(bool enable)
{
    GPIO_PIN_CTRL(SYS_POWER_PIN, enable ? 1 : 0);
    LOG_PBUS("sys_power_gpio_control: %d", enable);
}

static void sys_3v3_en_gpio_control(bool enable)
{
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1)
    GPIO_PIN_CTRL(SYS_3V3_EN_PIN, enable ? 0 : 1);
    LOG_PBUS("sys_3v3_en_gpio_control: %d", enable);
#endif
}

static int sys_power_dev_open(struct rt_device_node *dev)
{
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1 || V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
    sys_power_gpio_init();
#endif
#if IS_SUPPORT_HARDWARE_VERSION_ADAPTIVE
    s_hardware_version = hardware_version_check() > 0 ? PRODUCT_INFO_V3PRO : PRODUCT_INFO_V3;
#endif
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
        case OP_CODE_POWER_CONTROL: {
            EXCEPTION_CHECK_INT(temp);
            sys_power_gpio_control(temp[0] > 0 ? true : false);
        }
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
#if IS_SUPPORT_HARDWARE_VERSION_ADAPTIVE
int get_hardware_version(void)
{
    return s_hardware_version;
}
#endif

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
