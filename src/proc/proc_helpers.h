#pragma once

// This file defines helpers for interacting with the memmi library.

enum proc_Status {
    PROC_STATUS_OK = 0,
    PROC_STATUS_NO_SUCH_PROCESS,
    PROC_STATUS_INSUFFICIENT_PERMISSIONS,
    PROC_STATUS_PARTIAL_READ_OR_WRITE,
    PROC_STATUS_OTHER_ERROR,
};

proc_Status     proc_status_from_memmi_status(memmi_Status status);
memmi_Allocator proc_memmi_allocator_from_arena(Arena *arena);
