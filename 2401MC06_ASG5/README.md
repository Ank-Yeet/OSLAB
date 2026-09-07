# Assignment 5: Process Synchronization in xv6

**Student Name:** Ankit Basu  
**Roll Number:** 2401MC06  
**Course:** Operating Systems Lab  
**Assignment:** Assignment 5 - Process Synchronization  

---

## Repository Structure

```text
OSLAB/
├── Kernel/
│   ├── sysproc.c       # Implementations of shm_get, sys_sem_init, sys_sem_wait, sys_sem_post
│   ├── syscall.h       # System call number definitions
│   ├── syscall.c       # System call vector table pointers
│   └── vm.c            # Modified deallocuvm to protect shared memory page (0x60000000)
├── User/
│   ├── user.h          # System call user declarations
│   ├── usys.pl         # System call entry stubs
│   ├── peterson.c      # Question 1: Mutual Exclusion via Peterson's Algorithm
│   ├── prodcons.c      # Question 2: Bounded-Buffer Producer-Consumer Problem
│   └── readwrite.c     # Question 3: Readers-Writers Problem
├── Output_LOGS/
│   └── transcript.txt  # Full QEMU execution log transcript
├── Makefile            # Updated UPROGS to include user binaries
└── README.md           # Assignment documentation