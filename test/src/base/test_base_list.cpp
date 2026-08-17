/* Doubly linked lists */
TEST_CASE(dl_list_push_front_basic)
{
    String_List list = {};

    String_Node *node = arena_alloc_struct(test_arena, String_Node);

    dll_push_front(&list, node);

    REQUIRE(list.first == node);
    REQUIRE(list.last == node);
    REQUIRE(node->next == 0);
    REQUIRE(node->prev == 0);
}

TEST_CASE(dl_list_push_front_nonempty)
{
    String_List list = {};

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);

    dll_push_front(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node2);

    REQUIRE(list.first == node2);
    REQUIRE(list.last == node1);

    REQUIRE(node1->prev == node2);
    REQUIRE(node1->next == 0);

    REQUIRE(node2->prev == 0);
    REQUIRE(node2->next == node1);
}

TEST_CASE(dl_list_push_front_multiple_nodes)
{
    String_List list = {};

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node2);

    String_Node *node3 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node3);

    REQUIRE(list.first == node3);
    REQUIRE(list.last == node1);

    REQUIRE(node1->prev == node2);
    REQUIRE(node1->next == 0);

    REQUIRE(node2->prev == node3);
    REQUIRE(node2->next == node1);

    REQUIRE(node3->prev == 0);
    REQUIRE(node3->next == node2);
}

TEST_CASE(dl_list_push_back_basic)
{
    String_List list = {};

    String_Node *node = arena_alloc_struct(test_arena, String_Node);

    dll_push_back(&list, node);

    REQUIRE(list.first == node);
    REQUIRE(list.last == node);
    REQUIRE(node->next == 0);
    REQUIRE(node->prev == 0);
}

TEST_CASE(dl_list_push_back_nonempty)
{
    String_List list = {};

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node2);

    REQUIRE(list.first == node1);
    REQUIRE(list.last == node2);

    REQUIRE(node1->next == node2);
    REQUIRE(node1->prev == 0);

    REQUIRE(node2->next == 0);
    REQUIRE(node2->prev == node1);
}

TEST_CASE(dl_list_push_back_multiple_nodes)
{
    String_List list = {};

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node2);

    String_Node *node3 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node3);

    REQUIRE(list.first == node1);
    REQUIRE(list.last == node3);

    REQUIRE(node1->prev == 0);
    REQUIRE(node1->next == node2);

    REQUIRE(node2->prev == node1);
    REQUIRE(node2->next == node3);

    REQUIRE(node3->prev == node2);
    REQUIRE(node3->next == 0);
}

TEST_CASE(dl_list_push_front_then_back)
{
    String_List list = {};

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node2);

    REQUIRE(list.first == node1);
    REQUIRE(list.last == node2);

    REQUIRE(node1->prev == 0);
    REQUIRE(node1->next == node2);

    REQUIRE(node2->prev == node1);
    REQUIRE(node2->next == 0);
}

TEST_CASE(dl_list_push_back_then_front)
{
    String_List list = {};

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node2);

    REQUIRE(list.first == node2);
    REQUIRE(list.last == node1);

    REQUIRE(node1->prev == node2);
    REQUIRE(node1->next == 0);

    REQUIRE(node2->prev == 0);
    REQUIRE(node2->next == node1);
}

TEST_CASE(dl_list_push_front_then_back_nonempty)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node2);

    REQUIRE(list.first == node1);
    REQUIRE(list.last == node2);

    REQUIRE(node0->prev == node1);
    REQUIRE(node0->next == node2);

    REQUIRE(node1->prev == 0);
    REQUIRE(node1->next == node0);

    REQUIRE(node2->prev == node0);
    REQUIRE(node2->next == 0);
}

TEST_CASE(dl_list_push_back_then_front_nonempty)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node2);

    REQUIRE(list.first == node2);
    REQUIRE(list.last == node1);

    REQUIRE(node0->prev == node2);
    REQUIRE(node0->next == node1);

    REQUIRE(node1->prev == node0);
    REQUIRE(node1->next == 0);

    REQUIRE(node2->prev == 0);
    REQUIRE(node2->next == node0);
}

TEST_CASE(dl_list_insert_after_basic)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_insert_after(&list, node1, node0);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node1);

    REQUIRE(node0->next == node1);
    REQUIRE(node0->prev == 0);

    REQUIRE(node1->next == 0);
    REQUIRE(node1->prev == node0);
}

TEST_CASE(dl_list_insert_before_basic)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_front(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_insert_before(&list, node1, node0);

    REQUIRE(list.first == node1);
    REQUIRE(list.last == node0);

    REQUIRE(node0->next == 0);
    REQUIRE(node0->prev == node1);

    REQUIRE(node1->next == node0);
    REQUIRE(node1->prev == 0);
}

TEST_CASE(dl_list_insert_before_first_node)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_insert_before(&list, node2, node0);

    REQUIRE(list.first == node2);
    REQUIRE(list.last == node1);

    REQUIRE(node0->next == node1);
    REQUIRE(node0->prev == node2);

    REQUIRE(node1->next == 0);
    REQUIRE(node1->prev == node0);

    REQUIRE(node2->next == node0);
    REQUIRE(node2->prev == 0);
}

TEST_CASE(dl_list_insert_before_middle_node)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node2);

    String_Node *node3 = arena_alloc_struct(test_arena, String_Node);
    dll_insert_before(&list, node3, node1);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node2);

    REQUIRE(node0->next == node3);
    REQUIRE(node0->prev == 0);

    REQUIRE(node3->next == node1);
    REQUIRE(node3->prev == node0);

    REQUIRE(node1->next == node2);
    REQUIRE(node1->prev == node3);

    REQUIRE(node2->next == 0);
    REQUIRE(node2->prev == node1);
}

TEST_CASE(dl_list_insert_before_last_node)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_insert_before(&list, node2, node1);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node1);

    REQUIRE(node0->next == node2);
    REQUIRE(node0->prev == 0);

    REQUIRE(node1->next == 0);
    REQUIRE(node1->prev == node2);

    REQUIRE(node2->next == node1);
    REQUIRE(node2->prev == node0);
}

TEST_CASE(dl_list_insert_after_first_node)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_insert_after(&list, node2, node0);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node1);

    REQUIRE(node0->next == node2);
    REQUIRE(node0->prev == 0);

    REQUIRE(node1->next == 0);
    REQUIRE(node1->prev == node2);

    REQUIRE(node2->next == node1);
    REQUIRE(node2->prev == node0);
}

TEST_CASE(dl_list_insert_after_middle_node)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node2);

    String_Node *node3 = arena_alloc_struct(test_arena, String_Node);
    dll_insert_after(&list, node3, node1);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node2);

    REQUIRE(node0->next == node1);
    REQUIRE(node0->prev == 0);

    REQUIRE(node1->next == node3);
    REQUIRE(node1->prev == node0);

    REQUIRE(node3->next == node2);
    REQUIRE(node3->prev == node1);

    REQUIRE(node2->next == 0);
    REQUIRE(node2->prev == node3);
}

TEST_CASE(dl_list_insert_after_last_node)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_insert_after(&list, node2, node1);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node2);

    REQUIRE(node0->next == node1);
    REQUIRE(node0->prev == 0);

    REQUIRE(node1->next == node2);
    REQUIRE(node1->prev == node0);

    REQUIRE(node2->next == 0);
    REQUIRE(node2->prev == node1);
}

TEST_CASE(dl_list_remove_single_elem)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    dll_remove(&list, node0);

    REQUIRE(list.first == 0);
    REQUIRE(list.last == 0);
}

TEST_CASE(dl_list_remove_first_of_two)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    dll_remove(&list, node0);

    REQUIRE(list.first == node1);
    REQUIRE(list.last == node1);

    REQUIRE(node0->next == 0);
    REQUIRE(node0->prev == 0);

    REQUIRE(node1->prev == 0);
    REQUIRE(node1->next == 0);
}

TEST_CASE(dl_list_remove_last_of_two)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    dll_remove(&list, node1);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node0);

    REQUIRE(node1->next == 0);
    REQUIRE(node1->prev == 0);

    REQUIRE(node0->prev == 0);
    REQUIRE(node0->next == 0);
}

TEST_CASE(dl_list_remove_first)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node2);

    dll_remove(&list, node0);

    REQUIRE(list.first == node1);
    REQUIRE(list.last == node2);

    REQUIRE(node0->next == 0);
    REQUIRE(node0->prev == 0);

    REQUIRE(node1->prev == 0);
    REQUIRE(node1->next == node2);

    REQUIRE(node2->prev == node1);
    REQUIRE(node2->next == 0);
}

TEST_CASE(dl_list_remove_middle)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node2);

    dll_remove(&list, node1);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node2);

    REQUIRE(node1->next == 0);
    REQUIRE(node1->prev == 0);

    REQUIRE(node0->prev == 0);
    REQUIRE(node0->next == node2);

    REQUIRE(node2->prev == node0);
    REQUIRE(node2->next == 0);
}

TEST_CASE(dl_list_remove_last)
{
    String_List list = {};

    String_Node *node0 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node0);

    String_Node *node1 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node1);

    String_Node *node2 = arena_alloc_struct(test_arena, String_Node);
    dll_push_back(&list, node2);

    dll_remove(&list, node2);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node1);

    REQUIRE(node2->next == 0);
    REQUIRE(node2->prev == 0);

    REQUIRE(node0->prev == 0);
    REQUIRE(node0->next == node1);

    REQUIRE(node1->prev == node0);
    REQUIRE(node1->next == 0);
}

/* Singly linked lists */
TEST_CASE(sl_list_push_front_empty)
{
    String_SList list = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_front(&list, node0);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node0);

    REQUIRE(node0->next == 0);
}

TEST_CASE(sl_list_push_front_single_elem)
{
    String_SList list = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_front(&list, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_front(&list, node1);

    REQUIRE(list.first == node1);
    REQUIRE(list.last == node0);

    REQUIRE(node0->next == 0);
    REQUIRE(node1->next == node0);
}

TEST_CASE(sl_list_push_front_multi_elem)
{
    String_SList list = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_front(&list, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_front(&list, node1);

    String_SNode *node2 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_front(&list, node2);

    REQUIRE(list.first == node2);
    REQUIRE(list.last == node0);

    REQUIRE(node0->next == 0);
    REQUIRE(node1->next == node0);
    REQUIRE(node2->next == node1);
}

TEST_CASE(sl_list_push_back_empty)
{
    String_SList list = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node0);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node0);

    REQUIRE(node0->next == 0);
}

TEST_CASE(sl_list_push_back_single_elem)
{
    String_SList list = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node1);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node1);

    REQUIRE(node0->next == node1);
    REQUIRE(node1->next == 0);
}

TEST_CASE(sl_list_push_back_multi_elem)
{
    String_SList list = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node1);

    String_SNode *node2 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node2);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node2);

    REQUIRE(node0->next == node1);
    REQUIRE(node1->next == node2);
    REQUIRE(node2->next == 0);
}

TEST_CASE(sl_list_insert_single_elem)
{
    String_SList list = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    sll_insert(&list, node1, node0);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node1);

    REQUIRE(node0->next == node1);
    REQUIRE(node1->next == 0);
}

TEST_CASE(sl_list_insert_after_first)
{
    String_SList list = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node1);

    String_SNode *node2 = arena_alloc_struct(test_arena, String_SNode);
    sll_insert(&list, node2, node0);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node1);

    REQUIRE(node0->next == node2);
    REQUIRE(node2->next == node1);
    REQUIRE(node1->next == 0);
}

TEST_CASE(sl_list_insert_after_middle)
{
    String_SList list = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node1);

    String_SNode *node2 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node2);

    String_SNode *node3 = arena_alloc_struct(test_arena, String_SNode);
    sll_insert(&list, node3, node1);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node2);

    REQUIRE(node0->next == node1);
    REQUIRE(node1->next == node3);
    REQUIRE(node3->next == node2);
    REQUIRE(node2->next == 0);
}

TEST_CASE(sl_list_insert_after_last)
{
    String_SList list = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node1);

    String_SNode *node2 = arena_alloc_struct(test_arena, String_SNode);
    sll_push_back(&list, node2);

    String_SNode *node3 = arena_alloc_struct(test_arena, String_SNode);
    sll_insert(&list, node3, node2);

    REQUIRE(list.first == node0);
    REQUIRE(list.last == node3);

    REQUIRE(node0->next == node1);
    REQUIRE(node1->next == node2);
    REQUIRE(node2->next == node3);
    REQUIRE(node3->next == 0);
}

/* Stacks */
TEST_CASE(stack_push_empty)
{
    String_Stack stack = {};
    String_SNode *first = arena_alloc_struct(test_arena, String_SNode);

    stack_push(&stack, first);

    REQUIRE(stack.head == first);
    REQUIRE(first->next == 0);
}

TEST_CASE(stack_push_two)
{
    String_Stack stack = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node1);

    REQUIRE(stack.head == node1);
    REQUIRE(node1->next == node0);
    REQUIRE(node0->next == 0);
}

TEST_CASE(stack_push_multiple)
{
    String_Stack stack = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node1);

    String_SNode *node2 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node2);

    REQUIRE(stack.head == node2);
    REQUIRE(node2->next == node1);
    REQUIRE(node1->next == node0);
    REQUIRE(node0->next == 0);
}

TEST_CASE(stack_pop_empty)
{
    String_Stack stack = {};
    stack_pop(&stack);

    REQUIRE(stack.head == 0);
}

TEST_CASE(stack_pop_to_empty)
{
    String_Stack stack = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node0);

    stack_pop(&stack);

    REQUIRE(stack.head == 0);
    REQUIRE(node0->next == 0);
}

TEST_CASE(stack_pop_nonempty)
{
    String_Stack stack = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node1);

    stack_pop(&stack);

    REQUIRE(stack.head == node0);
    REQUIRE(node0->next == 0);
}

TEST_CASE(stack_pop_to_nonempty_multiple)
{
    String_Stack stack = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node1);

    String_SNode *node2 = arena_alloc_struct(test_arena, String_SNode);
    stack_push(&stack, node2);

    stack_pop(&stack);

    REQUIRE(stack.head == node1);
}


/* Queues */
TEST_CASE(queue_basic)
{
    String_SList queue = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    queue_push(&queue, node0);

    REQUIRE(queue.first == node0);
    REQUIRE(queue.last == node0);
}

TEST_CASE(queue_push_multiple)
{
    String_SList queue = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    queue_push(&queue, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    queue_push(&queue, node1);

    String_SNode *node2 = arena_alloc_struct(test_arena, String_SNode);
    queue_push(&queue, node2);

    REQUIRE(queue.first == node0);
    REQUIRE(queue.last == node2);

    REQUIRE(node0->next == node1);
    REQUIRE(node1->next == node2);
    REQUIRE(node2->next == 0);
}

TEST_CASE(queue_pop_empty)
{
    String_SList queue = {};
    queue_pop(&queue);

    REQUIRE(queue.first == 0);
    REQUIRE(queue.last == 0);
}

TEST_CASE(queue_pop_to_empty)
{
    String_SList queue = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    queue_push(&queue, node0);

    queue_pop(&queue);

    REQUIRE(queue.first == 0);
    REQUIRE(queue.last == 0);

    REQUIRE(node0->next == 0);
}

TEST_CASE(queue_pop_to_one_elem)
{
    String_SList queue = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    queue_push(&queue, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    queue_push(&queue, node1);

    queue_pop(&queue);

    REQUIRE(queue.first == node1);
    REQUIRE(queue.last == node1);
}

TEST_CASE(queue_pop_to_two_elem)
{
    String_SList queue = {};

    String_SNode *node0 = arena_alloc_struct(test_arena, String_SNode);
    queue_push(&queue, node0);

    String_SNode *node1 = arena_alloc_struct(test_arena, String_SNode);
    queue_push(&queue, node1);

    String_SNode *node2 = arena_alloc_struct(test_arena, String_SNode);
    queue_push(&queue, node2);

    queue_pop(&queue);

    REQUIRE(queue.first == node1);
    REQUIRE(queue.last == node2);
}
