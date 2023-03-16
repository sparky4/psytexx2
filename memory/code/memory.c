/*
    SunDog: memory.cpp. Multiplatform memory manager
    Copyright (C) 2002 - 2007  Zolotov Alexandr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//*** Contact info: Zolotov Alexandr (NightRadio project)
//***               Ekaterinburg. Russia.
//***               Email: nightradio@gmail.com
//***               WWW: warmplace.ru

#include "../../core/core.h"
#include "../../core/debug.h"
#include "../../filesystem/v3nus_fs.h"
#include "../memory.h"

//Special things for PalmOS:
#ifndef NONPALM
    #define memNewChunkFlagNonMovable    0x0200
    #define memNewChunkFlagAllowLarge    0x1000  // this is not in the sdk *g*
    SysAppInfoPtr ai1, ai2, appInfo;
    unsigned short ownID;
#endif

#ifdef NONPALM
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
#endif

#ifdef WINDOWS
    #include <windows.h>
#endif

void *dstart = 0;
void *sstart = 0;
void* prev_dblock = 0;  //Previous mem block in dynamic heap
void* prev_sblock = 0;  //Previous mem block in storage heap
int dsize = 0;
int ssize = 0;
int max_dsize = 0;
int max_ssize = 0;
long mem_manager_started = 0;

long get_block_value( void *ptr, long offset )
{
    char *p = (char*)ptr; p += offset;
    long *m = (long*)p;
    return m[0];
}


void set_block_value( void *ptr, long offset, long value )
{
    char *p = (char*)ptr; p += offset;
    long *m = (long*)p;
    m[0] = value;
}


void mem_free_all()
{
    char *ptr;
    char *next;
    long size;
    if( sstart )
    {
        prints( "MEMORY CLEANUP (STORAGE)" );
	ptr = (char*)sstart;
	for(;;)
	{
	    next = (char*)get_block_value( ptr + MEM_BLOCK_INFO, MEM_NEXT );
	    size = get_block_value( ptr + MEM_BLOCK_INFO, MEM_SIZE );
	    dprint( "FREE %d %s\n", size, ptr - MEM_NAME );
#ifndef NONPALM
	    MemPtrFree( ptr );
#else
	    free( ptr );
#endif
	    if( next == 0 ) break;
	    ptr = next;
	}
    }
    if( dstart )
    {
        prints( "MEMORY CLEANUP (DYNAMIC)" );
	ptr = (char*)dstart;
	for(;;)
	{
	    next = (char*)get_block_value( ptr + MEM_BLOCK_INFO, MEM_NEXT );
	    size = get_block_value( ptr + MEM_BLOCK_INFO, MEM_SIZE );
	    dprint( "FREE %d %s\n", size, ptr - MEM_NAME );
#ifndef NONPALM
	    MemPtrFree( ptr );
#else
	    free( ptr );
#endif
	    if( next == 0 ) break;
	    ptr = next;
	}
    }
    dprint( "Max dynamic memory used: %d\n", max_dsize );
    dprint( "Max storage memory used: %d\n", max_ssize );
    dprint( "%d %d\n", dsize, ssize );
    remove( "mem_storage" );
    remove( "mem_dynamic" );
}


//Main functions:
void* mem_new(unsigned long heap, unsigned long size, char *name, unsigned long id)
{
    unsigned long real_size = size;
    if( mem_manager_started == 0 )
    {
	mem_manager_started = 1;
#ifdef PALMOS
#ifndef NOSTORAGE
	FILE *f = fopen( "mem_storage", "rb" );
	if( f )
	{
		//0000 remove files for now until this program gets more stable --sparky4
		remove( "mem_storage" );
		remove( "mem_dynamic" );
/*	    prints( "MEMORY CLEANUP" );
	    char *ptr;
	    char *next;
	    long size;
	    fread( &ptr, 4, 1, f );
	    if( ptr )
	    for(;;)
	    {
		next = (char*)get_block_value( ptr + MEM_BLOCK_INFO, MEM_NEXT );
		size = get_block_value( ptr + MEM_BLOCK_INFO, MEM_SIZE );
		MemPtrFree( ptr );
		prints2( "FREE ", size );
		if( next == 0 ) break;
		ptr = next;
	    }
*/
	    fclose( f );
	}
#endif
#endif
    }

    void *retval;
    size += MEM_BLOCK_INFO; //Add structure with info to an our memory block
#ifndef PALMOS
    retval = (void*)malloc(size);
#else
    //PalmOS:
    #ifdef NOSTORAGE
    heap = HEAP_DYNAMIC;
    #endif
    retval = MemChunkNew( MemHeapID( 0, heap ), size, ownID | memNewChunkFlagNonMovable | memNewChunkFlagAllowLarge );
#endif
    //Save info about new memory block:
    long *m = (long*)retval;
    if( m )
    {
	mem_off();
	if( heap == HEAP_DYNAMIC )
	{
	    m[0] = (long)prev_dblock;
	    m[1] = 0;
	    if( prev_dblock == 0 )
	    {
		//It is the first block. Save address:
		FILE *f = fopen( "mem_dynamic", "wb" );
		if( f )
		{
		    fwrite( &retval, 4, 1, f );
		    fclose( f );
		}
		dstart = retval;
		prev_dblock = retval;
	    }
	    else
	    { //It is not the first block:
		long *prev = (long*)prev_dblock;
		prev[1] = (long)retval;
		prev_dblock = retval;
	    }
	}
	else
	{
	    m[0] = (long)prev_sblock;
	    m[1] = 0;
	    if( prev_sblock == 0 )
	    {
		//It is the first block. Save address:
		FILE *f = fopen( "mem_storage", "wb" );
		if( f )
		{
		    fwrite( &retval, 4, 1, f );
		    fclose( f );
		}
		sstart = retval;
		prev_sblock = retval;
	    }
	    else
	    { //It is not the first block:
		long *prev = (long*)prev_sblock;
		prev[1] = (long)retval;
		prev_sblock = retval;
	    }
	}
	m[2] = size - MEM_BLOCK_INFO;
	m[3] = heap + 123456;
	uchar *mname = (uchar*)&m[4];
	int np;
	for( np = 0; np < 15; np++ ) { mname[ np ] = name[ np ]; if( name[ np ] == 0 ) break; }
	mname[ 15 ] = 0;
        m += ( MEM_BLOCK_INFO >> 2 );
	mem_on();
    }
    if( !m )
    {
	prints2( "MEM ALLOC ERROR ", size ); prints( name );
#ifdef PALMOS
	prints( "####" );
	prints( "####" );
	prints( "####" );
#endif
    }
    else
    {
	if( heap == HEAP_DYNAMIC ) { dsize += real_size; if( dsize > max_dsize ) max_dsize = dsize; }
	else
	if( heap == HEAP_STORAGE ) { ssize += real_size; if( ssize > max_ssize ) max_ssize = ssize; }
    }
    return (void*)m;
}


void mem_free( void *ptr )
{
    if( ptr == 0 ) return;
    mem_off();
    if( mem_get_heap( ptr ) == HEAP_DYNAMIC ) dsize -= mem_get_size( ptr );
    else
    if( mem_get_heap( ptr ) == HEAP_STORAGE ) ssize -= mem_get_size( ptr );
    char *prev = (char*)get_block_value( ptr, MEM_PREV );
    char *next = (char*)get_block_value( ptr, MEM_NEXT );
    if( prev && next )
    {
	set_block_value( prev + MEM_BLOCK_INFO, MEM_NEXT, (long)next );
	set_block_value( next + MEM_BLOCK_INFO, MEM_PREV, (long)prev );
    }
    if( prev && next == 0 )
    {
	set_block_value( prev + MEM_BLOCK_INFO, MEM_NEXT, 0 );
	if( mem_get_heap( ptr ) == HEAP_DYNAMIC ) prev_dblock = prev; else prev_sblock = prev;
    }
    if( prev == 0 && next )
    {
	set_block_value( next + MEM_BLOCK_INFO, MEM_PREV, 0 );
	FILE *f;
	if( mem_get_heap( ptr ) == HEAP_DYNAMIC )
	{
	    f = fopen( "mem_dynamic", "wb" );
	    if( f )
	    {
		fwrite( &next, 4, 1, f );
		fclose( f );
	    }
	    dstart = (void*)next;
	}
	else
	{
	    f = fopen( "mem_storage", "wb" );
	    if( f )
	    {
		fwrite( &next, 4, 1, f );
		fclose( f );
	    }
	    sstart = (void*)next;
	}
    }
    if( prev == 0 && next == 0 )
    {
	if( mem_get_heap( ptr ) == HEAP_DYNAMIC )
	{
	    prev_dblock = 0;
	    remove( "mem_dynamic" );
	    dstart = 0;
	}
	else
	{
	    prev_sblock = 0;
	    remove( "mem_storage" );
	    sstart = 0;
	}
    }
    mem_on();
    char *p = (char*)ptr; p -= MEM_BLOCK_INFO;
    ptr = (void*)p;
#ifdef NONPALM
    free(ptr);
#else
    MemPtrFree(ptr);
#endif
}


void mem_set( void *ptr, unsigned long size, unsigned char value )
{
    if( ptr == 0 ) return;
#ifdef NONPALM
    memset(ptr,value,size);
#else
    MemSet(ptr,size,value);
#endif
}

void* mem_resize( void *ptr, int new_size )
{
	int a;
//#ifdef NONPALM
//    realloc( ptr, size );
//    return ptr;
//#else
    if( ptr == 0 )
    {
	return MEM_NEW( HEAP_DYNAMIC, new_size );
    }
    mem_off();
    int old_size = mem_get_size( ptr );
    void *new_mem = mem_new( mem_get_heap( ptr ), new_size, "resized block", 0 );
    uchar *c = (uchar*)new_mem;
    if( old_size > new_size )
	mem_copy( new_mem, ptr, new_size );
    else
	mem_copy( new_mem, ptr, old_size );
    mem_free( ptr );
    if( old_size < new_size )
    for( a = old_size; a < new_size; a++ )
    {
	c[ a ] = 0;
    }
    mem_on();
    return new_mem;
//#endif
}

void mem_copy( void *dest, const void *src, unsigned long size )
{
    if( dest == 0 || src == 0 ) return;
#ifdef NONPALM
    memcpy( dest, src, size );
#else
    MemMove( dest, src, size ); //It's for dinamic heap only!!
#endif
}

int mem_cmp( const char *p1, const char *p2, unsigned long size )
{
    if( p1 == 0 || p2 == 0 ) return 0;
#ifdef NONPALM
    return memcmp( p1, p2, size );
#else
    return MemCmp( p1, p2, size ); //It's for dinamic heap only!!
#endif
}

void mem_strcat( char *dest, const char* src )
{
    if( dest == 0 || src == 0 ) return;
#ifndef NONPALM
    StrCat( dest, src );
#else
    strcat( dest, src );
#endif
}

long mem_strcmp( const char *s1, const char *s2 )
{
#ifndef NONPALM
    return StrCompare( s1, s2 );
#else
    return strcmp( s1, s2 );
#endif
}

int mem_strlen( const char *s )
{
    if( s == 0 ) return 0;
    int a;
    for( a = 0;; a++ ) if( s[ a ] == 0 ) break;
    return a;
}

long mem_strcpy( char *s1, const char *s2 )
{
#ifndef NONPALM
    return (long)*StrCopy( s1, s2 );
#else
    return (long)strcpy( s1, s2 );
#endif
}

char *mem_strdup( const char *s1 )
{
    int len = mem_strlen( s1 );
    char *newstr = (char*)MEM_NEW( HEAP_DYNAMIC, len + 1 );
    mem_copy( newstr, s1, len + 1 );
    return newstr;
}

long mem_get_heap( void *ptr )
{
    if( ptr == 0 ) return 0;
#ifndef NOSTORAGE
    //if( (long)ptr & 3 ) return HEAP_DYNAMIC;
    char *p = (char*)ptr; p += MEM_HEAP;
    long *m = (long*)p;
    return m[0] - 123456;
#else
    return HEAP_DYNAMIC;
#endif
}

long mem_get_size( void *ptr )
{
    if( ptr == 0 ) return 0;
    char *p = (char*)ptr; p += MEM_SIZE;
    long *m = (long*)p;
    return m[0];
}

char *mem_get_name( void *ptr )
{
    if( ptr == 0 ) return 0;
    char *p = (char*)ptr; p += MEM_NAME;
    return p;
}

//0000
//memory info
char * mem_info_max_ssize ()
{
	static char s[64],dest[64];
	mem_strcpy(dest, "Max smem used: ");
	int_to_string( max_ssize, s );
	mem_strcat(dest, s);
	return dest;
}

char * mem_info_max_dsize ()
{
	static char s[64],dest[64];
	mem_strcpy(dest, "Max dmem used: ");
	int_to_string( max_dsize, s );
	mem_strcat(dest, s);
	return dest;
}

char * mem_info_ssize ()
{
	static char s[64],dest[64];
	mem_strcpy(dest, "smem:          ");
	int_to_string( ssize, s );
	mem_strcat(dest, s);
	return dest;
}

char * mem_info_dsize ()
{
	static char s[64],dest[64];
	mem_strcpy(dest, "dmem:          ");
	int_to_string( dsize, s );
	mem_strcat(dest, s);
	return dest;
}


int off_count = 0;
void mem_on(void)
{
#ifndef NONPALM
#ifndef NOSTORAGE
    off_count--;
    if( off_count == 0 )
	MemSemaphoreRelease(1);
#endif
#endif
}

void mem_off(void)
{
#ifndef NONPALM
#ifndef NOSTORAGE
    if( off_count == 0 )
	MemSemaphoreReserve(1);
    off_count++;
#endif
#endif
}

void mem_palm_normal_mode(void)
{
#ifndef NONPALM
#ifndef NOSTORAGE
    if( off_count > 0 )
    { //At the moment mem protection is off:
	MemSemaphoreRelease(1); //mem protection on
    }
#endif
#endif
}

void mem_palm_our_mode(void)
{
#ifndef NONPALM
#ifndef NOSTORAGE
    if( off_count > 0 )
    {
	MemSemaphoreReserve(1); //mem protection off
    }
#endif
#endif
}

//Only for PalmOS:
#ifndef NONPALM
void *malloc ( int size )
{
    return MEM_NEW( HEAP_DYNAMIC, size );
}

void *realloc ( void * ptr, int size )
{
    return mem_resize( ptr, size );
}

void free ( void * ptr )
{
    mem_free( ptr );
}

void *memcpy ( void * destination, const void * source, int num )
{
    mem_copy( destination, source, num );
}

int strcmp ( const char * str1, const char * str2 )
{
    return mem_strcmp( str1, str2 );
}

int memcmp ( const char * p1, const char * p2, int size )
{
    return mem_cmp( p1, p2, size );
}

int strlen ( const char * str1 )
{
    return mem_strlen( str1 );
}

int strcpy ( char * str1, const char * str2 )
{
    return mem_strcpy( str1 , str2);
}

#endif
