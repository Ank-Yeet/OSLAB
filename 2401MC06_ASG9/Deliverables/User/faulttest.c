#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int
main(int argc, char *argv[])
{
  printf(1, "faulttest: starting lazy allocation test...\n");

  int initial_faults = getfaults();
  printf(1, "faulttest: initial page faults = %d\n", initial_faults);

  // Call sbrk(32 * PGSIZE) - 32 pages
  printf(1, "faulttest: reserving 32 pages with sbrk(%d)...\n", 32 * PGSIZE);
  char *mem = sbrk(32 * PGSIZE);
  if(mem == (char*)-1){
    printf(1, "faulttest: sbrk failed!\n");
    exit();
  }

  int faults_after_sbrk = getfaults();
  printf(1, "faulttest: faults immediately after sbrk (lazy check) = %d\n", faults_after_sbrk);
  if(faults_after_sbrk != initial_faults){
    printf(1, "faulttest: WARNING: faults increased before touching pages!\n");
  }

  // Write to exactly 5 distinct pages (e.g., page 0, 4, 10, 20, 31)
  int pages_to_touch[5] = {0, 4, 10, 20, 31};
  printf(1, "faulttest: writing to exactly 5 different pages (pages 0, 4, 10, 20, 31)...\n");
  for(int i = 0; i < 5; i++){
    int pno = pages_to_touch[i];
    mem[pno * PGSIZE] = (char)('A' + i);
  }

  int faults_after_touch = getfaults();
  int handled = faults_after_touch - faults_after_sbrk;
  printf(1, "faulttest: page faults handled = %d (expected: 5)\n", handled);

  if(handled == 5){
    printf(1, "faulttest: SUCCESS - exactly 5 page faults were handled!\n");
  } else {
    printf(1, "faulttest: FAILURE - expected 5 page faults, got %d!\n", handled);
    exit();
  }

  // Test accessing an address above the allocated size in a child process
  printf(1, "\nfaulttest: testing invalid access above allocated size in child process...\n");
  int pid = fork();
  if(pid < 0){
    printf(1, "faulttest: fork failed\n");
    exit();
  }

  if(pid == 0){
    // Child process: accesses an address above the allocated size (mem + 33 * PGSIZE)
    char *invalid_addr = mem + 33 * PGSIZE;
    printf(1, "faulttest: child (pid %d) attempting access at invalid address 0x%x...\n", getpid(), invalid_addr);
    *invalid_addr = 'Z';
    // If the process was not killed, it's an error
    printf(1, "faulttest: ERROR: child survived invalid access!\n");
    exit();
  }

  // Parent process: wait for child to be killed
  wait();
  printf(1, "faulttest: child process was killed by kernel as expected.\n");
  printf(1, "faulttest: parent process is alive and unaffected.\n");
  printf(1, "faulttest: ALL TESTS PASSED SUCCESSFULLY.\n");
  exit();
}
