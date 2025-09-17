BECO_TEST_ITEM ?= BECO_FULLY_CONNECTED_S8_TEST

# BECO_API_TEST
# BECO_MATRIX_TEST
# BECO_FULLY_CONNECTED_S8_TEST
# BECO_FULLY_CONNECTED_S16_TEST
# BECO_CONVOLVE_S8_TEST
# BECO_CONVOLVE_S16_TEST
# BECO_CONVOLVE_1X1_S8_TEST
# BECO_CONVOLVE_1X1_S16_TEST
# BECO_DEPTHWISE_CONV_S8_TEST
# BECO_DEPTHWISE_CONV_S16_TEST
# BECO_AVGPOOL_S8_TEST
# BECO_AVGPOOL_S16_TEST
# BECO_MAX_POOL_S8_TEST
# BECO_MAX_POOL_S16_TEST
# BECO_GRU_S16_TEST
# BECO_LSTM_S16_TEST


ifeq ($(BECO_TEST_ITEM), BECO_API_TEST)
obj-y += beco_api_test.o
ccflags-y += -DBECO_API_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_MATRIX_TEST)
obj-y += matrix_mult.o
ccflags-y += -DBECO_MATRIX_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_FULLY_CONNECTED_S8_TEST)
obj-y += fully_connected_s8_test/beco_fully_connected_s8_test.o
ccflags-y += -DBECO_FULLY_CONNECTED_S8_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_FULLY_CONNECTED_S16_TEST)
obj-y += fully_connected_s16_test/beco_fully_connected_s16_test.o
ccflags-y += -DBECO_FULLY_CONNECTED_S16_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_CONVOLVE_S8_TEST)
obj-y += convolve_s8_test/beco_convolve_s8_test.o
ccflags-y += -DBECO_CONVOLVE_S8_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_CONVOLVE_S16_TEST)
obj-y += convolve_s16_test/beco_convolve_s16_test.o
ccflags-y += -DBECO_CONVOLVE_S16_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_CONVOLVE_1X1_S8_TEST)
obj-y += convolve_1x1_s8_test/beco_convolve_1x1_s8_test.o
ccflags-y += -DBECO_CONVOLVE_1X1_S8_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_CONVOLVE_1X1_S16_TEST)
obj-y += convolve_1x1_s16_test/beco_convolve_1x1_s16_test.o
ccflags-y += -DBECO_CONVOLVE_1X1_S16_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_DEPTHWISE_CONV_S8_TEST)
obj-y += depthwise_conv_s8_test/beco_depthwise_conv_nx3_s8_test.o
obj-y += depthwise_conv_s8_test/beco_transpose_depthwise_conv_s8_test.o
ccflags-y += -DBECO_DEPTHWISE_CONV_S8_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_DEPTHWISE_CONV_S16_TEST)
obj-y += depthwise_conv_s16_test/beco_depthwise_conv_nx3_s16_test.o
obj-y += depthwise_conv_s16_test/beco_transpose_depthwise_conv_s16_test.o
ccflags-y += -DBECO_DEPTHWISE_CONV_S16_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_AVGPOOL_S8_TEST)
obj-y += avgpool_s8_test/beco_avgpool_s8_test.o
ccflags-y += -DBECO_AVGPOOL_S8_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_AVGPOOL_S16_TEST)
obj-y += avgpool_s16_test/beco_avgpool_s16_test.o
ccflags-y += -DBECO_AVGPOOL_S16_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_MAX_POOL_S8_TEST)
obj-y += max_pool_s8_test/beco_max_pool_s8_test.o
ccflags-y += -DBECO_MAX_POOL_S8_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_MAX_POOL_S16_TEST)
obj-y += max_pool_s16_test/beco_max_pool_s16_test.o
ccflags-y += -DBECO_MAX_POOL_S16_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_GRU_S16_TEST)
obj-y += gru_s16_test/beco_gru_s16_pc_test.o
obj-y += gru_s16_test/beco_gru_s16_pt_test.o
obj-y += gru_s16_test/beco_bi_gru_s16_test.o
ccflags-y += -DBECO_GRU_S16_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_LSTM_S16_TEST)
obj-y += lstm_s16_test/beco_lstm_s16_pt_test.o
obj-y += lstm_s16_test/beco_lstm_s16_pc_test.o
obj-y += lstm_s16_test/beco_bi_lstm_s16_test.o
ccflags-y += -DBECO_LSTM_S16_TEST
endif

obj-y += beco_test.o
ccflags-y += -Iinclude/beco/ \
             -Iinclude/beco/beco_nn/ \
             -Iinclude/cmsis_nn \
             -Iutils/hwtimer_list/
