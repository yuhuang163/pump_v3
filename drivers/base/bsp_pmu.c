/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "bsp_common.h"

/*
 * DEFINES
 *****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static rt_device_node_t *node = NULL;
static bool s_full_load_running = false;   /// 满负载运行标志，由电机泵驱动在泵开启/暂停时设置

/*
 * GLOBAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
#if 0
static void pmu_gpio_init(void)
{
    app_io_init_t io_init = {
        .pull = APP_IO_NOPULL,
        .mode = APP_IO_MODE_INPUT,
        .pin  = MOISTURE_DETECTION_PIN,
        .mux  = BSP_COMMON_IO_MUX
    };

    app_io_init(MOISTURE_DETECTION_IO_TYPE, &io_init);
}
#endif

static int pmu_dev_open(struct rt_device_node *dev)
{
    return 0;
}

static int pmu_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int pmu_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    return 0;
}

static int pmu_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return 0;
}

static int pmu_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    switch (op_code) {
        default:
            return -1;
    }
}

static int pmu_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int pmu_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void *register_device_for_pmu(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "pmu";
    node->id = ID_DEVICE_PMU;
    node->open = pmu_dev_open;
    node->close = pmu_dev_close;
    node->suspend = pmu_dev_suspend;
    node->resume = pmu_dev_resume;
    node->read = pmu_dev_read;
    node->write = pmu_dev_write;
    node->ioctl = pmu_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}

bool is_full_load_running(void)
{
    return s_full_load_running;
    return false;
}

void bsp_set_full_load_running(bool is_running)
{
    s_full_load_running = is_running;
}
