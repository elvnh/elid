#pragma once

enum proc_Register {
#if ARCHITECTURE_X64
    PROC_REG_RAX,
    PROC_REG_RCX,
    PROC_REG_RDX,
    PROC_REG_RSI,
    PROC_REG_RDI,
    PROC_REG_RSP,
    PROC_REG_RBP,
    PROC_REG_RBX,
#elif ARCHITECTURE_X86
    PROC_REG_EAX,
    PROC_REG_ECX,
    PROC_REG_EDX,
    PROC_REG_ESI,
    PROC_REG_EDI,
    PROC_REG_ESP,
    PROC_REG_EBP,
    PROC_REG_EBX,
#endif

    PROC_REG_R8,
    PROC_REG_R9,
    PROC_REG_R10,
    PROC_REG_R11,
    PROC_REG_R12,
    PROC_REG_R13,
    PROC_REG_R14,
    PROC_REG_R15,

#if ARCHITECTURE_X64
    PROC_REG_RIP,
    PROC_REG_RFLAGS,
#elif ARCHITECTURE_X86
    PROC_REG_EIP,
    PROC_REG_EFLAGS,
#endif

    PROC_REG_CS,
    PROC_REG_SS,
    PROC_REG_DS,
    PROC_REG_ES,
    PROC_REG_FS,
    PROC_REG_GS,

    PROC_REG_DR0,
    PROC_REG_DR1,
    PROC_REG_DR2,
    PROC_REG_DR3,
    PROC_REG_DR6,
    PROC_REG_DR7,

    PROC_REG_COUNT
};

#if ARCHITECTURE_X64
    typedef u64 proc_RegisterValue;
#elif ARCHITECTURE_X86
    typedef u32 proc_RegisterValue;
#endif

struct proc_Registers {
    proc_Status status;
    proc_RegisterValue values[PROC_REG_COUNT];
};

proc_Registers  proc_get_thread_registers(memmi_TID tid);
char           *proc_register_to_cstr(Arena *arena, proc_Register reg);
char           *proc_register_value_to_cstr(Arena *arena, proc_RegisterValue value, b32 hex);
