/*
** SCCS ID:	@(#)types.h	1.1	04/04/08
**
** File:	types.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Common type definitions
*/

#ifndef _TYPES_H
#define _TYPES_H

#ifndef __ASM__20073__

/*
** Start of C-only definitions
*/

/*
** Data types
*/

	// Status return values

typedef enum {
	ERR_NONE = 0, ERR_QNODE, ERR_PCB, ERR_STACK, ERR_NULLPTR,
	ERR_EMPTYQ
} Status;

	// system time

typedef unsigned int Time;

typedef unsigned char  u08_t;
typedef unsigned short u16_t;
typedef unsigned int   u32_t;

typedef unsigned char  uchar_t;
typedef unsigned short ushort_t;
typedef unsigned int   uint_t;
typedef unsigned long  ulong_t;

typedef char  int8_t;
typedef short int16_t;
typedef int   int32_t;

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

#endif

#endif
