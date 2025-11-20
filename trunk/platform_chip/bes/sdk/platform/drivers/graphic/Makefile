cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c :=
#obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

ifeq ($(CHIP_HAS_GPU),1)
obj_c += gpu/
endif

ifeq ($(CHIP_HAS_LCDC),1)
obj_c += lcdc/
endif

ifeq ($(TEST_WITH_GPU),1)
subdir-ccflags-y += -DTEST_WITH_GPU
endif

ifeq ($(GRAPHIC_TEST),1)
obj_c += test_main.c check_raw_image.c
endif

ifeq ($(FBTEST_DRV),1)
obj_c += lcd/
obj_c += fb/
endif

ifeq ($(NUTTX_BUILD),1)
obj_c += osl/gra_sem_nuttx.c
subdir-ccflags-y += -DGRA_SEM_NUTTX -Iplatform/drivers/graphic/osl/
else ifeq ($(RTOS),1)
obj_c += osl/gra_sem_cmsis.c
subdir-ccflags-y += -DGRA_SEM_CMSIS -Iplatform/drivers/graphic/osl/
else
obj_c += osl/gra_sem_no_os.c
subdir-ccflags-y += -DGRA_SEM_NO_OS -Iplatform/drivers/graphic/osl/
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)


ifeq ($(GRADRV_TMST), 1)
subdir-ccflags-y += -DGRAPHICDRV_TIME_STATIS
endif

ifeq ($(GRADRV_DEBUG), 1)
subdir-ccflags-y += -DGRAPHIC_DRV_DEBUG
endif

ifeq ($(TEST_WITH_LCD), 1)
subdir-ccflags-y += -DTEST_WITH_LCD
endif

ifeq ($(BAREMETAL), 1)
subdir-ccflags-y += -D_BAREMETAL=1
endif

ifneq ($(BOARD_NAME), )
subdir-ccflags-y += -DBOARD_NAME=$(BOARD_NAME)
endif

subdir-ccflags-y += \
				   -Iutils/heap/ \
				   -Iplatform/drivers/graphic \
				   -Iplatform/drivers/graphic/fb \
				   -Iplatform/drivers/graphic/lcdc \
				   -Iplatform/drivers/graphic/lcd

