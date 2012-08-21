/*
** SCCS ID:	@(#)clock.h	1.1	04/04/08
**
** File:	clock.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Clock handler
*/

#ifndef _CLOCK_H
#define _CLOCK_H

/*
** General (C and/or assembly) definitions
*/

#define	CLOCK_FREQUENCY		100

#ifndef __ASM__20073__

/*
** Start of C-only definitions
*/

/*
** Globals
*/

extern Time g_sys_time;

/*
** Prototypes
*/

/*
** _init_clock()
**
** initialize all clock-related variables
*/

void _init_clock( void );

#endif

#endif
