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

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if 0
static int mk_fs_user_dir(fs_interface_t *fs)
{
    const char *user_fs_dir[] = {
        FS_PATH_RES,
        FS_PATH_AUDIO,
        FS_PATH_CONFIG,
        FS_PATH_GIFT,
        FS_PATH_THEMES,
        FS_PATH_TMP,
        FS_PATH_MUSIC,
    };

    int retry_count = 0;
    int err = 0;

    LOG_PBUS("START");

retry:
    if (fs->mkdir) {
        for (int i = 0; i < LENGTH_OF_ARRAY(user_fs_dir); i++) {
            err += fs->mkdir(user_fs_dir[i]);
            LOG_PBUS("mkdir: %s, err: %d", user_fs_dir[i], err);
        }

        if (err && (++retry_count < 2)) {
            LOG_PBUS("#retry: %d", retry_count);
            msleep(1000);
            goto retry;
        }
    }

    return err;
}

/// 挂载fs
static int fs_remount(void)
{
    fs_interface_t *fs = thiz->get_fs();
    int err = 0;
    int ret1 = -1;
    int ret2 = -1;
    int ret3 = -1;
    int retry_count = 0;

    LOG_PBUS("enter");
    EXCEPTION_CHECK_INT(fs);

    if (fs && fs->fs_mount) {
        err = fs->fs_mount(FS_PATH_ROOT, FS_PARTTION_USERDATA);
        if (err < 0) {
        retry_repair:
            /// 尝试格式化后再挂载.
            LOG_PBUS("retry fs_format: %d", retry_count);
            ret1 = fs->fs_format(FS_PATH_ROOT, FS_PARTTION_USERDATA);
            if (ret1 == 0) {
                LOG_PBUS("retry fs_mount");
                ret2 = fs->fs_mount(FS_PATH_ROOT, FS_PARTTION_USERDATA);
                if (ret2) {
                    LOG_PBUS("fs_mount fail: %d", ret2);
                } else {
                    ret3 = mk_fs_user_dir(fs);
                    if (ret3) {
                        LOG_PBUS("=>mk_fs_user_dir fail: %d", ret3);
                    }
                }
            } else {
                LOG_PBUS("fs_format fail:%d", ret1);
            }

            if (ret1 < 0 || ret1 < 0 || ret3 < 0) {
                err = -1;
                if (++retry_count < 5) {
                    msleep(2000);
                    goto retry_repair;
                }
                LOGE("#########fs repair failure!!!!#########");
            } else {
                LOG_PBUS("fs repair success, retry_count: %d", retry_count);
                err = 0;
            }
        } 
        else {
            ret3 = mk_fs_user_dir(fs);
            if (ret3) {
                LOG_PBUS("=>mk_fs_user_dir fail: %d", ret3);
                err = -1;
            }
        }
    }

    LOG_PBUS("final-fs_mount: %d", err);
    if (err != 0) {
        __system_reboot("=>fs_repair_failure.");
        msleep(5000);
    }

    fs->fs_is_ready = 0;
    
    return err;
}

/// 卸载fs
static int fs_unmount(void)
{
    fs_interface_t *fs = thiz->get_fs();
    int err = -1;

    EXCEPTION_CHECK_INT(fs);
    fs->fs_is_ready = -1;
    LOG_PBUS("fs_is_ready: -1");

    if (fs && fs->fs_unmount) {
        err = fs->fs_unmount(FS_PATH_ROOT);
    }

    LOG_PBUS("fs_unmount: %d", err);

    return err;
}

static int fs_format(void)
{
    fs_interface_t *fs = thiz->get_fs();
    int ret = -1;

    EXCEPTION_CHECK_INT(fs);
    fs->fs_is_ready = -1;

    if (fs && fs->fs_format) {
        ret = fs->fs_format(FS_PATH_ROOT, FS_PARTTION_USERDATA);
    }

    LOG_PBUS("fs_format: %d", ret);

    return ret;
}

static bool fs_ready(void)
{
    fs_interface_t *fs = thiz->get_fs();

    if (fs && fs->fs_mount) {
        return (fs->fs_is_ready == 0);
    }
    return 1;
}

static fs_interface_t *get_fs(void)
{
    peripheral_bus_context_t *ctx = thiz;

    if (ctx->priv.fs == NULL) {
        FIND_EXT_FLASH_IF(ctx, ext_if);
        if (ext_if) {
            ctx->priv.fs = ext_if->fs;
        }
    }

    return ctx->priv.fs;
}
#endif

static int fatfs_dev_open(struct rt_device_node *dev)
{
    return 0;
}

static int fatfs_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int fatfs_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    return 0;
}

static int fatfs_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return 0;
}

static int fatfs_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    return 0;
}

static int fatfs_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

static int fatfs_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_fatfs(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "fatfs";
    node->id = ID_DEVICE_FATFS;
    node->open = fatfs_dev_open;
    node->close = fatfs_dev_close;
    node->read = fatfs_dev_read;
    node->write = fatfs_dev_write;
    node->ioctl = fatfs_dev_ioctl;
    node->suspend = fatfs_dev_suspend;
    node->resume = fatfs_dev_resume;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}