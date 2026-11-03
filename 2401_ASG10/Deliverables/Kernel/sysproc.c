#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Assignment 10, Question 1: memstats system call wrapper
int
sys_memstats(void)
{
  return memstats();
}

// Assignment 10, Question 2: vtop system call wrapper
int
sys_vtop(void)
{
  int va;
  if(argint(0, &va) < 0)
    return -1;
  return vtop(va);
}

// Assignment 10, Question 3: pageinfo system call wrapper
int
sys_pageinfo(void)
{
  int va;
  if(argint(0, &va) < 0)
    return -1;
  return pageinfo(va);
}

// Assignment 10, Question 3: setreadonly system call wrapper
int
sys_setreadonly(void)
{
  int va;
  if(argint(0, &va) < 0)
    return -1;
  return setreadonly(va);
}

