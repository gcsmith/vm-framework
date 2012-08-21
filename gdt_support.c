 /*
 ** File:   gdt_support.c
 **
 ** Author: Alex Merritt
 **
 ** Contributor:
 **
 ** Description:    GDT Descriptor manipulation functions.
 */
#include "gdt_support.h"
#include "bootstrap.h"
#include "defs.h"
#include "klib.h"	// for _kpanic
#include "stacks.h"	// for g_system_esp

//
// Pointer to physical memory where the GDT resides.
//
static gdt_entry_t *g_gdt_table = (gdt_entry_t *)GDT_ADDRESS;

//
// TSS in memory. Should be one per CPU.
//
static volatile tss_t g_tss;

//
// Index of the TSS in the GDT.
//
static int g_tss_index = 0;
void __tss_set_index( int i ) { g_tss_index = i; }
int  __tss_get_index( void )  { return g_tss_index; }

//
// __gdt_set_entry
//
void __gdt_set_entry(int index, u32_t base, u32_t limit, u08_t access) {
	__set_descriptor( &g_gdt_table[index], base, limit, access );
}

//
// __set_descriptor
//
void __set_descriptor( gdt_entry_t *descriptor, u32_t base, u32_t limit, u08_t access ) {
	descriptor->seglim_15_0 = (limit & 0xFFFF);
	descriptor->base_15_0   = (base & 0xFFFF);
	descriptor->base_23_16  = (base >> 16) & 0xFF;
	descriptor->access      = access;
	descriptor->granularity = ((limit >> 16) & 0x0F) | 0xC0;
	descriptor->base_31_24  = (base >> 24) & 0xFF;
}

//
// __gdt_read_seg
//
int __gdt_read_seg( gdt_entry_t *seg, unsigned int id ) {
	if( NULL == seg ) return -1;
	if( id > GDT_MAX ) return -1;

	*seg = g_gdt_table[ id ];

	return 0;
}

//
// __gdt_store_seg
//
int __gdt_store_seg( gdt_entry_t *seg, unsigned int id ) {
	if( NULL == seg ) return -1;
	if( id > GDT_MAX ) return -1;

	g_gdt_table[ id ] = *seg;

	return 0;
}

//
// __gdt_set_base
//
int __gdt_set_base( gdt_entry_t *seg, unsigned int baseAddr ) {
	if( NULL == seg ) return -1;

	/* Copy first two bytes over. */
	seg->base_15_0 = *((unsigned short *)&baseAddr);
	baseAddr >>= 16;

	/* Copy next byte over. */
	seg->base_23_16 = *((unsigned char *)&baseAddr);
	baseAddr >>= 8;

	/* Copy final byte over. */
	seg->base_31_24 = *((unsigned char *)&baseAddr);

	return 0;
}

//
// __gdt_get_base
//
int __gdt_get_base( gdt_entry_t *seg, unsigned int *baseAddr ) {
	if( NULL == seg ) return -1;
	if( NULL == baseAddr ) return -1;

	/* Why is this so complicated?! */
	*baseAddr = ((seg->base_31_24 << 24) & 0xff000000) |
		((seg->base_23_16 << 16) & 0x00ff0000) |
		((seg->base_15_0) & 0x0000ffff);

	return 0;
}

//
// __gdt_set_seglim
//
int __gdt_set_seglim( gdt_entry_t *seg, unsigned int seglim ) {
	if( NULL == seg ) return -1;

	/* Note that the segment limit field is a total of 20 bits,
	 * so any of the bits 31:21 are ignored from the argument. */

	/* Copy first two bytes over. */
	seg->seglim_15_0 = *((unsigned short *)&seglim);
	seglim >>= 16;

	/* Since we have 4 bits remaining to copy, we must combine
	 * them with the adjacent flags before storing. */

	seglim &= 0x0f; /* retain only first four bits */
	seg->granularity &= 0xf0; /* null the four seglim bits */
	seg->granularity |= seglim; /* combine and store */

	return 0;
}

//
// __gdt_get_seglim
//
int __gdt_get_seglim( gdt_entry_t *seg, unsigned int *seglim ) {
	if( NULL == seg ) return -1;
	if( NULL == seglim ) return -1;

	/* Must this also be so complicated?! */
	*seglim = ((seg->granularity & 0x00ff) << 16) |
		(seg->seglim_15_0 & 0x0000ffff);

	return 0;
}


//
// __gdt_set_type
//
int __gdt_set_type( gdt_entry_t *seg, unsigned char type ) {
	if( NULL == seg ) return -1;
	/* Note that the type field is only 4 bits, so bits 7:4 are ignored. */

	type &= 0x0f; /* remove unneeded bits */
	seg->access &= 0xf0; /* clear type in descriptor */
	seg->access |= type; /* combine and store */

	return 0;
}

//
// __gdt_get_type
//
int __gdt_get_type( gdt_entry_t *seg, unsigned char *type ) {
	if( NULL == seg ) return -1;
	if( NULL == type ) return -1;

	*type = seg->access & 0x0f;

	return 0;
}

// 
// __gdt_set_flagS
//
int __gdt_set_flagS( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->access |= 0x10;
	return 0;
}

//
// __gdt_clear_flagS
//
int __gdt_clear_flagS( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->access &= ~0x10;
	return 0;
}

// 
// __gdt_set_flagDPL
//
int __gdt_set_flagDPL( gdt_entry_t *seg, unsigned char val ) {
	if( NULL == seg ) return -1;
	val &= 0x03; /* keep only first two bits */
	val <<= 5; /* put the two bits into their position */
	seg->access &= ~0x60; /* clear DPL in the descriptor */
	seg->access |= val; /* store them */
	return 0;
}

// 
// __gdt_set_flagP
//
int __gdt_set_flagP( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->access |= 0x80;
	return 0;
}

// 
// __gdt_clear_flagP
//
int __gdt_clear_flagP( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->access &= ~0x80;
	return 0;
}

// 
// __gdt_set_flagAVL
//
int __gdt_set_flagAVL( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->granularity |= 0x10;
	return 0;
}

//
// __gdt_clear_flagAVL
//
int __gdt_clear_flagAVL( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->granularity &= ~0x10;
	return 0;
}

//
// __gdt_set_flagL
//
int __gdt_set_flagL( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->granularity |= 0x20;
	return 0;
}

// 
// __gdt_clear_flagL
//
int __gdt_clear_flagL( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->granularity &= ~0x20;
	return 0;
}

// 
// __gdt_set_flagDB
//
int __gdt_set_flagDB( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->granularity |= 0x40;
	return 0;
}

//
// __gdt_clear_flagDB
//
int __gdt_clear_flagDB( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->granularity &= ~0x40;
	return 0;
}

// 
// __gdt_set_flagG
//
int __gdt_set_flagG( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->granularity |= 0x80;
	return 0;
}

// 
// __gdt_clear_flagG
//
int __gdt_clear_flagG( gdt_entry_t *seg ) {
	if( NULL == seg ) return -1;
	seg->granularity &= ~0x80;
	return 0;
}

//
// __gdt_set_tss_entry
//
void __gdt_set_tss( int index, u32_t base, u32_t limit, u08_t access ) {
	g_gdt_table[ index ].seglim_15_0 = (limit & 0xFFFF);
	g_gdt_table[ index ].base_15_0   = (base & 0xFFFF);
	g_gdt_table[ index ].base_23_16  = (base >> 16) & 0xFF;
	g_gdt_table[ index ].access      = access;
	g_gdt_table[ index ].granularity = ((limit >> 16) & 0x0F) | 0x80;
	g_gdt_table[ index ].base_31_24  = (base >> 24) & 0xFF;
}

//
// __tss_load
//
void __tss_load( void ) {

	// check allocation of descriptor; done in _init_memory()
	if( 0 >= g_tss_index ) {
		_kpanic( "__tss_load: tss descriptor not reserved " );
	}

	// clear and set tss
	_memclr( (void *)&g_tss, sizeof( tss_t ) );
	g_tss.ss0 = GDT_STACK;
	g_tss.esp0 = (unsigned int)g_system_esp;

	// TODO something to think about: does g_tss.esp0 need to be updated
	// on a continuous basis? We're not really going to use a TSS unless we
	// implement priviledge levels.

	// set descriptor fields
	__gdt_set_tss( g_tss_index, (u32_t)&g_tss, sizeof(tss_t), ACC_PRES | SYS_32TSS_AVAIL );

	// task register. Intel manual 3a 3.4.2
	asm volatile( "ltr %%ax": : "a" (SEL_SETINDEX(g_tss_index) | SEL_GDT) );
}

//
// __ldtr_load
//
void __ldtr_load( u16_t ldtr ) {
	asm volatile( "lldt %%ax": : "a" (ldtr & ~(SEL_LDT)) );
}

//
// __ldtr_store
//
u16_t __ldtr_store( void ) {
	u16_t retVal = 0;
	asm volatile( "sldt %%ax" : "=r" (retVal) );
	return retVal;
}

//
// __copy_gdt_entry
//
void __copy_gdt_entry( gdt_entry_t *dst, unsigned int gdt_index ) {

	if( NULL == dst ) return;

	/* copy 8 bytes */
	const gdt_entry_t * const src = &(g_gdt_table[gdt_index]);
	_memcpy( (void *)dst, (void *)src, sizeof(gdt_entry_t) );

}
