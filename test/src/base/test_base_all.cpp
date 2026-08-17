#include "test_base_arena.cpp"
#include "test_base_pool.cpp"
#include "test_base_list.cpp"

static void test_base(void)
{
    /* Arena */
    test_arena_basic();
    test_arena_reset();
    test_arena_reset_after_growing();
    test_arena_memory_usage_basic();
    test_arena_memory_usage_after_growing();
    test_arena_zeroed_fresh_arena();
    test_arena_zeroed_after_reset();
    test_arena_zeroed_after_growing_and_resetting();
    test_arena_alignment();
    test_arena_alignment_after_growing();
    test_arena_alignment_after_growing_and_resetting();
    test_arena_pop_to();
    test_arena_pop_to_after_growing();
    test_arena_pop_to_new_buffer();
    test_arena_different_alignments_and_sizes();
    test_arena_allocate_entire_arena();
    test_arena_allocate_entire_arena_different_sizes();

    /* Pool */
    test_pool_basic();
    test_pool_grow();
    test_pool_grow_beyond_arena();
    test_pool_free_node_count();
    test_pool_free_node_count_grow();
    test_pool_total_node_count();
    test_pool_free();
    test_pool_used_node_count();
    test_pool_used_node_count_grow();
    test_pool_alignment();
    test_pool_different_alignments_and_sizes();
    test_pool_reset();
    test_pool_reset_after_growing();
    test_pool_realloc_after_reset_after_growing();
    test_pool_zeroed();
    test_pool_zeroed_after_reset();
    test_pool_zeroed_after_growing();
    test_pool_zeroed_after_growing_and_reset();

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
