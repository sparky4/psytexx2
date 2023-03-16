#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *f,*f2,*f3;
unsigned char *pdb;
unsigned char pdb_header[76]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0x3B,0x62,0xC2,0x5C,0x3B,0x62,0xC2,0x5C,0x3B,0x62,0xC2,0x5C,
                              0,0,0,0,0,0,0,0,0,0,0,0,0x50,0x53,0x59,0x58, //Type: PSYX
                              0x5A,0x55,0x4C,0x55,0,0,0,0,0,0,0,0}; //Creator: ZULU

unsigned short MWORD(unsigned short val)  //Mirror word value.
{
    unsigned short val2;
    val2=val<<8;
    val>>=8;val+=val2;
    return val;
}

unsigned long MDWORD(unsigned long val)  //Mirror dword value.
{
    unsigned long val2;
    val2=0;
    val2+=((val&0x000000FF)<<24);
    val2+=((val&0x0000FF00)<<8);
    val2+=((val&0x00FF0000)>>8);
    val2+=((val&0xFF000000)>>24);
    return val2;
}

void convert( unsigned long data_size, char *input_data, char *new_file_name )
{
    unsigned long a,records,nrec,table,off;
    unsigned long record[2]={0,0};

    //CREATING PDB FILE:
    f = fopen( (const char*)new_file_name, "wb" );
    records = ( data_size / 54000 ) + 1; //Records for data (each - 54000kb)
    fwrite( pdb_header, 1, 76, f ); //Save header
    nrec = MWORD( (unsigned short)records );
    fwrite( &nrec, 1, 2, f ); //Save number of records
    table = 8 * records; //Size of offset table

    off = 0;
    for( a = 0; a < records; a++ )
    {
	record[ 0 ] = MDWORD( 78 + table + off ); //record offset
	fwrite( record, 1, 8, f );
	off += 54000;
    }

    f2 = fopen( input_data, "rb" );
    for( a = 0; a < data_size; a++ )
    { //save data
	putc( getc(f2), f );
    }
    fclose(f2);
    fclose(f);
}

void make_output_name( char *str )
{
    int a;
    for( a = 0; a < 31; a++ )
    {
    	if( str[ a ] == 0 ) { pdb_header[a] = 0; break; }
	pdb_header[ a ] = str[ a ];
    }
    pdb_header[ 31 ] = 0;
}

