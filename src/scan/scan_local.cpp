#include "scan_local.h"

static scan_Value scan__load_value_at_address(void *address, scan_ValueType type)
{
    scan_Value result = {};

    usize type_size = scan_val_type_get_size(type);

    copy_mem(&result, address, 1, type_size);

    return result;
}

static scan_MatchList *scan__alloc_match_list(Arena *arena)
{
    scan_MatchList *result = arena_alloc_struct(arena, scan_MatchList);
    result->first = result;
    result->last = result;

    return result;
}

static void scan__push_match(Arena *arena, scan_MatchList *list, scan_Match match, scan_MatchHistory history)
{
    scan_MatchList *current = list->last;

    ASSERT(current->count <= array_capacity(current->match_data));
    ASSERT(array_capacity(current->match_data) == array_capacity(current->match_history_data));

    // TODO: use array_try_push here
    if (current->count == array_capacity(current->match_data)) {
        current = scan__alloc_match_list(arena);
        sll_push_back(list, current);
    }

    usize index = current->count++;
    current->match_data[index] = match;
    current->match_history_data[index] = history;

    ++list->total_match_count;
}

struct scan_LocalScanArgs {
    void *memory;
    usize memory_size;
    Arena **per_thread_result_arenas;
    scan_MatchList *per_thread_match_lists;
    uintptr_t base_address;
    scan_Comparison cmp;
    usize alignment;
    scan_MultithreadArgs mt_args;
    usize local_offset; // TODO: remove, unneccessary if we store absolute address in scan_Match
};

static void scan__local_memory_first_async(TaskID task, void *data)
{
    scan_LocalScanArgs args = *(scan_LocalScanArgs *)data;

    args.mt_args.parent_task = task;

    scan_local_memory_first_new(
        args.per_thread_result_arenas,
        args.per_thread_match_lists,
        args.memory,
        args.memory_size,
        args.base_address,
        args.cmp,
        args.alignment,
        args.mt_args,
        args.local_offset);
}

void scan_local_memory_first_new(Arena **per_thread_result_arenas, scan_MatchList *per_thread_match_lists,
    void *mem, usize mem_size, uintptr_t base_address, scan_Comparison cmp, usize alignment, scan_MultithreadArgs mt_args,
    usize local_offset /* TODO: remove */)
{
    if (mem_size > mt_args.max_bytes_to_scan_per_thread) {
        printf("splitting\n");
        ASSERT(mem_size > 0);

        WorkQueue *wq = mt_args.work_queue;

        usize value_size = scan_val_type_get_size(cmp.value_type);
        usize sz = mem_size / 2;

        ASSERT(sz > 0);

        // TODO: make sure these are aligned
        scan_LocalScanArgs left_half_args = {};
        left_half_args.memory = mem;
        left_half_args.memory_size = sz; // TODO: these regions should overlap
        left_half_args.per_thread_result_arenas = per_thread_result_arenas;
        left_half_args.per_thread_match_lists = per_thread_match_lists;
        left_half_args.base_address = base_address;
        left_half_args.cmp = cmp;
        left_half_args.alignment = alignment;
        left_half_args.mt_args = mt_args;
        left_half_args.local_offset = local_offset;

        scan_LocalScanArgs right_half_args = left_half_args;
        right_half_args.memory = ptr_byte_offset(mem, sz);
        right_half_args.memory_size = mem_size - sz;
        right_half_args.local_offset = local_offset + sz;

        TaskID left_half_task = wq_push_task_as_child(wq, scan__local_memory_first_async, left_half_args, mt_args.parent_task);
        TaskID right_half_task = wq_push_task_as_child(wq, scan__local_memory_first_async, right_half_args, mt_args.parent_task);

        wq_wait_for_task(left_half_task);
        wq_wait_for_task(right_half_task);
    } else {
        Arena *arena = per_thread_result_arenas[wq_get_thread_index()];
        scan_MatchList *match_list = &per_thread_match_lists[wq_get_thread_index()];

        // TODO: now that we're providing the base address, we may as well store the
        // absolute address in scan_Match

        ASSERT(!scan_mode_is_comparative(cmp.mode));
        ASSERT(is_pow2(alignment)); // TODO: don't require pow2 alignment

        ASSERT_ALWAYS(is_aligned(base_address, alignment));

        usize type_size = scan_val_type_get_size(cmp.value_type);

        usize offset = 0;

        while ((offset + type_size) <= mem_size) {
            void *local_address = ptr_byte_offset(mem, offset);

            scan_Value value_at_address = scan__load_value_at_address(local_address, cmp.value_type);

            if (scan_cmp_match(value_at_address, cmp)) {
                scan_Match match = {};
                match.value = value_at_address;
                match.offset_from_base = offset + local_offset;

                // There is no history yet as this is the first scan.
                scan_MatchHistory match_history = {};
                match_history.first_value = match.value;
                match_history.previous_value = match.value;

                scan__push_match(arena, match_list, match, match_history);
            }

            offset += alignment;
        }
    }
}

scan_MatchList *scan_local_memory_first(Arena *arena, void *mem, usize mem_size, uintptr_t base_address,
    scan_Comparison cmp, usize alignment)
{
    // TODO: now that we're providing the base address, we may as well store the
    // absolute address in scan_Match

    ASSERT(!scan_mode_is_comparative(cmp.mode));
    ASSERT(is_pow2(alignment)); // TODO: don't require pow2 alignment

    ASSERT_ALWAYS(is_aligned(base_address, alignment));

    scan_MatchList *result = scan__alloc_match_list(arena);

    usize type_size = scan_val_type_get_size(cmp.value_type);

    usize offset = 0;

    while ((offset + type_size) <= mem_size) {
        void *local_address = ptr_byte_offset(mem, offset);

        scan_Value value_at_address = scan__load_value_at_address(local_address, cmp.value_type);

        if (scan_cmp_match(value_at_address, cmp)) {
            scan_Match match = {};
            match.value = value_at_address;
            match.offset_from_base = offset;

            // There is no history yet as this is the first scan.
            scan_MatchHistory match_history = {};
            match_history.first_value = match.value;
            match_history.previous_value = match.value;

            scan__push_match(arena, result, match, match_history);
        }

        offset += alignment;
    }

    return result;
}

scan_MatchList *scan_local_memory_next(Arena *arena, void *mem, usize mem_size, scan_Comparison cmp,
    scan_MatchList *prev_matches)
{
    scan_MatchList *result = scan__alloc_match_list(arena);

    for (scan_MatchList *match_list = prev_matches; match_list; match_list = list_next(match_list)) {
        for (usize i = 0; i < match_list->count; ++i) {
            scan_Match prev_match = match_list->match_data[i];
            scan_MatchHistory history = match_list->match_history_data[i];
            history.previous_value = prev_match.value;

            ASSERT(prev_match.offset_from_base < mem_size);

            void *address = ptr_byte_offset(mem, prev_match.offset_from_base);
            scan_Value current_value = scan__load_value_at_address(address, cmp.value_type);
            scan_Value prev_value = prev_match.value;

            if (scan_cmp_match_with_previous(current_value, cmp, prev_value)) {
                scan_Match new_match = {};
                new_match.value = current_value;
                new_match.offset_from_base = prev_match.offset_from_base;

                scan__push_match(arena, result, new_match, history);
            }
        }
    }

    return result;
}
