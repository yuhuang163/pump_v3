cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))

ccflags-y += \
	-Iplatform/drivers/bt_host \
	-Iutils/hwtimer_list \

ifeq ($(NOAPP),1)
ccflags-y += -DNOAPP
endif

ifeq ($(BTH_TRC_TO_SYS),1)
obj-y += ../../platform/drivers/rmt_trace/
TRC_FLAGS := -Iplatform/drivers/rmt_trace -DBTH_TRC_TO_SYS
ifeq ($(RMT_TRC_IN_MSG_CHAN),1)
TRC_FLAGS += -DRMT_TRC_IN_MSG_CHAN
endif
CFLAGS_bth_main.o += $(TRC_FLAGS)
CFLAGS_bth_trc_to_sys.o += $(TRC_FLAGS)
endif

ifeq ($(SYS2BTH_MSG_TEST),1)
obj-y += ../../tests/hwtest/rmt_msg_test.c
ccflags-y += -DSYS2BTH_MSG_TEST -Itests/hwtest
endif

ifeq ($(SLEEP_TEST),1)
CFLAGS_bth_main.o += -DSLEEP_TEST
ifeq ($(NO_TIMER),1)
CFLAGS_bth_main.o += -DNO_TIMER
endif
endif

ifeq ($(BTH_ISPI_RF_ENABLE),1)
CFLAGS_bth_main.o += -DBTH_ISPI_RF_ENABLE
endif
ifeq ($(BTH_ISPI_XTAL_ENABLE),1)
CFLAGS_bth_main.o += -DBTH_ISPI_XTAL_ENABLE
endif

ifeq ($(IQ_CALI_TEST),1)
export IQ_CALI_TEST       ?= 1
export IQ_CALI_ADDATA_LOG ?= 1
export TX_IQ_CAL          ?= 1
export RX_IQ_CAL          ?= 1
export BT_ECC_CONFIG_BLK  ?= 2
obj-y += \
	../../platform/drivers/bt/ \
	../../utils/heap/ \
	../../tests/ramrun_test/testcases/iq_cali_test.c

ccflags-y += -DIQ_CALI_TEST -Iservices/bt_if/inc -Iplatform/drivers/bt
subdir-ccflags-y += -DNEW_NV_RECORD_ENABLED
ifeq ($(IQ_CALI_ADDATA_LOG),1)
ccflags-y += -DIQ_CALI_ADDATA_LOG
endif
endif

ifeq ($(BTH_BT_TEST),1)
BTH_HWTEST := 1
obj-y += ../../tests/hwtest/bt_conn_test.c
ccflags-y += -DBTH_BT_TEST
ifeq ($(BT_SLEEP_ONLY),1)
ccflags-y += -DBT_SLEEP_ONLY
endif
endif

ifeq ($(COREMARK_TEST),1)
BTH_HWTEST := 1
obj-y += ../../tests/coremark/
ccflags-y += -DCOREMARK_TEST
endif

ifeq ($(DHRYSTONE_TEST),1)
BTH_HWTEST := 1
obj-y += ../../tests/dhrystone2/
ccflags-y += -DDHRYSTONE_TEST
endif

ifeq ($(SBC_TEST),1)
BTH_HWTEST := 1
obj-y += ../../multimedia/audio/codec/sbc/
obj-y += ../../tests/hwtest/sbc_test.c
ccflags-y += -Imultimedia/inc/
ccflags-y += -DSBC_TEST
SBC_REDUCE_SIZE := 1
ifeq ($(SBC_REDUCE_SIZE),1)
ccflags-y += -DSBC_REDUCE_SIZE
endif
ifeq ($(SBC_CURRENT_TEST),1)
ccflags-y += -DSBC_CURRENT_TEST
endif
endif

ifeq ($(BTH_HWTEST),1)
ccflags-y += -DHWTEST -Itests/hwtest
endif

ifeq ($(BTH_MEM_TEST),1)
obj-y += ../../tests/mem_test/cases/bth_mem_test.c
ccflags-y += -DBTH_MEM_TEST -Itests/mem_test/inc
ifneq ($(BTH_MEM_TEST_BUFF_SIZE),)
ccflags-y += -DBTH_MEM_TEST_BUFF_SIZE=$(BTH_MEM_TEST_BUFF_SIZE)
endif
BTH_MEM_TEST_CLOCK ?= 52M
CLOCK_STEM := $(subst M,,$(BTH_MEM_TEST_CLOCK))
ccflags-y += -DBTH_MEM_TEST_CLOCK=$(CLOCK_STEM)
endif

ifeq ($(PT_APP),1)
obj-y += ../../tests/mem_test/cases/pt_app/
ccflags-y += -DPT_APP -Itests/mem_test/cases/pt_app
endif

ifeq ($(BT_CONN_TEST),1)
obj-y += ../../tests/hwtest/bt_conn_test.c
ccflags-y += -DBT_CONN_TEST -Itests/hwtest
endif

ifeq ($(SINGLE_UART_TEST),1)
ifeq ($(TEST_UART_PORT),)
$(error need to specify TEST_UART_PORT, available value is 1,2,3,4)
endif
ifeq ($(CHIP),best1600)
ccflags-y += -DBTH_SINGLE_UART_TEST
endif
obj-y += ../mem_test/single_uart_test.c
ccflags-y += -DSINGLE_UART_TEST -DTEST_UART_PORT=$(TEST_UART_PORT)
ccflags-y += -Iplatform/drivers/bt_host -Iplatform/drivers/ana -Iplatform/hal -Itests/mem_test
endif

ifneq ($(DEBUG_PORT),)
CFLAGS_bth_main.o += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(SUBSYS_FLASH_BOOT),1)
CFLAGS_bth_main.o += -DSUBSYS_FLASH_BOOT
endif

ifeq ($(NO_FLASH_INIT),1)
CFLAGS_bth_main.o += -DNO_FLASH_INIT
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)
