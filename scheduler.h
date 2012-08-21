/*
** SCCS ID:	@(#)scheduler.h	1.1	04/04/08
**
** File:	scheduler.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Scheduler and dispatcher
*/

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

/*
** General (C and/or assembly) definitions
*/

#ifndef __ASM__20073__

#include "types.h"

#include "processes.h"

/*
** Start of C-only definitions
*/

/*
** Prototypes
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

Status _schedule( Pcb *pcb );

/*
** _dispatch(void)
**
** dispatch the next process
*/

void _dispatch( void );

#endif

#endif
