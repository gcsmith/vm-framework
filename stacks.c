/*
** SCCS ID:	@(#)stacks.c	1.1	04/04/08
**
** File:	stack.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	stack-related routines
*/

#define	__KERNEL__20073__

#include "headers.h"

#include "stacks.h"
#include "queues.h"

/*
** PRIVATE GLOBAL VARIABLES
*/

static Stack g_stacks[N_PROCESSES];	// all available user stacks

static Stack g_system_stack;		// system stack

/*
** PUBLIC GLOBAL VARIABLES
*/

unsigned int *g_system_esp;

/*
** PUBLIC FUNCTIONS
*/

/*
** _get_stack()
**
** allocate a Stack structure
**
** returns a pointer to the structure, or NULL
*/

Stack *_get_stack( void ) {
	Stack *stack;

	if( _deque(&g_free_stacks,(void **)&stack) != ERR_NONE ) {
		return( 0 );
	}

	_memclr( (void *) stack, sizeof(Stack) );

	return( stack );

}

/*
** _free_stack()
**
** deallocate a Stack structure
**
** returns the status from the _enque() call, or ERR_NULLPTR
*/

Status _free_stack( Stack *stack ) {

	if( stack != 0 ) {
		return( _enque(&g_free_stacks,(void *)stack,T_STACK) );
	}

	return( ERR_NULLPTR );

}

/*
** _init_stacks()
**
** initialize all stack-related data structures
*/

void _init_stacks( void ) {
	int i;

	// first, put all stacks into the free pool
	//
	// NOTE:  queues must have been initialized first!

	for( i = 0; i < N_PROCESSES; ++i ) {
		if( _free_stack(&g_stacks[i]) != ERR_NONE ) {
			_kpanic( "init stacks:  enqueue failed" );
		}
	}

	// set up the system stack

	_memclr( (void *)&g_system_stack, sizeof(Stack) );

	g_system_esp = ((unsigned int *)(&g_system_stack + 1)) - 2;

	// announce that we have initialized the stack module

	c_puts( " stacks" );

}
