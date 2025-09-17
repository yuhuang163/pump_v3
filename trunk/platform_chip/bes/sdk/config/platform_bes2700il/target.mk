CHIP		?= best1503

DEBUG       ?= 1

FPGA        ?= 0

RTOS        ?= 1

LIBC_ROM    ?= 0

KERNEL      ?= RTX5

ifeq ($(RTOS), 1)
export OS_DYNAMIC_MEM_SIZE ?= 16*1024
ifeq ($(KERNEL), FREERTOS)
KBUILD_CPPFLAGS += -DconfigTOTAL_HEAP_SIZE=$(OS_DYNAMIC_MEM_SIZE)
endif
endif

VERSION_INFO ?= best1503_tester

export LIBC_OVERRIDE ?= 1

export USER_SECURE_BOOT ?= 0
# enable:1
# disable:0

export OSC_26M_X4_AUD2BB	        ?= 1

export ULTRA_LOW_POWER		        ?= 1

export POWER_MODE			?= DIG_DCDC

BT_APP  ?= 0
export NO_BT_DRIVER ?= 1

export ASSERT_SHOW_FILE_FUNC ?= 1

DEBUG_PORT  ?= 1
# 0: usb
# 1: uart0
# 2: uart1

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
# ALL

export NO_OVERLAY ?= 1

export CURRENT_TEST ?= 0

ifeq ($(CURRENT_TEST),1)
export SMALL_RET_RAM ?= 1
export CORE_SLEEP_POWER_DOWN 		?= 1
#INTSRAM_RUN ?= 1
endif
ifeq ($(INTSRAM_RUN),1)
LDS_FILE ?= best1000_intsram.lds
else
LDS_FILE ?= best1000.lds
endif

KBUILD_CPPFLAGS += -D__SYS_AS_MAIN__

export FLASH_SIZE ?= 0x400000
export FLASH_SUSPEND ?= 1

export UTILS_ESHELL_EN ?= 0

export ESHELL_ON_TRACE ?= 1

export TRACE_BUF_SIZE ?= 4*1024

export TRACE_BAUD_RATE ?= 8*115200

export IDLE_MIN_SYS_FREQ_MHZ ?= 52

export WATCHER_DOG ?= 1

init-y		:=
core-y		:= platform/main/ platform/cmsis/ platform/drivers/ platform/hal/ apps/common/
core-y      += utils/crc32/ utils/heap/ utils/cqueue/

core-y += tests/besair_platform/bsp_tester/

# ifeq ($(ESHELL_UNITTEST),1)
# core-y      += services/audioflinger/
# endif

NEW_NV_RECORD_ENABLED ?= 0
ifeq ($(NEW_NV_RECORD_ENABLED),1)
KBUILD_CPPFLAGS += -DNEW_NV_RECORD_ENABLED
core-y      += services/nv_section/userdata_section/
export BT_ADDR_PRINT_CONFIG ?= 0
endif

ifeq ($(TFLITE_MICRO), 1)
export NN_LIB ?= 1
export DSP_LIB ?= 1
core-y	+= utils/tflite_micro/
endif

export PLATFORM_BES2700IL ?= 1

ifeq ($(FATFS_ENABLE),1)
KBUILD_CPPFLAGS += -DFS_FATFS_ENABLE
core-y      += services/fs/mbed/fat/ services/fs/fatfs/
core-y      += services/fs/block/
endif

ifeq ($(LITTLEFS_ENABLE),1)
KBUILD_CPPFLAGS += -DFS_LITTLEFS_ENABLE
core-y      += services/fs/mbed/littlefs/ services/fs/littlefs/
core-y      += services/fs/block/
endif

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Iservices/audioflinger -Iapps/common -Iapps/audioplayers -Imultimedia/inc/audio/codec/sbc/inc \
				-Itests/besair_platform/bsp_tester/dac_eq_interface -Iinclude/beco -Iinclude/cmsis_dsp -Iinclude/cmsis_nn -Iinclude/beco/beco_nn

export RTC_CALENDAR ?= 0


export USE_NORFLASH_API ?= 0
ifeq ($(USE_NORFLASH_API), 1)
KBUILD_CPPFLAGS += -DUSE_NORFLASH_API
core-y += services/norflash_api/
core-y += services/nv_section/log_section/
endif

export CPU_PC_DUMP ?= 1

export CORE_DUMP ?= 0
ifeq ($(CORE_DUMP), 1)
export DUMP_RAM_RAW_ONLY ?= 0
ifeq ($(DUMP_RAM_RAW_ONLY), 1)
KBUILD_CPPFLAGS += -DDUMP_RAM_RAW_ONLY
endif
ifeq ($(USE_NORFLASH_API), 1)
export CORE_DUMP_TO_FLASH ?= 0
ifeq ($(CORE_DUMP_TO_FLASH), 1)
export CORE_DUMP_SECTION_SIZE ?= 1408*1024
endif
endif # USE_NORFLASH_API
endif # CORE_DUMP

ifeq ($(USE_NORFLASH_API), 1)
export USER_DATA_TO_FLASH ?= 0
ifeq ($(USER_DATA_TO_FLASH), 1)
export USER_DATA_SECTION_SIZE ?= 4*1024
endif

export USER_GPS_TO_FLASH ?= 0
ifeq ($(USER_GPS_TO_FLASH), 1)
export USER_GPS_SECTION_SIZE ?= 60*1024
ifneq ($(USER_GPS_SECTION_SIZE),)
KBUILD_CPPFLAGS += -DUSER_GPS_SECTION_SIZE=$(USER_GPS_SECTION_SIZE)
endif
endif # USER_GPS_TO_FLASH
endif # USE_NORFLASH_API

export PSRAM_ENABLE ?=0
export PSRAM_SPEED ?= 0
ifeq ($(PSRAM_ENABLE),1)
export PSRAM_SIZE ?= 0x800000
export PSRAM_SPEED ?= 200
export PSRAM_XCCELA_MODE ?= 1

KBUILD_CPPFLAGS += -DSYS_PSRAM_BASE=PSRAM_BASE -DSYS_PSRAM_NC_BASE=PSRAM_NC_BASE
endif

export ALLOW_WARNING ?= 1

export OS_TASKCNT ?= 16
export OS_SCHEDULERSTKSIZE ?= 512
export OS_IDLESTKSIZE ?= 1024
export OS_FIFOSZ ?= 24

LIB_LDFLAGS += -lstdc++ -lsupc++

export ENABLE_PRINT_FLOAT ?= 1

ifeq ($(OPENAMP_ENABLE),1)
export RPMSG_ROLE_MASTER 	?= 1
export RPTUN_CPU2SENS    	?= 1

core-y += services/openamp/
endif

# export CHIP_HAS_JPEG ?= 1

export TEST_RGB565 ?= 0
ifeq ($(TEST_RGB565),1)
KBUILD_CPPFLAGS += -DTEST_RGB565
endif

ifneq ($(TEST_HAL_PATTERN),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN
endif

export ENABLE_LVGL ?= 0
ifeq ($(TEST_HAL_PATTERN),TEST_LVGL)
export ENABLE_LVGL := 1
export I2C2_IOMUX_INDEX := 56
endif
ifeq ($(ENABLE_LVGL),1)
export LVGL_GPU_VGLITE ?= 1
export LVGL_GPU_ARM2D ?= 0
export LVGL_TP_TMA525C ?= 1
core-y += thirdparty/lvgl/
ifeq ($(LVGL_GPU_ARM2D),1)
export ENABLE_ARM2D := 1
endif
endif

ifeq ($(TEST_HAL_PATTERN),TEST_TMA525C)
export I2C2_IOMUX_INDEX := 56
endif

export ENABLE_ARM2D ?= 0
ifeq ($(TEST_HAL_PATTERN),TEST_ARM2D)
export ENABLE_ARM2D := 1
endif
ifeq ($(ENABLE_ARM2D),1)
core-y += thirdparty/arm2d/
core-y += platform/cmsis/DSP_Lib/
endif

ifneq ($(STACK_SECTION_SIZE),)
KBUILD_CPPFLAGS += -DSTACK_SECTION_SIZE=$(STACK_SECTION_SIZE)
else
KBUILD_CPPFLAGS += -DSTACK_SECTION_SIZE=0x1000
endif
ifneq ($(HEAP_SECTION_SIZE),)
KBUILD_CPPFLAGS += -DHEAP_SECTION_SIZE=$(HEAP_SECTION_SIZE)
else
KBUILD_CPPFLAGS += -DHEAP_SECTION_SIZE=0x2000
endif
KBUILD_CPPFLAGS += -DFAST_XRAM_SECTION_SIZE=0x400


ifeq ($(ENABLE_BECO),1)
export NN_LIB ?= 1
KBUILD_CPPFLAGS += -Iinclude/cmsis_dsp -Iinclude/beco -Iinclude/cmsis_dsp
core-y += platform/beco/
endif

export FLASH_DEEP_SLEEP ?= 1
export CORE_SLEEP_POWER_DOWN ?= 1
export SPLIT_CORE_M_CORE_L ?= 0
export SLEEP_TEST ?= 1
export FLASH_HIGH_SPEED ?= 1
ifeq ($(FLASH_HIGH_SPEED), 1)
export FLASH_SPEED_MHZ := 104
export FLASH_SRC_FREQ_MHZ := 208
endif

export CP_SUBSYS_TEST ?= 0

ifeq ($(CP_SUBSYS_TEST),1)
export CP_SUBSYS_TRC_TO_MCU ?= 1
export CP_AS_SUBSYS ?= 1
export CP_INTR_RELAY ?= 1
ifneq ($(CP_AS_SUBSYS),1)
$(error CP_AS_SUBSYS=1 missing when CP_SUBSYS_TEST=1)
endif
obj-y += tests/besair_platform/bsp_tester/cp_subsys_test.c

KBUILD_CPPFLAGS += -DCP_SUBSYS_TEST \
			 -Iplatform/drivers/cp_subsys
ifeq ($(NO_TIMER),1)
CFLAGS_cp_subsys_test.o += -DNO_TIMER
endif
ifeq ($(MCU2CP_MSG_TEST),1)
CFLAGS_cp_subsys_test.o += -DMCU2CP_MSG_TEST
endif
ifeq ($(CP_SUBSYS_TRC_TO_MCU),1)
RMT_TRACE := 1
export RMT_TRC_IN_MSG_CHAN ?= 1
endif
endif # CP_SUBSYS_TEST

export RC_CLK_ENABLE ?= 1
ifeq ($(RC_CLK_ENABLE),1)
KBUILD_CPPFLAGS += -DRC_CLK_ENABLE
export PMU_FOCE_LP_MODE ?= 1
export SYS_RC_ENABLE ?= 1
else
export SYS_USE_BBPLL ?= 1
endif

export COREMARK_DHRY_BUILD ?= gcc

ifeq ($(TEST_HAL_PATTERN),TEST_SPI_SLAVE)
export MCU_SPI_SLAVE := 1
export MCU_SPI_SLAVE_SPECIFY_MEM ?= 0
ifeq ($(MCU_SPI_SLAVE_SPECIFY_MEM), 1)
KBUILD_CPPFLAGS += -DMCU_SPI_SLAVE_SPECIFY_MEM
export MCU_SPI_SLAVE_BUF_SIZE ?= 10*1024
ifneq ($(MCU_SPI_SLAVE_BUF_SIZE),)
KBUILD_CPPFLAGS += -DMCU_SPI_SLAVE_BUF_SIZE=$(MCU_SPI_SLAVE_BUF_SIZE)
endif
endif # MCU_SPI_SLAVE_SPECIFY_MEM
export USE_BBK_SPI_IF ?= 0
ifeq ($(USE_BBK_SPI_IF),1)
KBUILD_CPPFLAGS += -DUSE_BBK_SPI_IF
endif
export USE_SPI_DUMP_CRASH ?= 0
ifeq ($(USE_SPI_DUMP_CRASH),1)
KBUILD_CPPFLAGS += -DUSE_SPI_DUMP_CRASH
endif
export CORE_DUMP_TO_SPI ?= 0
ifeq ($(CORE_DUMP_TO_SPI),1)
KBUILD_CPPFLAGS += -DCORE_DUMP_TO_SPI
endif
export SPI_WAKEUP ?= 0
ifeq ($(SPI_WAKEUP), 1)
KBUILD_CPPFLAGS += -DSPI_WAKEUP
endif
endif # TEST_SPI_SLAVE

ifeq ($(TEST_HAL_PATTERN),TEST_SPI)
export USE_BBK_SPI_IF ?= 0
ifeq ($(USE_BBK_SPI_IF),1)
KBUILD_CPPFLAGS += -DUSE_BBK_SPI_IF
endif
export SPI_WAKEUP ?= 0
ifeq ($(SPI_WAKEUP), 1)
KBUILD_CPPFLAGS += -DSPI_WAKEUP
endif
export MCU_SPI_SLAVE_SPECIFY_MEM ?= 0
ifeq ($(MCU_SPI_SLAVE_SPECIFY_MEM), 1)
KBUILD_CPPFLAGS += -DMCU_SPI_SLAVE_SPECIFY_MEM
export MCU_SPI_SLAVE_BUF_SIZE ?= 10*1024
ifneq ($(MCU_SPI_SLAVE_BUF_SIZE),)
KBUILD_CPPFLAGS += -DMCU_SPI_SLAVE_BUF_SIZE=$(MCU_SPI_SLAVE_BUF_SIZE)
endif
endif # MCU_SPI_SLAVE_SPECIFY_MEM
endif # TEST_SPI

export CHIP_DMA_CFG_IDX ?= 1
ifeq ($(CHIP_DMA_CFG_IDX), 1)
KBUILD_CPPFLAGS += -DCHIP_DMA_CFG_IDX
endif

ifeq ($(EVR_TRACING), 1)

export NO_SLEEP := 1

export SYSTEM_USE_FILE	?= 0
ifeq ($(SYSTEM_USE_FILE),1)
KBUILD_CPPFLAGS += -DSYSTEM_USE_FILE

export SYSTEMVIEW_TO_PERI ?= uart
ifeq ($(SYSTEMVIEW_TO_PERI), uart)
KBUILD_CPPFLAGS += -DSYSTEMVIEW_TO_UART
export UTILS_ESHELL_EN := 1
export AUDIO_DEBUG := 1
ifeq ($(AUDIO_DEBUG),1)
export AUDIO_DEBUG
export AUDIO_DEBUG_CMD ?= 1
KBUILD_CPPFLAGS += -DAUDIO_DEBUG
core-y += services/audio_dump/
endif
endif

ifeq ($(SYSTEMVIEW_TO_PERI), flash)
KBUILD_CPPFLAGS += -DSYSTEMVIEW_TO_FLASH
endif

ifeq ($(SYSTEMVIEW_TO_PERI), spi)
KBUILD_CPPFLAGS += -DSYSTEMVIEW_TO_SPI
endif
endif # SYSTEM_USE_FILE
endif # EVR_TRACING

ifeq ($(TEST_HAL_PATTERN), TEST_SYSVIEW)

ifeq ($(SYSTEMVIEW_TO_PERI), spi)
export MCU_SPI_SLAVE := 1
endif

export USE_BBK_SPI_IF := 1
ifeq ($(USE_BBK_SPI_IF),1)
KBUILD_CPPFLAGS += -DUSE_BBK_SPI_IF
endif

endif # TEST_SYSVIEW

export STORE_ISPI_LIST ?= 1
ifeq ($(STORE_ISPI_LIST),1)
KBUILD_CPPFLAGS += -DSTORE_ISPI_LIST
endif

ifeq ($(TEST_HAL_PATTERN), CMSIS_TEST)
LDS_CPPFLAGS += -DCMSIS_TEST
core-y += platform/cmsis/DSP_Lib/
endif
