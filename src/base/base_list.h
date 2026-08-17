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
    };                                          \
    struct t##_SList {                          \
        t##_SNode *first;                       \
        t##_SNode *last;                        \
    };

/* Doubly linked lists */
// TODO: fix this one
#define sll_insert_after(node, after)           \
    (node)->next = (after)->next,               \
    (after)->next = (node)

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
        : 0))

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
            : 0))

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
            : 0))

#define dll_insert_after2(list, node, after) dll_insert_after_npfl((list), (node), (after), next, prev, first, last)

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
            : 0))

#define dll_insert_before(list, node, before) dll_insert_before_npfl((list), (node), (before), next, prev, first, last)

/* Singly linked lists */
#define sll_push_front_npfl(list, node, next, prev, first, last)    \
    (((node)->next = (list)->first),                                \
        (list)->first = (node),                                     \
        (!((list)->last)                                            \
            ? ((list)->last = (node))                               \
            : 0))

#define sll_push_front(list, node) sll_push_front_npfl((list), (node), next, prev, first, last)

#define sll_push_back_npfl(list, node, next, prev, first, last) \
    (((list)->last                                              \
        ? ((list)->last->next = node)                           \
        : 0),                                                   \
        ((list)->last = (node)),                                \
        ((!(list)->first)                                       \
            ? (list)->first = (node)                            \
            : 0))

#define sll_push_back(list, node) sll_push_back_npfl((list), (node), next, prev, first, last)

#define sll_insert_npfl(list, node, after, next, prev, first, last) \
    ((node)->next = (after)->next,                                  \
        (after)->next = (node),                                     \
        ((list)->last == (after)                                    \
            ? ((list)->last = node)                                 \
            : 0))

#define sll_insert(list, node, after) sll_insert_npfl((list), (node), (after), next, prev, first, last)
