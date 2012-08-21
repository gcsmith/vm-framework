/*
** SCCS ID:	@(#)queues.c	1.1	04/04/08
**
** File:	queues.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	queue-related definitions
*/

#define	__KERNEL__20073__

#include "headers.h"

#include "queues.h"
#include "processes.h"

/*
** PRIVATE DEFINITIONS
*/

	// number of QNodes
	// rationale:  one per PCB, one per Stack, plus some extra

#define	N_QNODES	((2 * N_PROCESSES) + 8)

/*
** PRIVATE DATA TYPES
*/

// Generic queue internal node
typedef
	struct qnode {
		void *data;
		struct qnode *next;
		struct qnode *prev;
		int type;
	}
		QNode;

/*
** PRIVATE GLOBAL VARIABLES
*/

static QNode *g_free_qnodes;		// list of available queue nodes
static QNode g_qnodes[ N_QNODES ];	// static allocation of QNodes

/*
** PUBLIC GLOBAL VARIABLES
*/

Queue g_sleeping;
Queue g_ready[ N_PRIORITIES ];
Queue g_sio_blocked;
Queue g_free_pcbs;
Queue g_free_stacks;

/*
** PRIVATE FUNCTIONS
*/

/*
** _get_qnode():  allocate a queue node
*/

static QNode *_get_qnode( void ) {
	QNode *qn;

	qn = g_free_qnodes;
	if( qn != 0 ) {
		g_free_qnodes = qn->next;
		qn->next = 0;
	}

	return( qn );

}

/*
** _free_qnode():  deallocate a queue node
*/

static void _free_qnode( QNode *node ) {

	// MIGHT WANT TO MAKE THIS FANCIER

	if( node != 0 ) {	/* 20080320 */
		node->next = g_free_qnodes;
		g_free_qnodes = node;
	}

}


/*
** PUBLIC FUNCTIONS
*/


/*
** _enque( queue, data, type )
**
** add 'data' to 'queue' with type field 'type'
**
** returns:  status of insertion attempt
*/

Status _enque( Queue *queue, void *data, int type ) {
	QNode *qn;
	QNode *current;

	if( queue == 0 ) {
		return( ERR_NULLPTR );
	}

	qn = _get_qnode();
	if( qn == 0 ) {
		return( ERR_QNODE );
	}

	qn->data = data;
	qn->type = type;

	if( queue->compare == 0 ) {

		current = 0;

	} else {

		current = (QNode *) queue->front;
		while( current != 0 &&
			(*(queue->compare))( current->data, data ) <= 0 ) {
			current = current->next;
		}

	}

	if( current == 0 ) {

		// new last node in the list

		qn->prev = queue->back;
		qn->next = 0;

		if( queue->back == 0 ) {
			queue->front = (void *) qn;
		} else {
			((QNode *)(queue->back))->next = qn;
		}

		queue->back = (void *) qn;
	
	} else {

		// new first or middle node

		qn->next = current;
		qn->prev = current->prev;

		if( current->prev == 0 ) {

			// first node

			queue->front = (void *) qn;
		
		} else {

			current->prev->next = qn;
		
		}

		current->prev = qn;
	
	}

	return( ERR_NONE );

}


/*
** _deque( queue, data )
**
** remove something from 'queue' putting pointer into 'data'
**
** returns:  status of removal attempt
*/

Status _deque( Queue *queue, void **data ) {
	QNode *qn;

	if( queue == 0 || data == 0 ) {
		return( ERR_NULLPTR );
	}

	qn = (QNode *) (queue->front);

	if( qn == 0 ) {
		return( ERR_EMPTYQ );
	}

	*data = qn->data;

	queue->front = (void *) (qn->next);

	if( queue->front == 0 ) {
		queue->back = 0;
	} else {
		((QNode *)(queue->front))->prev = 0;
	}

	qn->prev = qn->next = 0;
	// might also want to set data and type fields to 0

	_free_qnode( qn );

	return( ERR_NONE );

}


/*
** _init_queues( void )
**
** initialize all queue-related variables
*/

void _init_queues( void ) {
	int i;

	// first, the queue structures themselves

	g_sleeping.front = g_sleeping.back = 0;
	g_sleeping.compare = cmp_wakeup;

	g_sio_blocked.front = g_sio_blocked.back = 0;
	g_sio_blocked.compare = 0;

	g_free_pcbs.front = g_free_pcbs.back = 0;
	g_free_pcbs.compare = 0;

	g_free_stacks.front = g_free_stacks.back = 0;
	g_free_stacks.compare = 0;

	for( i = 0; i < N_PRIORITIES; ++i ) {
		g_ready[i].front = g_ready[i].back = 0;
		g_ready[i].compare = 0;
	}

	// now, the queue nodes

	g_free_qnodes = 0;
	for( i = 0; i < N_QNODES; ++i ) {
		_free_qnode( &g_qnodes[i] );
	}

	// announce that we have initialized the queue module

	c_puts( " queues" );

}


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

int cmp_wakeup( void *first, void *second ) {

	if( first == 0 || second == 0 ) {
		_kpanic( "cmp_wakeup:  NULL argument" );
	}

	return( ((Pcb *)first)->wakeup - ((Pcb *)second)->wakeup );

}

/*
** _get_node_type(node)
**
** returns contents of type field in node
*/

int _get_node_type( void *node ) {

	return( node ? ((QNode *)node)->type : 0 );

}
