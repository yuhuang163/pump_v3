/***************************************************************************
 *
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
 *
 ****************************************************************************/

#include "string.h"
#include "plat_types.h"
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_jpeg.h"
#include "hal_cache.h"
#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_chipid.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "qlook.h"
#include "qetable.h"
#include "hetable.h"
#include "jpeg_drv.h"
#include "yuvconv.h"

//#define JPEG_DEBUG
#define JPEG_LOG(...)           TR_INFO( TR_MOD(HAL), "//jpeg_drv: "  __VA_ARGS__)
#ifdef JPEG_DEBUG
#define JPEG_DBG(...)           TR_INFO( TR_MOD(HAL), "//jpeg_drv: "  __VA_ARGS__)
#define JPEG_DUMP8(data, len) \
    do { \
        for (int ii = 0; ii < len; ii += 16) \
            DUMP8("%02x,", data + ii, 16); \
    } while (0)
#else
#define JPEG_DBG(...)
#define JPEG_DUMP8(data, len)
#endif

#define JPEG_WAIT_MS            (200) //ms

#define JPEG_CHECK_ALIGN(x)     ((uint32_t)x & 0x3f) //64 bytes

#define GETC(p)                 ((*p++) & 0xff)
#define PUTC(p, c) \
    do { \
        *p++ = (c) & 0xff; \
    } while (0)
#define PUTH(p, v) \
    do { \
        *p++ = (v >> 8) & 0xff; \
        *p++ = (v >> 0) & 0xff; \
    } while(0)

//#define JPEG_DUMP_ECS

#define HBASE_BUF_SZ            64
#define HSYMB_BUF_SZ            336
#define HMIN_BUF_SZ             64
#define QTABLE_BUF_SZ           256

#define QTABLE_MAX_COUNT        256
#define HTABLE_MAX_SIZE         192

enum JPEG_HTABLE_TYPE {
    JPEG_HTABLE_DC = 0,
    JPEG_HTABLE_AC,
};

struct jpeg_htable_t {
    uint8_t id;
    uint8_t type;
    uint8_t code[16];
    uint8_t data[HTABLE_MAX_SIZE];
    uint8_t size;
};

struct jpeg_info_t {
    uint16_t width;
    uint16_t height;
    enum JPEG_FORMAT format;

    uint8_t *ecs_data;
    uint32_t ecs_size;

    uint8_t  qtable_precision;          /* 0: 8bits, 1: 16 bites */
    uint32_t qtables[QTABLE_MAX_COUNT]; /* 0~63: table 0, 64~127: table 1*/

    struct jpeg_htable_t htables[4];    /* 0: DC table 0, 1: AC table 0,
                                         * 2: DC table 1, 3: AC table 1, */

    uint8_t color_qtable[3];
    uint8_t color_htable_ac[3];
    uint8_t color_htable_dc[3];

    uint8_t color_vsample[3];
    uint8_t color_hsample[3];

    int rst_nmcu;
};

POSSIBLY_UNUSED
static const char yuvstr[3][8] = {
    "YUV422", "YUV420", "YUV444",
};

struct jpeg_task_t {
    enum HAL_JPEG_ID_T codec_id;

    bool opened;
    bool busy;

#ifdef RTOS
    osSemaphoreId_t sem;
    osMutexId_t mutex;
#endif

    /* jpeg file data struct */
    struct jpeg_info_t info;

    /* buffers for jpeg decoding */
    int hbase_buf[HBASE_BUF_SZ];
    int hsymb_buf[HSYMB_BUF_SZ];
    int hmin_buf[HMIN_BUF_SZ];
    int qtable_buf[QTABLE_BUF_SZ];
};

static struct jpeg_task_t jpeg_task[HAL_JPEG_ID_QTY] = {
    {
        .codec_id = HAL_JPEG_ID_0,
        .opened = false,
        .busy = false,
#ifdef RTOS
        .sem = NULL,
        .mutex = NULL,
#endif
    },
#ifdef JPEG1_BASE
    {
        .codec_id = HAL_JPEG_ID_1,
        .opened = false,
        .busy = false,
#ifdef RTOS
        .sem = NULL,
        .mutex = NULL,
#endif
    },
#endif
};

static int jpeg_task_count = sizeof(jpeg_task) / sizeof(struct jpeg_task_t);

/***********************************************************************************/

POSSIBLY_UNUSED
static char *marker_str(int c)
{
    switch (c) {
        case 0xc0:
        case 0xc1:
        case 0xc2:
        case 0xc3:
        case 0xc5:
        case 0xc6:
        case 0xc7:
        case 0xc9:
        case 0xca:
        case 0xcb:
        case 0xcd:
        case 0xce:
        case 0xcf:
            return "SOF";
        case 0xc4:
            return "DHT";
        case 0xc8:
            return "EXT";
        case 0xcc:
            return "DAC";
        case 0xd0:
        case 0xd1:
        case 0xd2:
        case 0xd3:
        case 0xd4:
        case 0xd5:
        case 0xd6:
        case 0xd7:
            return "RST";
        case 0xd8:
            return "SOI";
        case 0xd9:
            return "EOI";
        case 0xda:
            return "SOS";
        case 0xdb:
            return "DQT";
        case 0xdd:
            return "DRI";
        case 0xe0:
        case 0xe1:
        case 0xe2:
        case 0xe3:
        case 0xe4:
        case 0xe5:
        case 0xe6:
        case 0xe7:
        case 0xe8:
        case 0xe9:
        case 0xea:
        case 0xeb:
        case 0xec:
        case 0xed:
        case 0xee:
        case 0xef:
            return "APP";
        case 0xfe:
            return "COM";
        default:
            return "UNKNOWN";
    }
}

/**
 * @brief Load jpeg file data to jpeg data struct.
 *
 * @param info jpeg data struct;
 * @param jpeg_data jpeg file data buffer;
 * @param jpeg_size jpeg file data size;
 *
 * @return JPEG_OK for OK, other for failed;
 */
static int jpeg_load_file(struct jpeg_info_t *info, uint8_t *jpeg_data, uint32_t jpeg_size)
{
    int v;
    int rst = 0, nrst = 0;
    uint8_t *ecs = info->ecs_data;
    uint32_t ecs_sz = 0;
    uint32_t *qtable = info->qtables;
    int qtable_count = 0;
    uint8_t *p = jpeg_data;
    uint32_t sos_count = 0;

    /* Start of Image */
    if (p[0] != 0xff || p[1] != 0xd8) {
        JPEG_LOG("ERROR: Not jpeg file.");
        return -1;
    }

    JPEG_LOG("Load jpeg file.");

    while (p < (jpeg_data + jpeg_size)) {
        int m, len;

        if (!(p[0] == 0xff && p[1] > 0 && p[1] < 0xff)) {
            p++;
            continue;
        }

        m = p[1];
        JPEG_DBG("==== Marker 0x%02x - %s", m, marker_str(m));

        if (m >= 0xc0 && m <= 0xc3) { /* Start of frame */
            int bpp, color_num;

            len = (p[2] << 8 | p[3]);
            JPEG_DBG("  Segment Size: %d", len);

            p += 4;
            bpp = GETC(p);
            if (bpp != 8) {
                JPEG_LOG("ERROR: Only support 8 bits per pixel.");
                return -1;
            }

            v = GETC(p);
            info->height = (v << 8) | GETC(p);
            v = GETC(p);
            info->width = (v << 8) | GETC(p);

            color_num = GETC(p);
            if (color_num != 3) {
                JPEG_LOG("ERROR: Only support jpeg file with YUV(YCbCr) format.");
                return -1;
            }

            JPEG_LOG("  Width:          %d", info->width);
            JPEG_LOG("  Height:         %d", info->height);
            JPEG_LOG("  Colors:         %d", color_num);
            for (int i = 0; i < color_num; i++) {
                v = GETC(p);
                if (v == 1) {
                    JPEG_DBG("    ID:           0x%02x (Y)", v);
                } else if (v == 2) {
                    JPEG_DBG("    ID:           0x%02x (Cb)", v);
                } else if (v == 3) {
                    JPEG_DBG("    ID:           0x%02x (Cr)", v);
                } else {
                    JPEG_LOG("ERROR: invalid color ID [0x%02x] in SOF.", v);
                    return -1;
                }
                v = GETC(p);
                info->color_vsample[i] = v & 0xf;
                info->color_hsample[i] = (v >> 4) & 0xf;
                JPEG_DBG("    V_Sample:     %d", info->color_vsample[i]);
                JPEG_DBG("    H_Sample:     %d", info->color_hsample[i]);
                info->color_qtable[i] = GETC(p);
                JPEG_DBG("    QTable:       %d", info->color_qtable[i]);
            }
        } else if ((m >= 0xc5 && m <= 0xc7) || (m >= 0xc9 && m <= 0xcb) || (m >= 0xcd && m <= 0xcf)) { /* other SOFs */
            len = (p[2] << 8 | p[3]);
            JPEG_DBG("  Segment Size: %d", len);
            p += len + 2;;

        } else if (m == 0xc4) { /* H table */
            uint8_t id, type;
            struct jpeg_htable_t *htable;
            uint8_t *head;

            len = (p[2] << 8 | p[3]) - 2;
            JPEG_DBG("  Segment Size: %d", len + 2);
            p += 4;
            head = p;

            while (p < (head + len)) {
                v = GETC(p);
                id = v & 0xf;
                type = v >> 4;
                if (id > 1 || type > 1) {
                    JPEG_LOG("ERROR: Invalid htable id [%d] or type[%d].", id, type);
                    return -1;
                }

                v = (id << 1) | type;
                htable = &(info->htables[v]);
                htable->id = id;
                htable->type = type;

                JPEG_DBG("  Huffman Table:");
                JPEG_DBG("    Type:         %s (%d)", type ? "AC" : "DC", type);
                JPEG_DBG("    Identifier:   %d", id);
                JPEG_DBG("    Code:");
                JPEG_DUMP8(p, 16);
                memcpy(htable->code, p, 16);
                p += 16;

                /* calculate data size */
                htable->size = 0;
                for (int i = 0; i < 16; i++)
                    htable->size += htable->code[i];
                if (htable->size > (len - (p - head))) {
                    JPEG_LOG("ERROR: Failed to copy htable data, need %d bytes, only %d bytes.",
                            htable->size, (int)(len - (p - head)));
                    return -1;
                }
                if (htable->size > HTABLE_MAX_SIZE) {
                    JPEG_LOG("ERROR: htable data size is larger, %d bytes, max %d bytes.",
                            htable->size, HTABLE_MAX_SIZE);
                    return -1;
                }

                JPEG_DBG("    Data Size:    %d", htable->size);
                JPEG_DBG("    Data Dump:");
                JPEG_DUMP8(p, htable->size);
                memcpy(htable->data, p, htable->size);
                p += htable->size;
            }

        } else if (m == 0xc8) {
            JPEG_LOG("ERROR: JPG extensions!");
            return JPEG_FAILED;

        } else if (m == 0xcc) {
            JPEG_LOG("ERROR: arithmatic coding not supported!");
            return JPEG_FAILED;

        } else if (m >= 0xd0 && m <= 0xd7) {
            p += 2;
            rst++;

        } else if (m == 0xd8) { /* SOI */
            JPEG_DBG("Start of Image.");
            p += 2;

        } else if (m == 0xd9) { /* End of image */
            JPEG_DBG("#### ECS data start [%d].", ecs_sz);
            ecs[ecs_sz++] = 0xff;
            ecs[ecs_sz++] = 0xd9;
            JPEG_DBG("#### ECS data end   [%d].", ecs_sz);
#ifdef JPEG_DUMP_ECS
            JPEG_DBG("#### ECS Data dump:");
            JPEG_DUMP8(ecs, ecs_sz);
#endif
            JPEG_DBG("End of Image.");
            break;

        } else if (m == 0xda) { /* Start of Scans */
            len = (p[2] << 8 | p[3]) - 2;
            JPEG_DBG("  Segment Size: %d", len + 2);
            p += 4;
            if (len == 10 || len == 8 || len == 6) {
                int color_num = GETC(p);
                JPEG_DBG("  Colors:         %d", color_num);
                for (int i = 0; i < color_num; i++) {
                    int n = GETC(p);
                    if (n == 1) {
                        JPEG_DBG("    ID:           0x%02x (Y)", n);
                    } else if (n == 2) {
                        JPEG_DBG("    ID:           0x%02x (Cb)", n);
                    } else if (n == 3) {
                        JPEG_DBG("    ID:           0x%02x (Cr)", n);
                    } else {
                        JPEG_LOG("ERROR: invalid color ID [0x%02x] in SOS.", n);
                        return -1;
                    }
                    v = GETC(p);
                    n--;
                    info->color_htable_dc[n] = v >> 4;
                    info->color_htable_ac[n] = v & 0xf;
                    JPEG_DBG("    HTable:       DC %d, AC %d", info->color_htable_dc[n],
                                    info->color_htable_ac[n]);
                }

                JPEG_DBG("    Spectrum:     %02x, %02x, %02x", p[0], p[1], p[2]);
                p += 3;
            } else {
                p += len;
            }
            JPEG_DBG("#### ECS data start [%d].", ecs_sz);
            while (p < (jpeg_data + jpeg_size)) {
                if (p[0] == 0xff) {
                    if (p[1] != 0xff && p[1] >= 0xc0 && (p[1] & 0xf8) != 0xd0) {
                        break;
                    } else {
                        ecs[ecs_sz++] = 0xff;
                        ecs[ecs_sz++] = p[1];
                        if ((p[1] & 0xf8) == 0xd0)
                            rst++;
                        p += 2;
                    }
                } else {
                    ecs[ecs_sz++] = GETC(p);
                }
            }
            JPEG_DBG("#### ECS data end   [%d].", ecs_sz);
            if (ecs_sz > 0)
                sos_count++;

        } else if (m == 0xdb) { /* Q table */
            uint8_t preci, id;
            uint8_t *head;

            len = (p[2] << 8 | p[3]) - 2;
            JPEG_DBG("  Segment Size: %d", len + 2);
            p += 4;
            head = p;

            while (p < (head + len)) {
                v = GETC(p);
                preci = v >> 4;
                id = v & 0xf;
                JPEG_DBG("  Quantization Table:");
                JPEG_DBG("    Precision:    %d", preci);
                JPEG_DBG("    Identifier:   %d", id);

                if (id > 3 || preci > 1) {
                    JPEG_LOG("ERROR: invalid id [%d] or precision [%d], valid value 0/1.", id, preci);
                    return -1;
                }
                info->qtable_precision = preci;

                JPEG_DBG("    Data Size:    64");
                JPEG_DBG("    Data Dump:");
                JPEG_DUMP8(p, 64 * (preci + 1));

                for (int i = 0; i < 64; i++) {
                    if (preci) {
                        qtable[qtable_count++] = p[i] << 8 | p[i + 1];
                        i++;
                    } else {
                        qtable[qtable_count++] = p[i];
                    }
                    if (qtable_count >= QTABLE_MAX_COUNT)
                        break;
                }
                p += 64 * (preci + 1);
            }

        } else if (m == 0xdd) { /* Restart interval */
            len = (p[2] << 8 | p[3]) - 2;
            JPEG_DBG("  Segment Size: %d", len + 2);
            p += 4;

            nrst = p[0] << 8 | p[1];
            JPEG_DBG("  MCU numbers in restart interval: %d", nrst);
            p += len;

        } else if (m >= 0xe0 && m <= 0xef) { /* APPs */
            len = (p[2] << 8 | p[3]) - 2;
            JPEG_DBG("  Segment Size: %d", len + 2);
            p += 4;
            if (p[0] == 0x4a && p[1] == 0x46 && p[2] == 0x49 && p[3] == 0x46 && p[4] == 0) {
                JPEG_DBG("  Identifier: JFIF0");
                JPEG_DBG("  Version:    %d.%d", p[5], p[6]);
            } else if (p[0] == 0x45 && p[1] == 0x78 && p[2] == 0x69 && p[3] == 0x66 && p[4] == 0) {
                JPEG_DBG("  Identifier: Exif0");
            } else {
                JPEG_DBG("WARN: unkown marker, ignore.");
            }
            p += len;

        } else if (m == 0xfe) { /* Comments */
            len = (p[2] << 8 | p[3]);
            JPEG_DBG("  Segment Size: %d", len);
            p += len + 2;
        } else {
            JPEG_DBG("  Unkown marker, Ignored!");
            p += 2;
        }
    }
    info->ecs_size = ecs_sz;
    if (rst > 0 && nrst > 0)
        info->rst_nmcu = nrst;
    else
        info->rst_nmcu = 0;

    if (info->color_hsample[0] == 1 && info->color_vsample[0] == 1 &&
        info->color_hsample[1] == 1 && info->color_vsample[1] == 1 &&
        info->color_hsample[2] == 1 && info->color_vsample[2] == 1) {
        info->format = JPEG_YUV444;
    } else if (info->color_hsample[0] == 2 && info->color_vsample[0] == 2 &&
               info->color_hsample[1] == 1 && info->color_vsample[1] == 1 &&
               info->color_hsample[2] == 1 && info->color_vsample[2] == 1) {
        info->format = JPEG_YUV420;
    } else if (info->color_hsample[0] == 2 && info->color_vsample[0] == 1 &&
               info->color_hsample[1] == 1 && info->color_vsample[1] == 1 &&
               info->color_hsample[2] == 1 && info->color_vsample[2] == 1) {
        info->format = JPEG_YUV422;
    } else {
        JPEG_LOG("Unknown or unsupported format, Abort!");
        return -1;
    }

    if (sos_count > 1) {
        JPEG_LOG("File with multi SOS segments, not supported!");
        return -1;
    }

    JPEG_LOG("Load file OK.");
    JPEG_DBG("  Size:     %dx%d", info->width, info->height);
    JPEG_DBG("  Format:   %s", yuvstr[info->format]);
    JPEG_DBG("  Restart:  %d", info->rst_nmcu);
    JPEG_DBG("  ECS:      %d bytes", info->ecs_size);
    JPEG_DBG("  Y:        QT %d, HAT %d, HDT %d, Sample %dx%d",
                info->color_qtable[0], info->color_htable_ac[0], info->color_htable_dc[0],
                info->color_hsample[0], info->color_vsample[0]);
    JPEG_DBG("  U(Cb):    QT %d, HAT %d, HDT %d, Sample %dx%d",
                info->color_qtable[1], info->color_htable_ac[1], info->color_htable_dc[1],
                info->color_hsample[1], info->color_vsample[1]);
    JPEG_DBG("  V(Cr):    QT %d, HAT %d, HDT %d, Sample %dx%d",
                info->color_qtable[2], info->color_htable_ac[2], info->color_htable_dc[2],
                info->color_hsample[2], info->color_vsample[2]);
    return JPEG_OK;
}

/**
 * @brief Save jpeg data to jpeg file data buffer.
 *
 * @param info jpeg data struct;
 * @param jpeg_data jpeg file data buffer;
 * @param jpeg_size jpeg file data size pointer;
 *
 * @return JPEG_OK for OK, other for failed;
 */
static int jpeg_save_file(struct jpeg_info_t *info, uint8_t *jpeg_data, uint32_t *jpeg_size)
{
    uint8_t *p = jpeg_data;

    if (!info)
        return -1;
    if (info->width == 0 || info->height == 0 || info->ecs_size == 0)
        return -1;
    if (info->format != JPEG_YUV422 && info->format != JPEG_YUV420 &&
        info->format != JPEG_YUV444)
        return -1;

    JPEG_LOG("Save jpeg file.");

    /* SOI, Start of image */
    PUTH(p, 0xffd8);

    /* APP0 */
    PUTH(p, 0xffe0);
    PUTH(p, 0x0010); /* 16 bytes */
    PUTC(p, 0x4a); /* 'J' */
    PUTC(p, 0x46); /* 'F' */
    PUTC(p, 0x49); /* 'I' */
    PUTC(p, 0x46); /* 'F' */
    PUTC(p, 0x00); /* 0 */
    PUTH(p, 0x0101); /* 1.1 version */
    PUTC(p, 0x00);
    PUTH(p, 0x0001);
    PUTH(p, 0x0001);
    PUTH(p, 0x0000);

    /* DQT, Quantization tables */
    for (int i = 0; i < 2; i++) {
        PUTH(p, 0xffdb);
        PUTH(p, 0x0043); /* 67 bytes */
        if (info->qtable_precision)
            PUTC(p, 0x10 | i);
        else
            PUTC(p, i);
        for (int j = 0; j < 64; j++) {
            if (info->qtable_precision)
                PUTH(p, info->qtables[i * 64 + j]);
            else
                PUTC(p, info->qtables[i * 64 + j]);
        }
    }

    /* SOF, Start of frame */
    PUTH(p, 0xffc0);
    PUTH(p, 0x0011); /* 17 bytes */
    PUTC(p, 0x08); /* bpp 8bits */
    PUTH(p, info->height);
    PUTH(p, info->width);
    PUTC(p, 0x03); /* 3 colors, YCbCr */
    /* Y */
    PUTC(p, 0x01);
    if (info->format == JPEG_YUV422)
        PUTC(p, 0x21);
    else if (info->format == JPEG_YUV420)
        PUTC(p, 0x22);
    else if (info->format == JPEG_YUV444)
        PUTC(p, 0x11);
    PUTC(p, info->color_qtable[0]);
    /* Cb */
    PUTC(p, 0x02);
    PUTC(p, 0x11);
    PUTC(p, info->color_qtable[1]);
    /* Cr */
    PUTC(p, 0x03);
    PUTC(p, 0x11);
    PUTC(p, info->color_qtable[2]);

    /* DHT, Huffman tables */
    for (int id = 0; id < 2; id++) {
        for (int type = 0; type < 2; type++) {
            int v = (id << 1) | type;
            struct jpeg_htable_t *htable = &(info->htables[v]);
            int len = htable->size + 16 + 3;
            PUTH(p, 0xffc4);
            PUTH(p, len);
            PUTC(p, (type << 4) | id);
            memcpy(p, htable->code, 16);
            p += 16;
            memcpy(p, htable->data, htable->size);
            p += htable->size;
        }
    }

    /* DRI */
    if (info->rst_nmcu > 0) {
        PUTH(p, 0xffdd);
        PUTH(p, 0x0004); /* 4 bytes */
        PUTH(p, info->rst_nmcu);
    }

    /* ECS header */
    PUTH(p, 0xffda);
    PUTH(p, 0x000c); /* 12 bytes */
    PUTC(p, 0x03); /* 3 colors, YCbCr */
    PUTC(p, 0x01); /* Y */
    PUTC(p, info->color_htable_dc[0] << 4 | info->color_htable_ac[0]); /* Y DC & AC */
    PUTC(p, 0x02); /* Cb */
    PUTC(p, info->color_htable_dc[1] << 4 | info->color_htable_ac[1]); /* Cb DC & AC */
    PUTC(p, 0x03); /* Cr */
    PUTC(p, info->color_htable_dc[2] << 4 | info->color_htable_ac[2]); /* Cr DC & AC */
    PUTH(p, 0x003f);
    PUTC(p, 0x00);

    /* ECS Data */
    for (int i = 0; i < info->ecs_size; i++) {
        if (info->ecs_data[i] == 0xff && info->ecs_data[i + 1] == 0xd9)
            break;
        PUTC(p, info->ecs_data[i]);
    }

    /* EOI, End of image */
    PUTH(p, 0xffd9);

    *jpeg_size = (p - jpeg_data);
    JPEG_LOG("Save file OK, file size %d bytes.", *jpeg_size);
    return JPEG_OK;
}

/**
 * @brief Convert huffman AC/DC tables to encode table.
 *
 * @param htables huffman AC/DC tables, load from jpeg file;
 * @param hetable data array (364 count) for hetable mem of JPEG Codec, valid in encoding;
 *
 * @return JPEG_OK for OK, other for failed;
 */
POSSIBLY_UNUSED
static int jpeg_convert_htable_to_hetable(struct jpeg_htable_t *htables, uint32_t *hetable)
{
    int hb, v;
    struct jpeg_htable_t *htable;

    if (!htables || !hetable)
        return -1;

    for (int i = 0; i < 384; i++)
        hetable[i] = 0xfff;
    for (int i = 168, j = 0xfd0; i < 176; i++, j++)
        hetable[i] = j;
    for (int i = 344, j = 0xfd0; i < 352; i++, j++)
        hetable[i] = j;

    /* AC table 0 */
    hb = 0;
    htable = &(htables[1]);
    for (int i = 0, hi = 0, hc = 0; i < 16; i++) {
        if (htable->code[i]) {
            for (int j = 0; j < htable->code[i]; j++) {
                int n = 0;
                v = htable->data[hi++];
                if (v == 0)
                    n = 160;
                else if (v == 0xf0)
                    n = 161;
                else
                    n = (v >> 4) * 10 + (v & 0xf) - 1;
                hetable[hb + n] = (i << 8) | (hc & 0xff);
                hc++;
            }
        }
        hc <<= 1;
    }

    /* AC table 1 */
    hb = 176;
    htable = &(htables[3]);
    for (int i = 0, hi = 0, hc = 0; i < 16; i++) {
        if (htable->code[i]) {
            for (int j = 0; j < htable->code[i]; j++) {
                int n = 0;
                v = htable->data[hi++];
                if (v == 0)
                    n = 160;
                else if (v == 0xf0)
                    n = 161;
                else
                    n = (v >> 4) * 10 + (v & 0xf) - 1;
                hetable[hb + n] = (i << 8) | (hc & 0xff);
                hc++;
            }
        }
        hc <<= 1;
    }

    /* DC table 0 */
    hb = 352;
    htable = &(htables[0]);
    for (int i = 0, hi = 0, hc = 0; i < 16; i++) {
        if (htable->code[i]) {
            for (int j = 0; j < htable->code[i]; j++) {
                v = htable->data[hi++];
                hetable[hb + v] = (i << 8) | (hc & 0xff);
                hc++;
            }
        }
        hc <<= 1;
    }

    /* DC table 1 */
    hb = 368;
    htable = &(htables[2]);
    for (int i = 0, hi = 0, hc = 0; i < 16; i++) {
        if (htable->code[i]) {
            for (int j = 0; j < htable->code[i]; j++) {
                v = htable->data[hi++];
                hetable[hb + v] = (i << 8) | (hc & 0xff);
                hc++;
            }
        }
        hc <<= 1;
    }
    return JPEG_OK;
}

/**
 * @brief Convert huffman encode data array to AC/DC tables.
 *
 * @param hetable data array for encoding, 384 count;
 * @param htables huffman AC/DC tables;
 *
 * @return JPEG_OK for OK, other for failed;
 */
POSSIBLY_UNUSED
static int jpeg_convert_hetable_to_htable(uint32_t *hetable, struct jpeg_htable_t *htables)
{
    int i, di, hc;
    uint32_t *p;
    struct jpeg_htable_t *htable;

    if (!hetable || !htables)
        return -1;

    /* AC table 0 */
    p = &hetable[0];
    htable = &(htables[1]);
    memset(htable->code, 0, 16);
    memset(htable->data, 0, HTABLE_MAX_SIZE);
    for (i = 0; i < 16; i++) {
        int cn = 0;
        for (int j = 0; j < 168; j++) {
            if (p[j] != 0xfff && p[j] >> 8 == i)
                cn++;
        }
        htable->code[i] = cn;
    }
    for (i = 0, di = 0, hc = 0; i < 16; i++) {
        if (htable->code[i]) {
            for (int j = 0; j < htable->code[i]; j++) {
                int n = 0;
                int v = i << 8 | (hc & 0xff);
                for (n = 0; n < 168; n++) {
                    if (p[n] == v)
                        break;
                }
                if (n == 168) {
                    break;
                } else if (n == 160) {
                    v = 0;
                } else if (n == 161) {
                    v = 0xf0;
                } else {
                    n++;
                    if (n % 10 == 0)
                        v = (n / 10 - 1) << 4 | 0xa;
                    else
                        v = (n / 10) << 4 | (n % 10);
                }
                htable->data[di++] = v;
                hc++;
            }
        }
        hc <<= 1;
    }
    JPEG_DBG("  Huffman Table: AC 0");
    JPEG_DBG("    Code:");
    JPEG_DUMP8(htable->code, 16);
    JPEG_DBG("    Data Size:   %d", di);
    JPEG_DBG("    Data Dump:");
    JPEG_DUMP8(htable->data, di);
    htable->size = di;

    /* AC table 1 */
    p = &hetable[176];
    htable = &(htables[3]);
    memset(htable->code, 0, 16);
    memset(htable->data, 0, HTABLE_MAX_SIZE);
    for (i = 0; i < 16; i++) {
        int cn = 0;
        for (int j = 0; j < 168; j++) {
            if (p[j] != 0xfff && p[j] >> 8 == i)
                cn++;
        }
        htable->code[i] = cn;
    }
    for (i = 0, di = 0, hc = 0; i < 16; i++) {
        if (htable->code[i]) {
            for (int j = 0; j < htable->code[i]; j++) {
                int n = 0;
                int v = i << 8 | (hc & 0xff);
                for (n = 0; n < 168; n++) {
                    if (p[n] == v)
                        break;
                }
                if (n == 168) {
                    break;
                } else if (n == 160) {
                    v = 0;
                } else if (n == 161) {
                    v = 0xf0;
                } else {
                    n++;
                    if (n % 10 == 0)
                        v = (n / 10 - 1) << 4 | 0xa;
                    else
                        v = (n / 10) << 4 | (n % 10);
                }
                htable->data[di++] = v;
                hc++;
            }
        }
        hc <<= 1;
    }
    JPEG_DBG("  Huffman Table: AC 1");
    JPEG_DBG("    Code:");
    JPEG_DUMP8(htable->code, 16);
    JPEG_DBG("    Data Size:   %d", di);
    JPEG_DBG("    Data Dump:");
    JPEG_DUMP8(htable->data, di);
    htable->size = di;

    /* DC table 0 */
    p = &hetable[352];
    htable = &(htables[0]);
    memset(htable->code, 0, 16);
    memset(htable->data, 0, HTABLE_MAX_SIZE);
    for (i = 0; i < 16; i++) {
        int cn = 0;
        for (int j = 0; j < 16; j++) {
            if (p[j] != 0xfff && p[j] >> 8 == i)
                cn++;
        }
        htable->code[i] = cn;
    }
    for (i = 0, di = 0, hc = 0; i < 16; i++) {
        if (htable->code[i]) {
            for (int j = 0; j < htable->code[i]; j++) {
                int n = 0;
                int v = i << 8 | (hc & 0xff);
                for (n = 0; n < 16; n++) {
                    if (p[n] == v)
                        break;
                }
                htable->data[di++] = n;
                hc++;
            }
        }
        hc <<= 1;
    }
    JPEG_DBG("  Huffman Table: DC 0");
    JPEG_DBG("    Code:");
    JPEG_DUMP8(htable->code, 16);
    JPEG_DBG("    Data Size:   %d", di);
    JPEG_DBG("    Data Dump:");
    JPEG_DUMP8(htable->data, di);
    htable->size = di;

    /* DC table 1 */
    p = &hetable[368];
    htable = &(htables[2]);
    memset(htable->code, 0, 16);
    memset(htable->data, 0, HTABLE_MAX_SIZE);
    for (i = 0; i < 16; i++) {
        int cn = 0;
        for (int j = 0; j < 16; j++) {
            if (p[j] != 0xfff && p[j] >> 8 == i)
                cn++;
        }
        htable->code[i] = cn;
    }
    for (i = 0, di = 0, hc = 0; i < 16; i++) {
        if (htable->code[i]) {
            for (int j = 0; j < htable->code[i]; j++) {
                int n = 0;
                int v = i << 8 | (hc & 0xff);
                for (n = 0; n < 16; n++) {
                    if (p[n] == v)
                        break;
                }
                htable->data[di++] = n;
                hc++;
            }
        }
        hc <<= 1;
    }
    JPEG_DBG("  Huffman Table: DC 0");
    JPEG_DBG("    Code:");
    JPEG_DUMP8(htable->code, 16);
    JPEG_DBG("    Data Size:   %d", di);
    JPEG_DBG("    Data Dump:");
    JPEG_DUMP8(htable->data, di);
    htable->size = di;
    return JPEG_OK;
}

/**
 * @brief Convert huffman AC/DC tables to decode tables.
 *
 * @param htables huffman AC/DC tables, load from jpeg file;
 * @param hbase hbase array for hbase mem of JPEG Codec;
 * @param hmin hmin array for hmin mem of JPEG Codec;
 * @param hsymb hsymb array for hsymb mem of JPEG Codec;
 *
 * @return JPEG_OK for OK, other for failed;
 */
POSSIBLY_UNUSED
static int jpeg_convert_htable_to_hdtables(struct jpeg_htable_t *htables, uint32_t *hbase,
                                   uint32_t *hmin, uint32_t *hsymb)
{
    uint8_t hmin_tmp_buf[64];

    if (!htables || !hbase || !hmin || !hsymb)
        return -1;

    for (int i = 0; i < 64; i++)
        hbase[i] = 0;
    for (int i = 0; i < 336; i++)
        hsymb[i] = 0;
    for (int i = 0; i < 16; i++)
        hmin[i] = 0;

    for (int id = 0; id < 2; id++) {
        for (int type = 0; type < 2; type++) {
            int v = (id << 1) | type;
            int a = 0, b = 0;
            struct jpeg_htable_t *htable = &(htables[v]);

            if (type == 1)
                a = (id << 1);
            else
                a = (id << 1) | 1;
            switch (a) {
                case 1:
                case 3:
                    b = 162;
                    break;
                case 2:
                    b = 174;
                    break;
                default:
                    b = 0;
                    break;
            }
            a <<= 4;

            for (int i = a; i < a + 16; i++)
                hbase[i] = 0xff;

            for (int i = 0, hc = 0, di = 0; i < 16; i++, a++) {
                hmin_tmp_buf[a] = hc;
                hbase[a] = (b - hc) & 0x1ff;
                if (htable->code[i]) {
                    for (int j = 0; j < htable->code[i]; j++, b++) {
                        if (type == 0) { /* DC */
                            int c = htable->data[di++] & 0xf;
                            if (id == 1)
                                c <<= 4;
                            hsymb[b] |= c;
                        } else { /* AC */
                            hsymb[b] = htable->data[di++];
                        }
                        hc++;
                    }
                }
                hc <<= 1;
            }
        }
    }

    /* convert hmin data */
    for (int i = 0, j = 0; i < 64; ) {
        int v, v1, v2, v3, v4;
        v = hmin_tmp_buf[i++]&1;
        v <<= 2;
        v |= hmin_tmp_buf[i++]&3;
        v <<= 3;
        v |= hmin_tmp_buf[i++]&7;
        v4 = v>>2;

        v <<= 4;
        v |= hmin_tmp_buf[i++]&15;
        v <<= 5;
        v |= hmin_tmp_buf[i++]&31;
        v <<= 6;
        v |= hmin_tmp_buf[i++]&63;
        v <<= 7;
        v |= hmin_tmp_buf[i++]&127;
        v <<= 8;
        v |= hmin_tmp_buf[i++]&255;
        v3 = v;

        v = hmin_tmp_buf[i++]&255;
        v <<= 8;
        v |= hmin_tmp_buf[i++]&255;
        v <<= 8;
        v |= hmin_tmp_buf[i++]&255;
        v <<= 8;
        v |= hmin_tmp_buf[i++]&255;
        v2 = v;

        v = hmin_tmp_buf[i++]&255;
        v <<= 8;
        v |= hmin_tmp_buf[i++]&255;
        v <<= 8;
        v |= hmin_tmp_buf[i++]&255;
        v <<= 8;
        v |= hmin_tmp_buf[i++]&255;
        v1 = v;

        hmin[j++] = v1;
        hmin[j++] = v2;
        hmin[j++] = v3;
        hmin[j++] = v4;
    }
    return JPEG_OK;
}

/**
 * @brief Convert quatization table to encode table;
 *
 * @param qtable Quatization tables data array (128 count), load from jpeg file;
 * @param qetable Enable data array for qetable mem of JPEG Codec, valid in ecoding;
 *
 * @return JPEG_OK for OK, other for failed;
 */
POSSIBLY_UNUSED
static int jpeg_convert_qtable_to_qetable(uint32_t *qtable, uint32_t *qetable)
{
    if (!qtable || !qetable)
        return -1;
    for (int i = 0; i < QTABLE_MAX_COUNT; i++) {
        qetable[i] = qlook_table[qtable[i]];
    }
    return JPEG_OK;
}

/**
 * @brief Convert quatization table to encode table;
 *
 * @param qtable Quatization tables data array (128 count), load from jpeg file;
 * @param qetable Enable data array for qetable mem of JPEG Codec, valid in ecoding;
 *
 * @return JPEG_OK for OK, other for failed;
 */
POSSIBLY_UNUSED
static int jpeg_convert_qetable_to_qtable(uint32_t *qetable, uint32_t *qtable)
{
    if (!qetable || !qtable)
        return -1;
    for (int i = 0; i < QTABLE_MAX_COUNT; i++) {
        uint32_t v = 0;
        for (int j = 0; j < sizeof(qlook_table); j++) {
            if (qlook_table[j] == qetable[i]) {
                v = j;
                break;
            }
        }
        qtable[i] = v;
    }
    return JPEG_OK;
}

/***********************************************************************************/

void jpeg_init(void)
{
    for (int i = 0; i < jpeg_task_count; i++) {
#ifdef RTOS
        if (!jpeg_task[i].sem)
            jpeg_task[i].sem = osSemaphoreNew(1, 0, NULL);
        if (!jpeg_task[i].mutex) {
            osMutexAttr_t attr = { NULL,
                    osMutexRecursive | osMutexPrioInherit | osMutexRobust,
                    NULL, 0U };
            jpeg_task[i].mutex = osMutexNew(&attr);
        }
#endif
        hal_jpeg_open(jpeg_task[i].codec_id);

        jpeg_task[i].busy = false;
        jpeg_task[i].opened = true;
    }
    hal_dma_open();
}

void jpeg_exit(void)
{
    for (int i = 0; i < jpeg_task_count; i++) {
        jpeg_task[i].busy = false;
        jpeg_task[i].opened = false;

        hal_jpeg_close(jpeg_task[i].codec_id);
    }
}

static struct jpeg_task_t *jpeg_request_and_lock(void)
{
    for (int i = 0; i < jpeg_task_count; i++) {
#ifdef RTOS
        if (jpeg_task[i].opened && !jpeg_task[i].busy) {
            if (jpeg_task[i].mutex)
                osMutexAcquire(jpeg_task[i].mutex, osWaitForever);
            jpeg_task[i].busy = true;
            return &jpeg_task[i];
        }
#else
        uint32_t flag = int_lock();
        if (jpeg_task[i].opened && !jpeg_task[i].busy) {
            jpeg_task[i].busy = true;
            int_unlock(flag);
            return &jpeg_task[i];
        }
        int_unlock(flag);
#endif
    }
    return NULL;
}

static void jpeg_unlock(struct jpeg_task_t *task)
{
    task->busy = false;
#ifdef RTOS
    if (task->mutex)
        osMutexRelease(task->mutex);
#endif
}

static void jpeg_done_cb(enum HAL_JPEG_ID_T id)
{
#ifdef RTOS
    struct jpeg_task_t *task = &jpeg_task[id];
    if (task->sem)
        osSemaphoreRelease(task->sem);
#endif
}

static int jpeg_wait_done(struct jpeg_task_t *task)
{
    bool timeout = false;
    uint32_t start = hal_sys_timer_get();
#ifdef RTOS
    if (task->sem) {
        if (osSemaphoreAcquire(task->sem, JPEG_WAIT_MS))
            timeout = true;
    } else
#endif
    {
        while (hal_jpeg_busy(task->codec_id) &&
               TICKS_TO_MS(hal_sys_timer_get() - start) < JPEG_WAIT_MS)
#ifdef RTOS
            osDelay(1);
#else
            hal_sys_timer_delay(1);
#endif
        if (hal_jpeg_busy(task->codec_id))
            timeout = true;
    }
    if (timeout)
        return JPEG_TIMEOUT;
    return JPEG_OK;
}

int jpeg_decode(struct jpeg_handle_t *handle)
{
    POSSIBLY_UNUSED uint32_t start;
    bool support_hw_swap = true;
    int ret = HAL_JPEG_OK;
    int *q;
    struct jpeg_task_t *task;
    struct HAL_JPEG_CONFIG_T cfg;

    if (!handle)
        return JPEG_FAILED;
    if (!handle->pix_data || !handle->pix_size ||
        !handle->jpeg_data || !handle->jpeg_size ||
        !handle->ecs_buf || !handle->ecs_bufsz)
        return JPEG_FAILED;
    if (JPEG_CHECK_ALIGN(handle->ecs_buf) || JPEG_CHECK_ALIGN(handle->ecs_bufsz) ||
        JPEG_CHECK_ALIGN(handle->pix_data) || JPEG_CHECK_ALIGN(handle->pix_size)) {
        JPEG_LOG("ECS/PIX buffer address and size must be aligned to 64 bytes.");
        return JPEG_BUF_UNALIGNED;
    }

    task = jpeg_request_and_lock();
    if (!task) {
        JPEG_LOG("All CODEC modules are busy (or not opened)!");
        return JPEG_BUSY;
    }

#ifdef CHIP_BEST1503
    if (hal_get_chip_metal_id() < HAL_CHIP_METAL_ID_6)
        support_hw_swap = false;
#endif
#ifdef CHIP_BEST1700
    support_hw_swap = false;
#endif

    memset(&cfg, 0, sizeof(cfg));
    cfg.mode = HAL_JPEG_DEC_MODE;
    cfg.ecs = handle->ecs_buf;
    cfg.pix = handle->pix_data;
    cfg.qtable = task->qtable_buf;
    cfg.hbase = task->hbase_buf;
    cfg.hmin = task->hmin_buf;
    cfg.hsymb = task->hsymb_buf;
    cfg.swap_uv = false;
    cfg.swap_yu = false;
    cfg.done_cb = jpeg_done_cb;

    if (support_hw_swap) {
        if (handle->pix_fmt == PIX_YUV422_YUYV) {
            cfg.swap_yu = true;
        } else if (handle->pix_fmt == PIX_YUV422_UYVY) {
            cfg.swap_uv = true;
        } else if (handle->pix_fmt == PIX_YUV422_YVYU) {
            cfg.swap_uv = true;
            cfg.swap_yu = true;
        }
    }

    start = hal_fast_sys_timer_get();

    memset(&task->info, 0, sizeof(task->info));
    task->info.ecs_data = handle->ecs_buf;

    if (jpeg_load_file(&task->info, handle->jpeg_data, handle->jpeg_size)) {
        JPEG_LOG("Failed to load file data.");
        ret = JPEG_FILE_ERR;
        goto exit;
    }

    if (task->info.format == JPEG_YUV420) {
        cfg.width = (task->info.width + 15) / 16 * 16;
        cfg.height = (task->info.height + 15) / 16 * 16;
        cfg.format = HAL_JPEG_YUV420;
    } else if (task->info.format == JPEG_YUV444) {
        cfg.width = (task->info.width + 7) / 8 * 8;
        cfg.height = (task->info.height + 7) / 8 * 8;
        cfg.format = HAL_JPEG_YUV444;
    } else {
        cfg.width = (task->info.width + 15) / 16 * 16;
        cfg.height = (task->info.height + 7) / 8 * 8;
        cfg.format = HAL_JPEG_YUV422;
    }
    cfg.ecs_size = task->info.ecs_size;
    cfg.rst_nmcu = task->info.rst_nmcu;
    for (int i = 0; i < 3; i++) {
        cfg.qt[i] = task->info.color_qtable[i];
        cfg.hat[i] = task->info.color_htable_ac[i];
        cfg.hdt[i] = task->info.color_htable_dc[i];
    }

    /* Check pixel data buffer size */
    if (cfg.width != task->info.width || cfg.height != task->info.height) {
        JPEG_LOG("Warning: picture size is aligned to %dx%d.", cfg.width, cfg.height);
        if (cfg.width * cfg.height * 2 > handle->pix_size) {
            JPEG_LOG("Pixel buffer is too small, need %dx%dx2.", cfg.width, cfg.height);
            ret = JPEG_BUF_SMALL;
            goto exit;
        }
    }

    jpeg_convert_htable_to_hdtables(task->info.htables, cfg.hbase, cfg.hmin, cfg.hsymb);
    cfg.hbase_size = 64 * 4;
    cfg.hmin_size = 16 * 4;
    cfg.hsymb_size = 336 * 4;

    q = (int *)cfg.qtable;
    for (int i = 0; i < 128; i++) {
        q[i] = task->info.qtables[i];
    }
    cfg.qtable_size = 128 * 4;

    JPEG_LOG("Load jpeg file [%dx%d, %s] used %uus.",
            cfg.width, cfg.height, yuvstr[cfg.format],
            FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start));

    JPEG_LOG("CODEC[%d] Decode start.", task->codec_id);

    start = hal_fast_sys_timer_get();

    hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, (uint32_t)handle->pix_data, handle->pix_size);
    hal_cache_sync(HAL_CACHE_ID_D_CACHE, (uint32_t)cfg.ecs, cfg.ecs_size);

    if (hal_jpeg_start(task->codec_id, &cfg)) {
        JPEG_LOG("CODEC[%d] Decode start failed!", task->codec_id);
        ret = JPEG_HAL_ERR;
        goto exit;
    }

    if (jpeg_wait_done(task)) {
        JPEG_LOG("CODEC[%d] Decode timeout!", task->codec_id);
        ret = JPEG_TIMEOUT;
    }

    hal_jpeg_stop(task->codec_id);

    if (ret != JPEG_OK)
        goto exit;

    JPEG_LOG("CODEC[%d] Decode [%dx%d, %s]->[%dx%d, YUV422] used %uus.", task->codec_id,
            task->info.width, task->info.height, yuvstr[cfg.format],
            cfg.width, cfg.height, FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start));

    handle->jpeg_width = task->info.width;
    handle->jpeg_height = task->info.height;
    handle->width = cfg.width;
    handle->height = cfg.height;
    handle->jpeg_fmt = task->info.format;
    handle->pix_size = cfg.pix_size;

    if (!support_hw_swap) {
        if (handle->pix_fmt == PIX_YUV422_YUYV) {
            yuv422_vyuy_to_yuyv(handle->pix_data, handle->pix_data, handle->width, handle->height);
            hal_cache_sync(HAL_CACHE_ID_D_CACHE, (uint32_t)handle->pix_data, handle->pix_size);
            JPEG_LOG("Convert YUV422 VYUY -> YUYV.");
        } else if (handle->pix_fmt == PIX_YUV422_UYVY) {
            yuv422_vyuy_to_uyvy(handle->pix_data, handle->pix_data, handle->width, handle->height);
            hal_cache_sync(HAL_CACHE_ID_D_CACHE, (uint32_t)handle->pix_data, handle->pix_size);
            JPEG_LOG("Convert YUV422 VYUY -> UYVY.");
        } else if (handle->pix_fmt == PIX_YUV422_YVYU) {
            yuv422_vyuy_to_yvyu(handle->pix_data, handle->pix_data, handle->width, handle->height);
            hal_cache_sync(HAL_CACHE_ID_D_CACHE, (uint32_t)handle->pix_data, handle->pix_size);
            JPEG_LOG("Convert YUV422 VYUY -> YVYU.");
        }
    }

exit:
    jpeg_unlock(task);
    return ret;
}

int jpeg_encode(struct jpeg_handle_t *handle)
{
    POSSIBLY_UNUSED uint32_t start;
    int ret = HAL_JPEG_OK;
    struct jpeg_task_t *task;
    struct HAL_JPEG_CONFIG_T cfg;

    if (!handle)
        return JPEG_FAILED;
    if (!handle->pix_data || !handle->jpeg_data || !handle->pix_size ||
        !handle->ecs_buf || !handle->ecs_bufsz)
        return JPEG_FAILED;
    if (JPEG_CHECK_ALIGN(handle->ecs_buf) || JPEG_CHECK_ALIGN(handle->ecs_bufsz) ||
        JPEG_CHECK_ALIGN(handle->pix_data) || JPEG_CHECK_ALIGN(handle->pix_size)) {
        JPEG_LOG("ECS/PIX buffer address and size must be aligned to 64 bytes.");
        return JPEG_BUF_UNALIGNED;
    }
    if (handle->jpeg_fmt != JPEG_YUV422)
        return JPEG_NOT_SUPPORT;

    task = jpeg_request_and_lock();
    if (!task) {
        JPEG_LOG("All CODEC modules are busy (or not opened)!");
        return JPEG_BUSY;
    }

    memset(&cfg, 0, sizeof(cfg));
    cfg.mode = HAL_JPEG_ENC_MODE;
    cfg.width = handle->width;
    cfg.height = handle->height;
    cfg.format = HAL_JPEG_YUV422;
    cfg.ecs = handle->ecs_buf;
    cfg.pix = handle->pix_data;
    cfg.pix_size = handle->pix_size;
    cfg.qtable = default_qetable;
    cfg.qtable_size = sizeof(default_qetable);
    cfg.hetable = default_hetable;
    cfg.hetable_size = sizeof(default_hetable);
    cfg.done_cb = jpeg_done_cb;

    if (handle->pix_fmt == PIX_YUV422_YUYV) {
        cfg.swap_uv = false;
        cfg.swap_yu = true;
    } else if (handle->pix_fmt == PIX_YUV422_UYVY) {
        cfg.swap_uv = true;
        cfg.swap_yu = false;
    } else if (handle->pix_fmt == PIX_YUV422_YVYU) {
        cfg.swap_uv = true;
        cfg.swap_yu = true;
    } else {
        cfg.swap_uv = false;
        cfg.swap_yu = false;
    }
    for (int i = 0; i < 3; i++) {
        cfg.qt[i] = i > 0 ? 1 : 0;
        cfg.hat[i] = i > 0 ? 1 : 0;
        cfg.hdt[i] = i > 0 ? 1 : 0;
    }

    JPEG_LOG("CODEC[%d] Encode start.", task->codec_id);

    start = hal_fast_sys_timer_get();

    hal_cache_sync(HAL_CACHE_ID_D_CACHE, (uint32_t)cfg.pix, cfg.pix_size);
    hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, (uint32_t)handle->ecs_buf, handle->ecs_bufsz);

    if (hal_jpeg_start(task->codec_id, &cfg)) {
        JPEG_LOG("CODEC[%d] Encode start failed!", task->codec_id);
        ret = JPEG_HAL_ERR;
        goto exit;
    }

    if (jpeg_wait_done(task)) {
        JPEG_LOG("CODEC[%d] Encode timeout!", task->codec_id);
        ret = JPEG_TIMEOUT;
    }

    hal_jpeg_stop(task->codec_id);

    if (ret != JPEG_OK)
        goto exit;

    JPEG_LOG("CODEC[%d] Encode [%dx%d, %s] used %uus.", task->codec_id,
            cfg.width, cfg.height, yuvstr[cfg.format],
            FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start));

    start = hal_fast_sys_timer_get();

    memset(&task->info, 0, sizeof(task->info));

    task->info.width = cfg.width;
    task->info.height = cfg.height;
    if (cfg.format == HAL_JPEG_YUV420)
        task->info.format = JPEG_YUV420;
    else if (cfg.format == HAL_JPEG_YUV444)
        task->info.format = JPEG_YUV420;
    else
        task->info.format = JPEG_YUV422;

    for (int i = 0; i < 3; i++) {
        task->info.color_qtable[i] = cfg.qt[i];
        task->info.color_htable_ac[i] = cfg.hat[i];
        task->info.color_htable_dc[i] = cfg.hdt[i];
    }

    task->info.ecs_data = (uint8_t *)cfg.ecs;
    task->info.ecs_size = cfg.ecs_size;

    jpeg_convert_qetable_to_qtable(cfg.qtable, task->info.qtables);
    jpeg_convert_hetable_to_htable(cfg.hetable, task->info.htables);

    ret = jpeg_save_file(&task->info, handle->jpeg_data, &handle->jpeg_size);
    JPEG_LOG("Save jpeg file [%u bytes] used %uus.",
            handle->jpeg_size,
            FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start));

exit:
    jpeg_unlock(task);
    return ret;
}

