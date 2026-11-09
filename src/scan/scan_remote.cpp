#include "scan_remote.h"

// This is used to skip the huge memory regions that ASan allocates.
// TODO: can this be done in a better way?
// TODO: this is not necessary for 32 bit
#define SCAN_SKIP_REGION_SIZE_THRESHOLD TB(13)

/*
  TODO:
  - Report errors when in multithreaded scans
 */

static void scan__push_region_matches(Arena *arena, scan_Result *result, scan_MatchList *matches,
    uintptr_t region_address, usize region_size, proc_Object_Maybe containing_object)
{
    if (matches->count > 0) {
        scan_RegionMatches *region_matches = arena_alloc_struct(arena, scan_RegionMatches);
        region_matches->region_base_address = region_address;
        region_matches->region_size = region_size;
        region_matches->head = matches;
        region_matches->match_count_in_region = matches->total_match_count;
        region_matches->containing_object = containing_object;

        result->total_match_count += region_matches->match_count_in_region;

        sll_push_back(result, region_matches);

    } else {
        ASSERT(!matches->next);
    }
}

static scan_RegionMatches *scan__make_region_matches(Arena *arena,
    uintptr_t region_address, usize region_size, proc_Object_Maybe containing_object)
{
    scan_RegionMatches *result = arena_alloc_struct(arena, scan_RegionMatches);
    result->region_base_address = region_address;
    result->region_size = region_size;
    result->containing_object = containing_object;
    result->last = result;
    // result->head = arena_alloc_struct(arena, scan_MatchList);

    return result;
}

// TODO: rename and move elsewhere
struct scan_RemoteScanArgs {
    memmi_Process proc;
    scan_Settings settings;
    scan_Comparison cmp;
    scan_Progress progress;
    scan_MultithreadArgs mt_args;
    scan_RegionMatches *new_region_matches;
    scan_RegionMatches *previous_region_matches;

    proc_Status *errors_bitset_ptr;
    b32 *at_least_one_read_succeeded_ptr;

    uintptr_t region_address;
    usize region_size;
    proc_Object_Maybe object_containing_region;

    // Per-thread data, can be indexed into using wq_get_thread_index().
    Arena **per_thread_result_arena_array;
    scan_MatchList *per_thread_match_list_array; // Per thread, per region, TODO: remove
    scan_RegionMatches **per_thread_region_lists;
};

static Arena *scan__get_thread_result_arena(Arena **arenas)
{
    usize index = wq_get_thread_index();
    Arena *result = arenas[index];

    return result;
}

void scan__remote_first_single_region_async(TaskID task, void *data)
{
    // TODO: Clean up this function

    scan_RemoteScanArgs args = *(scan_RemoteScanArgs *)data;

    memmi_Process proc = args.proc;
    scan_Settings settings = args.settings;
    scan_Comparison cmp = args.cmp;
    scan_Progress progress = args.progress;
    scan_MultithreadArgs mt_args = args.mt_args;
    scan_RegionMatches *region = args.new_region_matches;
    Arena *result_arena = scan__get_thread_result_arena(args.per_thread_result_arena_array);

    // Pre-allocate a match list for each thread to use. Later we'll connect them in a linked list.
    usize thread_count = wq_get_thread_count(mt_args.work_queue);
    args.per_thread_match_list_array = arena_alloc_array(result_arena, scan_MatchList, thread_count);

    for (usize i = 0; i < thread_count; ++i) {
        scan_MatchList *match_list = &args.per_thread_match_list_array[i];
        match_list->first = match_list;
        match_list->last = match_list;
    }

    // For temporarily storing the memory of a region during the scan of it. We can't use a single
    // arena per thread, since we will create child tasks that scan this region, and when we're
    // waiting for them to finish, this thread might start executing some other task in the
    // meantime. If that task happens to be this same function but for another region, we will reset
    // the arena again, and the child tasks that we were waiting for will scan invalid memory.
    ScratchArena scan_scratch_arena = scratch_begin_using(SCRATCH_ARENA_CATEGORY_PER_SCAN);

    // TODO: don't zero out allocation, as we'll just overwrite it when reading in region
    u8 *region_memory = arena_alloc_array(scan_scratch_arena.arena, u8, region->region_size);

    memmi_ReadMemory read_mem_result = memmi_read_memory(
        proc, region_memory, region->region_base_address, region->region_size);

    if (read_mem_result.status != MEMMI_OK) {
        proc_Status status = proc_status_from_memmi_status(read_mem_result.status);
        atomic_or_s32(args.errors_bitset_ptr, status);
    } else {
        atomic_store_s32(args.at_least_one_read_succeeded_ptr, true);

        mt_args.parent_task = task;

        // TODO: use stride provided by caller
        // TODO: local memory scan just advances by stride each step.
        // Make sure we compensate if the region provided is not aligned
        // by aligning forwards the first step.

        scan_local_memory_first_new(
            args.per_thread_result_arena_array,
            args.per_thread_match_list_array,
            region_memory,
            read_mem_result.bytes_read,
            region->region_base_address,
            cmp,
            settings.required_alignment,
            mt_args,
            0);

        ASSERT(region->match_count_in_region == 0);
        ASSERT(!region->head);
    }

    // All threads have independently pushed to their respective match lists,
    // now we'll concatenate all the lists into one.
    #if 1
    for (usize i = 0; i < thread_count; ++i) {
        scan_MatchList *thread_match_list = &args.per_thread_match_list_array[i];
        scan_MatchList *next_thread_match_list =
            (i < (thread_count - 1))
            ? &args.per_thread_match_list_array[i + 1]
            : 0;

        if (next_thread_match_list) {
            thread_match_list->last->next = next_thread_match_list;
            thread_match_list->last = next_thread_match_list;
        }

        // The first node in the list should hold the sum of the  match count of all nodes in the list.
        scan_MatchList *first_match_list = &args.per_thread_match_list_array[0];
        if (thread_match_list != first_match_list) {
            first_match_list->total_match_count += thread_match_list->total_match_count;
        }
    }
    #else
    for (usize i = 0; 1 < thread_count; ++i) {
        scan_MatchList *thread_match_list = &args.per_thread_match_list_array[i];

        for (scan_MatchList *m = thread_match_list; m; m = list_next(m)) {
            for (usize j = 0; j < m->count; ++j) {
                scan__push_match(result_arena, &args.per_thread_match_list_array[0], m->match_data[j], m->match_history_data[j]);
            }

        }
    }
    #endif

    // The first node in the linked list will hold the total match count of all nodes in the list.
    scan_MatchList *first_match_list = &args.per_thread_match_list_array[0];
    region->match_count_in_region = first_match_list->total_match_count;
    region->head = first_match_list;

    // TODO: do we want to update progress bar with higher granularity than region?
    // Consider the region as scanned with regards to the progress bar whether it failed or succeeded.
    atomic_add_usize(progress.bytes_scanned_ptr, region->region_size);
}

void scan__remote_first_single_region_async_new(TaskID task, void *data)
{
    // TODO: Clean up this function
    scan_RemoteScanArgs args = *(scan_RemoteScanArgs *)data;
    memmi_Process proc = args.proc;
    scan_Settings settings = args.settings;
    scan_Comparison cmp = args.cmp;
    scan_Progress progress = args.progress;
    scan_MultithreadArgs mt_args = args.mt_args;
    Arena *result_arena = scan__get_thread_result_arena(args.per_thread_result_arena_array);

    if (args.region_size > args.mt_args.max_bytes_to_scan_per_thread) {
        printf("splitting\n");
        ASSERT(args.region_size > 0);

        WorkQueue *wq = mt_args.work_queue;

        usize value_size = scan_val_type_get_size(cmp.value_type);
        usize sz = args.region_size / 2;

        ASSERT(sz > 0);

        // TODO: make sure these are aligned
        scan_RemoteScanArgs left_half_args = args;
        left_half_args.region_size = sz; // TODO: these regions should overlap
        left_half_args.cmp = cmp;

        scan_RemoteScanArgs right_half_args = left_half_args;
        right_half_args.region_address = args.region_address + sz;
        right_half_args.region_size = args.region_size - sz;

        TaskID left_half_task = wq_push_task_as_child(wq, scan__remote_first_single_region_async_new,
            left_half_args, mt_args.parent_task);
        TaskID right_half_task = wq_push_task_as_child(wq, scan__remote_first_single_region_async_new,
            right_half_args, mt_args.parent_task);

        wq_wait_for_task(left_half_task);
        wq_wait_for_task(right_half_task);
    } else {
        ASSERT(is_pow2(args.settings.required_alignment));
        ASSERT(is_aligned_pow2(args.region_address, args.settings.required_alignment));

        // For temporarily storing the memory of a region during the scan of it. We can't use a single
        // arena per thread, since we will create child tasks that scan this region, and when we're
        // waiting for them to finish, this thread might start executing some other task in the
        // meantime. If that task happens to be this same function but for another region, we will reset
        // the arena again, and the child tasks that we were waiting for will scan invalid memory.
        ScratchArena scan_scratch_arena = scratch_begin_using(SCRATCH_ARENA_CATEGORY_PER_SCAN);

        // TODO: don't zero out allocation, as we'll just overwrite it when reading in region
        u8 *region_memory = arena_alloc_array(scan_scratch_arena.arena, u8, args.region_size);

        memmi_ReadMemory read_mem_result = memmi_read_memory(
            proc, region_memory, args.region_address, args.region_size);

        if (read_mem_result.status != MEMMI_OK) {
            proc_Status status = proc_status_from_memmi_status(read_mem_result.status);
            atomic_or_s32(args.errors_bitset_ptr, status);
        } else {
            atomic_store_s32(args.at_least_one_read_succeeded_ptr, true);

            // TODO: use stride provided by caller
            // TODO: local memory scan just advances by stride each step.
            // Make sure we compensate if the region provided is not aligned
            // by aligning forwards the first step.
            scan_MatchList *matches_in_region = scan_local_memory_first(
                result_arena,
                region_memory,
                read_mem_result.bytes_read,
                args.region_address,
                cmp,
                settings.required_alignment);

            if (matches_in_region->total_match_count > 0) {
                scan_RegionMatches *region = scan__make_region_matches(
                    result_arena, args.region_address, args.region_size, args.object_containing_region);
                region->head = matches_in_region;
                region->match_count_in_region = matches_in_region->total_match_count;

                scan_RegionMatches **region_list_head = &args.per_thread_region_lists[wq_get_thread_index()];

                ASSERT(*region_list_head != region);

                if (*region_list_head) {
                    region->next = *region_list_head;
                    region->last = (*region_list_head)->last;
                }

                *region_list_head = region;
            }
        }

        atomic_add_usize(progress.bytes_scanned_ptr, args.region_size);
    }
}

void scan__remote_next_single_region_async(TaskID task, void *data)
{
    scan_RemoteScanArgs args = *(scan_RemoteScanArgs *)data;

    memmi_Process proc = args.proc;
    scan_Settings settings = args.settings;
    scan_Comparison cmp = args.cmp;
    scan_Progress progress = args.progress;
    scan_MultithreadArgs mt_args = args.mt_args;
    scan_RegionMatches *new_region = args.new_region_matches;
    scan_RegionMatches *prev_region = args.previous_region_matches;
    Arena *result_arena = scan__get_thread_result_arena(args.per_thread_result_arena_array);

    ASSERT(prev_region->region_base_address == new_region->region_base_address);
    ASSERT(prev_region->region_size == new_region->region_size);

    // For temporarily storing the memory of a region during the scan of it. See the comment in
    // scan__remote_first_single_region_async for why we can't reuse the same thread-local arena
    // each time we scan a region.
    ScratchArena scan_scratch_arena = scratch_begin_using(SCRATCH_ARENA_CATEGORY_PER_SCAN);

    // TODO: don't zero out allocation, as we'll just overwrite it when reading in region
    u8 *region_memory = arena_alloc_array(scan_scratch_arena.arena, u8, new_region->region_size);

    memmi_ReadMemory read_mem_result = memmi_read_memory(
        proc, region_memory, new_region->region_base_address, new_region->region_size);

    if (read_mem_result.status != MEMMI_OK) {
        proc_Status status = proc_status_from_memmi_status(read_mem_result.status);
        atomic_or_s32(args.errors_bitset_ptr, status);
    } else {
        atomic_store_s32(args.at_least_one_read_succeeded_ptr, true);

        scan_MatchList *prev_matches_in_region = prev_region->head;

        scan_MatchList *new_matches_in_region =
            scan_local_memory_next(result_arena, region_memory,
                read_mem_result.bytes_read, cmp, prev_matches_in_region);

        ASSERT(new_region->match_count_in_region == 0);
        ASSERT(!new_region->head);
        ASSERT(new_matches_in_region->total_match_count <= prev_matches_in_region->total_match_count);

        new_region->head = new_matches_in_region;
        new_region->match_count_in_region = new_matches_in_region->total_match_count;
    }

    // TODO: do we want to update progress bar with higher granularity than region?
    // Consider the region as scanned with regards to the progress bar whether it failed or succeeded.
    atomic_add_usize(progress.bytes_scanned_ptr, new_region->region_size);
}

scan_Result scan_remote_memory_first_new(Arena **per_thread_result_arenas, memmi_Process proc,
    scan_Settings settings, scan_Comparison cmp, scan_Progress progress, scan_MultithreadArgs mt_args)
{
    #if DEBUG_BUILD
    {
        for (usize i = 0; i < wq_get_thread_count(mt_args.work_queue); ++i) {
            ASSERT((arena_get_memory_usage(per_thread_result_arenas[i]) == 0)
                && "All arenas should have been reset before call to this function.");
        }
    }
    #endif

    u64 t1 = get_monotonic_clock_ns();

    Arena *result_arena = scan__get_thread_result_arena(per_thread_result_arenas);

    scan_Result result = {};
    result.value_type = cmp.value_type;
    result.scan_mode = cmp.mode;
    result.scanned_process = proc;
    result.process_objects = proc_get_loaded_objects(result_arena, proc);

    ASSERT(result.process_objects.status == PROC_STATUS_OK);

    usize value_type_size = scan_val_type_get_size(cmp.value_type);

    // For allocations that need to live for entire lifetime of function.
    ScratchArena scratch = scratch_begin();
    memmi_Allocator temp_allocator = proc_memmi_allocator_from_arena(scratch.arena);

    memmi_MemoryRegions memory_regions = memmi_get_process_memory_regions(proc, temp_allocator);

    b32 at_least_one_read_succeeded = false;
    usize total_memory_region_size = 0;

    usize thread_count = wq_get_thread_count(mt_args.work_queue);
    scan_RegionMatches **per_thread_region_lists = arena_alloc_array(result_arena, scan_RegionMatches *, thread_count);

    if (memory_regions.status != MEMMI_OK) {
        result.errors = proc_status_from_memmi_status(memory_regions.status);
    } else {
        // Calculate total size of memory regions so that progress bar can track scan progress.
        {
            for (usize i = 0; i < memory_regions.count; ++i) {
                // TODO: this check is repeated further down
                memmi_MemoryRegion region = memory_regions.data[i];

                b32 should_skip_region =
                    !(region.permissions & MEMMI_REGION_PERMISSION_READ)
                    || (region.kind == MEMMI_REGION_MAPPED_FILE)
                    || (region.size >= SCAN_SKIP_REGION_SIZE_THRESHOLD);

                if (!should_skip_region) {
                    total_memory_region_size += region.size;
                }
            }

            atomic_store_usize(progress.bytes_to_scan_ptr, total_memory_region_size);
        }

        TaskID root_task = wq_create_task(mt_args.work_queue, wq_null_function, wq_null_arg);

        for (usize i = 0; i < memory_regions.count; ++i) {
            memmi_MemoryRegion region = memory_regions.data[i];

            // TODO: allow user to scan through memory mapped files
            b32 should_skip_region =
                !(region.permissions & MEMMI_REGION_PERMISSION_READ)
                || (region.kind == MEMMI_REGION_MAPPED_FILE)
                || (region.size >= SCAN_SKIP_REGION_SIZE_THRESHOLD);

            if (!should_skip_region) {
                proc_Object_Maybe containing_object =
                    proc_get_object_address_is_contained_in(
                        result.process_objects, region.base_address, value_type_size);

                scan_RemoteScanArgs args = {};
                args.proc = proc;
                args.settings = settings;
                args.cmp = cmp;
                args.progress = progress;
                args.mt_args = mt_args;
                args.per_thread_result_arena_array = per_thread_result_arenas;
                args.at_least_one_read_succeeded_ptr = &at_least_one_read_succeeded;
                args.errors_bitset_ptr = &result.errors;
                args.region_address = region.base_address;
                args.region_size = region.size;
                args.object_containing_region = containing_object;
                args.per_thread_region_lists = per_thread_region_lists;

                wq_push_task_as_child(mt_args.work_queue, scan__remote_first_single_region_async_new, args, root_task);
            }
        }

        // Wait for all regions to finish scanning.
        wq_run_task(root_task);
        wq_wait_for_task(root_task);

        // All threads have pushed their region results to their own lists, now we'll concatenate
        // those lists.
        for (usize i = 0; i < thread_count; ++i) {
            scan_RegionMatches *region = per_thread_region_lists[i];

            if (region) {
                if (result.last) {
                    ASSERT(!result.last->next);
                    result.last->next = region;
                } else {
                    ASSERT(!result.first);

                    result.first = region;
                }

                result.last = region->last;

                for (; region; region = list_next(region)) {
                    // TODO: make it so that we don't have to walk the list again, let each thread keep a count
                    result.total_match_count += region->match_count_in_region;
                }
            }
        }
    }

    if (at_least_one_read_succeeded && !(result.errors & PROC_STATUS_NO_SUCH_PROCESS)) {
        // Some reads failing is to be expected, for example a region could be unmapped during the
        // scan. As long as we managed to read at least one region, we'll count it as a success.
        result.errors = PROC_STATUS_OK;
    }

    u64 t2 = get_monotonic_clock_ns();

    u64 elapsed_ms = (t2 - t1) / MILLION(1);

    printf("Scanned %zu bytes in %" PRIu64 " milliseconds\n", total_memory_region_size, elapsed_ms);

    return result;
}

scan_Result scan_remote_memory_first(Arena *arena, memmi_Process proc, scan_Settings settings,
    scan_Comparison cmp, scan_Progress progress)
{
    u64 t1 = get_monotonic_clock_ns();

    scan_Result result = {};
    result.value_type = cmp.value_type;
    result.scan_mode = cmp.mode;
    result.scanned_process = proc;
    result.process_objects = proc_get_loaded_objects(arena, proc);

    ASSERT(result.process_objects.status == PROC_STATUS_OK);

    usize value_type_size = scan_val_type_get_size(cmp.value_type);

    // For temporarily storing the memory of a region during the scan of it.
    ScratchArena scan_scratch_arena = scratch_begin();

    // For allocations that need to live for entire lifetime of function.
    ScratchArena scratch = scratch_begin();
    memmi_Allocator temp_allocator = proc_memmi_allocator_from_arena(scratch.arena);

    memmi_MemoryRegions memory_regions = memmi_get_process_memory_regions(proc, temp_allocator);

    b32 at_least_one_read_succeeded = false;

    if (memory_regions.status != MEMMI_OK) {
        result.errors = proc_status_from_memmi_status(memory_regions.status);
    } else {
        // Calculate total size of memory regions so that progress bar can track scan progress.
        usize total_memory_region_size = 0;

        for (usize i = 0; i < memory_regions.count; ++i) {
            // TODO: this check is repeated further down
            memmi_MemoryRegion region = memory_regions.data[i];

            b32 should_skip_region =
                !(region.permissions & MEMMI_REGION_PERMISSION_READ)
                || (region.kind == MEMMI_REGION_MAPPED_FILE)
                || (region.size >= SCAN_SKIP_REGION_SIZE_THRESHOLD);

            if (!should_skip_region) {
                total_memory_region_size += region.size;
            }
        }

        atomic_store_usize(progress.bytes_to_scan_ptr, total_memory_region_size);

        for (usize i = 0; i < memory_regions.count; ++i) {
            memmi_MemoryRegion region = memory_regions.data[i];

            // TODO: allow user to scan through memory mapped files
            b32 should_skip_region =
                !(region.permissions & MEMMI_REGION_PERMISSION_READ)
                || (region.kind == MEMMI_REGION_MAPPED_FILE)
                || (region.size >= SCAN_SKIP_REGION_SIZE_THRESHOLD);

            if (!should_skip_region) {
                // Reset the arena before reading each regions memory, as we only need it during the
                // scan of that region.
                arena_reset(scan_scratch_arena.arena);

                // TODO: don't zero out allocation
                char *region_memory = arena_alloc_array(scan_scratch_arena.arena, char, region.size);

                memmi_ReadMemory read_mem_result = memmi_read_memory(
                    proc, region_memory, region.base_address, region.size);

                if (read_mem_result.status != MEMMI_OK) {
                    set_flag(&result.errors, proc_status_from_memmi_status(read_mem_result.status));

                    if (read_mem_result.status == MEMMI_NO_SUCH_PROCESS) {
                        break;
                    }
                } else {
                    at_least_one_read_succeeded = true;

                    // TODO: use stride provided by caller
                    // TODO: local memory scan just advances by stride each step.
                    // Make sure we compensate if the region provided is not aligned
                    // by aligning forwards the first step.
                    scan_MatchList *matches =
                        scan_local_memory_first(arena, region_memory,
                            read_mem_result.bytes_read, region.base_address,
                            cmp, settings.required_alignment);

                    ASSERT(matches);

                    proc_Object_Maybe containing_object =
                        proc_get_object_address_is_contained_in(
                            result.process_objects, region.base_address, value_type_size);

                    scan__push_region_matches(arena, &result, matches, region.base_address,
                        region.size, containing_object);
                }

                // TODO: do we want to update progress bar with higher granularity than region?
                // Consider the region as scanned with regards to the progress bar whether it failed or succeeded.
                atomic_add_usize(progress.bytes_scanned_ptr, region.size);
            }
        }

        u64 t2 = get_monotonic_clock_ns();

        u64 elapsed_ms = (t2 - t1) / MILLION(1);
        printf("Scanned %zu bytes in %" PRIu64 " milliseconds\n", total_memory_region_size, elapsed_ms);
    }

    if (at_least_one_read_succeeded && !(result.errors & PROC_STATUS_NO_SUCH_PROCESS)) {
        // Some reads failing is to be expected, for example a region could be unmapped during the
        // scan. As long as we managed to read at least one region, we'll count it as a success.
        result.errors = PROC_STATUS_OK;
    }

    return result;
}

/* TODO: Split memory regions into separate scan tasks if too large. This has some benefits:
 * - Next scan workload will be more evenly distributed, instead of rescanning an entire region per thread
 * - We don't have to multithread local scans
 * - Reading region into memory will always fit, provided we make the scan scratch arena size match the max size per thread
 */
scan_Result scan_remote_memory_next_new(Arena **per_thread_result_arenas, memmi_Process proc, scan_Settings settings,
    scan_Comparison cmp, scan_Result prev_results, scan_Progress progress, scan_MultithreadArgs mt_args)
{
    u64 t1 = get_monotonic_clock_ns();

    // For temporarily storing the memory of a region during the scan of it.
    ScratchArena scan_scratch_arena = scratch_begin();

    Arena *result_arena = scan__get_thread_result_arena(per_thread_result_arenas);

    scan_Result result = {};
    result.value_type = cmp.value_type;
    result.scan_mode = cmp.mode;
    result.scanned_process = proc;
    result.process_objects = proc_get_loaded_objects(result_arena, proc);

    b32 at_least_one_read_succeeded = false;

    // Get the total number of bytes we need to scan so the progress bar can show our progress.
    usize bytes_to_scan = 0;
    for (scan_RegionMatches *region = list_head(&prev_results); region; region = list_next(region)) {
        bytes_to_scan += region->region_size;
    }

    atomic_store_usize(progress.bytes_to_scan_ptr, bytes_to_scan);

    TaskID root_task = wq_create_task(mt_args.work_queue, wq_null_function, wq_null_arg);

    for (scan_RegionMatches *prev_region = list_head(&prev_results); prev_region; prev_region = list_next(prev_region)) {
        proc_Object_Maybe containing_object = {};

        if (prev_region->containing_object.ok) {
            // proj_Object contains a string allocated in the previous result arena, so we'll have to copy it.
            containing_object = SOME(proc_object_copy(result_arena, prev_region->containing_object.value));
        }

        scan_RegionMatches *new_region = scan__make_region_matches(
            result_arena, prev_region->region_base_address, prev_region->region_size, containing_object);
        sll_push_back(&result, new_region);

        scan_RemoteScanArgs args = {};
        args.proc = proc;
        args.settings = settings;
        args.cmp = cmp;
        args.progress = progress;
        args.mt_args = mt_args;
        args.new_region_matches = new_region;
        args.previous_region_matches = prev_region;
        args.per_thread_result_arena_array = per_thread_result_arenas;
        args.at_least_one_read_succeeded_ptr = &at_least_one_read_succeeded;
        args.errors_bitset_ptr = &result.errors;

        wq_push_task_as_child(mt_args.work_queue, scan__remote_next_single_region_async, args, root_task);
    }

    wq_run_task(root_task);
    wq_wait_for_task(root_task);

    for (scan_RegionMatches *r = result.first; r; r = list_next(r)) {
        result.total_match_count += r->match_count_in_region;
    }

    if (at_least_one_read_succeeded && !(result.errors & PROC_STATUS_NO_SUCH_PROCESS)) {
        // Some reads failing is to be expected, for example a region could be unmapped during the
        // scan. As long as we managed to read at least one region, we'll count it as a success.
        result.errors = PROC_STATUS_OK;
    }

    u64 t2 = get_monotonic_clock_ns();

    u64 elapsed_ms = (t2 - t1) / MILLION(1);
    printf("Scanned %zu bytes in %" PRIu64 " milliseconds\n", bytes_to_scan, elapsed_ms);

    return result;
}

scan_Result scan_remote_memory_next(Arena *arena, memmi_Process proc, scan_Settings settings,
    scan_Comparison cmp, scan_Result prev_results, scan_Progress progress)
{
    u64 t1 = get_monotonic_clock_ns();

    scan_Result result = {};
    result.value_type = cmp.value_type;
    result.scan_mode = cmp.mode;
    result.scanned_process = proc;
    result.process_objects = proc_get_loaded_objects(arena, proc);

    // For temporarily storing the memory of a region during the scan of it.
    ScratchArena scan_scratch_arena = scratch_begin();

    b32 at_least_one_read_succeeded = false;

    // Get the total number of bytes we need to scan so the progress bar can show our progress.
    usize bytes_to_scan = 0;
    for (scan_RegionMatches *region = list_head(&prev_results); region; region = list_next(region)) {
        bytes_to_scan += region->region_size;
    }

    atomic_store_usize(progress.bytes_to_scan_ptr, bytes_to_scan);

    // TODO: rename match_list
    for (scan_RegionMatches *match_list = list_head(&prev_results); match_list; match_list = list_next(match_list)) {
        // Reset the arena before reading each regions memory, as we only need it during the scan of
        // that region.
        arena_reset(scan_scratch_arena.arena);

        char *region_memory = arena_alloc_array(scan_scratch_arena.arena, char, match_list->region_size);

        memmi_ReadMemory read_mem_result =
            memmi_read_memory(proc, region_memory, match_list->region_base_address, match_list->region_size);

        if (read_mem_result.status != MEMMI_OK) {
            set_flag(&result.errors, proc_status_from_memmi_status(read_mem_result.status));

            if (read_mem_result.status == MEMMI_NO_SUCH_PROCESS) {
                break;
            }
        } else {
            scan_MatchList *matches = scan_local_memory_next(arena, region_memory,
                read_mem_result.bytes_read, cmp, match_list->head);
            ASSERT(matches);

            scan__push_region_matches(arena, &result, matches, match_list->region_base_address,
                match_list->region_size, match_list->containing_object);
        }

        atomic_add_usize(progress.bytes_scanned_ptr, match_list->region_size);
    }

    u64 t2 = get_monotonic_clock_ns();

    u64 elapsed_ms = (t2 - t1) / MILLION(1);
    printf("Scanned %zu bytes in %" PRIu64 " milliseconds\n", bytes_to_scan, elapsed_ms);

    // TODO: reduce code duplication between this and first scan
    if (at_least_one_read_succeeded && !(result.errors & PROC_STATUS_NO_SUCH_PROCESS)) {
        // Some reads failing is to be expected, for example a region could be unmapped during the
        // scan. As long as we managed to read at least one region, we'll count it as a success.
        result.errors = PROC_STATUS_OK;
    }

    return result;
}

scan_Value_Maybe scan_remote_read_single_value(memmi_Process proc, scan_ValueType value_type, uintptr_t address)
{
    scan_Value_Maybe result = {};

    usize size = scan_val_type_get_size(value_type);

    memmi_ReadMemory read_mem_result = memmi_read_memory(proc, &result.value, address, size);
    result.ok = (read_mem_result.status == MEMMI_OK);

    return result;
}
