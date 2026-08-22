#include "scan_remote.h"

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

    if (memory_regions.status != MEMMI_OK) {
        result.status = proc_status_from_memmi_status(memory_regions.status);
    } else {
        for (usize i = 0; i < memory_regions.count; ++i) {
            memmi_MemoryRegion region = memory_regions.data[i];

            if (!(region.permissions & MEMMI_REGION_PERMISSION_READ)) {
                // TODO: filter out non-readable regions
                continue;
            }

            // Reset the arena before reading each regions memory, as we only need it
            // during the scan of that region.
            arena_reset(scan_scratch_arena);

            memmi_ReadMemory read_mem_result = memmi_read_memory(proc, region.base_address, region.size, scan_scratch_allocator);

            if (read_mem_result.status != MEMMI_OK) {
                result.status = proc_status_from_memmi_status(read_mem_result.status);
            } else {
                scan_MatchList *matches =
                    scan_local_memory_first(arena, read_mem_result.memory, read_mem_result.bytes_read, cmp, stride);

                ASSERT(matches);

                if (matches->count > 0) {
                    scan_RegionMatches *region_matches = arena_alloc_struct(arena, scan_RegionMatches);
                    region_matches->base_address = region.base_address;
                    region_matches->region_size = region.size;
                    region_matches->head = matches;

                    sll_push_back(&result, region_matches);

                } else {
                    ASSERT(!matches->next);
                }
            }
        }
    }

    return result;
}

scan_Result scan_remote_memory_next(Arena *arena, memmi_Process proc, scan_Comparison cmp, scan_Result prev_results)
{
    scan_Result result = {};

    // For temporarily storing the memory of a region during the scan of it.
    Arena *scan_scratch_arena = thread_ctx_get()->per_scan_scratch_arena;
    memmi_Allocator scan_scratch_allocator = proc_memmi_allocator_from_arena(scan_scratch_arena);

    for (scan_RegionMatches *match_list = list_head(&prev_results); match_list; match_list = list_next(match_list)) {
        // Reset the arena before reading each regions memory, as we only need it
        // during the scan of that region.
        arena_reset(scan_scratch_arena);

        memmi_ReadMemory read_mem_result =
            memmi_read_memory(proc, match_list->base_address, match_list->region_size, scan_scratch_allocator);

        void *region_mem = read_mem_result.memory;
        usize bytes_read = read_mem_result.bytes_read;

        scan_MatchList *matches = scan_local_memory_next(arena, region_mem, bytes_read, cmp, match_list->head);
        ASSERT(matches);

        if (matches->count > 0) {
            scan_RegionMatches *region_matches = arena_alloc_struct(arena, scan_RegionMatches);
            region_matches->base_address = match_list->base_address;
            region_matches->region_size = match_list->region_size;
            region_matches->head = matches;

            sll_push_back(&result, region_matches);

        } else {
            ASSERT(!matches->next);
        }
    }

    return result;
}
