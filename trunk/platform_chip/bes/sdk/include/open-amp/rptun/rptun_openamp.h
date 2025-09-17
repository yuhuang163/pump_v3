/****************************************************************************
 * include/nuttx/rptun/openamp.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __RPTUN_OPENAMP_H__
#define __RPTUN_OPENAMP_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <openamp/open_amp.h>
#include <openamp/remoteproc_loader.h>

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef void (*rpmsg_dev_cb_t)(struct rpmsg_device * rdev, void *priv);
typedef bool (*rpmsg_match_cb_t)(struct rpmsg_device * rdev,
                                 void *priv, const char *name, uint32_t dest);
typedef void (*rpmsg_bind_cb_t)(struct rpmsg_device * rdev,
                                void *priv, const char *name, uint32_t dest);

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* int rpmsg_wait(struct rpmsg_endpoint *ept, sem_t *sem); */
/* int rpmsg_post(struct rpmsg_endpoint *ept, sem_t *sem); */

const char *rpmsg_get_cpuname(struct rpmsg_device *rdev);

int rpmsg_register_callback(void *priv,
                                rpmsg_dev_cb_t device_created,
                                rpmsg_dev_cb_t device_destroy,
                                rpmsg_match_cb_t ns_match, rpmsg_bind_cb_t ns_bind);

void rpmsg_unregister_callback(void *priv,
                                   rpmsg_dev_cb_t device_created,
                                   rpmsg_dev_cb_t device_destroy,
                                   rpmsg_match_cb_t ns_match, rpmsg_bind_cb_t ns_bind);

#ifdef __cplusplus
}
#endif
#endif                          /* __RPTUN_OPENAMP_H__ */
