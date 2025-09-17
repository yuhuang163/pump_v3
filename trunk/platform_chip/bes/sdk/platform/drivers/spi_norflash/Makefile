cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y := -Iutils/boot_struct -Iplatform/drivers/norflash

ifeq ($(FLASH_UNIQUE_ID),1)
CFLAGS_spi_norflash_drv.o += -DFLASH_UNIQUE_ID
endif

ifeq ($(SPI_NORFLASH_USING_SPILCD),1)
CFLAGS_spi_norflash_drv.o += -DSPI_NORFLASH_USING_SPILCD
endif

ifeq ($(SPI_NORFLASH_DRV_DEBUG),1)
CFLAGS_spi_norflash_drv.o += -DSPI_NORFLASH_DRV_DEBUG
endif

