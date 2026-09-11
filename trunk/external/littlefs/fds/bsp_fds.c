/**
 *****************************************************************************************
 *
 * @file bsp_fds.c
 *
 * @brief App Flash Data Storage Implementation.
 *
 *****************************************************************************************
 * @attention
  #####Copyright (c) 2019 GOODIX
  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of GOODIX nor the names of its contributors may be used
    to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "bsp_fds.h"
#include "cmsis_os_api.h"

#define APP_FDS_VERIFY(ret)           \
do                                    \
{                                     \
    if (ret)                          \
    {                                 \
        return ret;                   \
    }                                 \
} while(0)

extern app_fds_config_t s_app_fds_config;

static app_fds_t s_default_instance;


static int app_fds_lock(const struct lfs_config *c);
static int app_fds_unlock(const struct lfs_config *c);
static int app_fds_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
static int app_fds_write(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
static int app_fds_erase(const struct lfs_config *c, lfs_block_t block);
static int app_fds_sync(const struct lfs_config *c);


#if !APP_FDS_KEY_STRING_TYPE
static uint32_t app_fds_char_key_convert(const char *str)
{
    uint32_t num = 0;
    const char *p = str;

    while (*p >= '0' && *p <= '9')
    {
        num = num * 10 + (*p - '0');
        p++;
    }

    return num;
}

static char * app_fds_int_key_convert(app_fds_t *p_fds, uint32_t key)
{
    uint16_t digit = 0;
    uint32_t temp = key;

    do
    {
        temp /= 10;
        digit++;
    } while (temp > 0);

    p_fds->key_char[digit] = '\0';

    do {
        digit--;
        p_fds->key_char[digit] = key % 10 + '0';
        key /= 10;
    } while (key > 0);

    return p_fds->key_char;
}
#endif

static int app_fds_lock(const struct lfs_config *c)
{
    if (((app_fds_config_t *)c->context)->fds_lock)
    {
        return ((app_fds_config_t *)c->context)->fds_lock();
    }

    return APP_FDS_ERR_CORRUPT;
}

static int app_fds_unlock(const struct lfs_config *c)
{
    if (((app_fds_config_t *)c->context)->fds_unlock)
    {
        return ((app_fds_config_t *)c->context)->fds_unlock();
    }

    return APP_FDS_ERR_CORRUPT;
}

static int app_fds_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    if (((app_fds_config_t *)c->context)->fds_read)
    {
        return ((app_fds_config_t *)c->context)->fds_read(((app_fds_config_t *)c->context)->partition_name,
                ((app_fds_config_t *)c->context)->fds_start_addr + block * c->block_size + off, (uint8_t *)buffer, size);
    }

    return APP_FDS_ERR_CORRUPT;
}

static int app_fds_write(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    if (((app_fds_config_t *)c->context)->fds_write)
    {
        return ((app_fds_config_t *)c->context)->fds_write(((app_fds_config_t *)c->context)->partition_name,
                ((app_fds_config_t *)c->context)->fds_start_addr + block * c->block_size + off, (uint8_t *)buffer, size);
    }

    return APP_FDS_ERR_CORRUPT;
}

static int app_fds_erase(const struct lfs_config *c, lfs_block_t block)
{
    if (((app_fds_config_t *)c->context)->fds_erase)
    {
        return ((app_fds_config_t *)c->context)->fds_erase(((app_fds_config_t *)c->context)->partition_name,
                ((app_fds_config_t *)c->context)->fds_start_addr + block * c->block_size, c->block_size);
    }

    return APP_FDS_ERR_CORRUPT;
}
static int app_fds_sync(const struct lfs_config *c)
{
    if (((app_fds_config_t *)c->context)->fds_sync)
    {
        return((app_fds_config_t *)c->context)->fds_sync();
    }

    return APP_FDS_ERR_CORRUPT;
}

int app_fds_init(const char *partition_name, uint32_t fds_start_addr, uint32_t fds_block_cnt)
{
    return app_fds_mount(&s_default_instance, partition_name, fds_start_addr, fds_block_cnt);
}

int app_fds_mount(app_fds_t *p_fds, const char *partition_name, uint32_t fds_start_addr, uint32_t fds_block_cnt)
{
    return app_fds_mount_ex(p_fds, partition_name, fds_start_addr, fds_block_cnt, 1);
}

int app_fds_mount_ex(app_fds_t *p_fds, const char *partition_name,
                     uint32_t fds_start_addr, uint32_t fds_block_cnt,
                     int allow_format)
{
    int ret;

    if (!p_fds || !partition_name || fds_start_addr % 0x1000 || !fds_block_cnt)
    {
		PRINT_LOG("Fds init Failed!\r\n");
        return APP_FDS_ERR_INVAL;
    }
    p_fds->config = s_app_fds_config;
    p_fds->config.fds_start_addr   = fds_start_addr;
    p_fds->config.fds_4k_block_cnt = fds_block_cnt;
    p_fds->config.partition_name   = partition_name;

    memset(&p_fds->lfs_cfg, 0, sizeof(p_fds->lfs_cfg));
    p_fds->lfs_cfg.context            = &p_fds->config;
    p_fds->lfs_cfg.read               = app_fds_read;
    p_fds->lfs_cfg.prog               = app_fds_write;
    p_fds->lfs_cfg.erase              = app_fds_erase;
    p_fds->lfs_cfg.sync               = app_fds_sync;
    p_fds->lfs_cfg.lock               = app_fds_lock;
    p_fds->lfs_cfg.unlock             = app_fds_unlock;
    p_fds->lfs_cfg.read_size          = 4;
    p_fds->lfs_cfg.prog_size          = 4;
    p_fds->lfs_cfg.block_cycles       = 500;
    p_fds->lfs_cfg.cache_size         = APP_FDS_CACHE_SIZE;
    p_fds->lfs_cfg.lookahead_size     = APP_FDS_LOOKAHEAD_SIZE;
    p_fds->lfs_cfg.read_buffer        = p_fds->read_buffer;
    p_fds->lfs_cfg.prog_buffer        = p_fds->prog_buffer;
    p_fds->lfs_cfg.lookahead_buffer   = p_fds->lookahead_buffer;
    p_fds->lfs_cfg.block_count        = fds_block_cnt;
    p_fds->lfs_cfg.block_size         = 0x1000;

    ret = lfs_mount(&p_fds->lfs, &p_fds->lfs_cfg);
    if (ret)
    {
        PRINT_LOG("lfs_mount ret: %d, allow_format=%d\r\n", ret, allow_format);
        if (!allow_format) {
            return ret;
        }
        ret = lfs_format(&p_fds->lfs, &p_fds->lfs_cfg);
        PRINT_LOG("lfs_format ret: %d\r\n", ret);
        APP_FDS_VERIFY(ret);
        ret = lfs_mount(&p_fds->lfs, &p_fds->lfs_cfg);
        PRINT_LOG("lfs_mount ret: %d\r\n", ret);
        APP_FDS_VERIFY(ret);
    }
    if (allow_format) {
        lfs_fs_gc(&p_fds->lfs);
    }
    return APP_FDS_ERR_OK;
}

int app_fds_value_write(
#if  APP_FDS_KEY_STRING_TYPE
                              char *p_key,
#else
                              uint32_t key,
#endif
                              const void *p_value,
                              uint32_t length)
{
#if  APP_FDS_KEY_STRING_TYPE
    return app_fds_value_write_ex(&s_default_instance, p_key, p_value, length);
#else
    return app_fds_value_write_ex(&s_default_instance, key, p_value, length);
#endif
}

int app_fds_value_write_ex(
                              app_fds_t *p_fds,
#if  APP_FDS_KEY_STRING_TYPE
                              char *p_key,
#else
                              uint32_t key,
#endif
                              const void *p_value,
                              uint32_t length)
{
    int ret;

    lfs_file_t              file_id;
    struct lfs_file_config  file_config = {0};

    if (!p_fds) {
        return APP_FDS_ERR_INVAL;
    }

    file_config.buffer = p_fds->file_buffer;
 
#if  APP_FDS_KEY_STRING_TYPE
    ret = lfs_file_opencfg(&p_fds->lfs, &file_id, p_key, LFS_O_RDWR | LFS_O_CREAT, &file_config);
    APP_FDS_VERIFY(ret);
#else
    ret = lfs_file_opencfg(&p_fds->lfs, &file_id, app_fds_int_key_convert(p_fds, key), LFS_O_RDWR | LFS_O_CREAT, &file_config);
    APP_FDS_VERIFY(ret);
#endif

    ret = lfs_file_rewind(&p_fds->lfs, &file_id);
    if (ret)
    {
        lfs_file_close(&p_fds->lfs, &file_id);
        return ret;
    }

    ret = lfs_file_write(&p_fds->lfs, &file_id, p_value, length);
    if (ret)
    {
        lfs_file_close(&p_fds->lfs, &file_id);
        return ret;
    }

    ret = lfs_file_close(&p_fds->lfs, &file_id);
    APP_FDS_VERIFY(ret);

    return 0;
}

int app_fds_value_write_at_ex(
                              app_fds_t *p_fds,
#if  APP_FDS_KEY_STRING_TYPE
                              char *p_key,
#else
                              uint32_t key,
#endif
                              uint32_t offset,
                              const void *p_value,
                              uint32_t length)
{
    int ret;
    lfs_file_t file_id;
    struct lfs_file_config file_config = {0};

    if (!p_fds || !p_value || length == 0) {
        return APP_FDS_ERR_INVAL;
    }

    file_config.buffer = p_fds->file_buffer;
#if  APP_FDS_KEY_STRING_TYPE
    ret = lfs_file_opencfg(&p_fds->lfs, &file_id, p_key, LFS_O_RDWR | LFS_O_CREAT, &file_config);
#else
    ret = lfs_file_opencfg(&p_fds->lfs, &file_id, app_fds_int_key_convert(p_fds, key),
                           LFS_O_RDWR | LFS_O_CREAT, &file_config);
#endif
    if (ret < 0) {
        return ret;
    }

    ret = lfs_file_seek(&p_fds->lfs, &file_id, (lfs_soff_t)offset, LFS_SEEK_SET);
    if (ret < 0) {
        lfs_file_close(&p_fds->lfs, &file_id);
        return ret;
    }

    ret = lfs_file_write(&p_fds->lfs, &file_id, p_value, length);
    if (ret < 0 || (uint32_t)ret != length) {
        lfs_file_close(&p_fds->lfs, &file_id);
        return (ret < 0) ? ret : APP_FDS_ERR_IO;
    }

    ret = lfs_file_close(&p_fds->lfs, &file_id);
    if (ret < 0) {
        return ret;
    }
    return 0;
}

int app_fds_value_read_at_ex(
                             app_fds_t *p_fds,
#if  APP_FDS_KEY_STRING_TYPE
                             char *p_key,
#else
                             uint32_t key,
#endif
                             uint32_t offset,
                             void *p_buffer,
                             uint32_t length)
{
    int ret;
    lfs_file_t file_id;
    struct lfs_file_config file_config = {0};

    if (!p_fds || !p_buffer || length == 0) {
        return APP_FDS_ERR_INVAL;
    }

    file_config.buffer = p_fds->file_buffer;
#if  APP_FDS_KEY_STRING_TYPE
    ret = lfs_file_opencfg(&p_fds->lfs, &file_id, p_key, LFS_O_RDONLY, &file_config);
#else
    ret = lfs_file_opencfg(&p_fds->lfs, &file_id, app_fds_int_key_convert(p_fds, key),
                           LFS_O_RDONLY, &file_config);
#endif
    if (ret < 0) {
        return ret;
    }

    ret = lfs_file_seek(&p_fds->lfs, &file_id, (lfs_soff_t)offset, LFS_SEEK_SET);
    if (ret < 0) {
        lfs_file_close(&p_fds->lfs, &file_id);
        return ret;
    }

    ret = lfs_file_read(&p_fds->lfs, &file_id, p_buffer, length);
    if (ret < 0 || (uint32_t)ret != length) {
        lfs_file_close(&p_fds->lfs, &file_id);
        return (ret < 0) ? ret : APP_FDS_ERR_IO;
    }

    ret = lfs_file_close(&p_fds->lfs, &file_id);
    if (ret < 0) {
        return ret;
    }
    return 0;
}

int app_fds_value_read(
#if  APP_FDS_KEY_STRING_TYPE
                             char *p_key,
#else
                             uint32_t key,
#endif
                             void *p_buffer,
                             uint32_t length)
{
#if  APP_FDS_KEY_STRING_TYPE
    return app_fds_value_read_ex(&s_default_instance, p_key, p_buffer, length);
#else
    return app_fds_value_read_ex(&s_default_instance, key, p_buffer, length);
#endif
}

int app_fds_value_read_ex(
                             app_fds_t *p_fds,
#if  APP_FDS_KEY_STRING_TYPE
                             char *p_key,
#else
                             uint32_t key,
#endif
                             void *p_buffer,
                             uint32_t length)
{
    int ret;

    lfs_file_t              file_id;
    struct lfs_file_config  file_config = {0};

    if (!p_fds) {
        return APP_FDS_ERR_INVAL;
    }

    file_config.buffer = p_fds->file_buffer;
 
#if  APP_FDS_KEY_STRING_TYPE
    ret = lfs_file_opencfg(&p_fds->lfs, &file_id, p_key, LFS_O_RDWR | LFS_O_CREAT, &file_config);
    APP_FDS_VERIFY(ret);
#else
    ret = lfs_file_opencfg(&p_fds->lfs, &file_id, app_fds_int_key_convert(p_fds, key), LFS_O_RDONLY, &file_config);
    APP_FDS_VERIFY(ret);
#endif
    ret = lfs_file_rewind(&p_fds->lfs, &file_id);
    if (ret)
    {
        lfs_file_close(&p_fds->lfs, &file_id);
        return ret;
    }

    ret = lfs_file_read(&p_fds->lfs, &file_id, p_buffer, length);
    if (ret)
    {
        lfs_file_close(&p_fds->lfs, &file_id);
        return ret;
    }

    ret = lfs_file_close(&p_fds->lfs, &file_id);
    APP_FDS_VERIFY(ret);

    return 0;
}


int app_fds_value_delete(
#if  APP_FDS_KEY_STRING_TYPE
                               char *p_key)
#else
                               uint32_t key)
#endif
{
#if  APP_FDS_KEY_STRING_TYPE
    return app_fds_value_delete_ex(&s_default_instance, p_key);
#else
    return app_fds_value_delete_ex(&s_default_instance, key);
#endif
}

int app_fds_value_delete_ex(
                               app_fds_t *p_fds,
#if  APP_FDS_KEY_STRING_TYPE
                               char *p_key)
#else
                               uint32_t key)
#endif
{
    if (!p_fds) {
        return APP_FDS_ERR_INVAL;
    }

#if  APP_FDS_KEY_STRING_TYPE
    return lfs_remove(&p_fds->lfs, p_key);
#else
     return lfs_remove(&p_fds->lfs, app_fds_int_key_convert(p_fds, key));
#endif
}


int app_fds_traverse(app_fds_traverse_cb_t traverse_cb)
{
    int ret;
    lfs_dir_t dir;
    struct lfs_info info;

    ret =lfs_dir_open(&s_default_instance.lfs, &dir, ".");
    APP_FDS_VERIFY(ret);

    bool is_continue;

    while(1)
    {
        ret =  lfs_dir_read(&s_default_instance.lfs, &dir, &info);
        if (ret == APP_FDS_ERR_OK)
        {
            return lfs_dir_close(&s_default_instance.lfs, &dir);
        }


        if (ret <= APP_FDS_ERR_OK)
        {
            return lfs_dir_close(&s_default_instance.lfs, &dir);
        }

        if (LFS_TYPE_REG == info.type && traverse_cb)
        {
#if  APP_FDS_KEY_STRING_TYPE
            traverse_cb(info.name, info.size, &is_continue);
#else
            traverse_cb(app_fds_char_key_convert(info.name), info.size, &is_continue);
#endif
            if (!is_continue)
            {
                return APP_FDS_ERR_OK;
            }
        }
    };
}


