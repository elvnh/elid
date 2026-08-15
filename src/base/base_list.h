#pragma once

#define dll_insert_after(n, after)             \
    (n)->prev = (after),                        \
    (n)->next = (after)->next,                  \
    (after)->next = (n),                        \
    (n)->next ? (n)->next->prev = (n) : 0

#define sll_insert_after(node, after)           \
    (node)->next = (after)->next,               \
    (after)->next = (node)
