#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "\n=======================================================\n");
  printf(1, "  Question 1: Physical Memory Allocation Tracking Test \n");
  printf(1, "=======================================================\n\n");

  printf(1, "[memtest] Step 1: Displaying initial physical memory statistics...\n");
  memstats();

  printf(1, "[memtest] Step 2: Allocating 4 pages (16384 bytes) via sbrk(16384)...\n");
  char *ptr = sbrk(16384);
  if(ptr == (char*)-1){
    printf(1, "[memtest] Error: sbrk allocation failed!\n");
    exit();
  }

  // Touch the allocated pages
  int i;
  for(i = 0; i < 16384; i += 4096){
    ptr[i] = 'A' + (i / 4096);
  }
  printf(1, "[memtest] Successfully touched all 4 newly allocated pages.\n");

  printf(1, "[memtest] Step 3: Displaying memory statistics after allocation...\n");
  memstats();

  printf(1, "[memtest] Step 4: Releasing 4 pages via sbrk(-16384)...\n");
  if(sbrk(-16384) == (char*)-1){
    printf(1, "[memtest] Error: sbrk deallocation failed!\n");
    exit();
  }
  printf(1, "[memtest] Deallocation completed.\n");

  printf(1, "[memtest] Step 5: Displaying memory statistics after deallocation...\n");
  memstats();

  printf(1, "[memtest] Question 1 test completed successfully!\n\n");
  exit();
}
