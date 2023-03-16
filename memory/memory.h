#ifndef __MEMORY__
#define __MEMORY__

enum
{
    HEAP_DYNAMIC = 0,
    HEAP_STORAGE
};

#define MEM_BLOCK_INFO   32
#define MEM_NAME        -16
#define MEM_HEAP        -20
#define MEM_SIZE        -24
#define MEM_NEXT        -28
#define MEM_PREV        -32

#define MEM_NEW( heap, size ) mem_new( heap, size, (char*)__FUNCTION__, 0 )

void int_to_string( int value, char *str ) sec4;

//mem_new:
//heap - heap number:
//0 = dynamic heap for small blocks
//1 = storage heap for large static blocks
//size - block size
void mem_free_all();
void* mem_new( unsigned long heap, unsigned long size, char *name, unsigned long id ); //Each memory block has own name and ID
void mem_free( void *ptr );
void mem_set( void *ptr, unsigned long size, unsigned char value );
void* mem_resize( void *ptr, int size );
void mem_copy( void *dest, const void *src, unsigned long size );
int mem_cmp( const char *p1, const char *p2, unsigned long size );
void mem_strcat( char *dest, const char *src );
long mem_strcmp( const char *s1, const char *s2 );
int mem_strlen( const char *s );
char *mem_strdup( const char *s1 );

//Get info about memory block:
long mem_get_heap( void *ptr );
long mem_get_size( void *ptr );
char *mem_get_name( void *ptr );

//0000
//memory info
char * mem_info_max_ssize ();
char * mem_info_max_dsize ();
char * mem_info_ssize ();
char * mem_info_dsize ();

//Palm specific:
void mem_on(void);  //Storage protection ON
void mem_off(void); //Storage protection OFF
void mem_palm_normal_mode(void); //Switch to normal mode (Storage protection ON)
void mem_palm_our_mode(void);    //Switch back to our mode (Storage protection is ON or OFF)

#ifdef PALMOS
//posix compatibility for PalmOS devices:
#include <PalmOS.h>

extern SysAppInfoPtr ai1, ai2, appInfo;
extern unsigned short ownID;
void *malloc ( int size );
void *realloc ( void * ptr, int size );
void free ( void * ptr );
void *memcpy ( void * destination, const void * source, int num );
int strcmp ( const char * str1, const char * str2 );
int memcmp ( const char * p1, const char * p2, int size );
int strlen ( const char * str1 );
int strcpy ( char * str1, const char * str2 );
#endif

#endif
