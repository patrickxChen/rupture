# rupture — Implementation Plan

## Phase 1 — Core Engine (complete)

Process control, register inspection, memory access, breakpoints, single-step, and ELF symbol parsing via ptrace and manual ELF reading.

### Deliverables
- `src/debugger.c` — fork/exec child, PTRACE_TRACEME, wait loop
- `src/registers.c` — PTRACE_GETREGS/SETREGS
- `src/memory.c` — PTRACE_PEEKDATA/POKEDATA, hex dump
- `src/breakpoint.c` — INT3 patching, enable/disable, step-over
- `src/elf_parser.c` — manual ELF64 .symtab parsing

## Phase 2 — ncurses TUI (complete)

Full terminal UI with three panes: registers, disassembly (via capstone), and scrolling output. Single-line input at the bottom.

### Deliverables
- `src/tui.c` — window layout, color pairs, refresh functions, input loop
- `src/repl.c` — command dispatch connected to TUI

## Phase 3 — DWARF Source-Level Debugging (planned)

Parse `.debug_info` and `.debug_line` to map addresses to source file/line. Show the current source line in a new pane above disasm.

### Deliverables
- `src/dwarf.c` — DWARF line-number program interpreter
- New TUI pane: source lines centered on current PC

## Phase 4 — Watchpoints (planned)

Use x86-64 debug registers (DR0–DR7) via `PTRACE_POKEUSER` to set hardware watchpoints on memory addresses without patching code.

### Deliverables
- `src/watchpoint.c` — set/clear DR0–DR3, configure DR7
- New REPL commands: `watch <addr>`, `wl`, `wc <n>`

## Phase 5 — Stack Unwinding (planned)

Parse `.eh_frame` / `.debug_frame` to produce a full backtrace without frame pointers.

### Deliverables
- `src/unwind.c` — CIE/FDE parser, CFI interpreter
- New REPL command: `bt` (backtrace)

## Phase 6 — PIE + ASLR Support (planned)

Read `/proc/<pid>/maps` to determine the actual load base of PIE binaries, and adjust all symbol addresses accordingly.

### Deliverables
- `src/proc_maps.c` — parse `/proc/pid/maps`, find load bias
- Adjust `dbg_sym_lookup` / `dbg_sym_list` to add bias automatically

---

## Command Reference

| Command               | Status  | Description                              |
|-----------------------|---------|------------------------------------------|
| `help` / `h`          | done    | Print command table                      |
| `quit` / `q`          | done    | Exit rupture                             |
| `regs` / `r`          | done    | Print all registers                      |
| `mem` / `x <a> <n>`   | done    | Hex dump n bytes at address a            |
| `break` / `b <a>`     | done    | Set breakpoint at address or symbol name |
| `bl` / `breakpoints`  | done    | List all breakpoints                     |
| `bc <n>`              | done    | Disable breakpoint n                     |
| `continue` / `c`      | done    | Continue execution                       |
| `step` / `s`          | done    | Single-step one instruction              |
| `sym <name>`          | done    | Look up symbol address                   |
| `syms`                | done    | List all function symbols                |
| `bt`                  | planned | Print stack backtrace                    |
| `watch <addr>`        | planned | Set hardware watchpoint                  |
| `wl`                  | planned | List watchpoints                         |
| `wc <n>`              | planned | Clear watchpoint n                       |
| `src`                 | planned | Show current source line                 |
