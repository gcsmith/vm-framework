/*
** File:	segment.h
**
** Author:	Garrett Smith
**
** Contributor:
**
** Description:	Memory allocation and segment creation routines.
*/

#ifndef _SEGMENT_H
#define _SEGMENT_H

#include "types.h"
#include "gdt_support.h"	// for ldt_t

#define MAX_BLOCKS      256
#define SEGMENT_SIZE    (1 << 20)
#define MEMORY_ALIGN    4096

typedef unsigned long addr_t;

typedef struct freeblock_s {
    addr_t offset;                  /* offset into physical memory */
    addr_t length;                  /* length of memory region */
    struct freeblock_s *next;       /* pointer to next block */
} freeblock_t; // 12 bytes

typedef struct physblock_s {
    addr_t offset;                  /* offset into physical memory */
    addr_t length;                  /* length of memory region */
} physblock_t; // 8 bytes

typedef struct segment_s {
    physblock_t mem;                /* allocated memory parameters */
    ldt_t	ldt;                /* LDT structure for the containing PCB */
    u32_t	ldtr;               /* LDT register contents (index, TI, RPL) */
} segment_t; // 20 bytes?

typedef segment_t *seghandle_t;

void _init_memory(void);
void _debug_memory(void);
int  _phys_alloc(unsigned long size, physblock_t *memory);
int  _phys_resize(physblock_t *mem, addr_t length);
void _phys_free(physblock_t memory);
void _phys_copy(physblock_t dst, physblock_t src, unsigned int length);
int  _gdt_alloc(void);
void _gdt_free(int index);
addr_t _get_physmem(void);
addr_t _get_kernstart(void);
addr_t _get_kernlength(void);

#endif /* _SEGMENT_H */

