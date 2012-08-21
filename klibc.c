/*
** SCCS ID:	@(#)klibc.c	1.1	04/04/08
**
** File:	klibc.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	C implementations of some kernel-level library routines
*/

#define	__KERNEL__20073__

#include "headers.h"

/*
** _memclr - initialize all bytes of a block of memory to zero
**
** usage:  _memclr( buffer, length )
*/

void _memclr( void *buffer, unsigned int length ) {
	register unsigned char *buf = (unsigned char *)buffer;
	register unsigned int num = length;

	while( num-- ) {
		*buf++ = 0;
	}

}

/*
** _memcpy - copy a block from one place to another
**
** usage:  _memcpy( dest, src, length )
**
** may not correctly deal with overlapping buffers
*/

void _memcpy( void *destination, void *source, unsigned int length ) {
	register unsigned char *dst = (unsigned char *)destination;
	register unsigned char *src = (unsigned char *)source;
	register unsigned int num = length;

	while( num-- ) {
		*dst++ = *src++;
	}

}


/*
** _kpanic(msg)
**
** kernel-level panic routine
**
** usage:  _kpanic( msg )
**
** Prints an EBP chain (up to five values), then calls __panic.
*/

void _kpanic( char *msg ) {
	unsigned int ebp;
	int n;

	c_puts( "\n\n***** KERNEL PANIC *****\n\nChain of EBP values:\n\n" );

	//
	// Get current EBP so we can follow the chain
	//

	ebp = _get_ebp();

	c_printf( "EBP  %08x\n", ebp );

	//
	// Follow the chain until either EBP becomes 0, or
	// we have done five levels.
	//

	for( n = 0; ebp != 0 && n < 5; ++n ) {
		ebp = *(unsigned int *)ebp;
		c_printf( "-->  %08x\n", ebp );
	}

	//
	// Finally, call __panic to wrap things up
	//

	__panic( msg );

}


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

void _kprt_status( char *msg, Status stat ) {

	if( msg == NULL ) {
		return;
	}

	switch( stat ) {
		case ERR_NONE:
			c_printf( msg, "ERR_NONE" );
			break;

		case ERR_QNODE:
			c_printf( msg, "ERR_QNODE" );
			break;

		case ERR_PCB:
			c_printf( msg, "ERR_PCB" );
			break;

		case ERR_STACK:
			c_printf( msg, "ERR_STACK" );
			break;

		case ERR_NULLPTR:
			c_printf( msg, "ERR_NULLPTR" );
			break;

		case ERR_EMPTYQ:
			c_printf( msg, "ERR_EMPTYQ" );
			break;

		default:
			c_printf( msg, "ERR_????" );
			break;
	
	}

}
