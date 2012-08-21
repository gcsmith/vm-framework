/*
** SCCS ID:	@(#)sio.c	1.1	04/04/08
**
** File:	sio.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	SIO module
**
** Our SIO scheme is very simple:
**
**	Input:	We maintain a buffer of incoming characters that haven't
**		yet been read by processes.  When a character comes in,
**		if there is no process waiting for it, it goes in the 
**		buffer; otherwise, the first waiting process is awakened
**		and it gets the character.
**
**		When a process invokes read(), if there is a character in
**		the input buffer, the process gets it; otherwise, it is
**		blocked until input appears.
**
**		Communication with user processes is via the _sio_read()
**		routine.  This returns the first available character (if
**		there is one), resetting the input variables if this was
**		the last character in the buffer.  If there are no
**		characters in the buffer, _sio_read() returns a -1
**		(presumably so the requesting process can be blocked).
**
**	Output:	We maintain a buffer of outgoing characters that haven't
**		yet been sent to the device, and an indication of whether
**		or not we are in the middle of a transmit sequence.  When
**		an interrupt comes in, if there is another character to
**		send we copy it to the transmitter buffer; otherwise, we
**		end the transmit sequence.
**
**		Communication with user processes is via the _sio_write()
**		routine.  If we are in the middle of a transmit sequence,
**		the character will be added to the output buffer (from
**		where it will be sent automatically); otherwise, we send
**		the character directly, and set the "sending" flag to 
**		indicate that we're expecting a transmitter interrupt.
*/

#define	__KERNEL__20073__

#include "headers.h"

#include "sio.h"
#include "processes.h"
#include "queues.h"
#include "scheduler.h"

#include "startup.h"
#include <uart.h>
#include <x86arch.h>

/*
** PRIVATE DEFINITIONS
*/

#define	BUF_SIZE	1024

/*
** PRIVATE GLOBALS
*/

	// input character buffer
static char g_inbuffer[ BUF_SIZE ];
static char *g_inlast;
static char *g_innext;
static int g_incount;

	// output character buffer
static char g_outbuffer[ BUF_SIZE ];
static char *g_outlast;
static char *g_outnext;
static int g_outcount;

	// output control flag
static int g_sending;

/*
** PRIVATE FUNCTIONS
*/

/*
** _isr_sio - serial i/o ISR
*/

static void _isr_sio( int vector, int code ) {
	Pcb *pcb;
	int eir;
	int ch;

	//
	// Must process all pending events; loop until the EIR
	// says there's nothing else to do.
	//

	for( ; ; ) {
		// get the "pending event" indicator
		eir = __inb( UA4_EIR ) & UA4_EIR_INT_PRI_MASK;

		// process this event
		switch( eir ) {

		   case UA4_EIR_TX_INT_PENDING:
			// if there is another character, send it
			if( g_sending && g_outcount > 0 ) {
				__outb( UA4_TXD, *g_outnext );
				++g_outnext;
				--g_outcount;
			} else {
				// no more data - reset the output vars
				g_outcount = 0;
				g_outlast = g_outnext = g_outbuffer;
				g_sending = 0;
			}
			break;

		   case UA4_EIR_RX_INT_PENDING:
			// get the character
			ch = __inb( UA4_RXD );
			if( ch == '\r' ) {	// map CR to LF
				ch = '\n';
			}

			//
			// If there is a waiting process, this must be 
			// the first input character; give it to that
			// process and awaken the process.
			//

			if( g_sio_blocked.front != 0 ) {

				if( _deque(&g_sio_blocked,(void **)&pcb) != ERR_NONE ) {
					_kpanic( "sio isr - serial wakeup failed" );
				}
				pcb->context->eax = ch & 0xff;
				_schedule( pcb );

			} else {

				//
				// Nobody waiting - add to the input buffer
				// if there is room, otherwise just ignore it.
				//

				if( g_incount < BUF_SIZE ) {
					*g_inlast++ = ch;
					++g_incount;
				}
			
			}

			break;

		   case UA4_EIR_NO_INT:
			// nothing to do - tell the PIC we're done
			__outb( PIC_MASTER_CMD_PORT, PIC_EOI );
			return;

		   default:
			_kpanic( "sio isr - unknown device status" );

		}
	
	}

}

/*
** PUBLIC FUNCTIONS
*/

/*
** _init_sio
**
** Initialize the UART chip.
*/
void _init_sio( void ) {

	//
	// Initialize SIO variables.
	//

	_memclr( (void *) g_inbuffer, sizeof(g_inbuffer) );
	g_inlast = g_innext = g_inbuffer;
	g_incount = 0;

	_memclr( (void *) g_outbuffer, sizeof(g_outbuffer) );
	g_outlast = g_outnext = g_outbuffer;
	g_outcount = 0;
	g_sending = 0;

	//
	// Next, initialize the UART.
	//

	// Initialize the FIFOs
	//
	// this is a bizarre little sequence of operations

	__outb( UA4_FCR, 0x20 );
	__outb( UA4_FCR, 0x00 );		// reset
	__outb( UA4_FCR, UA5_FCR_FIFO_EN );	// 0x01
	__outb( UA4_FCR, UA5_FCR_FIFO_EN |
			 UA5_FCR_RXSR );	// 0x03
	__outb( UA4_FCR, UA5_FCR_FIFO_EN |
			 UA5_FCR_RXSR |
			 UA5_FCR_TXSR );	// 0x07

	// disable interrupts

	__outb( UA4_IER, 0 );

	// select bank 1 and set the data rate

	__outb( UA4_LCR, UA4_LCR_BANK1 );
	__outb( UA4_LBGD_L, BAUD_LOW_BYTE( BAUD_9600 ) );
	__outb( UA4_LBGD_H, BAUD_HIGH_BYTE( BAUD_9600 ) );

	// Select bank 0, and at the same time set the LCR for our
	// data characteristics.

	__outb( UA4_LCR, UA4_LCR_BANK0 |
			 UA4_LCR_BITS_8 |
			 UA4_LCR_1_STOP_BIT |
			 UA4_LCR_NO_PARITY );
	
	// Set the ISEN bit to enable the interrupt request signal.

	__outb( UA4_MCR, UA4_MCR_ISEN | UA4_MCR_DTR | UA4_MCR_RTS );

	// Install our ISR

	__install_isr( INT_VEC_SERIAL_PORT_1, _isr_sio );

	// Enable device interrupts.

	__outb( UA4_IER, UA4_IER_TX_INT_ENABLE | UA4_IER_RX_INT_ENABLE );

	// Report that we're done.

	c_puts( " sio" );

}


/*
** _sio_read - get the next input character
**
** usage:	ch = _sio_read()
**
** returns the next character, or -1 if no character is available
*/

int _sio_read( void ) {
	int ch;

	// assume there is no character available
	ch = -1;

	// 
	// If there is a character, return it
	//

	if( g_incount > 0 ) {

		// take it out of the input buffer
		ch = (int)(*g_innext++) & 0xff;
		--g_incount;

		// reset the buffer variables if this was the last one
		if( g_incount < 1 ) {
			g_inlast = g_innext = g_inbuffer;
		}

	}

	return( ch );

}


/*
** _sio_write - write a character to the serial output
**
** usage:	_sio_write( ch )
*/

void _sio_write( int ch ){

	//
	// Must do LF -> CRLF mapping
	//

	if( ch == '\n' ) {
		_sio_write( '\r' );
	}

	//
	// If we're currently transmitting, just add this to the buffer
	//

	if( g_sending ) {
		*g_outlast++ = ch;
		++g_outcount;
		return;
	}

	//
	// Not sending - must prime the pump
	//

	g_sending = 1;
	__outb( UA4_TXD, ch );

}
