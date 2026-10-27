# Operating Systems Assignment 9: Virtual Memory & Page Replacement Report

## Overview
This report details the implementation and experimental results for Operating Systems Assignment 9 on the 32-bit x86 xv6 operating system. The assignment covers three key memory management concepts: inspecting process address space mapping via page tables, on-demand lazy memory allocation using page faults, and page replacement algorithm simulations comparing FIFO and LRU.

---

## Question 1: Page-Table Printing
### Implementation Details
* **System Call Registration**: Added `SYS_vmprint` (syscall #22) across `syscall.h`, `syscall.c`, `user.h`, `usys.S`, and `defs.h`. Dispatched via `sys_vmprint()` in `sysproc.c`.
* **Page Table Traversal (`vm.c`)**:
  Implemented `int vmprint(void)`. Following xv6 memory conventions and `freevm()`:
  * Iterates across the calling process's page directory for user space indices `0` to `PDX(KERNBASE) - 1` (entries `0` to `511`, corresponding to virtual addresses `< 0x80000000`).
  * For each present directory entry (`PTE_P`), counts the page-table page and inspects all `NPTENTRIES` (1024) page table entries.
  * For each present PTE, extracts the virtual address (`PGADDR(i, j, 0)`), physical frame address (`PTE_ADDR(pte)`), and permission flags `P`, `W`, and `U`.
  * Outputs the mapping for each valid user page and totals the number of valid user pages and page-table pages.
* **Testing (`pttest.c`)**:
  Calls `vmprint()` at startup, executes `sbrk(4 * 4096)` to reserve 4 pages, writes to each new page, and calls `vmprint()` again.
  * **Result**: Initial state showed 3 user pages (code/data at `0x0`, stack guard at `0x1000` with `U=0`, user stack at `0x2000`). After allocation, exactly 4 new pages (`0x3000`, `0x4000`, `0x5000`, `0x6000`) appeared, totaling 7 user pages under 1 page-table page.

---

## Question 2: Handling Page Faults (Lazy Allocation)
### Implementation Details
* **Lazy Heap Growth (`sysproc.c`)**:
  Modified `sys_sbrk()`: when increment `n > 0`, it increments `myproc()->sz += n` without eagerly allocating physical pages or creating PTEs, returning the previous size. When `n < 0`, it invokes `growproc(n)` to deallocate immediately.
* **Page Fault Handler (`trap.c`)**:
  Handled `case T_PGFLT:` using `rcr2()` to read the faulting virtual address `va`:
  * If `va < myproc()->sz` and not a protection violation (`!(tf->err & 1)`): allocates a physical page via `kalloc()`, zeroes it with `memset()`, maps it at `PGROUNDDOWN(va)` with `mappages()` (`PTE_W | PTE_U`), increments `myproc()->pgfaults`, and resumes execution.
  * If `va >= myproc()->sz`: prints `pid <pid>: invalid memory access at <va>` and sets `myproc()->killed = 1`, terminating only the offending process without affecting the parent or kernel.
* **Kernel & VM Safety**:
  * Made `mappages()` non-static in `vm.c` and declared it in `defs.h`.
  * In `copyuvm()` and `deallocuvm()`, skipped unallocated pages (`pte == 0 || !(*pte & PTE_P)`) with `continue` instead of calling `panic()`.
  * Added `uint pgfaults;` counter in `struct proc` (`proc.h`), initialized in `allocproc()` (`proc.c`), and reset in `exec()` (`exec.c`).
  * Registered `SYS_getfaults` (syscall #23) returning `myproc()->pgfaults`.
* **Testing (`faulttest.c`)**:
  * Reserved 32 pages (`sbrk(32 * 4096)`), confirmed zero faults before accesses.
  * Wrote to 5 distinct pages (`0, 4, 10, 20, 31`), and confirmed `getfaults()` returned exactly 5.
  * Forked a child process to access `sz + 4096` (`0x24000`). The kernel caught the invalid fault, printed `pid 5: invalid memory access at 0x24000`, and killed only the child. The parent was unaffected and exited cleanly.

---

## Question 3: Page Replacement Algorithms (FIFO and LRU)
### Implementation Details
* Implemented `pagerepl.c` on xv6 simulating FIFO and LRU over arbitrary reference strings and frame capacities.
* **Algorithms**:
  * **FIFO**: Tracks frame arrival queue using circular victim pointer index (`(victim + 1) % frames`).
  * **LRU**: Maintains access timestamp arrays for all occupied frames, replacing the frame with minimum timestamp upon miss.
  * Formats hit ratio percentages with 2-decimal precision natively within xv6's integer `printf`.
* **Experimental Verification**:
  Reference string: `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1` (20 references)
  * **3 Frames**:
    * **FIFO**: 15 page faults (5 hits, 25.00% hit ratio) — **PASS**
    * **LRU**: 12 page faults (8 hits, 40.00% hit ratio) — **PASS**
  * **Comparison Table (3, 4, 5 Frames)**:

| Frame Count | FIFO Faults | FIFO Hit Ratio | LRU Faults | LRU Hit Ratio |
| :---: | :---: | :---: | :---: | :---: |
| **3** | 15 | 25.00% | 12 | 40.00% |
| **4** | 10 | 50.00% | 8 | 60.00% |
| **5** | 9 | 55.00% | 7 | 65.00% |

### Conclusion
LRU consistently outperforms FIFO across all tested capacities on this reference sequence. Increasing frame allocation from 3 to 5 reduces page faults from 15 to 9 for FIFO and from 12 to 7 for LRU, confirming theoretical expectations.
