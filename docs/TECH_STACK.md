# rupture — Tech Stack

## Language

**C11** compiled with `gcc -Wall -Wextra -g -O0`. No C++ or third-party frameworks. Standard POSIX interfaces throughout.

## ptrace API

The Linux kernel's `ptrace(2)` syscall is the foundation of rupture. Key operations used:

| ptrace request          | Purpose                                        |
|-------------------------|------------------------------------------------|
| `PTRACE_TRACEME`        | Child opts into tracing before exec            |
| `PTRACE_GETREGS`        | Read all general-purpose registers             |
| `PTRACE_SETREGS`        | Write registers (used to rewind RIP)           |
| `PTRACE_PEEKDATA`       | Read one 8-byte word from child memory         |
| `PTRACE_POKEDATA`       | Write one 8-byte word to child memory          |
| `PTRACE_CONT`           | Resume child until next stop                   |
| `PTRACE_SINGLESTEP`     | Execute exactly one instruction then stop      |
| `PTRACE_KILL`           | Terminate the traced child                     |

## ncurses

Terminal UI windowing library. Used for the three-pane layout (registers / disasm / output) plus the single-line input prompt. Linked as `-lncurses` (package: `libncurses-dev`).

## capstone

Multi-architecture disassembly engine. rupture uses:
- `cs_open(CS_ARCH_X86, CS_MODE_64, &handle)` — open an x86-64 handle
- `cs_option(..., CS_OPT_SYNTAX_INTEL)` — Intel syntax output
- `cs_disasm(handle, code, size, addr, count, &insns)` — disassemble a block

Linked as `-lcapstone` (package: `libcapstone-dev`).

## ELF parsing

Manual parsing of the ELF64 binary format using `<elf.h>` (kernel headers). No dependency on libelf or libbfd. rupture reads:
- `Elf64_Ehdr` — file header (type, entry point, section header offset)
- `Elf64_Shdr` — section headers (type, name, offset, size)
- `Elf64_Sym` — symbol table entries (name, value, type)

The `.symtab` section provides function addresses for non-stripped binaries. The `.strtab` section provides the corresponding name strings.

## Build System

**GNU Make**. Single top-level `Makefile` compiles all `.c` files in one `gcc` invocation for simplicity. Tests are in a separate `tests/Makefile` built via `$(MAKE) -C tests`.

## CI

**GitHub Actions** on `ubuntu-latest`. The workflow installs `libncurses-dev` and `libcapstone-dev`, runs `make`, and runs `make test-target`.

## Target Platform

**Linux x86-64 only.** The code depends on:
- Linux-specific `ptrace(2)` and `waitpid(2)` semantics
- `struct user_regs_struct` from `<sys/user.h>` (x86-64 layout)
- `SIGTRAP` delivery model for breakpoints and single-step
- ELF64 binary format
