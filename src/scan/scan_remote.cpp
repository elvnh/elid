#include "scan_remote.h"

// This is used to skip the huge memory regions that ASan allocates.
// TODO: can this be done in a better way?
// TODO: this is not necessary for 32 bit
#define SCAN_SKIP_REGION_SIZE_THRESHOLD TB(13)

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

scan_Result scan_remote_memory_first(Arena *arena, memmi_Process proc, scan_Settings settings, scan_Comparison cmp)
{
    scan_Result result = {};
    result.value_type = cmp.value_type;
    result.scan_mode = cmp.mode;
    result.process_objects = proc_get_loaded_objects(arena, proc);

    ASSERT(result.process_objects.status == PROC_STATUS_OK);

    usize value_type_size = scan_val_type_get_size(cmp.value_type);

    // For temporarily storing the memory of a region during the scan of it.
    Arena *scan_scratch_arena = thread_ctx_get()->per_scan_scratch_arena;
    memmi_Allocator scan_scratch_allocator = proc_memmi_allocator_from_arena(scan_scratch_arena);

    // For allocations that need to live for entire lifetime of function.
    Temp temp = scratch_begin();
    memmi_Allocator temp_allocator = proc_memmi_allocator_from_arena(temp.arena);

    memmi_MemoryRegions memory_regions = memmi_get_process_memory_regions(proc, temp_allocator);

    b32 at_least_one_read_succeeded = false;

    if (memory_regions.status != MEMMI_OK) {
        result.errors = proc_status_from_memmi_status(memory_regions.status);
    } else {
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
                arena_reset(scan_scratch_arena);

                memmi_ReadMemory read_mem_result = memmi_read_memory(
                    proc, region.base_address, region.size, scan_scratch_allocator);

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
                        scan_local_memory_first(arena, read_mem_result.memory,
                            read_mem_result.bytes_read, cmp, SCAN_DEFAULT_STRIDE);

                    ASSERT(matches);

                    proc_Object_Maybe containing_object =
                        proc_get_object_address_is_contained_in(
                            result.process_objects, region.base_address, value_type_size);

                    scan__push_region_matches(arena, &result, matches, region.base_address,
                        region.size, containing_object);
                }
            }
        }
    }

    if (at_least_one_read_succeeded && !(result.errors & PROC_STATUS_NO_SUCH_PROCESS)) {
        // Some reads failing is to be expected, for example a region could be unmapped during the
        // scan. As long as we managed to read at least one region, we'll count it as a success.
        result.errors = PROC_STATUS_OK;
    }

    return result;
}

scan_Result scan_remote_memory_next(Arena *arena, memmi_Process proc, scan_Settings settings, scan_Comparison cmp, scan_Result prev_results)
{
    scan_Result result = {};
    result.value_type = cmp.value_type;
    result.scan_mode = cmp.mode;
    result.process_objects = proc_get_loaded_objects(arena, proc);
    ASSERT(result.process_objects.status == PROC_STATUS_OK);


    // For temporarily storing the memory of a region during the scan of it.
    Arena *scan_scratch_arena = thread_ctx_get()->per_scan_scratch_arena;
    memmi_Allocator scan_scratch_allocator = proc_memmi_allocator_from_arena(scan_scratch_arena);

    b32 at_least_one_read_succeeded = false;

    for (scan_RegionMatches *match_list = list_head(&prev_results); match_list; match_list = list_next(match_list)) {
        // Reset the arena before reading each regions memory, as we only need it during the scan of
        // that region.
        arena_reset(scan_scratch_arena);

        memmi_ReadMemory read_mem_result =
            memmi_read_memory(proc, match_list->region_base_address,
                match_list->region_size, scan_scratch_allocator);

        if (read_mem_result.status != MEMMI_OK) {
            set_flag(&result.errors, proc_status_from_memmi_status(read_mem_result.status));

            if (read_mem_result.status == MEMMI_NO_SUCH_PROCESS) {
                break;
            }
        } else {
            void *region_mem = read_mem_result.memory;
            usize bytes_read = read_mem_result.bytes_read;

            scan_MatchList *matches = scan_local_memory_next(arena, region_mem,
                bytes_read, cmp, match_list->head);
            ASSERT(matches);

            scan__push_region_matches(arena, &result, matches, match_list->region_base_address,
                match_list->region_size, match_list->containing_object);
        }
    }

    // TODO: reduce code duplication between this and first scan
    if (at_least_one_read_succeeded && !(result.errors & PROC_STATUS_NO_SUCH_PROCESS)) {
        // Some reads failing is to be expected, for example a region could be unmapped during the
        // scan. As long as we managed to read at least one region, we'll count it as a success.
        result.errors = PROC_STATUS_OK;
    }

    return result;
}
