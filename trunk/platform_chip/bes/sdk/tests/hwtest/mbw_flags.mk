
ccflags-y += -DMBW_TEST

ifeq ($(MBW_SRAM),1)
CFLAGS_mbw_test.o += -DMBW_SRAM
endif
ifeq ($(MBW_A7_SRAM),1)
CFLAGS_mbw_test.o += -DMBW_A7_SRAM
endif
ifeq ($(MBW_PSRAM),1)
CFLAGS_mbw_test.o += -DMBW_PSRAM
endif
ifeq ($(MBW_PSRAM_NC),1)
CFLAGS_mbw_test.o += -DMBW_PSRAM_NC
endif
ifeq ($(MBW_PSRAMUHS),1)
CFLAGS_mbw_test.o += -DMBW_PSRAMUHS
endif
ifeq ($(MBW_PSRAMUHS_NC),1)
CFLAGS_mbw_test.o += -DMBW_PSRAMUHS_NC
endif
ifeq ($(MBW_FLASH),1)
CFLAGS_mbw_test.o += -DMBW_FLASH
endif
ifeq ($(MBW_FLASH_NC),1)
CFLAGS_mbw_test.o += -DMBW_FLASH_NC
endif
ifeq ($(MBW_TEST_RW_ONLY),1)
CFLAGS_mbw_test.o += -DMBW_TEST_RW_ONLY
endif
