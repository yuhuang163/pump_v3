/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"

/*
* DEFINES
*****************************************************************************************
*/
#define AUDIO_TIME_PERIOD                         (100) 
#define AUDIO_SEND_TIMEOUT                        (8) 
#define IS_SUPPORT_AUDIO_TIMEOUT                  (0) 

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static rt_device_node_t *node = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int audio_dev_open(struct rt_device_node *dev)
{
    return 0;
}

static int audio_dev_close(struct rt_device_node *dev)
{
    return 0;
}

static int audio_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    int *val = (int *)buf;
    *val = 0;
    return *val;
}

static int audio_dev_write(struct rt_device_node *dev, const void *buf, uint32_t n_bytes)
{
    return -1;
}

static int audio_dev_ioctl(struct rt_device_node *dev, uint32_t op_code, void *args)
{
    // uint32_t *tmp = (uint32_t *)args;
    switch (op_code) {
        default:
            LOGE("err op_code: %lx", op_code);
            return -1;
    }
}

static int audio_dev_suspend(struct rt_device_node *dev)
{
    return 0;
}

static int audio_dev_resume(struct rt_device_node *dev)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_audio(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));

    node->name = "audio";
    node->id = ID_DEVICE_AUDIO;
    node->open = audio_dev_open;
    node->close = audio_dev_close;
    node->suspend = audio_dev_suspend;
    node->resume = audio_dev_resume;
    node->read = audio_dev_read;
    node->write = audio_dev_write;
    node->ioctl = audio_dev_ioctl;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}
