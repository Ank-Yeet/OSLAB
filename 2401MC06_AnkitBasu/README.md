# Operating System Lab Examination (MA3105)

- **Name:** Ankit Basu
- **Roll Number:** 2401MC06
- **Set Number:** 1

## Build and Execution

Build and run xv6 with 1 CPU:
```sh
make qemu-nox CPUS=1
```

Inside the xv6 shell, run:
- `synctest` (Question 1)
- `memtest` (Question 2)

Added system calls use identifiers as asked in the questions(`sem_init`, `sem_wait`, `sem_post`, `freepages`, `va2pa`).

---

## Question 1: Counting Semaphores for Process Synchronization

- Added `sem.c` defining an array of 8 semaphores (`struct sem { int value; int inuse; }`), protected by a spinlock `semtable.lock`; `seminit()` initializes the spinlock and table entries at boot.
- Implemented `sem_init`, `sem_wait`, and `sem_post` with validation (`id` between 0 and 7, `value >= 0`), returning `-1` for invalid IDs or uninitialized semaphores.
- Handled blocking via `sleep(&semtable.sems[id], &semtable.lock)` in a `while (value == 0)` loop to recheck condition after wakeups; `sem_post` increments `value` and calls `wakeup()` while holding the lock.
- Added `seminit()` invocation in `main.c`, declared prototypes in `defs.h`, added system call wrappers in `sysproc.c`, registered dispatch entries in `syscall.h`, `syscall.c`, `user.h`, and `usys.S`, and updated `Makefile` (`OBJS` and `UPROGS`).
- Created `synctest.c`: parent initializes semaphores 0, 1, 2 to values 1, 0, 0, and forks 3 children printing `A `, `B `, `C ` 5 times strictly in order. The parent reaps all children with `wait()`, prints `Done`, and tests boundary/uninitialized error handling.

---

## Question 2: Memory Statistics and Address Translation

- Added `free_pages` tracking to `struct kmem` in `kalloc.c`, incremented in `kfree()` and decremented in `kalloc()` while holding `kmem.lock`; exposed to user space through `freepages()`.
- Implemented `va2pa(int va)` in `vm.c` using `walkpgdir(myproc()->pgdir, (void*)va, 0)`. Checks `PTE_P` and `PTE_U` flags and returns `PTE_ADDR(*pte) | (va & 0xFFF)`, or `-1` for unmapped or supervisor-only addresses.
- Declared `freepages()` and `va2pa()` in `defs.h`, implemented `sys_freepages()` and `sys_va2pa()` in `sysproc.c`, and added entries to `syscall.h`, `syscall.c`, `user.h`, `usys.S`, and `Makefile`.
- Created `memtest.c` to test: (i) `freepages()` at startup, (ii) reduction by 10 pages after `sbrk(10*4096)`, (iii) VA and PA of a global variable, (iv) `va2pa(0x7FFFFFFF) == -1`, and (v) distinct PAs for the same global VA across parent and child after `fork()`, with free pages returning to the exact pre-fork count after `wait()` (no leaks).

---

## Output Evidence

- **Question 1:** `Q1_Output.txt` / `Q1_Output.png` (roll number echoed before test)
- **Question 2:** `Q2_Output.txt` / `Q2_Output.png` (roll number echoed before test)
