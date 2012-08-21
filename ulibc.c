/*
** SCCS ID:	@(#)ulibc.c	1.1	04/04/08
**
** File:	ulibc.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	C implementations of some user-level library routines
*/

#include "headers.h"


/*
** prt_status - print a status value to the console
**
** the 'msg' argument should contain a %s where
** the desired status value should be printed
*/

void prt_status( char *msg, Status stat ) {

	if( msg == NULL ) {
		return;
	}

	switch( stat ) {
		case ERR_NONE:
			printf( msg, "ERR_NONE" );
			break;

		case ERR_QNODE:
			printf( msg, "ERR_QNODE" );
			break;

		case ERR_PCB:
			printf( msg, "ERR_PCB" );
			break;

		case ERR_STACK:
			printf( msg, "ERR_STACK" );
			break;

		case ERR_NULLPTR:
			printf( msg, "ERR_NULLPTR" );
			break;

		case ERR_EMPTYQ:
			printf( msg, "ERR_EMPTYQ" );
			break;

		default:
			printf( msg, "ERR_????" );
			break;
	
	}

}

int isdigit(int ch) {
    return (ch >= '0' && ch <= '9');
}

long int strtol(const char *str, char **endptr, int base) {
    long value = 0;

    /* skip past any whitespace */
    while ((*str != '0') && (*str != '\0')) { str++; }

    if (str[0] == '0' && isdigit(str[1])) {
        /* source string is octal format */
        str++;

        while (*str != '\0') {
            char c = *str++;
            if (c >= '0' && c <= '7') {
                value = (value * 8) + (long)(c - '0');
            }
            else break; /* hit an invalid character */
        }
    }
    else if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        /* source string is hexadecimal format */
        str += 2;

        while (*str != '\0') {
            char c = *str++;
            if (c >= '0' && c <= '9') {
                value = (value << 4) | (int)(c - '0');
            }
            else if (c >= 'a' && c <= 'f') {
                value = (value << 4) | (int)(c - 'a' + 10);
            }
            else if (c >= 'A' && c <= 'F') {
                value = (value << 4) | (int)(c - 'A' + 10);
            }
            else break; /* hit an invalid character */
        }
    }
    else {
        /* source string is decimal format */
        while (*str != '\0') {
            char c = *str++;
            if (c >= '0' && c <= '9') {
                value = (value * 10) + (long)(c - '0');
            }
            else break; /* hit an invalid character */
        }
    }

    /* optionally set end pointer */
    if (NULL != endptr) {
        *endptr = (char *)str;
    }

    return value;
}

char __hexdigits[] = "0123456789ABCDEF";

char * __cvtdec0( char *buf, int value ){
	int	quotient;

	quotient = value / 10;
	if( quotient < 0 ){
		quotient = 214748364;
		value = 8;
	}
	if( quotient != 0 ){
		buf = __cvtdec0( buf, quotient );
	}
	*buf++ = value % 10 + '0';
	return buf;
}

int __cvtdec( char *buf, int value ){
	char	*bp = buf;

	if( value < 0 ){
		*bp++ = '-';
		value = -value;
	}
	bp = __cvtdec0( bp, value );
	*bp = '\0';

	return bp - buf;
}

int __cvthex( char *buf, int value ){
	int	i;
	int	chars_stored = 0;
	char	*bp = buf;

	for( i = 0; i < 8; i += 1 ){
		int	val;

		val = ( value & 0xf0000000 );
		if( i == 7 || val != 0 || chars_stored ){
			chars_stored = 1;
			val >>= 28;
			val &= 0xf;
			*bp++ = __hexdigits[ val ];
		}
		value <<= 4;
	}
	*bp = '\0';

	return bp - buf;
}

int __cvtoct( char *buf, int value ){
	int	i;
	int	chars_stored = 0;
	char	*bp = buf;
	int	val;

	val = ( value & 0xc0000000 );
	val >>= 30;
	for( i = 0; i < 11; i += 1 ){

		if( i == 10 || val != 0 || chars_stored ){
			chars_stored = 1;
			val &= 0x7;
			*bp++ = __hexdigits[ val ];
		}
		value <<= 3;
		val = ( value & 0xe0000000 );
		val >>= 29;
	}
	*bp = '\0';

	return bp - buf;
}

static int __pad( int x, int y, int extra, int padchar ){
	while( extra > 0 ){
		if( x != -1 || y != -1 ){
			//c_putchar_at( x, y, padchar );
			x += 1;
		}
		else {
			putc( padchar );
		}
		extra -= 1;
	}
	return x;
}

static int __padstr( int x, int y, char *str, int len,
                     int width, int leftadjust, int padchar ){
	int	extra;

	if( len < 0 ){
		len = strlen( str );
	}
	extra = width - len;
	if( extra > 0 && !leftadjust ){
		x = __pad( x, y, extra, padchar );
	}
	if( x != -1 || y != -1 ){
		//c_puts_at( x, y, str );
		x += len;
	}
	else {
		puts( str );
	}
	if( extra > 0 && leftadjust ){
		x = __pad( x, y, extra, padchar );
	}
	return x;
}

static void __do_printf( int x, int y, char **f ){
	char	*fmt = *f;
	char	*ap;
	char	buf[ 12 ];
	char	ch;
	char	*str;
	int	leftadjust;
	int	width;
	int	len;
	int	padchar;

	/*
	** Get characters from the format string and process them
	*/
	ap = (char *)( f + 1 );
	while( (ch = *fmt++) != '\0' ){
		/*
		** Is it the start of a format code?
		*/
		if( ch == '%' ){
			/*
			** Yes, get the padding and width options (if there).
			** Alignment must come at the beginning, then fill,
			** then width.
			*/
			leftadjust = 0;
			padchar = ' ';
			width = 0;
			ch = *fmt++;
			if( ch == '-' ){
				leftadjust = 1;
				ch = *fmt++;
			}
			if( ch == '0' ){
				padchar = '0';
				ch = *fmt++;
			}
			while( ch >= '0' && ch <= '9' ){
				width *= 10;
				width += ch - '0';
				ch = *fmt++;
			}

			/*
			** What data type do we have?
			*/
			switch( ch ){
			case 'c':
				ch = *(int *)ap++;
				buf[ 0 ] = ch;
				buf[ 1 ] = '\0';
				x = __padstr( x, y, buf, 1, width, leftadjust, padchar );
				break;

			case 'd':
				len = __cvtdec( buf, *(int *)ap);
				x = __padstr( x, y, buf, len, width, leftadjust, padchar );
                ap += sizeof(int);
				break;

			case 's':
				str = *(char **)ap;
				x = __padstr( x, y, str, -1, width, leftadjust, padchar );
                ap += sizeof(char *);
				break;

			case 'x':
				len = __cvthex( buf, *(int *)ap);
				x = __padstr( x, y, buf, len, width, leftadjust, padchar );
                ap += sizeof(int);
				break;

			case 'o':
				len = __cvtoct( buf, *(int *)ap);
				x = __padstr( x, y, buf, len, width, leftadjust, padchar );
                ap += sizeof(int);
				break;

			}
		}
		else {
			if( x != -1 || y != -1 ){
				//c_putchar_at( x, y, ch );
				switch( ch ){
				case '\n':
					y += 1;
					/* FALL THRU */

				case '\r':
					//x = scroll_min_x;
					break;

				default:
					x += 1;
				}
			}
			else {
				putc( ch );
			}
		}
	}
}

unsigned int strlen( char const *str ) {
	unsigned int	len = 0;

	while( *str++ != '\0' ){
		len += 1;
	}
	return len;
}

char *strcpy(char *dest, const char *src) {
    char *ret = dest;

    while (0 != (*dest++ = *src++));

    return ret;
}

void printf( char *fmt, ... ) {
    __do_printf(-1, -1, &fmt);
}

