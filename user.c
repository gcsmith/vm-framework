/*
** SCCS ID:	@(#)user.c	1.1	04/04/08
**
** File:	user.c
**
** Author:	4003-506 class of 20073
**
** Contributor:
**
** Description:	User routines.
*/

#include "headers.h"

#include "user.h"
#include "processes.h"
#include "segment.h"
#include "bootstrap.h"

/*
** USER PROCESSES
**
** Each is designed to test some facility of the OS:
**
**	User(s)		Tests/Features
**	=======		===============================================
**	A, B, C		Basic operation
**	D		Spawns Z and exits
**	E, F, G		Sleep for different lengths of time
**	H		Doesn't call exit()
**	J		Tries to spawn 2*N_PROCESSES copies of Y
**	K		Spawns several copies of X
**	L		Spawns several copies of X, changes PID
**	M		Spawns W three times, reporting PIDs
**	N		Spawns several copies of X, changes priority
**	P		Iterates three times, printing system time
**	Q		Tries to execute a bogus system call (bad code)
**	R		Reading and writing
**	S		Loops forever, sleeping 30 seconds at a time
**
**	W, X, Y, Z	Print characters (spawned by other processes)
**
** Output from user processes is always alphabetic.  Uppercase 
** characters are "expected" output; lowercase are "erroneous"
** output.
**
** More specific information about each user process can be found in
** the header comment for that function (below).
**
** To spawn a specific user process, uncomment its SPAWN_x
** definition in the user.h header file.
*/

/*
** Prototypes for all one-letter user main routines
*/

void user_a_main( void ); void user_b_main( void ); void user_c_main( void );
void user_d_main( void ); void user_e_main( void ); void user_f_main( void );
void user_g_main( void ); void user_h_main( void ); void user_i_main( void );
void user_j_main( void ); void user_k_main( void ); void user_l_main( void );
void user_m_main( void ); void user_n_main( void ); void user_o_main( void );
void user_p_main( void ); void user_q_main( void ); void user_r_main( void );
void user_s_main( void ); void user_t_main( void ); void user_u_main( void );
void user_v_main( void ); void user_w_main( void ); void user_x_main( void );
void user_y_main( void ); void user_z_main( void );

void busy_wait(void) {
	int i, j;

	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < STD_DELAY; ++j )
			continue;
		write( 'Z' );
		write( 'z' );
	}
}

void mem_test_main(void) {
    //int n = 8, i, j;

    /*physblock_t block[128];*/
    char *msg_data   = "- message (data)\n";
    char msg_stack[] = "- message (stack)\n";

    puts("user mem_test running\n");
    puts(msg_data);
    puts(msg_stack);

    /*_phys_alloc(1 << 20, &block[0]); _debug_memory();
    _phys_alloc(1 << 20, &block[1]); _debug_memory();
    _phys_alloc(1 << 20, &block[2]); _debug_memory();
    _phys_alloc(1 << 20, &block[3]); _debug_memory();
    _phys_alloc(1 << 20, &block[4]); _debug_memory();
    _phys_alloc(1 << 20, &block[5]); _debug_memory();
    _phys_alloc(1 << 20, &block[6]); _debug_memory();
    _phys_alloc(1 << 20, &block[7]); _debug_memory();

    _phys_free(block[4]); _debug_memory(); busy_wait();
    _phys_free(block[0]); _debug_memory(); busy_wait();
    _phys_free(block[2]); _debug_memory(); busy_wait();
    _phys_free(block[6]); _debug_memory(); busy_wait();
    _phys_free(block[3]); _debug_memory(); busy_wait();
    _phys_free(block[1]); _debug_memory(); busy_wait();
    _phys_free(block[5]); _debug_memory(); busy_wait();
    _phys_free(block[7]); _debug_memory(); busy_wait();*/

    printf("----------------------------\n");
    printf("Total Physical Memory = 0x%x\n", _get_physmem());
    printf("Kernel Start Address  = 0x%x\n", _get_kernstart());
    printf("Kernel Image Length   = 0x%x\n", _get_kernlength());
    printf("----------------------------\n\n");

    /* make sure alloc fails when out of memory */
    puts("Attempting to exhaust all memory...\n");
    
    //while (_phys_alloc(1 << 20, &block[n])) {
    //    write('+');
    //    ++n;
    //}

    printf("Exhausted at index = %d\n", 0/*n*/);

    exit();
}

void user_swap(void) {
    int i;
    for (i = 5; i > 0; i--) {
        printf("Hotswapping in %d...\n", i);
        sleep(200);
    }
    hotswap();
    exit();
}

/*
** Users A, B, and C are identical, except for the character they
** print out via write().  Each prints its ID, then loops 30
** times delaying and printing, before exiting.
*/

void user_a_main( void ) {
	int i, j;

	puts( "User process A running\n" );
	write( 'A' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < STD_DELAY; ++j )
			continue;
		write( 'A' );
	}

	puts( "User process A exiting\n" );
	exit();

}

void user_b_main( void ) {
	int i, j;

	puts( "User process B running\n" );
	write( 'B' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < STD_DELAY; ++j )
			continue;
		write( 'B' );
	}

	puts( "User process B exiting\n" );
	exit();

}

void user_c_main( void ) {
	int i, j;

	puts( "User process C running\n" );
	write( 'C' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < STD_DELAY; ++j )
			continue;
		write( 'C' );
	}

	puts( "User process C exiting\n" );
	exit();

	write( 'c' );	/* shouldn't happen! */

}


/*
** User D spawns user Z.
*/

void user_d_main( void ) {
	int pid;

	puts( "User process D running\n" );
	write( 'D' );
	pid = fork();
	if( pid < 0 ) {
		write( 'd' );
	} else if( pid == 0 ) {
		exec( user_z_main );
		write( 'z' );
		puts( "User D, exec of Z failed\n" );
		exit();
	}
	write( 'D' );

	puts( "User process D exiting\n" );
	exit();

}


/*
** Users E, F, and G test the sleep facility.
**
** User E sleeps for 10 seconds at a time.
*/

void user_e_main( void ) {
	int i;

	puts( "User process E running\n" );
	write( 'E' );
	for( i = 0; i < 5 ; ++i ) {
		sleep( 1000 );
		write( 'E' );
	}

	puts( "User process E exiting\n" );
	exit();

}


/*
** User F sleeps for 5 seconds at a time.
*/

void user_f_main( void ) {
	int i;

	puts( "User process F running\n" );
	write( 'F' );
	for( i = 0; i < 5 ; ++i ) {
		sleep( 500 );
		write( 'F' );
	}

	puts( "User process F exiting\n" );
	exit();

}


/*
** User G sleeps for 15 seconds at a time.
*/

void user_g_main( void ) {
	int i;

	puts( "User process G running\n" );
	write( 'G' );
	for( i = 0; i < 5; ++i ) {
		sleep( 1500 );
		write( 'G' );
	}

	puts( "User process G exiting\n" );
	exit();

}


/*
** User H is like A-C except it only loops 5 times and doesn't
** call exit().
*/

void user_h_main( void ) {
	int i, j;

	puts( "User process H running\n" );
	write( 'H' );
	for( i = 0; i < 5; ++i ) {
		for( j = 0; j < STD_DELAY; ++j )
			continue;
		write( 'H' );
	}

	puts( "User process H returning without exiting!\n" );

}


/*
** User J tries to spawn 2*N_PROCESSES copies of user_y.
*/

void user_j_main( void ) {
	int i, pid;

	puts( "User process J running\n" );
	write( 'J' );

	for( i = 0; i < N_PROCESSES * 2 ; ++i ) {
		pid = fork();
		if( pid < 0 ) {
			write( 'j' );
		} else if( pid == 0 ) {
			exec( user_y_main );
			write( 'y' );
			puts( "User J, exec of y failed\n" );
		} else {
			write( 'J' );
		}
	}

	puts( "User process J exiting\n" );
	exit();

}


/*
** User K prints, goes into a loop which runs three times, and exits.
** In the loop, it does a spawn of user_x, sleeps 30 seconds, and prints.
*/

void user_k_main( void ) {
	int i, pid;

	puts( "User process K running\n" );
	write( 'K' );

	for( i = 0; i < 3 ; ++i ) {
		pid = fork();
		if( pid < 0 ) {
			write( 'k' );
		} else if( pid == 0 ) {
			exec( user_x_main );
			write( 'x' );
			puts( "User K, exec of x failed\n" );
		} else {
			sleep( 3000 );
			write( 'K' );
		}
	}

	puts( "User process K exiting\n" );
	exit();

}


/*
** User L is like user K, except that it reports the ssytem time in each
** iteration of the loop.
*/

void user_l_main( void ) {
	int i, pid;
	Time time;

	puts( "User process L running, " );
	time = gettime();
	printf( " initial time %d\n", time );
	write( 'L' );

	for( i = 0; i < 3 ; ++i ) {
		time = gettime();
		pid = fork();
		if( pid < 0 ) {
			write( 'l' );
		} else if( pid == 0 ) {
			exec( user_x_main );
			write( 'x' );
			printf( "User L, exec failed at time %d\n", time );
		} else {
			printf( "User L, exec succeeded at time %d\n", time );
			sleep( 3000 );
			write( 'L' );
		}
	}

	time = gettime();
	printf( "User process L exiting at time %d\n", time );
	exit();

}


/*
** User M iterates spawns three copies of user W, reporting their PIDs.
*/

void user_m_main( void ) {
	int i, pid[3];

	puts( "User process M running\n" );
	for( i = 0; i < 3; ++i ) {
		pid[i] = fork();
		if( pid[i] < 0 ) {
			write( 'm' );
		} else if( pid[i] == 0 ) {
			exec( user_w_main );
			write( 'w' );
			puts( "User M, exec of W failed\n" );
		} else {
			printf( "User M spawned W, PID %d\n", pid[i] );
			write( 'M' );
		}
	}

	puts( "User process M exiting\n" );
	exit();

}


/*
** User N is like user L, except that it reports its PID when starting
*/

void user_n_main( void ) {
	int i, pid;
	Time time;

	puts( "User process N running, " );
	pid = getpid();
	printf( "pid %d\n", pid );
	write( 'N' );

	for( i = 0; i < 3 ; ++i ) {
		time = gettime();
		pid = fork();
		if( pid < 0 ) {
			write( 'n' );
		} else if( pid == 0 ) {
			exec( user_x_main );
			write( 'x' );
			printf( "User N, exec failed at time %d\n", time );
		} else {
			printf( "User N, exec succeeded at time %d\n", time );
			sleep( 3000 );
			write( 'N' );
		}
	}

	puts( "User process N exiting\n" );
	exit();

}


/*
** User P iterates three times.  Each iteration sleeps for two seconds,
** then gets and prints the system time.
*/

void user_p_main( void ) {
	Time t;
	int i;

	printf( "User process P running, " );
	t = gettime();
	printf( " time %d\n", t );

	write( 'P' );

	for( i = 0; i < 3; ++i ) {
		sleep( 200 );
		t = gettime();
		printf( "User process P reporting time %d\n", t );
		write( 'P' );
	}

	printf( "User process P exiting\n" );
	exit();

}


/*
** User Q does a bogus system call
*/

void user_q_main( void ) {

	puts( "User process Q running\n" );
	write( 'Q' );
	bogus();
	puts( "User process Q exiting!?!?!\n" );
	exit();

}


/*
** User R loops 3 times reading/writing, then exits.
*/

void user_r_main( void ) {
	int i;
	int ch = '&';

	puts( "User process R running\n" );
	for( i = 0; i < 3; ++i ) {
		do {
			write( 'R' );
			ch = read();
			if( ch == -1 ) {	/* wait a bit */
				sleep( 100 );
			}
		} while( ch == -1 );
		write( ch );
	}

	puts( "User process R exiting\n" );
	exit();

}


/*
** User S sleeps for 30 seconds at a time, and loops forever.
*/

void user_s_main( void ) {

	puts( "User process S running\n" );
	write( 'S' );
	for(;;) {
		sleep( 3000 );
		write( 'S' );
	}

	puts( "User process S exiting!?!?!n" );
	exit();

}


/*
** User W prints W characters 20 times, sleeping 3 seconds between.
*/

void user_w_main( void ) {
	int i;
	int pid;

	printf( "User process W running, " );
	pid = getpid();
	printf( " PID %d\n", pid );
	for( i = 0; i < 20 ; ++i ) {
		write( 'W' );
		sleep( 300 );
	}

	printf( "User process W exiting, PID %d\n", pid );
	exit();

}


/*
** User X prints X characters 20 times.  It is spawned multiple
** times, and prints its PID when started and exiting.
*/

void user_x_main( void ) {
	int i, j;
	int pid;

	puts( "User process X running, " );
	pid = getpid();
	printf( "PID %d, ", pid );

	for( i = 0; i < 20 ; ++i ) {
		write( 'X' );
		for( j = 0; j < STD_DELAY; ++j )
			continue;
	}

	printf( "User process X exiting, PID %d\n", pid );
	exit();

}


/*
** User Y prints Y characters 10 times.
*/

void user_y_main( void ) {
	int i, j;

	puts( "User process Y running\n" );
	for( i = 0; i < 10 ; ++i ) {
		write( 'Y' );
		for( j = 0; j < ALT_DELAY; ++j )
			continue;
		sleep( 100 );
	}

	puts( "User process Y exiting\n" );
	exit();

}


/*
** User Z prints Z characters 10 times.
*/

void user_z_main( void ) {
	int i, j;

	puts( "User process Z running\n" );
	for( i = 0; i < 10 ; ++i ) {
		write( 'Z' );
		for( j = 0; j < STD_DELAY; ++j )
			continue;
	}

	puts( "User process Z exiting\n" );
	exit();

}


/*
** SYSTEM PROCESSES
*/


/*
** Idle process
*/

void idle_main( void ) {
	int i;

	write( '.' );

	for(;;) {
		for( i = 0; i < LONG_DELAY; ++i )
			continue;
		write( '.' );
	}

	puts( "+++ Idle process done!?!?!\n" );

	exit();
}


/*
** Initial process; it starts the other top-level user processes.
*/

void first_main( void ) {
    char *pargs[] = {"user2", "arg1", "hello", "world", "arg4", NULL};
	int pid;

	write( '$' );

#ifdef SPAWN_EXT_USER1
    pid = fork();
    if (pid < 0) {
        puts("first: can't fork user 'user1'\n");
    }
    else if (pid == 0) {
        execve("0x40000", NULL);
        puts("first: can't execve user 'user1'\n");
        exit();
    }
#endif

#ifdef SPAWN_EXT_USER2
    pid = fork();
    if (pid < 0) {
        puts("first: can't fork user 'user2'\n");
    }
    else if (pid == 0) {
        execve("0x50000", pargs);
        puts("first: can't execve user 'user2'\n");
        exit();
    }
#endif

#ifdef SPAWN_SWAP
    pid = fork();
    if (pid < 0) {
        puts("first: can't fork user 'swap'\n");
    }
    else if (pid == 0) {
        exec(user_swap);
        puts("first: can't execve user 'swap'\n");
        exit();
    }
#endif

#ifdef SPAWN_A
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user A\n" );
	} else if( pid == 0 ) {
		exec( user_a_main );
		puts( "first: can't exec user A\n" );
		exit();
	}
#endif

#ifdef SPAWN_B
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user B\n" );
	} else if( pid == 0 ) {
		exec( user_b_main );
		puts( "first: can't exec user B\n" );
		exit();
	}
#endif

#ifdef SPAWN_C
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user C\n" );
	} else if( pid == 0 ) {
		exec( user_c_main );
		puts( "first: can't exec user C\n" );
		exit();
	}
#endif

#ifdef SPAWN_D
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user D\n" );
	} else if( pid == 0 ) {
		exec( user_d_main );
		puts( "first: can't exec user D\n" );
		exit();
	}
#endif

#ifdef SPAWN_E
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user E\n" );
	} else if( pid == 0 ) {
		exec( user_e_main );
		puts( "first: can't exec user E\n" );
		exit();
	}
#endif

#ifdef SPAWN_F
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user F\n" );
	} else if( pid == 0 ) {
		exec( user_f_main );
		puts( "first: can't exec user F\n" );
		exit();
	}
#endif

#ifdef SPAWN_G
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user G\n" );
	} else if( pid == 0 ) {
		exec( user_g_main );
		puts( "first: can't exec user G\n" );
		exit();
	}
#endif

#ifdef SPAWN_H
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user H\n" );
	} else if( pid == 0 ) {
		exec( user_h_main );
		puts( "first: can't exec user H\n" );
		exit();
	}
#endif

#ifdef SPAWN_J
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user J\n" );
	} else if( pid == 0 ) {
		exec( user_j_main );
		puts( "first: can't exec user J\n" );
		exit();
	}
#endif

#ifdef SPAWN_K
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user K\n" );
	} else if( pid == 0 ) {
		exec( user_k_main );
		puts( "first: can't exec user K\n" );
		exit();
	}
#endif

#ifdef SPAWN_L
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user L\n" );
	} else if( pid == 0 ) {
		exec( user_l_main );
		puts( "first: can't exec user L\n" );
		exit();
	}
#endif

#ifdef SPAWN_M
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user M\n" );
	} else if( pid == 0 ) {
		exec( user_m_main );
		puts( "first: can't exec user M\n" );
		exit();
	}
#endif

#ifdef SPAWN_N
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user N\n" );
	} else if( pid == 0 ) {
		exec( user_n_main );
		puts( "first: can't exec user N\n" );
		exit();
	}
#endif

#ifdef SPAWN_P
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user P\n" );
	} else if( pid == 0 ) {
		exec( user_p_main );
		puts( "first: can't exec user P\n" );
		exit();
	}
#endif

#ifdef SPAWN_Q
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user Q\n" );
	} else if( pid == 0 ) {
		exec( user_q_main );
		puts( "first: can't exec user Q\n" );
		exit();
	}
#endif

#ifdef SPAWN_R
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user R\n" );
	} else if( pid == 0 ) {
		exec( user_r_main );
		puts( "first: can't exec user R\n" );
		exit();
	}
#endif

#ifdef SPAWN_S
	pid = fork();
	if( pid < 0 ) {
		puts( "first: can't fork user S\n" );
	} else if( pid == 0 ) {
		exec( user_s_main );
		puts( "first: can't exec user S\n" );
		exit();
	}
#endif

	write( '!' );

	exit();

}
