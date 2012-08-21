/*
** SCCS ID:	@(#)syscalls.h	1.1	04/04/08
**
** File:	syscalls.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	System call handlers
*/

#ifndef _SYSCALLS_H
#define _SYSCALLS_H

/*
** General (C and/or assembly) definitions
*/

#include <x86arch.h>

	// syscall codes

#define	SYS_exit		0
#define	SYS_read		1
#define	SYS_write		2
#define	SYS_sleep		3
#define	SYS_fork		4
#define	SYS_exec		5
#define	SYS_gettime		6
#define	SYS_getpid		7
#define SYS_puts        8
#define SYS_putc        9
#define SYS_execve      10
#define SYS_hotswap     11

	// number of "real" system calls

#define N_SYSCALLS		12

	// dummy syscall code to test the syscall ISR

#define	SYS_bogus		0xfeedface

	// system call interrupt vector

#define	INT_VEC_SYSCALL		0x80

	// default contents of EFLAGS register

#define	DEFAULT_EFLAGS		(EFLAGS_MB1 | EFLAGS_IF)

#ifndef __ASM__20073__

/*
** Start of C-only definitions
*/

/*
** Prototypes
*/

void _init_syscalls( void );

#endif

#endif
