#include "app_watch.h"

#define APP_WATCH_TIMEOUT_MS       10
#define APP_WATCH_ARENA_SIZE       MB(1)
#define APP_WATCH_EVENT_POOL_SIZE  128

void app_watch__loop(app_TaskID, void *user_data)
{
    app_WatchThreadContext *watch = (app_WatchThreadContext *)user_data;

    while (!atomic_load_s32(&watch->should_terminate)) {
        memmi_Process proc = watch->process;
        memmi_DebugEvent debug_event = memmi_wait_for_debug_event(proc, MEMMI_CONTINUE_UNHANDLED, APP_WATCH_TIMEOUT_MS);

        if (debug_event.status != MEMMI_OK) {
            ASSERT(0);
        } else {
            if (debug_event.kind == MEMMI_DEBUG_EVENT_BREAKPOINT) {
                b32 breakpoint_is_ours = debug_event.as.breakpoint.breakpoint_index == watch->breakpoint_index;

                if (breakpoint_is_ours) {
                    // TODO: use lockless queue
                    ScopedLock lock = scoped_lock(watch->event_queue.mutex);

                    app_WatchEvent *event = pool_alloc(watch->event_queue.pool, app_WatchEvent);
                    event->instruction_pointer_after_execution = debug_event.as.breakpoint.ip_register;

                    queue_push(&watch->event_queue, event);

                    printf("event\n");
                }
            }
        }
    }
}

app_Watch app_watch_begin(app_WorkQueue *wq, memmi_Process proc, uintptr_t address, app_WatchCondition cond, u32 size, u32 index)
{
    // NOTE: must be attached when attempting this
    app_Watch result = {};

    memmi_BreakpointCondition memmi_cond = {};

    if (cond == APP_WATCH_COND_WRITE) {
        memmi_cond = MEMMI_BREAKPOINT_WRITE;
    } else {
        ASSERT(cond == APP_WATCH_COND_READ_WRITE);
        memmi_cond = MEMMI_BREAKPOINT_READ_WRITE;
    }

    memmi_Status breakpoint_result = memmi_set_hardware_breakpoint(
        proc, address, memmi_cond, index, (memmi_BreakpointLength)size);

    if (breakpoint_result != MEMMI_OK) {
        result.status = proc_status_from_memmi_status(breakpoint_result);
    } else {
        result.is_active = true;
        result.status = PROC_STATUS_OK;
        result.arena = arena_create(APP_WATCH_ARENA_SIZE);
        result.thread_context.process = proc;
        result.thread_context.event_queue.mutex = mutex_create();
        result.thread_context.event_queue.pool = pool_create(result.arena, app_WatchEvent, APP_WATCH_EVENT_POOL_SIZE);

        app_WatchThreadContext *watch_thread_arg = &result.thread_context;
        result.task = app_wq_push_task(wq, app_watch__loop, watch_thread_arg);
    }

    return result;
}

void app_watch_destroy(app_Watch *watch)
{
    atomic_store_s32(&watch->thread_context.should_terminate, true);

    app_wq_wait_for_task(watch->task);

    mutex_destroy(watch->thread_context.event_queue.mutex);

    arena_destroy(watch->arena);

    *watch = {};
}

app_WatchInstruction app_watch_find_instruction_preceding_address(
    app_Watch *watch, proc_MemoryRegionContentsList regions, uintptr_t address)
{
    // TODO: pass in memory region after it's been found instead
    app_WatchInstruction result = {};

    // Find the region that the instruction is contained in.
    proc_MemoryRegionContents *containing_region = 0;

    for (usize i = 0; i < regions.count; ++i) {
        proc_MemoryRegionContents *region = &regions.data[i];
        uintptr_t region_begin = region->metadata.address;
        uintptr_t region_end   = region_begin + region->metadata.size;

        if ((region_begin <= address) && (address <= region_end)) {
            containing_region = region;
            break;
        }
    }

    if (ALWAYS(containing_region)) {
        b32 found_instruction = true;

        usize instruction_address = containing_region->metadata.address;
        usize offset = 0;

        ZydisMachineMode zydis_machine_mode =
        #if ARCHITECTURE_X86
            ZYDIS_MACHINE_MODE_LONG_COMPAT_32
        #elif ARCHITECTURE_X64
            ZYDIS_MACHINE_MODE_LONG_64
        #endif
            ;

        ZydisDisassembledInstruction zydis_instruction = {};

        while (!found_instruction
            && ZYAN_SUCCESS(ZydisDisassembleIntel(
                    zydis_machine_mode,
                    instruction_address,
                    (char *)containing_region->contents + offset,
                    containing_region->metadata.size - offset,
                    &zydis_instruction))) {
            uintptr_t opcode_end_address = zydis_instruction.runtime_address + zydis_instruction.info.length;

            if (opcode_end_address == address) {
                found_instruction = true;
            } else {
                offset += zydis_instruction.info.length;
                instruction_address += zydis_instruction.info.length;
            }

        }

        ASSERT(found_instruction);

        result.start_address = zydis_instruction.runtime_address;
        result.opcode_size = zydis_instruction.info.length;
        result.mnemonic = str_from_cstr(str_clone_cstr(watch->arena, zydis_instruction.text));
    }

    return result;
}

proc_Status app_watch_poll_events(app_Watch *watch)
{
    proc_Status result = {};

        Temp temp = scratch_begin();
    memmi_Process process = watch->thread_context.process;

    proc_MemoryRegionList executable_regions = proc_get_memory_regions_with_permissions(
        temp.arena, watch->thread_context.process, PROC_PERMISSION_EXECUTE);

    if (executable_regions.status != PROC_STATUS_OK) {
        result = executable_regions.status;
    } else {
        // TODO: read in the contents of the memory regions on demand instead
        proc_MemoryRegionContentsList executable_regions_contents =
            proc_get_contents_of_memory_regions(temp.arena, process, executable_regions);

        // TODO: don't hold the lock for this long
        ScopedLock lock = scoped_lock(watch->thread_context.event_queue.mutex);

        app_WatchEvent *head_event = watch->thread_context.event_queue.first;

        // TODO: list_clear macro
        watch->thread_context.event_queue.first = 0;
        watch->thread_context.event_queue.last = 0;
        watch->thread_context.event_queue.list_node_count = 0;

        for (app_WatchEvent *e = head_event; e; e = list_next(e)) {
            uintptr_t event_address = e->instruction_pointer_after_execution;

            // Find out if this instruction has been triggered already.
            app_WatchInstruction *instr = 0;
            for (instr = list_head(&watch->triggered_instruction_list); instr; instr = list_next(instr)) {
                if ((instr->start_address + instr->opcode_size) == event_address) {
                    break;
                }
            }

            if (instr) {
                ++instr->trigger_count;
            } else {
                // This instruction hasn't been triggered yet, allocate a new one.
                instr = arena_alloc_struct(watch->arena, app_WatchInstruction);
                *instr = app_watch_find_instruction_preceding_address(watch, executable_regions_contents, event_address);
            }
        }

        pool_reset(watch->thread_context.event_queue.pool);
    }

    return result;
}
