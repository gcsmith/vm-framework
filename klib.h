/*
** SCCS ID:	@(#)klib.h	1.1	04/04/08
**
** File:	klib.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Prototypes for kernel-level library routines
**
** Note that some of these are defined in the klibs.S source file,
** and others in the klibc.c source file.
*/

#ifndef _KLIB_H
#define _KLIB_H

#ifndef __ASM__20073__

#include "types.h"

/*
** Start of C-only definitions
*/

/*
** Definitions
*/

	/* limit EBP chain following to addressed under this value */

#define	EBP_ADDR_LIMIT		0x00100000

	/* how many EPB entries to follow in _kpanic */

#define	EBP_CHAIN_LIMIT		5

/*
** Prototypes
*/

/*
** _get_ebp - return current contents of EBP at the time of the call
**
** Primarily used only by _kpanic
*/

unsigned int _get_ebp( void );

/*
** _memclr - initialize all bytes of a block of memory to zero
**
** usage:  _memclr( buffer, length )
*/

void _memclr( void *buffer, unsigned int length );

/*
** _memcpy - copy a block from one place to another
**
** usage:  _memcpy( dest, src, length )
**
** may not correctly deal with overlapping buffers
*/

void _memcpy( void *destination, void *source, unsigned int length );

/*
** _kpanic - kernel-level panic routine
*/

void _kpanic( char *msg );

/*
** _kprt_status - print a status value to the console
**
** the 'msg' argument should contain a %s where
** the desired status value should be printed
**
** virtually the same as the user-level prt_status(),
** but put here so that the OS doesn't use any of the
** user-level libraries (to simplify VM conversion,
** if anyone wants to do that)
*/

void _kprt_status( char *msg, Status stat );

#endif

#endif
