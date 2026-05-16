#include <stdio.h>
#include <stdlib.h>
#include "debugger.h"
#include "tui.h"
#include "log.h"

void repl_run(debugger_t *dbg);

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: rupture <program>\n");
        return 1;
    }
    debugger_t dbg;
    dbg_init(&dbg, argv[1]);
    tui_init();
    log_ok("rupture v%s — x86-64 Linux debugger\n", RUPTURE_VERSION);
    log_msg("target: %s\n", argv[1]);
    dbg_launch(&dbg);
    repl_run(&dbg);
    tui_cleanup();
    dbg_cleanup(&dbg);
    return 0;
}
