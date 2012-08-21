/*
** sccs id:	@(#)ulibs.S	1.1	04/04/08
**
** File:	ulibs.S
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Assembly implementations of some user-level library routines
**
** These are library routines written in assembly language either
** for speed, or because they must do things that can't be done in C.
*/

/*
** Only want the "general" parts of the syscalls.h file
*/

#define	__ASM__20073__

#include "syscalls.h"

/*
** System call stubs
**
** All have the same structure:
**
**	move a code into EAX
**	generate the interrupt
**	return to the caller
**
** As these are simple "leaf" routines, we don't use
** the standard enter/leave method to set up a stack
** frame - that takes time, and we don't really need it.
*/

#define	SYSCALL(name)			\
	.globl	name			; \
name:	movl	$SYS_##name, %eax	; \
	int	$INT_VEC_SYSCALL	; \
	ret

SYSCALL(exit)
SYSCALL(read)
SYSCALL(write)
// 	.globl	write
// 	.globl	c_printf
// write:	movl	$SYS_write, %eax
// 	int	$INT_VEC_SYSCALL
// 	pushl	$fmt
// 	call	c_printf
// 	addl	$4, %esp
// 	ret
// fmt:	.string	"write() is returning to %08x after printing %08x\n"
SYSCALL(sleep)
SYSCALL(fork)
SYSCALL(exec)
SYSCALL(gettime)
SYSCALL(getpid)
SYSCALL(puts)
SYSCALL(putc)
SYSCALL(execve)
SYSCALL(hotswap)

/*
** This is a bogus system call; it's here so that we can test
** our handling of out-of-range syscall codes in the syscall ISR.
*/

SYSCALL(bogus)
