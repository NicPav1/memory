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
int z = 0; 

struct run {
  struct run *next;
};

struct run *processes[16384]; //ADDED for v2

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
    pid[p] = -2; //ADDED
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
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE) //CHANGED
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
  if(kmem.use_lock) {
    for (int e = 0; e < 16384; e++) { //ADDED
      if (frames[e] == (uint)(V2P(v) >> 12 & 0xffff)) {
        for (int n = e; n < 16383; n++){
          frames[n] = frames[n+1];
          pid[n] = pid[n+1];
          processes[n] = processes[n+1];
        }
        frames[16383] = - 1;
        pid[16383] = -2;
        //break;
      }
    } //END of ADDED
    z--;
    release(&kmem.lock);
  }
}



// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  int p = -2;
  return kalloc2(p);
  
}

char*
kalloc2(int p) {
  struct run *r;
  struct run *rr;
  int rrr = 0;
  
  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) 
    kmem.freelist = r->next;
  if(kmem.use_lock) {
    int shift = (uint)(V2P(r) >> 12 & 0xffff);
    if (z != 0) {
      if (pid[z-1] == p || p == -2 || pid[z-1] == -2) {
        frames[z] = shift;
        processes[z] = r;
        pid[z] = p;
        z++;
      }
      else if (pid[z-1] != p){
        rr = kmem.freelist;
        if (rr)
          kmem.freelist = rr->next;
        frames[z] =  (uint)(V2P(rr) >> 12 & 0xffff);
        processes[z] = rr;
        pid[z] = p;
        z++;
        rrr = 1;
      }
    }
    else {
      frames[z] = shift;
      processes[z] = r;
      pid[z] = p;
      z++;
    }
    
    release(&kmem.lock);
  }
  if (rrr == 1) return (char*)rr;
  else return (char*)r;
}

int
dump_mem(int *f, int *p, int n) {
  for (int u = 0; u < 16384; u++)                     //Loop for ascending ordering
	{
		for (int v = 0; v < 16384; v++)             //Loop for comparing other values
		{
			if (frames[v] < frames[u])                //Comparing other array elements
			{
				int tmpf = frames[u];
        int tmpp = pid[u];         //Using temporary variable for storing last value
				frames[u] = frames[v];            //replacing value
        pid[u] = pid[v];
				frames[v] = tmpf;             //storing last value
        pid[v] = tmpp;
			}  
		}
	}
  for (int j = 0; j < n; j++) {
    if (frames[j] != -1) {
      f[j] = frames[j];
      p[j] = pid[j];
    }
  }
  return 0;
}
