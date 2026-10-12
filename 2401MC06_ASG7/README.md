# OS Lab Assignment 7 — xv6: Threads, Semaphores, Deadlock Detection and Priority Scheduling

This assignment extends the xv6 (x86) teaching kernel with four features, each with a user-space test program:

| Question | Feature | Kernel changes | Test program |
|---|---|---|---|
| Q1 | Kernel-supported threads that share one address space (`clone`, `join`) | `proc.c`, `proc.h`, `sysproc.c` | `threadtest.c` |
| Q2 | Counting semaphores (`sem_init`, `sem_wait`, `sem_signal`) and a producer–consumer demo | `proc.c`, `param.h`, `sysproc.c` | `synctest.c` |
| Q3 | Deadlock detection and the Banker's algorithm | none (user space only) | `deadlock.c` |
| Q4 | Priority-based preemptive scheduler with aging | `proc.c`, `proc.h`, `trap.c`, `param.h` | `priotest.c` |

The design, results and problems encountered are written up in **`REPORT.pdf`**.

## Contents

```
kernel/    proc.c  proc.h  sysproc.c  syscall.c  syscall.h  defs.h  param.h  trap.c
user/      threadtest.c  synctest.c  deadlock.c  priotest.c  user.h  usys.S
Makefile
output_logs/   terminal output of every run (see "Output logs")
figures/       q4_scheduling.png
REPORT.pdf
README.md
```

These are the files that differ from stock xv6; everything else comes from the base xv6 source tree.

## Building and running

You need an xv6 (x86) source tree, `gcc` (32-bit capable), `make` and `qemu-system-i386`.

1. Copy the files in `kernel/` and `user/` into the root of the xv6 source tree, and replace its `Makefile` with the one provided.
2. Build and boot:

   ```
   make clean
   make qemu-nox-2cpu      # 2 CPUs: use for Q1, Q2 and Q3
   make qemu-nox-1cpu      # 1 CPU:  use for Q4
   ```

   `make qemu-nox` and `make qemu-nox CPUS=1` are equivalent to the two shortcuts. The default is 2 CPUs.
3. At the xv6 shell (`$`), run `threadtest`, `synctest`, `deadlock` or `priotest`. Quit QEMU with `Ctrl-A` then `X`.

**Why two CPUs.** The Q1 race needs two threads running at the same instant, so it needs at least two CPUs. The Makefile passes `-smp N,sockets=N` to QEMU; without `sockets=N`, recent QEMU versions made xv6 see only one CPU. You should see `cpu1: starting 1` at boot. **Why one CPU for Q4.** With two CPUs the two highest-priority jobs run in parallel, so completion order says less about priority.

## System calls added

| No. | Call | Behaviour | Returns |
|---|---|---|---|
| 23 | `sem_init(int id, int value)` | initialize semaphore `id` (0–9) | 0, or −1 if `id` is out of range |
| 24 | `sem_wait(int id)` | decrement; block if the value goes below 0 | 0, or −1 |
| 25 | `sem_signal(int id)` | increment; wake one waiter if any | 0, or −1 |
| 26 | `clone(void (*fcn)(void*), void *arg, void *stack)` | new thread sharing the caller's address space, running `fcn(arg)` on `stack` | thread pid, or −1 |
| 27 | `join(void)` | wait for a thread created by `clone` and release it | pid of the thread, or −1 if there are none |
| 28 | `set_priority(int pid, int priority)` | priority must be 1 (highest) to 20 (lowest) | 0, or −1 |
| 29 | `get_priority(int pid)` | read a process's current priority | priority, or −1 for an unknown pid |

`shm_get` (22) is also present in the supplied kernel files but none of the four programs use it. If you build on a tree that does not define it, remove `SYS_shm_get`, `sys_shm_get` (in `sysproc.c` and `syscall.c`) and the `shm_get` lines in `usys.S` and `user.h`.

## Test programs

**`threadtest`** (Q1). Creates 3 threads with `clone()`; each increments a shared `volatile int` counter 100,000 times. The parent releases all threads at once with a `go` flag, calls `join()` three times and prints the final value and the number of lost updates. Thread stacks are static, page-aligned buffers declared before any `clone()`. There is no synchronization, so the result is less than 300,000.

**`synctest`** (Q2). Part 1 repeats `threadtest` with `sem_wait`/`sem_signal` around `counter++` and prints `CORRECT` if the total is exactly 300,000. Part 2 is the producer–consumer problem: one producer and one consumer thread share a 5-slot circular buffer, controlled by semaphores `EMPTY` (starts at 5), `FULL` (0) and `BUFMUTEX` (1). They exchange 20 items and every step is printed with the buffer count. Overflow and underflow flags are checked at the end.

**`deadlock`** (Q3). Uses static arrays for up to 5 processes and 3 resource types and runs five tests:

1. Detection on the textbook matrix: no deadlock.
2. The same matrix with one extra request: deadlock among P1, P2, P3, P4.
3. Banker's algorithm: safe state, then a safe request is granted.
4. Banker's algorithm: a request that would make the state unsafe is denied and rolled back.
5. A real deadlock between two xv6 processes and two pipes (A holds Pipe1 and waits on Pipe2, B holds Pipe2 and waits on Pipe1). The program lets 100 ticks pass, runs `detect_deadlock()` on the resulting matrices, then kills both processes.

Available, Allocation, Max/Request and Need are printed at each step.

**`priotest`** (Q4). Forks 4 CPU-bound children with priorities 2, 6, 12 and 20. A monitor process prints each aging step of the priority-20 child. When all children finish, it prints the completion order with the finishing tick of each.

## Expected results (from the logged runs)

| Test | Result |
|---|---|
| `threadtest`, 3 runs | 271,794 / 211,379 / 223,534 (not 300,000; varies from run to run) |
| `synctest` part 1 | 300,000 (exact) |
| `synctest` part 2 | 20 items in order, `final count=0, overflow=no, underflow=no` |
| `deadlock` | tests 1–5 give no deadlock / deadlock / safe + granted / unsafe + denied / circular wait detected |
| `priotest`, 1 CPU | order priority 2, 6, 12, 20 (about 147 / 293 / 439 / 581 ticks); the priority-20 child ages 20 → 6 and still finishes last, so it does not starve |

Exact tick counts and thread counters change slightly from run to run.

## Output logs

`output_logs/full_session_2cpu.txt` is one continuous 2-CPU boot containing all four programs in order. `q4_priotest_1cpu.txt` is a separate 1-CPU boot. The per-question files for Q1–Q3 and the 2-CPU Q4 run are excerpts of the full session, so process ids continue from one program to the next.

## Known limitations

- A process must `join()` all of its threads before it exits. If it exits first, its page directory is freed when its parent reaps it, while the threads may still be running.
- Threads must not call `sbrk` or `malloc`. Each thread keeps its own copy of the memory size, so heap growth is not shared. Allocate all stacks (at least 4096 bytes each) before calling `clone()`.
- Calling `sem_init` on a semaphore that has sleeping processes is not supported.
- Aging is permanent: a process that has been aged does not return to its original priority.

See `REPORT.pdf` for the full design discussion.
