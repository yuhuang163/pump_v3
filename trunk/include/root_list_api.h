#ifndef __ROOT_LIST_API_H__
#define __ROOT_LIST_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdio.h> // IWYU pragma: keep
#include <stdlib.h> // IWYU pragma: keep

typedef struct list_head {
    struct list_head *next;
} list_head_t;

typedef void *(*list_del_condition_cb_fptr)(void *data, list_head_t *pos);

#ifndef LIST_HEAD
#define LIST_HEAD(name) \
    struct list_head name = {NULL}
#endif

#ifndef INIT_LIST_HEAD
#define INIT_LIST_HEAD(head) \
    (head)->next = NULL
#endif

/// 需要配置 CFLAGS += -std=c99
#ifndef LIST_FOR_EACH
#define LIST_FOR_EACH(pos, head) \
    for (list_head_t *(pos) = (head)->next; (pos) != NULL; (pos) = (pos)->next)
#endif

#define LIST_EMPTY(head, cb) \
do { \
    for (list_head_t *(pos) = (head)->next; (pos) != NULL; ) { \
        void *node = (cb)(NULL, pos); \
        head->next = pos->next; \
        (pos) = (pos)->next; \
        u_free(node); \
    } \
} while (0)

#ifndef CONTAINER_OF
#define CONTAINER_OF(ptr, type, menber) (type *)((char *)ptr - (char *) &(((type *)0)->menber))
#endif

int __list_add(list_head_t *new_node, list_head_t *head);
int __list_add_tail(list_head_t *new_node, list_head_t *head);
int __list_del(list_head_t *head, void *data, list_del_condition_cb_fptr cb);
int __list_empty(list_head_t *head, list_del_condition_cb_fptr cb);

#ifdef __cplusplus
}
#endif

#endif 


