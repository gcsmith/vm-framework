/*
** SCCS ID:	@(#)bootstrap.h	1.2	03/15/05
**
** File:	bootstrap.h
**
** Author:	K. Reek
**
** Contributor:	Warren R. Carithers, Garrett C. Smith
**
** Description:	Addresses where various stuff goes in memory.
*/

#ifndef	_BOOTSTRAP_H
#define	_BOOTSTRAP_H


/*
** The target program itself
*/
#define TARGET_SEGMENT	0x00001000	/* 0001:0000 */
#define TARGET_ADDRESS	0x00010000	/* and upward */
#define	TARGET_STACK	0x00010000	/* and downward */

/*
** The Global Descriptor Table (0000:0500 - 0000:2500)
*/
#define	GDT_SEGMENT	0x00000050
#define	GDT_ADDRESS	0x00000500

#define	GDT_LINEAR	0x08		/* All of memory, R/W */
#define	GDT_CODE	0x10		/* All of memory, R/E */
#define	GDT_DATA	0x18		/* All of memory, R/W */
#define	GDT_STACK	0x20		/* All of memory, R/W */
#define GDT_CODE16  0x28        /* 16-bit pseudo code selector */
#define GDT_DATA16  0x30        /* 16-bit pseudo data selector */

/*
** The Interrupt Descriptor Table (0000:2500 - 0000:2D00)
*/
#define	IDT_SEGMENT	0x00000250
#define IDT_ADDRESS	0x00002500

/*
** Physical Memory Map Table (0000:2D00 - 0000:2D08)
*/
#define MMAP_SEGMENT  0x000002D0
#define MMAP_ADDRESS  0x00002D00
#define MMAP_EXT_LO   0x00 /* extended memory - low register */
#define MMAP_EXT_HI   0x02 /* extended memory - high register */
#define MMAP_CFG_LO   0x04 /* configured memory - low register */
#define MMAP_CFG_HI   0x06 /* configured memory - high register */
#define MMAP_PROGRAMS 0x08 /* number of programs loaded from disk (+ kernel) */
#define MMAP_SECTORS  0x0A /* table of sector counts for each program */

/*
** Real Mode Program(s) Text Area (0000:3000 - 0x7C00)
*/
#define RMTEXT_SEGMENT  0x00000300
#define RMTEXT_ADDRESS  0x00003000

#endif

