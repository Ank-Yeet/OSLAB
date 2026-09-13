# Operating Systems — Lab Assignment 6: Deadlocks

This repository contains the solutions for the four questions in Lab Assignment 6,
covering deadlock avoidance, detection, prevention, and combined synchronization,
implemented as xv6 user-space programs.

## Contents

| File | Question | Purpose |
|---|---|---|
| `bankers.c` | Q1 | Banker's Algorithm — safety check + resource-request algorithm |
| `bankers_log.txt` | Q1 | Output log: safe sequence, one granted request, one denied request |
| `deadlockdetect.c` | Q2 | Deadlock detection via wait-for graph + DFS cycle detection |
| `deadlockdetect_log.txt` | Q2 | Output log: one acyclic scenario, one scenario with a 3-process cycle |
| `resorder_bad.c` | Q3 | Two processes acquire two locks in opposite order → deadlock |
| `resorder_fixed.c` | Q3 | Same workload, locks acquired in a consistent global order → no deadlock |
| `resourceorder_explanation.txt` | Q3 | Why consistent lock ordering eliminates circular wait |
| `resourceorder_log.txt` | Q3 | Output logs for both the bad (hanging) and fixed runs |
| `syncdeadlock.c` | Q4 | 5 processes sharing Scanner/Printer/Disk pools, resource-ordering strategy |
| `syncdeadlock_explanation.txt` | Q4 | Strategy used (hierarchical resource ordering) and justification |
| `syncdeadlock_log.txt` | Q4 | Output log showing all 5 processes completing without deadlock |

## Question 1 — Banker's Algorithm

`bankers.c` hardcodes 5 processes and 3 resource types (`Allocation`, `Max`, `Available`),
computes `Need = Max - Allocation`, and implements:

- **`is_safe()`** — runs the safety algorithm and returns a valid safe sequence if one exists.
- **`request_resources()`** — validates a request against `Need` and `Available`, tentatively
  grants it, re-runs the safety check, and either commits the allocation or rolls it back and
  prints `Request denied — would lead to unsafe state.`

**Test scenarios (see `bankers_log.txt`):**
1. Initial state safety check → safe sequence `P1 P3 P4 P0 P2`.
2. P1 requests `(1, 0, 2)` → within Need and Available, safety check still passes → **granted**.
3. P0 requests `(0, 2, 0)` → passes the Need/Available check but leaves no process able to
   finish → **denied**, and the tentative allocation is rolled back.

## Question 2 — Deadlock Detection (Resource Allocation Graph)

`deadlockdetect.c` builds a wait-for graph from an allocation matrix and a request matrix:
an edge `Pi -> Pj` exists whenever `Pi` is waiting on a resource currently held by `Pj`.
A DFS with a recursion-stack marker (`recStack`) detects back-edges, which correspond to
cycles in the wait-for graph, and reconstructs the cycle via a `parent[]` array.

**Test scenarios (see `deadlockdetect_log.txt`):**
1. 3 processes in a linear wait chain (`P0 -> P1 -> P2`, `P2` waits on nothing) → acyclic →
   `No deadlock detected.`
2. 3 processes waiting in a cycle (`P0 -> P1 -> P2 -> P0`) → detected, with the cycle printed
   as `P0 -> P1 -> P2 -> P0`.

## Question 3 — Deadlock Prevention via Resource Ordering

Two locks are implemented with a semaphore syscall (`sem_init` / `sem_wait` / `sem_post`).

- **`resorder_bad.c`**: Process A acquires `Lock1` then requests `Lock2`; Process B acquires
  `Lock2` then requests `Lock1`, with a deliberate `sleep(50)` between acquisitions so both
  processes are guaranteed to be holding their first lock when they request the second.
  This reproduces the classic circular-wait deadlock — the run hangs (see
  `resourceorder_log.txt`).
- **`resorder_fixed.c`**: identical workload, but both processes now acquire `Lock1` before
  `Lock2`. With a single global order enforced, the run always completes.

`resourceorder_explanation.txt` explains why this eliminates circular wait: no process can
hold a higher-ordered lock while waiting on a lower-ordered one, since it must have acquired
the lower-ordered lock first — so the wait-for graph can never close into a cycle.

## Question 4 — Combined Synchronization & Deadlock Avoidance

`syncdeadlock.c` models 5 processes, each needing 2 of 3 shared resource pools
(1 Scanner, 2 Printers, 2 Disks — implemented with counting semaphores), running for
3 work cycles each:

- P0, P4 → Scanner then Printer
- P1, P3 → Printer then Disk
- P2 → Scanner then Disk

**Strategy:** hierarchical resource ordering (Scanner < Printer < Disk), reused from Q3.
Every process requests resources in increasing order of this global rank, so no process
ever holds a higher-ranked resource while requesting a lower-ranked one — this makes a
circular wait, and therefore deadlock, impossible regardless of scheduling.

`syncdeadlock_log.txt` shows all 5 processes interleaving resource requests/grants/releases
across multiple cycles and finishing with `All processes completed successfully (no
deadlock).`

## How to Run (xv6)

1. Copy the `.c` files into the xv6 source tree and add each program name to the
   `UPROGS` list in the `Makefile`.
2. `make qemu-nox` (or your platform's equivalent) to build and boot xv6.
3. From the xv6 shell, run each program by name, e.g.:
   ```
   $ bankers
   $ deadlockdetect
   $ resorder_bad      # observe the hang, then interrupt/reset
   $ resorder_fixed
   $ syncdeadlock
   ```
4. Redirect or copy console output into the corresponding `_log.txt` file for submission.

## Notes on the Logs

The `_log.txt` files in this submission are the captured console output of the programs
above, lightly formatted for readability. The `_explanation.txt` files are short written
justifications required by Q3 and Q4, explaining which Coffman condition (circular wait) is
broken and why the chosen strategy guarantees it stays broken.
