#include "test_base_list.cpp"

static void test_base(void)
{
    /* Doubly linked lists */
    test_base_dl_list_push_front_basic();
    test_base_dl_list_push_front_nonempty();
    test_base_dl_list_push_front_multiple_nodes();

    test_base_dl_list_push_back_basic();
    test_base_dl_list_push_back_nonempty();
    test_base_dl_list_push_back_multiple_nodes();

    test_base_dl_list_push_front_then_back();
    test_base_dl_list_push_back_then_front();
    test_base_dl_list_push_front_then_back_nonempty();
    test_base_dl_list_push_back_then_front_nonempty();

    test_base_dl_list_insert_after_basic();
    test_base_dl_list_insert_before_basic();

    test_base_dl_list_insert_before_first_node();
    test_base_dl_list_insert_after_middle_node();
    test_base_dl_list_insert_before_last_node();
    test_base_dl_list_insert_after_first_node();
    test_base_dl_list_insert_before_middle_node();
    test_base_dl_list_insert_after_last_node();

    /* Singly linked lists */
    test_base_sl_list_push_front_empty();
    test_base_sl_list_push_front_single_elem();
    test_base_sl_list_push_front_multi_elem();

    test_base_sl_list_push_back_empty();
    test_base_sl_list_push_back_single_elem();
    test_base_sl_list_push_back_multi_elem();

    test_base_sl_list_insert_single_elem();
    test_base_sl_list_insert_after_first();
    test_base_sl_list_insert_after_middle();
    test_base_sl_list_insert_after_last();
}
