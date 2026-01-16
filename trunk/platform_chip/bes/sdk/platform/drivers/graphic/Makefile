cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c :=

ifeq ($(CHIP_HAS_GPU),1)
obj_c += gpu/
endif

ifeq ($(CHIP_HAS_MINI3D),1)
obj_c += ../../../thirdparty/gllite/
endif

ifeq ($(CHIP_HAS_LCDC),1)
obj_c += lcdc/
endif

ifneq ($(filter 1, $(FB_DRV) $(FBTEST_DRV)),)
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

subdir-ccflags-y += \
				   -Iutils/heap/ \
				   -Iplatform/drivers/graphic \
				   -Iplatform/drivers/graphic/fb \
				   -Iplatform/drivers/graphic/lcdc \
				   -Iplatform/drivers/graphic/lcd
