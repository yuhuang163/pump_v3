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

typedef enum fs_cmd {
    FS_CMD_UNKNOWN = 0,
    FS_CMD_RM = 1,
    FS_CMD_MKFILE = 2,
    FS_CMD_MKDIR = 3,
    FS_CMD_LS = 4,
    FS_CMD_WRITE = 5,
    FS_CMD_READ = 6,
    FS_CMD_MOUNT = 7,
    FS_CMD_UMOUNT = 8,
    FS_CMD_STATFS = 9,
    FS_CMD_RENAME = 10
} fs_cmd_e;

int32_t fs_cmd_statfs(char *path)
{
    int32_t ret;
    uint64_t max_cap = 0;
    uint64_t free_cap = 0;

    struct statfs stat;
    memset(&stat, 0x00, sizeof(stat));
    ret = mbed_statfs(path, &stat);
    if (ret != 0) {
        eshell_putstring("%s: statfs fail, path:%s res:%d", __FUNCTION__, path, ret);
        return -1;
    }

    max_cap = (uint64_t)stat.f_blocks * (uint64_t)stat.f_bsize;
    free_cap = (uint64_t)stat.f_bavail * (uint64_t)stat.f_bsize;
    uint32_t max_M;
    uint32_t max_B;
    uint32_t free_M;
    uint32_t free_B;
    max_M = max_cap / (1024 * 1024);
    max_B = max_cap % (1024 * 1024);
    free_M = free_cap / (1024 * 1024);
    free_B = free_cap % (1024 * 1024);
    eshell_putstring("%s: space: max:%u(mb)%u(byte) free:%u(mb)%u(byte)",
        __FUNCTION__, max_M, max_B, free_M, free_B);
    return 0;
}

int32_t fs_cmd_stat(char *path)
{
    int32_t ret;
    uint32_t type;
    char buff[128] = {'\0'};
    struct stat result;
    struct tm *tm = NULL;

    memset(&result, 0x00, sizeof(result));
    ret = stat(path, &result);
    if (ret != 0) {
        eshell_putstring("%s: file not exist err, path:%s\r\n", __FUNCTION__, path);
        return DT_UNKNOWN;
    }

    if (S_ISDIR(result.st_mode))
        type = DT_DIR;
    else
        type = DT_REG;

    tm = localtime(&result.st_mtime);
    snprintf(buff, sizeof(buff)-1, "%04d/%02d/%02d-%02d:%02d:%02d",
        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    if (type == DT_DIR)
        eshell_putstring("%s: d %s %06u %s\r\n", __FUNCTION__, buff, result.st_size, path);
    else
        eshell_putstring("%s: f %s %06u %s\r\n", __FUNCTION__, buff, result.st_size, path);

    return type;
}

void fs_cmd_readdir(char *path)
{
    DIR *dir = NULL;
    struct dirent *dir_entry= NULL;
    char ndir[PATH_MAX + 1] = {'\0'};

    dir = opendir(path);
    if (dir == NULL) {
        eshell_putstring("%s: opendir fail, path:%s", __FUNCTION__, path);
        return;
    }

    while ((dir_entry = readdir(dir)) != NULL) {
       if (dir_entry->d_name[0] == '\0' || strlen(dir_entry->d_name) <= 0) {
           continue;
       }
       if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) {
           continue;
       }
       memset(ndir, 0x00, sizeof(ndir));
       sprintf(ndir, "%s/", path);
       strcat(ndir, dir_entry->d_name);
       fs_cmd_stat(ndir);
       if (dir_entry->d_type == DT_DIR) {
            //fs_cmd_readdir(ndir);
       }
    }

    closedir(dir);
}

static int fs_cmd_ls(char *path)
{
    if (path == NULL || path[0] == '\0') {
        eshell_putstring("%s: ls fail, param err\r\n", __FUNCTION__);
        return -1;
    }

    int ret = 0;
    ret = fs_cmd_stat(path);
    if (ret == DT_REG) {
        return 0;
    }

    fs_cmd_readdir(path);
    return 0;
}

static int fs_read_data(char *path, uint32_t offset, char *buff, uint32_t total_len)
{
    FILE *fp = NULL;
    fp = fopen(path, "ab+");
    if (fp == NULL) {
        eshell_putstring("%s: fopen fail, path:%s\r\n", __FUNCTION__, path);
        return -1;
    }

    int32_t ret;
    ret = fseek(fp, offset, SEEK_SET);
    if (ret != 0) {
        fclose(fp);
        eshell_putstring("%s: fseek fail, path:%s res:%d errno:%d\r\n",
            __FUNCTION__, path, ret);
        return -1;
    }

    ret = fread(buff, 1, total_len, fp);
    if (ret < 0) {
        eshell_putstring("%s: fread fail, path:%s res:%d errno:%d\r\n",
            __FUNCTION__, path, ret);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return ret;
}

static int fs_write_data(char *path, uint32_t offset, char *buff, uint32_t total_len)
{
    FILE *fp = NULL;
    fp = fopen(path, "ab+");
    if (fp == NULL) {
        eshell_putstring("%s: fopen fail, path:%s\r\n", __FUNCTION__, path);
        return -1;
    }

    int32_t ret;
    ret = fseek(fp, offset, SEEK_SET);
    if (ret != 0) {
        fclose(fp);
        eshell_putstring("%s: fseek fail, path:%s res:%d\r\n", __FUNCTION__, path, ret);
        return -1;
    }

    ret = fwrite(buff,1, total_len, fp);
    if (ret < 0) {
        eshell_putstring("%s: fwrite fail, path:%s res:%d\r\n", __FUNCTION__, path, ret);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return ret;
}

static int fs_cmd_read(char *path, uint32_t offset, char *buff, uint32_t total_len)
{
    uint32_t read_len = 0;
    uint32_t single_size = 1024;
    uint32_t srclen = total_len;

    memset(buff, 0x00, total_len);
    while (total_len > 0) {
        int32_t res = 0;
        int32_t tmp = 0;
        tmp = total_len > single_size ? single_size : total_len;
        res = fs_read_data(path, offset + read_len, buff + read_len, tmp);
        if (res <= 0) {
            break;
        }
        total_len -= res;
        read_len += res;
    }

    if (read_len == srclen)
        eshell_putstring("%s: fread pass, path:%s len:%d\r\n", __FUNCTION__, path, read_len);
    else
        eshell_putstring("%s: fread fail, path:%s len:%d\r\n", __FUNCTION__, path, read_len);
    return read_len;
}

static int fs_cmd_write(char *path, uint32_t offset, char *buff, uint32_t total_len)
{
    uint32_t write_len = 0;
    uint32_t single_size = 1024;
    uint32_t srclen = total_len;

    while (total_len > 0) {
        int32_t res = 0;
        int32_t tmp = 0;
        tmp = total_len > single_size ? single_size : total_len;
        res = fs_write_data(path, offset + write_len, buff + write_len, tmp);
        if (res <= 0) {
            break;
        }
        total_len -= res;
        write_len += res;
    }

    if (write_len == srclen)
        eshell_putstring("fwrite pass, path:%s len:%d\r\n", path, write_len);
    else
        eshell_putstring("fwrite pass, path:%s len:%d\r\n", path, write_len);
    return write_len;
}

static void fs_cmd_mkdir(char *path)
{
    if (path == NULL || path[0] == '\0') {
        eshell_putstring("%s: mkdir fail, param err\r\n", __FUNCTION__);
        return;
    }

    int res;
    res = mkdir(path, 0777);
    if (res != 0) {
        eshell_putstring("%s: mkdir fail, path:%s\r\n", __FUNCTION__, path);
        return;
    }

    fs_cmd_stat(path);
    eshell_putstring("%s: mkdir pass, path:%s\r\n", __FUNCTION__, path);
}

static int fs_cmd_mkfile(char *path)
{
    if (path == NULL || path[0] == '\0') {
        eshell_putstring("%s: mkfile fail, param err\r\n", __FUNCTION__);
        return -1;
    }

    int32_t ret;
    struct stat result;
    memset(&result, 0x00, sizeof(result));
    ret = stat(path, &result);
    if (ret == 0) {
        fs_cmd_stat(path);
        eshell_putstring("%s: mkfile pass, path:%s\r\n", __FUNCTION__, path);
        return 0;
    }

    FILE *fp = NULL;
    fp = fopen(path, "ab+");
    if (fp == NULL) {
        eshell_putstring("%s: fopen fail, path:%s\r\n", __FUNCTION__, path);
        return -1;
    }

    fclose(fp);
    fs_cmd_stat(path);
    eshell_putstring("%s: mkfile pass, path:%s\r\n", __FUNCTION__, path);
    return 0;
}

static void fs_cmd_remove(char *path)
{
    if (path == NULL || path[0] == '\0') {
        eshell_putstring("%s: remove fail, param err\r\n", __FUNCTION__);
        return;
    }

    int res;
    res = remove(path);
    if (res != 0) {
        eshell_putstring("%s: remove fail, path:%s res:%d\r\n", __FUNCTION__, path, res);
        return;
    }

    eshell_putstring("%s: remove pass, path:%s\r\n", __FUNCTION__, path);
}

static void fs_cmd_rename(char *path, char *dst)
{
    if (path == NULL || path[0] == '\0') {
        eshell_putstring("%s: rename fail, param err\r\n", __FUNCTION__);
        return;
    }

    if (dst == NULL || dst[0] == '\0') {
        eshell_putstring("%s: rename fail, param err\r\n", __FUNCTION__);
        return;
    }

    int res;
    res = rename(path, dst);
    if (res != 0) {
        eshell_putstring("%s: rename fail, path:%s->%s res:%d\r\n",
            __FUNCTION__, path, dst, res);
        return;
    }

    eshell_putstring("%s: rename pass, path:%s->%s\r\n", __FUNCTION__, path, dst);
}

static int fs_cmd_mount(char *name)
{
    int res;
    res = mbed_fs_mount(name);
    if (res != 0 ) {
        eshell_putstring("%s: mount fail\r\n", __FUNCTION__);
        return res;
    }
    eshell_putstring("%s: mount pass\r\n", __FUNCTION__);
    return 0;
}

static int fs_cmd_umount(char *name)
{
    int res;
    res = mbed_fs_umount(name);
    if (res != 0 ) {
        eshell_putstring("%s: umount fail\r\n", __FUNCTION__);
        return res;
    }
    eshell_putstring("%s: umount pass\r\n", __FUNCTION__);
    return 0;
}

static void fs_cmd_tools_usage(void)
{
    eshell_putstring("Usage: Basic file system command set.\r\n");
    eshell_putstring("             fs    rm path - rm dir or path\r\n");
    eshell_putstring("                touch path - create a new file\r\n");
    eshell_putstring("                mkdir path - create a new dir\r\n");
    eshell_putstring("                   ls path - show dir or file info\r\n");
    eshell_putstring("                write path addr offset size - Specify file write\r\n");
    eshell_putstring("                 read path addr offset size - Specify file read \r\n");
    eshell_putstring("               rename path1 path2 - rename file or dir\r\n");
    eshell_putstring("               statfs path - file system info\r\n");
    eshell_putstring("                mount name - mount fs\r\n");
    eshell_putstring("               umount name - umount fs\r\n");
    mbed_fs_name();
}

static int32_t fs_cmd_type(char *cmd)
{
    if (cmd == NULL || strlen(cmd) == 0) {
        return FS_CMD_UNKNOWN;
    }

    if (strncmp(cmd, "mkdir", strlen("mkdir")) == 0)
        return FS_CMD_MKDIR;
    else if (strncmp(cmd, "touch", strlen("touch")) == 0)
        return FS_CMD_MKFILE;
    else if (strncmp(cmd, "rm", strlen("rm")) == 0)
        return FS_CMD_RM;
    else if (strncmp(cmd, "ls", strlen("ls")) == 0)
        return FS_CMD_LS;
    else if (strncmp(cmd, "write", strlen("write")) == 0)
        return FS_CMD_WRITE;
    else if (strncmp(cmd, "read", strlen("read")) == 0)
        return FS_CMD_READ;
    else if (strncmp(cmd, "mount", strlen("mount")) == 0)
        return FS_CMD_MOUNT;
    else if (strncmp(cmd, "umount", strlen("umount")) == 0)
        return FS_CMD_UMOUNT;
    else if (strncmp(cmd, "rename", strlen("rename")) == 0)
        return FS_CMD_RENAME;
    else if (strncmp(cmd, "statfs", strlen("statfs")) == 0)
        return FS_CMD_STATFS;

    return FS_CMD_UNKNOWN;
}

static void fs_cmd_tools(int argc, char *argv[])
{
    if (argc <= 2) {
        fs_cmd_tools_usage();
        return;
    }

    char *cmd = argv[1];
    int32_t type = fs_cmd_type(cmd);
    switch (type)
    {
        char *path;
        uint64_t off;
        uint32_t len;
        char *buff;
        char *dst;
        uint32_t addr;

        case FS_CMD_MKDIR:
           if (argc != 3) {
                goto err;
           }
           path = argv[2];
           fs_cmd_mkdir(path);
           break;

        case FS_CMD_MKFILE:
            if (argc != 3) {
                goto err;
            }
            path = argv[2];
            fs_cmd_mkfile(path);
            break;

        case FS_CMD_RM:
            if (argc != 3) {
                goto err;
            }
            path = argv[2];
            fs_cmd_remove(path);
            break;

        case FS_CMD_LS:
            if (argc != 3) {
                goto err;
            }
            path = argv[2];
            fs_cmd_ls(path);
            break;

        case FS_CMD_READ:
            if (argc != 6) {
                goto err;
            }
            path = argv[2];
            addr = strtol(argv[3], NULL, 16);
            buff = (char*)addr;
            off = atoll(argv[4]);
            len = atoi(argv[5]);
            fs_cmd_read(path, off, buff, len);
            break;

        case FS_CMD_WRITE:
            if (argc != 6) {
                goto err;
            }
            path = argv[2];
            addr = strtol(argv[3], NULL, 16);
            buff = (char*)addr;
            off = atoll(argv[4]);
            len = atoi(argv[5]);
            fs_cmd_write(path, off, buff, len);
            break;

        case FS_CMD_MOUNT:
            if (argc != 3) {
                goto err;
            }
            path = argv[2];
            fs_cmd_mount(path);
            break;

        case FS_CMD_UMOUNT:
            if (argc != 3) {
                goto err;
            }
            path = argv[2];
            fs_cmd_umount(path);
            break;

        case FS_CMD_RENAME:
            if (argc != 4) {
                goto err;
            }
            path = argv[2];
            dst = argv[3];
            fs_cmd_rename(path, dst);
            break;

        case FS_CMD_STATFS:
            if (argc != 3) {
                goto err;
            }
            path = argv[2];
            fs_cmd_statfs(path);
            break;

        default:
            fs_cmd_tools_usage();
            break;
    }

    return;
err:
    fs_cmd_tools_usage();
    return;
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "fs", "Basic file system command set",
    fs_cmd_tools);

#endif
#endif //UTILS_ESHELL_EN
