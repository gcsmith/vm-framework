/*
** SCCS ID:	@(#)scheduler.c	1.1	04/04/08
**
** File:	scheduler.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Scheduler and dispatcher
*/

#define	__KERNEL__20073__

#include "headers.h"

#include "scheduler.h"
#include "queues.h"
#include "sio.h"

/*
** PUBLIC FUNCTIONS
*/

/*
** _schedule(pcb)
**
** put PCB into the ready queue
**
** returns:
**	NULL if given a NULL argument
**	_enque() status otherwise
*/

Status _schedule( Pcb *pcb ) {
	
	if( pcb == 0 ) {
		return( ERR_NULLPTR );
	}

	pcb->state = ST_READY;

	return( _enque(&g_ready[_PRIO_TO_INDEX(pcb->prio)],(void *)pcb,T_PCB) );

}


/*
** _dispatch(void)
**
** dispatch the next process
*/

void _dispatch( void ) {
	Pcb *pcb;
	int i;
	Status stat;

	for( i = 0; i < N_PRIORITIES; ++i ) {

		stat = _deque( &g_ready[i], (void **)&pcb );

		if( stat == ERR_NONE ) {
			pcb->state = ST_RUNNING;
			g_current = pcb;
			g_quantum = _PRIO_TO_QUANTUM(pcb->prio);
			return;
		}

		if( stat == ERR_EMPTYQ ) {
			continue;
		}

		_kprt_status( "dispatch: ready_g deque status %s\n", stat );
		_kpanic( "dispatch: deque failed" );
	
	}

	_kpanic( "dispatch:  ready queue empty???" );

}
