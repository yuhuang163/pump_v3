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
#ifndef __INCLUDE_AOS_ENDIANPROC_H
#define __INCLUDE_AOS_ENDIANPROC_H

#include <sys/param.h>

#define CONFIG_HAVE_LONG_LONG

#define __swap_uint16(n) \
    (uint16_t)(((((uint16_t)(n)) & 0x00ff) << 8) | \
               ((((uint16_t)(n)) >> 8) & 0x00ff))

#define __swap_uint32(n) \
    (uint32_t)(((((uint32_t)(n)) & 0x000000ffUL) << 24) | \
               ((((uint32_t)(n)) & 0x0000ff00UL) <<  8) | \
               ((((uint32_t)(n)) & 0x00ff0000UL) >>  8) | \
               ((((uint32_t)(n)) & 0xff000000UL) >> 24))

#ifdef CONFIG_HAVE_LONG_LONG
#define __swap_uint64(n) \
        (uint64_t)(((((uint64_t)(n)) & 0x00000000000000ffULL) << 56) | \
                   ((((uint64_t)(n)) & 0x000000000000ff00ULL) << 40) | \
                   ((((uint64_t)(n)) & 0x0000000000ff0000ULL) << 24) | \
                   ((((uint64_t)(n)) & 0x00000000ff000000ULL) <<  8) | \
                   ((((uint64_t)(n)) & 0x000000ff00000000ULL) >>  8) | \
                   ((((uint64_t)(n)) & 0x0000ff0000000000ULL) >> 24) | \
                   ((((uint64_t)(n)) & 0x00ff000000000000ULL) >> 40) | \
                   ((((uint64_t)(n)) & 0xff00000000000000ULL) >> 56))
#endif

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define htobe16(n) __swap_uint16((uint16_t)n)
#define htole16(n) (n)
#define be16toh(n) __swap_uint16((uint16_t)n)
#define le16toh(n) (n)

#define htobe32(n) __swap_uint32((uint32_t)n)
#define htole32(n) (n)
#define be32toh(n) __swap_uint32((uint32_t)n)
#define le32toh(n) (n)

#ifdef CONFIG_HAVE_LONG_LONG
#define htobe64(n) __swap_uint64((uint64_t)n)
#define htole64(n) (n)
#define be64toh(n) __swap_uint64((uint64_t)n)
#define le64toh(n) (n)
#endif
#else
/* Big-endian byte order macros */
#define htobe16(n) (n)
#define htole16(n) __swap_uint16((uint16_t)n)
#define be16toh(n) (n)
#define le16toh(n) __swap_uint16((uint16_t)n)

#define htobe32(n) (n)
#define htole32(n) __swap_uint32((uint32_t)n)
#define be32toh(n) (n)
#define le32toh(n) __swap_uint32(n)

#ifdef CONFIG_HAVE_LONG_LONG
#define htobe64(n) (n)
#define htole64(n) __swap_uint64((uint64_t)n)
#define be64toh(n) (n)
#define le64toh(n) __swap_uint64((uint64_t)n)
#endif
#endif

#define swap16 __swap_uint16
#define swap32 __swap_uint32
#define swap64 __swap_uint64
#define betoh16 be16toh
#define letoh16 le16toh
#define bemtoh16(x) betoh16(*(FAR uint16_t *)(x))
#define htobem16(x, v) (*(FAR uint16_t *)(x) = htobe16(v))
#define lemtoh16(x) letoh16(*(FAR uint16_t *)(x))
#define htolem16(x, v) (*(FAR uint16_t *)(x) = htole16(v))
#define betoh32 be32toh
#define letoh32 le32toh
#define bemtoh32(x) htobe32(*(FAR uint32_t *)(x))
#define htobem32(x, v) (*(FAR uint32_t *)(x) = htobe32(v))
#define lemtoh32(x) letoh32(*(FAR uint32_t *)(x))
#define htolem32(x, v) (*(FAR uint32_t *)(x) = htole32(v))

#ifdef CONFIG_HAVE_LONG_LONG
#define betoh64 be64toh
#define letoh64 le64toh
#define bemtoh64(x) htobe64(*(FAR uint64_t *)(x))
#define htobem64(x, v) (*(FAR uint64_t *)(x) = htobe64(v))
#define lemtoh64(x) letoh64(*(FAR uint64_t *)(x))
#define htolem64(x, v) (*(FAR uint64_t *)(x) = htole64(v))
#endif

#endif /*__INCLUDE_AOS_ENDIANPROC_H*/
