/*
** SCCS ID:	@(#)syscalls.c	1.1	04/04/08
**
** File:	syscalls.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	System call implementation
*/

#define	__KERNEL__20073__

#include <x86arch.h>
#include <uart.h>
#include "headers.h"
#include "startup.h"

#include "syscalls.h"
#include "clock.h"
#include "processes.h"
#include "queues.h"
#include "scheduler.h"
#include "sio.h"
#include "system.h"
#include "bootstrap.h"
#include "gdt_support.h"
#include "elf.h"
#include "ulib.h"

#define _P2V(pcb, address)  ((unsigned long)(address) - (pcb)->seg.mem.offset)
#define _V2P(pcb, address)  ((unsigned long)(address) + (pcb)->seg.mem.offset)

/*
** PRIVATE GLOBAL VARIABLES
*/

// system call function table

static void (*g_syscalls[N_SYSCALLS])(Context *, unsigned int *);

/*
** PRIVATE FUNCTIONS
*/

/*
** _isr_syscall()
**
** common handler for all system calls; selects the correct
** second-level routine to invoke based on contents of EAX
*/

static void _isr_syscall( int vector, int code ) {
	Context *context;
	unsigned int syscode;

	if( g_current == 0 ) {
		_kpanic( "isr_syscall:  no current process" );
	}

	context = (Context *)_V2P(g_current, g_current->context);
	if( context == 0 ) {
		_kpanic( "isr_syscall:  null context field" );
	}

	syscode = context->eax;
	if( syscode >= N_SYSCALLS ) {
		c_printf( "*** SYSCALL: pid %d code %d - forced exit()\n",
				g_current->pid, syscode );
		syscode = SYS_exit;
	}

	// Invoke the handler.  The first argument is the process
	// context; the second is the address of the first user
	// syscall argument, which is the second longword following
	// the context save area on the stack

	(*g_syscalls[syscode])( context, (((unsigned int *)(context + 1)) + 1) );

	// tell the PIC we're done

	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );

}

/*
** Second-level syscall handlers
**
** All have this prototype:
**
**      static void _sys_NAME( Context *context, unsigned int *args );
*/

/*
** _sys_exit - terminate the current process and clean up after it
**
** usage:       exit();
*/

static void _sys_exit( Context *context, unsigned int *args ) {
	Status status;
	Pid pid;

	// remember who this is

	pid = g_current->pid;

	// reset process state

	g_current->state = ST_FREE;

	// deallocate the segment memory (unless kernel process)
	
	if (g_current->seg.mem.offset != 0) { // check for not kernel process
		_phys_free(g_current->seg.mem);
       		_gdt_free(SEL_GETINDEX(g_current->seg.ldtr));
	} else {
		// deallocate the stack
       		status = _free_stack( g_current->stack );
        	if( status != ERR_NONE ) {
            		c_printf( "sys exit:  pid %d", pid );
            		_kprt_status( " stack free code %s\n", status );
		}
	}

	// deallocate the PCB

	status = _free_pcb( g_current );
	if( status != ERR_NONE ) {
		c_printf( "sys exit:  pid %d", pid );
		_kprt_status( " pcb free code %s\n", status );
	}

	// select a new current process

	_dispatch();

}

/*
** _sys_read - read one character from the SIO
**
** usage:       ch = read();
**
** returns:
**      the next input character
**
** based on a simple interrupt-driven SIO model taken from SP1
*/

static void _sys_read( Context *context, unsigned int *args ) {
	int ch;
	Status status;

	// Try to get the next character

	ch = _sio_read();

	// If there was a character, give it to the process;
	// otherwise, block the process until one comes in.

	if( ch >= 0 ) {

		context->eax = ch;

	} else {

		// No character; put this process on the serial i/o queue

		g_current->state = ST_WAITING;

		status = _enque( &g_sio_blocked, (void *)g_current, T_PCB );
		if( status != ERR_NONE ) {
			_kprt_status( "sys read: enque status %s\n", status );
			_kpanic( "sys read: can't block g_current" );
		}

		// Select a new current process

		_dispatch();

	}

}

/*
** _sys_write - write one character to the SIO
**
** usage:	write( ch );
**
** based on a simple interrupt-driven SIO model taken from SP1
*/

static void _sys_write( Context *context, unsigned int *args ) {
	int ch = (int) args[0];

	// Almost insanely simple, but this separates the
	// low-level device access from the higher-level
	// syscall implementation.

	_sio_write( ch );

}

/*
** _sys_sleep - put the current process to sleep
**
** usage:	sleep( ticks );
*/

static void _sys_sleep( Context *context, unsigned int *args ) {
	Status status;
	unsigned int ticks;

	// get the user sleep time

	ticks = args[0];

	// If the sleep time is 0, just schedule the process
	// (i.e., treat it as a "yield" syscall)
	//
	// Otherwise, calculate the wakeup time and block the process

	if( ticks == 0 ) {

		status = _schedule( g_current );
		if( status != ERR_NONE ) {
			_kprt_status( "sys sleep: schedule status %s\n",
					status );
			_kpanic( "sys sleep:  schedule current failed" );
		}

	} else {

		g_current->wakeup = g_sys_time + ticks;
		g_current->state = ST_SLEEPING;
		status = _enque( &g_sleeping, (void *) g_current,
				g_current->wakeup );
		if( status != ERR_NONE ) {
			_kprt_status( "sys sleep: sleep enque status %s\n",
					status );
			_kpanic( "sys sleep: sleep enque failed" );
		}

	}

	// either way, we must select a new current process

	_dispatch();

}

/*
** _sys_gettime - return the current system time
**
** usage:	n = gettime();
**
*/

static void _sys_gettime( Context *context, unsigned int *args ) {
	
	context->eax = g_sys_time;

}

/*
** _sys_getpid - return the PID of the current process
**
** usage:	n = getpid();
**
*/

static void _sys_getpid( Context *context, unsigned int *args ) {
	
	context->eax = g_current->pid;

}

/*
** TODO: I DEMAND A DESCRIPTION
*/

static void _sys_puts( Context *context, unsigned int *args ) {
    char *str = (char *)_V2P(g_current, args[0]);
	c_puts(str);
}

/*
** TODO: I DEMAND A DESCRIPTION
*/

static void _sys_putc( Context *context, unsigned int *args ) {
	char msg = (char)(args[0]);
	c_putchar(msg);
}

/*
** _sys_fork - create a duplicate of the current process
**
** usage:	id = fork();
**
** returns:
**	-1  on error
**	= 0 in the new process
**	> 0 in the original process (pid of the new process)
*/

static void _sys_fork( Context *context, unsigned int *args ) {
	Pcb *pcb;
	Stack *dst_stack, *src_stack;
	Context *phys_context;
	Status status;

	// first, allocate a new PCB

	pcb = _get_pcb();
	if( pcb == 0 ) {
		context->eax = -1;
		return;
	}

	// duplicate the PCB and stack from the current process

	_memcpy( (void *)pcb, (void *)g_current, sizeof(Pcb) );

	// allocate a segment for the process, copy the callers address space

	if (_phys_alloc(SEGMENT_SIZE, &pcb->seg.mem) == 0) {
		_kpanic("sys_fork: failed to allocate memory for process");
	}

    _phys_copy(pcb->seg.mem, g_current->seg.mem, SEGMENT_SIZE);

	// set up the process' stack at the beginning of its segment

	src_stack = (Stack *)_V2P(g_current, g_current->stack);
	dst_stack = (Stack *)_V2P(pcb, 0);

	_memcpy((void *)dst_stack, src_stack, sizeof(Stack));

	// assign information specific to this process

	pcb->pid   = g_next_pid++;	// REMOVE FROM _get_pcb();
	pcb->ppid  = g_current->pid;
	pcb->stack = (Stack *)0;

	// We duplicated the parent's stack contents; this means that
	// the child's context pointer, ESP, and EBP are still pointing
	// into the parent's stack.  We must correct this.
	//
	// Determine the distance between the two stacks in memory.  
	// This is the adjustment value we must add to the three
	// pointers to correct them.
	//
	// update the pointers

	pcb->context = (Context *)((int)g_current->context - (int)g_current->stack);
	phys_context = (Context *)_V2P(pcb, pcb->context);

	phys_context->esp = (int)context->esp - (int)g_current->stack;
	phys_context->ebp = (int)context->ebp - (int)g_current->stack;

	// IMPORTANT NOTE
	//
	// We have to change EBP because that's how the compiled code for
	// the user process accesses its local variables.  If we didn't
	// change this, as soon as the child was dispatched, it would
	// start to stomp on the local variables in the parent's stack.
	//
	// The child's ESP and EBP will be set by the context restore when
	// we dispatch it.  However, we don't fix the EBP chain, so if the
	// child doesn't immediately exec() but instead returns, it will
	// switch back over to the parent's stack.
	//
	// None of this would be an issue if we were doing "real" virtual
	// memory, as we would be talking about virtual addresses here rather
	// than physical addresses, and all processes would share the same
	// virtual address space layout.

	// set up return values for the parent and child

	context->eax = pcb->pid;
	phys_context->eax = 0;

	// Set the segment selectors.
	phys_context->ss = LDT_DSEG;
	phys_context->ds = LDT_DSEG;
	phys_context->es = LDT_DSEG;
	phys_context->fs = LDT_DSEG;
	phys_context->gs = LDT_DSEG;
	phys_context->cs = LDT_CSEG;

	// Initialize the descriptors in the LDT
	__set_descriptor(&(pcb->seg.ldt.dseg), pcb->seg.mem.offset, 0xFFFF,
			ACC_D_RW | ACC_NONSYS | ACC_PRES);
	__set_descriptor(&(pcb->seg.ldt.cseg), pcb->seg.mem.offset, 0xFFFF,
			ACC_C_ER | ACC_NONSYS | ACC_PRES);

	// Allocate a slot in the GDT for the LDT descriptor,
	// and initialize this PCB's LDT register variable.
	pcb->seg.ldtr = SEL_SETINDEX(_gdt_alloc()) | SEL_GDT | SEL_RPL(0);

	// Initialize the LDT descriptor in the GDT
	__gdt_set_entry( SEL_GETINDEX(pcb->seg.ldtr),
			(u32_t)&(pcb->seg.ldt), sizeof(ldt_t),
			ACC_PRES | ACC_DPL(0) | ACC_SYS | SYS_LDT );

    phys_context->ldtr = pcb->seg.ldtr;

	// Philosophical issue:  should the child run immediately, or
	// should the parent continue?
	//
	// We opt for the former; we schedule the child first, then
	// schedule the parent, so that the child will be dispatched
	// before the parent.

	status = _schedule( pcb );
	if( status != ERR_NONE ) {
		_kprt_status( "sys fork: schedule child status %s\n", status );
		_kpanic( "sys fork:  schedule child failed" );
	}

	status = _schedule( g_current );
	if( status != ERR_NONE ) {
		_kprt_status( "sys fork: schedule parent status %s\n", status );
		_kpanic( "sys fork:  schedule parent failed" );
	}

	// select the next process to run

	_dispatch();

}

#define ARGV_EXEC (char **)-1

void _exec_common(Pcb *pcb, physblock_t old, unsigned int vaddr, char **argv) {
	// Clear out and initialize our new stack

	Stack *dst_stack = (Stack *)_V2P(pcb, pcb->stack);
	unsigned int *ptr = (unsigned int *)dst_stack[1];
	unsigned int offsets[32];
	unsigned int poffsets = 0;
	int index = 0, i;

	_memclr((void *)dst_stack, sizeof(Stack));

	// If present, copy over the command line arguments

	if (NULL != argv) {
		char **pargs = (char **)((unsigned int)argv + old.offset);

		while (*pargs) {
			char *arg = old.offset + *pargs++;
			int length = strlen(arg);

			ptr -= length;
			strcpy((char *)ptr, arg);
			offsets[index++] = (unsigned long)ptr;
		}

		for (i = index - 1; i >= 0; i--) {
			--ptr;
			*(unsigned int *)ptr = _P2V(pcb, offsets[i]);
		}

		poffsets = (unsigned int)_P2V(pcb, ptr);
	}

	if (argv == ARGV_EXEC) {
		ptr -= 2;
		*ptr = (unsigned int)exit;
	}
	else {
		*(--ptr) = poffsets; /* set argv */
		*(--ptr) = index;    /* set argc */
	}

	pcb->context = (Context *)ptr - 1;
	pcb->context->eip = (unsigned int)vaddr;
	pcb->context->esp = (unsigned int)ptr;
	pcb->context->eflags = DEFAULT_EFLAGS;

	// Initialize segment selectors
	pcb->context->ss = LDT_DSEG;
	pcb->context->ds = LDT_DSEG;
	pcb->context->es = LDT_DSEG;
	pcb->context->fs = LDT_DSEG;
	pcb->context->gs = LDT_DSEG;
	pcb->context->cs = LDT_CSEG;

	// Initialize descriptors inside LDT
	__set_descriptor(&(pcb->seg.ldt.dseg), pcb->seg.mem.offset, 0xFFFF,
			ACC_D_RW | ACC_NONSYS | ACC_PRES);
	__set_descriptor(&(pcb->seg.ldt.cseg), pcb->seg.mem.offset, 0xFFFF,
			ACC_C_ER | ACC_NONSYS | ACC_PRES);

	// Already have a slot in the GDT for our LDT descriptor,
	// so no need to initialize it.

	// Initialize the descriptor in the GDT for the LDT
	__gdt_set_entry( GDT_INDEX(pcb->seg.ldtr),
			(u32_t)&(pcb->seg.ldt), sizeof(ldt_t),
			ACC_PRES | ACC_DPL(0) | ACC_SYS | SYS_LDT );

    pcb->context->ldtr = pcb->seg.ldtr;
	
	// Philosophical issue:  schedule it, or let it complete what's
	// left of its current quantum?
	//
	// We opt for the latter, and don't schedule it.

	pcb->context = (Context *)_P2V(pcb, pcb->context);

	// destroy the calling process' segment. although it may seem like a waste
	// of memory to keep the process around until the new segment has been
	// initialized, doing this allows us to copy over command line arguments and
	// prevents bad things from happening should exec fail

	_phys_free(old);
}

/*
** _sys_exec - replace the memory image of the current process. dirt simple
**             exec that just makes a copy of the kernel image and begins
**             execution at the specified "user function" address.
**
** usage:	exec( fcn );
**
*/

static void _sys_exec( Context *context, unsigned int *args ) {
	Pcb *pcb = g_current;
	physblock_t old = pcb->seg.mem;

	/* allocate a new segment for the process */
	if (_phys_alloc(SEGMENT_SIZE, &pcb->seg.mem) == 0) {
		_kpanic("sys_fork: failed to allocate memory for process");
	}

    /* copy the whole kernel over into our new address space */
    _phys_copy(pcb->seg.mem, old, SEGMENT_SIZE);

    /* perform the rest of the shared exec() code */
    _exec_common(pcb, old, args[0], ARGV_EXEC);
}

/*
** _sys_execve - implementation of the more traditional execve system call.
**               instead of copying the kernel image, this syscall loads a
**               separately compiled and linked elf executable. as there is
**               no I/O subsystem or filesystem implementation, the string is
**               simply an address in main memory where the elf image is stored.
**               as is, this address is specified in the build process using
**               the BuildImage tool.
**
** usage:	execve( name, argv );
**
*/

static void _sys_execve( Context *context, unsigned int *args ) {
	Pcb *pcb = g_current;
	physblock_t old = pcb->seg.mem;
	Elf32_Ehdr *header;
    char *str = (char *)_V2P(pcb, args[0]);

	/* assuming ELF loaded on floppy at fixed address, verify address */
	unsigned int address = strtol((const char *)str, NULL, 0);
	if (!address) {
		return;
	}

	/* verify that this is a valid ELF32 image */
	header = (Elf32_Ehdr *)address;
	if (!_elf_valid(header)) {
		return;
	}

	/* allocate a new segment for the process and set stack address */
	if (_phys_alloc(SEGMENT_SIZE, &pcb->seg.mem) == 0) {
		_kpanic("sys_fork: failed to allocate memory for process");
	}

	if (!_elf_load(header, (void *)pcb->seg.mem.offset, (void *)address)) {
		return;
	}

    /* perform the rest of the shared exec() code */
    _exec_common(pcb, old, header->e_entry, (char **)args[1]);
}

/*
** _sys_hotswap()
**
** Performs a fast soft-reset by re-loading the disk image from the floppy
** device and passing control to the bootloader at 0x7C00.
**
** Given that we must use BIOS services to accomplish this, we must switch
** back into 16-bit real mode and do the bulk of the work in low memory.
**
** usage:   hotswap()
**
*/

static void _sys_hotswap( Context *context, unsigned int *args ) {
    c_printf("attempting to hot swap kernel...\n");

    /* disable clock interrupts */
    __outb( TIMER_0, 0 );
    __outb( TIMER_0, 0 );

    /* disable serial interrupts */
    __outb(UA4_IER, 0);

    asm("cli\n"
        "lcall %0, %1\n"
        : /* no outputs */
        : "i"(GDT_CODE), "i"(RMTEXT_ADDRESS));

    c_printf("let's hope we eventually dont get here\n");
}

/*
** PUBLIC FUNCTIONS
*/

/*
** _init_syscalls()
**
** initialize the syscall module
*/

void _init_syscalls( void ) {

	// Set up the table of handlers

	g_syscalls[ SYS_exec ]    = _sys_exec;
	g_syscalls[ SYS_read ]    = _sys_read;
	g_syscalls[ SYS_write ]   = _sys_write;
	g_syscalls[ SYS_sleep ]   = _sys_sleep;
	g_syscalls[ SYS_fork ]    = _sys_fork;
	g_syscalls[ SYS_exit ]    = _sys_exit;
	g_syscalls[ SYS_gettime ] = _sys_gettime;
	g_syscalls[ SYS_getpid ]  = _sys_getpid;
    g_syscalls[ SYS_puts ]    = _sys_puts;
    g_syscalls[ SYS_putc ]    = _sys_putc;
    g_syscalls[ SYS_execve ]  = _sys_execve;
    g_syscalls[ SYS_hotswap ] = _sys_hotswap;

	// Register the ISR

	__install_isr( INT_VEC_SYSCALL, _isr_syscall );

	// Report this module as initialized

	c_puts( " syscalls" );

}
