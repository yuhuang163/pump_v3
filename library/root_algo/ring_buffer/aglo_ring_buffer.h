/*******************************************************************************
 * Copyright (c) Shenzhen Root Innovation
 * File name: aglo_ring_buffer.h
 * Description: 轻量级通用环形缓冲区管理库（基于LwRB设计理念）
 * Author: Auto-generated
 * Date: 2026-01-07
 ******************************************************************************/

#ifndef _AGLO_RING_BUFFER_H_
#define _AGLO_RING_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>

/*******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief 环形缓冲区结构体
 */
typedef struct {
    uint8_t *buff;          ///< 缓冲区数据指针
    unsigned int size;            ///< 缓冲区总大小（字节）
    unsigned int element_size;    ///< 单个元素大小（字节）
    unsigned int in;              ///< 写入位置索引（字节偏移）
    unsigned int out;             ///< 读取位置索引（字节偏移）
    unsigned int count;           ///< 当前元素个数
} aglo_ring_buffer_t;

/*******************************************************************************
 * Function declarations
 ******************************************************************************/

/**
 * @brief 初始化环形缓冲区
 * @param rb 环形缓冲区指针
 * @param buff 缓冲区数据指针（用户提供）
 * @param size 缓冲区总大小（字节）
 * @param element_size 单个元素大小（字节）
 * @return 0:成功, -1:失败
 */
int aglo_ring_buffer_init(aglo_ring_buffer_t *rb, void *buff, unsigned int size, unsigned int element_size);

/**
 * @brief 清空环形缓冲区
 * @param rb 环形缓冲区指针
 */
void aglo_ring_buffer_reset(aglo_ring_buffer_t *rb);

/**
 * @brief 获取缓冲区中可用的元素个数
 * @param rb 环形缓冲区指针
 * @return 可用元素个数
 */
unsigned int aglo_ring_buffer_get_count(const aglo_ring_buffer_t *rb);

/**
 * @brief 获取缓冲区中空闲的元素个数
 * @param rb 环形缓冲区指针
 * @return 空闲元素个数
 */
unsigned int aglo_ring_buffer_get_free(const aglo_ring_buffer_t *rb);

/**
 * @brief 检查缓冲区是否为空
 * @param rb 环形缓冲区指针
 * @return 1:空, 0:非空
 */
int aglo_ring_buffer_is_empty(const aglo_ring_buffer_t *rb);

/**
 * @brief 检查缓冲区是否已满
 * @param rb 环形缓冲区指针
 * @return 1:满, 0:未满
 */
int aglo_ring_buffer_is_full(const aglo_ring_buffer_t *rb);

/**
 * @brief 写入单个元素到缓冲区
 * @param rb 环形缓冲区指针
 * @param data 要写入的数据指针
 * @return 实际写入的元素个数（0或1）
 */
unsigned int aglo_ring_buffer_write(aglo_ring_buffer_t *rb, const void *data);

/**
 * @brief 批量写入元素到缓冲区
 * @param rb 环形缓冲区指针
 * @param data 要写入的数据指针
 * @param count 要写入的元素个数
 * @return 实际写入的元素个数
 */
unsigned int aglo_ring_buffer_write_multiple(aglo_ring_buffer_t *rb, const void *data, unsigned int count);

/**
 * @brief 从缓冲区读取单个元素
 * @param rb 环形缓冲区指针
 * @param data 读取数据存储指针
 * @return 实际读取的元素个数（0或1）
 */
unsigned int aglo_ring_buffer_read(aglo_ring_buffer_t *rb, void *data);

/**
 * @brief 从缓冲区批量读取元素
 * @param rb 环形缓冲区指针
 * @param data 读取数据存储指针
 * @param count 要读取的元素个数
 * @return 实际读取的元素个数
 */
unsigned int aglo_ring_buffer_read_multiple(aglo_ring_buffer_t *rb, void *data, unsigned int count);

/**
 * @brief 查看缓冲区中的数据（不移动读取位置）
 * @param rb 环形缓冲区指针
 * @param data 数据存储指针
 * @param offset 偏移量（从读取位置开始的元素偏移）
 * @return 实际查看的元素个数（0或1）
 */
unsigned int aglo_ring_buffer_peek(const aglo_ring_buffer_t *rb, void *data, unsigned int offset);

/**
 * @brief 跳过指定数量的元素（不读取数据，只移动读取位置）
 * @param rb 环形缓冲区指针
 * @param count 要跳过的元素个数
 * @return 实际跳过的元素个数
 */
unsigned int aglo_ring_buffer_skip(aglo_ring_buffer_t *rb, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif /* _AGLO_RING_BUFFER_H_ */

