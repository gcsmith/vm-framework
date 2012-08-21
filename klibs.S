/*
** sccs id:	@(#)klibs.S	1.1	04/04/08
**
** File:	klibs.S
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Assembly implementations of some kernel-level library routines
**
** These are library routines written in assembly language either
** for speed, or because they must do things that can't be done in C.
*/

#define	__ASM__20073__

/*
** _get_ebp - return current contents of EBP at the time of the call
**
** Primarily used only by _kpanic
*/

	.globl	_get_ebp
_get_ebp:
	movl	%ebp, %eax
	ret
