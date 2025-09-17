/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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

#include "plat_addr_map.h"

#if DSP_RAM_SIZE
#define __TTB_SIZE                  0x5000
#else
#define __TTB_SIZE                  0x8000
#ifndef SYNC_FLAGS_SIZE
#define SYNC_FLAGS_SIZE             0x4000
#endif
#endif

#if defined(__NuttX__)
#undef __TTB_SIZE
#define __TTB_SIZE                  0
#endif

#ifdef NOSTD
#define __HEAP_SIZE                 0x1000
#else
#ifndef RTOS
#ifdef EXEC_IN_RAM
#define __HEAP_SIZE                 0x1000
#else
#define __HEAP_SIZE                 0x280000
#endif
#else
#define __HEAP_SIZE                 0x1000
#endif
#endif

#ifdef KERNEL_RTT
#define __STACK_SIZE                0x4000
#define __MAIN_STACK_SIZE           0x4000
#elif KERNEL_LITEOS_A
#define __STACK_SIZE                0x8000
#define __MAIN_STACK_SIZE           0
#else
#define __STACK_SIZE                0x1000
#define __MAIN_STACK_SIZE           0x1000
#endif
#ifdef KERNEL_RTT
#define __FIQ_STACK_SIZE            0
#define __IRQ_STACK_SIZE            0x2000
#define __SVC_STACK_SIZE            0x2000
#define __ABT_STACK_SIZE            0
#define __UND_STACK_SIZE            0x2000
#elif KERNEL_LITEOS_A
#define __FIQ_STACK_SIZE            0
#define __IRQ_STACK_SIZE            0
#define __SVC_STACK_SIZE            0
#define __ABT_STACK_SIZE            0
#define __UND_STACK_SIZE            0
#else
#define __FIQ_STACK_SIZE            0
#define __IRQ_STACK_SIZE            0x1000
#define __SVC_STACK_SIZE            0x1000
#define __ABT_STACK_SIZE            0
#define __UND_STACK_SIZE            0
#endif

/* TODO: Add FLASH_REGION_OFFSET if there are multiple levels of boot loaders */
#if !defined(OTA_BOOT_SIZE) && defined(OTA_CODE_OFFSET)
#define FLASH_REGION_BASE           (FLASH_BASE + OTA_CODE_OFFSET)
#else
#ifdef OTA_BOOT_OFFSET
#define FLASH_REGION_BASE			(FLASH_BASE + OTA_BOOT_OFFSET)
#else
#define FLASH_REGION_BASE			(FLASH_BASE)
#endif
#endif

#ifndef FLASH_REGION_SIZE
#define FLASH_REGION_SIZE           (FLASH_SIZE - (FLASH_REGION_BASE - FLASH_BASE))
#endif

#if (FLASH_REGION_BASE + FLASH_REGION_SIZE) > (FLASH_BASE + FLASH_SIZE)
#error "(FLASH_REGION_BASE + FLASH_REGION_SIZE) should <= (FLASH_BASE + FLASH_SIZE)"
#endif

#ifdef EXEC_IN_RAM
#define EXEC_REGION                 RAM
#elif defined(EXEC_IN_PSRAM)
#define EXEC_REGION                 PSRAM
#else
#define EXEC_REGION                 PSRAMUHS
#endif

#ifdef LOAD_IN_RAM
#define LOAD_REGION                 RAM
#elif defined(LOAD_IN_PSRAM)
#define LOAD_REGION                 PSRAM
#else
#define LOAD_REGION                 PSRAMUHS
#endif

#define _SYS_STACK_SIZE             (__MAIN_STACK_SIZE + __STACK_SIZE + __FIQ_STACK_SIZE + __IRQ_STACK_SIZE + __SVC_STACK_SIZE + __ABT_STACK_SIZE + __UND_STACK_SIZE)

MEMORY
{
    BOOT        (rwx)   : ORIGIN = DSP_BOOT_BASE,           LENGTH = DSP_BOOT_SIZE
    FLASH       (rx)    : ORIGIN = FLASH_REGION_BASE,       LENGTH = FLASH_REGION_SIZE
#if DSP_RAM_SIZE
    RAM         (rwx)   : ORIGIN = DSP_RAM_BASE,            LENGTH = DSP_RAM_SIZE - __TTB_SIZE
    L_TTB       (rw)    : ORIGIN = DSP_RAM_BASE + DSP_RAM_SIZE - __TTB_SIZE,    LENGTH = __TTB_SIZE
#else
    RAM         (rwx)   : ORIGIN = DSP_PSRAMUHS_BASE + DSP_PSRAMUHS_SIZE - __TTB_SIZE - SYNC_FLAGS_SIZE,            LENGTH = SYNC_FLAGS_SIZE
    L_TTB       (rw)    : ORIGIN = DSP_PSRAMUHS_BASE + DSP_PSRAMUHS_SIZE - __TTB_SIZE,    LENGTH = __TTB_SIZE
#endif

#if defined(PSRAM_BASE) && (PSRAMDSP_SIZE > 0)
    PSRAM       (rwx)   : ORIGIN = PSRAMDSP_BASE,              LENGTH = PSRAMDSP_SIZE
#endif

#if defined(SRAM_NC_SIZE) && (SRAM_NC_SIZE > 0)
    SRAM_NC (rx)    : ORIGIN = DSP_RAM_BASE + DSP_RAM_SIZE ,         LENGTH = SRAM_NC_SIZE
#endif

#ifdef DSP_PSRAMUHS_BASE
#if DSP_RAM_SIZE
    PSRAMUHS    (rwx)   : ORIGIN = DSP_PSRAMUHS_BASE,          LENGTH = DSP_PSRAMUHS_SIZE
    PSRAMUHSX   (rx)    : ORIGIN = DSP_PSRAMUHSX_BASE,         LENGTH = DSP_PSRAMUHS_SIZE
#else
    PSRAMUHS    (rwx)   : ORIGIN = DSP_PSRAMUHS_BASE,          LENGTH = DSP_PSRAMUHS_SIZE - __TTB_SIZE - SYNC_FLAGS_SIZE
    PSRAMUHSX   (rx)    : ORIGIN = DSP_PSRAMUHSX_BASE,         LENGTH = DSP_PSRAMUHS_SIZE - __TTB_SIZE - SYNC_FLAGS_SIZE
#endif
#if defined(PSRAMUHS_NC_SIZE) && (PSRAMUHS_NC_SIZE > 0)
    PSRAMUHS_NC (rx)    : ORIGIN = DSP_PSRAMUHS_NC_BASE + DSP_PSRAMUHS_SIZE,         LENGTH = PSRAMUHS_NC_SIZE
#endif
#endif
}

/* Library configurations */
#if !(defined(__NuttX__))
GROUP(libgcc.a)
#endif

#ifdef GEN_BOOT_SECTION
ENTRY(Vectors)
#else
#ifndef RTOS
ENTRY(Reset_Handler)
#else
#ifdef KERNEL_RTT
ENTRY(system_vectors)
#elif defined(KERNEL_LITEOS_A)
ENTRY(reset_vector)
#else
ENTRY(vector_reset)
#endif
#endif
#endif

SECTIONS
{
#ifdef GEN_BOOT_SECTION

    .boot :
    {
        KEEP(*(.vectors))
    } > BOOT

    /DISCARD/ :
    {
        *(*)
    }
#else // !GEN_BOOT_SECTION

    .boot_loader :
    {
#if defined(__NuttX__) && !(defined(PSRAM_BASE) && (PSRAM_SIZE > 0) && !defined(EXEC_IN_PSRAM)) || defined(KERNEL_LITEOS_A)
        KEEP(*(.vectors))
#else
        *(.boot_loader)
        *(.boot_text_flash*)
#endif
        . = ALIGN(4);
    } > EXEC_REGION

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > EXEC_REGION

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > EXEC_REGION
    __exidx_end = .;

    .sram_text :
    {
        __sram_text_data_start__ = .;
        __sram_text_start__ = .;
#if !defined(__NuttX__)
        *:_aeabi_*.o(.text*)
        *:arm_*.o(.text* .rodata*)
#endif
        /* memcpy.o or libc_nano.a(lib_a-memcpy.o/lib_a-memcpy-stub.o) */
        *:memcpy.o(.text*)
        *:lib_a-memcpy*.o(.text*)
        *:libc_rom.o(.text*)

        /* memset.o or libc_nano.a(lib_a-memset.o) */
        *:memset.o(.text*)
        *:lib_a-memset*.o(.text*)

        . = ALIGN(4);
        __kernel_init_level_0 = ABSOLUTE(.);
        KEEP(*( SORT (.rodata.init.kernel.0.*)));
        __kernel_init_level_1 = ABSOLUTE(.);
        KEEP(*( SORT (.rodata.init.kernel.1.*)));
        __kernel_init_level_2 = ABSOLUTE(.);
        KEEP(*( SORT (.rodata.init.kernel.2.*)));
        __kernel_init_level_3 = ABSOLUTE(.);
        KEEP(*( SORT (.rodata.init.kernel.3.*)));
        __kernel_init_level_4 = ABSOLUTE(.);
        KEEP(*( SORT (.rodata.init.kernel.4.*)));
        __kernel_init_level_5 = ABSOLUTE(.);
        KEEP(*( SORT (.rodata.init.kernel.5.*)));
        __kernel_init_level_6 = ABSOLUTE(.);
        KEEP(*( SORT (.rodata.init.kernel.6.*)));
        __kernel_init_level_7 = ABSOLUTE(.);
        KEEP(*( SORT (.rodata.init.kernel.7.*)));
        __kernel_init_level_8 = ABSOLUTE(.);
        KEEP(*( SORT (.rodata.init.kernel.8.*)));
        __kernel_init_level_9 = ABSOLUTE(.);
        KEEP(*( SORT (.rodata.init.kernel.9.*)));
        __kernel_init_level_10 = ABSOLUTE(.);

        _hdf_drivers_start = .;
        KEEP(*(.hdf.driver));
        _hdf_drivers_end = .;

        KEEP(*( SORT (.liteos.table.*)));

        *:hal_timer.o(.text* .rodata*)
        *:hal_sysfreq.o(.text* .rodata*)
        *:hal_cmu_best2003.o(.text* .rodata*)
#ifdef COMPRESS_DSP_CODE
        *:rtos_lib.o(.text* .rodata*)
        *:cmsis_gic.o(.text* .rodata*)
        *:default_irq_ca.o(.text* .rodata*)
        *:irq_ctrl_gic.o(.text* .rodata*)
        *:mmu_ARMCA.o(.text* .rodata*)
        *:os_tick_gtim.o(.text* .rodata*)
        *:system_ARMCA.o(.text* .rodata*)
        *:system_compress.o(.text* .rodata*)
        *:bes_lzma_dec_code.o(.text* .rodata*)
        *:dsp_code_decom.o(.text* .rodata*)
        *:lzma_decode.o(.text* .rodata*)
        *(.boot_text_sram*)
        *(.sram_text*)
        *(.fast_text_sram*)
#endif
#if (LOAD_REGION != EXEC_REGION)
        /* hal transq*/
        *:hal_transq.o(.text*)

        *(.boot_text_sram*)
        *(.sram_text*)
        *(.fast_text_sram*)

        . = ALIGN(4);
#endif
        . = ALIGN(0x1000);
        __sram_text_end__ = .;
    } > EXEC_REGION AT > EXEC_REGION

	__sram_text_start_load__ = LOADADDR(.sram_text);
	__sram_text_end_load__ = LOADADDR(.sram_text) + SIZEOF(.sram_text);

    .sram_data :
    {
        __sram_data_start__ = .;
#if (LOAD_REGION != EXEC_REGION)
        *(.boot_rodata_sram*)
        *(.boot_data*)
        . = ALIGN(4);
#if defined(__NuttX__)
        *:arm_vectors.o(.data*)
#endif
        *:hal_transq.o(.data*)
        *(.sram_data*)
        *(.psram_data*)
        . = ALIGN(4);
#else
        *(.boot_data*)
        *:hal_timer.o(.data*)
        *:hal_sysfreq.o(.data*)
        *:hal_cmu_best2003.o(.data*)
#if defined(COMPRESS_DSP_CODE)
        *:rtos_lib.o(.data*)
        *:cmsis_gic.o(.data*)
        *:default_irq_ca.o(.data*)
        *:irq_ctrl_gic.o(.data*)
        *:mmu_ARMCA.o(.data*)
        *:os_tick_gtim.o(.data*)
        *:system_ARMCA.o(.data*)
        *:bes_lzma_dec_code.o(.data*)
        *:system_compress.o(.data*)
        *:dsp_code_decom.o(.data*)
        *:lzma_decode.o(.data*)
#endif
#endif

#ifdef OPENAMP_ENABLE
        . = ALIGN(8);
        __resource_table_start__ = .;
        KEEP(*(.resource_table*))
        __resource_table_end__ = .;
        . += OPENAMP_SHBUF_SIZE;
#endif
        __sram_data_end__ = .;
    } > EXEC_REGION AT > EXEC_REGION

	__sram_data_start_load__ = LOADADDR(.sram_data);
	__sram_data_end_load__ = LOADADDR(.sram_data) + SIZEOF(.sram_data);

#if defined(PSRAM_BASE) && (PSRAMDSP_SIZE > 0) && !defined(EXEC_IN_PSRAM)
    . = ORIGIN(PSRAM);
    __psram_text_data_start__ = .;
    /*
    __psram_text_data_start_load__ = .;
    .psram_text (ORIGIN(PSRAM)) : AT (__psram_text_data_start_load__)
    */
    .psram_text (.) : AT (ADDR(.psram_text))
    {
        __psram_text_start = .;
        KEEP(*(.vectors))
        *(.psram_text*)
#ifdef DSP_USE_PSRAM
        *(.text*)
        *(.flash_text*)
#ifdef KERNEL_LITEOS_A
        *(EXCLUDE_FILE(*rtos_lib.o *platform.o *los_*.o *vfs*.o *fs*.o *romfs*.o) .rodata*)
#else
        *(.rodata*)
#endif
#endif
        . = ALIGN(0x1000);
        __psram_text_end = .;
    } > PSRAM

	.psram_data (.) : AT (ADDR(.psram_data))
	{
		*(.psram_data*)
		. = ALIGN(4);
	} > PSRAM

	.psram_bss (.) (NOLOAD) : AT (ADDR(.psram_bss))
	{
		__psram_bss_start__ = .;
		*(.psram_bss*)
		. = ALIGN(0x1000);
		__psram_bss_end__ = .;
	} > PSRAM
#endif
	. = __sram_data_end_load__;

#ifdef DSP_USE_PSRAM
    _mmu_text_start = ORIGIN(PSRAM);
#if defined(__NuttX__)
    _mmu_text_end = __psram_text_end;
#endif
#else
    _mmu_text_start = ORIGIN(EXEC_REGION);
#endif
    _stext = _mmu_text_start;
    _etext = _mmu_text_end;
    /*
    _mmu_text_end = __rodata_end;
    */
    _mmu_text_len = _mmu_text_end - _mmu_text_start;

    .copy.table (.) :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;
        LONG (__etext)
        LONG (__data_start__)
        LONG (__data_end__ - __data_start__)
        __copy_table_end__ = .;
    } > EXEC_REGION

    .zero.table :
    {
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG (__bss_start__)
        LONG (__bss_end__ - __bss_start__)
        __zero_table_end__ = .;
    } > EXEC_REGION

    __etext = .;

    .tdata : {
        __tdata_start = .;
        *(.tdata)
        *(.tdata.*)
        *(.gnu.linkonce.td.*)
        __tdata_end = .;
    } > EXEC_REGION

#ifdef RHINO_VER_V33
    .core_driver_entry (.):
    {
        __core_driver_start__ = .;
        KEEP(*(.core_driver_entry))
        . = ALIGN(4);
        __core_driver_end__ = .;
    } > EXEC_REGION
    .bus_driver_entry (.):
    {
        __bus_driver_start__ = .;
        KEEP(*(*.bus_driver_entry))
        __bus_driver_end__ = .;
    } > EXEC_REGION
    .early_driver_entry (.):
    {
        __early_driver_start__ = .;
        KEEP(*(*.early_driver_entry))
        __early_driver_end__ = .;
    } > EXEC_REGION
    .vfs_driver_entry (.):
    {
        __vfs_driver_start__ = .;
        KEEP(*(*.vfs_driver_entry))
        __vfs_driver_end__ = .;
    } > EXEC_REGION
    .level0_driver_entry (.):
    {
        __level0_driver_start__ = .;
        KEEP(*(*.level0_driver_entry))
        __level0_driver_end__ = .;
    } > EXEC_REGION
    .level1_driver_entry (.):
    {
        __level1_driver_start__ = .;
        KEEP(*(*.level1_driver_entry))
        __level1_driver_end__ = .;
    } > EXEC_REGION
    .level2_driver_entry (.):
    {
        __level2_driver_start__ = .;
        KEEP(*(*.level2_driver_entry))
        __level2_driver_end__ = .;
    } > EXEC_REGION
    .level3_driver_entry (.):
    {
        __level3_driver_start__ = .;
        KEEP(*(*.level3_driver_entry))
        __level3_driver_end__ = .;
    } > EXEC_REGION
    .post_driver_entry (.):
    {
        __post_driver_start__ = .;
        KEEP(*(*.post_driver_entry))
        __post_driver_end__ = .;
    } > EXEC_REGION
#endif

    .sram_bss (NOLOAD) : AT (ADDR(.sram_bss))
    {
        __sram_bss_start__ = .;
        *(.boot_bss*)
        . = ALIGN(4);
        *(.sram_bss*)
        . = ALIGN(4);
        __sram_bss_end__ = .;
    } > EXEC_REGION

#ifndef EXEC_IN_RAM
    __ram_end = .;
#endif
    .sbss (NOLOAD) : {
        __sbss_start = .;
        *(.sbss)
        *(.sbss.*)
        *(.gnu.linkonce.sb.*)
        __sbss_end = .;
    } > EXEC_REGION

    .tbss (NOLOAD) : {
        __tbss_start = .;
        *(.tbss)
        *(.tbss.*)
        *(.gnu.linkonce.tb.*)
        __tbss_end = .;
    } > EXEC_REGION

#if defined(__HEAP_SIZE) && (__HEAP_SIZE > 0)
    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        Image$$HEAP$$ZI$$Base = .;
        . += __HEAP_SIZE;
        Image$$HEAP$$ZI$$Limit = .;
        __HeapLimit = .;
    } > EXEC_REGION
#endif

#ifdef EXEC_IN_RAM
    __ram_end = .;
#else
    __free_exec_region = ORIGIN(EXEC_REGION) + LENGTH(EXEC_REGION) - .;
#endif

#ifdef EXEC_IN_RAM
    __psramuhs_text_data_start_load__ = LOADADDR(.data) + SIZEOF(.data);
    .psramuhs_text (ORIGIN(PSRAMUHS)) : AT (__psramuhs_text_data_start_load__)
    {
        __psramuhs_text_start = .;
        *(.psramuhs_text*)
        . = ALIGN(4);
        __psramuhs_text_end = .;
    } > PSRAMUHS
    __psramuhs_text_data_end_load__ = __psramuhs_text_data_start_load__ + SIZEOF(.psramuhs_text);

    .psramuhs_data (ORIGIN(PSRAMUHS) + SIZEOF(.psramuhs_text)) :
		AT (__psramuhs_text_data_end_load__)
    {
        __psramuhs_data_start = .;
        *(.psramuhs_data*)
        . = ALIGN(4);
        __psramuhs_data_end = .;
    } > PSRAMUHS
    __psramuhs_text_data_end_load__ += SIZEOF(.psramuhs_data);
    .psramuhs_bss (.) (NOLOAD) : AT (ADDR(.psramuhs_bss))
    {
        __psramuhs_bss_start__ = .;
        *(.psramuhs_bss*)
        . = ALIGN(4);
        __psramuhs_bss_end__ = .;
    } > PSRAMUHS
#else
    __psramuhs_text_data_end_load__ = LOADADDR(.data) + SIZEOF(.data);
    .psramuhs_bss (.) (NOLOAD) : AT (ADDR(.psramuhs_bss))
    {
        __psramuhs_bss_start__ = .;
        _sbss = .;
        *(.psramuhs_bss*)
        . = ALIGN(4);
        _ebss = .;
        . = ALIGN(8);
        __psramuhs_bss_end__ = .;
    } > PSRAMUHS
#endif

#if defined(EXEC_IN_RAM) || defined(EXEC_IN_PSRAM)
	. = __psramuhs_text_data_end_load__;
#endif

	.build_info (.) :
	{
		KEEP(*(.build_info))
		. = ALIGN(4);
    } > EXEC_REGION

	__boot_struct_build_info = ADDR(.build_info);
    __exec_load_end__ = LOADADDR(.build_info) + SIZEOF(.build_info);

    /* The following section MUST be the last loaded section */
    .code_start_addr (.):
    {
        LONG(ADDR(.boot_loader));
#ifdef OPENAMP_ENABLE
        LONG(ABSOLUTE(__resource_table_start__));
#endif
#if defined(OS_HEAP_SIZE) && (OS_HEAP_SIZE > 0)
        LONG(ABSOLUTE(heap_start));
#endif
    } > EXEC_REGION

    __text_start = .;
#ifdef COMPRESS_DSP_CODE
    .text (.) : AT (ADDR(.text))
#else
    .text (.) :
#endif
    {
        . = ALIGN(16);
        . += 16;

        *:_aeabi_*.o(.text*)
        *:arm_*.o(.text* .rodata*)
        *(SVC_TABLE)
        *(.text*)
        *(.flash_text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        . = ALIGN(16);
        _sinit = .;
        /* preinit data */
        PROVIDE (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE (__preinit_array_end = .);
        /* init data */
        PROVIDE(__ctors_start__ = .);
        PROVIDE (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE (__init_array_end = .);
        PROVIDE(__ctors_end__ = .);
        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)
        _einit = .;
        . = ALIGN(4);
        /* finit data */
        PROVIDE (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE (__fini_array_end = .);
        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        . = ALIGN(4);

        __rodata_start = .;

        __exc_table_start = .;
        KEEP(*(__exc_table))
        __exc_table_end = .;

        *(.rodata*)
        *(.boot_rodata*)
        __rodata_end = .;

        . = ALIGN(4);

        KEEP(*(.eh_frame*))
        *(.note.gnu.build-id)

        /*. = ALIGN(4);*/
        . = ALIGN(0x1000);
        _mmu_text_end = .;
        _etext = .;
    } > EXEC_REGION
    __text_end = .;

    .data :
    {
        Image$$RW_DATA$$Base = .;
        __data_start__ = .;
        _sdata = .;
        *(.data*)
        Image$$RW_DATA$$Limit = .;
        . = ALIGN(4);
        /* All data end */
        *(*.got*)
        __data_end__ = .;
        _edata = .;

    } > EXEC_REGION

#ifdef EXEC_IN_RAM
#if defined(SRAM_NC_SIZE) && (SRAM_NC_SIZE > 0)
    .sync_flags (ORIGIN(SRAM_NC)) (NOLOAD) :
    {
        __sync_flags_sram_start = .;
        *(.sync_flags*)
        __sync_flags_sram_end = .;
    } > SRAM_NC
#else
    .sync_flags ALIGN(0x1000) (NOLOAD) :
    {
        __sync_flags_sram_start = .;
        *(.sync_flags*)
        . = ALIGN(0x1000);
        __sync_flags_sram_end = .;
    } > EXEC_REGION
#endif
#else
#if defined(PSRAMUHS_NC_SIZE) && (PSRAMUHS_NC_SIZE > 0)
    .sync_flags (ORIGIN(PSRAMUHS_NC)) (NOLOAD) :
    {
        __sync_flags_sram_start = .;
        *(.sync_flags*)
        __sync_flags_sram_end = .;
    } > PSRAMUHS_NC
#elif defined(SRAM_NC_SIZE) && (SRAM_NC_SIZE > 0)
    .sync_flags (ORIGIN(SRAM_NC)) (NOLOAD) :
    {
        __sync_flags_sram_start = .;
        *(.sync_flags*)
        __sync_flags_sram_end = .;
    } > SRAM_NC
#else
    .sync_flags (ORIGIN(RAM)) (NOLOAD) :
    {
        __sync_flags_sram_start = .;
        *(.sync_flags*)
        __sync_flags_sram_end = .;
    } > RAM
#endif
#endif

    .bss (NOLOAD) :
    {
        . = ALIGN(0x400);
        __bss_start = .;
        Image$$ZI_DATA$$Base = .;
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
        Image$$ZI_DATA$$Limit = .;
        __end__ = .;
        end = __end__;
        . = ALIGN(4);
        __bss_end = .;
    } > EXEC_REGION

    .os_heap (NOLOAD) :
    {
#if defined(OS_HEAP_SIZE) && (OS_HEAP_SIZE > 0)
        heap_start = .;
        . += OS_HEAP_SIZE;
#endif
    } > EXEC_REGION

#if defined(__NuttX__)
    .noinit :
    {
        _snoinit = ABSOLUTE(.);
        *(.noinit*)
        _enoinit = ABSOLUTE(.);
        . = ALIGN(8);
        . += 0x1000; /* Idle task stack*/
        . = ALIGN(8);
    } > EXEC_REGION
#endif

#if !defined(__NuttX__)
#if DSP_RAM_SIZE
    .stack (ORIGIN(RAM) + LENGTH(RAM) - __TTB_SIZE - _SYS_STACK_SIZE) (NOLOAD) : AT (ADDR(.stack))
#else
    .stack (ORIGIN(EXEC_REGION) + LENGTH(EXEC_REGION) - _SYS_STACK_SIZE) (NOLOAD) : AT (ADDR(.stack))
#endif
    {
        . = ALIGN(8);
         __StackLimit = .;
#ifndef KERNEL_LITEOS_A
        . += __MAIN_STACK_SIZE;
        Image$$SYS_STACK$$ZI$$Base = .;
        . += __STACK_SIZE;
        Image$$SYS_STACK$$ZI$$Limit = .;
        __stack = .;

        Image$$FIQ_STACK$$ZI$$Base = .;
        . += __FIQ_STACK_SIZE;
        Image$$FIQ_STACK$$ZI$$Limit = .;

        Image$$IRQ_STACK$$ZI$$Base = .;
        . += __IRQ_STACK_SIZE;
        Image$$IRQ_STACK$$ZI$$Limit = .;

        Image$$SVC_STACK$$ZI$$Base = .;
        . += __SVC_STACK_SIZE;
        Image$$SVC_STACK$$ZI$$Limit = .;

        Image$$ABT_STACK$$ZI$$Base = .;
        . += __ABT_STACK_SIZE;
        Image$$ABT_STACK$$ZI$$Limit = .;

        Image$$UND_STACK$$ZI$$Base = .;
        . += __UND_STACK_SIZE;
        Image$$UND_STACK$$ZI$$Limit = .;
        . = ALIGN(8);
#else
        *(.int_stack*)
#endif
        __StackTop = .;
#if DSP_RAM_SIZE
    } > RAM
#else
    } > EXEC_REGION
#endif

    ASSERT(__StackLimit >= __bss_end, "region RAM overflowed with stack")

    __free_ram = ADDR(.stack) - __ram_end;
#ifdef EXEC_IN_RAM
    __free_exec_region = __free_ram;
#endif
#else

#if defined(PSRAMUHS_NC_SIZE) && (PSRAMUHS_NC_SIZE > 0)
    __free_ram_uncache = (ORIGIN(PSRAMUHS_NC) + LENGTH(PSRAMUHS_NC) - BES_RPTUN_HEAP_SIZE) - __sync_flags_sram_end;
#else
    __free_ram_uncache = (ORIGIN(SRAM_NC) + LENGTH(SRAM_NC) - BES_RPTUN_HEAP_SIZE) - __sync_flags_sram_end;
#endif
    ASSERT( __free_ram_uncache >= 0, "region PSRAM/SRAM NC overflowed")
#ifdef EXEC_IN_RAM
    _sbss = __bss_start;
    _ebss = __bss_end;
#endif

    _nuttx_heap_start = .;
    _nuttx_heap_end = ORIGIN(EXEC_REGION) + LENGTH(EXEC_REGION) - 0x10000 - 4;
#endif

    .ttb_l2 (NOLOAD) :
    {
        Image$$TTB$$ZI$$Base = .;
        *(.ttb_l2)
    } > L_TTB

    .ttb_l1 (NOLOAD) :
    {
        *(.ttb_l1)
        Image$$TTB$$ZI$$Limit = .;
    } > L_TTB

    __free_ttb = ORIGIN(L_TTB) + LENGTH(L_TTB) - .;

#if defined(__NuttX__)
    .framebuffer (ORIGIN(RAM)) (NOLOAD) :
    {
        *(.framebuffer*)
    } > RAM
#endif

#endif // !GEN_BOOT_SECTION
}

