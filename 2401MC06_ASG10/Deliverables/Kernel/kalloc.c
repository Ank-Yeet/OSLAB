// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

// Assignment 10, Question 1: Counters for physical memory allocation and deallocation
uint total_allocated_pages = 0;
uint total_freed_pages = 0;
static int in_freerange = 0; // Flag to exclude allocator bootstrap from freed counters

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  in_freerange = 1; // Mark bootstrap phase so initial pages are not counted as freed
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
  in_freerange = 0;
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  // Assignment 10, Question 1: Track freed pages
  if(!in_freerange)
    total_freed_pages++;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    // Assignment 10, Question 1: Track allocated pages
    total_allocated_pages++;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

// Assignment 10, Question 1: System call to display physical memory statistics
int
memstats(void)
{
  uint allocated, freed, current, available;
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  allocated = total_allocated_pages;
  freed = total_freed_pages;
  current = allocated - freed;

  available = 0;
  for(r = kmem.freelist; r; r = r->next)
    available++;

  if(kmem.use_lock)
    release(&kmem.lock);

  cprintf("\n--- Physical Memory Statistics ---\n");
  cprintf("Total allocated pages: %d\n", allocated);
  cprintf("Total freed pages: %d\n", freed);
  cprintf("Currently allocated pages: %d\n", current);
  cprintf("Available physical memory pages: %d\n", available);
  cprintf("----------------------------------\n\n");

  return 0;
}


