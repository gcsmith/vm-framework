/*
** SCCS ID:	@(#)user.h	1.1	04/04/08
**
** File:	user.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	User routines.
*/

#ifndef _USER_H
#define _USER_H

/*
** General (C and/or assembly) definitions
*/

/*
** Delay loop counts
*/

#define	LONG_DELAY	100000000
#define	STD_DELAY	  2500000
#define	ALT_DELAY	  4500000

#ifndef __ASM__20073__

/*
** Start of C-only definitions
*/

/*
** User process controls.
**
** The value field of these definitions contains a list of the
** system calls this user process uses.
**
** To spawn a specific user process from the initial process,
** uncomment its entry in this list.
**
** NOTE:  users W-Z should never be spawned directly.
*/

#define	SPAWN_A		write, exit
#define	SPAWN_B		write, exit
#define	SPAWN_C		write, exit
#define	SPAWN_D		write, fork, exec, exit
#define	SPAWN_E		write, sleep, exit
#define	SPAWN_F		write, sleep, exit
#define	SPAWN_G		write, sleep, exit
#define	SPAWN_H		write
// no user i
#define	SPAWN_J		write, fork, exec, exit
#define	SPAWN_K		write, sleep, fork, exec, exit
#define	SPAWN_L		write, sleep, fork, exec, gettime, exit
#define	SPAWN_M		write, fork, exec, sleep, exit
#define	SPAWN_N		write, sleep, fork, exec, gettime, getpid, exit
// no user O
#define	SPAWN_P		write, sleep, gettime
#define	SPAWN_Q		write, bogus, exit
#define	SPAWN_R		write, read, sleep, exit
#define	SPAWN_S		write, sleep, forever
// no user T
// no user U
// no user V

// user W:  write, sleep, getpid, exit
// user X:  write, getpid, exit
// user Y:  write, sleep, exit
// user Z:  write, exit

/* special user functions for testing and debugging purposes */
#define SPAWN_EXT_USER1
#define SPAWN_EXT_USER2
#define SPAWN_SWAP

/*
** Prototypes for externally-visible routines
*/

void idle_main( void );

void first_main( void );

#endif

#endif
