#include <string.h>
#include "debugger.h"
#include "log.h"

void dbg_break_enable(debugger_t *dbg, breakpoint_t *bp)
{
    if (bp->enabled)
        return;
    uint64_t patched = (bp->orig_word & ~(uint64_t)0xFF) | 0xCC;
    dbg_mem_write_word(dbg, bp->addr, patched);
    bp->enabled = true;
}

void dbg_break_disable(debugger_t *dbg, breakpoint_t *bp)
{
    if (!bp->enabled)
        return;
    dbg_mem_write_word(dbg, bp->addr, bp->orig_word);
    bp->enabled = false;
}

breakpoint_t *dbg_break_find(debugger_t *dbg, uint64_t addr)
{
    for (int i = 0; i < dbg->bp_count; i++) {
        if (dbg->breakpoints[i].addr == addr)
            return &dbg->breakpoints[i];
    }
    return NULL;
}

void dbg_break_set(debugger_t *dbg, uint64_t addr)
{
    if (dbg->bp_count >= MAX_BREAKPOINTS) {
        log_err("max breakpoints (%d) reached\n", MAX_BREAKPOINTS);
        return;
    }
    if (dbg_break_find(dbg, addr)) {
        log_warn("breakpoint already set at 0x%016lx\n", addr);
        return;
    }
    breakpoint_t *bp = &dbg->breakpoints[dbg->bp_count];
    memset(bp, 0, sizeof(*bp));
    bp->addr      = addr;
    bp->orig_word = dbg_mem_read_word(dbg, addr);
    bp->enabled   = false;
    bp->hit       = false;
    dbg->bp_count++;
    dbg_break_enable(dbg, bp);
    log_ok("breakpoint set at 0x%016lx\n", addr);
}

void dbg_break_list(debugger_t *dbg)
{
    if (dbg->bp_count == 0) {
        log_msg("no breakpoints\n");
        return;
    }
    for (int i = 0; i < dbg->bp_count; i++) {
        breakpoint_t *bp = &dbg->breakpoints[i];
        log_msg("[%d]  0x%016lx  %s\n", i, bp->addr,
                bp->enabled ? "enabled" : "disabled");
    }
}
