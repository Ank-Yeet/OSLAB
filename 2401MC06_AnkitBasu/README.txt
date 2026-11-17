Operating System Lab Examination (MA3105)
Name       : Ankit Basu
Roll Number: 2401MC06
Set Number : 1

Build and execution:
Start by running'make qemu-nox CPUS=1' from the xv6 directory. 
Inside the xv6 shell, first I have echoed 'Name RollNumber' and then executed 'synctest' for Question 1 and 'memtest' for Question 2.
The added system calls use numbers 22 through 26 (sem_init, sem_wait, sem_post, freepages, va2pa) - as instructed in the assignment.

Question 1: Counting Semaphores for Process Synchronization
- Added a file 'sem.c' with a spinlock protected table for 8 semaphores.
- The file 'main'c' calls seminit to initialise the table at the start.
- sem_wait blocks using kernel sleep(&sem, &lock) in a while loop, and sem_post increments and calls wakeup() once done.
- System call registration files that I modified are: syscall.h, syscall.c, sysproc.c, user.h, usys.S, defs.h and Makefile.



Question 2: Memory Statistics and Address Translation
- Added free_pages to struct kmem in kalloc.c, updated in kalloc() and kfree(). Exposed via freepages().
- Mention va2pa() added in vm.c walks the calling process page table using walkpgdir(), checks PTE_P and PTE_U, and returns the physical address or -1.
- System call files updated for freepages and va2pa are: syscall.h, syscall.c, sysproc.c, defs.h, user.h, usys.S, Makefile.
- The file 'memtest.c' verifies initial free pages, 10-page reduction after sbrk, address translation for global variable, and parent/child physical address divergence with complete memory recovery after wait().
- x86 uses two-level paging (Page Directory -> Page Table -> Physical Page Frame). 



Output evidence: Q1_Output.txt/.png and Q2_Output.txt/.png (Name + Roll number echoed before each test).
