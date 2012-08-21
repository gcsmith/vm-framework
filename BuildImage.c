/*
** SCCS ID:	@(#)BuildImage.c	1.2	05/16/02
**
** File:	BuildImage.c
**
** Author:	K. Reek
**
** Contributor: Jon Coles, Warren R. Carithers, Garrett C. Smith
**
** Description:	Modify the bootstrap image to include the information
**		on the programs to be loaded, and produce the file
**		that contains the concatenation of these programs.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	TRUE	1
#define	FALSE	0

char	*progname;
char	*output_filename;
FILE	*out;

/*
** Array into which program information will be stored, starting at the
** end and moving back toward the front.  The array is sized at 512
** bytes, which is guaranteed to be larger than the maximum possible
** space available for this stuff in the bootstrap image.  Thus, the
** bootstrap image itself (and the amount of space available on the
** floppy) are the only limiting factors on how many program sections
** can be loaded.
*/
#define	N_INFO	( 512 / sizeof( short ) )
short	info[ N_INFO ];
int	n_info = N_INFO;

void quit( char *msg, int call_perror ) {
	if( msg != NULL ){
		fprintf( stderr, "%s: ", progname );
		if( call_perror ){
			perror( msg );
		}
		else {
			fprintf( stderr, "%s\n", msg );
		}
	}
	if( output_filename != NULL ){
		unlink( output_filename );
	}
	exit( EXIT_FAILURE );
}

char	usage_error_msg[] =
  "\nUsage: %s -o outputfile boot_file drive { prog_file load_address } ...\n\n"
  "\tThere must be at least one program file and load_address.\n\n"
  "\tLoad addresses may be specified either as 32-bit quantities in hex,\n"
  "\tdecimal or octal (e.g. 0x10c00, 68608, 0206000 are all equivalent),\n"
  "\tor as an explicit segment:offset pair whose digits are always\n"
  "\tinterpreted as hexadecimal values (e.g. 10c0:0000, 1000:0c00 are\n"
  "\tboth equivalent to the previous examples).\n\n";

void usage_error( void ){
	fprintf( stderr, usage_error_msg, progname );
	quit( NULL, FALSE );
}

int copy_file( FILE *in ){
	int	n_sectors = 0;
	char	buf[ 512 ];
	int	n_bytes;
	int	i;

	/*
	** Copy the file to the output, being careful that the
	** last block is padded with null bytes.
	*/
	n_sectors = 0;
	while( (n_bytes = fread( buf, 1, sizeof( buf ), in )) > 0 ){
		if( n_bytes < sizeof( buf ) ){
			int	i;

			for( i = n_bytes; i < sizeof( buf ); i += 1 ){
				buf[ i ] = '\0';
			}
		}
		if( fwrite( buf, 1, sizeof( buf ), out ) != sizeof( buf ) ){
			quit( "Write failed or was wrong size", FALSE );
		}
		n_sectors += 1;
	}
	return n_sectors;
}

void process_file( char *name, char *addr ){
	FILE	*in;
	int	n_sectors;
	long	address;
	short	segment, offset;
	char	*cp;
	int	n_bytes;
	int	valid_address;

	/*
	** Open the input file.
	*/
	in = fopen( name, "rb" );
	if( in == NULL ){
		quit( name, TRUE );
	}

	/*
	** Copy the file to the output, being careful that the
	** last block is padded with null bytes.
	*/
	n_sectors = copy_file( in );
	fclose( in );

	/*
	** Decode the address they gave us.
	*/
	valid_address = FALSE;
	cp = strchr( addr, ':' );
	if( cp != NULL ){
		if( strlen( addr ) == 9 && cp == addr + 4 ){
			char	*u1, *u2;
			int	a1, a2;

			segment = strtol( addr, &u1, 16 );
			offset = strtol( addr + 5, &u2, 16 );
			address = ( segment << 4 ) + offset;
			valid_address = *u1 == '\0' && *u2 == '\0';
		}
	}
	else {
		char	*unused;

		address = strtol( addr, &unused, 0 );
		segment = (short)( address >> 4 );
		offset = (short)( address & 0xf );
		valid_address = *unused == '\0' && address <= 0x0009ffff;
	}
	if( !valid_address ){
		fprintf( stderr, "%s: Invalid address: %s\n", progname, addr );
		quit( NULL, FALSE );
	}

	/*
	** Make sure the program will fit!
	*/
	if( address + n_sectors * 512 > 0x0009ffff ){
		fprintf( stderr, "Program %s too large to start at 0x%08lx\n",
		    name, address );
		quit( NULL, FALSE );
	}
	if( n_info < 3 ){
		quit( "Too many programs!", FALSE );
	}


	/*
	** Looks good: report and store the information.
	*/
	fprintf( stderr, "%s: %d sectors, loaded at 0x%lx\n",
	    name, n_sectors, address );

	info[ --n_info ] = n_sectors;
	info[ --n_info ] = segment;
	info[ --n_info ] = offset;
}

int main( int ac, char **av ) {
	FILE	*bootimage;
	int	bootimage_size, drive;
	int	n_bytes, n_words;
	short	existing_data[ N_INFO ];
	int	i;
	
	/*
	** Save the program name for error messages
	*/
	progname = strrchr( av[ 0 ], '/' );
	if( progname != NULL ){
		progname++;
	}
	else {
		progname = av[ 0 ];
	}
	ac--; av++;

	if ( ac < 6 ) {
		usage_error();
	}

	/*
	** Process argument: output file name.  Output must be opened for
	** read and write, since we'll be patching it at the end.
	*/
	if( strcmp( av[ 0 ], "-o" ) != 0 ){
		usage_error();
	}

	output_filename = av[ 1 ];
	out = fopen( av[ 1 ], "wb+" );
	if( out == NULL ){
		quit( av[ 1 ], TRUE );
	}
	ac -= 2; av += 2;

	/*
	** Open the bootstrap file and copy it to the output image.
	*/
	bootimage = fopen( av[ 0 ], "rb" );
	if( bootimage == NULL ){
		quit( av[ 0 ], TRUE );
	}
	bootimage_size = copy_file( bootimage ) * 512;
	fclose( bootimage );
	ac--; av++;

    /*
    ** Write the drive index.
    */

    drive = strtol( av[ 0 ], NULL, 0 );
    ac--; av++;

	/*
	** Process the programs one by one
	*/
	while( ac >= 2 ){
		process_file( av[ 0 ], av[ 1 ] );
		ac -= 2; av += 2;
	}

	/*
	** Check for oddball leftover argument
	*/
	if( ac > 0 ){
		usage_error();
	}

	/*
	** Seek to where this array must begin and read what's already there.
	*/
	n_words = ( N_INFO - n_info );
	n_bytes = n_words * sizeof( info[ 0 ] );
	fseek( out, bootimage_size - n_bytes, SEEK_SET );
	if( fread( existing_data, sizeof( info[ 0 ] ), n_words, out ) != n_words ){
		quit( "Read from boot image failed or was too short", FALSE );
	}

	for( i = 0; i < n_words; i += 1 ){
		if( existing_data[ i ] != 0 ){
			quit( "Too many programs to load!", FALSE );
		}
	}

	/*
	** We know that we're only overwriting zeros at the end of
	** the bootstrap image, so it is ok to go ahead and do it.
	*/
	fseek( out, bootimage_size - n_bytes, SEEK_SET );
	if( fwrite( info + n_info, sizeof( info[ 0 ] ), n_words, out ) != n_words ){
		quit( "Write to boot image failed or was too short", FALSE );
	}

    /*
    ** Write the drive index to the end of the image.
    */
    fseek( out, 508, SEEK_SET );
    fwrite( (void *)&drive, sizeof( short ), 1, out );

	fclose( out );

	return EXIT_SUCCESS;
}
