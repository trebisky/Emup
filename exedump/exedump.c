/*
 * exedump.c
 *
 * Tom Trebisky  10/27/2026
 *
 * A program to dump portions of a DOS EXE file
 *
 * The initial motivation is to extract strings
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

/* epp512.exe is 84790 bytes in size
 */
#define MAXBIN	128*1024

char exebuf[MAXBIN];

typedef short WORD;

struct exe_header {
    WORD   magic;                     // Magic number
    WORD   cblp;                      // Bytes on last page of file
    WORD   cp;                        // Pages in file
    WORD   crlc;                      // Relocations
    WORD   hsize;                     // Size of header in paragraphs
    WORD   minalloc;                  // Minimum extra paragraphs needed
    WORD   maxalloc;                  // Maximum extra paragraphs needed
    WORD   ss;                        // Initial (relative) SS value
    WORD   sp;                        // Initial SP value
    WORD   csum;                      // Checksum
    WORD   ip;                        // Initial IP value
    WORD   cs;                        // Initial (relative) CS value
    WORD   lfarlc;                    // File address of relocation table
    WORD   ovno;                      // Overlay number
};

// int verbose = 0;
int verbose = 1;

static void
error ( char *msg )
{
	fprintf ( stderr, "%s\n", msg );
	exit (1);
}

static void
usage ( void )
{
	error ( "usage: exedump addr" );
}

// works for me and short
int
hex ( char *str )
{
    return strtol ( str, NULL, 16 );
}

void
show_str ( char *ss )
{
	char *p, *bp;
	char buf[256];

	p = ss;
	bp = buf;

	for ( p=ss; *p; p++ ) {
		if ( *p == '\r' ) {
			*bp++ = '\\';
			*bp++ = 'r';
			continue;
		}
		if ( *p == '\n' ) {
			*bp++ = '\\';
			*bp++ = 'n';
			continue;
		}
		*bp++ = *p;
	}
	*bp = '\0';

	printf ( "%s\n", buf );
}

int
main ( int argc, char **argv )
{
	char *infile = "epp512.exe";
	int fd;
	int nio;
	struct exe_header *ehp;
	int dsval;
	int lm_offset;
	int str_base;
	int str_addr;
	char *strp;
	int odx_offset;

	if ( argc < 2 )
		usage ();

	// printf ( "arg: %s\n", argv[1] );
	str_addr = hex ( argv[1] );

	fd = open ( infile, O_RDONLY );
	if ( fd < 0 )
		error ( "Cannot open input" );
	nio = read ( fd, exebuf, MAXBIN );
	close ( fd );


	ehp = (struct exe_header *) exebuf;
	lm_offset = ehp->hsize*16;

	if ( verbose ) {
		printf ( "%d bytes read from input file\n", nio );
		printf ( "0x%x in header\n", ehp->hsize );
		printf ( "code offset: 0x%x\n", lm_offset );
	}

	/* I don't know any way to get this from the header */
	dsval = 0xfbe;

	str_base = lm_offset + dsval*16;

	if ( verbose ) {
		odx_offset = dsval * 16 + str_addr;
		printf ( "odx offset: 0x%x\n", odx_offset );
	}

	// str_addr = 0x7fd;
	// str_addr = 0x7b1;

	strp = &exebuf[str_base + str_addr];
	show_str ( strp );
}

/* THE END */
