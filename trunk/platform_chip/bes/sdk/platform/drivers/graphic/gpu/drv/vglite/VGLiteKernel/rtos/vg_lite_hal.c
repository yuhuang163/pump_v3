/****************************************************************************
*
*    Copyright 2012 - 2022 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
****************************************************************************/


#include "vg_lite_platform.h"
#include "vg_lite_kernel.h"
#include "vg_lite_hal.h"
#include "vg_lite_hw.h"
#include "gpu_port.h"
#include "hal_timer.h"
#include "heap_api.h"


static uint32_t registerMemBase = GPU_BASE;

/* If bit31 is activated this indicates a bus error */
#define IS_AXI_BUS_ERR(x) ((x)&(1U << 31))
#define HEAP_NODE_USED  0xABBAF00D
#define DUMMY_HANDLE    0xDEADABBA

volatile void* contiguousMem[VG_SYSTEM_RESERVE_COUNT] = {
    [0 ... VG_SYSTEM_RESERVE_COUNT - 1] = NULL
};

uint32_t gpuMemBase[VG_SYSTEM_RESERVE_COUNT] = {
    [0 ... VG_SYSTEM_RESERVE_COUNT - 1] = 0
};

/* Default heap size is 16MB. */
static uint32_t heap_size[VG_SYSTEM_RESERVE_COUNT] = {
    [0 ... VG_SYSTEM_RESERVE_COUNT - 1] = MAX_CONTIGUOUS_SIZE
};

void __attribute__((weak)) vg_lite_bus_error_handler();

void vg_lite_init_mem(vg_module_parameters_t *param)
{
    uint32_t i;

    registerMemBase = param->register_mem_base;

    for (i = 0; i< VG_SYSTEM_RESERVE_COUNT; i++) {
        gpuMemBase[i]      = param->gpu_mem_base[i];
        contiguousMem[i]   = param->contiguous_mem_base[i];
        heap_size[i]       = param->contiguous_mem_size[i];
    }
}

/* Implementation of list. ****************************************/
#define INIT_LIST_HEAD(entry) \
        (entry)->next = (entry);\
        (entry)->prev = (entry);

/* Add the list item in front of "head". */
static inline void add_list(list_head_t *to_add, list_head_t *head)
{
    /* Link the new item. */
    to_add->next = head;
    to_add->prev = head->prev;

  /* Modify the neighbor. */
    head->prev = to_add;
    if (to_add->prev != NULL) {
        to_add->prev->next = to_add;
    }
}

/* Remove an entry out of the list. */
static inline void delete_list(list_head_t *entry)
{
    if (entry->prev != NULL) {
        entry->prev->next = entry->next;
    }
    if (entry->next != NULL) {
        entry->next->prev = entry->prev;
    }
}

/* End of list implementation. ***********/
static inline void _memset(void *mem, unsigned char value, int size)
{
    int i;
    for (i = 0; i < size; i++) {
        ((unsigned char *)mem)[i] = value;
    }
}

struct memory_heap {
    uint32_t free;
    list_head_t list;
};

struct mapped_memory {
    void * logical;
    uint32_t physical;
    int page_count;
    struct page ** pages;
};

struct vg_lite_device {
    /* void * gpu; */
    uint32_t register_base;    /* Always use physical for register access in RTOS. */
    /* struct page * pages; */
    volatile void *contiguous[VG_SYSTEM_RESERVE_COUNT];
    unsigned int order;
    unsigned int heap_size[VG_SYSTEM_RESERVE_COUNT];
    void *virtual[VG_SYSTEM_RESERVE_COUNT];
    uint32_t physical[VG_SYSTEM_RESERVE_COUNT];
    uint32_t size[VG_SYSTEM_RESERVE_COUNT];
    struct memory_heap heap[VG_SYSTEM_RESERVE_COUNT];
    int irq_enabled;
    volatile uint32_t int_flags;
    void * device;
    int registered;
    int major;
    struct class * class;
    int created;
    vg_lite_gpu_execute_state_t gpu_execute_state;
};

struct client_data {
    struct vg_lite_device * device;
    struct vm_area_struct * vm;
    void * contiguous_mapped;
};

static struct vg_lite_device Device, * device;

void vg_lite_set_gpu_execute_state(vg_lite_gpu_execute_state_t state)
{
    device->gpu_execute_state = state;
}

void *vg_lite_hal_alloc(unsigned long size)
{
    return gpu_hal_malloc(size);
}

void vg_lite_hal_free(void * memory)
{
    gpu_hal_free(memory);
}

void vg_lite_hal_delay(uint32_t ms)
{
#if defined(__NuttX__)
    usleep(1000*ms);
#else
    hal_sys_timer_delay(MS_TO_TICKS(ms));
#endif
}

void vg_lite_hal_barrier(void)
{
    /*Memory barrier. */
    __asm("DSB");
    gpu_hal_cache_sync();
}

static int vg_lite_init(void);
void vg_lite_hal_initialize(void)
{
    /* Enable GPU clock & IRQ, prepare buffers. */
    gpu_hal_init();

    vg_lite_init();
}

void vg_lite_hal_deinitialize(void)
{
    /* Disable clock & IRQ */
    gpu_hal_exit();
}

void vg_lite_hal_reset(void)
{
    gpu_hal_reset();
}

static int split_node(heap_node_t * node, unsigned long size)
{
    /* TODO: the original is linux specific list based, needs rewrite.
    */
    heap_node_t * split;

    /* Allocate a new node. */
    split = vg_lite_hal_alloc(sizeof(heap_node_t));
    if (split == NULL)
        return -1;

    /* Fill in the data of this node of the remaning size. */
    split->offset = node->offset + size;
    split->size = node->size - size;
    split->status = 0;

    /* Add the new node behind the current node. */
    add_list(&split->list, &node->list);

    /* Adjust the size of the current node. */
    node->size = size;
    return 0;
}

vg_lite_error_t vg_lite_hal_allocate_contiguous(unsigned long size, vg_lite_vidmem_pool_t pool, void ** logical, void ** klogical, uint32_t * physical, void ** node)
{
    unsigned long aligned_size;
    heap_node_t *pos;

    /* Judge if it exceeds the range of pool */
    if (pool >= VG_SYSTEM_RESERVE_COUNT)
        pool = VG_SYSTEM_RESERVE_COUNT - 1;

    /* Align the size to 64 bytes. */
    aligned_size = (size + 63) & ~63;

    /* Check if there is enough free memory available. */
    if (aligned_size > device->heap[pool].free) {
        return VG_LITE_OUT_OF_MEMORY;
    }

    /* Walk the heap backwards. */
    for (pos = (heap_node_t *)device->heap[pool].list.prev;
                 &pos->list != &device->heap[pool].list;
                 pos = (heap_node_t*) pos->list.prev) {
        /* Check if the current node is free and is big enough. */
        if (pos->status == 0 && pos->size >= aligned_size) {
            /* See if we the current node is big enough to split. */
                if (0 != split_node(pos, aligned_size))
                {
                    return VG_LITE_OUT_OF_RESOURCES;
                }
            /* Mark the current node as used. */
            pos->status = 0xABBAF00D;

            /*  Return the logical/physical address. */
            /* *logical = (uint8_t *) private_data->contiguous_mapped + pos->offset; */
            *logical = (uint8_t *)device->virtual[pool] + pos->offset;
            *klogical = *logical;
            *physical = ADDR_CPU_TO_DEV((uint32_t)(*logical));
            /* Mark which pool the pos belong to */
            pos->pool = pool;

            device->heap[pool].free -= aligned_size;

            *node = pos;
            return VG_LITE_SUCCESS;
        }
    }

    /* Out of memory. */
    return VG_LITE_OUT_OF_MEMORY;
}

void vg_lite_hal_free_contiguous(void * memory_handle)
{
    /* TODO: no list available in RTOS. */
    heap_node_t *pos, *node;
    vg_lite_vidmem_pool_t pool;

    /* Get pointer to node. */
    node = memory_handle;

    if (node->status != 0xABBAF00D) {
        return;
    }

    /* Determine which pool the node belongs to */
    pool = node->pool;

    /* Mark node as free. */
    node->status = 0;

    /* Add node size to free count. */
    device->heap[pool].free += node->size;

    /* Check if next node is free. */
    pos = node;
    for (pos = (heap_node_t *)pos->list.next;
         &pos->list != &device->heap[pool].list;
         pos = (heap_node_t *)pos->list.next) {
        if (pos->status == 0) {
            /* Merge the nodes. */
            node->size += pos->size;
            if (node->offset > pos->offset)
                node->offset = pos->offset;
            /* Delete the next node from the list. */
            delete_list(&pos->list);
            vg_lite_hal_free(pos);
        }
        break;
    }

    /* Check if the previous node is free. */
    pos = node;
    for (pos = (heap_node_t *)pos->list.prev;
         &pos->list != &device->heap[pool].list;
         pos = (heap_node_t *)pos->list.prev) {
        if (pos->status == 0) {
            /* Merge the nodes. */
            pos->size += node->size;
            if (pos->offset > node->offset)
                pos->offset = node->offset;
            /* Delete the current node from the list. */
            delete_list(&node->list);
            vg_lite_hal_free(node);
        }
        break;
    }

    /* when release command buffer node and ts buffer node to exit,release the linked list*/
    /* if(device->heap[pool].list.next == device->heap[pool].list.prev) {
        delete_list(&pos->list);
        vg_lite_hal_free(pos);
    }*/
}

void vg_lite_hal_free_os_heap(void)
{
    struct heap_node *pos, *n;
    uint32_t i;

    /* Check for valid device. */
    if (device != NULL) {
        /* Process each node. */
        for (i = 0; i < VG_SYSTEM_RESERVE_COUNT; i++) {
            for (pos = (heap_node_t *)device->heap[i].list.next,
                 n = (heap_node_t *)pos->list.next;
                 &pos->list != &device->heap[i].list;
                 pos = n, n = (heap_node_t *)n->list.next) {
                /* Remove it from the linked list. */
                delete_list(&pos->list);
                /* Free up the memory. */
                vg_lite_hal_free(pos);
            }
        }
    }
}

/* Portable: read register value. */
uint32_t vg_lite_hal_peek(uint32_t address)
{
    /* Read data from the GPU register. */
    return (uint32_t) (*(volatile uint32_t *) (device->register_base + address));
}

/* Portable: write register. */
void vg_lite_hal_poke(uint32_t address, uint32_t data)
{
    /* Write data to the GPU register. */
    volatile uint32_t *LocalAddr = (uint32_t *)(device->register_base + address);
    *LocalAddr = data;
}

vg_lite_error_t vg_lite_hal_query_mem(vg_lite_kernel_mem_t *mem)
{
    if(device != NULL){
        mem->bytes  = device->heap[mem->pool].free;
        return VG_LITE_SUCCESS;
    }
    mem->bytes = 0;
    return VG_LITE_NO_CONTEXT;
}

vg_lite_error_t vg_lite_hal_map_memory(vg_lite_kernel_map_memory_t *node)
{
    node->logical = (void *)node->physical;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_hal_unmap_memory(vg_lite_kernel_unmap_memory_t *node)
{
    return VG_LITE_SUCCESS;
}

void __attribute__((weak)) vg_lite_bus_error_handler()
{
    /*
     * Default implementation of the bus error handler does nothing. Application
     * should override this handler if it requires to be notified when a bus
     * error event occurs.
     */
     return;
}

void vg_lite_IRQHandler(void)
{
}

int32_t vg_lite_hal_wait_interrupt(uint32_t timeout, uint32_t mask, uint32_t * value)
{
    if (gpu_hal_wait_done() == -1)
        return 0;

    if (IS_AXI_BUS_ERR(*value)) {
        vg_lite_bus_error_handler();
    }
    return 1;
}

vg_lite_error_t vg_lite_hal_memory_export(int32_t *fd)
{
    return VG_LITE_SUCCESS;
}

void * vg_lite_hal_map(unsigned long bytes, void * logical, uint32_t physical, uint32_t * gpu)
{

    (void) bytes;
    (void) logical;
    (void) physical;
    (void) gpu;
    // return (void *)0;
    *gpu = physical;
    return (void *)DUMMY_HANDLE;
}

void vg_lite_hal_unmap(void * handle)
{

    (void) handle;
}

vg_lite_error_t vg_lite_hal_operation_cache(void *handle, vg_lite_cache_op_t cache_op)
{
    (void) handle;
    (void) cache_op;

    return VG_LITE_SUCCESS;
}

static void vg_lite_exit(void)
{
    heap_node_t *pos;
    heap_node_t *n;
    uint32_t i;

    /* Check for valid device. */
    if (device != NULL) {
        /* TODO: unmap register mem should be unnecessary. */
        device->register_base = 0;

        for (i = 0; i < VG_SYSTEM_RESERVE_COUNT; i++) {
            /* Process each node. */
            for (pos = (heap_node_t *)device->heap[i].list.next, n = (heap_node_t *)pos->list.next;
                 &pos->list != &device->heap[i].list;
                 pos = n, n = (heap_node_t *)n->list.next) {
                /* Remove it from the linked list. */
                delete_list(&pos->list);

                /* Free up the memory. */
                vg_lite_hal_free(pos);
            }
        }

        /* Free up the device structure. */
        vg_lite_hal_free(device);
    }
}

static int vg_lite_init(void)
{
    heap_node_t *node;
    uint32_t i;

    /* Initialize memory and objects ***************************************/
    /* Create device structure. */
    device = &Device;

    /* Zero out the enture structure. */
    _memset(device, 0, sizeof(struct vg_lite_device));

    /* Setup register memory. **********************************************/
    device->register_base = registerMemBase;

    /* Initialize contiguous memory. ***************************************/
    /* Allocate the contiguous memory. */
    for (i = 0; i < VG_SYSTEM_RESERVE_COUNT; i++) {
        device->heap_size[i] = heap_size[i];
        device->contiguous[i] = (volatile void *)contiguousMem[i];
        _memset((void *)device->contiguous[i], 0, heap_size[i]);
        /* Make 64byte aligned. */
        while ((((uint32_t)device->contiguous[i]) & 63) != 0)
        {
            device->contiguous[i] = ((unsigned char *)device->contiguous[i]) + 4;
            device->heap_size[i] -= 4;
        }

        /* Check if we allocated any contiguous memory or not. */
        if (device->contiguous[i] == NULL) {
            vg_lite_exit();
            return -1;
        }


        device->virtual[i] = (void *)device->contiguous[i];
        device->physical[i] = ADDR_CPU_TO_DEV((uint32_t)device->virtual[i]);
        device->size[i] = device->heap_size[i];

        /* Create the heap. */
        INIT_LIST_HEAD(&device->heap[i].list);
        device->heap[i].free = device->size[i];

        node = vg_lite_hal_alloc(sizeof(heap_node_t));
        if (node == NULL) {
            vg_lite_exit();
            return -1;
        }
        node->offset = 0;
        node->size = device->size[i];
        node->status = 0;
        add_list(&node->list, &device->heap[i].list);
    }

    device->int_flags = 0;
    /* Success. */
    return 0;
}
