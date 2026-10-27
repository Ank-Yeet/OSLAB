#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "pttest: calling vmprint() at start-up...\n");
  vmprint();

  printf(1, "\npttest: calling sbrk(4 * 4096) to allocate 4 pages...\n");
  char *p = sbrk(4 * 4096);
  if(p == (char*)-1){
    printf(1, "pttest: sbrk failed\n");
    exit();
  }

  // Touch each page so this works both with eager and lazy allocation
  for(int i = 0; i < 4; i++){
    p[i * 4096] = (char)('A' + i);
  }

  printf(1, "\npttest: calling vmprint() after sbrk(4 * 4096)...\n");
  vmprint();

  printf(1, "\npttest: test finished successfully.\n");
  exit();
}
