/*
** SCCS ID:	@(#)system.h	1.1	04/04/08
**
** File:	system.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Miscellaneous system routines
*/

#ifndef _SYSTEM_H
#define _SYSTEM_H

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
** _setup_context(stack,entry)
**
** initialize a process context
**
** ASSUMES that the stack argument points to a stack which can be
** cleared and initialized
*/

Context *_setup_stack( Stack *stack, unsigned int entry );

/*
** _init:	system initialization routine
**
** Called by startup code immediately before returning into the
** first user-level process.  The name is taken from there.
*/

void _init( void );

#endif

#endif
