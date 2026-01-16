
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

ifeq ($(TEST_WITH_GPU),1)
subdir-ccflags-y += -DTEST_WITH_GPU
endif

ifeq ($(TEST_WITH_LCD), 1)
subdir-ccflags-y += -DTEST_WITH_LCD
endif

ifneq ($(BOARD_NAME), )
subdir-ccflags-y += -DBOARD_NAME=$(BOARD_NAME)
endif

subdir-ccflags-y += -DFB_TEST

subdir-ccflags-y += \
            -Itests/hwtest/ \
            -Iutils/hwtimer_list/

ifeq ($(FB_DSI_60HZ_TEST_CURRENT_TEST_1HZ_REFRESH),1)
subdir-ccflags-y += -DFB_DSI_60HZ_TEST_CURRENT_TEST_1HZ_REFRESH
endif

# LCD panel select
ifeq ($(DISPLAY_LCD),rm69330)
subdir-ccflags-y += -DDISPLAY_RM69330
else ifeq ($(DISPLAY_LCD),gc9503np)
subdir-ccflags-y += -DDISPLAY_GC9503NP
else ifeq ($(DISPLAY_LCD),st7701sn)
subdir-ccflags-y += -DDISPLAY_ST7701SN
else ifeq ($(DISPLAY_LCD),st7701s)
subdir-ccflags-y += -DDISPLAY_ST7701S
else ifeq ($(DISPLAY_LCD),st7703)
subdir-ccflags-y += -DDISPLAY_ST7703
else ifeq ($(DISPLAY_LCD),st7703ds)
subdir-ccflags-y += -DDISPLAY_ST7703DS
else ifeq ($(DISPLAY_LCD),st7785m)
subdir-ccflags-y += -DDISPLAY_ST7785M
else ifeq ($(DISPLAY_LCD),a064)
subdir-ccflags-y += -DDISPLAY_A064
else ifeq ($(DISPLAY_LCD),hx8394f)
subdir-ccflags-y += -DDISPLAY_HX8394F
else ifeq ($(DISPLAY_LCD),hx8394f720)
subdir-ccflags-y += -DDISPLAY_HX8394F720
else ifeq ($(DISPLAY_LCD),hx8399c)
subdir-ccflags-y += -DDISPLAY_HX8399C
else ifeq ($(DISPLAY_LCD),icna3310)
subdir-ccflags-y += -DDISPLAY_ICNA3310
else ifeq ($(DISPLAY_LCD),rm69090)
subdir-ccflags-y += -DDISPLAY_RM69090
else ifeq ($(DISPLAY_LCD),9c01)
subdir-ccflags-y += -DDISPLAY_9C01
else ifeq ($(DISPLAY_LCD),ili9881c)
subdir-ccflags-y += -DDISPLAY_ILI9881C
else ifeq ($(DISPLAY_LCD),fl7703ni)
subdir-ccflags-y += -DDISPLAY_FL7703NI
else ifeq ($(DISPLAY_LCD),eqt400)
subdir-ccflags-y += -DDISPLAY_EQT400
else ifeq ($(DISPLAY_LCD),rm690a0)
subdir-ccflags-y += -DDISPLAY_RM690A0
else ifeq ($(DISPLAY_LCD),st77916)
subdir-ccflags-y += -DDISPLAY_ST77916
else
subdir-ccflags-y += -DDISPLAY_RM69330
endif

ifneq ($(DISPLAY_RST_PIN),)
subdir-ccflags-y += -DDISPLAY_RST_PIN=$(DISPLAY_RST_PIN)
endif
ifneq ($(DISPLAY_TE_PIN),)
subdir-ccflags-y += -DDISPLAY_TE_PIN=$(DISPLAY_TE_PIN)
endif

# Display BUS select
ifeq ($(DISPLAY_BUS),qspi)
subdir-ccflags-y += -DCONFIG_LCDC_USE_QSPI
subdir-ccflags-y += -DCONFIG_LCDC_QSPI
else ifeq ($(DISPLAY_BUS),rgb)
subdir-ccflags-y += -DCONFIG_LCDC_RGB
else ifeq ($(DISPLAY_BUS),mip)
subdir-ccflags-y += -DCONFIG_LCDC_MIP
else ifeq ($(DISPLAY_BUS),dual_dsi)
subdir-ccflags-y += -DCONFIG_LCDC_DUAL_DSI -DCONFIG_LCDC_DSI
else ifeq ($(DISPLAY_BUS),dsi)
subdir-ccflags-y += -DCONFIG_LCDC_DSI
else ifeq ($(DISPLAY_BUS),dsi1)
subdir-ccflags-y += -DCONFIG_LCDC_USE_DSI1
else
ifeq ($(CHIP),best1503)
subdir-ccflags-y += -DCONFIG_LCDC_USE_QSPI
subdir-ccflags-y += -DCONFIG_LCDC_QSPI
else ifeq ($(CHIP),best1306p)
subdir-ccflags-y += -DCONFIG_LCDC_USE_QSPI
subdir-ccflags-y += -DCONFIG_LCDC_QSPI
else
subdir-ccflags-y += -DCONFIG_LCDC_DSI
endif
endif

# force video mode
ifeq ($(DISPLAY_VIDEO_MODE),1)
subdir-ccflags-y += -DCONFIG_DSI_VIDEO_MODE
endif
