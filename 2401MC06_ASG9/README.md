# OS Lab Assignment 9 — Virtual Memory & Page Replacement

This directory contains the xv6 (x86) implementation for Operating Systems Lab Assignment 9.

## Implementation Overview

| Question | Feature | Source Files | Test Program |
|---|---|---|---|
| **Question 1** | Page-Table Printing (`vmprint` system call) | `Kernel/vm.c`, `Kernel/sysproc.c`, `Kernel/syscall.c`, `Kernel/syscall.h`, `Kernel/defs.h`, `User/user.h`, `User/usys.S` | `pttest` |
| **Question 2** | Lazy Memory Allocation & Page Fault Handling (`getfaults` system call) | `Kernel/sysproc.c`, `Kernel/trap.c`, `Kernel/vm.c`, `Kernel/proc.c`, `Kernel/proc.h`, `Kernel/exec.c` | `faulttest` |
| **Question 3** | FIFO and LRU Page Replacement Algorithms | `User/pagerepl.c` | `pagerepl` |

## Repository Structure

```text
2401MC06_ASG9/
├── OS_Assignment_9.pdf
└── Deliverables/
    ├── Makefile
    ├── README.md
    ├── report.md
    ├── Kernel/
    │   ├── defs.h
    │   ├── exec.c
    │   ├── proc.c
    │   ├── proc.h
    │   ├── syscall.c
    │   ├── syscall.h
    │   ├── sysproc.c
    │   ├── trap.c
    │   └── vm.c
    ├── User/
    │   ├── faulttest.c
    │   ├── pagerepl.c
    │   ├── pttest.c
    │   ├── user.h
    │   └── usys.S
    └── Output_logs/
        ├── full_session.txt
        ├── q1_pttest.txt
        ├── q2_faulttest.txt
        └── q3_pagerepl.txt
```

## Build and Run

1. Copy or build within an xv6-public x86 repository.
2. Compile xv6 and build disk image:
   ```sh
   make clean && make
   ```
3. Boot QEMU in non-graphic mode:
   ```sh
   make qemu-nox
   ```
4. Run each test program at the xv6 shell:
   ```sh
   $ pttest
   $ faulttest
   $ pagerepl
   ```
5. Exit QEMU:
   Press `Ctrl+A`, then `X`.
