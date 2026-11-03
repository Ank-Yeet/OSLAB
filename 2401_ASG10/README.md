# OS Lab Assignment 10 — Memory Management, Paging & Page Replacement

This directory contains the xv6 (x86) implementation, test programs, logs, and report for Operating Systems Lab Assignment 10.

## Implementation Overview

| Question | Feature | Source Files | Test Program |
|---|---|---|---|
| **Question 1** | Memory Allocation Tracking (`memstats` syscall) | `Kernel/kalloc.c`, `Kernel/sysproc.c`, `Kernel/syscall.c`, `Kernel/syscall.h`, `Kernel/defs.h`, `User/user.h`, `User/usys.S` | `memtest` |
| **Question 2** | Virtual-to-Physical Address Translation (`vtop` syscall) | `Kernel/vm.c`, `Kernel/sysproc.c`, `Kernel/syscall.c`, `Kernel/syscall.h`, `Kernel/defs.h`, `User/user.h`, `User/usys.S` | `vttest` |
| **Question 3** | Paging & Page Protection (`pageinfo`, `setreadonly` syscalls) | `Kernel/vm.c`, `Kernel/sysproc.c`, `Kernel/syscall.c`, `Kernel/syscall.h`, `Kernel/defs.h`, `User/user.h`, `User/usys.S` | `pagetest` |
| **Question 4** | Page Replacement Algorithms (FIFO, LRU, Optimal) | `User/pagereplace.c` | `pagereplace` |

## Repository Structure

```text
2401_ASG10/
├── OS_Assignment_10.pdf
├── README.md
└── Deliverables/
    ├── Makefile
    ├── README.md
    ├── report.md
    ├── Kernel/
    │   ├── defs.h
    │   ├── kalloc.c
    │   ├── syscall.c
    │   ├── syscall.h
    │   ├── sysproc.c
    │   └── vm.c
    ├── User/
    │   ├── memtest.c
    │   ├── pagereplace.c
    │   ├── pagetest.c
    │   ├── user.h
    │   ├── usys.S
    │   └── vttest.c
    └── Output_logs/
        ├── full_session.txt
        ├── q1_memtest.txt
        ├── q1_memtest.png
        ├── q2_vttest.txt
        ├── q2_vttest.png
        ├── q3_pagetest.txt
        ├── q3_pagetest.png
        ├── q4_pagereplace.txt
        └── q4_pagereplace.png
```

## Build and Run Instructions

1. Navigate to the xv6 source directory:
   ```sh
   cd /home/ankit-basu/os/xv6-x86
   ```
2. Build the kernel and file system:
   ```sh
   make clean && make
   ```
3. Launch xv6 in QEMU (non-graphic mode):
   ```sh
   make qemu-nox
   ```
4. Run each test program at the xv6 shell:
   ```sh
   $ memtest
   $ vttest
   $ pagetest
   $ pagereplace
   ```
5. To exit QEMU, press `Ctrl+A`, then `X`.
