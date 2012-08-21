/*
** SCCS ID:	@(#)stacks.h	1.1	04/04/08
**
** File:	stacks.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	stack-related types and functions
*/

#ifndef _STACK_H
#define _STACK_H

/*
** General (C and/or assembly) definitions
*/

	// size of stack in longwords

#define	STACK_SIZE	1024

#ifndef __ASM__20073__

#include "types.h"

/*
** Start of C-only definitions
*/

/*
** Types
*/

typedef unsigned int Stack[ STACK_SIZE ];	// per-process stack

/*
** Globals
*/

extern unsigned int *g_system_esp;

/*
** Prototypes
*/

/*
** _get_stack()
**
** allocate a Stack structure
**
** returns a pointer to the structure, or NULL
*/

Stack *_get_stack( void );

/*
** _free_stack()
**
** deallocate a Stack structure
**
** returns the status from the _enque() call, or ERR_NULLPTR
*/

Status _free_stack( Stack *stack );

/*
** _init_stacks()
**
** initialize all stack-related data structures
*/

void _init_stacks( void );

#endif

#endif
