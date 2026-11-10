#pragma once

enum proc_Status {
    PROC_STATUS_OK                       = 0,
    PROC_STATUS_NO_SUCH_PROCESS          = (1u << 0u),
    PROC_STATUS_INSUFFICIENT_PERMISSIONS = (1u << 1u),
    PROC_STATUS_PARTIAL_READ_OR_WRITE    = (1u << 2u),
    PROC_STATUS_OTHER_ERROR              = (1u << 3u),
};

proc_Status     proc_status_from_memmi_status(memmi_Status status);
memmi_Allocator proc_memmi_allocator_from_arena(Arena *arena);
String          proc_copy_memmi_string(Arena *arena, memmi_String string);
