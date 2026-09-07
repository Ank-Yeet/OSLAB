# Assignment 5: Process Synchronization in xv6

**Course:** Operating Systems Lab  
**Submission Format:** `RollNumber_Name_Assignment4`  

---

## Executive Summary & Overview

This project implements classical process synchronization mechanisms and algorithms within the RISC-V **xv6** operating system kernel and user space. Because standard xv6 processes execute in isolated virtual address spaces without built-in process-to-process shared memory or counting semaphores, this implementation introduces two fundamental kernel primitives:

1. **Kernel Shared Memory Subsystem (`shm_get`)**: Allows parent and child processes to map identical physical memory pages into their virtual memory spaces.
2. **System-wide Semaphore Subsystem (`sem_init`, `sem_wait`, `sem_signal`)**: Provides sleep/wakeup-backed counting semaphores managed within the kernel.

Using these primitives, four classical synchronization problems are solved and verified:
- **Question 1**: Pure software mutual exclusion via **Peterson's Algorithm** over shared memory.
- **Question 2**: The **Bounded-Buffer Producer-Consumer Problem** using counting semaphores.
- **Question 3**: The **First Readers-Writers Problem** (Reader Priority with exclusive writer lock).
- **Question 4**: Deadlock-free **Dining Philosophers Problem** using asymmetric resource allocation.

---

## Directory & File Structure

```text
RollNumber_Name_Assignment4/
├── kernel/
│   ├── shm.c             # Kernel implementation of shm_get and semaphores
│   ├── defs.h            # Function prototypes (shminit, shm_get, sem operations)
│   ├── sysproc.c         # System call handlers (sys_shm_get, sys_sem_*)
│   ├── syscall.h         # System call numbers
│   ├── syscall.c         # System call dispatch table
│   └── main.c            # Kernel initialization (calls shminit())
├── user/
│   ├── user.h            # User-level function prototypes for shm & sem
│   ├── usys.pl           # System call stubs generator
│   ├── peterson.c        # Question 1: Peterson's Algorithm executable
│   ├── prodcons.c        # Question 2: Producer-Consumer executable
│   ├── readwrite.c       # Question 3: Readers-Writers executable
│   └── dining.c          # Question 4: Dining Philosophers executable
├── Makefile              # Updated UPROGS to compile user applications
├── README.md             # Design documentation and setup guide
└── output_logs/          # Execution transcripts and output logs
    ├── q1_peterson.txt
    ├── q2_prodcons.txt
    ├── q3_readwrite.txt
    └── q4_dining.txt
```

---

## Kernel Infrastructure Implementation

### 1. Shared Memory (`sys_shm_get`)

Standard xv6 process creation via `fork()` duplicates user page tables, keeping parent and child memory strictly isolated. To facilitate direct memory sharing:

- A physical page array (`shm_table`) is allocated in `kernel/shm.c` protected by a spinlock.
- Calling `shm_get(id)` allocates a physical page (via `kalloc()`) on first access or retrieves the existing page for region `id`.
- The physical page is mapped into the calling process's page table at `PGROUNDUP(p->sz)` with read, write, and user permissions (`PTE_R | PTE_W | PTE_U`).
- The process size (`p->sz`) is expanded by `PGSIZE` ($4096$ bytes), returning the mapped virtual address to user space.

### 2. System-Call-Based Semaphores

To avoid busy-waiting in high-level synchronization applications, counting semaphores are backed by kernel spinlocks and xv6's condition-variable primitives (`sleep()` and `wakeup()`):

- `sem_init(id, val)`: Initializes semaphore `id` with integer capacity `val` and initializes its underlying kernel spinlock.
- `sem_wait(id)`: Acquires the spinlock. If `val <= 0`, the process releases the spinlock and enters sleep mode on the semaphore's address. Upon waking, it re-checks `val`, decrements it when positive, and releases the spinlock.
- `sem_signal(id)`: Acquires the spinlock, increments `val`, invokes `wakeup()` to unblock waiting processes, and releases the spinlock.

---

## Detailed Task Implementations

### Question 1: Mutual Exclusion using Peterson's Algorithm

#### Design & Logic
Peterson's Algorithm achieves mutual exclusion between two processes purely through software, operating on two shared variables in shared memory:
- `int flag[2]`: Indicates if process $i$ wants to enter the Critical Section (CS).
- `int turn`: Indicates whose turn it is to enter when both processes express intent simultaneously.

```c
// Entry Section for Process i (other = 1 - i)
flag[i] = 1;
turn = other;
while (flag[other] == 1 && turn == other) {
    // Spin / Busy wait
}

// CRITICAL SECTION
// Increments shared_counter and prints progress

// Exit Section
flag[i] = 0;
```

#### Verification & Correctness
- **Race Condition Prevention**: Simulated delays (`delay()`) inside the critical section force opportunity for race conditions. Peterson's flags ensure strict serial entry.
- **Expected Outcome**: Each process completes 10 iterations, yielding an exact final `shared_counter` value of **20** without interleaved "in CS" logs from both processes.

---

### Question 2: Producer-Consumer Problem (Bounded Buffer)

#### Design & Logic
A bounded circular buffer of size $N = 5$ is hosted in shared memory. Synchronization relies on three semaphores:
1. `SEM_EMPTY` (Counting semaphore initialized to $N$): Tracks free slots available for production.
2. `SEM_FULL` (Counting semaphore initialized to $0$): Tracks produced items available for consumption.
3. `SEM_MUTEX` (Binary semaphore initialized to $1$): Enforces mutual exclusion during circular queue manipulations.

#### Synchronization Flow
- **Producer**: Executes `sem_wait(SEM_EMPTY)` $\rightarrow$ `sem_wait(SEM_MUTEX)` $\rightarrow$ inserts item into `buffer[in]` $\rightarrow$ `sem_signal(SEM_MUTEX)` $\rightarrow$ `sem_signal(SEM_FULL)`.
- **Consumer**: Executes `sem_wait(SEM_FULL)` $\rightarrow$ `sem_wait(SEM_MUTEX)` $\rightarrow$ extracts item from `buffer[out]` $\rightarrow$ `sem_signal(SEM_MUTEX)` $\rightarrow$ `sem_signal(SEM_EMPTY)`.

#### Verification
- When the producer fills all 5 slots, subsequent `sem_wait(SEM_EMPTY)` calls block the producer until the consumer consumes an item.
- When the buffer drains, `sem_wait(SEM_FULL)` blocks the consumer until the producer inserts new items.
- All 20 items (1 through 20) are produced and consumed in FIFO sequence.

---

### Question 3: Readers-Writers Problem

#### Design & Logic
Solves the **First Readers-Writers Problem (Readers Priority)**, permitting simultaneous reading while enforcing strict mutual exclusion for writers.

- `read_count`: Shared variable tracking active readers.
- `SEM_MUTEX`: Binary semaphore protecting modifications to `read_count`.
- `SEM_RWMUTEX`: Binary semaphore granting exclusive write access to `shared_data`.

#### Reader Protocol
1. Acquire `SEM_MUTEX`.
2. Increment `read_count`. If `read_count == 1` (first reader), execute `sem_wait(SEM_RWMUTEX)` to block writers.
3. Release `SEM_MUTEX`.
4. **Read `shared_data` concurrently**.
5. Acquire `SEM_MUTEX`.
6. Decrement `read_count`. If `read_count == 0` (last reader), execute `sem_signal(SEM_RWMUTEX)` to unblock writers.
7. Release `SEM_MUTEX`.

#### Writer Protocol
1. Execute `sem_wait(SEM_RWMUTEX)`.
2. **Write / update `shared_data` exclusively**.
3. Execute `sem_signal(SEM_RWMUTEX)`.

#### Verification
Output logs show multiple readers reading concurrently (showing identical or active reader counts), while writers execute exclusively without active readers or competing writers present.

---

### Question 4: Dining Philosophers Problem (Deadlock Avoidance)

#### Design & Strategy
5 philosophers sit at a circular table with 5 shared forks represented as binary semaphores (`0` to `4`). Each philosopher alternates between `THINKING`, `HUNGRY`, and `EATING`.

To prevent deadlock, an **Asymmetric Resource Allocation Strategy** is applied:
- **Even Philosophers ($0, 2, 4$)**: Pick up their **Right Fork** first `((id + 1) % 5)`, then their **Left Fork** `(id)`.
- **Odd Philosophers ($1, 3$)**: Pick up their **Left Fork** first `(id)`, then their **Right Fork** `((id + 1) % 5)`.

#### Deadlock Freedom Proof
Deadlock requires Coffman's *Circular Wait* condition (a closed dependency loop where every process holds one resource and waits for another). 
- If all 5 philosophers were symmetric (e.g., all grabbing left first), all could simultaneously acquire 1 fork and block forever waiting for the second.
- By breaking symmetry, adjacent philosophers request forks in opposing orders. At least one philosopher will always be able to acquire both forks, eat, release them, and break potential wait cycles.

---

## Build & Execution Instructions

### Prerequisites
- QEMU emulator (`qemu-system-riscv64`)
- RISC-V GNU Toolchain (`gcc-riscv64-linux-gnu` or `riscv64-unknown-elf-gcc`)

### Step-by-Step Compilation and Run Guide

1. **Clean and Build xv6 Source Code**:
   ```bash
   make clean
   make qemu
   ```

2. **Execute Programs in the xv6 Shell**:

   - **Question 1: Peterson's Algorithm**
     ```text
     $ peterson
     ```
     *Validates software-based mutual exclusion across 20 counter increments.*

   - **Question 2: Bounded-Buffer Producer-Consumer**
     ```text
     $ prodcons
     ```
     *Demonstrates producer/consumer blocking and circular buffer synchronization.*

   - **Question 3: Readers-Writers Problem**
     ```text
     $ readwrite
     ```
     *Demonstrates concurrent reader execution and exclusive writer access.*

   - **Question 4: Dining Philosophers Problem**
     ```text
     $ dining
     ```
     *Executes 5 complete cycles for 5 philosophers without encountering deadlock.*

3. **Exiting QEMU**:
   Press `Ctrl+A` then release and press `X`.