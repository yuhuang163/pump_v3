cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y :=
obj-y += ConvolutionFunctions/
obj-y += FullyConnectedFunctions/
obj-y += NNSupportFunctions/
obj-y += RNNFunctions/
obj-y += PoolingFunctions/

subdir-ccflags-y += -Iinclude/beco/beco_nn -Iinclude/cmsis_nn -Iinclude/cmsis_dsp

beco_nn_lib-y := $(obj-y)
obj-y := beco_nn_lib.a
