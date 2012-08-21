/*
** SCCS ID:	@(#)sio.h	1.1	04/04/08
**
** File:	sio.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	SIO definitions
*/

#ifndef _SIO_H
#define _SIO_H

#ifndef __ASM__20073__

/*
** Start of C-only definitions
*/

/*
** _init_sio
**
** Initialize the UART chip.
*/

void _init_sio( void );

/*
** _sio_read - get the next input character
**
** usage:	ch = _sio_read()
**
** returns the next character, or -1 if no character is available
*/

int _sio_read( void );

/*
** _sio_write - write a character to the serial output
**
** usage:	_sio_write( ch )
*/

void _sio_write( int ch );

#endif

#endif
