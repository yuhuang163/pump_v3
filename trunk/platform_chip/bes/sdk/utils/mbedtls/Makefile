
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := library/

LIC_FILE_PATTERN := NOTICE

ifeq ($(MBEDTLS_TEST),1)
obj-y += tests/
endif

ifeq ($(MBEDTLS_PORTS),1)
obj-y += ports/
ifneq ($(MBEDTLS_CONFIG_FILE),)
subdir-ccflags-y += -DMBEDTLS_CONFIG_FILE='"$(MBEDTLS_CONFIG_FILE)"'
else
subdir-ccflags-y += -DMBEDTLS_CONFIG_FILE='"mbedtls_ports_config.h"'
endif # MBEDTLS_CONFIG_FILE
subdir-ccflags-y += -Iutils/mbedtls/ports/include
else # MBEDTLS_PORTS != 1
ifneq ($(MBEDTLS_CONFIG_FILE),)
subdir-ccflags-y += -DMBEDTLS_CONFIG_FILE='"$(MBEDTLS_CONFIG_FILE)"'
else
subdir-ccflags-y += -DMBEDTLS_CONFIG_FILE='<config-rsa.h>'
endif # MBEDTLS_CONFIG_FILE
endif # MBEDTLS_PORTS

ifeq ($(KERNEL), RTX5)
# Configure RTX5 HTTPS.
# There are defects in RTX5 memory management, resulting in problems with HTTPS functions.
# Use the built-in memory management function of mbedtls to replace the memory management function of RTX5.
# HTTPS with full certificate resolution and request requires at least 372KB(0x5D000) of memory.
export MBEDTLS_MEMORY_BUFFER_SIZE ?= 0
ifneq ($(MBEDTLS_MEMORY_BUFFER_SIZE), 0)
subdir-ccflags-y += -DMBEDTLS_MEMORY_BUFFER_ALLOC_C
subdir-ccflags-y += -DMBEDTLS_MEMORY_BUFFER_SIZE=$(MBEDTLS_MEMORY_BUFFER_SIZE)
endif # MBEDTLS_MEMORY_BUFFER_SIZE
endif # RTX5

ifeq ($(NET_SUPPORT),1)
subdir-ccflags-y += -Inet/lwip_2.1.2/src/include/compat/posix
subdir-ccflags-y += -D__DEFINED_socklen_t
endif

