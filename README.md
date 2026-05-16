# rupture

A terminal debugger for Linux programs. Set breakpoints, inspect registers, read memory, and step through code — all in a live terminal UI.

```
┌─ registers ────────────────────────────────────────────────────────────────┐
│ rip  0x0000000000401136  rsp  0x00007ffd3c2a1b80  rbp  0x0000000000000000  │
│ rax  0x0000000000000000  rbx  0x0000000000000000  rcx  0x0000000000000000  │
│ rdx  0x0000000000000000  rdi  0x0000000000000001  rsi  0x00007ffd3c2a1c98  │
│ r8   0x0000000000000000  r9   0x0000000000000000  rflags  0x0000000000000246│
└────────────────────────────────────────────────────────────────────────────┘
┌─ disasm ───────────────────────────────────────────────────────────────────┐
│ => 0x0000000000401136  push   rbp                                          │
│    0x0000000000401137  mov    rbp, rsp                                     │
│    0x000000000040113a  sub    rsp, 0x10                                    │
│    0x000000000040113e  mov    dword ptr [rbp - 4], edi                     │
│    0x0000000000401141  mov    qword ptr [rbp - 0x10], rsi                  │
└────────────────────────────────────────────────────────────────────────────┘
┌─ output ───────────────────────────────────────────────────────────────────┐
│ rupture v0.1.0 — x86-64 Linux debugger                                    │
│ target: ./tests/target                                                     │
│ => breakpoint hit at 0x0000000000401136                                    │
└────────────────────────────────────────────────────────────────────────────┘
rupture> _
```

## Install

You need Linux (x86-64), gcc, and two libraries:

```bash
sudo apt install gcc libncurses-dev libcapstone-dev
```

Then build and install:

```bash
git clone https://github.com/patrickxChen/rupture
cd rupture
make
sudo make install
```

## Usage

```bash
rupture ./your-program
```

For the best experience, compile your program like this so rupture can find function names:

```bash
gcc -g -O0 -no-pie -o myapp myapp.c
rupture ./myapp
```

## Commands

| Command | What it does |
|---|---|
| `break main` | Set a breakpoint at a function by name |
| `break 0x401234` | Set a breakpoint at an address |
| `continue` / `c` | Run until the next breakpoint |
| `step` / `s` | Execute one instruction |
| `regs` / `r` | Show all registers |
| `mem 0x401000 64` | Dump 64 bytes of memory starting at an address |
| `syms` | List all functions in the binary |
| `sym main` | Look up the address of a specific function |
| `bl` | List all breakpoints |
| `bc 0` | Disable breakpoint #0 |
| `quit` / `q` | Exit |

## Quick example

```
rupture> break main
  main -> 0x0000000000401136
  breakpoint set at 0x0000000000401136
rupture> continue
=> breakpoint hit at 0x0000000000401136
rupture> step
  stopped: rip = 0x0000000000401137
rupture> regs
  rip  0x0000000000401137  rsp  0x00007ffd3c2a1b78
  ...
rupture> mem 0x401136 32
  0x0000000000401136  55 48 89 e5 48 83 ec 10  89 7d fc 48 89 75 f0 48  |UH..H....}.H.u.H|
  ...
rupture> quit
```

## Limitations

- Linux only (x86-64)
- If `break main` says "symbol not found", compile with `-g -no-pie` (see above)
- If you've removed debug info from your binary, use `break 0x401234` with an address instead of a name

## What's planned

- Show source code lines while stepping (requires `-g`)
- Watchpoints (break when a variable changes)
- Stack backtrace (`bt` command)
- Command history with arrow keys
