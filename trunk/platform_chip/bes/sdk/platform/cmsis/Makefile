cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))

ifeq ($(NUTTX_BUILD),1)
NO_RETARGET_GCC := 1
endif
ifeq ($(NO_RETARGET_GCC),1)
obj-y := $(filter-out retarget_gcc.cpp, $(obj-y))
endif

ifeq ($(KERNEL),LITEOS_A)
NO_STACK_PROTECTOR := 1
endif
ifeq ($(NO_STACK_PROTECTOR),1)
obj-y := $(filter-out stack_protector.c, $(obj-y))
endif

ifeq ($(DSP_LIB),1)
obj-y += DSP_Lib/
endif

ifeq ($(NN_LIB),1)
obj-y += NN_Lib/
endif

ifeq ($(CMSIS_M55_LIB),1)
ifeq ($(DSP_LIB),1)
$(error DSP_LIB and CMSIS_M55_LIB can not be defined simultaneously!)
endif
ifeq ($(NN_LIB),1)
$(error NN_LIB and CMSIS_M55_LIB can not be defined simultaneously!)
endif
obj-y += ../../thirdparty/cmsis_m55_lib/
endif

ifeq ($(CMSIS_NN_M55_LIB),1)
ifeq ($(DSP_LIB),1)
$(error DSP_LIB and CMSIS_NN_M55_LIB can not be defined simultaneously!)
endif
ifeq ($(NN_LIB),1)
$(error NN_LIB and CMSIS_NN_M55_LIB can not be defined simultaneously!)
endif
obj-y += ../../thirdparty/cmsis_nn_m55_lib/
endif

ifneq ($(filter a%,$(CPU)),)
obj-y += ca/
endif

obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ifneq ($(LTO_CFLAGS),)
KBUILD_CFLAGS := $(filter-out $(LTO_CFLAGS),$(KBUILD_CFLAGS))
KBUILD_CPPFLAGS := $(filter-out $(LTO_CPPFLAGS),$(KBUILD_CPPFLAGS))
endif

ccflags-y := -I$(obj)/inc

MPU_FLAGS :=
ifeq ($(NO_MPU_DEFAULT_MAP),1)
MPU_FLAGS += -DNO_MPU_DEFAULT_MAP
endif
ifeq ($(MPU_DEBUG),1)
MPU_FLAGS += -DMPU_DEBUG
endif
CFLAGS_mpu_armv7m.o += $(MPU_FLAGS)
CFLAGS_mpu_armv8m.o += $(MPU_FLAGS)
CFLAGS_mpu_cfg.o += $(MPU_FLAGS)

ifeq ($(CP_FLASH_ACCESS),1)
CFLAGS_mpu_cfg.o += -DCP_FLASH_ACCESS
endif
ifeq ($(CP_PSRAM_ACCESS),1)
CFLAGS_mpu_cfg.o += -DCP_PSRAM_ACCESS
endif

FAULT_FLAGS :=
USAGE_FAULT ?= 1
ifeq ($(USAGE_FAULT),1)
FAULT_FLAGS += -DUSAGE_FAULT
endif
BUS_FAULT ?= 1
ifeq ($(BUS_FAULT),1)
FAULT_FLAGS += -DBUS_FAULT
endif
MEM_FAULT ?= 1
ifeq ($(MEM_FAULT),1)
FAULT_FLAGS += -DMEM_FAULT
endif
MONITOR_FAULT ?= 1
ifeq ($(MONITOR_FAULT),1)
FAULT_FLAGS += -DMONITOR_FAULT
endif
CFLAGS_system_ARMCM.o += $(FAULT_FLAGS)
CFLAGS_system_cp.o += $(FAULT_FLAGS)

CFLAGS_system_subsys.o += -Iutils/boot_struct

ifeq ($(SUBSYS_CACHE_ENABLE),1)
CFLAGS_system_subsys.o += -DSUBSYS_CACHE_ENABLE
endif
ifeq ($(SUBSYS_FLASH_BOOT),1)
CFLAGS_system_subsys.o += -DSUBSYS_FLASH_BOOT
endif
ifeq ($(SUBSYS_IMAGE_SEGMENT),1)
CFLAGS_system_subsys.o += -DSUBSYS_IMAGE_SEGMENT
endif
ifeq ($(SUBSYS_LOAD_SRAM_DATA),1)
CFLAGS_system_subsys.o += -DSUBSYS_LOAD_SRAM_DATA
endif

CFLAGS_system_utils.o += -Iutils/boot_struct

ifeq ($(TOOLCHAIN),armclang)
# Disable special handling and optimization of standard C library
# functions for booting codes (before memcpy/memset/... are ready)
CFLAGS_system_utils.o += -fno-builtin
endif

ifeq ($(COMPRESS_SRAM_CODE),1)
CFLAGS_system_utils.o += -DCOMPRESS_SRAM_CODE
CFLAGS_system_cp.o += -DCOMPRESS_SRAM_CODE
endif
ifeq ($(COMPRESS_PSRAM_CODE),1)
CFLAGS_system_utils.o += -DCOMPRESS_PSRAM_CODE
CFLAGS_system_cp.o += -DCOMPRESS_PSRAM_CODE
endif

ifeq ($(ACCURATE_DB_TO_FLOAT),1)
CFLAGS_system_utils.o += -DACCURATE_DB_TO_FLOAT
endif

ifeq ($(CHIP_HAS_PSRAM),1)
CFLAGS_system_utils.o += -DCHIP_HAS_PSRAM
CFLAGS_system_subsys.o += -DCHIP_HAS_PSRAM
endif
ifeq ($(CHIP_HAS_PSRAMUHS),1)
CFLAGS_system_utils.o += -DCHIP_HAS_PSRAMUHS
endif

ifeq ($(INTSRAM_RUN),1)
CFLAGS_system_utils.o += -DINTSRAM_RUN
endif

ifeq ($(RELOC_TRACE),1)
CFLAGS_reloc_run.o += -DRELOC_TRACE
else ifeq ($(RELOC_PRINTF),1)
CFLAGS_reloc_run.o += -DRELOC_PRINTF
endif

ifeq ($(SUBSYS_LOAD_FROM_FS),1)
CFLAGS_system_subsys.o += -DSUBSYS_LOAD_FROM_FS
endif

ifeq ($(SEPARATE_RAM_RAMX),1)
CFLAGS_system_subsys.o += -DSEPARATE_RAM_RAMX
endif

ifeq ($(NO_CACHE),1)
CFLAGS_system_utils.o += -DNO_CACHE
endif

