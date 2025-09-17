
ifeq ($(CHIP),best1502x)
include config/$(T)/best1502x_target.mk
endif

ifeq ($(CHIP),best1503)
include config/$(T)/best1503_target.mk
endif

ifeq ($(CHIP),best1700)
include config/$(T)/best1700_target.mk
endif

ifeq ($(CHIP),best1501p)
include config/$(T)/best1501p_target.mk
endif

ifeq ($(CHIP),best1600)

export COMPILE_TARGET    ?=    SYS_AS_MAIN
COMPILE_TARGET_VALID := 0

ifeq ($(COMPILE_TARGET),SYS_AS_MAIN)
KBUILD_CPPFLAGS += -D__SYS_AS_MAIN__
COMPILE_TARGET_VALID := 1
include config/$(T)/best1600_target.mk
else ifeq ($(COMPILE_TARGET),BTH_AS_MAIN)
KBUILD_CPPFLAGS += -D__BTH_AS_MAIN__
COMPILE_TARGET_VALID := 1
include config/$(T)/best1600_bth_as_main.mk
else ifeq ($(COMPILE_TARGET),BTH_AS_SUBSYS)
KBUILD_CPPFLAGS += -D__BTH_AS_SUBSYS__
COMPILE_TARGET_VALID := 1
include config/$(T)/best1600_bth_as_subsys.mk
else ifeq ($(COMPILE_TARGET),M55_AS_SUBSYS)
KBUILD_CPPFLAGS += -D__M55_AS_SUBSYS__
COMPILE_TARGET_VALID := 1
include config/$(T)/best1600_m55_as_subsys.mk
endif

ifeq ($(COMPILE_TARGET_VALID),0)
$(error Invalid COMPILE_TARGET=$(COMPILE_TARGET) for COMPILE_TARGET=$(COMPILE_TARGET))
endif

endif
