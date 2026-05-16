#pragma once
#include <sys/types.h>
#include <sys/user.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define RUPTURE_VERSION "0.1.0"
#define MAX_BREAKPOINTS 64

typedef struct {
    uint64_t addr;
    uint64_t orig_word;
    bool     enabled;
    bool     hit;
} breakpoint_t;

typedef struct {
    pid_t        pid;
    const char  *target;
    breakpoint_t breakpoints[MAX_BREAKPOINTS];
    int          bp_count;
    bool         running;
} debugger_t;

/* debugger.c */
void dbg_init(debugger_t *dbg, const char *target);
void dbg_launch(debugger_t *dbg);
void dbg_cleanup(debugger_t *dbg);
void dbg_wait(debugger_t *dbg);
void dbg_continue(debugger_t *dbg);
void dbg_step(debugger_t *dbg);

/* registers.c */
struct user_regs_struct dbg_get_regs_raw(debugger_t *dbg);
void     dbg_print_regs(debugger_t *dbg);
uint64_t dbg_get_rip(debugger_t *dbg);
void     dbg_set_rip(debugger_t *dbg, uint64_t rip);

/* memory.c */
uint64_t dbg_mem_read_word(debugger_t *dbg, uint64_t addr);
void     dbg_mem_write_word(debugger_t *dbg, uint64_t addr, uint64_t val);
void     dbg_mem_dump(debugger_t *dbg, uint64_t addr, size_t n);

/* breakpoint.c */
void          dbg_break_enable(debugger_t *dbg, breakpoint_t *bp);
void          dbg_break_disable(debugger_t *dbg, breakpoint_t *bp);
breakpoint_t *dbg_break_find(debugger_t *dbg, uint64_t addr);
void          dbg_break_set(debugger_t *dbg, uint64_t addr);
void          dbg_break_list(debugger_t *dbg);

/* elf_parser.c */
uint64_t dbg_sym_lookup(debugger_t *dbg, const char *name);
void     dbg_sym_list(debugger_t *dbg);
