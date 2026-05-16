# rupture — Progress Tracker

| Feature                     | Status        | Notes                                              |
|-----------------------------|---------------|----------------------------------------------------|
| Process launch (fork/exec)  | done          | PTRACE_TRACEME + execvp in child                   |
| Register read/write         | done          | PTRACE_GETREGS / PTRACE_SETREGS                    |
| Memory hex dump             | done          | PTRACE_PEEKDATA, 16-byte rows with ASCII           |
| Breakpoints (INT3)          | done          | Enable/disable/find/set/list; step-over on resume  |
| Single-step                 | done          | PTRACE_SINGLESTEP + dbg_wait                       |
| ELF symbol lookup           | done          | Manual .symtab/.strtab parsing, no libelf          |
| ELF symbol list             | done          | Lists all STT_FUNC symbols with addresses          |
| ncurses TUI                 | done          | Three-pane layout + input line                     |
| Disassembly (capstone)      | done          | x86-64 Intel syntax, DISASM_LINES instructions     |
| Pluggable log backend       | done          | TUI backend set via log_set_backend                |
| DWARF source lines          | planned       | Phase 3                                            |
| Hardware watchpoints        | planned       | Phase 4, via DR0-DR7                               |
| Stack backtrace             | planned       | Phase 5, .eh_frame unwind                          |
| PIE / ASLR support          | planned       | Phase 6, /proc/pid/maps load bias                  |
| Command history (readline)  | planned       | Optional, requires -lreadline                      |
| Signal forwarding           | planned       | Pass non-SIGTRAP signals back to child             |
