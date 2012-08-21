/*
** SCCS ID:	%W%	%G%
**
** File:		Offsets.c
**
** Author:		Warren R. Carithers
**
** Description:		Print byte offsets for fields in various structures.
**
** This program exists to simplify life.  If/when fields in a structure are
** changed, this can be modified, recompiled and executed to come up with
** byte offsets for use in accessing structure fields from assembly language.
**
*/

#include <stdio.h>
#undef NULL

#include "defs.h"
#include "types.h"
#include "stacks.h"
#include "processes.h"
#include "syscalls.h"

	/*
	** We want to have access to the QNode type; however, it's
	** private to the queue module, which means we must include
	** queues.c directely.  This, in turn, causes us to compile
	** in references to some external routines.  MAJOR HACK:
	** we define those routines as null macros here.  As a result,
	** we get warning messages like these:

c_io.h:115: warning: useless keyword or type name in empty declaration
c_io.h:115: warning: empty declaration
klib.h:73: warning: useless keyword or type name in empty declaration
klib.h:73: warning: empty declaration

	*/

void c_puts( char *str )  { /* stub */ }
void _kpanic( char *msg ) { /* stub */ }

#include "queues.c"

QNode qnode;
Queue queue;
Context context;
Pcb pcb;

int main( void ) {

    printf( "Offsets into QNode (%zu bytes):\n", sizeof(qnode) );
    printf( "   data:\t%lu\n", (char *)&qnode.data - (char *)&qnode );
    printf( "   prev:\t%lu\n", (char *)&qnode.prev - (char *)&qnode );
    printf( "   next:\t%lu\n", (char *)&qnode.next - (char *)&qnode );
    printf( "   type:\t%lu\n", (char *)&qnode.type - (char *)&qnode );
    putchar( '\n' );

    printf( "Offsets into Queue (%zu bytes):\n", sizeof(queue) );
    printf( "   front:\t%lu\n", (char *)&queue.front - (char *)&queue );
    printf( "   back:\t%lu\n", (char *)&queue.back - (char *)&queue );
    printf( "   compare:\t%lu\n", (char *)&queue.compare - (char *)&queue );
    putchar( '\n' );

    printf( "Offsets into Context (%zu bytes):\n", sizeof(context) );
    printf( "   ss:\t\t%lu\n", (char *)&context.ss - (char *)&context );
    printf( "   gs:\t\t%lu\n", (char *)&context.gs - (char *)&context );
    printf( "   fs:\t\t%lu\n", (char *)&context.fs - (char *)&context );
    printf( "   es:\t\t%lu\n", (char *)&context.es - (char *)&context );
    printf( "   ds:\t\t%lu\n", (char *)&context.ds - (char *)&context );
    printf( "   edi:\t\t%lu\n", (char *)&context.edi - (char *)&context );
    printf( "   esi:\t\t%lu\n", (char *)&context.esi - (char *)&context );
    printf( "   ebp:\t\t%lu\n", (char *)&context.ebp - (char *)&context );
    printf( "   esp:\t\t%lu\n", (char *)&context.esp - (char *)&context );
    printf( "   ebx:\t\t%lu\n", (char *)&context.ebx - (char *)&context );
    printf( "   edx:\t\t%lu\n", (char *)&context.edx - (char *)&context );
    printf( "   ecx:\t\t%lu\n", (char *)&context.ecx - (char *)&context );
    printf( "   eax:\t\t%lu\n", (char *)&context.eax - (char *)&context );
    printf( "   vector:\t%lu\n",(char *)&context.vector - (char *)&context);
    printf( "   code:\t%lu\n", (char *)&context.code - (char *)&context );
    printf( "   eip:\t\t%lu\n", (char *)&context.eip - (char *)&context );
    printf( "   cs:\t\t%lu\n", (char *)&context.cs - (char *)&context );
    printf( "   eflags:\t%lu\n",(char *)&context.eflags - (char *)&context);
    putchar( '\n' );

    printf( "Offsets into Pcb (%zu bytes):\n", sizeof(pcb) );
    printf( "   context:\t%lu\n", (char *)&pcb.context - (char *)&pcb );
    printf( "   stack:\t%lu\n", (char *)&pcb.stack - (char *)&pcb );
    printf( "   wakeup:\t%lu\n", (char *)&pcb.wakeup - (char *)&pcb );
    printf( "   pid:\t\t%lu\n", (char *)&pcb.pid - (char *)&pcb );
    printf( "   ppid:\t%lu\n", (char *)&pcb.ppid - (char *)&pcb );
    printf( "   state:\t%lu\n", (char *)&pcb.state - (char *)&pcb );
    printf( "   prio:\t%lu\n", (char *)&pcb.prio - (char *)&pcb );

    return( 0 );

}
