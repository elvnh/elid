#include "app_watch.h"

#define APP_WATCH_TIMEOUT_MS       10
#define APP_WATCH_ARENA_SIZE       MB(1)
#define APP_WATCH_EVENT_POOL_SIZE  128

void app_watch__loop(app_Watch *watch)
{
    memmi_ContinueMode continue_mode = MEMMI_CONTINUE_UNHANDLED;

    while (!atomic_load_s32(&watch->should_terminate)) {
        memmi_Process proc = watch->process;

        // TODO: don't use infinite timeout since the thread won't exit when app exits
        memmi_DebugEvent debug_event = memmi_wait_for_debug_event(
            proc, continue_mode, MEMMI_TIMEOUT_INFINITE);

        if (debug_event.status == MEMMI_OK) {
            if (debug_event.kind == MEMMI_DEBUG_EVENT_BREAKPOINT) {
                // We don't want to crash the application when a breakpoint is triggered.
                continue_mode = MEMMI_CONTINUE_HANDLED;

                b32 breakpoint_is_ours = debug_event.as.breakpoint.breakpoint_index == watch->breakpoint_index;

                if (breakpoint_is_ours) {
                    // TODO: use lockless queue
                    ScopedLock lock = scoped_lock(watch->event_queue.mutex);

                    app_WatchEvent *event = pool_alloc(watch->event_queue.pool, app_WatchEvent);
                    event->instruction_pointer_after_execution = debug_event.as.breakpoint.ip_register;
                    event->registers_after_execution = proc_get_thread_registers(debug_event.id_of_affected_thread);

                    queue_push(&watch->event_queue, event);
                }
            } else {
                // Don't handle any non-breakpoint exceptions, pass them on to the debuggee.
                continue_mode = MEMMI_CONTINUE_UNHANDLED;
            }
        }
    }
}

void app_watch_run(app_Watch *watch)
{
    memmi_BreakpointCondition memmi_cond = {};

    if (watch->breakpoint_condition == APP_WATCH_COND_WRITE) {
        memmi_cond = MEMMI_BREAKPOINT_WRITE;
    } else if (watch->breakpoint_condition == APP_WATCH_COND_READ_WRITE) {
        memmi_cond = MEMMI_BREAKPOINT_READ_WRITE;
    }

    // NOTE: we attach on this thread rather than the main thread since on Linux, attaching from a
    // thread only makes that one thread the tracer, rather than the entire process. Since we'll
    // later perform tracer-exclusive actions such as setting breakpoints and waiting for debug
    // events, that thread has to be the tracer.
    memmi_Status attach_result = memmi_attach_to_process(watch->process);

    memmi_Status breakpoint_result = memmi_set_hardware_breakpoint(
        watch->process,
        watch->breakpoint_address,
        memmi_cond,
        watch->breakpoint_index,
        (memmi_BreakpointLength)watch->breakpoint_length);

    if (attach_result != MEMMI_OK) {
        TODO("communicate error to attach");
    } else if (breakpoint_result != MEMMI_OK) {
        TODO("communicate error to set breakpoint");
    } else {
        app_watch__loop(watch);

        memmi_detach_from_process(watch->process);
    }
}

void app_watch_destroy(Pool *pool, app_Watch *watch)
{
    atomic_store_s32(&watch->should_terminate, true);

    thread_join(watch->thread);

    mutex_destroy(watch->event_queue.mutex);

    arena_destroy(watch->arena);

    pool_free(pool, watch);
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
        b32 found_instruction = false;

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
                // TODO: no need to track these separately
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

    ScratchArena scratch = scratch_begin();

    proc_RegionPermission permissions = (proc_RegionPermission)((s32)PROC_PERMISSION_EXECUTE | (s32)PROC_PERMISSION_READ);

    proc_MemoryRegionList executable_regions = proc_get_memory_regions_with_permissions(
        scratch.arena, watch->process, permissions);

    if (executable_regions.status != PROC_STATUS_OK) {
        // TODO: why is this triggered?
        result = executable_regions.status;
    } else {
        // TODO: read in the contents of the memory regions on demand instead
        proc_MemoryRegionContentsList executable_regions_contents =
            proc_get_contents_of_memory_regions(scratch.arena, watch->process, executable_regions);

        // TODO: don't hold the lock for this long
        ScopedLock lock = scoped_lock(watch->event_queue.mutex);

        app_WatchEvent *head_event = watch->event_queue.first;

        // TODO: list_clear macro
        watch->event_queue.first = 0;
        watch->event_queue.last = 0;
        watch->event_queue.list_node_count = 0;

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
                *instr = app_watch_find_instruction_preceding_address(
                    watch, executable_regions_contents, event_address);
                instr->trigger_count = 1;

                ASSERT(instr->start_address);
                ASSERT(instr->mnemonic.data);
                ASSERT(instr->mnemonic.data);

                sll_push_back(&watch->triggered_instruction_list, instr);
            }

            // The registers are updated each time the instruction is hit.
            instr->registers_after_execution = e->registers_after_execution;
        }

        pool_reset(watch->event_queue.pool);
    }

    return result;
}

void app_watch_init(app_Watch *watch, memmi_Process proc, uintptr_t address,
    u32 breakpoint_index, app_WatchCondition cond, u32 breakpoint_length)
{
    *watch = {};
    watch->arena = arena_create(APP_WATCH_ARENA_SIZE);
    watch->process = proc;
    watch->event_queue.mutex = mutex_create();
    watch->event_queue.pool = pool_create(watch->arena, app_WatchEvent, APP_WATCH_EVENT_POOL_SIZE);
    watch->breakpoint_address = address;
    watch->breakpoint_index = breakpoint_index;
    watch->breakpoint_condition = cond;
    watch->breakpoint_length = breakpoint_length;
}
