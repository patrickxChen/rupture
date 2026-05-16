# Vibecoding rupture with Claude Code

Best practices for developing rupture incrementally with an AI assistant.

## 1. One chunk at a time

Implement one module, confirm it compiles and produces the expected ptrace behavior, then move to the next. Never add two new modules simultaneously when one depends on the other being correct.

## 2. Test with a real binary

Compile `tests/target` with `-g -O0 -no-pie` and use it for every test. Synthetic inputs do not expose real ptrace edge cases. Always verify:

```
rupture ./tests/target
```

Then set a breakpoint on `main`, `fib`, or `factorial` and continue.

## 3. Always `make clean && make`

rupture is compiled in a single-invocation Makefile. There are no object files, so a clean build is fast and avoids stale state confusion. Run this before reporting any build error.

## 4. Use gdb as ground truth

When rupture gives a wrong register value, wrong disassembly, or a breakpoint miss, attach gdb to the same binary at the same address:

```
gdb ./tests/target
(gdb) b fib
(gdb) run
```

Compare gdb's output to rupture's. If gdb agrees with the kernel, the bug is in rupture.

## 5. dbg_log is the only print mechanism

Never add raw `printf` or `fprintf` calls to any source file in `src/`. All diagnostic output must go through `dbg_log` (or the `log_msg`, `log_ok`, `log_warn`, `log_err` macros). This ensures the TUI backend intercepts all output correctly.

## 6. New commands belong only in repl.c

The command dispatch in `repl_run` is the single place where user input is translated into debugger actions. Never call `ptrace` directly from `repl.c` — call the appropriate `dbg_*` function. This keeps the REPL as a thin routing layer.

## 7. Understand the breakpoint state machine

```
set → enabled
hit → disabled (INT3 removed), rip rewound, bp->hit = true
continue → SINGLESTEP one insn, re-enable, PTRACE_CONT
```

If you skip the step-over, the INT3 fires again immediately on the next PTRACE_CONT. If you re-enable before the step, execution never reaches the original instruction. The state machine in `dbg_continue` is critical — do not simplify it.

## 8. Read /proc/pid/maps before adding PIE support

PIE binaries have symbol addresses relative to the load base. The load base is determined at runtime by the kernel/dynamic linker. Before `dbg_sym_lookup` can return useful addresses for PIE, you need:

```c
uint64_t base = read_load_base_from_proc_maps(dbg->pid, dbg->target);
return sym->st_value + base;
```

## 9. Memory writes require the child to be stopped

`PTRACE_POKEDATA` fails silently or gives wrong results if the child is running. Always ensure `dbg_wait` has returned (child is in `WIFSTOPPED` state) before calling `dbg_mem_write_word` or `dbg_break_enable`.

## 10. Never skip dbg_wait after dbg_step

After `dbg_step` issues `PTRACE_SINGLESTEP`, the child is running for exactly one instruction. Until `dbg_wait` returns, any ptrace call (GETREGS, PEEKDATA, etc.) will fail with `ESRCH` or return stale data. The REPL always calls `dbg_wait` immediately after `dbg_step` — do not remove this call.
