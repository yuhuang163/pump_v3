/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

#ifndef elm_precom_h
#define elm_precom_h

/* System headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* VGLite hdaders. */
#include "vg_lite.h"

/* Project headers. */
#include "../inc/Elm.h"
#include "velm.h"

#include "hal_trace.h"
#include "gpu_port.h"

#define JUMP_IF_NON_ZERO_VALUE(x, label) { int ret = x; if ( (ret) != 1 ) { goto label; }  }
#define JUMP_IF_NULL(x, label) { if (x == NULL) { goto label;} }

#undef malloc
#define malloc gpu_malloc

#undef free
#define free gpu_free

#undef  calloc
#define calloc(num_objects,object_size) gpu_malloc(num_objects * object_size)

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)
#define printf  GPU_REP

#endif /* elm_precom_h */
