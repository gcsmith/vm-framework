/*
** SCCS ID:	@(#)ulib.h	1.1	04/04/08
**
** File:	ulib.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Prototypes for user-level library routines
**
** Note that some of these are defined in the ulibs.S source file,
** and others in the ulibc.c source file.
*/

#ifndef _ULIB_H
#define _ULIB_H

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
** read - read one character from the SIO
**
** usage:	ch = read();
**
** returns:
**	the next input character
*/

int read( void );

/*
** write - write one character to the SIO
**
** usage:	write( ch );
**
** based on a simple interrupt-driven SIO model taken from SP1
*/

void write( int ch );

/*
** exit - terminate the current process and clean up after it
**
** usage:	exit();
*/

void exit( void );

/*
** sleep - put the current process to sleep
**
** usage:	sleep( ticks );
*/

void sleep( int ticks );

/*
** gettime - retrieve the current system time
**
** usage:	n = gettime();
**
** returns:
**	- the current system time
*/

Time gettime( void );

/*
** getpid - retrieve pid of the current process
**
** usage:	n = getpid();
**
** returns:
**	- the current process' pid
*/

Pid getpid( void );

/*
** fork - create a duplicate of the current process
**
** usage:	id = fork();
**
** returns:
**	-1  on error
**	= 0 in the new process
**	> 0 in the original process (pid of the new process)
*/

int fork( void );

/*
** exec - replace the memory image of the current process
**
** usage:	exec( fcn );
**
*/

void exec( void (*fcn)(void) );

/*
** execve - replace the memory image of the current process
**
** usage:	execve( name, argv );
**
*/

void execve( const char *name, char *const argv[] );

/*
** TODO: I DEMAND A DESCRIPTION
*/

void puts(char *msg);

/*
** TODO: I DEMAND A DESCRIPTION
*/

void putc(char msg);

/*
** TODO: I DEMAND A DESCRIPTION
*/

void hotswap(void);

/*
** bogus - a bogus system call
**
** should force the calling process to exit
*/

void bogus( void );

/*
** prt_status - print a status value to the console
*/

void prt_status( char *msg, Status stat );

/*
** TODO: I DEMAND A DESCRIPTION
*/

long int strtol(const char *str, char **endptr, int base);

/*
** TODO: I DEMAND A DESCRIPTION
*/

unsigned int strlen( const char *str );

/*
** TODO: I DEMAND A DESCRIPTION
*/

char *strcpy(char *dest, const char *src);

/*
** TODO: I DEMAND A DESCRIPTION
*/

void printf( char *fmt, ... );

#endif

#endif
