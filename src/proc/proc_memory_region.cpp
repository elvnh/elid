#include "proc_memory_region.h"

memmi_MemoryRegionPermission proc__permissions_to_memmi_permissions(proc_RegionPermission perms)
{
    memmi_MemoryRegionPermission result = {};

    if (perms & PROC_PERMISSION_READ) {
        set_flag(&result, MEMMI_REGION_PERMISSION_READ);
    }

    if (perms & PROC_PERMISSION_WRITE) {
        set_flag(&result, MEMMI_REGION_PERMISSION_WRITE);
    }

    if (perms & PROC_PERMISSION_EXECUTE) {
        set_flag(&result, MEMMI_REGION_PERMISSION_EXECUTE);
    }

    return result;
}

proc_RegionPermission proc__permissions_from_memmi_permissions(memmi_MemoryRegionPermission perms)
{
    proc_RegionPermission result = {};

    if (perms & MEMMI_REGION_PERMISSION_READ) {
        set_flag(&result, PROC_PERMISSION_READ);
    }

    if (perms & MEMMI_REGION_PERMISSION_WRITE) {
        set_flag(&result, PROC_PERMISSION_WRITE);
    }

    if (perms & MEMMI_REGION_PERMISSION_EXECUTE) {
        set_flag(&result, PROC_PERMISSION_EXECUTE);
    }

    return result;
}

proc_RegionKind proc__region_kind_from_memmi_kind(memmi_MemoryRegionKind kind)
{
    proc_RegionKind result = {};

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (kind) {
        case MEMMI_REGION_NORMAL: {
            result = PROC_REGION_KIND_NORMAL;
        } break;

        case MEMMI_REGION_MAPPED_FILE: {
            result = PROC_REGION_KIND_MAPPED_FILE;
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

proc_MemoryRegionList proc_get_memory_regions_where(
    Arena *arena,
    memmi_Process proc,
    proc_RegionPermission required_permissions,
    b32 allow_memory_mapped_files,
    usize max_size)
{
    proc_MemoryRegionList result = {};

    ScratchArena scratch = scratch_begin();
    memmi_Allocator temp_allocator = proc_memmi_allocator_from_arena(scratch.arena);

    memmi_MemoryRegions regions = memmi_get_process_memory_regions(proc, temp_allocator);

    memmi_MemoryRegionPermission memmi_perms = proc__permissions_to_memmi_permissions(required_permissions);

    if (regions.status != MEMMI_OK) {
        result.status = proc_status_from_memmi_status(regions.status);
    } else {
        result.data = arena_alloc_array(arena, proc_MemoryRegion, regions.count);

        for (usize i = 0; i < regions.count; ++i) {
            memmi_MemoryRegion memmi_region = regions.data[i];

            b32 region_should_be_added =
                (memmi_region.permissions & memmi_perms) == memmi_perms
                && ((memmi_region.kind == MEMMI_REGION_NORMAL) || (allow_memory_mapped_files))
                && (memmi_region.size <= max_size);

            if (region_should_be_added) {
                proc_MemoryRegion region = {};
                region.address = memmi_region.base_address;
                region.size = memmi_region.size;
                region.permissions = proc__permissions_from_memmi_permissions(memmi_region.permissions);
                region.kind = proc__region_kind_from_memmi_kind(memmi_region.kind);

                result.data[result.count++] = region;
                result.total_regions_size += region.size;
            }
        }
    }

    return result;
}

proc_MemoryRegionContentsList proc_get_contents_of_memory_regions(Arena *arena, memmi_Process proc, proc_MemoryRegionList regions)
{
    ASSERT(regions.status == PROC_STATUS_OK);

    proc_MemoryRegionContentsList result = {};
    result.data = arena_alloc_array(arena, proc_MemoryRegionContents, regions.count);

    for (usize i = 0; i < regions.count; ++i) {
        proc_MemoryRegionContents region_contents = {};
        region_contents.metadata = regions.data[i];
        region_contents.contents = (void *)arena_alloc_array(arena, char, region_contents.metadata.size);

        memmi_ReadMemory read_mem_result = memmi_read_memory(
            proc, region_contents.contents, region_contents.metadata.address, region_contents.metadata.size);

        if (ALWAYS(read_mem_result.status == MEMMI_OK)) {
            result.data[result.count++] = region_contents;
        }
    }

    return result;
}
