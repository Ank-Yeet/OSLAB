#include "types.h"
#include "stat.h"
#include "user.h"

int global_var = 42;

int
main(int argc, char *argv[])
{
  int before_fork, pid;

  printf(1, "(i)   freepages at start            : %d\n", freepages());

  sbrk(10 * 4096);
  printf(1, "(ii)  freepages after sbrk(10*4096) : %d\n", freepages());

  printf(1, "(iii) &global_var  va = 0x%x  pa = 0x%x\n",
         (uint)&global_var, va2pa((int)&global_var));

  printf(1, "(iv)  va2pa(0x7FFFFFFF) = %d\n", va2pa(0x7FFFFFFF));

  before_fork = freepages();
  printf(1, "(v)   freepages before fork         : %d\n", before_fork);

  pid = fork();
  if(pid < 0){
    printf(1, "memtest: fork failed\n");
    exit();
  }
  if(pid == 0){
    printf(1, "      child : va = 0x%x  pa = 0x%x\n",
           (uint)&global_var, va2pa((int)&global_var));
    exit();
  }

  int parent_pa = va2pa((int)&global_var);
  int after_fork = freepages();
  wait();
  printf(1, "      parent: va = 0x%x  pa = 0x%x\n",
         (uint)&global_var, parent_pa);
  printf(1, "      freepages after fork          : %d\n", after_fork);
  printf(1, "      freepages after wait          : %d (before fork: %d) %s\n",
         freepages(), before_fork,
         freepages() == before_fork ? "OK, no leak" : "MISMATCH");
  exit();
}
