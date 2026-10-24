#include "proc_registers.h"

proc_Registers proc__registers_from_memmi_registers(memmi_Registers regs)
{
    proc_Registers result = {};
    result.status = proc_status_from_memmi_status(regs.status);

    #if ARCHITECTURE_X64
    {
        result.values[PROC_REG_RAX] = regs.values[PROC_REG_RAX];
        result.values[PROC_REG_RCX] = regs.values[PROC_REG_RCX];
        result.values[PROC_REG_RDX] = regs.values[PROC_REG_RDX];
        result.values[PROC_REG_RSI] = regs.values[PROC_REG_RSI];
        result.values[PROC_REG_RDI] = regs.values[PROC_REG_RDI];
        result.values[PROC_REG_RSP] = regs.values[PROC_REG_RSP];
        result.values[PROC_REG_RBP] = regs.values[PROC_REG_RBP];
        result.values[PROC_REG_RBX] = regs.values[PROC_REG_RBX];
    }
    #elif ARCHITECTURE_X86
    {
        result.values[PROC_REG_EAX] = regs.values[PROC_REG_EAX];
        result.values[PROC_REG_ECX] = regs.values[PROC_REG_ECX];
        result.values[PROC_REG_EDX] = regs.values[PROC_REG_EDX];
        result.values[PROC_REG_ESI] = regs.values[PROC_REG_ESI];
        result.values[PROC_REG_EDI] = regs.values[PROC_REG_EDI];
        result.values[PROC_REG_ESP] = regs.values[PROC_REG_ESP];
        result.values[PROC_REG_EBP] = regs.values[PROC_REG_EBP];
        result.values[PROC_REG_EBX] = regs.values[PROC_REG_EBX];
    }
    #endif

    result.values[PROC_REG_R8] = regs.values[PROC_REG_R8];
    result.values[PROC_REG_R9] = regs.values[PROC_REG_R9];
    result.values[PROC_REG_R10] = regs.values[PROC_REG_R10];
    result.values[PROC_REG_R11] = regs.values[PROC_REG_R11];
    result.values[PROC_REG_R12] = regs.values[PROC_REG_R12];
    result.values[PROC_REG_R13] = regs.values[PROC_REG_R13];
    result.values[PROC_REG_R14] = regs.values[PROC_REG_R14];
    result.values[PROC_REG_R15] = regs.values[PROC_REG_R15];

    #if ARCHITECTURE_X64
    {
        result.values[PROC_REG_RIP] = regs.values[PROC_REG_RIP];
        result.values[PROC_REG_RFLAGS] = regs.values[PROC_REG_RFLAGS];
    }
    #elif ARCHITECTURE_X86
    {
        result.values[PROC_REG_EIP] = regs.values[PROC_REG_EIP];
        result.values[PROC_REG_EFLAGS] = regs.values[PROC_REG_EFLAGS];
    }
    #endif

    result.values[PROC_REG_CS] = regs.values[PROC_REG_CS];
    result.values[PROC_REG_SS] = regs.values[PROC_REG_SS];
    result.values[PROC_REG_DS] = regs.values[PROC_REG_DS];
    result.values[PROC_REG_ES] = regs.values[PROC_REG_ES];
    result.values[PROC_REG_FS] = regs.values[PROC_REG_FS];
    result.values[PROC_REG_GS] = regs.values[PROC_REG_GS];

    result.values[PROC_REG_DR0] = regs.values[PROC_REG_DR0];
    result.values[PROC_REG_DR1] = regs.values[PROC_REG_DR1];
    result.values[PROC_REG_DR2] = regs.values[PROC_REG_DR2];
    result.values[PROC_REG_DR3] = regs.values[PROC_REG_DR3];
    result.values[PROC_REG_DR6] = regs.values[PROC_REG_DR6];
    result.values[PROC_REG_DR7] = regs.values[PROC_REG_DR7];

    return result;
}


proc_Registers proc_get_thread_registers(memmi_TID tid)
{
    memmi_Registers memmi_regs = memmi_get_thread_registers(tid);
    proc_Registers result = proc__registers_from_memmi_registers(memmi_regs);

    return result;
}

char *proc_register_to_cstr(Arena *arena, proc_Register reg)
{
    const char *str = 0;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (reg) {
#if ARCHITECTURE_X64
        case PROC_REG_RAX: { str = "RAX"; } break;
        case PROC_REG_RCX: { str = "RCX"; } break;
        case PROC_REG_RDX: { str = "RDX"; } break;
        case PROC_REG_RSI: { str = "RSI"; } break;
        case PROC_REG_RDI: { str = "RDI"; } break;
        case PROC_REG_RSP: { str = "RSP"; } break;
        case PROC_REG_RBP: { str = "RBP"; } break;
        case PROC_REG_RBX: { str = "RBX"; } break;
#elif ARCHITECTURE_X86
        case PROC_REG_EAX: { str = "EAX"; } break;
        case PROC_REG_ECX: { str = "ECX"; } break;
        case PROC_REG_EDX: { str = "EDX"; } break;
        case PROC_REG_ESI: { str = "ESI"; } break;
        case PROC_REG_EDI: { str = "EDI"; } break;
        case PROC_REG_ESP: { str = "ESP"; } break;
        case PROC_REG_EBP: { str = "EBP"; } break;
        case PROC_REG_EBX: { str = "EBX"; } break;
#endif

        case PROC_REG_R8: { str = "R8"; } break;
        case PROC_REG_R9: { str = "R9"; } break;
        case PROC_REG_R10: { str = "R10"; } break;
        case PROC_REG_R11: { str = "R11"; } break;
        case PROC_REG_R12: { str = "R12"; } break;
        case PROC_REG_R13: { str = "R13"; } break;
        case PROC_REG_R14: { str = "R14"; } break;
        case PROC_REG_R15: { str = "R15"; } break;

#if ARCHITECTURE_X64
        case PROC_REG_RIP: { str = "RIP"; } break;
        case PROC_REG_RFLAGS: { str = "RFLAGS"; } break;
#elif ARCHITECTURE_X86
        case PROC_REG_EIP: { str = "EIP"; } break;
        case PROC_REG_EFLAGS: { str = "EFLAGS"; } break;
#endif

        case PROC_REG_CS: { str = "CS"; } break;
        case PROC_REG_SS: { str = "SS"; } break;
        case PROC_REG_DS: { str = "DS"; } break;
        case PROC_REG_ES: { str = "ES"; } break;
        case PROC_REG_FS: { str = "FS"; } break;
        case PROC_REG_GS: { str = "GS"; } break;

        case PROC_REG_DR0: { str = "DR0"; } break;
        case PROC_REG_DR1: { str = "DR1"; } break;
        case PROC_REG_DR2: { str = "DR2"; } break;
        case PROC_REG_DR3: { str = "DR3"; } break;
        case PROC_REG_DR6: { str = "DR6"; } break;
        case PROC_REG_DR7: { str = "DR7"; } break;

        INVALID_CASE(PROC_REG_COUNT);
        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    char *result = str_clone_cstr(arena, str);

    return result;
}

char *proc_register_value_to_cstr(Arena *arena, proc_RegisterValue value, b32 hex)
{
    char *result = 0;

    if (hex) {
        result = str_format_cstr(arena, "%" PRIxPTR, value);
    } else {
        result = str_format_cstr(arena, "%" PRIuPTR, value);
    }

    return result;
}
