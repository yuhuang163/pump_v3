/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "bsp_fds.h"
#include "norflash_api.h"
#include "app_flash_api.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;
static app_fds_t s_ext_fds;

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
    LOG_PBUS("type=%d, addr=0x%x, len=0x%x, result=%d, suspend_num=%d.",
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

static app_fds_t *database_get_ext_fds(const uint32_t *args, uint32_t n_bytes)
{
    if (n_bytes >= sizeof(uint32_t) * 4 && args[3] == DB_STORAGE_EXTERNAL) {
        return &s_ext_fds;
    }

    return NULL;
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
    ret = app_fds_init(PARTITION_FDS_NAME, (uint32_t)FDS_START_ADDR, FDS_SECTOR_NUM);
    if (ret != BSP_OK) {
        LOGE("database init failure!:%d", ret);
        return -1;
    }
    /// init external littlefs
    ret = app_fds_mount(&s_ext_fds, PARTITION_FDS_EXT_NAME, (uint32_t)FDS_EXT_START_ADDR, FDS_EXT_SECTOR_NUM);
    if (ret != BSP_OK) {
        LOGE("database ext init failure!:%d", ret);
        return -1;
    }
    return BSP_OK;
}

static int database_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    uint32_t *tmp = (uint32_t *)buf;
    app_fds_t *fds = NULL;

    EXCEPTION_CHECK_INT(tmp);
    fds = database_get_ext_fds(tmp, n_bytes);
    if (fds) {
        return app_fds_value_read_ex(fds, tmp[0], (void *)tmp[1], tmp[2]) < 0 ? BSP_ERROR : BSP_OK;
    }
    return app_fds_value_read(tmp[0], (void *)tmp[1], tmp[2]) < 0 ? BSP_ERROR : BSP_OK;
}

static int database_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    uint32_t *tmp = (uint32_t *)buf;
    app_fds_t *fds = NULL;

    EXCEPTION_CHECK_INT(tmp);
    fds = database_get_ext_fds(tmp, n_bytes);
    if (fds) {
        return app_fds_value_write_ex(fds, tmp[0], (void *)tmp[1], tmp[2]) < 0 ? BSP_ERROR : BSP_OK;
    }
    return app_fds_value_write(tmp[0], (void *)tmp[1], tmp[2]) < 0 ? BSP_ERROR : BSP_OK;
}

static int database_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    uint32_t *temp = (uint32_t *)args;
    switch (op_code) {
        case OP_CODE_DB_DELETE:
            return app_fds_value_delete(temp[0]);

        case OP_CODE_DB_EXT_DELETE:
            return app_fds_value_delete_ex(&s_ext_fds, temp[0]);

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
