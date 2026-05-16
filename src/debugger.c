#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "debugger.h"
#include "log.h"

void dbg_init(debugger_t *dbg, const char *target)
{
    memset(dbg, 0, sizeof(*dbg));
    dbg->target  = target;
    dbg->pid     = -1;
    dbg->running = false;
}

void dbg_launch(debugger_t *dbg)
{
    pid_t pid = fork();
    if (pid < 0) {
        log_err("fork failed: %s\n", strerror(errno));
        return;
    }
    if (pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
            perror("ptrace(PTRACE_TRACEME)");
            _exit(1);
        }
        char *const argv[] = { (char *)dbg->target, NULL };
        execvp(dbg->target, argv);
        perror("execvp");
        _exit(1);
    }
    dbg->pid     = pid;
    dbg->running = true;
}

void dbg_cleanup(debugger_t *dbg)
{
    if (dbg->pid != -1) {
        ptrace(PTRACE_KILL, dbg->pid, NULL, NULL);
        waitpid(dbg->pid, NULL, 0);
        dbg->pid     = -1;
        dbg->running = false;
    }
}

void dbg_wait(debugger_t *dbg)
{
    if (dbg->pid == -1)
        return;

    int status;
    if (waitpid(dbg->pid, &status, 0) < 0) {
        log_err("waitpid failed: %s\n", strerror(errno));
        return;
    }

    if (WIFEXITED(status)) {
        log_warn("process exited with code %d\n", WEXITSTATUS(status));
        dbg->pid     = -1;
        dbg->running = false;
        return;
    }

    if (WIFSIGNALED(status)) {
        log_warn("process killed by signal %d (%s)\n",
                 WTERMSIG(status), strsignal(WTERMSIG(status)));
        dbg->pid     = -1;
        dbg->running = false;
        return;
    }

    if (WIFSTOPPED(status)) {
        int sig = WSTOPSIG(status);
        if (sig == SIGTRAP) {
            uint64_t rip = dbg_get_rip(dbg);
            breakpoint_t *bp = dbg_break_find(dbg, rip - 1);
            if (bp && bp->enabled) {
                dbg_break_disable(dbg, bp);
                dbg_set_rip(dbg, rip - 1);
                bp->hit = true;
                log_ok("=> breakpoint hit at 0x%016lx\n", rip - 1);
            } else {
                log_msg("stopped: rip = 0x%016lx\n", rip);
            }
        } else {
            log_warn("[stopped: %s]\n", strsignal(sig));
        }
    }
}

void dbg_continue(debugger_t *dbg)
{
    if (dbg->pid == -1)
        return;

    bool any_hit = false;
    for (int i = 0; i < dbg->bp_count; i++) {
        if (dbg->breakpoints[i].hit) {
            any_hit = true;
            break;
        }
    }

    if (any_hit) {
        if (ptrace(PTRACE_SINGLESTEP, dbg->pid, NULL, NULL) < 0) {
            log_err("ptrace(SINGLESTEP) failed: %s\n", strerror(errno));
            return;
        }
        int status;
        waitpid(dbg->pid, &status, 0);

        if (!WIFSTOPPED(status)) {
            if (WIFEXITED(status)) {
                log_warn("process exited with code %d\n", WEXITSTATUS(status));
                dbg->pid     = -1;
                dbg->running = false;
            }
            return;
        }

        for (int i = 0; i < dbg->bp_count; i++) {
            if (dbg->breakpoints[i].hit) {
                dbg_break_enable(dbg, &dbg->breakpoints[i]);
                dbg->breakpoints[i].hit = false;
            }
        }
    }

    if (ptrace(PTRACE_CONT, dbg->pid, NULL, NULL) < 0) {
        log_err("ptrace(CONT) failed: %s\n", strerror(errno));
    }
}

void dbg_step(debugger_t *dbg)
{
    if (dbg->pid == -1)
        return;

    for (int i = 0; i < dbg->bp_count; i++)
        dbg->breakpoints[i].hit = false;

    if (ptrace(PTRACE_SINGLESTEP, dbg->pid, NULL, NULL) < 0) {
        log_err("ptrace(SINGLESTEP) failed: %s\n", strerror(errno));
    }
}
