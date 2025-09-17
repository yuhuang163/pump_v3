/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "norflash_api.h"
#include "hal_norflash.h"

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
static void flash_callback(void* param)
{
    NORFLASH_API_OPERA_RESULT *opera_result;
    opera_result = (NORFLASH_API_OPERA_RESULT*)param;
    LOGI("flash callback: type = %d, addr = 0x%x,len = 0x%x,result = %d,suspend_num=%d.",
                opera_result->type, opera_result->addr, opera_result->len, opera_result->result,
                opera_result->suspend_num);
}

static int flash_section_init(void)
{
    enum NORFLASH_API_RET_T result;
    enum HAL_FLASH_ID_T flash_id;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    uint32_t page_size = 0;

    flash_id = norflash_api_get_dev_id_by_addr(FACTORY_DATA_START_ADDR);
    hal_norflash_get_size(flash_id, NULL, &block_size, &sector_size, &page_size);
    result = norflash_api_register(NORFLASH_API_MODULE_ID_FACTORY_DATA,
                    flash_id, (uint32_t)FACTORY_DATA_START_ADDR, 
                    (uint32_t)FACTORY_DATA_END_ADDR - (uint32_t)FACTORY_DATA_START_ADDR,
                    block_size, sector_size, page_size, FACTORY_DATA_BLOCK_SIZE, 
                    flash_callback);

    return result;
}

static int flash_dev_open(struct rt_device_node *dev)
{
    int ret = 0;

    LOGI("flash dev open: %p, %p", FACTORY_DATA_START_ADDR, FACTORY_DATA_END_ADDR);
    ret = flash_section_init();
    if (ret != BSP_OK) {
        LOGE("flash section init failure! result = %d.",ret);
        return BSP_ERROR;
    }
    return BSP_OK;
}

static int flash_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int flash_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    int ret = 0;
    uint32_t lock = 0;
    uint32_t *tmp = (uint32_t *)buf;
    
    EXCEPTION_CHECK_INT(tmp);
    lock = platform_int_lock_global();
    ret = platform_flash_read(PARTITION_FACTORY_DATA_NAME, tmp[0], (void *)tmp[1], tmp[2]) < 0 ? BSP_ERROR : BSP_OK;
    platform_int_unlock_global(lock);
    if (ret != BSP_OK) {
        LOGE("flash dev read failure! result = %d.",ret);
        return BSP_ERROR;
    }
    return BSP_OK;
}

static int flash_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    int ret = 0;
    uint32_t lock = 0;
    uint32_t *tmp = (uint32_t *)buf;

    EXCEPTION_CHECK_INT(tmp);
    lock = platform_int_lock_global();
    ret = platform_flash_write(PARTITION_FACTORY_DATA_NAME, tmp[0], (void *)tmp[1], tmp[2]) < 0 ? BSP_ERROR : BSP_OK;
    platform_int_unlock_global(lock);
    if (ret != BSP_OK) {
        LOGE("flash dev write failure! result = %d.",ret);
        return BSP_ERROR;
    }
    return BSP_OK;
}

static int flash_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    int ret = 0;
    uint32_t lock = 0;
    uint32_t *temp = (uint32_t *)args;
    switch (op_code) {
        case OP_CODE_FLASH_ERASE:
            lock = platform_int_lock_global();
            ret = platform_flash_erase(PARTITION_FACTORY_DATA_NAME, temp[0], temp[1]) < 0 ? BSP_ERROR : BSP_OK;
            platform_int_unlock_global(lock);
            if (ret != BSP_OK) {
                LOGE("flash dev erase failure! result = %d.",ret);
                return BSP_ERROR;
            }
            return BSP_OK;

        case OP_CODE_FLASH_ERASE_ALL:
            return 0;

        default:
            LOGE("database err op_code: %lx", op_code);
            return -1;
    }
}


static int flash_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

static int flash_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_flash(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "flash";
    node->id = ID_DEVICE_FLASH;
    node->open = flash_dev_open;
    node->close = flash_dev_close;
    node->read = flash_dev_read;
    node->write = flash_dev_write;
    node->ioctl = flash_dev_ioctl;
    node->suspend = flash_dev_suspend;
    node->resume = flash_dev_resume;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
