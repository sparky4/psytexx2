// Code by Alex Zolotov (NightRadio): nightradio@gmail.com (www.warmplace.ru)
#include <stdio.h>
#include "pdb_create.h"

char *info = "\n\
     '              '              '          .\n\
     |           '      '          :   .  '   . \n\
  -  +--+  +--- -|  .   |  - +--- -|. .|  |. -|   -\n\
    .| '|  |     |. |   |.   |     '| |'  '| |'\n\
 -  -+--+- +--+ -+--+- -+--- +---  - X -  - X  -\n\
    '|'      .|     |   |    |.    .| |.  .| |. \n\
     |-  - ---+- ---+   +-- -+---- |'- |  |-  | \n\
     :                             '   .  '   :\n\
     .                                    '   .\n\
     \n\
     Files -> PalmOS PDB converter\n\
     USAGE:\n\
     pconv [input files]\n\n\
";

long get_file_size( char* filename )
{
    long len;
    FILE *f = fopen( filename, "rb" );
    if( f )
    {
	fseek( f, 0, 2 );
	len = ftell( f );
	fclose( f );
	return len;
    } else return 0;
}

char pdb_name[1024];
void get_pdb_name( char *src )
{
    int a = strlen(src) - 1;
    strcpy( pdb_name, src );
    for( ; a >= 0 ; a-- )
    {
	pdb_name[a] = src[a];
	if( pdb_name[a] == '.' ) break;
    }
    pdb_name[a+1] = 'p';
    pdb_name[a+2] = 'd';
    pdb_name[a+3] = 'b';
    pdb_name[a+4] = 0;
}

int main(int argc, char* argv[])
{
    char *par1;
    char pdb_name[ 512 ];
    int a, b;
    int filenum;
    long output_size;

    if( argc > 1 )
    {
	for( filenum = 0; filenum < argc - 1; filenum ++ )
	{
    	    par1 = argv[ filenum + 1 ];
	    output_size = get_file_size( par1 );
	    if( output_size )
	    {
		make_output_name( par1 ); //Palm database name (internal)
		//Make output PDB name:
		for( a = 0; ; a++ ) { pdb_name[a] = par1[a]; if( par1[a] == 0 ) break; }
		for( b = a; b >= 0; b-- )
		{
		    if( par1[b] == '.' ) break;
		}
		if( b <= 0 ) b = a;
		pdb_name[ b++ ] = '.';
		pdb_name[ b++ ] = 'p';
		pdb_name[ b++ ] = 'd';
		pdb_name[ b++ ] = 'b';
		pdb_name[ b++ ] = 0;
		//Convert:
		convert( output_size, par1, pdb_name );
		printf( "%s --> %s\n", par1, pdb_name );
	    } else printf( "%s not found\n", par1 );
	}
    } else printf( info );
    return 0;
}
