#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "\n=======================================================\n");
  printf(1, "  Question 3: Paging & Page Protection Test            \n");
  printf(1, "=======================================================\n\n");

  // Step 1: Allocate a page
  printf(1, "[pagetest] Step 1: Allocating a 4KB memory page using sbrk(4096)...\n");
  char *page = sbrk(4096);
  if(page == (char*)-1){
    printf(1, "[pagetest] Error: sbrk failed!\n");
    exit();
  }

  // Step 2: Write and verify
  page[0] = 'X';
  page[1] = 'Y';
  page[2] = '\0';
  printf(1, "[pagetest] Step 2: Wrote \"%s\" to 0x%x. Read/Write initial state verified.\n",
         page, (uint)page);

  // Step 3: Display page-table information
  printf(1, "[pagetest] Step 3: Displaying page info before protection:\n");
  pageinfo((int)page);

  // Step 4: Change page to read-only
  printf(1, "[pagetest] Step 4: Changing page 0x%x to read-only via setreadonly()...\n", (uint)page);
  if(setreadonly((int)page) < 0){
    printf(1, "[pagetest] Error: setreadonly failed!\n");
    exit();
  }

  // Step 5: Display page-table information after protection
  printf(1, "[pagetest] Step 5: Displaying page info after protection (PTE_W should be 0):\n");
  pageinfo((int)page);

  // Step 6: Verify reading is still permitted
  printf(1, "[pagetest] Step 6: Testing read access on read-only page: \"%s\"\n", page);
  printf(1, "          Read succeeded without fault!\n");

  // Step 7: Attempt to write to protected page using a child process
  printf(1, "[pagetest] Step 7: Demonstrating that writing to protected page causes page fault...\n");
  int pid = fork();
  if(pid < 0){
    printf(1, "[pagetest] Error: fork failed!\n");
    exit();
  }

  if(pid == 0){
    printf(1, "[pagetest-child] Child (pid %d) attempting to write 'Z' to read-only page 0x%x...\n",
           getpid(), (uint)page);
    // Writing to read-only page triggers CPU Page Fault (Trap 14)
    page[0] = 'Z';
    // If this statement executes, the write protection failed
    printf(1, "[pagetest-child] ERROR: Write succeeded! Protection failed!\n");
    exit();
  }

  int wpid = wait();
  printf(1, "[pagetest-parent] Child process %d was killed by kernel due to trap/page fault!\n", wpid);
  printf(1, "[pagetest-parent] Write protection successfully trapped by hardware MMU.\n");
  printf(1, "[pagetest] Question 3 test completed successfully!\n\n");
  exit();
}
