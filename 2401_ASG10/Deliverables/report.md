# Operating Systems Assignment 10: Memory Management, Paging & Page Replacement

## Overview
This report details the implementation, architectural modifications, algorithms, and experimental results for **Operating Systems Lab Assignment 10** implemented on the 32-bit x86 xv6 operating system kernel. The assignment encompasses four core memory management concepts:
1. **Tracking Physical Memory Allocation** in the kernel page allocator (`kalloc.c`).
2. **Virtual-to-Physical Address Translation** via x86 two-level page directory walking (`vm.c`).
3. **Paging & Page Protection** via page table inspection and write-protection bit manipulation (`vm.c`).
4. **Page Replacement Algorithm Simulation** comparing FIFO, LRU, and Optimal algorithms across different frame allocations.

---

## Question 1: Memory Management: Tracking Physical Memory Allocation

### Implementation Details
* **Allocation and Deallocation Counters (`kalloc.c`)**:
  * Added global tracking counters `uint total_allocated_pages` and `uint total_freed_pages`.
  * Added `static int in_freerange` to distinguish allocator bootstrap memory initialization from runtime memory frees.
  * In `freerange()`, `in_freerange` is asserted during the initial population of `kmem.freelist` so initial physical frames are not erroneously counted as freed pages.
  * In `kalloc()`, incremented `total_allocated_pages` under `kmem.lock` whenever a frame is popped from `kmem.freelist`.
  * In `kfree()`, incremented `total_freed_pages` under `kmem.lock` for all runtime page deallocations.
  * Computed `currently_allocated_pages` as `total_allocated_pages - total_freed_pages`.
  * Traversed `kmem.freelist` to obtain the exact count of `available_physical_pages`.
* **System Call Registration**:
  * Implemented `int memstats(void)` in `kalloc.c` and registered `SYS_memstats` (syscall #22) in `syscall.h`, `syscall.c`, `sysproc.c`, `defs.h`, `user.h`, and `usys.S`.
  * `memstats()` outputs a formatted summary containing total allocated, total freed, currently allocated, and available physical memory pages.

### Testing & Observations (`memtest.c`)
* **Baseline**: At test launch, the kernel reported 490 total allocated pages, 214 total freed pages, 276 currently allocated pages, and 56,789 available physical pages.
* **Allocation**: Calling `sbrk(16384)` (allocating 4 pages) and writing to each page increased total allocated pages from 490 to 494 (+4), increased currently allocated pages from 276 to 280 (+4), and decreased available physical pages from 56,789 to 56,785 (-4).
* **Deallocation**: Calling `sbrk(-16384)` (releasing 4 pages) increased total freed pages from 214 to 218 (+4), decreased currently allocated pages back to 276 (-4), and restored available pages to 56,789 (+4).
* **Observation**: The counters accurately reflect physical memory transactions with zero leakage.

---

## Question 2: Virtual Memory: Virtual-to-Physical Address Translation

### Implementation Details
* **Address Translation Logic (`vm.c`)**:
  * Implemented `int vtop(int virtual_address)` taking a virtual address `va`.
  * Utilized `walkpgdir(p->pgdir, (void*)va, 0)` to locate the page table entry (PTE) corresponding to the calling process's page directory without allocating missing page table pages.
  * Checked if the PTE pointer is non-null and the present bit is set (`*pte & PTE_P`). If not mapped, returns `-1`.
  * Combined the physical frame base address `PTE_ADDR(*pte)` with the 12-bit page offset `(va & 0xFFF)` using bitwise OR:
    $$\text{Physical Address} = \text{PTE\_ADDR}(*pte) \mid (\text{va} \ \& \ \text{0xFFF})$$
  * Registered `SYS_vtop` (syscall #23) across `syscall.h`, `syscall.c`, `sysproc.c`, `defs.h`, `user.h`, and `usys.S`.

### Testing & Observations (`vttest.c`)
* **Code Segment**: `main` at `VA = 0x0` translated to `PA = 0xDEE2000` (PFN: `0xDEE2`, Offset: `0x0`).
* **Data Segment**: Global variable `&global_counter` at `VA = 0x1294` translated to `PA = 0xDEE0294` (PFN: `0xDEE0`, Offset: `0x294`). The offset was verified to match `0x294`.
* **Stack Segment**: Local variable `&local_var` at `VA = 0x3FBC` translated to `PA = 0xDEDEFBC` (PFN: `0xDEDE`, Offset: `0xFBC`).
* **Heap Memory**: Calling `sbrk(8192)` allocated two consecutive virtual pages (`0x4000` and `0x5000`). `vtop(0x42A4)` mapped to `0xDEE32A4`, confirming proper page offset retention (`0x2A4`).
* **Unmapped & Deallocated Addresses**: Testing unmapped address `0x60000000` returned `-1`. After deallocating heap pages via `sbrk(-8192)`, `vtop(0x4000)` immediately returned `-1`, demonstrating accurate validation of the `PTE_P` bit.

---

## Question 3: Paging: Page Table Management and Page Protection

### Implementation Details
* **Page Table Inspection (`pageinfo` in `vm.c`)**:
  * Implemented `int pageinfo(int virtual_address)`.
  * Uses `walkpgdir()` to retrieve the PTE. If not mapped or not present, displays an error message and returns `-1`.
  * Extracts Physical Frame Number (`PTE_ADDR(*pte) >> 12`) and flags: Present (`PTE_P`), Writable (`PTE_W`), and User-accessible (`PTE_U`).
* **Page Write-Protection (`setreadonly` in `vm.c`)**:
  * Implemented `int setreadonly(int virtual_address)`.
  * Clears the write bit from the target PTE: `*pte &= ~PTE_W`.
  * Flushes the Translation Lookaside Buffer (TLB) using `lcr3(V2P(p->pgdir))` so hardware MMU caching is updated immediately.
* **System Call Registration**: Registered `SYS_pageinfo` (syscall #24) and `SYS_setreadonly` (syscall #25).

### Testing & Observations (`pagetest.c`)
* **Step 1 & 2**: Allocated a 4KB heap page at `0x4000` and wrote `"XY"`, verifying normal read/write operation.
* **Step 3**: Invoked `pageinfo(0x4000)`. Output confirmed flags: `P=1, W=1, U=1` (Frame `0xDFBC`).
* **Step 4 & 5**: Invoked `setreadonly(0x4000)`. Re-inspecting with `pageinfo(0x4000)` confirmed flags: `P=1, W=0, U=1` (`PTE_W` successfully cleared).
* **Step 6**: Read from `0x4000`, confirming read access remains fully functional.
* **Step 7 (Fault Demonstration)**: Forked a child process that attempted to write `'Z'` to `0x4000`. The hardware MMU caught the write protection violation, triggering CPU interrupt vector 14 (`T_PGFLT`, err code 7). The kernel killed the child process (`pid 4 pagetest: trap 14 err 7 on cpu 0 eip 0x118 addr 0x4000--kill proc`), and the parent safely reaped the child via `wait()`.

---

## Question 4: Page Replacement Algorithms (FIFO, LRU, Optimal)

### Implementation Details & Algorithms (`pagereplace.c`)
* **Reference String**: `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1` (20 page references).
* **Algorithms**:
  * **FIFO (First-In First-Out)**: Replaces the oldest page using a circular replacement pointer `victim = (victim + 1) % frames`.
  * **LRU (Least Recently Used)**: Tracks the reference timestamp of each frame on hit or miss; upon replacement, evicts the frame with the smallest timestamp.
  * **Optimal (Bélády's Algorithm)**: Looks forward into future references and evicts the frame whose next reference is furthest in the future (or never referenced again).
* Implemented fixed-point percentage formatting to output ratios with two decimal places.

### Experimental Performance Comparison Table
Testing all three algorithms across 3, 4, and 5 frames yielded the following empirical results:

| Frame Count | Algorithm | Page Faults | Page Hits | Hit Ratio | Page Fault Ratio |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **3** | **FIFO** | 15 | 5 | 25.00% | 75.00% |
| **3** | **LRU** | 12 | 8 | 40.00% | 60.00% |
| **3** | **Optimal** | **9** | **11** | **55.00%** | **45.00%** |
| **4** | **FIFO** | 10 | 10 | 50.00% | 50.00% |
| **4** | **LRU** | **8** | **12** | **60.00%** | **40.00%** |
| **4** | **Optimal** | **8** | **12** | **60.00%** | **40.00%** |
| **5** | **FIFO** | 9 | 11 | 55.00% | 45.00% |
| **5** | **LRU** | **7** | **13** | **65.00%** | **35.00%** |
| **5** | **Optimal** | **7** | **13** | **65.00%** | **35.00%** |

### Analysis & Comparative Observations
1. **Optimal Algorithm Supremacy**:
   Optimal page replacement achieves the absolute lowest number of page faults across all frame allocations (9 faults at 3 frames, 8 at 4 frames, 7 at 5 frames). Since it leverages future knowledge of page requests, it provides the theoretical performance upper bound (minimum page faults possible).
2. **LRU Superiority over FIFO**:
   LRU consistently and significantly outperforms FIFO across all tested capacities (12 vs 15 faults at 3 frames; 8 vs 10 at 4 frames; 7 vs 9 at 5 frames). LRU exploits temporal locality, recognizing that frequently and recently referenced pages (such as pages `0`, `2`, and `3`) are likely to be accessed again soon.
3. **LRU Matches Optimal**:
   At 4 and 5 frames, LRU achieves identical performance to the Optimal algorithm (8 faults at 4 frames and 7 faults at 5 frames), demonstrating that LRU is an exceptional real-world online approximation of the optimal offline policy.
4. **FIFO Limitations**:
   FIFO suffers from poor eviction choices because arrival age does not correlate with reference frequency, leading to repeated misses on hot pages.

---

## Conclusion
All requirements of Assignment 10 were successfully implemented, verified, and benchmarked within xv6. The system calls (`memstats`, `vtop`, `pageinfo`, and `setreadonly`) function robustly without memory leaks or race conditions, page protection correctly invokes the x86 MMU fault handler, and the page replacement simulations accurately validate theoretical memory management principles.
