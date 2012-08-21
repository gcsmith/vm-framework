/*
** File:	segment.c
**
** Author:	Garrett Smith
**
** Contributor:
**
** Description:	Memory allocation and segment creation routines.
*/

#define __KERNEL__20073__

#include "headers.h"
#include "bootstrap.h"
#include "segment.h"
#include "gdt_support.h"

/* System Call Interface
 * ---------------------
 *
 *  #define CURR_PID (Pid)-1
 *
 *  syscopy(Pid srcid, void *srcaddr, Pid dstid, void *dstaddr, long length);
 *
 *  - srcid and/or dstid may be a valid pid or CURR_PID (for current process)
 *  - push FS and GS to the stack (safety)
 *  - set FS to the source segment (fetch from srcid)
 *  - set GS to the destination segment (from from dstid)
 *  - copy memory (size of length) from FS:srcaddr to GS:dstaddr
 *
 *  seghandle_t makeseg(unsigned long length)
 *
 *  - create a shared segment of size 'length'
 *  - seghandle_t is nothing more than an identifier for user programs
 *  - seghandle_t is a pointer to a structure in kernel address space
 *
 *  segread(seghandle_t seg, void *src, void *dest, unsigned long length)
 *
 *  - copies 'length' bytes from 'src' (shared seg) to 'dest' (current process)
 *  - 'src' is a virtual address in segment 'seg', 'dest' is currenet segment
 *
 *  segwrite(seghandle_t seg, void *src, void *dest, unsigned long length)
 *
 *  - copies 'length' bytes from 'src' (current process) to 'dest' (shared seg)
 *  - 'src' is current segment, 'dest' is virtual address in segment 'seg'
 *
 */

static int g_gdtpool[GDT_MAX];
static int g_gdtcount = 0;

static freeblock_t g_memnodes[MAX_BLOCKS]; /* array of memory block nodes */
static freeblock_t *g_nodepool;            /* free, unused nodes for our list */
static freeblock_t *g_freemem;             /* pool of free memory regions */

static ulong_t g_physical_memory = 0; /* total amount of physical memory */
static ulong_t g_kernel_start = 0;    /* kernel image starting address */
static ulong_t g_kernel_length = 0;   /* kernel image length */

/*
** _mem_mknode()
**
** Allocate a new freeblock_t node from the node pool.
*/

freeblock_t *_mem_mknode(addr_t offset, addr_t length, freeblock_t *next) {
    if (g_nodepool != NULL) {
        /* pop off the first node - move pool head forward */
        freeblock_t *ret = g_nodepool;
        g_nodepool = g_nodepool->next;

        /* initialize and return the new node */
        ret->offset = offset;
        ret->length = length;
        ret->next   = next;
        return ret;
    }

    /* we've run out of nodes - this is bad */
    _kpanic("_mem_getnode - free block node pool exhausted");
    return NULL;
}

/*
** _mem_rmnode()
**
** Free the specified freeblock_t node and return it to the node pool.
*/

void _mem_rmnode(freeblock_t *node) {
    /* simply return the freed node to the head of the list */
    node->next = g_nodepool;
    g_nodepool = node;
}

/*
** _init_memory()
**
** Initialize all memory and segment related data-structures and algorithms.
*/

void _init_memory() {
    unsigned int i;
    u16_t ext_lo, ext_hi;

    /* record the amount of available physical memory */
    ext_lo = *(u16_t *)(MMAP_ADDRESS + MMAP_EXT_LO);
    ext_hi = *(u16_t *)(MMAP_ADDRESS + MMAP_EXT_HI);

    g_physical_memory = (ext_lo + 0x400) * (1 << 10) + ext_hi * (1 << 16);
    g_kernel_start    = TARGET_ADDRESS;
    g_kernel_length   = SEGMENT_SIZE - TARGET_ADDRESS;
    /* g_kernel_length   = *(u16_t *)(MMAP_ADDRESS + MMAP_SECTORS) * 512; */

    /* initialize the free gdt entry pool */
    for (i = 0; i < GDT_MAX; ++i) {
        g_gdtpool[i] = GDT_MAX - i - 1; /* 1023, 1022 ... 1, 0 */
    }

    /* reserved for NULL, LINEAR, CODE, DATA, STACK, CODE16, DATA16 */
    g_gdtcount = GDT_MAX - 7;

    /* initialize the free node pool, we'll use the 0th index initially */
    for (i = 1; i < MAX_BLOCKS; ++i) {
        g_memnodes[i].offset = 0;
        g_memnodes[i].length = 0;
        g_memnodes[i].next   = &g_memnodes[i + 1];
    }

    g_memnodes[MAX_BLOCKS - 1].next = NULL;

    /* create a single node that spans all of memory */
    g_memnodes[0].offset = g_kernel_start + g_kernel_length;
    g_memnodes[0].length = g_physical_memory - g_memnodes[0].offset;
    g_memnodes[0].next   = NULL;

    g_freemem  = &g_memnodes[0];
    g_nodepool = &g_memnodes[1];

    /* set up TSS */
    __tss_set_index(_gdt_alloc());
    __tss_load();

    c_puts(" memory");
}

/*
** _debug_memory()
**
** (For debugging only). Dumps status of the free memory list to the console.
*/

void _debug_memory(void) {
    freeblock_t *curr = g_freemem;
    int i = 0;

    c_printf("<IX> <OFFSET> <LENGTH>\n");
    while (curr != NULL) {
        c_printf(" %3d %8d %8d\n",
               ++i, curr->offset, curr->length);
        curr = curr->next;
    }
    c_printf("\n");
}

/*
** _phys_alloc()
**
** Allocate a linear block of physical memory of 'size' bytes using first fit.
*/

int _phys_alloc(unsigned long size, physblock_t *memory) {
    freeblock_t *curr = g_freemem;
    freeblock_t *prev = NULL;

    if (!memory) {
        _kpanic("_phys_alloc - invalid function argument");
    }
        
    while (curr != NULL) {
        /* is the current free block large enough to fit us? */
        if (curr->length >= size) {

            /* consume the low chunk of the free block's memory for our alloc */
            memory->offset = curr->offset;
            memory->length = size;

            /* split this off from the free block, remove if totally consumed */
            curr->offset += size;
            curr->length -= size;

            /* if we consume the whole block, put the node back in the pool */
            if (!curr->length) {
                if (prev) {
                    prev->next = curr->next;
                }
                else {
                    g_freemem = curr->next;
                }
                _mem_rmnode(curr);
            }

            return 1;
        }

        prev = curr;
        curr = curr->next;
    }

    /* we shouldn't get here unless we're out of memory */
    memory->offset = 0;
    memory->length = 0;
    return 0;
}

/*
** _phys_resize()
**
** Resize the specified block of memory. If there is free memory adjacent to
** this block, it will be resized in place. Otherwise, the block will be freed
** and reallocated elsewhere. Keep in mind that this routine will not actually
** copy the memory from the source to destination segments. This task will be
** left up to the caller.
*/

int _phys_resize(physblock_t *mem, addr_t length) {
    freeblock_t *curr = g_freemem;
    freeblock_t *prev = NULL;
    addr_t target, difference;

    /* make sure the input is valid */
    if (!mem) {
        _kpanic("mem_resize - invalid input parameter");
    }

    /* find the next closest free block beyond the specified segment */
    target = mem->offset + mem->length;

    while (curr && (curr->offset < target)) {
        prev = curr;
        curr = curr->next;
    }

    if (length < mem->length) {
        /* attempt decrease the segment size */
        difference = mem->length - length;
        mem->length -= difference;

        if (target == curr->offset) {
            /* enlarge the free block to fill in the gap */
            curr->offset -= difference;
            curr->length += difference;
        }
        else {
            /* create a new free block and add it to the list */
            addr_t off = mem->offset + mem->length;
            addr_t len = difference;
            freeblock_t *block = _mem_mknode(off, len, curr);

            if (prev) {
                prev->next = block;
            }
            else {
                g_freemem = block;
            }
        }
    }
    else {
        /* attempt to increase the segment size */
        difference = length - mem->length;

        if ((target == curr->offset) && (difference <= curr->length)) {
            /* expand the left segment into the right block */
            mem->length += difference;
            curr->offset += difference;
            curr->length -= difference;

            /* if consumed, remove the node and put it back in the pool */
            if (!curr->length) {
                if (prev) {
                    prev->next = curr->next;
                }
                else {
                    g_freemem = curr->next;
                }
                _mem_rmnode(curr);
            }
            return 1;
        }
        else {
            /* no room, just free this block and attempt to reallocate */
            _phys_free(*mem);
            return _phys_alloc(length, mem);
        }
    }

    return 0;
}

/*
** _phys_free()
**
** Free the specified block of memory. If the block is adjacent to one or more
** free memory regions, their nodes will be combined in the free memory list
** to prevent fragmentation.
*/

void _phys_free(physblock_t mem) {
    freeblock_t *curr = g_freemem;
    freeblock_t *prev = NULL;

    /* verify that the input is within a reasonable range */
    if ((mem.offset < g_kernel_start) ||
        (mem.offset + mem.length) >= g_physical_memory) {
        _kpanic("_phys_free - block exists out of allocatable range");
    }

    /* if the free memory list is empty, simply add a single new node */
    if (!g_freemem) {
        g_freemem = _mem_mknode(mem.offset, mem.length, NULL);
        return;
    }

    /* search each free block and see if we can be placed in front of it */
    while (curr != NULL) {
        if (mem.offset < curr->offset) {

            /* case 1 - merge with free block below us */
            if (prev && (prev->offset + prev->length) == mem.offset) {
                prev->length += mem.length;

                /* have we completely filled in a gap? */
                if ((prev->offset + prev->length) == curr->offset) {
                    /* shift left node into right node, release right node */
                    prev->length += curr->length;
                    prev->next = curr->next;
                    _mem_rmnode(curr);
                }
            } 
            /* case 2 - merge with free block above us */
            else if (curr->offset == (mem.offset + mem.length)) {
                curr->offset -= mem.length;
                curr->length += mem.length;

                /* have we completely filled in a gap? */
                if (prev && curr->offset == (prev->offset + prev->length)) {
                    /* shift left node into right node, release right node */
                    prev->length += curr->length;
                    prev->next = curr->next;
                    _mem_rmnode(curr);
                }
            }
            /* case 3 - create a new free block between the current two */
            else {
                freeblock_t *block = _mem_mknode(mem.offset, mem.length, curr);

                /* attach to the previous block */
                if (prev) {
                    prev->next = block;
                }
                else {
                    g_freemem = block;
                }
            }
            
            return;
        }

        prev = curr;
        curr = curr->next;
    }

    if ((prev->offset + prev->length) == mem.offset) {
        /* extend the last block to include the freed space */
        prev->length += mem.length;
    }
    else {
        /* add a new node at the very end of the list */
        freeblock_t *block = _mem_mknode(mem.offset, mem.length, prev->next);
        prev->next = block;
    }
}

void _phys_copy(physblock_t dst, physblock_t src, unsigned int length) {
    /* offsets, lengths, and copy sizes should be at least dword aligned */
    if (length % MEMORY_ALIGN) {
        _kpanic("_phys_copy: length to copy is not aligned");
    }

    /* do a fast double-word copy (dividing length by 4) */
    asm("cld\n"
        "rep\n"
        "movsd\n"
        : /* no outputs */
        : "c"(length >> 2), "S"(src.offset), "D"(dst.offset));
}

/*
** _gdt_alloc()
**
** Allocate an unused GDT descriptor index.
*/

int _gdt_alloc(void) {
    if (g_gdtcount == 0) {
        _kpanic("_gdt_alloc - free gdt pool exhausted");
    }

    return g_gdtpool[--g_gdtcount];
}

/*
** _gdt_free()
**
** Free the GDT descriptor index.
*/

void _gdt_free(int index) {
    if (g_gdtcount == GDT_MAX) {
        _kpanic("_gdt_free - free gdt pool overflowed");
    }

    g_gdtpool[g_gdtcount++] = index;
}

/*
** _get_physmem()
**
** Returns the total amount of physical memory available in the system.
*/

addr_t _get_physmem(void) {
    return g_physical_memory;
}

/*
** _get_kernstart()
**
** Returns the address where the kernel image is loaded into memory.
*/

addr_t _get_kernstart(void) {
    return g_kernel_start;
}

/*
** _get_kernlength()
**
** Returns the size, in bytes, of the kernel image in memory.
*/

addr_t _get_kernlength(void) {
    return g_kernel_length;
}

