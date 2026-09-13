# Assignment 5: Process Synchronization in xv6

**Student Name:** Ankit Basu
**Roll Number:** 2401MC06
**Course:** Operating Systems Lab
**Assignment:** Assignment 5

---

## Repository Structure

```text id="t2qz4m"
OSLAB/
├── Kernel/
│   ├── sysproc.c       # Implementations of shm_get, sys_sem_init, sys_sem_wait, sys_sem_post
│   ├── syscall.h       # System call number definitions
│   ├── syscall.c       # System call vector table pointers
│   └── vm.c            # Modified deallocuvm to protect shared memory page (0x60000000)
├── USER/
│   ├── user.h          # System call user declarations
│   ├── usys.pl         # System call entry stubs
│   ├── peterson.c      # Question 1: Mutual Exclusion via Peterson's Algorithm
│   ├── prodcons.c      # Question 2: Bounded-Buffer Producer-Consumer Problem
│   └── readwrite.c     # Question 3: Readers-Writers Problem
├── Output_LOGS/
│   └── transcript.txt  # Full QEMU execution log transcript
├── Makefile            # Updated UPROGS to include user binaries
└── README.md           # Assignment documentation
```

## Building and Running

To build and run xv6, run the following command:

```bash id="8m9gcf"
make qemu-nox
```

Once inside xv6, you can run the programs by typing their names:

* `peterson`
* `prodcons`
* `readwrite`

## Question 1: Peterson's Algorithm

The design sets up a shared memory region using a new `shm_get()` system call. When a parent process calls `shm_get()`, it either allocates a physical kernel page or maps an already allocated one into `0x60000000`. The child process calls `shm_get()` after `fork()` to ensure the same physical page is mapped into its own virtual address space at `0x60000000`. `vm.c` was modified to prevent `deallocuvm` from freeing this specific shared physical page.

Peterson's algorithm is implemented purely in user space by pointing `flag`, `turn`, and `counter` variables to offsets within the shared memory page. `volatile` is used to prevent the C compiler from optimizing away the busy-wait loop.

The `flag` variables indicate whether each process wants to enter the critical section, while `turn` resolves contention when both processes attempt to enter simultaneously. The shared `counter` acts as the protected critical-section resource, allowing mutual exclusion to be demonstrated directly.

## Question 2: Producer-Consumer Problem

For the producer-consumer problem, three semaphores (`empty`, `full`, and `mutex`) were implemented as xv6 kernel system calls: `sys_sem_init`, `sys_sem_wait`, and `sys_sem_post`.

The semaphores are backed by xv6's `spinlock` and `sleep`/`wakeup` primitives. The spinlock protects the internal semaphore state, while `sleep` and `wakeup` allow blocked processes to suspend execution instead of continuously busy-waiting.

The shared bounded buffer is placed in the same shared page obtained via `shm_get()`. The producer and consumer synchronize using the semaphores to ensure the consumer blocks when the buffer is empty, and the producer blocks when the buffer is full (which is visible in the logs when producer produces in bursts).

Here, `empty` keeps track of the number of free buffer slots, `full` keeps track of occupied slots, and `mutex` ensures mutual exclusion while the shared buffer is being accessed.

## Question 3: Readers-Writers Problem

The readers-writers problem was implemented using the custom semaphore system calls from Question 2. A fair readers-priority solution was designed using a mutex for the `read_count`, a `rw_mutex` for exclusive access by writers (or the first reader), and a `turnstile` semaphore to ensure that writers do not starve indefinitely.

Readers and writers access a shared `shared_data` variable located in the shared memory page.

The `read_count` variable tracks the number of readers currently accessing the shared resource. The first reader acquires `rw_mutex`, preventing writers from entering while readers are active, and the last reader releases it. The `turnstile` prevents a continuous stream of new readers from indefinitely delaying a waiting writer.

## Note

As per instructions, no comments were added to the code.
