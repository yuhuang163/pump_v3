FREETYPE_MODULES := \
        autofit \
        bdf \
        cff \
        cid \
        gzip \
        pcf \
        pfr \
        psaux \
        pshinter \
        psnames \
        raster \
        sdf \
        sfnt \
        smooth \
        svg \
        truetype \
        type1 \
        type42 \
        winfonts

SRC_DIR := ../../freetype/src
BASE_DIR := $(SRC_DIR)/base
SYSTEM_ZLIB := 1

include $(FREETYPE_DIR)/src/base/rules.mk
include $(patsubst %,$(FREETYPE_DIR)/src/%/rules.mk,$(FREETYPE_MODULES))
include $(FREETYPE_DIR)/src/dlg/rules.mk

BASE_SRC += $(BASE_DIR)/ftinit.c \
            $(BASE_DIR)/ftmm.c \
            $(BASE_DIR)/ftdebug.c \
            $(BASE_DIR)/ftglyph.c \
            $(BASE_DIR)/ftbitmap.c \
            $(BASE_DIR)/ftstroke.c

CSRCS += $(BASE_SRC) $(FT_SYS_SRC) $(FT_TEST_SRC) $(GZIP_DRV_SRC)
CSRCS += $(AUTOF_DRV_SRC) $(BDF_DRV_SRC) $(CFF_DRV_SRC) $(CID_DRV_SRC)
CSRCS += $(PCF_DRV_SRC) $(PFR_DRV_SRC) $(PSAUX_DRV_SRC) $(PSHINTER_DRV_SRC)
CSRCS += $(PSNAMES_DRV_SRC) $(RASTER_DRV_SRC) $(SDF_DRV_SRC)
CSRCS += $(SFNT_DRV_SRC) $(SMOOTH_DRV_SRC) $(SVG_DRV_SRC)
CSRCS += $(TT_DRV_SRC) $(T1_DRV_SRC) $(T42_DRV_SRC) $(FNT_DRV_SRC)

