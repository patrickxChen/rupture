#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include "debugger.h"
#include "log.h"

uint64_t dbg_mem_read_word(debugger_t *dbg, uint64_t addr)
{
    errno = 0;
    long val = ptrace(PTRACE_PEEKDATA, dbg->pid, (void *)addr, NULL);
    if (val == -1 && errno != 0)
        log_err("ptrace(PEEKDATA) at 0x%016lx: %s\n", addr, strerror(errno));
    return (uint64_t)val;
}

void dbg_mem_write_word(debugger_t *dbg, uint64_t addr, uint64_t val)
{
    if (ptrace(PTRACE_POKEDATA, dbg->pid, (void *)addr, (void *)val) < 0)
        log_err("ptrace(POKEDATA) at 0x%016lx: %s\n", addr, strerror(errno));
}

void dbg_mem_dump(debugger_t *dbg, uint64_t addr, size_t n)
{
    if (n > 4096)
        n = 4096;

    uint64_t aligned = addr & ~(uint64_t)7;
    size_t   offset  = (size_t)(addr - aligned);
    size_t   total   = offset + n;
    if (total % 8)
        total = (total / 8 + 1) * 8;

    uint8_t buf[4096 + 8];
    size_t  gathered = 0;
    for (size_t i = 0; i < total; i += 8) {
        uint64_t word = dbg_mem_read_word(dbg, aligned + i);
        memcpy(buf + i, &word, 8);
        gathered += 8;
        if (gathered >= sizeof(buf))
            break;
    }

    for (size_t row = 0; row < total; row += 16) {
        char line[128];
        int  pos = 0;
        pos += snprintf(line + pos, sizeof(line) - pos,
                        "0x%016lx  ", aligned + row);

        for (int col = 0; col < 16; col++) {
            size_t idx = row + col;
            if (col == 8)
                pos += snprintf(line + pos, sizeof(line) - pos, " ");
            if (idx < total)
                pos += snprintf(line + pos, sizeof(line) - pos,
                                "%02x ", buf[idx]);
            else
                pos += snprintf(line + pos, sizeof(line) - pos, "   ");
        }

        pos += snprintf(line + pos, sizeof(line) - pos, "  |");
        for (int col = 0; col < 16; col++) {
            size_t idx = row + col;
            if (idx < total) {
                uint8_t c = buf[idx];
                pos += snprintf(line + pos, sizeof(line) - pos,
                                "%c", (c >= 0x20 && c < 0x7f) ? c : '.');
            } else {
                pos += snprintf(line + pos, sizeof(line) - pos, " ");
            }
        }
        pos += snprintf(line + pos, sizeof(line) - pos, "|");
        log_msg("%s\n", line);
    }
}
