#include "types.h"
#include "stat.h"
#include "user.h"

#define NCHILD 3
#define ROUNDS 5

int
main(int argc, char *argv[])
{
  char letters[NCHILD] = { 'A', 'B', 'C' };
  int i, r;
  int pid;

  if(sem_init(0, 1) < 0 || sem_init(1, 0) < 0 || sem_init(2, 0) < 0){
    printf(1, "synctest: sem_init failed\n");
    exit();
  }

  for(i = 0; i < NCHILD; i++){
    pid = fork();
    if(pid < 0){
      printf(1, "synctest: fork failed\n");
      exit();
    }
    if(pid == 0){
      for(r = 0; r < ROUNDS; r++){
        sem_wait(i);
        printf(1, "%c ", letters[i]);
        sem_post((i + 1) % NCHILD);
      }
      exit();
    }
  }

  for(i = 0; i < NCHILD; i++)
    wait();

  printf(1, "\nDone\n");

  printf(1, "Error checks: sem_init(8,1)=%d sem_init(0,-1)=%d sem_wait(-1)=%d sem_post(9)=%d\n",
         sem_init(8, 1), sem_init(0, -1), sem_wait(-1), sem_post(9));
  printf(1, "Uninitialised: sem_wait(5)=%d sem_post(5)=%d\n",
         sem_wait(5), sem_post(5));

  exit();
}
