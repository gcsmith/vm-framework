/*
** File:    hswap.S
**
** Author:  Garrett Smith
**
** Contributor:
**
** Description: Switch to real mode and hot swap the kernel.
**
*/

#include <x86arch.h>
#include "bootstrap.h"

.macro M_OUTB port=0, value=0
    movw    $\port, %dx
    movb    $\value, %al
    outb    %al, %dx
.endm

    .text
    .globl _start

_start: .code32
    /* disable interrupts and jump to 16-bit code segment */
    cli
    ljmp    $GDT_CODE16, $hotswap16

hotswap16: .code16
    /* set all data selectors to a 16-bit data segment */
    movw    $GDT_DATA16, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss

    /* switch back to real mode and far jump to flush the prefetch queue */
    movl    %cr0, %eax
    andb    $0xFE, %al
    movl    %eax, %cr0
    ljmp    $0, $realmode

realmode:
    /* restore real mode segment registers */
    xorw    %ax, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    movl    $0x8000, %esp

    /* load LDT and reconfigure the APIC for real mode */
    lidt    idt_rm
    sti

    M_OUTB  PIC_MASTER_CMD_PORT, PIC_ICW1BASE | PIC_NEEDICW4
    M_OUTB  PIC_SLAVE_CMD_PORT,  PIC_ICW1BASE | PIC_NEEDICW4

    M_OUTB  PIC_MASTER_IMR_PORT, 0x08
    M_OUTB  PIC_SLAVE_IMR_PORT,  0x70

    M_OUTB  PIC_MASTER_IMR_PORT, PIC_MASTER_SLAVE_LINE
    M_OUTB  PIC_SLAVE_IMR_PORT,  PIC_SLAVE_ID
    
    M_OUTB  PIC_MASTER_IMR_PORT, 0x00
    M_OUTB  PIC_SLAVE_IMR_PORT,  0x00

    M_OUTB  PIC_MASTER_IMR_PORT, 0x00
    M_OUTB  PIC_SLAVE_IMR_PORT,  0x00

    /* reset video mode (80x25) and display a nice, reassuring message */
    mov     $0x0003, %ax
    int     $0x10

    movw    0x7DFC, %ax
    movb    %al, drive

    movw    $banner, %si
    call    dispMsg

show_prompt:
    movb    drive, %al      /* display the default selection in string */
    shrb    $0x04, %al
    addb    $'0', %al
    movb    %al, prompt_digits
    movb    drive, %al
    andb    $0x0F, %al
    addb    $'0', %al
    movb    %al, prompt_digits + 1

    movw    $prompt, %si
    call    dispMsg

    xorw    %ax, %ax
    int     $0x16

    cmpb    $'0', %al
    jl      show_prompt
    cmpb    $'3', %al
    jg      show_prompt

    xorb    %ah, %ah
    subw    $'0', %ax
    movw    $boot_idx, %bx
    addw    %ax, %bx
    movb    (%bx), %al
    movb    %al, drive

disk_reset:                 /* reset and enable the disk drive */
    movb    $0x00, %ah
    xorb    %dh, %dh
    movb    drive, %dl
    int     $0x13

    movw    $0x03, %cx      /* timeout after 3 failed attempts */
    pushw   %cx

disk_read:                  /* read the first sector to boot entry address */
    movw    $0x7C0, %ax
    movw    %ax, %es        /* ES should already be cleared, but whatever */
    movw    $0x0000, %bx    /* destination offset 07C0:0000 */
    movw    $0x0001, %cx    /* track 0 sector 1 */
    xorb    %dh, %dh
    movb    drive, %dl      /* head 0 drive 0x80 (HDD) */
    movw    $0x0201, %ax    /* read 1 sector from drive */
    int     $0x13
    jnc     disk_done

disk_fail:                  /* read failed, retry if we haven't timed out */
    popw    %cx
    decw    %cx
    pushw   %cx
    cmpw    $0, %cx
    jne     disk_read
    jmp     show_prompt

disk_done:
    #movw    $msg_realmode2, %si
    #call    dispMsg

    ljmp     $0, $0x7c00

idt_rm:
    .word   0x3FF           /* 0000:03FF is IVT end address */
    .long   0x0             /* 0000:0000 is IVT start address */

drive:
    .byte   0x80            /* store the drive index for INT13 */
boot_idx:
    .byte   0x00
    .byte   0x01
    .byte   0x80
    .byte   0x81

banner:
    .ascii  "[Real Mode Boot Menu]\r\n\r\n"
    .ascii  "0 - [0x00] Floppy Drive A\r\n"
    .ascii  "1 - [0x01] Floppy Drive B\r\n"
    .ascii  "2 - [0x80] Hard Disk 0\r\n"
    .asciz  "3 - [0x81] Hard Disk 1\r\n\r\n"
prompt:
    .ascii  "Select boot device [0x"
prompt_digits:
    .asciz  "??]:\r\n"

dispMsg:	                /* stolen verbatim from bootsector */
	pushw	%ax             /* because that's how I roll */
	pushw	%bx
repeat:
	lodsb			        /* grab next character */
	movb	$0x0e, %ah	    /* write and advance cursor */
	movw	$0x07, %bx	    /* page 0, white on blank, no blink */
	orb	    %al, %al	    /* AL is character to write */
	jz	    getOut		    /* if we've reached the NUL, get out */
	int	    $0x10		    /* otherwise, print and repeat */
	jmp	    repeat	
getOut:				        /* we're done, so return */
	popw	%bx
	popw	%ax
	ret

    .end

