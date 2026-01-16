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

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static int y1tab[256], y2tab[256], utab[256], vtab[256];

void yuv420_planar_to_yuv422_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    /* Copy Y data */
    for (int i = 0; i < w * h; i++)
        *q++ = *p++;

    /* Convert U data, 1 U pixel -> 2x1 U pixels,
     * one row -> two rows. */
    for (int i = 0; i < h / 2; i++) {
        uint8_t *row = p;
        for (int j = 0; j < w / 2; j++) {
            *q++ = row[j];
        }
        for (int j = 0; j < w / 2; j++) {
            *q++ = row[j];
        }
        p += w / 2;
    }

    /* Convert V data, 1 V pixel -> 2x1 V pixels,
     * one row -> two rows. */
    for (int i = 0; i < h / 2; i++) {
        uint8_t *row = p;
        for (int j = 0; j < w / 2; j++) {
            *q++ = row[j];
        }
        for (int j = 0; j < w / 2; j++) {
            *q++ = row[j];
        }
        p += w / 2;
    }
}

void yuv420_planar_to_yuv444_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    /* Copy Y data */
    for (int i = 0; i < w * h; i++)
        *q++ = *p++;

    /* Convert U data, one U pixel -> 2x2 U pixels,
     * one row -> two rows, with width x 2. */
    for (int i = 0; i < h / 2; i++) {
        uint8_t *row = p;
        for (int j = 0; j < w / 2; j++) {
            *q++ = row[j];
            *q++ = row[j];
        }
        for (int j = 0; j < w / 2; j++) {
            *q++ = row[j];
            *q++ = row[j];
        }
        p += w / 2;
    }

    /* Convert V data, one V pixel -> 2x2 V pixels,
     * one row -> two rows, with width x 2. */
    for (int i = 0; i < h / 2; i++) {
        uint8_t *row = p;
        for (int j = 0; j < w / 2; j++) {
            *q++ = row[j];
            *q++ = row[j];
        }
        for (int j = 0; j < w / 2; j++) {
            *q++ = row[j];
            *q++ = row[j];
        }
        p += w / 2;
    }
}

void yuv422_planar_to_yuv420_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    /* Copy Y data */
    for (int i = 0; i < w * h; i++)
        *q++ = *p++;

    /* Convert U data, 2x1 U pixels -> 1 U pixel,
     * two rows -> one row, U use row 0. */
    for (int i = 0; i < h; i++) {
        if (i % 2 == 0) {
            uint8_t *row = p;
            for (int j = 0; j < w / 2; j++) {
                *q++ = row[j];
            }
        }
        p += w / 2;
    }

    /* Convert V data, 2x1 V pixels -> 1 V pixel,
     * two rows -> one row, V use row 1. */
    for (int i = 0; i < h; i++) {
        if (i % 2 == 1) {
            uint8_t *row = p;
            for (int j = 0; j < w / 2; j++) {
                *q++ = row[j];
            }
        }
        p += w / 2;
    }
}

void yuv422_planar_to_yuv444_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    /* Copy Y data */
    for (int i = 0; i < w * h; i++)
        *q++ = *p++;

    /* Convert U data, 2x1 U pixels -> 2x2 U pixels,
     * one row -> one row, with width x 2. */
    for (int i = 0; i < h; i++) {
        uint8_t *row = p;
        for (int j = 0; j < w / 2; j++) {
            *q++ = row[j];
            *q++ = row[j];
        }
        p += w / 2;
    }

    /* Convert V data, 2x1 V pixels -> 2x2 V pixels,
     * one row -> one row, with width x 2. */
    for (int i = 0; i < h; i++) {
        uint8_t *row = p;
        for (int j = 0; j < w / 2; j++) {
            *q++ = row[j];
            *q++ = row[j];
        }
        p += w / 2;
    }
}

void yuv444_planar_to_yuv422_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    /* Copy Y data */
    for (int i = 0; i < w * h; i++)
        *q++ = *p++;

    /* Convert U data, 2x2 U pixels -> 2x1 U pixels,
     * one row -> one row, with half width. */
#if 0
    for (int i = 0; i < h; i++) {
        uint8_t *row = p;
        for (int j = 0; j < w; j++) {
            if (j % 2 == 0)
                *q++ = row[j];
        }
        p += w;
    }
#else
    for (int i = 0; i < w * h; i++) {
        if (i % 2 == 0)
            *q++ = *p++;
        else
            p++;
    }
#endif

    /* Convert V data, 2x2 V pixels -> 2x1 V pixels,
     * one row -> one row, with half width. */
#if 0
    for (int i = 0; i < h; i++) {
        uint8_t *row = p;
        for (int j = 0; j < w; j++) {
            if (j % 2 == 1)
                *q++ = row[j];
        }
        p += w;
    }
#else
    for (int i = 0; i < w * h; i++) {
        if (i % 2 == 1)
            *q++ = *p++;
        else
            p++;
    }
#endif
}

void yuv444_planar_to_yuv420_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    /* Copy Y data */
    for (int i = 0; i < w * h; i++)
        *q++ = *p++;

    /* Convert U data, 2x2 U pixels -> 1x1 U pixels,
     * two row -> one row, with half width, U use row 0. */
    for (int i = 0; i < h; i++) {
        if (i % 2 == 0) {
            uint8_t *row = p;
            for (int j = 0; j < w; j++) {
                if (j % 2 == 0)
                    *q++ = row[j];
            }
        }
        p += w;
    }

    /* Convert V data, 2x2 V pixels -> 1x1 V pixels,
     * two row -> one row, with half width, V use row 1. */
    for (int i = 0; i < h; i++) {
        if (i % 2 == 1) {
            uint8_t *row = p;
            for (int j = 0; j < w; j++) {
                if (j % 2 == 1)
                    *q++ = row[j];
            }
        }
        p += w;
    }
}

void yuv420_planar_to_rgba8888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *ybase = srcbuf;
    uint8_t *ubase = srcbuf + (w * h);
    uint8_t *vbase = srcbuf + (w * h * 5 / 4);
    uint8_t *q = dstbuf;

    for (int i = 0; i < 256; i++) {
        y1tab[i] = 11644 * i;
        y2tab[i] = 19837 * i - 311710;
        utab[i] = 20238 * i - 2771300;
        vtab[i] = 15938 * i - 2221300;
    }

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int y = ybase[i * w + j];
            int u = ubase[i / 2 * w / 2 + j / 2];
            int v = vbase[i / 2 * w / 2 + j / 2];
            int r = MAX(0, MIN(255, (vtab[v] + y1tab[y]) / 10000));
            int b = MAX(0, MIN(255, (utab[u] + y1tab[y]) / 10000));
            int g = MAX(0, MIN(255, (y2tab[y] - 5094 * r - 1942 * b) / 10000));
            int a = 0xff;
            *q++ = r;
            *q++ = g;
            *q++ = b;
            *q++ = a;
        }
    }
}

void yuv422_planar_to_rgba8888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *ybase = srcbuf;
    uint8_t *ubase = srcbuf + (w * h);
    uint8_t *vbase = srcbuf + (w * h * 3 / 2);
    uint8_t *q = dstbuf;

    for (int i = 0; i < 256; i++) {
        y1tab[i] = 11644 * i;
        y2tab[i] = 19837 * i - 311710;
        utab[i] = 20238 * i - 2771300;
        vtab[i] = 15938 * i - 2221300;
    }

    for (int i = 0; i < w * h; i++) {
        int y = ybase[i];
        int u = ubase[i / 2];
        int v = vbase[i / 2];
        int r = MAX(0, MIN(255, (vtab[v] + y1tab[y]) / 10000));
        int b = MAX(0, MIN(255, (utab[u] + y1tab[y]) / 10000));
        int g = MAX(0, MIN(255, (y2tab[y] - 5094 * r - 1942 * b) / 10000));
        int a = 0xff;
        *q++ = r;
        *q++ = g;
        *q++ = b;
        *q++ = a;
    }
}

void yuv444_planar_to_rgba8888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *ybase = srcbuf;
    uint8_t *ubase = srcbuf + (w * h);
    uint8_t *vbase = srcbuf + (w * h * 2);
    uint8_t *q = dstbuf;

    for (int i = 0; i < 256; i++) {
        y1tab[i] = 11644 * i;
        y2tab[i] = 19837 * i - 311710;
        utab[i] = 20238 * i - 2771300;
        vtab[i] = 15938 * i - 2221300;
    }

    for (int i = 0; i < w * h; i++) {
        int y = ybase[i];
        int u = ubase[i];
        int v = vbase[i];
        int r = MAX(0, MIN(255, (vtab[v] + y1tab[y]) / 10000));
        int b = MAX(0, MIN(255, (utab[u] + y1tab[y]) / 10000));
        int g = MAX(0, MIN(255, (y2tab[y] - 5094 * r - 1942 * b) / 10000));
        int a = 0xff;
        *q++ = r;
        *q++ = g;
        *q++ = b;
        *q++ = a;
    }
}

void rgba8888_to_yuv420_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *ybase = dstbuf;
    uint8_t *ubase = dstbuf + (w * h);
    uint8_t *vbase = dstbuf + (w * h * 5 / 4);

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            float a = (float)p[3];
            float r = (float)p[0] * a / 255.0f;
            float g = (float)p[1] * a / 255.0f;
            float b = (float)p[2] * a / 255.0f;
            ybase[i * w + j] = (uint8_t) (0.2126f * r + 0.7152f * g + 0.0722f * b);
            if (i % 2 == 0 && j % 2 == 0)
                ubase[i / 2 * w / 2 + j / 2] = (uint8_t) (128 - 0.1146f * r - 0.3854f * g + 0.5f * b);
            else if (i % 2 == 1 && j % 2 == 1)
                vbase[i / 2 * w / 2 + j / 2] = (uint8_t) (128 + 0.5f * r - 0.4542f * g - 0.0458f * b);
            p += 4;
        }
    }
}

void rgba8888_to_yuv422_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *ybase = dstbuf;
    uint8_t *ubase = dstbuf + (w * h);
    uint8_t *vbase = dstbuf + (w * h * 3 / 2);

    for (int i = 0; i < w * h; i++) {
        float a = (float)p[3];
        float r = (float)p[0] * a / 255.0f;
        float g = (float)p[1] * a / 255.0f;
        float b = (float)p[2] * a / 255.0f;
        ybase[i] = (uint8_t) (0.2126f * r + 0.7152f * g + 0.0722f * b);
        if (i % 2 == 0)
            ubase[i / 2] = (uint8_t) (128 - 0.1146f * r - 0.3854f * g + 0.5f * b);
        else
            vbase[i / 2] = (uint8_t) (128 + 0.5f * r - 0.4542f * g - 0.0458f * b);
        p += 4;
    }
}

void rgba8888_to_yuv444_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *ybase = dstbuf;
    uint8_t *ubase = dstbuf + (w * h);
    uint8_t *vbase = dstbuf + (w * h * 2);

    for (int i = 0; i < w * h; i++) {
        float a = (float)p[3];
        float r = (float)p[0] * a / 255.0f;
        float g = (float)p[1] * a / 255.0f;
        float b = (float)p[2] * a / 255.0f;
        ybase[i] = (uint8_t) (0.2126f * r + 0.7152f * g + 0.0722f * b);
        ubase[i] = (uint8_t) (128 - 0.1146f * r - 0.3854f * g + 0.5f * b);
        vbase[i] = (uint8_t) (128 + 0.5f * r - 0.4542f * g - 0.0458f * b);
        p += 4;
    }
}

void yuv420_planar_to_rgb888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *ybase = srcbuf;
    uint8_t *ubase = srcbuf + (w * h);
    uint8_t *vbase = srcbuf + (w * h * 5 / 4);
    uint8_t *q = dstbuf;

    for (int i = 0; i < 256; i++) {
        y1tab[i] = 11644 * i;
        y2tab[i] = 19837 * i - 311710;
        utab[i] = 20238 * i - 2771300;
        vtab[i] = 15938 * i - 2221300;
    }

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int y = ybase[i * w + j];
            int u = ubase[i / 2 * w / 2 + j / 2];
            int v = vbase[i / 2 * w / 2 + j / 2];
            int r = MAX(0, MIN(255, (vtab[v] + y1tab[y]) / 10000));
            int b = MAX(0, MIN(255, (utab[u] + y1tab[y]) / 10000));
            int g = MAX(0, MIN(255, (y2tab[y] - 5094 * r - 1942 * b) / 10000));
            *q++ = r;
            *q++ = g;
            *q++ = b;
        }
    }
}

void yuv422_planar_to_rgb888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *ybase = srcbuf;
    uint8_t *ubase = srcbuf + (w * h);
    uint8_t *vbase = srcbuf + (w * h * 3 / 2);
    uint8_t *q = dstbuf;

    for (int i = 0; i < 256; i++) {
        y1tab[i] = 11644 * i;
        y2tab[i] = 19837 * i - 311710;
        utab[i] = 20238 * i - 2771300;
        vtab[i] = 15938 * i - 2221300;
    }

    for (int i = 0; i < w * h; i++) {
        int y = ybase[i];
        int u = ubase[i / 2];
        int v = vbase[i / 2];
        int r = MAX(0, MIN(255, (vtab[v] + y1tab[y]) / 10000));
        int b = MAX(0, MIN(255, (utab[u] + y1tab[y]) / 10000));
        int g = MAX(0, MIN(255, (y2tab[y] - 5094 * r - 1942 * b) / 10000));
        *q++ = r;
        *q++ = g;
        *q++ = b;
    }
}

void yuv444_planar_to_rgb888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *ybase = srcbuf;
    uint8_t *ubase = srcbuf + (w * h);
    uint8_t *vbase = srcbuf + (w * h * 2);
    uint8_t *q = dstbuf;

    for (int i = 0; i < 256; i++) {
        y1tab[i] = 11644 * i;
        y2tab[i] = 19837 * i - 311710;
        utab[i] = 20238 * i - 2771300;
        vtab[i] = 15938 * i - 2221300;
    }

    for (int i = 0; i < w * h; i++) {
        int y = ybase[i];
        int u = ubase[i];
        int v = vbase[i];
        int r = MAX(0, MIN(255, (vtab[v] + y1tab[y]) / 10000));
        int b = MAX(0, MIN(255, (utab[u] + y1tab[y]) / 10000));
        int g = MAX(0, MIN(255, (y2tab[y] - 5094 * r - 1942 * b) / 10000));
        *q++ = r;
        *q++ = g;
        *q++ = b;
    }
}

void rgb888_to_yuv420_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *ybase = dstbuf;
    uint8_t *ubase = dstbuf + (w * h);
    uint8_t *vbase = dstbuf + (w * h * 5 / 4);

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            float r = (float)p[0];
            float g = (float)p[1];
            float b = (float)p[2];
            ybase[i * w + j] = (uint8_t) (0.2126f * r + 0.7152f * g + 0.0722f * b);
            if (i % 2 == 0 && j % 2 == 0)
                ubase[i / 2 * w / 2 + j / 2] = (uint8_t) (128 - 0.1146f * r - 0.3854f * g + 0.5f * b);
            else if (i % 2 == 1 && j % 2 == 1)
                vbase[i / 2 * w / 2 + j / 2] = (uint8_t) (128 + 0.5f * r - 0.4542f * g - 0.0458f * b);
            p += 3;
        }
    }
}

void rgb888_to_yuv422_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *ybase = dstbuf;
    uint8_t *ubase = dstbuf + (w * h);
    uint8_t *vbase = dstbuf + (w * h * 3 / 2);

    for (int i = 0; i < w * h; i++) {
        float r = (float)p[0];
        float g = (float)p[1];
        float b = (float)p[2];
        ybase[i] = (uint8_t) (0.2126f * r + 0.7152f * g + 0.0722f * b);
        if (i % 2 == 0)
            ubase[i / 2] = (uint8_t) (128 - 0.1146f * r - 0.3854f * g + 0.5f * b);
        else
            vbase[i / 2] = (uint8_t) (128 + 0.5f * r - 0.4542f * g - 0.0458f * b);
        p += 3;
    }
}

void rgb888_to_yuv444_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *ybase = dstbuf;
    uint8_t *ubase = dstbuf + (w * h);
    uint8_t *vbase = dstbuf + (w * h * 2);

    for (int i = 0; i < w * h; i++) {
        float r = (float)p[0];
        float g = (float)p[1];
        float b = (float)p[2];
        ybase[i] = (uint8_t) (0.2126f * r + 0.7152f * g + 0.0722f * b);
        ubase[i] = (uint8_t) (128 - 0.1146f * r - 0.3854f * g + 0.5f * b);
        vbase[i] = (uint8_t) (128 + 0.5f * r - 0.4542f * g - 0.0458f * b);
        p += 3;
    }
}

void yuv422_vyuy_to_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    /* Copy Y data */
    p = srcbuf + 1;
    for (int i = 0; i < w * h; i++) {
        *q++ = *p;
        p += 2;
    }

    /* Copy U data */
    p = srcbuf + 2;
    for (int i = 0; i < w * h / 2; i++) {
        *q++ = *p;
        p += 4;
    }

    /* Copy V data */
    p = srcbuf;
    for (int i = 0; i < w * h / 2; i++) {
        *q++ = *p;
        p += 4;
    }
}

void yuv422_planar_to_vyuy(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    /* Copy Y data */
    q = dstbuf + 1;
    for (int i = 0; i < w * h; i++) {
        *q = *p++;
        q += 2;
    }

    /* Copy U data */
    q = dstbuf + 2;
    for (int i = 0; i < w * h / 2; i++) {
        *q = *p++;
        q += 4;
    }

    /* Copy V data */
    q = dstbuf;
    for (int i = 0; i < w * h / 2; i++) {
        *q = *p++;
        q += 4;
    }
}

void yuv422_yuyv_to_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    /* Copy Y data */
    p = srcbuf;
    for (int i = 0; i < w * h; i++) {
        *q++ = *p;
        p += 2;
    }

    /* Copy U data */
    p = srcbuf + 1;
    for (int i = 0; i < w * h / 2; i++) {
        *q++ = *p;
        p += 4;
    }

    /* Copy V data */
    p = srcbuf + 3;
    for (int i = 0; i < w * h / 2; i++) {
        *q++ = *p;
        p += 4;
    }
}

void yuv422_planar_to_yuyv(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    /* Copy Y data */
    q = dstbuf;
    for (int i = 0; i < w * h; i++) {
        *q = *p++;
        q += 2;
    }

    /* Copy U data */
    q = dstbuf + 1;
    for (int i = 0; i < w * h / 2; i++) {
        *q = *p++;
        q += 4;
    }

    /* Copy V data */
    q = dstbuf + 3;
    for (int i = 0; i < w * h / 2; i++) {
        *q = *p++;
        q += 4;
    }
}

void yuv422_vyuy_to_yuyv(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    for (int i = 0; i < w * h * 2; i += 4) {
        uint8_t v  = p[0];
        uint8_t y0 = p[1];
        uint8_t u  = p[2];
        uint8_t y1 = p[3];
        q[0] = y0;
        q[1] = u;
        q[2] = y1;
        q[3] = v;
        p += 4;
        q += 4;
    }
}

void yuv422_yuyv_to_vyuy(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    for (int i = 0; i < w * h * 2; i += 4) {
        uint8_t y0 = p[0];
        uint8_t u  = p[1];
        uint8_t y1 = p[2];
        uint8_t v  = p[3];
        q[0] = v;
        q[1] = y0;
        q[2] = u;
        q[3] = y1;
        p += 4;
        q += 4;
    }
}

void yuv422_vyuy_to_uyvy(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    for (int i = 0; i < w * h * 2; i += 4) {
        uint8_t v  = p[0];
        uint8_t y0 = p[1];
        uint8_t u  = p[2];
        uint8_t y1 = p[3];
        q[0] = u;
        q[1] = y0;
        q[2] = v;
        q[3] = y1;
        p += 4;
        q += 4;
    }
}

void yuv422_vyuy_to_yvyu(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;

    for (int i = 0; i < w * h * 2; i += 4) {
        uint8_t v  = p[0];
        uint8_t y0 = p[1];
        uint8_t u  = p[2];
        uint8_t y1 = p[3];
        q[0] = y0;
        q[1] = v;
        q[2] = y1;
        q[3] = u;
        p += 4;
        q += 4;
    }
}
