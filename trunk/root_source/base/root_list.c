/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "root_list_api.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "cmsis_os_api.h" // IWYU pragma: keep

/*
 * DEFINES
 *****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int __list_add(list_head_t *new_node, list_head_t *head)
{
    if (head->next == NULL) {
        head->next = new_node;
        new_node->next = NULL;
    } else {
        new_node->next = head->next;
        head->next = new_node;
    }
    return 0;
}

int __list_add_tail(list_head_t *new_node, list_head_t *head)
{
    if (head->next == NULL) {
        head->next = new_node;
        new_node->next = NULL;
    } else {
        LIST_FOR_EACH(pos, head) {
            if (pos->next == NULL) {
                pos->next = new_node;
                new_node->next = NULL;
                break;
            }
        }
    }
    return 0;
}

int __list_del(list_head_t *head, void *data, list_del_condition_cb_fptr cb)
{
    void *node = NULL;
    list_head_t * prev = NULL;
     
    LIST_FOR_EACH(pos, (head)) {
        if ((node = cb((data), pos)) != NULL) {
            if ((pos) == (head)->next) {
                (head)->next = (pos)->next;
            } else {
                prev->next = (pos)->next;
            }
            u_free(node);
            node = NULL;
            return 0;
        }
        prev = pos;
    }
    return -1;
}

int __list_empty(list_head_t *head, list_del_condition_cb_fptr cb)
{
    void *node = NULL;
     
    for (list_head_t *pos = head->next; pos != NULL; ) {
        node = cb(NULL, pos);
        head->next = pos->next;
        pos = pos->next;
        u_free(node);
    }

    return 0;
}
