#include <string.h>
#include <stdint.h>
#include <ncurses.h>
#include <capstone/capstone.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include "tui.h"
#include "debugger.h"
#include "log.h"

#define REGS_ROWS    6
#define DISASM_ROWS  12
#define DISASM_LINES 10
#define DISASM_BYTES (DISASM_LINES * 15)

#define COL_OK   1
#define COL_ERR  2
#define COL_WARN 3
#define COL_ADDR 4
#define COL_CUR  5
#define COL_DIM  6

static WINDOW *win_regs;
static WINDOW *win_disasm;
static WINDOW *win_output;
static WINDOW *win_input;

static void tui_log_backend(log_level_t level, const char *msg)
{
    switch (level) {
    case LOG_OK:
        wattron(win_output, COLOR_PAIR(COL_OK));
        break;
    case LOG_ERR:
        wattron(win_output, COLOR_PAIR(COL_ERR));
        break;
    case LOG_WARN:
        wattron(win_output, COLOR_PAIR(COL_WARN));
        break;
    default:
        wattron(win_output, COLOR_PAIR(COL_DIM));
        break;
    }
    wprintw(win_output, "%s", msg);
    switch (level) {
    case LOG_OK:
        wattroff(win_output, COLOR_PAIR(COL_OK));
        break;
    case LOG_ERR:
        wattroff(win_output, COLOR_PAIR(COL_ERR));
        break;
    case LOG_WARN:
        wattroff(win_output, COLOR_PAIR(COL_WARN));
        break;
    default:
        wattroff(win_output, COLOR_PAIR(COL_DIM));
        break;
    }
    wrefresh(win_output);
}

void tui_init(void)
{
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    use_default_colors();

    init_pair(COL_OK,   COLOR_GREEN,  -1);
    init_pair(COL_ERR,  COLOR_RED,    -1);
    init_pair(COL_WARN, COLOR_YELLOW, -1);
    init_pair(COL_ADDR, COLOR_CYAN,   -1);
    init_pair(COL_CUR,  COLOR_GREEN,  -1);
    init_pair(COL_DIM,  COLOR_WHITE,  -1);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int output_rows = rows - REGS_ROWS - DISASM_ROWS - 1;
    if (output_rows < 3)
        output_rows = 3;

    win_regs   = newwin(REGS_ROWS,   cols, 0,                        0);
    win_disasm = newwin(DISASM_ROWS, cols, REGS_ROWS,                 0);
    win_output = newwin(output_rows, cols, REGS_ROWS + DISASM_ROWS,   0);
    win_input  = newwin(1,           cols, rows - 1,                  0);

    scrollok(win_output, TRUE);

    log_set_backend(tui_log_backend);

    refresh();
}

void tui_cleanup(void)
{
    endwin();
}

void tui_refresh_regs(debugger_t *dbg)
{
    werase(win_regs);
    box(win_regs, 0, 0);
    wattron(win_regs, A_BOLD);
    mvwprintw(win_regs, 0, 2, " registers ");
    wattroff(win_regs, A_BOLD);

    if (dbg->pid == -1) {
        mvwprintw(win_regs, 1, 2, "no process");
        wrefresh(win_regs);
        return;
    }

    struct user_regs_struct r = dbg_get_regs_raw(dbg);

    wattron(win_regs, COLOR_PAIR(COL_ADDR));
    mvwprintw(win_regs, 1, 2,
              "rip  0x%016llx  rsp  0x%016llx  rbp  0x%016llx",
              r.rip, r.rsp, r.rbp);
    mvwprintw(win_regs, 2, 2,
              "rax  0x%016llx  rbx  0x%016llx  rcx  0x%016llx",
              r.rax, r.rbx, r.rcx);
    mvwprintw(win_regs, 3, 2,
              "rdx  0x%016llx  rdi  0x%016llx  rsi  0x%016llx",
              r.rdx, r.rdi, r.rsi);
    mvwprintw(win_regs, 4, 2,
              "r8   0x%016llx  r9   0x%016llx  rflags 0x%016llx",
              r.r8, r.r9, r.eflags);
    wattroff(win_regs, COLOR_PAIR(COL_ADDR));

    wrefresh(win_regs);
}

void tui_refresh_disasm(debugger_t *dbg)
{
    werase(win_disasm);
    box(win_disasm, 0, 0);
    wattron(win_disasm, A_BOLD);
    mvwprintw(win_disasm, 0, 2, " disasm ");
    wattroff(win_disasm, A_BOLD);

    if (dbg->pid == -1) {
        mvwprintw(win_disasm, 1, 2, "no process");
        wrefresh(win_disasm);
        return;
    }

    uint64_t rip = dbg_get_rip(dbg);

    uint8_t code[DISASM_BYTES];
    memset(code, 0x90, sizeof(code));
    for (size_t i = 0; i < sizeof(code); i += 8) {
        uint64_t word = dbg_mem_read_word(dbg, rip + i);
        memcpy(code + i, &word, 8);
    }

    csh handle;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        mvwprintw(win_disasm, 1, 2, "capstone init failed");
        wrefresh(win_disasm);
        return;
    }
    cs_option(handle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL);

    cs_insn *insns = NULL;
    size_t count = cs_disasm(handle, code, sizeof(code), rip, DISASM_LINES, &insns);

    int row = 1;
    for (size_t i = 0; i < count && row < DISASM_ROWS - 1; i++, row++) {
        bool is_current = (insns[i].address == rip);

        if (is_current) {
            wattron(win_disasm, COLOR_PAIR(COL_CUR) | A_BOLD);
            mvwprintw(win_disasm, row, 2, "=>");
            wattroff(win_disasm, COLOR_PAIR(COL_CUR) | A_BOLD);
        } else {
            mvwprintw(win_disasm, row, 2, "  ");
        }

        wattron(win_disasm, COLOR_PAIR(COL_ADDR));
        mvwprintw(win_disasm, row, 5, "0x%016lx", insns[i].address);
        wattroff(win_disasm, COLOR_PAIR(COL_ADDR));

        if (is_current)
            wattron(win_disasm, COLOR_PAIR(COL_CUR) | A_BOLD);
        mvwprintw(win_disasm, row, 24, "  %-10s %s",
                  insns[i].mnemonic, insns[i].op_str);
        if (is_current)
            wattroff(win_disasm, COLOR_PAIR(COL_CUR) | A_BOLD);
    }

    if (insns)
        cs_free(insns, count);
    cs_close(&handle);

    wrefresh(win_disasm);
}

void tui_refresh(debugger_t *dbg)
{
    tui_refresh_regs(dbg);
    tui_refresh_disasm(dbg);
}

void tui_get_input(char *buf, int len)
{
    werase(win_input);
    wattron(win_input, A_BOLD | COLOR_PAIR(COL_OK));
    wprintw(win_input, "rupture> ");
    wattroff(win_input, A_BOLD | COLOR_PAIR(COL_OK));
    wrefresh(win_input);

    int pos = 0;
    memset(buf, 0, (size_t)len);

    while (1) {
        int ch = wgetch(win_input);

        if (ch == '\n' || ch == '\r') {
            buf[pos] = '\0';
            wattron(win_output, COLOR_PAIR(COL_DIM));
            wprintw(win_output, "> %s\n", buf);
            wattroff(win_output, COLOR_PAIR(COL_DIM));
            wrefresh(win_output);
            return;
        }

        if (ch == 3) {
            strncpy(buf, "quit", (size_t)len - 1);
            buf[len - 1] = '\0';
            return;
        }

        if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            if (pos > 0) {
                pos--;
                buf[pos] = '\0';
                werase(win_input);
                wattron(win_input, A_BOLD | COLOR_PAIR(COL_OK));
                wprintw(win_input, "rupture> ");
                wattroff(win_input, A_BOLD | COLOR_PAIR(COL_OK));
                wprintw(win_input, "%s", buf);
                wrefresh(win_input);
            }
            continue;
        }

        if (ch >= 32 && ch < 127 && pos < len - 1) {
            buf[pos++] = (char)ch;
            buf[pos]   = '\0';
            waddch(win_input, (chtype)ch);
            wrefresh(win_input);
        }
    }
}
