# OS Lab Assignment 8 — Memory Management in xv6

## 1. Overview

This report documents Q1–Q4 of Operating Systems Lab Assignment 8, implemented on xv6-public for 32-bit x86. The work introduces three system calls—`va2pa`, `getmeminfo`, and `vmprint`—and implements lazy heap allocation.

The assignment PDF covers Q1–Q4. Although the repository checklist also mentions `cowtest.c`, `protecttest.c`, `shmtest.c`, and `pagerepl.c`, Q5–Q8 are not implemented in this submission.

## 2. Repository Structure

| Path | Purpose |
|---|---|
| `kernel/vm.c`, `trap.c` | Page-table operations, inspection, and page-fault handling |
| `kernel/kalloc.c` | Physical-page allocation and memory counters |
| `kernel/proc.c`, `proc.h`, `exec.c` | Process state and heap metadata |
| `kernel/sysproc.c`, `syscall.c`, `syscall.h` | System-call implementations and dispatch |
| `kernel/meminfo.h` | Memory-statistics structure |
| `user/` | `vatest`, `memtest`, `vmprinttest`, `lazytest` and system-call stubs |
| `output_logs/` | Captured test output |

## 3. Q1 — Virtual-to-Physical Translation (`va2pa`)

### Implementation

`va2pa` looks up a virtual address using `walkpgdir()` with allocation disabled. It returns `-1` if the page table or PTE is missing, the page is not present (`PTE_P`), or the mapping is not user-accessible (`PTE_U`). For a valid mapping, it combines the physical frame address with the original 12-bit page offset:

`PA = PTE_ADDR(*pte) | (VA & 0xFFF)`

The system call also prints the PDX, PTX, offset, and permission bits. `walkpgdir()` was made externally accessible and declared in `defs.h`.

### Results

Values below are from the `CPUS=1` log; physical addresses may vary between runs.

| Test | Result |
|---|---|
| Global variable at VA `0xD78` | PA `0xDEE8D78` |
| Stack variable at VA `0x2FCC` | PA `0xDEE5FCC` |
| `malloc(100)` at VA `0xAF98` | PA `0xDF76F98` |
| Unmapped VA `0x7FFFFFFF` | `-1` |
| Kernel VA `0x80100000` | `-1`, because `PTE_U` is clear |

After `fork()`, the parent and child have the same virtual address for a global variable but different physical addresses, since `copyuvm()` copies user pages into separate frames.

## 4. Q2 — Physical-Memory Statistics (`getmeminfo`)

### Implementation

The new `meminfo.h` defines:

```c
struct meminfo {
    int total_pages;
    int free_pages;
    int proc_pages;
    int pt_pages;
};
```

The allocator in `kalloc.c` tracks `ntotal` and `nfree`. Initialization updates the total count, `kfree()` increments the free count, and successful `kalloc()` calls decrement it. Updates and reads are protected by `kmem.lock`.

`proc_pages` is computed as `sz / PGSIZE`. `pt_pages`, computed by `ptpages()`, counts the page directory and present page tables, including those for kernel mappings. With lazy allocation, `proc_pages` describes the reserved virtual address space—not the number of physical frames actually allocated.

### Results

| `memtest` stage | Free pages | Process pages |
|---|---:|---:|
| Start | 56,789 | 3 |
| After allocating and touching 16 pages | 56,773 | 19 |
| Inside forked child | 56,687 | 19 |
| After child exits and is reaped | 56,773 | 19 |
| After shrinking by 16 pages | 56,789 | 3 |

The free-page count returns to its earlier value after the child is reaped and after the heap is shrunk, indicating no leak in the tested operations. The fork temporarily consumes additional frames for copied user memory, page tables, and the kernel stack.

## 5. Q3 — Page-Table Inspection (`vmprint`)

### Implementation

`vmprint(pgdir, sz)` scans the page directory and prints present user PDEs, their page-table physical addresses and flags, then each present PTE with its virtual address, physical frame, and flags. Kernel-half PDEs are summarized to keep the output manageable. Present pages without user permission are marked as guard pages.

The summary compares the number of mapped user pages with `sz / PGSIZE`, accounting for the stack guard page at VA `0x1000`.

### Results

| Test stage | `sz` | User pages | Guard pages |
|---|---:|---:|---:|
| Startup | `0x3000` | 2 | 1 |
| After `sbrk(8*4096)` and touching the pages | `0xB000` | 10 | 1 |
| After shrinking by one page | `0xA000` | 9 | 1 |

Because heap growth is lazy after Q4, `vmprinttest` touches each newly reserved page before printing the table. Shrinking the heap removes the final mapping, while the guard page remains inaccessible.

## 6. Q4 — Lazy Heap Allocation

### Implementation

For positive growth, `sys_sbrk()` increases the process's `sz` after validating the requested range, without allocating physical frames immediately. Negative growth continues through `growproc()` and `deallocuvm()`, which skips pages that were never mapped.

A `heapbase` field was added to `struct proc`. `exec()` initializes it and `fork()` copies it. When a page fault occurs, `lazyfault()` checks that the address lies in the valid heap range and that the page is not already present. It then allocates a frame, zeroes it, and maps it with user/write permissions. Invalid accesses and protection faults follow the existing process-fault handling.

`copyuvm()` was updated to skip absent page tables and non-present PTEs, since a process may reserve a large heap without allocating all its pages. Kernel accesses to valid lazy user buffers—such as buffers used by `read()` and `write()`—are also handled by checking the faulting address against the heap range.

### Results

| `lazytest` stage | Free pages | Observation |
|---|---:|---|
| Before reserving 32 MiB | 56,788 | Initial state |
| After `sbrk(32 MB)` | 56,788 | Virtual range grows without allocating heap data frames |
| After touching four locations | 56,781 | Four data pages and three page-table pages consume seven frames |
| Child touches a page, exits, and is reaped | 56,775 → 56,775 | No persistent leak observed |
| Child touches a reserved 400 MiB range | 56,775 → 56,775 | Child is killed after allocation fails |
| After shrinking the 32 MiB heap | 56,782 | Allocated data frames are freed |

New pages read as zero before being written. The tests also confirm that invalid accesses—beyond `sz`, to the stack guard page, and to a kernel address—terminate the child without stopping the parent.

**Known limitation:** an out-of-memory fault during a kernel-mode access can still lead to a kernel panic. The tested user-mode allocation-failure path is handled cleanly.

## 7. Build and Test

From `assignment8/submission/`:

```sh
make qemu-nox CPUS=1
```

At the xv6 shell, run:

```text
vatest
memtest
vmprinttest
lazytest
```

Use `make qemu-nox` for the recorded two-CPU configuration. Test output is stored in `output_logs/`, including `full_session_2cpu.txt`.

## 8. Issues and Resolutions

- **`walkpgdir()` declaration:** `pte_t` was not available when `defs.h` was included, so the declaration uses the compatible `uint *` type.
- **Accidental allocation during lookup:** page-table inspection uses `walkpgdir(..., 0)`.
- **Forking an untouched heap:** `copyuvm()` now skips absent page tables and non-present PTEs.
- **Page faults during system calls:** valid lazy heap faults are checked before taking the default kernel-fault path.
- **Page-table output after lazy growth:** tests touch reserved pages before inspecting them.

## 9. Conclusion

The implementation demonstrates address translation, memory accounting, page-table inspection, and lazy heap allocation in xv6. Tests show that invalid mappings are rejected, allocator counts recover after process cleanup, and reserving unused heap memory does not immediately consume physical frames. The main remaining limitation is recovery from out-of-memory faults that occur while the kernel is accessing lazy user memory.
