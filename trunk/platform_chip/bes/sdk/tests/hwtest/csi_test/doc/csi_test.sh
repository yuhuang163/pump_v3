#!/bin/bash
#
# Introduction
#
# This is a simple shell scripts based on bash for CSI test.
# The CSI is Camera Serial Interface and it defines an interface between a peripheral device(camera)
# and a host processor(baseband, application engine). For BES's Soc, some chip provides the CSI hardware
# controller. More details can be found in Specification for Camera Serial Interface 2.
#
#
#
# Compiling Commands
#
# For BES AOS platform ,the noapp_test target supports CSI test. The compiling command are shown as below;
#
#

noapp_test_csi_dsi_loop_test_gx6d()
{
	cmd="T=noapp_test CHIP=best2003 -j8 TRACE_BAUD_RATE=1500000 DEBUG_PORT=2 \
		NO_SLEEP=1 MCU_I2C_SLAVE=1 UART1_IOMUX_INDEX=32 \
		PSRAMUHS_ENABLE=1 PSRAMUHS_AP325608A=1 RF_TX_CONTROL_IO_PARAM=100 \
		ALT_BOOT_FLASH=1 GX6D=1 TRACE_BUF_SIZE=32*1024 \
		CSI_DSI_CONFIG=1 CSI_DSI_LOOP=1 DISPLAY_TEST_ONLY=0 CAMERA_TEST_ONLY=0 \
		CAMERA_USE_SOFT_ISP=0 \
		CSI_PHY_NO_CALIB=0"

	make ${cmd}
	echo "make ${cmd}"
}

noapp_test_csi_dsi_loop_test_ax4f()
{
	cmd="T=noapp_test CHIP=best2003 -j8 TRACE_BAUD_RATE=1500000 DEBUG_PORT=2 \
		NO_SLEEP=1 MCU_I2C_SLAVE=1 UART1_IOMUX_INDEX=32 \
		PSRAMUHS_ENABLE=1 PSRAMUHS_AP325608A=1 RF_TX_CONTROL_IO_PARAM=100 \
		ALT_BOOT_FLASH=1 AX4F=1 TRACE_BUF_SIZE=32*1024 \
		CSI_DSI_CONFIG=1 CSI_DSI_LOOP=1 DISPLAY_TEST_ONLY=0 CAMERA_TEST_ONLY=0 \
		CAMERA_USE_SOFT_ISP=0 \
		CSI_PHY_NO_CALIB=0"

	make ${cmd}
	echo "make ${cmd}"
}

# CSI_DSI_CONFIG
# set 1 to enable CSI and DSI configuration, target.mk will includes csi_dsi_cfg.mk for CSI test.
#
# CSI_DSI_LOOP
# set 1 to enable CSI+DSI loop test, display device shows the camera image.
#
# DISPLAY_TEST_ONLY
# set 1 to enable DSI test only, the CSI test is bypassed.
#
# CAMERA_TEST_ONLY
# set 1 to enable CSI test only, the DSI test is bypassed.
#
# CAMERA_USE_SOFT_ISP
# set 1 to enable SOFT ISP functoin, set 0 to disable it.
#
# CAMERA_DATA_RAW10_TO_YUV
# set 1 to enable RAW10 to YUV conversion for SC132GS camera device, set 0 to disable it.
#
#
#
# this shell function is CSI DSI loop test base on GX6D chip.
#noapp_test_csi_dsi_loop_test_gx6d

# this shell function is CSI DSI loop test base on AX4F chip.
noapp_test_csi_dsi_loop_test_ax4f





