/*******************************************************************************
 * Copyright (c) Shenzhen Root Innovation
 * File name: aglo_ring_buffer.c
 * Description: 轻量级通用环形缓冲区管理库实现（基于LwRB设计理念）
 * Author: Auto-generated
 * Date: 2026-01-07
 ******************************************************************************/

#include "aglo_ring_buffer.h"
#include <string.h>

/*******************************************************************************
 * Local helper functions
 ******************************************************************************/

/**
 * @brief 计算从写入位置开始的连续可用空间（字节）
 * 注意：这是连续空间，可能不是全部可用空间
 */
static unsigned int get_linear_free_space(const aglo_ring_buffer_t *rb)
{
    if (rb->in >= rb->out) {
        // 写入位置在读取位置之后或相等
        // 如果in==out且count==0，说明缓冲区完全为空
        if (rb->in == rb->out && rb->count == 0) {
            return rb->size;  // 整个缓冲区都可用
        }
        // 否则返回从in到size的连续空间
        return rb->size - rb->in;
    } else {
        // 写入位置在读取位置之前（已绕一圈）
        // 连续空间从in到out
        return rb->out - rb->in;
    }
}

/**
 * @brief 计算从读取位置到缓冲区末尾的连续数据长度（字节）
 */
static unsigned int get_linear_read_size(const aglo_ring_buffer_t *rb)
{
    if (rb->out >= rb->in) {
        // 读取位置在写入位置之后或相等，连续数据在末尾
        return rb->size - rb->out;
    } else {
        // 读取位置在写入位置之前，连续数据从out到in
        return rb->in - rb->out;
    }
}

/*******************************************************************************
 * Public functions
 ******************************************************************************/

int aglo_ring_buffer_init(aglo_ring_buffer_t *rb, void *buff, unsigned int size, unsigned int element_size)
{
    if (rb == NULL || buff == NULL || size == 0 || element_size == 0) {
        return -1;
    }
    
    // 确保缓冲区大小是元素大小的整数倍
    if (size % element_size != 0) {
        size = (size / element_size) * element_size;
    }
    
    rb->buff = (uint8_t *)buff;
    rb->size = size;
    rb->element_size = element_size;
    rb->in = 0;
    rb->out = 0;
    rb->count = 0;
    
    return 0;
}

void aglo_ring_buffer_reset(aglo_ring_buffer_t *rb)
{
    if (rb != NULL) {
        rb->in = 0;
        rb->out = 0;
        rb->count = 0;
    }
}

unsigned int aglo_ring_buffer_get_count(const aglo_ring_buffer_t *rb)
{
    if (rb == NULL) {
        return 0;
    }
    return rb->count;
}

unsigned int aglo_ring_buffer_get_free(const aglo_ring_buffer_t *rb)
{
    if (rb == NULL) {
        return 0;
    }
    unsigned int max_elements = rb->size / rb->element_size;
    return max_elements - rb->count;
}

int aglo_ring_buffer_is_empty(const aglo_ring_buffer_t *rb)
{
    if (rb == NULL) {
        return 1;
    }
    return (rb->count == 0) ? 1 : 0;
}

int aglo_ring_buffer_is_full(const aglo_ring_buffer_t *rb)
{
    if (rb == NULL) {
        return 1;
    }
    unsigned int max_elements = rb->size / rb->element_size;
    return (rb->count >= max_elements) ? 1 : 0;
}

unsigned int aglo_ring_buffer_write(aglo_ring_buffer_t *rb, const void *data)
{
    if (rb == NULL || data == NULL) {
        return 0;
    }
    
    if (aglo_ring_buffer_is_full(rb)) {
        return 0;
    }
    
    // 计算写入位置
    uint8_t *write_ptr = rb->buff + rb->in;
    memcpy(write_ptr, data, rb->element_size);
    
    // 更新写入位置和计数
    rb->in += rb->element_size;
    if (rb->in >= rb->size) {
        rb->in = 0;
    }
    rb->count++;
    
    return 1;
}

unsigned int aglo_ring_buffer_write_multiple(aglo_ring_buffer_t *rb, const void *data, unsigned int count)
{
    if (rb == NULL || data == NULL || count == 0) {
        return 0;
    }
    
    unsigned int free_count = aglo_ring_buffer_get_free(rb);
    if (free_count == 0) {
        return 0;
    }
    
    // 限制写入数量不超过可用空间
    if (count > free_count) {
        count = free_count;
    }
    
    const uint8_t *src = (const uint8_t *)data;
    unsigned int written = 0;
    
    while (written < count) {
        // 计算当前可连续写入的元素个数
        unsigned int linear_free = get_linear_free_space(rb);
        unsigned int elements_to_write = linear_free / rb->element_size;
        
        // 限制本次写入数量
        if (elements_to_write > (count - written)) {
            elements_to_write = count - written;
        }
        
        if (elements_to_write == 0) {
            break;
        }
        
        // 写入数据
        unsigned int bytes_to_write = elements_to_write * rb->element_size;
        uint8_t *write_ptr = rb->buff + rb->in;
        memcpy(write_ptr, src + (written * rb->element_size), bytes_to_write);
        
        // 更新位置和计数
        rb->in += bytes_to_write;
        if (rb->in >= rb->size) {
            rb->in = 0;
        }
        rb->count += elements_to_write;
        written += elements_to_write;
    }
    
    return written;
}

unsigned int aglo_ring_buffer_read(aglo_ring_buffer_t *rb, void *data)
{
    if (rb == NULL || data == NULL) {
        return 0;
    }
    
    if (aglo_ring_buffer_is_empty(rb)) {
        return 0;
    }
    
    // 计算读取位置
    uint8_t *read_ptr = rb->buff + rb->out;
    memcpy(data, read_ptr, rb->element_size);
    
    // 更新读取位置和计数
    rb->out += rb->element_size;
    if (rb->out >= rb->size) {
        rb->out = 0;
    }
    rb->count--;
    
    return 1;
}

unsigned int aglo_ring_buffer_read_multiple(aglo_ring_buffer_t *rb, void *data, unsigned int count)
{
    if (rb == NULL || data == NULL || count == 0) {
        return 0;
    }
    
    unsigned int available_count = aglo_ring_buffer_get_count(rb);
    if (available_count == 0) {
        return 0;
    }
    
    // 限制读取数量不超过可用数据
    if (count > available_count) {
        count = available_count;
    }
    
    uint8_t *dst = (uint8_t *)data;
    unsigned int read = 0;
    
    while (read < count) {
        // 计算当前可连续读取的元素个数
        unsigned int linear_read = get_linear_read_size(rb);
        unsigned int elements_to_read = linear_read / rb->element_size;
        
        // 限制本次读取数量
        if (elements_to_read > (count - read)) {
            elements_to_read = count - read;
        }
        
        if (elements_to_read == 0) {
            break;
        }
        
        // 读取数据
        unsigned int bytes_to_read = elements_to_read * rb->element_size;
        uint8_t *read_ptr = rb->buff + rb->out;
        memcpy(dst + (read * rb->element_size), read_ptr, bytes_to_read);
        
        // 更新位置和计数
        rb->out += bytes_to_read;
        if (rb->out >= rb->size) {
            rb->out = 0;
        }
        rb->count -= elements_to_read;
        read += elements_to_read;
    }
    
    return read;
}

unsigned int aglo_ring_buffer_peek(const aglo_ring_buffer_t *rb, void *data, unsigned int offset)
{
    if (rb == NULL || data == NULL) {
        return 0;
    }
    
    unsigned int available_count = aglo_ring_buffer_get_count(rb);
    if (offset >= available_count) {
        return 0;
    }
    
    // 计算实际读取位置（考虑偏移和循环）
    unsigned int read_offset = (rb->out + offset * rb->element_size) % rb->size;
    uint8_t *read_ptr = rb->buff + read_offset;
    memcpy(data, read_ptr, rb->element_size);
    
    return 1;
}

unsigned int aglo_ring_buffer_skip(aglo_ring_buffer_t *rb, unsigned int count)
{
    if (rb == NULL || count == 0) {
        return 0;
    }
    
    unsigned int available_count = aglo_ring_buffer_get_count(rb);
    if (available_count == 0) {
        return 0;
    }
    
    // 限制跳过数量不超过可用数据
    if (count > available_count) {
        count = available_count;
    }
    
    // 更新读取位置和计数
    unsigned int bytes_to_skip = count * rb->element_size;
    rb->out += bytes_to_skip;
    if (rb->out >= rb->size) {
        rb->out = rb->out % rb->size;
    }
    rb->count -= count;
    
    return count;
}

