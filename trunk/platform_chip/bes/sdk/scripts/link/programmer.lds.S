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

#ifdef NO_TRUSTZONE
#define RAMRUN_SEC_ADDR(a)      	RAM_NS_TO_S(a)
#else
#define RAMRUN_SEC_ADDR(a)      	(a)
#endif

#ifndef MIN_BURN_BUFFER_SIZE
#ifdef ROM_IN_FLASH
#define MIN_BURN_BUFFER_SIZE		0x18000
#else
#define MIN_BURN_BUFFER_SIZE		0x4000
#endif
#endif

#ifdef NOSTD
#define HEAP_SECTION_SIZE			0x40
#else
#define HEAP_SECTION_SIZE			0x2000
#endif
#define STACK_SECTION_SIZE			0x1000

#define CODE_MSG_OVERHEAD			8

#ifdef PROGRAMMER_LOAD_SIMU
#undef RAM_SIZE
#ifdef ROM_IN_FLASH
#define RAM_SIZE					0x20000
#else
#define RAM_SIZE					0x8000
#endif
#endif

#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
#define CP_STACK_SECTION_SIZE		0x1000
#endif

#ifdef MAIN_RAM_USE_TCM
#define RAM							ITCM
#define RAMX						ITCM
#ifdef __PIE__
/* mem_test PIE requires that:
   1) the content between __got_info_start and __HeapLimit should be continuous
      in physical memory region,
   2) stack should be in the same physical memory region as code/data/bss/heap
 */
#define REGION_VECTOR				ITCM
#define RAM_NOBITS					ITCM
#else
#define REGION_VECTOR				DTCM
#define RAM_NOBITS					DTCM
#endif
#else /* !MAIN_RAM_USE_TCM */
#ifdef RAM_NC_BASE
#define REGION_VECTOR				RAM_NC
#else
#define REGION_VECTOR				RAM
#endif
#define RAM_NOBITS					RAM
#endif /* !MAIN_RAM_USE_TCM */

#if !(defined(PROGRAMMER_LOAD_RAM) && defined(LOAD_RAM_BASE))
#define LOAD_RAM					RAM
#endif

/* Linker script to configure memory regions. */
MEMORY
{
	ROM    		(rx)	: ORIGIN = ROM_BASE,			LENGTH = ROM_SIZE	/* see plat_addr_map.h and common.mk */
#ifdef MAIN_RAM_USE_TCM
	DTCM   		(rw)	: ORIGIN = RAM_BASE,			LENGTH = RAM_SIZE
	ITCM   		(rwx)	: ORIGIN = RAMX_BASE,			LENGTH = RAMX_SIZE
#else
	RAM    		(rwx)	: ORIGIN = RAM_BASE,			LENGTH = RAM_SIZE
	RAMX   		(rx)	: ORIGIN = RAMX_BASE,			LENGTH = RAM_SIZE
#endif
#if defined(PROGRAMMER_LOAD_RAM) && defined(LOAD_RAM_BASE)
	LOAD_RAM	(rwx)	: ORIGIN = LOAD_RAM_BASE,		LENGTH = LOAD_RAM_SIZE
#endif
#ifdef RAM_NC_BASE
	RAM_NC		(rw)	: ORIGIN = RAM_NC_BASE,			LENGTH = RAM_NC_SIZE
#endif
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
	RAMCP 		(rwx)	: ORIGIN = RAMCP_BASE,			LENGTH = RAMCP_SIZE
	RAMCPX 		(rwx)	: ORIGIN = RAMCPX_BASE,			LENGTH = RAMCPX_SIZE
#endif
#ifdef MEM_POOL_BASE
	MEM_POOL	(rwx)	: ORIGIN = MEM_POOL_BASE,		LENGTH = MEM_POOL_SIZE
#endif
}

/* Library configurations */
GROUP(libgcc.a)

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions ROM and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __export_fn_rom
 *   __exec_struct_start
 *   __exec_load_start
 *   __exec_load_end
 *   __cust_cmd_init_tbl_start
 *   __cust_cmd_init_tbl_end
 *   __cust_cmd_hldr_tbl_start
 *   __cust_cmd_hldr_tbl_end
 *   __exidx_start
 *   __exidx_end
 *   __etext
 *   __data_start__
 *   __data_end__
 *   __pgm_bss_start__
 *   __pgm_bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __free_ram
 */
ENTRY(programmer_start)

SECTIONS
{
	__export_fn_rom = (ORIGIN(ROM) + LENGTH(ROM) - ROM_BUILD_INFO_SECTION_SIZE - ROM_EXPORT_FN_SECTION_SIZE);

	.vector_table (ORIGIN(REGION_VECTOR)) (NOLOAD) :
	{
		__vector_table_start = .;
		KEEP(*(.vector_table))
		. = VECTOR_SECTION_SIZE;
		. = ALIGN(4);
	} > REGION_VECTOR

	/* Just for linking vars in .reboot_param if any. Otherwise can be removed. */
	.reboot_param (NOLOAD) :
	{
		KEEP(*(.reboot_param))
		. = REBOOT_PARAM_SECTION_SIZE;
		. = ALIGN(4);
	} > RAM_NOBITS

	.burn_buffer (NOLOAD) :
	{
		/* The size of .burn_buffer should be greater than __rom_HeapLimit. In most cases 16K is enough. */
		*(.burn_buffer)
		. = MAX(., MIN_BURN_BUFFER_SIZE);
		. = ALIGN(4);
	} > RAM_NOBITS

/*
	.code_dummy (NOLOAD) :
	{
		. = CODE_MSG_OVERHEAD;
		. = ALIGN(4);
	} > RAM_NOBITS
*/

#if (defined(PROGRAMMER_LOAD_RAM) && defined(LOAD_RAM_BASE)) || (defined(MAIN_RAM_USE_TCM) && defined(ROM_IN_FLASH))
/* Avoid conflicts with RAM content used by ROM when:
 * (1) PROGRAMMER_LOAD_RAM && !MAIN_RAM_USE_TCM
 * (2) MAIN_RAM_USE_TCM && ROM_IN_FLASH
 */
#if !defined(MAIN_RAM_USE_TCM) || defined(ROM_IN_FLASH)
	.rom_ram_end (NOLOAD) :
	{
		/* The offset should be greater than __rom_HeapLimit. In most cases 16K is enough. */
		. = MIN_BURN_BUFFER_SIZE;
	} > LOAD_RAM
#endif
#endif

	.boot_struct :
	{
		__boot_struct_start__ = .;
		KEEP(*(.boot_struct))
		__boot_struct_end__ = .;
	} > LOAD_RAM

	__boot_struct_size__ = __boot_struct_end__ - __boot_struct_start__;

	/* ROM only accepts secure address when downloading ramrun */
	.exec_struct RAMRUN_SEC_ADDR(.) :
	{
		__exec_struct_start = .;
		KEEP(*(.pgm_code_ver))
		KEEP(*(.exec_struct))
		. = ALIGN(4);
	} AT > LOAD_RAM

#if defined(PROGRAMMER_LOAD_RAM) && defined(LOAD_RAM_BASE)
	.pgm_boot_text :
	{
		*(.pgm_start)
		. = ALIGN(4);
	} > LOAD_RAM
#endif

	.got_info :
	{
		__got_info_start = .;
		__got_plt_start = .;
		*(.got.plt)
		__got_plt_end = .;
		__got_start = .;
		*(.got)
		__got_end = .;
		. = ALIGN(4);
	} > LOAD_RAM

	.rel.plt :
	{
		__rel_plt_start = .;
		*(.rel.plt)
		__rel_plt_end = .;
	} > LOAD_RAM

	.rel.dyn :
	{
		__rel_dyn_start = .;
		*(.rel.data.rel.ro*)
		*(.rel.gnu.linkonce.d.rel.ro*)
		*(.rel.*)
		__rel_dyn_end = .;
	} > LOAD_RAM

	.data.rel.ro :
	{
		__rodata_rel_start = .;
		*(.data.rel.ro.local*)
		*(.data.rel.ro*)
		__rodata_rel_end = .;
	} > LOAD_RAM

#ifdef RAM_NC_BASE
	__sram_nc_data_start_flash__ = .;

	.sram_nc_data : AT (__sram_nc_data_start_flash__)
	{
		__sram_nc_data_start__ = .;
		*(.sram_nc_rodata*)
		. = ALIGN(4);
		*(.sram_nc_data*)
		__sram_nc_data_end__ = .;
	} > RAM_NC

	__sram_nc_data_end_flash__ = __sram_nc_data_start_flash__ + SIZEOF(.sram_nc_data);

	.sram_nc_bss (NOLOAD) : AT (ADDR(.sram_nc_bss))
	{
		__sram_nc_bss_start__ = .;
		*(.sram_nc_bss*)
		*(.sync_flags_sram*)
		. = ALIGN(4);
		__sram_nc_bss_end__ = .;
	} > RAM_NC

	__ram_nc_top = ORIGIN(RAM_NC) + LENGTH(RAM_NC);
	__free_ram_nc = __ram_nc_top - __sram_nc_bss_end__;
#else /* !RAM_NC_BASE */
	__sram_nc_data_end_flash__ = .;
#endif /* !RAM_NC_BASE */

	__exec_load_start = __sram_nc_data_end_flash__;

	.dummy_exec_data_start (NOLOAD) :
	{
		__exec_data_start = .;
	} > RAM

	. = RAM_TO_RAMX(.);

	.text (.) : AT (__exec_load_start)
	{
		__text_start__ = .;
		*(.text*)

#ifndef NOSTD
		KEEP(*(.init))
		KEEP(*(.fini))

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		/* .dtors */
		*crtbegin.o(.dtors)
		*crtbegin?.o(.dtors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
		*(SORT(.dtors.*))
		*(.dtors)
#endif

		. = ALIGN(4);
	} > RAMX

	.ARM.extab (.) : AT (LOADADDR(.text) + SIZEOF(.text))
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > RAMX

	__exidx_start = .;
	/* .ARM.exidx contains R_ARM_PREL31 (+-0x40000000) offset to functions, which means
	 * the session location cannot be too far away from the function addresses */
	.ARM.exidx (.) : AT (LOADADDR(.ARM.extab) + SIZEOF(.ARM.extab))
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > RAMX
	__exidx_end = .;

	. = RAMX_TO_RAM(.);

	.rodata (.) : AT (LOADADDR(.ARM.exidx) + SIZEOF(.ARM.exidx))
	{
		__rodata_start__ = .;
		*(.rodata*)
		*(.flash_rodata*)
		*(.flash_nc_rodata*)

		. = ALIGN(4);
		__cust_cmd_init_tbl_start = .;
		KEEP(*(.cust_cmd_init_tbl))
		__cust_cmd_init_tbl_end = .;

		. = ALIGN(4);
		__cust_cmd_hldr_tbl_start = .;
		KEEP(*(.cust_cmd_hldr_tbl))
		__cust_cmd_hldr_tbl_end = .;

		__eshell_command_table_start = .;
		KEEP(*(.eshell_command_table))
		__eshell_command_table_end = .;

#ifndef NOSTD
		KEEP(*(.eh_frame*))
#endif
		*(.note.gnu.build-id)
		. = ALIGN(4);
	} > RAM

#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
	__cp_text_sram_start_flash__ = LOADADDR(.rodata) + SIZEOF(.rodata);

	.cp_text_sram (ORIGIN(RAMCPX)) : AT (__cp_text_sram_start_flash__)
	{
		__cp_text_sram_exec_start__ = .;
		*(.cp_text_sram*)
		*:cp_queue.o(.text*)
		. = ALIGN(4);
		__cp_text_sram_exec_end__ = .;
	} > RAMCPX

	__cp_text_sram_start = RAMX_TO_RAM(__cp_text_sram_exec_start__);
	__cp_text_sram_end = RAMX_TO_RAM(__cp_text_sram_exec_end__);
	__free_ram_cpx = ORIGIN(RAMCPX) + LENGTH(RAMCPX) - __cp_text_sram_exec_end__;

	.vector_table_cp (ORIGIN(RAMCP)) (NOLOAD) :
	{
		KEEP(*(.vector_table_cp))
		. = VECTOR_SECTION_SIZE;
		. = ALIGN(4);
	} > RAMCP

	__cp_data_sram_start_flash__ = __cp_text_sram_start_flash__ + SIZEOF(.cp_text_sram);

	.cp_data_sram : AT (__cp_data_sram_start_flash__)
	{
		__cp_data_sram_start = .;
		*(.cp_data*)
		*:cp_queue.o(.data* .rodata*)
		. = ALIGN(4);
		__cp_data_sram_end = .;
	} > RAMCP

	__cp_sram_end_flash__ = __cp_data_sram_start_flash__ + SIZEOF(.cp_data_sram);

	.cp_bss_sram (NOLOAD) : AT (ADDR(.cp_bss_sram))
	{
		__cp_bss_sram_start = .;
		*(.cp_bss*)
		. = ALIGN(4);
		__cp_bss_sram_end = .;
	} > RAMCP

	__cp_stack_top = ORIGIN(RAMCP) + LENGTH(RAMCP);
	__cp_stack_limit = __cp_stack_top - CP_STACK_SECTION_SIZE;
	__free_ram_cp = __cp_stack_limit - __cp_bss_sram_end;

	__etext = __cp_sram_end_flash__;
#if !(defined(PROGRAMMER_LOAD_RAM) && defined(LOAD_RAM_BASE))
	. = __etext;
#endif
#else
	__etext = LOADADDR(.rodata) + SIZEOF(.rodata);
#endif

	.data (ADDR(.rodata) + SIZEOF(.rodata)) : AT (__etext)
	{
		__data_start__ = .;
		*(.data*)
		. = ALIGN(4);

#ifndef NOSTD
		*(vtable)

		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);

		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		KEEP(*(.jcr*))
		. = ALIGN(4);
#endif
		/* All data end */
		__data_end__ = .;
	} > RAM

	.build_info (.) : AT (LOADADDR(.data) + SIZEOF(.data))
	{
		KEEP(*(.build_info))
		. = ALIGN(4);
	} > RAM = 0x00000000

	/* __boot_struct_build_info must be in the same address space as __exec_struct_start */
	__boot_struct_build_info = RAMRUN_SEC_ADDR(LOADADDR(.build_info));

	/* The following section be the last loaded section */
	__exec_load_end = LOADADDR(.build_info) + SIZEOF(.build_info);

	.code_start_addr (__exec_load_end) :
	{
#ifdef MTEST_BUILTIN
		LONG(ABSOLUTE(__boot_struct_size__));
#endif
		/* ROM only accepts secure address when downloading ramrun */
		LONG(ABSOLUTE(__exec_struct_start));
	} > LOAD_RAM

	.bss (NOLOAD) :
	{
		. = ALIGN(4);
		__pgm_bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__pgm_bss_end__ = .;
#ifndef NOSTD
		/* Avoid link errors when using toolchain lib */
		__bss_start__ = .;
		__bss_end__ = .;
#endif
	} > RAM_NOBITS

	.heap (NOLOAD) :
	{
		. = ALIGN(8);
		__HeapBase = .;
		__end__ = .;
		end = __end__;
		. += HEAP_SECTION_SIZE;
		. = ALIGN(8);
		__HeapLimit = .;
	} > RAM_NOBITS

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy (COPY) :
	{
		. = STACK_SECTION_SIZE;
		. = ALIGN(8);
	} > RAM_NOBITS

	/DISCARD/ :
	{
		*(.interp)
		*(.hash)
		*(.dynsym)
		*(.dynstr)
		*(.gnu.version)
		*(.gnu.version_d)
		*(.gnu.version_r)
		*(.rela.plt)
		*(.rela.data.rel.ro*)
		*(.rela.gnu.linkonce.d.rel.ro*)
		*(.rela.*)
		*(.plt)
		*(.dynamic)
		*(.iplt*)
		*(.igot*)
	}

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	__StackTop = ORIGIN(RAM_NOBITS) + LENGTH(RAM_NOBITS);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);

	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
	__free_ram_nobits = __StackLimit - __HeapLimit;

	__free_load_ram = ORIGIN(LOAD_RAM) + LENGTH(LOAD_RAM) -
		(ADDR(.code_start_addr) + SIZEOF(.code_start_addr));

#ifdef MAIN_RAM_USE_TCM
	__free_itcm = __free_load_ram;
	__free_dtcm = __free_ram_nobits;
#else
	__free_ram = __free_ram_nobits;
#endif

#ifdef MEM_POOL_BASE
	.mem_pool (ORIGIN(MEM_POOL)) (NOLOAD) :
	{
		__mem_pool_start__ = .;
		. = LENGTH(MEM_POOL);
		__mem_pool_end__ = .;
	} > MEM_POOL
#elif !(defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0) && defined(MEM_POOL_IN_CP_RAM))
	__mem_pool_start__ = __HeapLimit;
	__mem_pool_end__ = __StackLimit;
#endif

	ASSERT((__vector_table_start & (1K - 1)) == 0, "vector table alignment error")
}
