/*
** File:    ustart.S
**
** Author:  Garrett Smith
**
** Contributor:
**
** Description: User program startup code.
**
** This file contains the entry point and initialization of user programs
** that are linked separately from the kernel. Execution begins at the address
** of _start, which simply clears the BSS then calls main.
**
** This startup routine is very similar to startup.S. However, it is not
** possible to simply re-use that code as it performs certain tasks exclusive
** to the kernel.
*/

    .globl _start
    .globl __bss_start
    .globl _end

    .text
_start:

    /* clear the BSS section */
    movl $_end, %ecx
    subl $__bss_start, %ecx
    movl $__bss_start, %edi
    movl $0x0, %eax
    rep stosb

    /* jump the program entry point, clean up afterward */
    call main
    call exit
    
    .end

