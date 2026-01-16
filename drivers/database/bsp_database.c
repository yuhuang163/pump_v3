/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "bsp_fds.h"
#include "norflash_api.h"
#include "app_flash_api.h"
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
static void database_callback(void* param)
{
    NORFLASH_API_OPERA_RESULT *opera_result;
    opera_result = (NORFLASH_API_OPERA_RESULT*)param;
    LOG_PBUS("database callback: type = %d, addr = 0x%x,len = 0x%x,result = %d,suspend_num=%d.",
                opera_result->type, opera_result->addr, opera_result->len, opera_result->result,
                opera_result->suspend_num);
}

static int database_section_init(void)
{
    enum HAL_FLASH_ID_T flash_id;

    flash_id = norflash_api_get_dev_id_by_addr(FDS_START_ADDR);
    /// 使用app_flash_register_module简化注册，自动获取flash参数并使用默认缓存大小
    app_flash_register_module(NORFLASH_API_MODULE_ID_LITTLEFS,
                    flash_id, 
                    (uint32_t)FDS_START_ADDR, 
                    (uint32_t)FDS_END_ADDR - (uint32_t)FDS_START_ADDR,
                    (uint32_t)database_callback);

    return 0;
}

static int database_dev_open(struct rt_device_node *dev)
{
    int ret = 0;

    /// register database section
    ret = database_section_init();
    if (ret != BSP_OK) {
        LOGE("database section init failure! result = %d.",ret);
        return -1;
    }
    /// init littlefs
    ret = app_fds_init((uint32_t)FDS_START_ADDR, FDS_SECTOR_NUM);
    if (ret != BSP_OK) {
        LOGE("database init failure!:%d", ret);
        return -1;
    }
    return BSP_OK;
}

static int database_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    uint32_t *tmp = (uint32_t *)buf;

    EXCEPTION_CHECK_INT(tmp);
    return app_fds_value_read(tmp[0], (void *)tmp[1], tmp[2]) < 0 ? BSP_ERROR : BSP_OK;
}

static int database_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    uint32_t *tmp = (uint32_t *)buf;

    EXCEPTION_CHECK_INT(tmp);
    return app_fds_value_write(tmp[0], (void *)tmp[1], tmp[2]) < 0 ? BSP_ERROR : BSP_OK;
}

static int database_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    uint32_t *temp = (uint32_t *)args;
    switch (op_code) {
        case OP_CODE_DB_DELETE:
            return app_fds_value_delete(temp[0]);

        case OP_CODE_DB_DELETE_ALL:
            return 0;

        default:
            LOGE("database err op_code: %lx", op_code);
            return -1;
    }
}

static int database_dev_close(struct rt_device_node *dev)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_database(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "database";
    node->id = ID_DEVICE_DB;
    node->open = database_dev_open;
    node->read =  database_dev_read;
    node->write = database_dev_write;
    node->ioctl =  database_dev_ioctl;
    node->close =  database_dev_close;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
