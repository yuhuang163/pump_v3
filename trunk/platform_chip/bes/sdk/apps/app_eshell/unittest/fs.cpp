/***************************************************************************
 *
 * Copyright 2015-2021 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#if defined(UTILS_ESHELL_EN) && defined(__SYS_AS_MAIN__)
#if defined(FS_FATFS_ENABLE) || defined(FS_LITTLEFS_ENABLE)

#include "eshell.h"
#include "app_eshell.h"
#include "plat_types.h"
#include "mbed_fs.h"
#include "stdio.h"
#include "hal_timer.h"
#include "crc32_c.h"

#define MAX_BUFF_SIZE       32768
#define MIN_BUFF_SIZE       128
#define FS_RATE_FILE        "fs_rate.log"
#define FS_RATE_BASE_LENGTH 500*1024
#define FS_MOUNT_NAME_LEN   32
#define TRUNCATE_EMPTY_YES 1
#define TRUNCATE_EMPTY_NO  0

static char g_buff[MAX_BUFF_SIZE+1] = {'\0'};
static char g_rate_name[NAME_MAX+1] = {'\0'};

typedef enum fs_cmd {
    FS_CMD_UNKNOWN = 0,
    FS_CMD_RWRATE,
    FS_CMD_RWEXACT,
    FS_CMD_RWCYCLIC,
    FS_CMD_RWRANDOM,
    FS_CMD_RWORDER,
    FS_CMD_MOUNT,
    FS_CMD_UMOUNT,
    FS_CMD_SHOW
} fs_cmd_e;

static int fs_safe_read(FILE *fd, char *buff, int32_t total_len, int32_t single_size)
{
    if (fd == NULL || buff == NULL) {
        return -1;
    }

    int32_t read_len = 0;
    memset(buff, 0, total_len);
    while (total_len > 0) {
        int32_t tmp;
        int32_t res;
        tmp = total_len > single_size ? single_size : total_len;
        res = fread(buff + read_len, tmp, 1, fd);
        if (res <= 0) {
            eshell_putstring("%s fail: res=%d\r\n", __func__, res);
            break;
        }
        total_len -= tmp;
        read_len += tmp;
    }
    return read_len;
}

static int fs_safe_write(FILE *fd, char *buff, int32_t total_len, int32_t single_size)
{
    if (fd == NULL || buff == NULL) {
        return -1;
    }

    int32_t written_len = 0;
    while (total_len > 0) {
        int32_t tmp;
        int32_t res;
        tmp = total_len > single_size ? single_size : total_len;
        res = fwrite(buff + written_len, tmp, 1, fd);
        if (res != 1) {
            eshell_putstring("%s fail: res=%d\r\n", __func__, res);
            break;
        }
        total_len -= tmp;
        written_len += tmp;
    }
    return written_len;
}

static int fs_file_builder(const char *path, char *buff, int len, int truncate)
{
    if (buff == NULL || len <= 0) {
        return -1;
    }

    FILE *fd = NULL;
    if (truncate) {
        fd = fopen(path, "w");
    } else {
        fd = fopen(path, "ab+");
    }

    if (fd == NULL) {
        eshell_putstring("%s fail: open %s err, res=%d\r\n", __func__, path);
        return -1;
    }

    int32_t written_len = 0;
    written_len = fs_safe_write(fd, buff, len, MIN_BUFF_SIZE);
    if (written_len != len){
        eshell_putstring("%s %s fail\r\n", __func__, path);
        fclose(fd);
        return -1;
    }

    fclose(fd);
    return 0;
}

static void fs_string_builder(char *dest, uint32_t len)
{
#define MAX_SEED_CHAR_LEN 63
#define TAIL_TAG_LEN 10
    const unsigned char seedchars[MAX_SEED_CHAR_LEN] =
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    uint32_t i;
    uint32_t randidx;
    srand((uint32_t)time(NULL));

    len = len - TAIL_TAG_LEN;
    for (i = 0; i < len; i++) {
        randidx = rand() % (MAX_SEED_CHAR_LEN-1);
        *dest = seedchars[randidx];
        dest++;
    }

    dest[i] = '\0';
    char tag[TAIL_TAG_LEN+1] = {'\0'};
    snprintf(tag, sizeof(tag)-1, "%d", len);
    strcat(dest, tag);
}

static void fs_rate_write(int32_t single_size)
{
    FILE *fd;
    fd = fopen(g_rate_name, "ab+");
    if (fd == NULL) {
        eshell_putstring("%s fail: open err, path=%s\r\n", __func__, g_rate_name);
        return;
    }

    int32_t write_len = 0;
    int32_t total_len = FS_RATE_BASE_LENGTH;
    memset(g_buff, 'M', MAX_BUFF_SIZE);
    uint32_t tvbegin = hal_sys_timer_get();
    while(total_len > 0) {
        uint32_t tmp;
        uint32_t res;
        tmp = total_len > MAX_BUFF_SIZE ? MAX_BUFF_SIZE : total_len;
        res = fs_safe_write(fd, g_buff, tmp, single_size);
        if (res != tmp) {
            break;
        }
        total_len -= res;
        write_len += res;
    }
    fclose(fd);
    uint32_t tvdone = hal_sys_timer_get();
    if (total_len > 0) {
        eshell_putstring("%s fail: not writing enough data\r\n", __func__);
        return;
    }
    uint32_t ms = TICKS_TO_MS(tvdone - tvbegin);
    if (ms == 0)
        ms = 1;
    eshell_putstring("%s:  sec=%d size=%d(B) rate=%d(KB/s) \r\n",
        __FUNCTION__, single_size, write_len, write_len / ms);
    return;
}

static void fs_rate_read(int32_t single_size)
{
    FILE *fd;
    fd = fopen(g_rate_name, "r");
    if (fd == NULL) {
        eshell_putstring("%s fail: open err, path=%s\r\n", __func__, g_rate_name);
        return;
    }

    int32_t read_len = 0;
    int32_t total_len = FS_RATE_BASE_LENGTH;
    uint32_t tvbegin = hal_sys_timer_get();
    while (total_len > 0) {
        int32_t res;
        int32_t tmp;
        tmp = total_len > single_size ? single_size : total_len;
        res = fread(g_buff, tmp, 1, fd);
        if (res <= 0 ) {
            eshell_putstring("%s fail: res=%d\r\n", __func__, res);
            break;
        }
        total_len -= tmp;
        read_len += tmp;
    }
    fclose(fd);
    uint32_t tvdone = hal_sys_timer_get();
    if (total_len > 0) {
        eshell_putstring("%s fail: not reading enough data leftlen:%d read_len:%d\r\n",
            __func__, total_len, read_len);
        return;
    }
    uint32_t ms = TICKS_TO_MS(tvdone - tvbegin);
    if (ms == 0)
        ms = 1;
    eshell_putstring("%s:  sec=%d size=%d(B) rate=%d(KB/s) \r\n",
        __FUNCTION__, single_size, read_len, read_len / ms);
    return;
}

static void fs_write_check(int32_t sec_size, int32_t truncate)
{
    if (sec_size < MIN_BUFF_SIZE || sec_size > MAX_BUFF_SIZE) {
        sec_size = MAX_BUFF_SIZE;
        eshell_putstring("%s fail: param size err, default=%d\r\n", __func__, sec_size);
    }

    int32_t ret;
    int32_t total_len = sec_size;
    unsigned long w_crc;
    memset(g_buff, '\0', MAX_BUFF_SIZE + 1);
    fs_string_builder(g_buff, total_len);
    w_crc = crc32_c(0, (unsigned char*)g_buff, total_len);
    ret = fs_file_builder(g_rate_name, g_buff, total_len, truncate);
    if (ret != 0) {
        eshell_putstring("%s fail: didn't write enough\r\n", __func__);
        return;
    }

    struct stat result;
    memset(&result, 0x00, sizeof(result));
    ret = stat(g_rate_name, &result);
    if (ret != 0) {
        eshell_putstring("%s: file not exist err, path:%s", __FUNCTION__, g_rate_name);
        return;
    }

    FILE *fd;
    int32_t read_len = 0;
    fd = fopen(g_rate_name, "r");
    if (fd == NULL) {
        eshell_putstring("%s fail: open err, path:%d\r\n", __func__, g_rate_name);
        return;
    }
    fseek(fd, result.st_size - total_len, SEEK_SET);
    memset(g_buff, '\0', MAX_BUFF_SIZE + 1);
    read_len = fs_safe_read(fd, g_buff, total_len, sec_size);
    fclose(fd);

    unsigned long r_crc;
    r_crc = crc32_c(0, (unsigned char*)g_buff, read_len);
    if (w_crc != r_crc){
        eshell_putstring("%s fail: w_crc is not equal with r_crc.\r\n", __func__);
        return;
    }
    eshell_putstring("%s PASS, write_size=%d\r\n", __func__, total_len);
    return;
}

static void utest_fs_rate(int32_t sec_size)
{
    int32_t secs[] = {128, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    if (sec_size < MIN_BUFF_SIZE || sec_size > MAX_BUFF_SIZE) {
        for (uint32_t i = 0; i < sizeof(secs)/sizeof(int); i++) {
            remove(g_rate_name);
            fs_rate_write(secs[i]);
            fs_rate_read(secs[i]);
        }
        return;
    }
    remove(g_rate_name);
    fs_rate_write(sec_size);
    fs_rate_read(sec_size);
    return;
}

static void utest_fs_cyclic(uint32_t times)
{
    if (times >=1000) {
        times = 1000;
    }
    for (uint32_t i = 0; i < times; i++) {
        fs_write_check(MAX_BUFF_SIZE, TRUNCATE_EMPTY_YES);
    }
}

static void utest_fs_random(uint32_t times)
{
    if (times >=1000) {
        times = 1000;
    }
    for (uint32_t i = 0; i < times; i++) {
        fs_write_check(MAX_BUFF_SIZE-i*10, TRUNCATE_EMPTY_YES);
    }
}

static void utest_fs_order(uint32_t times)
{
    if (times >=1000) {
        times = 1000;
    }
    for (uint32_t i = 0; i < times; i++) {
        fs_write_check(MAX_BUFF_SIZE-i*5, TRUNCATE_EMPTY_NO);
    }
}

static void utest_fs_exact(int32_t size)
{
    fs_write_check(size, TRUNCATE_EMPTY_YES);
}

static int fs_byte_print(char *buff, uint32_t total_len, uint32_t line)
{
    uint32_t i = 0;
    char blog[64] = {'\0'};
    unsigned char *c = (unsigned char*)buff;

    while (total_len > 0) {
        memset(blog, 0x00, sizeof(blog));
        uint32_t head = 0;
        uint32_t len = 0;
        uint32_t tmp = 0;
        for (i = 0; i < 16; i++) {
            if (head == 0) {
                line++;
                tmp = snprintf(blog + len, sizeof(blog)-len-1, "line:0x%04x ", line);
                len += tmp;
                head = 1;
            }
            if (i < total_len) {
                tmp = snprintf(blog + len, sizeof(blog)-len-1, "%02x ", c[i]);
                len += tmp;
            }
        }
        eshell_putstring("%s: %s\r\n", __FUNCTION__, blog);
        c += 16;
        if (total_len <= 16)
            break;
        total_len -= 16;
    }
    return line;
}

static void utest_fs_show_file(char *path)
{
    struct stat result;
    int32_t ret;

    memset(&result, 0x00, sizeof(result));
    ret = stat(path, &result);
    if (ret != 0 || S_ISDIR(result.st_mode)) {
        eshell_putstring("%s: file not exist err, path:%s", __FUNCTION__, path);
        return;
    }

    FILE *fp = NULL;
    fp = fopen(path, "r");
    if (fp == NULL) {
        eshell_putstring("%s: fopen fail, path:%s\r\n", __FUNCTION__, path);
        return;
    }

    int32_t read_len = 0;
    int32_t read_len_tmp = 0;
    uint32_t line = 0;
    char buff[1024] = {'\0'};
    while (read_len < result.st_size) {
        memset(buff, '\0', sizeof(buff));
        if (result.st_size - read_len > 1024)
            read_len_tmp = fs_safe_read(fp, buff, 1024, 1024);
        else
            read_len_tmp = fs_safe_read(fp, buff, result.st_size, result.st_size);
        if (read_len_tmp <= 0) {
            break;
        }
        read_len += read_len_tmp;
        line = fs_byte_print(buff, read_len_tmp, line);
    }
    fclose(fp);
}

static int utest_fs_mount(char *name)
{
    if (name == NULL || strlen(name) > FS_MOUNT_NAME_LEN) {
        eshell_putstring("%s: mount fail, name err\r\n", __FUNCTION__);
        return -1;
    }

    int res;
    res = mbed_fs_mount(name);
    if (res != 0 ) {
        eshell_putstring("%s: mount fail\r\n", __FUNCTION__);
        return res;
    }

    eshell_putstring("%s: mount pass\r\n", __FUNCTION__);
    memset(g_rate_name, 0x00, sizeof(g_rate_name));
    snprintf(g_rate_name, sizeof(g_rate_name)-1, "/%s/%s", name, FS_RATE_FILE);
    return 0;
}

static int utest_fs_umount(char *name)
{
    if (name == NULL || strlen(name) > FS_MOUNT_NAME_LEN) {
        eshell_putstring("%s: mount fail, name err\r\n", __FUNCTION__);
        return -1;
    }

    remove(g_rate_name);
    int res;
    res = mbed_fs_umount(name);
    if (res != 0 ) {
        eshell_putstring("%s: umount fail\r\n", __FUNCTION__);
        return res;
    }

    eshell_putstring("%s: umount pass\r\n", __FUNCTION__);
    return 0;
}

static int32_t utest_fs_cmd(char *cmd)
{
    if (cmd == NULL || strlen(cmd) == 0) {
        return FS_CMD_UNKNOWN;
    }

    if (strncmp(cmd, "rate", strlen("rate")) == 0)
        return FS_CMD_RWRATE;
    else if (strncmp(cmd, "exact", strlen("exact")) == 0)
        return FS_CMD_RWEXACT;
    else if (strncmp(cmd, "rand", strlen("rand")) == 0)
        return FS_CMD_RWRANDOM;
    else if (strncmp(cmd, "cyclic", strlen("cyclic")) == 0)
        return FS_CMD_RWCYCLIC;
    else if (strncmp(cmd, "order", strlen("order")) == 0)
        return FS_CMD_RWORDER;
    else if (strncmp(cmd, "mount", strlen("mount")) == 0)
        return FS_CMD_MOUNT;
    else if (strncmp(cmd, "umount", strlen("umount")) == 0)
        return FS_CMD_UMOUNT;
    else if (strncmp(cmd, "show", strlen("show")) == 0)
        return FS_CMD_SHOW;

    return FS_CMD_UNKNOWN;
}

static void utest_fs_usage(void)
{
    eshell_putstring("Usage: Basic interface test of file system.\r\n"
                     "       The param size range is between %d and %d.\r\n",
         MIN_BUFF_SIZE, MAX_BUFF_SIZE);
    eshell_putstring("  utest_fs   rate size - Rate test about write and read\r\n");
    eshell_putstring("            exact size - Conformance test about write and read\r\n");
    eshell_putstring("             rand time - Random test about write and read\r\n");
    eshell_putstring("           cyclic time - Cyclic test about write and read\r\n");
    eshell_putstring("            order time - Order test about write and read\r\n");
    eshell_putstring("            show  path - Show file content\r\n");
    eshell_putstring("            mount name - mount fs\r\n");
    eshell_putstring("           umount name - umount fs\r\n");
    mbed_fs_name();
}

static void utest_fs(int argc, char *argv[])
{
    if (argc != 3) {
        utest_fs_usage();
        return;
    }

    char *cmd = argv[1];
    int32_t type = utest_fs_cmd(cmd);
    switch (type)
    {
        int32_t len;
        char *name;
        case FS_CMD_RWRATE:
            len = atoi(argv[2]);
            utest_fs_rate(len);
            break;

        case FS_CMD_RWEXACT:
            len = atoi(argv[2]);
            utest_fs_exact(len);
            break;

        case FS_CMD_RWCYCLIC:
            len = atoi(argv[2]);
            utest_fs_cyclic(len);
            break;

        case FS_CMD_RWRANDOM:
            len = atoi(argv[2]);
            utest_fs_random(len);
            break;

        case FS_CMD_RWORDER:
            len = atoi(argv[2]);
            utest_fs_order(len);
            break;

        case FS_CMD_MOUNT:
            name = argv[2];
            utest_fs_mount(name);
            break;

        case FS_CMD_UMOUNT:
            name = argv[2];
            utest_fs_umount(name);
            break;

        case FS_CMD_SHOW:
            name = argv[2];
            utest_fs_show_file(name);
            break;

        default:
            goto usage;
    }

    return;
usage:
    utest_fs_usage();
    return;
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_fs", "Testing file system performance.",
                   utest_fs);

#endif
#endif
