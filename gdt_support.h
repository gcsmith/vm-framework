 /*
 ** File:   gdt_support.h
 **
 ** Author: Alex Merritt
 **
 ** Contributor: Garrett Smith
 **
 ** Description:    GDT Descriptor manipulation functions.
 */

#ifndef _GDT_SUPPORT_H
#define _GDT_SUPPORT_H

#include "types.h"

/*
 * Code/Data Segment Type field values. Valid when the S flag in the descriptor is set.
 * Section 3.4.5.1 in Intel Manual 3a
 */

#define ACC_D_RO    0x00        /* data - read only */
#define ACC_D_ROA   0x01        /* data - read only accessed */
#define ACC_D_RW    0x02        /* data - read write */
#define ACC_D_RWA   0x03        /* data - read write accessed */
#define ACC_D_ROX   0x04        /* data - read only expand down */
#define ACC_D_ROXA  0x05        /* data - read only expand down accessed */
#define ACC_D_RWX   0x06        /* data - read write expand down */
#define ACC_D_RWXA  0x07        /* data - read write expand down accessed */

#define ACC_C_EO    0x08        /* code - exec only */
#define ACC_C_EOA   0x09        /* code - exec only accessed */
#define ACC_C_ER    0x0A        /* code - exec read */
#define ACC_C_ERA   0x0B        /* code - exec read accessed */
#define ACC_C_EOC   0x0C        /* code - exec only conforming */
#define ACC_C_EOCA  0x0D        /* code - exec only conforming accessed */
#define ACC_C_ERC   0x0E        /* code - exec read conforming */
#define ACC_C_ERCA  0x0F        /* code - exec read conforming accessed */

/*
 * System Segment Type field values. Valid when the S flag in the descriptor is NOT set.
 * Section 3.5 in Intel Manual 3a. 32-bit Mode (not IA-32e Mode).
 */
#define SYS_LDT		0x02	/* sys seg - points to an LDT */
#define SYS_32TSS_AVAIL	0x09	/* sys seg - 32-bit available TSS */
#define SYS_32TSS_BUSY	0x0B	/* sys seg - 32-bit busy TSS */

/*
 * Misc field values.
 */
#define ACC_NONSYS  0x10        /* non-system use (data or code segment) */
#define ACC_SYS     0x00        /* system use (TSS, LDT descriptors, etc) */
#define ACC_DPL(x)  (x << 5)    /* default privledge level (0-3) */
#define ACC_PRES    0x80        /* segment is present */

#define GRAN_AVAIL  (1 << 4)    /* available for use by system software */
#define GRAN_OP16   0           /* 16-bit operation size */
#define GRAN_OP32   (1 << 6)    /* 32-bit operation size */
#define GRAN_BYTE   0           /* byte granularity (1 byte to 1 MB) */
#define GRAN_PAGE   (1 << 7)    /* page granularity (4 KB to 4 GB) */

#define GDT_OFFSET(x)   ((x) << 3) /* index of descriptor -> offset of descriptor */
#define GDT_INDEX(x)    ((x) >> 3) /* offset of descriptor -> index of descriptor */
#define GDT_MAX         1024

/*
 * Segment selector defines. Used for formatting the fields in a segment selector.
 */
#define	SEL_GDT	0x00				// table indicator bit
#define	SEL_LDT	0x04				// table indicator bit
#define	SEL_RPL(x)	((x) & 0x3)		// request privilege level bits (2)
#define	SEL_SETINDEX(x)	((x) << 3)		// place index into selector -- 13 bits
#define	SEL_GETINDEX(x)	((x) >> 3)		// extract selector index


/*
** Name:	gdt_entry_t
** 
** Description: Format of a GDT entry descriptor. Size must be 8 bytes.
*/
struct gdt_entry_s {
	short	seglim_15_0;
	short	base_15_0;
	char	base_23_16;
	char	access;
	char	granularity;
	char	base_31_24;
} __attribute__((packed));

typedef struct gdt_entry_s gdt_entry_t;

/*
 * Name:	tss_t
 *
 * Description:	Format of a task-state segment. 104 bytes.
 */
struct tss_s {

	unsigned short	link;
	unsigned short	link_r;

	unsigned int	esp0;

	unsigned short	ss0;
	unsigned short	ss0_r;

	unsigned int	esp1;

	unsigned short	ss1;
	unsigned short	ss1_r;

	unsigned int	esp2;

	unsigned short	ss2;
	unsigned short	ss2_r;

	unsigned int	cr3;
	unsigned int	eip;
	unsigned int	eflags;
	unsigned int	eax;
	unsigned int	ecx;
	unsigned int	edx;
	unsigned int	ebx;
	unsigned int	esp;
	unsigned int	ebp;
	unsigned int	esi;
	unsigned int	edi;

	unsigned short	es;
	unsigned short	es_r;

	unsigned short	cs;
	unsigned short	cs_r;

	unsigned short	ss;
	unsigned short	ss_r;

	unsigned short	ds;
	unsigned short	ds_r;

	unsigned short	fs;
	unsigned short	fs_r;

	unsigned short	gs;
	unsigned short	gs_r;

	unsigned short	ldt_ss;
	unsigned short	ldt_ss_r;

	unsigned short	trap;
	unsigned short	iomap;

} __attribute__ ((packed));

typedef volatile struct tss_s tss_t;

//
// LDT stuff
//

#define	LDT_DSEG	0x4	/* segment selector for data, 0100B */
#define	LDT_CSEG	0xC	/* segment selector for code, 1100B */

/*
 * LDT with two entries.
 */
struct ldt_s {

	gdt_entry_t dseg;	/* data segment descriptor */
	gdt_entry_t cseg;	/* code segment descriptor */

} __attribute__ ((packed));

typedef struct ldt_s ldt_t;

/*
 * TODO: I DEMAND A DESCRIPTION.
 */
void __gdt_set_entry(int index, u32_t base, u32_t limit, u08_t access);

/*
 * Name:	__set_descriptor
 *
 * Description:	Initialize an LDT/GDT descriptor.
 */
void __set_descriptor( gdt_entry_t *descriptor, u32_t base, u32_t limit, u08_t access );

/*
 * Name:	__gdt_read_seg, __gdt_store_seg
 *
 * Description:	Read off the specified GDT Segment Descriptor into
 * 		the provided argument, or store the argument into the
 * 		specified location in the GDT.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_read_seg( gdt_entry_t *seg, unsigned int id );
int __gdt_store_seg( gdt_entry_t *seg, unsigned int id );

/*
 * Name:	__gdt_set_base
 *
 * Description:	Splits up the base address provided and put the pieces
 * 		into the correct places in the provided segment descriptor.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_set_base( gdt_entry_t *seg, unsigned int baseAddr );

/*
 * Name:	__gdt_get_base
 *
 * Description:	Conglomerates all three chunks of the base into one value.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_get_base( gdt_entry_t *seg, unsigned int *baseAddr );

/*
 * Name:	__gdt_set_seglim
 *
 * Description:	Splits up the provided segment limit and puts the pieces
 * 		into the correct places in the provided segment descriptor.
 * 		Only the most significant 20 bits are considered in the new
 * 		value, as the segment limit fields total only 20 bits.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_set_seglim( gdt_entry_t *seg, unsigned int seglim );

/*
 * Name:	__gdt_get_seglim
 *
 * Description:	Conglomerates the two segment limit chunks into one.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_get_seglim( gdt_entry_t *seg, unsigned int *seglim );


/*
 * Name:	__gdt_set_type
 *
 * Description:	Sets the 'type' field in the provided segment descriptor
 * 		with the new value. Only most significant 4 bits in the new
 * 		value are considered, as the 'type' field is of that size.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_set_type( gdt_entry_t *seg, unsigned char type );

/*
 * Name:	__gdt_get_type
 *
 * Description:	Returns the 'type' field in the provided segment descriptor.
 * 		Note that only the most significant 4 bits are valid.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_get_type( gdt_entry_t *seg, unsigned char *type );

/*
 * Name:	__gdt_set_flagS, __gdt_clear_flagS, __gdt_get_flagS
 *
 * Description:	Set, clear or get flag "S" (descriptor type flag) for the given
 * 		segment descriptor. "Getting" the value of this field
 * 		will simply return you a 1 or 0.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_set_flagS( gdt_entry_t *seg );
int __gdt_clear_flagS( gdt_entry_t *seg );

/*
 * Name:	__gdt_set_flagDPL, __gdt_clear_flagDPL
 *
 * Description:	Set the new value for the two DPL (descriptor privilege level)
 * 		for the given segment descriptor.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_set_flagDPL( gdt_entry_t *seg, unsigned char val );

/*
 * Name:	__gdt_set_flagP, __gdt_clear_flagP
 *
 * Description:	Set or clear flag "P" (segment present) for the given
 * 		segment descriptor.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_set_flagP( gdt_entry_t *seg );
int __gdt_clear_flagP( gdt_entry_t *seg );

/*
 * Name:	__gdt_set_flagAVL, __gdt_clear_flagAVL
 *
 * Description:	Set or clear flag "AVL" (available for use by system software)
 * 		for the given segment descriptor.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_set_flagAVL( gdt_entry_t *seg );
int __gdt_clear_flagAVL( gdt_entry_t *seg );

/*
 * Name:	__gdt_set_flagL, __gdt_clear_flagL
 *
 * Description:	Set or clear flag "L" (64-bit code segment) for the given
 * 		segment descriptor.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_set_flagL( gdt_entry_t *seg );
int __gdt_clear_flagL( gdt_entry_t *seg );

/*
 * Name:	__gdt_set_flagDB, __gdt_clear_flagDB
 *
 * Description:	Set or clear flag "DB" (default operation size/default stack
 * 		pointer size and/or upper bound) for the given segment
 * 		descriptor.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_set_flagDB( gdt_entry_t *seg );
int __gdt_clear_flagDB( gdt_entry_t *seg );

/*
 * Name:	__gdt_set_flagG, __gdt_clear_flagG
 *
 * Description:	Set or clear flag "G" (granularity) for the given
 * 		segment descriptor.
 *
 * Return:	0 if okay, -1 on error
 */
int __gdt_set_flagG( gdt_entry_t *seg );
int __gdt_clear_flagG( gdt_entry_t *seg );

/*
 * Name:	__gdt_set_tss_entry
 *
 * Description:	TODO
 */
void __gdt_set_tss( int index, u32_t base, u32_t limit, u08_t access );

/*
 * Name:	__tss_[gs]et_index
 *
 * Description:	Get or set the index into the GDT of the TSS descriptor.
 */
void __tss_set_index( int i );
int  __tss_get_index( void );

/*
 * Name:	__tss_load
 *
 * Description:	Initializes the TSS, TSS descriptor, and task register.
 */
void __tss_load( void );

/*
 * Name:	__ldtr_load
 *
 * Description:	Loads the LDT register.
 */
void __ldtr_load( u16_t index );
u16_t __ldtr_store( void );

/*
 * Name:	__copy_gdt_entry
 *
 * Description:	Copies a descriptor from the GDT.
 */
 void __copy_gdt_entry( gdt_entry_t *dst, unsigned int gdt_index );


#endif /* _GDT_SUPPORT_H */

