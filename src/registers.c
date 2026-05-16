#include <errno.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include "debugger.h"
#include "log.h"

struct user_regs_struct dbg_get_regs_raw(debugger_t *dbg)
{
    struct user_regs_struct regs;
    memset(&regs, 0, sizeof(regs));
    if (ptrace(PTRACE_GETREGS, dbg->pid, NULL, &regs) < 0)
        log_err("ptrace(GETREGS) failed: %s\n", strerror(errno));
    return regs;
}

void dbg_print_regs(debugger_t *dbg)
{
    struct user_regs_struct r = dbg_get_regs_raw(dbg);
    log_msg("rip  0x%016llx  rsp  0x%016llx\n", r.rip, r.rsp);
    log_msg("rax  0x%016llx  rbx  0x%016llx\n", r.rax, r.rbx);
    log_msg("rcx  0x%016llx  rdx  0x%016llx\n", r.rcx, r.rdx);
    log_msg("rdi  0x%016llx  rsi  0x%016llx\n", r.rdi, r.rsi);
    log_msg("r8   0x%016llx  r9   0x%016llx\n", r.r8,  r.r9);
    log_msg("r10  0x%016llx  r11  0x%016llx\n", r.r10, r.r11);
    log_msg("r12  0x%016llx  r13  0x%016llx\n", r.r12, r.r13);
    log_msg("r14  0x%016llx  r15  0x%016llx  rflags 0x%016llx\n",
            r.r14, r.r15, r.eflags);
}

uint64_t dbg_get_rip(debugger_t *dbg)
{
    return dbg_get_regs_raw(dbg).rip;
}

void dbg_set_rip(debugger_t *dbg, uint64_t rip)
{
    struct user_regs_struct regs = dbg_get_regs_raw(dbg);
    regs.rip = rip;
    if (ptrace(PTRACE_SETREGS, dbg->pid, NULL, &regs) < 0)
        log_err("ptrace(SETREGS) failed: %s\n", strerror(errno));
}
