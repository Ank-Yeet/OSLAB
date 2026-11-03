#include "types.h"
#include "stat.h"
#include "user.h"

int global_counter = 0x1234;

int
main(int argc, char *argv[])
{
  int local_var = 0x5678;

  printf(1, "\n=======================================================\n");
  printf(1, "  Question 2: Virtual-to-Physical Address Translation \n");
  printf(1, "=======================================================\n\n");

  // 1. Code Segment
  int code_va = (int)main;
  int code_pa = vtop(code_va);
  printf(1, "[vttest] 1. Code Segment (main function):\n");
  printf(1, "         Virtual Address  : 0x%x\n", code_va);
  if(code_pa != -1){
    printf(1, "         Physical Address : 0x%x (PFN: 0x%x, Offset: 0x%x)\n",
           code_pa, (uint)code_pa >> 12, code_pa & 0xFFF);
  } else {
    printf(1, "         Physical Address : UNMAPPED\n");
  }

  // 2. Data Segment
  int data_va = (int)&global_counter;
  int data_pa = vtop(data_va);
  printf(1, "[vttest] 2. Data Segment (&global_counter):\n");
  printf(1, "         Virtual Address  : 0x%x\n", data_va);
  if(data_pa != -1){
    printf(1, "         Physical Address : 0x%x (PFN: 0x%x, Offset: 0x%x)\n",
           data_pa, (uint)data_pa >> 12, data_pa & 0xFFF);
  } else {
    printf(1, "         Physical Address : UNMAPPED\n");
  }

  // 3. User Stack Segment
  int stack_va = (int)&local_var;
  int stack_pa = vtop(stack_va);
  printf(1, "[vttest] 3. User Stack Segment (&local_var):\n");
  printf(1, "         Virtual Address  : 0x%x\n", stack_va);
  if(stack_pa != -1){
    printf(1, "         Physical Address : 0x%x (PFN: 0x%x, Offset: 0x%x)\n",
           stack_pa, (uint)stack_pa >> 12, stack_pa & 0xFFF);
  } else {
    printf(1, "         Physical Address : UNMAPPED\n");
  }

  // 4. Heap Allocation with sbrk()
  printf(1, "[vttest] 4. Heap Memory via sbrk(8192) [2 pages]:\n");
  char *heap = sbrk(8192);
  if(heap == (char*)-1){
    printf(1, "[vttest] Error: sbrk failed!\n");
    exit();
  }

  // Page 1 base
  int h1_va = (int)heap;
  int h1_pa = vtop(h1_va);
  printf(1, "         Heap Page 1 Base : VA=0x%x -> PA=0x%x (PFN: 0x%x, Offset: 0x%x)\n",
         h1_va, h1_pa, (uint)h1_pa >> 12, h1_pa & 0xFFF);

  // Page 1 with offset 0x2A4
  int h1_off_va = (int)heap + 0x2A4;
  int h1_off_pa = vtop(h1_off_va);
  printf(1, "         Heap Page 1+0x2A4: VA=0x%x -> PA=0x%x (PFN: 0x%x, Offset: 0x%x)\n",
         h1_off_va, h1_off_pa, (uint)h1_off_pa >> 12, h1_off_pa & 0xFFF);

  // Page 2 base
  int h2_va = (int)heap + 4096;
  int h2_pa = vtop(h2_va);
  printf(1, "         Heap Page 2 Base : VA=0x%x -> PA=0x%x (PFN: 0x%x, Offset: 0x%x)\n",
         h2_va, h2_pa, (uint)h2_pa >> 12, h2_pa & 0xFFF);

  // 5. Unmapped / Invalid Virtual Address
  int invalid_va = 0x60000000;
  int invalid_pa = vtop(invalid_va);
  printf(1, "[vttest] 5. Demonstrating Unmapped Virtual Address:\n");
  printf(1, "         Testing VA 0x%x ...\n", invalid_va);
  printf(1, "         vtop(0x%x) returned %d\n", invalid_va, invalid_pa);
  if(invalid_pa == -1){
    printf(1, "         Result: PASSED (Successfully identified as unmapped/invalid!)\n");
  } else {
    printf(1, "         Result: FAILED (Returned non -1 value)\n");
  }

  // 6. Deallocate heap and test again
  printf(1, "[vttest] 6. Testing After Heap Deallocation via sbrk(-8192):\n");
  sbrk(-8192);
  int dealloc_pa = vtop(h1_va);
  printf(1, "         vtop(0x%x) returned %d\n", h1_va, dealloc_pa);
  if(dealloc_pa == -1){
    printf(1, "         Result: PASSED (Deallocated page is now unmapped!)\n");
  } else {
    printf(1, "         Result: FAILED\n");
  }

  printf(1, "[vttest] Question 2 test completed successfully!\n\n");
  exit();
}
