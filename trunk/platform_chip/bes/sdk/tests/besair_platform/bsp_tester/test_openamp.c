/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifdef TEST_OPENAMP
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_trace.h"
#ifdef OPENAMP_ENABLE
#include <rptun/chips/rptun_chips.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg-echo.h"

#include <metal/irq.h>
#include <rptun/rptun_openamp.h>

#define SHUTDOWN_MSG	0xEF56A55A

#ifdef CHIP_SUBSYS_SENS

static struct rpmsg_endpoint lept;
static int shutdown_req;

struct rpmsg_rcv_msg {
	void *data;
	size_t len;
	struct rpmsg_endpoint *ept;
	struct rpmsg_rcv_msg *next;
};

static struct rpmsg_rcv_msg *rpmsg_list;

/*-----------------------------------------------------------------------------
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------
 */
static atomic_int ipi_nokick;

int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	(void)src;
	(void)priv;
	struct rpmsg_rcv_msg *rpmsg_node, *p_msg;

	/* On reception of a shutdown we signal the application to terminate */
	if ((*(unsigned int *)(((char *)data)+8)) == SHUTDOWN_MSG) {
		TRACE(0,"shutdown message is received.\r\n");
		shutdown_req = 1;
		return RPMSG_SUCCESS;
	}

	rpmsg_node = metal_allocate_memory(sizeof(*rpmsg_node));
	if (!rpmsg_node) {
		TRACE(0,"rpmsg_node allocation failed\r\n");
		return -1;
	}
	rpmsg_hold_rx_buffer(ept, data);
	rpmsg_node->ept = ept;
	rpmsg_node->data = data;
	rpmsg_node->len = len;
	rpmsg_node->next = NULL;
	TRACE(0,"slave_receive_str = %s \n",&(((char *)rpmsg_node->data)[8]));

	if (!rpmsg_list)
		rpmsg_list = rpmsg_node;
	else {
		p_msg = rpmsg_list;
		while (p_msg->next)
			p_msg = p_msg->next;
		p_msg->next = rpmsg_node;
	}
	atomic_flag_clear(&ipi_nokick);

	return RPMSG_SUCCESS;
}

void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	TRACE(0,"unexpected Remote endpoint destroy\r\n");
	shutdown_req = 1;
	atomic_flag_clear(&ipi_nokick);
}

static void poll_ept_evt(void *priv)
{
	unsigned int flags;

	while (1) {
		flags = metal_irq_save_disable();
		if (!(atomic_flag_test_and_set(&ipi_nokick))) {
			metal_irq_restore_enable(flags);
			return;
		}
		asm volatile ("wfi");
		metal_irq_restore_enable(flags);
	}

}

/*-----------------------------------------------------------------------------
 *  Application
 *-----------------------------------------------------------------------------
 */
int app(void *priv)
{
	int ret;
	struct rpmsg_rcv_msg *rpmsg_node;
	/* struct rpmsg_device *rdev; */

	/* Initialize RPMSG framework */
	TRACE(0,"Waiting for rpmsg endpoint ready...\r\n");

	while (!is_rpmsg_ept_ready(&lept))
		hal_sys_timer_delay(MS_TO_TICKS(10));

	TRACE(0,"Successfully binding rpmsg endpoint.\r\n");

	while (1) {
		poll_ept_evt(priv);
		/* we got a shutdown request, exit */
		if (shutdown_req) {
			break;
		}
		uint32_t flags = metal_irq_save_disable();
		while (rpmsg_list) {
			/* Send data back to host */
			ret = rpmsg_send(rpmsg_list->ept, rpmsg_list->data,
					 rpmsg_list->len);
			if (ret < 0) {
				TRACE(0,"rpmsg_send failed\r\n");
				return ret;
			}
			rpmsg_release_rx_buffer(rpmsg_list->ept,
						rpmsg_list->data);

			rpmsg_node = rpmsg_list;
			rpmsg_list = rpmsg_list->next;

			metal_free_memory(rpmsg_node);
		}
		metal_irq_restore_enable(flags);
	}
	rpmsg_destroy_ept(&lept);

	return 0;
}

struct echo_dev {
	char name[RPMSG_NAME_SIZE];
};

void rpmsg_echo_device_created(struct rpmsg_device *rdev, void *priv)
{
	int ret;
	TRACE(0,"%s annouce the serice", __func__);
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_service_unbind);
	if (ret) {
		TRACE(0,"Failed to create endpoint.\r\n");
		return;
	}
	//remoteproc
	lept.priv = priv;
	return;
}

void rpmsg_echo_device_destroy(struct rpmsg_device *rdev, void *priv)
{
	TRACE(0,"%s", __func__);
}

bool rpmsg_echo_device_ns_match(struct rpmsg_device *rdev,
				       void *priv,
				       const char *name, uint32_t dest)
{
	struct echo_dev *dev = priv;

	TRACE(0,"echo ns match %s dev name %s dest %d", name, dev->name, dest);
	return !strcmp(name, dev->name);
}

void rpmsg_echo_device_ns_bind(struct rpmsg_device *rdev,
				      void *priv,
				      const char *name, uint32_t dest)
{
	TRACE(0,"echo ns bind %s %d", name, dest);
}

/*-----------------------------------------------------------------------------
 *  Application entry point
 *-----------------------------------------------------------------------------
 */

int rpmsg_echo_bth2sens_slave_test(int argc, char *argv[])
{
	static struct echo_dev echo_dev;

	atomic_init(&ipi_nokick, 1);
	rpmsg_list = NULL;

	strncpy(&echo_dev.name[0], RPMSG_SERVICE_NAME, sizeof(echo_dev.name));

	TRACE(0,"Starting application...\r\n");

	rpmsg_register_callback(&echo_dev,
				rpmsg_echo_device_created,
				rpmsg_echo_device_destroy,
				rpmsg_echo_device_ns_match,
				rpmsg_echo_device_ns_bind);
	app(&echo_dev);

	TRACE(0,"Stopping application...\r\n");

	return 0;
}
#endif


#ifdef __SYS_AS_MAIN__
struct _payload {
	unsigned long num;
	unsigned long size;
	unsigned char data[];
};

struct rpmsg_rcv_msg {
	struct _payload *payload;
	size_t len;
	struct rpmsg_endpoint *ept;
};

struct ping_dev {
	char name[RPMSG_NAME_SIZE];
};

static int err_cnt;
static struct rpmsg_rcv_msg rcv_msg;

/* Globals */
static struct rpmsg_endpoint lept;
static int rnum;
static int err_cnt;
static int ept_deleted;
char *str_ = "hello world!";
#define SHUTDOWN_MSG	0xEF56A55A
static int rpmsg_check_rcv_msg(struct rpmsg_rcv_msg *rcv_msg, uint32_t exp_num)
{
	int i, ret = RPMSG_SUCCESS;
	struct _payload *r_payload = rcv_msg->payload;

	if (r_payload->num != exp_num) {
		TRACE(0,"Invalid message number received %ld, expected %d\r\n",
			r_payload->num, exp_num);
		ret = RPMSG_ERR_PARAM;
		goto out;
	}

	if (r_payload->size == 0) {
		TRACE(0," Invalid size of package is received.\r\n");
		ret = RPMSG_ERR_PARAM;
		goto out;
	}

	/* Validate data buffer integrity. */
	for (i = 0; i < (int)r_payload->size; i++) {
		if (r_payload->data[i] != str_[i]) {
			TRACE(0,"Data corruption at index %d\r\n", i);
			ret = RPMSG_ERR_PARAM;
			goto out;
		}
	}
out:
	rpmsg_release_rx_buffer(rcv_msg->ept, r_payload);
	return ret;
}

static atomic_int ipi_nokick;

static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	(void)src;
	(void)priv;

	rpmsg_hold_rx_buffer(ept, data);
	rcv_msg.ept = ept;
	rcv_msg.payload = (struct _payload *)data;
	rcv_msg.len = len;

	TRACE(0,"master_receive_str = %s \n",rcv_msg.payload->data);
	rnum = rcv_msg.payload->num + 1;
	atomic_flag_clear(&ipi_nokick);
	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	rpmsg_destroy_ept(&lept);
	TRACE(0,"echo test: service is destroyed\r\n");
	ept_deleted = 1;
}

static void poll_ept_evt(void *priv)
{
	/* struct remoteproc *rproc = priv; */
	unsigned int flags;

	while (1) {
		flags = metal_irq_save_disable();
		if (!(atomic_flag_test_and_set(&ipi_nokick))) {
			metal_irq_restore_enable(flags);
			return;
		}
		asm volatile ("wfi");
		metal_irq_restore_enable(flags);
	}

}

static int app(void *priv)
{
	int ret;
	int i, num_payloads;
	uint32_t size = strlen(str_), max_size;
	int expect_rnum = -1;
	struct rpmsg_device *rdev;

	while (!is_rpmsg_ept_ready(&lept))
        hal_sys_timer_delay(MS_TO_TICKS(10));

	rdev = lept.rdev;
	TRACE(0," 1 - Send data to remote core, retrieve the echo");
	TRACE(0," and validate its integrity ..\r\n");

	max_size = rpmsg_virtio_get_buffer_size(rdev);
	if ((int32_t) max_size < 0) {
		TRACE(0,"No available buffer size.\r\n");
		return -1;
	}
	max_size -= sizeof(struct _payload);
	num_payloads = 10;
	TRACE(0,"RPMSG endpoint is binded with remote.\r\n");

	for (i = 0; i < num_payloads; i++) {
		struct _payload *i_payload;

		i_payload = rpmsg_get_tx_payload_buffer(&lept, &max_size, 1);
		if (!i_payload) {
			TRACE(0,"Failed to get payload...\r\n");
			break;
		}

		i_payload->num = i;
		i_payload->size = size;
		/* Mark the data buffer. */
		for(int j = 0; j < size; j++){
			i_payload->data[j] = str_[j];
		}

		TRACE(0,"master_send_str = %s \n",i_payload->data);

		metal_sleep_usec(100 * 1000);
		ret = rpmsg_send_nocopy(&lept, i_payload,
					(2 * sizeof(unsigned long)) + size);

		if (ret < 0) {
			TRACE(0,"Failed to send data...\r\n");
			break;
		}

		expect_rnum++;
		do {
			/* platform_poll(priv); */
			poll_ept_evt(priv);
		} while ((rnum < expect_rnum) && !ept_deleted);

		if (ept_deleted)
			break;
		if (rpmsg_check_rcv_msg(&rcv_msg, expect_rnum))
			err_cnt++;
	}

	TRACE(0,"**********************************\r\n");
	TRACE(0," Test Results: Error count = %d \r\n", err_cnt);
	TRACE(0,"**********************************\r\n");
	/* Destroy the RPMsg endpoint */
	// rpmsg_destroy_ept(&lept);

	return 0;
}

static bool rpmsg_ping_device_ns_match(struct rpmsg_device *rdev,
				       void *priv,
				       const char *name, uint32_t dest)
{
	struct ping_dev *dev = priv;

	TRACE(0,"ns match %s dev name %s dest %d", name, dev->name, dest);
	return !strcmp(name, dev->name);
}

static void rpmsg_ping_device_ns_bind(struct rpmsg_device *rdev,
				      void *priv,
				      const char *name, uint32_t dest)
{
	int ret;
	TRACE(0,"ns bind %s %d", name, dest);
	/* Create RPMsg endpoint */
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
			RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			rpmsg_endpoint_cb, rpmsg_service_unbind);

	if (ret) {
		TRACE(0,"Failed to create RPMsg endpoint.\r\n");
		return;
	}
}


static int rpmsg_echo_bth2sens_master_test(void)
{
	static struct ping_dev ping_dev;

	atomic_init(&ipi_nokick, 1);

	strncpy(&ping_dev.name[0], RPMSG_SERVICE_NAME, sizeof(ping_dev.name));
	TRACE(0,"Starting application... %s", __func__);

	rpmsg_register_callback(&ping_dev,
				NULL,
				NULL,
				rpmsg_ping_device_ns_match,
				rpmsg_ping_device_ns_bind);

	app(&ping_dev);

	TRACE(0,"Stopping application %s ...\r\n", __func__);

	return 0;
}
#endif




void test_openamp(void)
{
	#ifdef __SYS_AS_MAIN__
	TRACE(0,"Starting test_openamp for m55 to sensor_hub...\n");
	while(1){
		rpmsg_echo_bth2sens_master_test();
		osSignalWait(0x0, 5000);
	}
	#endif

	#ifdef CHIP_SUBSYS_SENS
	TRACE(0,"Starting test_openamp for sensor_hub to m55...\n");
	rpmsg_echo_bth2sens_slave_test(0, NULL);
	#endif
}

#endif


