#pragma once

enum proc_RegionPermission {
    PROC_PERMISSION_READ    = (1u << 0u),
    PROC_PERMISSION_WRITE   = (1u << 1u),
    PROC_PERMISSION_EXECUTE = (1u << 2u),
};

enum proc_RegionKind {
    PROC_REGION_KIND_NORMAL,
    PROC_REGION_KIND_MAPPED_FILE,
};

struct proc_MemoryRegion {
    proc_RegionKind       kind;
    uintptr_t             address;
    usize                 size;
    proc_RegionPermission permissions;
};

struct proc_MemoryRegionList {
    proc_Status         status;
    proc_MemoryRegion  *data;
    usize               count;
    usize               total_regions_size;
};

struct proc_MemoryRegionContents {
    proc_MemoryRegion metadata;
    void *contents;
};

struct proc_MemoryRegionContentsList {
    proc_MemoryRegionContents *data;
    usize count;
};

DEFINE_MAYBE_TYPE(proc_MemoryRegion);

// TODO: replace the bool parameter
proc_MemoryRegionList proc_get_memory_regions_where(
    Arena *arena, memmi_Process proc, proc_RegionPermission required_permissions,
    b32 allow_memory_mapped_files, usize max_size);

proc_MemoryRegionContentsList proc_get_contents_of_memory_regions(Arena *arena, proc_MemoryRegionList regions);
