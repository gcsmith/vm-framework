/*
** SCCS ID:	@(#)processes.h	1.1	04/04/08
**
** File:	processes.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	process-related types and functions
*/

#ifndef _PROCESSES_H
#define _PROCESSES_H

#include "types.h"
#include "segment.h"

/*
** General (C and/or assembly) definitions
*/

	// maximum number of simultaneous processes in the system

#define	N_PROCESSES	24

	// process state

#define	ST_FREE		0
#define	ST_READY	1
#define	ST_RUNNING	2
#define	ST_WAITING	3
#define	ST_SLEEPING	4
#define	ST_TERMINATED	5

#define	N_STATES	6

	// process priorities

#define	PRI_REALTIME	(-2)
#define	PRI_HIGH	(-1)
#define	PRI_NORMAL	0
#define	PRI_LOW		1
#define	PRI_MINIMUM	2

#define	N_PRIORITIES	5

	// macros for converting priority values to other things

#define	_PRIO_TO_INDEX(n)	((n)+2)
#define	_INDEX_TO_PRIO(n)	((n)-2)
#define	_PRIO_TO_QUANTUM(n)	(3-(n))

#ifndef __ASM__20073__

#include "stacks.h"
#include "gdt_support.h"	// for ldt_t in the PCB

/*
** Start of C-only definitions
*/

/*
** Types
*/

typedef unsigned short Pid;		// per-process id
typedef unsigned char State;		// process state
typedef unsigned char Priority;		// process priority

	// process context
	// order of members depends on the register save
	// code in isr_stubs.S!

typedef struct context {
	unsigned int ss;
	unsigned int gs;
	unsigned int fs;
	unsigned int es;
	unsigned int ds;
    unsigned int ldtr;
	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int esp;
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int eax;
	unsigned int vector;
	unsigned int code;
	unsigned int eip;
	unsigned int cs;
	unsigned int eflags;
} Context;

	// process control block
	// fields are ordered by size to minimize space

typedef struct pcb {
	// four-byte fields
	Context *context;	// ptr to context on user stack			0
	Stack *stack;		// ptr to allocated stack			4
	Time wakeup;		// wakeup time for sleeping processes		8
	// two-byte fields
	Pid pid;		// this process' id				12
	Pid ppid;		// pid of parent of this process		14
	// one-byte fields
	State state;		// current process state			16
	Priority prio;		// scheduling priority				17
	// Large-byte field
	segment_t seg;		// Segment for this process; contains an LDT	20?
} Pcb;

/*
** Globals
*/

extern Pcb *g_current;		// current process
extern int g_quantum;		// quantum remaining to current process
extern Pid g_next_pid;		// next available PID

/*
** Prototypes
*/

/*
** _get_pcb()
**
** allocate a PCB structure
**
** returns a pointer to the PCB, or NULL on failure
*/

Pcb *_get_pcb( void );

/*
** _free_pcb()
**
** deallocate a PCB structure
**
** returns the status from the _enque() call, or ERR_NULLPTR
*/

Status _free_pcb( Pcb *pcb );

/*
** _init_processeses()
**
** initialize all process-related data structures
*/

void _init_processes( void );

#endif

#endif
