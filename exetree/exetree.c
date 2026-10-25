/*
 * exetree - generate a call graph/tree
 *
 * Tom Trebisky  10/25/2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 30284 lines
char *infile = "epp512_lm.raw";

// The biggest so far is 67 bytes
#define MAXLINE	100

// We have seen 8 believe it or not
#define MAXOP	10

// Max so far is 23
#define MAXARG	32

struct line {
	char orig[MAXLINE];
	int addr;
	char opcode[MAXOP];
	char arg[MAXARG];
	int is_jump;
	int is_call;
	int is_ret;
	int visited;
};

#define MAXLINES	40000
struct line lines[MAXLINES];
int nlines;

int max = 0;
// int max = 99;

/* ------------------------------------------------ */
/* "library" routines */

static void
error ( char *msg )
{
	fprintf ( stderr, "%s\n", msg );
	exit (1);
}

/* split a string in place.
 * tosses nulls into string, trashing it.
 */
int
split ( char *buf, char **bufp, int max )
{
    int i;
    char *p;

    p = buf;
    for ( i=0; i<max; ) {
        while ( *p && *p == ' ' )
            p++;
        if ( ! *p )
            break;
        bufp[i++] = p;
        while ( *p && *p != ' ' )
            p++;
        if ( ! *p )
        break;
        *p++ = '\0';
    }

    return i;
}

/* ------------------------------------------------ */

// The most we have seen is 6
// The minimum is 3
#define MAXW	8

static void
save_line ( char *ll )
{
	// char *p;
	struct line *lp;
	int size;
	char *wp[MAXW];
	int nw;

	// lp = (struct line *) = malloc ( sizeof(struct line) );
	size = strlen(ll) + 1;
	// p = malloc ( size );
	// if ( size > max )
	// 	max = size;

	lp = &lines[nlines];
	lp->is_jump = 0;
	lp->is_call = 0;
	lp->is_ret = 0;
	lp->visited = 0;

	strcpy ( lp->orig, ll );

	nw = split ( ll, wp, MAXW );
	if ( nw < 3 )
		error ( "Evil line" );
	// if ( nw < max )
	// 	max = nw;

	lp->addr = strtol ( wp[0], NULL, 16);

	strcpy ( lp->opcode, wp[2] );

	if ( nw > 3 ) {
		strcpy ( lp->arg, wp[3] );
		size = strlen ( wp[3] );
		if ( size > max )
			max = size;
	} else {
		strcpy ( lp->arg, "--" );
	}

	if ( lp->opcode[0] == 'j' )
		lp->is_jump = 1;
	if ( strcmp ( lp->opcode, "call" ) == 0 )
		lp->is_call = 1;
	if ( strncmp ( lp->opcode, "ret", 3 ) == 0 )
		lp->is_ret = 1;

	nlines++;
}

static void
loadfile ( char *path )
{
	FILE *ff;
	char lbuf[MAXLINE];

	ff = fopen ( infile, "r" );
	if ( ff == NULL )
		error ( "Cannot open input" );

	nlines = 0;
	// remember -- fgets reads the newline
	while ( fgets ( lbuf, MAXLINE, ff ) ) {
		// ditch trailing newline
		lbuf[strcspn(lbuf, "\n")] = 0;
		save_line ( lbuf );
	}

	// printf ( "%d lines loaded\n", nlines );
	// printf ( "%d max length\n", max );
	fclose ( ff );
}

/* ------------------------------------------------ */

/* We pipe this output to sort | uniq to get:
 * jmp
 * ja jna jc jnc
 * jg jng jl jnl
 * jo jno js jns
 * jz jnz 
 * jpe jpo, js
 * jcxz
 */

static void
alljumps ( void )
{
	int i;

	for ( i=0; i<nlines; i++ ) {
		if ( lines[i].is_jump )
			printf ( "%s\n", lines[i].opcode );
	}
}

/* A note on "call" lines, we see the following variety:
 *	call 0x16f:0x2836
 *	call 0x3f6d
 *	call far [ss:0x303a]
 *	call far [0x303a]
 *	call far [di]
 */

static int
find_addr ( int addr )
{
	int i;

	for ( i=0; i<nlines; i++ ) {
		if ( lines[i].addr == addr )
			return i;
	}
	error ( "Cannot find address" );
}

static void
chase ( int start )
{
	int nn;
	struct line *lp;

	nn = find_addr ( start );
	lp = &lines[nn];
	for ( ;; ) {
		if ( lp->is_call )
			printf ( "%s\n", lp->orig );
		if ( lp->is_ret ) {
			printf ( "%s\n", lp->orig );
			break;
		}
		lp++;
	}
}

int
main ( int argc, char **argv )
{
	loadfile ( infile );
	// alljumps ();
	chase ( 0x3da4 );

	exit (0);
}

/* THE END */
