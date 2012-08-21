/*
** SCCS ID:	@(#)queues.h	1.1	04/04/08
**
** File:	queues.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	queue manipulation types and routines
*/

#ifndef _QUEUES_H
#define _QUEUES_H

/*
** General (C and/or assembly) definitions
*/

#ifndef __ASM__20073__

#include "headers.h"

#include "processes.h"

/*
** Start of C-only definitions
*/

/*
** Types
*/

	// Queue header

typedef struct queue {
	void *front;			// first element in queue
	void *back;			// last element in queue
	int (*compare)(void *, void *); // per-queue element comparison fcn
} Queue;

	// Queue entry types

typedef enum qnodetypes {
	T_PCB, T_STACK
} QNodeType;

/*
** Globals
*/

extern Queue g_sleeping;
extern Queue g_ready[ N_PRIORITIES ];
extern Queue g_sio_blocked;
extern Queue g_free_pcbs;
extern Queue g_free_stacks;

/*
** Prototypes
*/

/*
** _enque( queue, data, type )
**
** add 'data' to 'queue' with type field 'type'
**
** returns:  status of insertion attempt
*/

Status _enque( Queue *queue, void *data, int type );

/*
** _deque( queue, data )
**
** remove something from 'queue' putting pointer into 'data'
**
** returns:  status of removal attempt
*/

Status _deque( Queue *queue, void **data );

/*
** _init_queues( void )
**
** initialize all queue-related variables
*/

void _init_queues( void );

/*
** cmp_wakeup( first, second )
**
** compare wakeup fields of two PCBs
**
** returns:
**	< 0	first < second
**	= 0	first = second
**	> 0	first > second
*/

int cmp_wakeup( void *first, void *second );

/*
** _get_node_type(node)
**
** returns contents of type field in node
*/

int _get_node_type( void *node );

#endif

#endif
