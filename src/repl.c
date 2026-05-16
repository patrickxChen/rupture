#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debugger.h"
#include "tui.h"
#include "log.h"

static void print_help(void)
{
    log_msg("┌──────────────────────────────────────────────────────┐\n");
    log_msg("│              rupture command reference               │\n");
    log_msg("├────────────────────────┬─────────────────────────────┤\n");
    log_msg("│  help / h              │ show this help              │\n");
    log_msg("│  quit / q              │ exit rupture                │\n");
    log_msg("│  regs / r              │ print registers             │\n");
    log_msg("│  mem / x <addr> <n>   │ hex dump n bytes at addr    │\n");
    log_msg("│  break / b <addr|sym> │ set breakpoint              │\n");
    log_msg("│  bl / breakpoints      │ list breakpoints            │\n");
    log_msg("│  bc <n>               │ clear breakpoint n          │\n");
    log_msg("│  continue / c          │ continue execution          │\n");
    log_msg("│  step / s              │ single-step one instruction │\n");
    log_msg("│  sym <name>           │ look up symbol address      │\n");
    log_msg("│  syms                  │ list all function symbols   │\n");
    log_msg("└────────────────────────┴─────────────────────────────┘\n");
}

void repl_run(debugger_t *dbg)
{
    dbg_wait(dbg);
    tui_refresh(dbg);

    char line[256];
    while (1) {
        tui_get_input(line, sizeof(line));

        char copy[256];
        strncpy(copy, line, sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = '\0';

        char *cmd = strtok(copy, " \t\n");
        if (!cmd || cmd[0] == '\0')
            goto refresh;

        if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
            print_help();

        } else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
            break;

        } else if (strcmp(cmd, "regs") == 0 || strcmp(cmd, "r") == 0) {
            if (dbg->pid != -1)
                dbg_print_regs(dbg);
            else
                log_warn("no process running\n");
            tui_refresh_regs(dbg);

        } else if (strcmp(cmd, "mem") == 0 || strcmp(cmd, "x") == 0) {
            char *arg1 = strtok(NULL, " \t\n");
            char *arg2 = strtok(NULL, " \t\n");
            if (!arg1 || !arg2) {
                log_err("usage: mem <addr_hex> <n>\n");
            } else if (dbg->pid == -1) {
                log_warn("no process running\n");
            } else {
                uint64_t addr = strtoull(arg1, NULL, 16);
                size_t   n    = (size_t)strtoull(arg2, NULL, 0);
                dbg_mem_dump(dbg, addr, n);
            }

        } else if (strcmp(cmd, "break") == 0 || strcmp(cmd, "b") == 0) {
            char *arg = strtok(NULL, " \t\n");
            if (!arg) {
                log_err("usage: break <addr|symbol>\n");
            } else if (dbg->pid == -1) {
                log_warn("no process running\n");
            } else {
                char    *endptr = NULL;
                uint64_t addr   = strtoull(arg, &endptr, 16);
                if (endptr == arg || *endptr != '\0') {
                    addr = dbg_sym_lookup(dbg, arg);
                }
                if (addr != 0)
                    dbg_break_set(dbg, addr);
            }

        } else if (strcmp(cmd, "bl") == 0 || strcmp(cmd, "breakpoints") == 0) {
            dbg_break_list(dbg);

        } else if (strcmp(cmd, "bc") == 0) {
            char *arg = strtok(NULL, " \t\n");
            if (!arg) {
                log_err("usage: bc <n>\n");
            } else {
                int idx = (int)strtol(arg, NULL, 10);
                if (idx < 0 || idx >= dbg->bp_count) {
                    log_err("no breakpoint %d\n", idx);
                } else {
                    dbg_break_disable(dbg, &dbg->breakpoints[idx]);
                    log_warn("breakpoint %d disabled\n", idx);
                }
            }

        } else if (strcmp(cmd, "continue") == 0 || strcmp(cmd, "c") == 0) {
            if (dbg->pid == -1) {
                log_warn("no process running\n");
            } else {
                dbg_continue(dbg);
                if (dbg->pid != -1)
                    dbg_wait(dbg);
                tui_refresh(dbg);
            }

        } else if (strcmp(cmd, "step") == 0 || strcmp(cmd, "s") == 0) {
            if (dbg->pid == -1) {
                log_warn("no process running\n");
            } else {
                dbg_step(dbg);
                dbg_wait(dbg);
                if (dbg->pid != -1) {
                    for (int i = 0; i < dbg->bp_count; i++)
                        dbg_break_enable(dbg, &dbg->breakpoints[i]);
                }
                tui_refresh(dbg);
            }

        } else if (strcmp(cmd, "sym") == 0) {
            char *arg = strtok(NULL, " \t\n");
            if (!arg) {
                log_err("usage: sym <name>\n");
            } else {
                uint64_t addr = dbg_sym_lookup(dbg, arg);
                if (addr != 0)
                    log_ok("%s = 0x%016lx\n", arg, addr);
            }

        } else if (strcmp(cmd, "syms") == 0) {
            dbg_sym_list(dbg);

        } else {
            log_err("unknown command '%s' (type 'help' for list)\n", cmd);
        }

refresh:
        tui_refresh(dbg);
    }
}
