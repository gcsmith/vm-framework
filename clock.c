/*
** SCCS ID:	@(#)clock.c	1.1	04/04/08
**
** File:	clock.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	Clock handler
*/

#define	__KERNEL__20073__

#include "headers.h"

#include "clock.h"
#include "processes.h"
#include "scheduler.h"
#include "queues.h"

#include "startup.h"
#include <x86arch.h>

/*
** PRIVATE GLOBAL VARIABLES
*/

	// pinwheel control variables
static unsigned int g_pinwheel;
static unsigned int g_pindex;

/*
** PUBLIC GLOBAL VARIABLES
*/

Time g_sys_time;

/*
** PRIVATE FUNCTIONS
*/

/*
** clock ISR
*/

static void _isr_clock( int vector, int code ) {
	Pcb *pcb;
	Status status;

	// bump the system time

	++g_sys_time;

	// spin the pinwheel

	++g_pinwheel;
	if( g_pinwheel == (CLOCK_FREQUENCY / 10) ) {
		g_pinwheel = 0;
		++g_pindex;
		c_putchar_at( 79, 0, "|/-\\"[ g_pindex & 3 ] );
	}

	// wake up any sleepers whose time has come

	while( g_sleeping.front != 0 &&
		_get_node_type(g_sleeping.front) <= g_sys_time ) {
		status = _deque( &g_sleeping, (void **)&pcb );
		if( status != ERR_NONE ) {
			_kprt_status( "clock isr: sleep deque status %s\n",
					status );
			_kpanic( "clock isr: sleep deque failed" );
		}
		status = _schedule( pcb );
		if( status != ERR_NONE ) {
			_kprt_status( "clock isr: sleeper schedule status %s\n",
					status );
			_kpanic( "clock isr: sleeper schedule failed" );
		}
	}

	// decrement remaining quantum for current process

	--g_quantum;

	// reschedule it if it has run out of time

	if( g_quantum < 1 ) {
		if( _schedule(g_current) != ERR_NONE ) {
			_kpanic( "clock isr: current schedule failed" );
		}
		_dispatch();
	}

	// tell the PIC that we're done

	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );

}


/*
** PUBLIC FUNCTIONS
*/

/*
** _init_clock()
**
** initialize all clock-related variables
*/

void _init_clock( void ) {

	// start the pinwheel

	g_pinwheel = g_pindex = 0;

	// reset system time to the epoch

	g_sys_time = 0;

	// set the clock to tick at CLOCK_FREQUENCY Hz

	__outb( TIMER_0, ( TIMER_FREQUENCY / CLOCK_FREQUENCY ) & 0xff );
	__outb( TIMER_0, ( TIMER_FREQUENCY / CLOCK_FREQUENCY ) >> 8 );

	// install the clock ISR

	(void) __install_isr( INT_VEC_TIMER, _isr_clock );

	// announce that we have initialized the clock module

	c_puts( " clock" );

}
