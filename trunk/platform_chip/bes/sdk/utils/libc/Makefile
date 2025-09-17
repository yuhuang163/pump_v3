cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(filter a%,$(CPU)),)
OPT_LIBC ?= 1
endif

ifneq ($(UNALIGNED_ACCESS),1)
ifeq ($(LIBC_ROM),1)
# Unaligned access is enabled for LIBC codes in ROM
override LIBC_ROM := 0
override LIBC_OVERRIDE := 1
endif
endif

ARM_STRCPY_SRCS := machine/arm/strcpy.c machine/arm/memchr.c
ARM_MEMCPY_SRCS := machine/arm/memcpy-arm.S machine/arm/aeabi_memset.c machine/arm/aeabi_memset-soft.S

BES_MEMCPY_SRCS := machine/arm/memcpy_move-kpv2.S machine/arm/memset-kp.S

# Core booting functions like memcpy/memset are written in assembly language and
# will not be affected by LTO option
ifeq (1,0)
ifneq ($(LTO_CFLAGS),)
KBUILD_CFLAGS := $(filter-out $(LTO_CFLAGS),$(KBUILD_CFLAGS))
KBUILD_CPPFLAGS := $(filter-out $(LTO_CPPFLAGS),$(KBUILD_CPPFLAGS))
endif
endif

ifeq ($(NOSTD)-$(NUTTX_BUILD),1-0)

# Use relative path to specify the include path (so that no change is needed after rename)
ccflags-y := -I$(obj)/inc

ifeq ($(LIBC_ROM),1)
	obj-y := libc_rom.c libc_rom_mem.c memcmp.c strlen.c strncmp.c

	CFLAGS_libc_rom.o += -Iutils/rom_utils
	CFLAGS_libc_rom_mem.o += -Iutils/rom_utils
else
	obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))
	EXCLUDE_SRCS := libc_rom.c libc_rom_mem.c
	obj-y := $(filter-out $(EXCLUDE_SRCS),$(obj-y))

	ifeq ($(OPT_LIBC),1)
		EXCLUDE_SRCS := memchr.c memcpy.c memmove.c memset.c strcpy.c
		obj-y := $(filter-out $(EXCLUDE_SRCS),$(obj-y))
		obj-y += $(ARM_STRCPY_SRCS)
		ifeq ($(UNALIGNED_ACCESS),1)
			obj-y += $(BES_MEMCPY_SRCS)
		else
			obj-y += $(ARM_MEMCPY_SRCS) memmove.c
		endif
	endif
endif

else ifeq ($(LIBC_ROM),1)

ifeq ($(NUTTX_BUILD),1)
	obj-y := libc_rom_mem.c

	CFLAGS_libc_rom_mem.o += -Iutils/rom_utils
else
	obj-y := libc_rom.c libc_rom_mem.c

	CFLAGS_libc_rom.o += -Iutils/rom_utils
	CFLAGS_libc_rom_mem.o += -Iutils/rom_utils
	# Avoid conflicting with libc in toolchain
	CFLAGS_libc_rom.o += -DNO_STRTOL
	CFLAGS_libc_rom.o += -DNO_STRTOUL
	CFLAGS_libc_rom.o += -DNO_VSSCANF
	CFLAGS_libc_rom.o += -DNO_ASSERT
endif

else ifeq ($(LIBC_OVERRIDE),1)

	# Add more LIBC functions to override
ifeq ($(OPT_LIBC),1)
	obj-y += $(ARM_STRCPY_SRCS)

	ifeq ($(UNALIGNED_ACCESS),1)
		obj-y += $(BES_MEMCPY_SRCS)
	else
		obj-y += $(ARM_MEMCPY_SRCS)
	endif
else
	obj-y := memcpy.c strcpy.c memchr.c memset.c memmove.c
endif

else

$(error Missing LIBC options)

endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ifeq ($(VSPRINTF_STR_WIDTH),1)
CFLAGS_vsprintf.o += -DVSPRINTF_STR_WIDTH
endif

# Avoid recursive calls when compiling C library codes
ifeq ($(TOOLCHAIN),armclang)
ccflags-y += -fno-builtin
else
ccflags-y += -fno-tree-loop-distribute-patterns
endif

ifeq ($(ENABLE_PRINT_FLOAT), 1)
CFLAGS_libc_rom.o += -DENABLE_PRINT_FLOAT
endif
