# rupture

x86-64 Linux process debugger

```
┌─ registers ─────────────────────────────────────────────────────────┐
│ rip  0x0000000000401234  rsp  0x00007ffd2e8c3f80  rbp  0x00007ffd2e8c3f90 │
│ rax  0x0000000000000000  rbx  0x0000000000000000  rcx  0x0000000000000000 │
│ rdx  0x00007ffd2e8c40a8  rdi  0x0000000000000001  rsi  0x00007ffd2e8c4098 │
│ r8   0x0000000000000000  r9   0x0000000000000000  rflags 0x0000000000000246 │
└─────────────────────────────────────────────────────────────────────┘
┌─ disasm ────────────────────────────────────────────────────────────┐
│ => 0x0000000000401234  push   rbp                                   │
│    0x0000000000401235  mov    rbp, rsp                              │
│    0x0000000000401238  sub    rsp, 0x10                             │
│    0x000000000040123c  mov    dword ptr [rbp - 4], edi              │
│    0x000000000040123f  mov    dword ptr [rbp - 8], esi              │
└─────────────────────────────────────────────────────────────────────┘
┌─ output ────────────────────────────────────────────────────────────┐
│ rupture v0.1.0 — x86-64 Linux debugger                             │
│ target: ./tests/target                                              │
│ stopped: rip = 0x0000000000401234                                   │
│ => breakpoint hit at 0x0000000000401280                             │
└─────────────────────────────────────────────────────────────────────┘
rupture> _
```

## Install

**Dependencies:** gcc, libncurses-dev, libcapstone-dev

```bash
# Ubuntu / Debian
sudo apt-get install gcc libncurses-dev libcapstone-dev

# Build and install
make
sudo make install
```

Or use the provided script:

```bash
bash install.sh
```

## Usage

```bash
rupture ./your-binary
```

Compile your targets with `-g -no-pie` for best results (absolute symbol addresses, debug info):

```bash
gcc -g -O0 -no-pie -o target target.c
```

## Command Reference

| Command               | Description                              |
|-----------------------|------------------------------------------|
| `help` / `h`          | Show this command table                  |
| `quit` / `q`          | Exit rupture                             |
| `regs` / `r`          | Print all registers                      |
| `mem <addr> <n>`      | Hex dump n bytes at address (hex addr)   |
| `x <addr> <n>`        | Alias for mem                            |
| `break <addr\|sym>`   | Set breakpoint at address or symbol      |
| `b <addr\|sym>`       | Alias for break                          |
| `bl`                  | List all breakpoints                     |
| `breakpoints`         | Alias for bl                             |
| `bc <n>`              | Disable breakpoint n                     |
| `continue` / `c`      | Continue execution                       |
| `step` / `s`          | Single-step one instruction              |
| `sym <name>`          | Look up symbol address in ELF            |
| `syms`                | List all function symbols                |

## Architecture

```
rupture/
├── include/
│   ├── log.h          — log levels + pluggable backend
│   ├── debugger.h     — core types and function declarations
│   └── tui.h          — ncurses TUI function declarations
├── src/
│   ├── log.c          — log implementation
│   ├── debugger.c     — process launch, wait, continue, step
│   ├── registers.c    — PTRACE_GETREGS/SETREGS
│   ├── memory.c       — PTRACE_PEEKDATA/POKEDATA, hex dump
│   ├── breakpoint.c   — INT3 patching, enable/disable, step-over
│   ├── elf_parser.c   — manual ELF .symtab parsing
│   ├── tui.c          — ncurses TUI (capstone disasm)
│   └── repl.c         — command dispatch
├── tests/
│   ├── target.c       — test binary with multiple functions
│   └── Makefile
└── main.c
```

**Core dependencies:**
- `ptrace(2)` — Linux process control interface
- `ncurses` — terminal windowing
- `capstone` — x86-64 disassembly

## Roadmap

- DWARF source-level debugging (`.debug_info`, `.debug_line`)
- Hardware watchpoints via x86-64 debug registers (DR0–DR7)
- Stack backtrace via `.eh_frame` unwinding
- PIE/ASLR support via `/proc/pid/maps` load base detection
- Command history via readline

## Platform

Linux x86-64 only.
