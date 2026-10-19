# OS Lab Assignment 8 — xv6 Memory Management

This directory contains an xv6-public based implementation of four memory-management exercises: virtual-to-physical address translation, physical-memory statistics, page-table inspection, and lazy heap allocation.

## Implementation overview

| Question | Feature | Test program |
|---|---|---|
| Q1 | `va2pa`: translate a valid user virtual address | `vatest` |
| Q2 | `getmeminfo`: report physical-memory and page-table statistics | `memtest` |
| Q3 | `vmprint`: print user page-table mappings and flags | `vmprinttest` |
| Q4 | Lazy allocation of heap pages on first access | `lazytest` |

The implementation adds system calls `va2pa` (22), `getmeminfo` (23), and `vmprint` (24), and updates the xv6 memory allocator and page-fault handling to support the exercises.

## Prerequisites

- Linux environment (or another environment supported by the xv6-public toolchain)
- GNU Make and GCC toolchain suitable for the project
- QEMU with x86 emulation support
- The compiler/binutils expected by the checked-out xv6-public version

Exact package names vary by distribution. If the build fails because a compiler or QEMU executable is missing, install the corresponding toolchain dependency before continuing.

## Build and run

Open a terminal in the submission directory:

```sh
cd submission
make qemu-nox CPUS=1
```

At the xv6 shell prompt, run each test:

```text
vatest
memtest
vmprinttest
lazytest
```

To run the combined session with two CPUs, use:

```sh
make qemu-nox
```

Exit QEMU using the key sequence supported by the xv6/QEMU terminal configuration (commonly `Ctrl+A`, then `X`).

## Test logs

Recorded output is available under `submission/output_logs/`:

- `q1_vatest.txt`
- `q2_memtest.txt`
- `q3_vmprinttest.txt`
- `q4_lazytest.txt`
- `full_session_cpu.txt`

Physical addresses and free-page totals can vary between runs. Use the logs as reference behavior, not as constants that every execution must reproduce exactly.

