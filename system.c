/*
** SCCS ID:	@(#)system.c	1.1	04/04/08
**
** File:	system.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Miscellaneous system routines
*/

#define	__KERNEL__20073__

#include "headers.h"

#include "ulib.h"

#include "system.h"
#include "clock.h"
#include "queues.h"
#include "scheduler.h"
#include "sio.h"
#include "stacks.h"
#include "syscalls.h"
#include "user.h"

#include "bootstrap.h"
#include "startup.h"
#include "support.h"

/*
** PRIVATE DATA TYPES
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** _setup_stack(stack,entry)
**
** initialize a process context
**
** ASSUMES that the stack argument points to a stack which can be
** cleared and initialized
*/

Context *_setup_stack( Stack *stack, unsigned int entry ){
	Context *context;
	unsigned int *ptr;

	// start by clearing the stack

	_memclr( (void *)stack, sizeof(Stack) );

	//
	// We need to set up the initial stack contents for a (new)
	// user process.
	//
	// We reserve a longword at the bottom of the stack for
	// some scratch space.  Above this, we'll place a dummy 
	// "return address" so that if the process ever returns
	// from its main routine it will "return" to the exit()
	// system call.  Finally, above that we'll initialize a
	// context for the process to use when dispatched.
	//

	// find the location immediately after the stack

	ptr = (unsigned int *)(stack + 1);

	// back up two longwords' distance

	ptr -= 2;

	// assign the dummy return address

	*ptr = (unsigned int) exit;

	// figure out where the process context area should be

	context = ((Context *)ptr) - 1;

	// initialize all the register fields in the context
	// area that should contain something other than zero
	//
	// first, the segment register save areas

	// This function is used for two processes that run within the kernel,
	// so they use the kernel stack. But since they are processes, they still
	// need an LDT.

	context->ss = LDT_DSEG;
	context->ds = LDT_DSEG;
	context->es = LDT_DSEG;
	context->fs = LDT_DSEG;
	context->gs = LDT_DSEG;

	context->cs = LDT_CSEG;

	// next, the entry point for the process

	context->eip = (unsigned int) entry;

	// the initial EFLAGS settings

	context->eflags = DEFAULT_EFLAGS;

	// finally, the context pointer goes into the PCB
	// so that the context can be "restored" when this
	// process is eventually dispatched

	context->esp = (unsigned int) ptr;

	// return the context pointer

	return( context );

}


/*
** _init:	system initialization routine
**
** Called by startup code immediately before returning into the
** first user-level process.  The name is taken from there.
*/

void _init( void ) {
	Pcb *pcb;

	//
	// BOILERPLATE CODE - taken from basic framework
	//
	// Initialize interrupt stuff.
	//

	__init_interrupts();	// IDT and PIC initialization

	//
	// I/O system.
	//

	c_io_init();
	c_clearscreen();
	c_setscroll( 0, 7, 99, 99 );
	c_puts_at( 0, 6, "================================================================================" );

	c_puts( "Init: " );

	//
	// 20073-SPECIFIC CODE STARTS HERE
	//

	//
	// Initialize various OS modules
	//

	_init_queues();		// must be first
	_init_memory();
	_init_processes();
	_init_stacks();
	_init_sio();
	_init_clock();
	_init_syscalls();

	c_puts( "\n" );

	//
	// Create the initial process
	//
	// Code mostly stolen from _sys_fork() and _sys_exec();
	// if either of those routines change, SO MUST THIS!!!
	//

	//
	// First, get a PCB and a stack
	//

	pcb = _get_pcb();
	if( pcb == 0 ) {
		_kpanic( "_init - can't allocate first pcb" );
	}

	pcb->stack = _get_stack();
	if( pcb->stack == 0 ) {
		_kpanic( "_init - can't allocate first stack" );
	}

	//
	// Next, set up various PCB fields
	//

	pcb->pid  = g_next_pid++;
	pcb->prio = PRI_NORMAL;

	//
	// Set up the initial process context.
	//
	// See _sys_exec() for an explanation of how this works.
	//

	pcb->context = _setup_stack( pcb->stack, (unsigned int) first_main );

	// Initialize memory segment. Equals that of the kernel's in the GDT.
	pcb->seg.mem.offset = 0x0;
	pcb->seg.mem.length = 0xFFFFFFFF;

	// Initialize LDT entries for this PCB
	// This is a "kernel" process, so we will just copy over the
	// descriptors from the GDT and stick them into this process' LDT.
	__copy_gdt_entry( &(pcb->seg.ldt.cseg), (GDT_INDEX(GDT_CODE)) );
	__copy_gdt_entry( &(pcb->seg.ldt.dseg), (GDT_INDEX(GDT_DATA)) );
	
	// Allocate a slot in the GDT for the LDT descriptor,
	// and initialize this PCB's LDT register variable.
	pcb->seg.ldtr = SEL_SETINDEX(_gdt_alloc()) | SEL_GDT | SEL_RPL(0);

	// Initialize the LDT descriptor located in the GDT
	__gdt_set_entry( SEL_GETINDEX(pcb->seg.ldtr),
			(u32_t)&(pcb->seg.ldt), sizeof(ldt_t),
			ACC_PRES | ACC_DPL(0) | ACC_SYS | SYS_LDT );

	//
	// Give it to the scheduler.
	//

	_schedule( pcb );

	//
	// Do it all again for the idle process.
	//

	pcb = _get_pcb();
	if( pcb == 0 ) {
		_kpanic( "_init - can't allocate idle pcb" );
	}

	pcb->stack = _get_stack();
	if( pcb->stack == 0 ) {
		_kpanic( "_init - can't allocate idle stack" );
	}

	pcb->pid  = g_next_pid++;
	pcb->prio = PRI_MINIMUM;

	pcb->context = _setup_stack( pcb->stack, (unsigned int) idle_main );

	pcb->seg.mem.offset = 0x0;
	pcb->seg.mem.length = 0xFFFFFFFF;

	__copy_gdt_entry( &(pcb->seg.ldt.cseg), (GDT_INDEX(GDT_CODE)) );
	__copy_gdt_entry( &(pcb->seg.ldt.dseg), (GDT_INDEX(GDT_DATA)) );
	
	pcb->seg.ldtr = SEL_SETINDEX(_gdt_alloc()) | SEL_GDT | SEL_RPL(0);

	__gdt_set_entry( SEL_GETINDEX(pcb->seg.ldtr),
			(u32_t)&(pcb->seg.ldt), sizeof(ldt_t),
			ACC_PRES | ACC_DPL(0) | ACC_SYS | SYS_LDT );
	
	_schedule( pcb );

	//
	// Dispatch the initial current process
	//

	_dispatch();

	//
	// END OF 20073-SPECIFIC CODE
	//
	// Finally, report that we're all done.
	//

	c_puts( "System initialization complete.\n" );

}
