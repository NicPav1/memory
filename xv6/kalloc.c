// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "proc.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld
int frames[16384];
int pid[16384];

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

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
  for (int i = 0; i < 16384; i++) {
    frames[i] = -1;
  }
  for (int p = 0; p < 16384; p++) {
    pid[p] = -2;
  }
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
  for(; p + PGSIZE <= (char*)vend; p += 2*PGSIZE)
    kfree(p);
}
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
  if(kmem.use_lock)
    release(&kmem.lock);
}

int z = 0; 
//int startNum = 57343; // Saw on Piazza and tests

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  int p = -2;
  return kalloc2(p);

  //struct run *r;

  //if(kmem.use_lock)
  //  acquire(&kmem.lock);
  //r = kmem.freelist;
  //if(r)
  //  kmem.freelist = r->next;
  //if(kmem.use_lock) {
  //  int shift = (uint)(V2P(r) >> 12 & 0xffff);
  //  frames[z] = shift + 1;
    //pid[z] = myproc()->pid;
  //  z++;
  //  release(&kmem.lock);
  //}
  //return (char*)r;
  
  
}

char*
kalloc2(int p) {
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  if(kmem.use_lock) {
    int shift = (uint)(V2P(r) >> 12 & 0xffff);
    frames[z] = shift + 1;
    pid[z] = p;
    //pid[z] = myproc()->pid;
    z++;
    release(&kmem.lock);
  }
  return (char*)r;
}

int
dump_mem(int *f, int *p, int n) {
  for (int j = 0; j < n; j++) {
    //if (frames[j] != -1) {
      f[j] = frames[j];
      p[j] = pid[j];
      //cprintf("%d", j);
    //}
  }
  return 0;
}
