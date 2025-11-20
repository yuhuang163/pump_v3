/***************************************************************************
 * Copyright 2015-2024 BES.
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
 ***************************************************************************/
#ifndef _MTP_FS_API_H_
#define _MTP_FS_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MTP_DT_REG 0x01
#define MTP_DT_DIR 0x02

int mtp_fs_closedir(void *dir);
void *mtp_fs_opendir(const char *srcdir);
int mtp_fs_readdir(void *pdir, char *name, int nlen, uint32_t *type);
int mtp_fs_stat(const char *srcdir, uint64_t *modified, uint32_t *size);
int mtp_fs_mkdir(const char *srcdir);
int mtp_fs_remove(const char *path);
int mtp_fs_rename(const char *oldpath, const char *newpath);
int mtp_fs_write(const char *path, const char *buff, uint32_t offset, uint32_t len);
int mtp_fs_read(const char *path, char *buff, uint32_t offset, uint32_t len);
int mtp_fs_statfs(const char *srcdir, uint64_t *max_capacity, uint64_t *free);
bool mtp_fs_file_exist(const char *srcdir);
int mtp_fs_fwrite(void *fp, const char *buff, uint32_t offset, uint32_t len);
int mtp_fs_fread(void *fp, char *buff, uint32_t offset, uint32_t len);
void *mtp_fs_fopen(const char *path);
void mtp_fs_fclose(void *fp);
void mtp_fs_fseek(void *fp, uint32_t offset);

#ifdef __cplusplus
}
#endif

#endif
