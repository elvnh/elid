#include "scan_remote.h"

// This is used to skip the huge memory regions that ASan allocates.
// TODO: can this be done in a better way?
// TODO: this is not necessary for 32 bit
#define SCAN_SKIP_REGION_SIZE_THRESHOLD TB(13)

void scan__push_region_matches(Arena *arena, scan_Result *result, scan_MatchList *matches, uintptr_t region_address, usize region_size)
{
    if (matches->count > 0) {
        scan_RegionMatches *region_matches = arena_alloc_struct(arena, scan_RegionMatches);
        region_matches->base_address = region_address;
        region_matches->region_size = region_size;
        region_matches->head = matches;
        region_matches->match_count_in_region = matches->total_match_count;

        result->total_match_count += region_matches->match_count_in_region;

        sll_push_back(result, region_matches);

    } else {
        ASSERT(!matches->next);
    }
}

scan_Result scan_remote_memory_first(Arena *arena, memmi_Process proc, scan_Comparison cmp, usize stride)
{
    scan_Result result = {};

    // For temporarily storing the memory of a region during the scan of it.
    Arena *scan_scratch_arena = thread_ctx_get()->per_scan_scratch_arena;
    memmi_Allocator scan_scratch_allocator = proc_memmi_allocator_from_arena(scan_scratch_arena);

    // For allocating memory region metadata array. We allocate those in a separate arena
    // as they're needed for the entire function, while the scan scratch arena gets reset
    // after each region.
    Temp scratch = scratch_begin();
    memmi_Allocator region_allocator = proc_memmi_allocator_from_arena(scratch.arena);

    memmi_MemoryRegions memory_regions = memmi_get_process_memory_regions(proc, region_allocator);

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

                memmi_ReadMemory read_mem_result = memmi_read_memory(proc, region.base_address, region.size, scan_scratch_allocator);

                if (read_mem_result.status != MEMMI_OK) {
                    set_flag(&result.errors, proc_status_from_memmi_status(read_mem_result.status));
                } else {
                    at_least_one_read_succeeded = true;

                    // TODO: return nil match list
                    scan_MatchList *matches =
                        scan_local_memory_first(arena, read_mem_result.memory, read_mem_result.bytes_read, cmp, stride);

                    ASSERT(matches);

                    scan__push_region_matches(arena, &result, matches, region.base_address, region.size);
                }
            }
        }
    }

    if (at_least_one_read_succeeded) {
        // Some reads failing is to be expected, for example a region could be unmapped during the
        // scan. As long as we managed to read at least one region, we'll count it as a success.
        result.errors = PROC_STATUS_OK;
    }

    return result;
}

scan_Result scan_remote_memory_next(Arena *arena, memmi_Process proc, scan_Comparison cmp, scan_Result prev_results)
{
    scan_Result result = {};

    // For temporarily storing the memory of a region during the scan of it.
    Arena *scan_scratch_arena = thread_ctx_get()->per_scan_scratch_arena;
    memmi_Allocator scan_scratch_allocator = proc_memmi_allocator_from_arena(scan_scratch_arena);

    b32 at_least_one_read_succeeded = false;

    for (scan_RegionMatches *match_list = list_head(&prev_results); match_list; match_list = list_next(match_list)) {
        // Reset the arena before reading each regions memory, as we only need it during the scan of
        // that region.
        arena_reset(scan_scratch_arena);

        memmi_ReadMemory read_mem_result =
            memmi_read_memory(proc, match_list->base_address, match_list->region_size, scan_scratch_allocator);

        if (read_mem_result.status != MEMMI_OK) {
            set_flag(&result.errors, proc_status_from_memmi_status(read_mem_result.status));
        } else {
            void *region_mem = read_mem_result.memory;
            usize bytes_read = read_mem_result.bytes_read;

            scan_MatchList *matches = scan_local_memory_next(arena, region_mem, bytes_read, cmp, match_list->head);
            ASSERT(matches);

            scan__push_region_matches(arena, &result, matches, match_list->base_address, match_list->region_size);
        }
    }

    if (at_least_one_read_succeeded) {
        // Some reads failing is to be expected, for example a region could be unmapped during the
        // scan. As long as we managed to read at least one region, we'll count it as a success.
        result.errors = PROC_STATUS_OK;
    }

    return result;
}
