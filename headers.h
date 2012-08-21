/*
** SCCS ID:	@(#)headers.h	1.1	04/04/08
**
** File:	headers.h
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Standard includes, pulled in by everything else
*/

#ifndef _HEADERS_H
#define _HEADERS_H

#ifndef __ASM__20073__

	// don't do these if we're in assembly language

#include "defs.h"
#include "types.h"
#include "c_io.h"
#include "support.h"

#ifdef __KERNEL__20073__
#include "klib.h"
#else
#include "ulib.h"
#endif

#endif

#endif
