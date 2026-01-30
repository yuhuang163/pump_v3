cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y := -Iutils/boot_struct -Iplatform/drivers/norflash -Iplatform/drivers/nandflash

ifeq ($(FLASH_UNIQUE_ID),1)
CFLAGS_spi_norflash_drv.o += -DFLASH_UNIQUE_ID
CFLAGS_spi_nandflash_drv.o += -DFLASH_UNIQUE_ID
CFLAGS_spi_flash.o += -DFLASH_UNIQUE_ID
endif

ifeq ($(FLASH_PAGE_MPM_ENABLE),1)
CFLAGS_spi_flash.o += -DFLASH_PAGE_MPM_ENABLE
endif

ifeq ($(SPI_FLASH_USING_SPILCD),1)
CFLAGS_spi_flash_trans.o += -DSPI_FLASH_USING_SPILCD
endif

ifeq ($(SPI_FLASH_DEBUG),1)
CFLAGS_spi_flash_trans.o += -DSPI_FLASH_DEBUG
CFLAGS_spi_norflash_drv.o += -DSPI_FLASH_DEBUG
CFLAGS_spi_nandflash_drv.o += -DSPI_FLASH_DEBUG
CFLAGS_spi_flash.o += -DSPI_FLASH_DEBUG
endif
