
obj-y :=
obj-y += source/FreeRTOS_POSIX_clock.o
obj-y += source/FreeRTOS_POSIX_mqueue.o
obj-y += source/FreeRTOS_POSIX_pthread_barrier.o
obj-y += source/FreeRTOS_POSIX_pthread_cond.o
obj-y += source/FreeRTOS_POSIX_pthread_mutex.o
obj-y += source/FreeRTOS_POSIX_pthread.o
obj-y += source/FreeRTOS_POSIX_sched.o
obj-y += source/FreeRTOS_POSIX_semaphore.o
obj-y += source/FreeRTOS_POSIX_timer.o
obj-y += source/FreeRTOS_POSIX_unistd.o
obj-y += source/FreeRTOS_POSIX_utils.o

ccflags-y :=
ccflags-y += -Irtos/freertos/POSIX/include/FreeRTOS-Plus-POSIX/
ccflags-y += -Irtos/freertos/POSIX/include/private/
ccflags-y += -Iinclude/rtos/freertos/

OS_HAVE_POSIX_TESTS ?= 0
ifeq ($(OS_HAVE_POSIX_TESTS),1)
obj-y += tests/iot_test_posix_clock.o
obj-y += tests/iot_test_posix_mqueue.o
obj-y += tests/iot_test_posix_pthread.o
obj-y += tests/iot_test_posix_semaphore.o
obj-y += tests/iot_test_posix_stress.o
obj-y += tests/iot_test_posix_timer.o
obj-y += tests/iot_test_posix_unistd.o
obj-y += tests/iot_test_posix_utils.o
obj-y += tests/aws_test_runner.o

obj-y += ../../../services/aiot/unity/
ccflags-y += -Iservices/aiot/unity/extras/fixture/src/
ccflags-y += -Iservices/aiot/unity/src/
endif