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

/* Stuff pertaining to the image */
int nbuf;
struct exe_header *ehp;
int lm_offset;
int dsval;

/* random global variables */
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
load_image ( char *path )
{
	int fd;
	int n;

	fd = open ( path, O_RDONLY );
	if ( fd < 0 )
		error ( "Cannot open input" );
	nbuf = read ( fd, exebuf, MAXBIN );
	close ( fd );

	ehp = (struct exe_header *) exebuf;
	lm_offset = ehp->hsize*16;

	/* I don't know any way to get this from the header */
	dsval = 0xfbe;
}

void
get_string ( int str_addr )
{
	int odx_offset;
	int str_base;
	char *strp;

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

/*
 * Note the weird 0x1f character in the first string.
 * This causes us to dump it as data.
0000fbe0 0000 0000 0000 0000 4320 4c69 6272 6172           C Librar
0000fbf0 7920 2d20 2843 2943 6f70 7972 6967 6874   y - (C)Copyright
0000fc00 204d 6963 726f 736f 6674 2043 6f72 7020    Microsoft Corp
0000fc10 3139 3836 1f00 4e4f 524d 414c 0049 4e54   1986  NORMAL INT
0000fc20 4c00 5155 4943 4b00 494e 5452 0046 4c41   L QUICK INTR FLA
 */

int
is_ok_char ( char xx )
{
	if ( xx == '\n' || xx == '\r' )
		return 1;
	if ( xx < ' ' )
		return 0;
	if ( xx > '~' )
		return 0;
	return 1;
}

int
is_string ( char *sp )
{
	if ( *sp == 0 )
		return 0;

	for ( ; *sp; sp++ )
		if ( ! is_ok_char ( *sp ) )
			return 0;
	return 1;
}

#define BUFSIZE 128

/* Make cr and nl visible */
void
fix_string ( char *buf, char *sp )
{
	char *p, *bp;

	bp = buf;

	for ( p = sp; *p; p++ ) {
		if ( *p == '\n' ) {
			*bp++ = '\\';
			*bp++ = 'n';
			continue;
		}
		if ( *p == '\r' ) {
			*bp++ = '\\';
			*bp++ = 'r';
			continue;
		}
		*bp++ = *p;
	}

	*bp = '\0';
}

/* avoid dumping byte swapped */
void
dump_short ( int val )
{
	// printf ( "%04x", val );
	printf ( "%02x", val & 0xff );
	printf ( "%02x", val >> 8 );
}

int
dump_one ( int off, char *dp )
{
	int data;
	int ns;
	char buf[BUFSIZE];
	int addr;

	// address relative to start of LM
	addr = dsval*16 + off;

	if ( is_string ( dp ) ) {
		fix_string ( buf, dp );
		if ( strlen(buf) == 1 ) {
			printf ( "cString %08x %04x: ", addr, off );
			data = *(unsigned short *) dp;
			// printf ( "(%04x) ", data );
			dump_short ( data );
			printf ( " %s\n", buf );
		} else {
			printf ( "String  %08x %04x: ", addr, off );
			printf ( "%s\n", buf );
		}
		if ( strlen(buf) > BUFSIZE )
			error ( "String too big" );
		ns = strlen ( dp );
		return ns + 1;
	}

	/* If it is odd, just dump one byte */
	if ( addr & 1 ) {
		printf ( "Data    %08x %04x: %02x\n", addr, off, *dp );
		return 1;
	}

	/* fall through to dump 2 bytes */
	data = *(unsigned short *) dp;
	printf ( "Data    %08x %04x: ", addr, off );
	dump_short ( data );
	printf ( "\n" );
	return 2;
}

void
dump_data ( void )
{
	int data_offset;
	int off;
	int ndata;
	int nd;

	char *data_base;

	data_offset = lm_offset + dsval*16;
	data_base = &exebuf[data_offset];
	ndata = nbuf - data_offset;

	printf ( "LM start: %08x\n", lm_offset );
	printf ( "Data start: %08x\n", data_offset );
	printf ( "Data end: %08x\n", nbuf );
	printf ( "Data size: %d bytes\n", nbuf - data_offset );

	for ( off = 0; off < ndata; off += nd ) {
		nd = dump_one ( off, &data_base[off] );
	}
}

enum mode { STRING, DATA };

int
main ( int argc, char **argv )
{
	char *infile = "epp512.exe";
	int str_addr;
	enum mode mode;

	if ( argc < 2 )
		usage ();

	if ( strcmp ( argv[1], "-d" ) == 0 )
		mode = DATA;
	else {
		// printf ( "arg: %s\n", argv[1] );
		str_addr = hex ( argv[1] );
		mode = STRING;
	}

	load_image ( infile );

	if ( verbose ) {
		printf ( "%d bytes read from input file\n", nbuf );
		printf ( "0x%x in header\n", ehp->hsize );
		printf ( "code offset: 0x%x\n", lm_offset );
	}

	if ( mode == STRING )
		get_string ( str_addr );

	if ( mode == DATA )
		dump_data ();
}

/* THE END */
