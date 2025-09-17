/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	config.h
 * @brief	Generated configuration settings for libmetal.
 */

#ifndef __METAL_CONFIG__H__
#define __METAL_CONFIG__H__

#ifdef __cplusplus
extern "C" {
#endif

/** Library major version number. */
#define METAL_VER_MAJOR		1

/** Library minor version number. */
#define METAL_VER_MINOR		0

/** Library patch level. */
#define METAL_VER_PATCH		0

/** Library version string. */
#define METAL_VER		"1.0.0"

/** System type (linux, generic, ...). */
#define METAL_SYSTEM		"linux"
#define METAL_SYSTEM_LINUX

/** Processor type (arm, x86_64, ...). */
#define METAL_PROCESSOR		"arm"
#define METAL_PROCESSOR_X86_64

/** Machine type (zynq, zynqmp, ...). */
#define METAL_MACHINE		"generic"
#define METAL_MACHINE_GENERIC

#define HAVE_STDATOMIC_H
#define HAVE_FUTEX_H

#define METAL_PROJECT_SYSTEM freertos

#ifdef __cplusplus
}
#endif

#endif /* __METAL_CONFIG__H__ */
