#include "scan_remote.h"

// This is used to skip the huge memory regions that ASan allocates.
// TODO: can this be done in a better way?
// TODO: this is not necessary for 32 bit
#define SCAN_SKIP_REGION_SIZE_THRESHOLD TB(13)
#define SCAN_MAX_RECURSION_DEPTH 8

// TODO: this struct is pretty similar to scan_Result
struct scan_RemoteScanThreadContext {
    Arena *result_arena;

    struct {
        scan_RegionMatches *first;
        scan_RegionMatches *last;
        usize list_node_count;
    } region_matches;

    usize match_count;

    // The sum of memory region sizes in which the thread found at least one match.
    usize matching_regions_size_sum;
};

struct scan_RemoteScanArgs {
    memmi_Process proc;
    scan_Settings settings;
    scan_Comparison cmp;
    scan_Progress progress;
    scan_MultithreadArgs mt_args;

    proc_Status *errors_bitset_ptr;
    b32 *at_least_one_read_succeeded_ptr;

    uintptr_t region_address;
    usize region_size;
    proc_Object_Maybe object_containing_region;
    scan_RegionMatches *previous_region_matches;

    usize recursion_depth;

    // Per-thread data, can be indexed into using wq_get_thread_index().
    scan_RemoteScanThreadContext *thread_contexts;
};

static scan_RemoteScanThreadContext *scan__alloc_thread_contexts(Arena **per_thread_arenas, usize thread_count)
{
    ASSERT(thread_count > 0);

    // Arbitrarily use the first threads arena to allocate the array.
    scan_RemoteScanThreadContext *result = arena_alloc_array(
        *per_thread_arenas, scan_RemoteScanThreadContext, thread_count);

    for (usize i = 0; i < thread_count; ++i) {
        scan_RemoteScanThreadContext *context = &result[i];
        context->result_arena = per_thread_arenas[i];
    }

    return result;
}

static scan_RemoteScanThreadContext *scan__get_thread_context(scan_RemoteScanThreadContext *contexts)
{
    usize index = wq_get_thread_index();

    scan_RemoteScanThreadContext *result = &contexts[index];

    return result;
}

static void scan__push_matches_to_thread_context(scan_RemoteScanThreadContext *context, scan_MatchList *matches,
    uintptr_t region_address, usize region_size, proc_Object_Maybe object_containing_region)
{
    if (matches->total_match_count > 0) {
        scan_RegionMatches *region = arena_alloc_struct(context->result_arena, scan_RegionMatches);
        region->region_base_address = region_address;
        region->region_size = region_size;
        region->containing_object = object_containing_region;
        region->last = region;
        region->head = matches;
        region->match_count_in_region = matches->total_match_count;

        sll_push_back(&context->region_matches, region);

        context->match_count += region->match_count_in_region;
        context->matching_regions_size_sum += region_size;
    }
}

static void scan__collect_thread_results(scan_Result *dst, scan_RemoteScanThreadContext *contexts, usize thread_count)
{
    for (usize i = 0; i < thread_count; ++i) {
        scan_RemoteScanThreadContext *thread_context = &contexts[i];

        sll_concat(dst, &thread_context->region_matches);

        dst->total_match_count += thread_context->match_count;
        dst->matching_regions_size_sum += thread_context->matching_regions_size_sum;
    }
}

static proc_MemoryRegionList scan__get_memory_regions_to_scan(Arena *arena, memmi_Process proc)
{
    // TODO: allow user to customize which regions are considered
    proc_RegionPermission required_permissions = PROC_PERMISSION_READ;

    proc_MemoryRegionList result = proc_get_memory_regions_where(
        arena, proc, required_permissions, false, SCAN_SKIP_REGION_SIZE_THRESHOLD);

    return result;
}

static proc_Status scan__interpret_scan_result_status(proc_Status status, b32 at_least_one_read_succeeded)
{
    proc_Status result = status;

    if (at_least_one_read_succeeded && !(result & PROC_STATUS_NO_SUCH_PROCESS)) {
        // Some reads failing is to be expected, for example a region could be unmapped during the
        // scan. As long as we managed to read at least one region, we'll count it as a success.
        result = PROC_STATUS_OK;
    }

    return result;
}

static scan_RemoteScanArgs scan__make_remote_scan_args(
    memmi_Process proc,
    uintptr_t region_address,
    usize region_size,
    proc_Object_Maybe object_containing_region,
    scan_Comparison cmp,
    scan_Settings settings,
    scan_MultithreadArgs mt_args,
    scan_RemoteScanThreadContext *thread_contexts,
    scan_Progress progress,
    b32 *at_least_one_read_succeeded_ptr,
    proc_Status *errors_bitset_ptr,
    scan_RegionMatches *previous_region_matches)
{
    scan_RemoteScanArgs result = {};
    result.proc                            = proc;
    result.settings                        = settings;
    result.cmp                             = cmp;
    result.progress                        = progress;
    result.mt_args                         = mt_args;
    result.thread_contexts                 = thread_contexts;
    result.at_least_one_read_succeeded_ptr = at_least_one_read_succeeded_ptr;
    result.errors_bitset_ptr               = errors_bitset_ptr;
    result.region_address                  = region_address;
    result.region_size                     = region_size;
    result.object_containing_region        = object_containing_region;
    result.previous_region_matches         = previous_region_matches;

    return result;
}

void scan__remote_first_single_region_async(TaskID task, void *data)
{
    scan_RemoteScanArgs args     = *(scan_RemoteScanArgs *)data;

    memmi_Process proc           = args.proc;
    scan_Settings settings       = args.settings;
    scan_Comparison cmp          = args.cmp;
    scan_Progress progress       = args.progress;
    scan_MultithreadArgs mt_args = args.mt_args;

    scan_RemoteScanThreadContext *thread_context = scan__get_thread_context(args.thread_contexts);

    if ((args.region_size > args.mt_args.max_bytes_to_scan_per_thread)
        // Recursive case: region is too large, split it in half and scan those recursively.

        && (args.recursion_depth < SCAN_MAX_RECURSION_DEPTH)) {
        ASSERT(args.region_size > 0);

        ++args.recursion_depth;

        WorkQueue *wq = mt_args.work_queue;

        usize sz = align_pow2(args.region_size / 2, settings.required_alignment);

        ASSERT(sz > 0);

        // TODO: these regions should overlap so values spanning both two regions can be found
        // TODO: function for splitting
        scan_RemoteScanArgs left_half_args = args;
        left_half_args.region_size = sz;
        left_half_args.cmp = cmp;

        scan_RemoteScanArgs right_half_args = left_half_args;
        right_half_args.region_address = args.region_address + sz;
        right_half_args.region_size = args.region_size - sz;

        TaskID left_half_task = wq_push_task_as_child(wq, scan__remote_first_single_region_async,
            left_half_args, task);
        TaskID right_half_task = wq_push_task_as_child(wq, scan__remote_first_single_region_async,
            right_half_args, task);

        wq_wait_for_task(left_half_task);
        wq_wait_for_task(right_half_task);
    } else {
        // Base case: region is small enough, scan it as normal.

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

            // TODO: local memory scan just advances by stride each step.
            // Make sure we compensate if the region provided is not aligned
            // by aligning forwards the first step.
            scan_MatchList *matches_in_region = scan_local_memory_first(
                thread_context->result_arena,
                region_memory,
                read_mem_result.bytes_read,
                args.region_address,
                cmp,
                settings.required_alignment);

            scan__push_matches_to_thread_context(thread_context, matches_in_region, args.region_address,
                args.region_size, args.object_containing_region);
        }

        atomic_add_usize(progress.bytes_scanned_ptr, args.region_size);
    }
}

void scan__remote_next_single_region_async(TaskID, void *data)
{
    scan_RemoteScanArgs args = *(scan_RemoteScanArgs *)data;

    memmi_Process proc              = args.proc;
    scan_Comparison cmp             = args.cmp;
    scan_Progress progress          = args.progress;
    scan_RegionMatches *prev_region = args.previous_region_matches;

    ASSERT(args.region_address > 0);
    ASSERT(args.region_size > 0);
    ASSERT(prev_region->region_base_address == args.region_address);
    ASSERT(prev_region->region_size == args.region_size);

    scan_RemoteScanThreadContext *thread_context = scan__get_thread_context(args.thread_contexts);

    // For temporarily storing the memory of a region during the scan of it. See the comment in
    // scan__remote_first_single_region_async for why we can't reuse the same thread-local arena
    // each time we scan a region.
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

        scan_MatchList *prev_matches_in_region = prev_region->head;

        scan_MatchList *new_matches_in_region =
            scan_local_memory_next(thread_context->result_arena, region_memory,
                read_mem_result.bytes_read, cmp, prev_matches_in_region);

        ASSERT(new_matches_in_region->total_match_count <= prev_matches_in_region->total_match_count);

        scan__push_matches_to_thread_context(thread_context, new_matches_in_region, args.region_address,
            args.region_size, args.object_containing_region);
    }

    // Consider the region as scanned with regards to the progress bar whether it failed or succeeded.
    atomic_add_usize(progress.bytes_scanned_ptr, args.region_size);
}

scan_Result scan_remote_memory_first(Arena **per_thread_result_arenas, memmi_Process proc,
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

    // For allocations that need to live for entire lifetime of function.
    ScratchArena scratch = scratch_begin();

    usize thread_count = wq_get_thread_count(mt_args.work_queue);
    scan_RemoteScanThreadContext *thread_context_array =
        scan__alloc_thread_contexts(per_thread_result_arenas, thread_count);

    Arena *result_arena = scan__get_thread_context(thread_context_array)->result_arena;

    scan_Result result = {};
    result.value_type = cmp.value_type;
    result.scan_mode = cmp.mode;
    result.scanned_process = proc;
    result.process_objects = proc_get_loaded_objects(result_arena, proc);

    ASSERT(result.process_objects.status == PROC_STATUS_OK);

    proc_MemoryRegionList memory_regions = scan__get_memory_regions_to_scan(scratch.arena, proc);
    atomic_store_usize(progress.bytes_to_scan_ptr, memory_regions.total_regions_size);

    b32 at_least_one_read_succeeded = false;

    if (memory_regions.status != PROC_STATUS_OK) {
        result.errors = memory_regions.status;
    } else {
        TaskID root_task = wq_create_task(mt_args.work_queue, wq_null_function, wq_null_arg);

        for (usize i = 0; i < memory_regions.count; ++i) {
            proc_MemoryRegion region = memory_regions.data[i];

            ASSERT(region.permissions & PROC_PERMISSION_READ);
            ASSERT(region.kind != PROC_REGION_KIND_MAPPED_FILE);
            ASSERT(region.size <= SCAN_SKIP_REGION_SIZE_THRESHOLD);

            usize value_type_size = scan_val_type_get_size(cmp.value_type);
            proc_Object_Maybe object_containing_region =
                proc_get_object_address_is_contained_in(
                    result.process_objects, region.address, value_type_size);

            scan_RemoteScanArgs args = scan__make_remote_scan_args(
                proc, region.address, region.size, object_containing_region, cmp, settings, mt_args,
                thread_context_array, progress, &at_least_one_read_succeeded, &result.errors, 0);

            wq_push_task_as_child(mt_args.work_queue, scan__remote_first_single_region_async, args, root_task);
        }

        wq_run_task(root_task);
        wq_wait_for_task(root_task);

        scan__collect_thread_results(&result, thread_context_array, thread_count);
    }

    result.errors = scan__interpret_scan_result_status(result.errors, at_least_one_read_succeeded);

    u64 t2 = get_monotonic_clock_ns();

    u64 elapsed_ms = (t2 - t1) / MILLION(1);

    printf("Scanned %zu bytes in %" PRIu64 " milliseconds\n", memory_regions.total_regions_size, elapsed_ms);

    return result;
}

scan_Result scan_remote_memory_next(Arena **per_thread_result_arenas, memmi_Process proc, scan_Settings settings,
    scan_Comparison cmp, scan_Result prev_results, scan_Progress progress, scan_MultithreadArgs mt_args)
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

    // For temporarily storing the memory of a region during the scan of it.
    ScratchArena scan_scratch_arena = scratch_begin();

    usize thread_count = wq_get_thread_count(mt_args.work_queue);
    scan_RemoteScanThreadContext *thread_context_array =
        scan__alloc_thread_contexts(per_thread_result_arenas, thread_count);

    Arena *result_arena = scan__get_thread_context(thread_context_array)->result_arena;

    scan_Result result = {};
    result.value_type = cmp.value_type;
    result.scan_mode = cmp.mode;
    result.scanned_process = proc;
    result.process_objects = proc_get_loaded_objects(result_arena, proc);

    b32 at_least_one_read_succeeded = false;

    atomic_store_usize(progress.bytes_to_scan_ptr, prev_results.matching_regions_size_sum);

    TaskID root_task = wq_create_task(mt_args.work_queue, wq_null_function, wq_null_arg);

    for (scan_RegionMatches *prev_region = list_head(&prev_results); prev_region; prev_region = list_next(prev_region)) {
        proc_Object_Maybe object_containing_region = {};

        if (prev_region->containing_object.ok) {
            // proj_Object contains a string allocated in the previous result arena, so we'll have to copy it.
            object_containing_region = SOME(proc_object_copy(result_arena, prev_region->containing_object.value));
        }

        scan_RemoteScanArgs args = scan__make_remote_scan_args(
            proc, prev_region->region_base_address, prev_region->region_size,
            object_containing_region, cmp, settings, mt_args, thread_context_array,
            progress, &at_least_one_read_succeeded, &result.errors, prev_region);

        // NOTE: Since the first scan divided the regions into manageable sizes, the next scan
        // workload should be fairly evenly distributed if we simply scan through one region per thread.
        wq_push_task_as_child(mt_args.work_queue, scan__remote_next_single_region_async, args, root_task);
    }

    wq_run_task(root_task);
    wq_wait_for_task(root_task);

    scan__collect_thread_results(&result, thread_context_array, thread_count);

    result.errors = scan__interpret_scan_result_status(result.errors, at_least_one_read_succeeded);

    u64 t2 = get_monotonic_clock_ns();

    u64 elapsed_ms = (t2 - t1) / MILLION(1);
    printf("Scanned %zu bytes in %" PRIu64 " milliseconds\n", prev_results.matching_regions_size_sum, elapsed_ms);

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

////////////////////////////////////////////////////////////////////////////////
// Single-threaded scans, reference implementation
////////////////////////////////////////////////////////////////////////////////
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

scan_Result scan_remote_memory_first_single_threaded(Arena *arena, memmi_Process proc, scan_Settings settings,
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

scan_Result scan_remote_memory_next_single_threaded(Arena *arena, memmi_Process proc, scan_Settings settings,
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
