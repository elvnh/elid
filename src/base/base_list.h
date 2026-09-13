#pragma once

// Defines singly- and doubly-linked list and node structs for a given type.
#define DEFINE_LIST_TYPES(t)                    \
    struct t##_Node {                           \
        t data;                                 \
        t##_Node *next;                         \
        t##_Node *prev;                         \
    };                                          \
    struct t##_SNode {                          \
        t data;                                 \
        t##_SNode *next;                        \
    };                                          \
    struct t##_List {                           \
        t##_Node *first;                        \
        t##_Node *last;                         \
        usize     list_node_count;              \
    };                                          \
    struct t##_SList {                          \
        t##_SNode *first;                       \
        t##_SNode *last;                        \
        usize     list_node_count;              \
    };                                          \
    struct t##_Stack {                          \
        t##_SNode *head;                        \
        usize     list_node_count;             \
    }

/* Doubly linked lists */
// TODO: save a copy of node to avoid macro bugs
#define dll_push_front_npfl(list, node, next, prev, first, last)    \
    (((node)->next = (list)->first),                                \
    ((node)->prev = 0),                                             \
    ((list)->first                                                  \
        ? (list)->first->prev = (node)                              \
        : 0),                                                       \
    ((list)->first = node),                                         \
    (!((list)->last)                                                \
        ? (list)->last = (node)                                     \
        : 0),                                                       \
        (++(list)->list_node_count)                                      \
    )

#define dll_push_front(list, node) dll_push_front_npfl((list), (node), next, prev, first, last)

#define dll_push_back_npfl(list, node, next, prev, first, last) \
    (((node)->prev = (list)->last),                             \
        ((node)->next = 0),                                     \
        ((list)->last                                           \
            ? (list)->last->next = (node)                       \
            : 0),                                               \
        ((list)->last = (node)),                                \
        ((!((list)->first))                                     \
            ? (list)->first = (node)                            \
            : 0),                                               \
        (++(list)->list_node_count)                             \
    )

#define dll_push_back(list, node) dll_push_back_npfl((list), (node), next, prev, first, last)

#define dll_insert_after_npfl(list, node, after, next, prev, first, last) \
    (((node)->prev = (after)),                                            \
        ((node)->next = (after)->next),                                   \
        ((after)->next                                                    \
            ? ((after)->next->prev = (node))                              \
            : 0),                                                         \
        ((after)->next = (node)),                                         \
        (((list)->last == (after))                                        \
            ? (list)->last = (node)                                       \
            : 0),                                                         \
     (++(list)->list_node_count)                                               \
    )

#define dll_insert_after(list, node, after) dll_insert_after_npfl((list), (node), (after), next, prev, first, last)

#define dll_insert_before_npfl(list, node, before, next, prev, first, last) \
    (((node)->prev = (before)->prev),                                   \
        ((node)->next = (before)),                                      \
        ((before)->prev                                                 \
            ? ((before)->prev->next = (node))                           \
            : 0                                                         \
        ),                                                              \
        ((before)->prev = (node)),                                      \
        (((list)->first == (before))                                    \
            ? (list)->first  = (node)                                   \
            : 0),                                                       \
        (++(list)->list_node_count)                                     \
    )

#define dll_insert_before(list, node, before) dll_insert_before_npfl((list), (node), (before), next, prev, first, last)

#define dll_remove_npfl(list, node, next, prev, first, last)    \
    (((node)->prev                                              \
        ? ((node)->prev->next = (node)->next)                   \
        : 0),                                                   \
        ((node)->next                                           \
            ? ((node)->next->prev = (node)->prev)               \
            : 0),                                               \
        ((list)->first == (node)                                \
            ? ((list)->first = (node)->next)                    \
            : 0),                                               \
        ((list)->last == (node)                                 \
            ? ((list)->last = (node)->prev)                     \
            : 0),                                               \
     ((node)->next = (node)->prev = 0),                         \
     (--(list)->list_node_count)                                     \
    )

#define dll_remove(list, node) dll_remove_npfl((list), (node), next, prev, first, last)

/* Singly linked lists */
#define sll_push_front_nfl(list, node, next, first, last)           \
    (((node)->next = (list)->first),                                \
        (list)->first = (node),                                     \
        (!((list)->last)                                            \
            ? ((list)->last = (node))                               \
            : 0),                                                   \
        (++(list)->list_node_count)                                 \
    )

#define sll_push_front(list, node) sll_push_front_nfl((list), (node), next, first, last)

#define sll_push_back_nfl(list, node, next, first, last)        \
    (((list)->last                                              \
        ? ((list)->last->next = node)                           \
        : 0),                                                   \
        ((list)->last = (node)),                                \
        ((!(list)->first)                                       \
            ? (list)->first = (node)                            \
            : 0),                                               \
        (++(list)->list_node_count)                                  \
    )

#define sll_push_back(list, node) sll_push_back_nfl((list), (node), next, first, last)

#define sll_insert_nfl(list, node, after, next, first, last) \
    ((node)->next = (after)->next,                                  \
        (after)->next = (node),                                     \
        ((list)->last == (after)                                    \
            ? ((list)->last = node)                                 \
            : 0),                                                   \
        (++(list)->list_node_count)                                      \
    )

#define sll_insert(list, node, after) sll_insert_nfl((list), (node), (after), next, first, last)

/* Stacks */
#define stack_push_nh_no_count(stack, node, next, head) \
    (((node)->next = (stack)->head),                    \
        ((stack)->head = (node)))

#define stack_push_nh(stack, node, next, head)          \
    (stack_push_nh_no_count(stack, node, next, head),   \
        (++(stack)->list_node_count)                   \
    )

#define stack_push(stack, node) stack_push_nh((stack), (node), next, head)
#define stack_push_no_count(stack, node) stack_push_nh((stack), (node), next, head)

// TODO: make popping operations set next link to null
#define stack_pop_nh_no_count(stack, next, head)         \
    (((stack)->head ?                           \
        ((stack)->head = (stack)->head->next)   \
        : 0))

#define stack_pop_nh(stack, next, head)                  \
    (((stack)->head ? (--(stack)->list_node_count) : 0), \
    stack_pop_nh_no_count(stack, next, head))


#define stack_pop(stack) stack_pop_nh((stack), next, head)

/* Queues */
#define queue_push_nfl(queue, node, next, first, last) sll_push_back_nfl((queue), (node), next, first, last)
#define queue_push(queue, node) queue_push_nfl((queue), (node), next, first, last)

#define queue_pop_nfl(queue, next, first, last)             \
    (((queue)->first ? (--(queue)->list_node_count) : 0),   \
    ((stack_pop_nh_no_count(queue, next, first)),           \
        ((!((queue)->first))                                \
            ? ((queue)->last = 0)                           \
            : 0)))

#define queue_pop(queue) queue_pop_nfl((queue), next, first, last)

/* Iteration helpers */
#define list_head_f(list, first) ((list)->first)
#define list_head(list) list_head_f((list), first)
#define list_next_n(list, next) ((list)->next)
#define list_next(list) list_next_n((list), next)

/* Query operations */
#define list_count(list) (list)->list_node_count
#define list_is_empty(list) ((list)->list_node_count == 0)
#define queue_is_empty(q) list_is_empty((q))
