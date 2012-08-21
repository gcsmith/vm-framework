/*
** SCCS ID:	@(#)processes.c	1.1	04/04/08
**
** File:	processes.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	process-related routines
*/

#define	__KERNEL__20073__

#include "headers.h"

#include "processes.h"
#include "queues.h"
#include "bootstrap.h"
#include "gdt_support.h"	/* for g_gdt_table, etc */

/*
** PRIVATE GLOBAL VARIABLES
*/

static Pcb g_pcbs[N_PROCESSES];		// all available PCBs

/*
** PUBLIC GLOBAL VARIABLES
*/

Pcb *g_current;			// current process
int g_quantum;			// quantum remaining to current process
Pid g_next_pid;			// next available PID

/*
** PUBLIC FUNCTIONS
*/

/*
** _get_pcb()
**
** allocate a PCB structure
**
** returns a pointer to the PCB, or NULL on failure
*/

Pcb *_get_pcb( void ) {
	Pcb *pcb;
	Status status;

	status = _deque( &g_free_pcbs, (void **)&pcb );
	if( status != ERR_NONE ) {
		_kprt_status( "get pcb: deque status %d\n", status );
		return( 0 );
	}

	_memclr( (void *) pcb, sizeof(Pcb) );

	return( pcb );

}

/*
** _free_pcb()
**
** deallocate a PCB structure
**
** returns the status from the _enque() call, or ERR_NULLPTR
*/

Status _free_pcb( Pcb *pcb ) {

	if( pcb != 0 ) {
		return( _enque(&g_free_pcbs,(void *)pcb,T_PCB) );
	}

	return( ERR_NULLPTR );

}

/*
** _init_processes()
**
** initialize all process-related data structures
*/

void _init_processes( void ) {
	int i;

	// first, put all PCBs into the free pool
	//
	// NOTE:  queues must have been initialized first!

	for( i = 0; i < N_PROCESSES; ++i ) {
		if( _free_pcb(&g_pcbs[i]) != ERR_NONE ) {
			_kpanic( "init process:  enqueue failed" );
		}
	}

	// miscellaneous process-related variables

	// set g_current to a temporary PCB slot. the SIO driver gets initialized
	// shortly after process initialization, but BEFORE the idle and first_main
	// PCBs are created. if an SIO interrupt occurs, causing a call to isr save
	// and restore, it will blow up when it tries to read the empty g_current.
	g_current = &g_pcbs[0];
	__copy_gdt_entry( &(g_current->seg.ldt.cseg), GDT_INDEX(GDT_CODE) );
	__copy_gdt_entry( &(g_current->seg.ldt.dseg), GDT_INDEX(GDT_DATA) );
	g_current->seg.mem.offset = 0;
	g_current->seg.mem.length = 0xFFFFFFFF;
	// yes this probably wastes an entry in the GDT
	g_current->seg.ldtr = SEL_SETINDEX(_gdt_alloc()) | SEL_GDT;
	__gdt_set_entry( GDT_INDEX(g_current->seg.ldtr),
			(u32_t)&(g_current->seg.ldt), sizeof(ldt_t), 
			ACC_PRES | ACC_DPL(0) | ACC_SYS | SYS_LDT );
	
	g_next_pid = 0;		// first PID

	// announce that we have initialized the PCB module

	c_puts( " pcbs" );
}
