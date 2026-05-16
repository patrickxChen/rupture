#pragma once
#include "debugger.h"

void tui_init(void);
void tui_cleanup(void);
void tui_refresh_regs(debugger_t *dbg);
void tui_refresh_disasm(debugger_t *dbg);
void tui_refresh(debugger_t *dbg);
void tui_get_input(char *buf, int len);
