#pragma once
#define _INC_STDIO
#include <intrin.h>
#include <setjmp.h>
#include <ntifs.h>
#include "stdint.h"
#include "libmd.h"

#define FILE_STDOUT ((FILE*)0x13370001)
#define FILE_STDERR ((FILE*)0x13370002)

// CRT initializers.
//
#pragma section(".CRT", read)
using fn_crt_initializer = void(*)();
__declspec( allocate( ".CRT" ) ) static const fn_crt_initializer crt_tracker = nullptr;

namespace crt
{
	static void initialize()
	{
		const fn_crt_initializer* entry = &crt_tracker;
		while ( *++entry )
			( *entry )( );
	}
};

// C++ memory decleration.
//
__declspec( restrict ) void* malloc( size_t n );
void free( void* p );
void* operator new( size_t, void* where );
void* operator new( size_t Size );
void* operator new[ ] ( size_t Size );
void operator delete( void* Adr );
void operator delete( void* Adr, size_t Size );
void operator delete[ ] ( void* Adr );
void operator delete[ ] ( void* Adr, size_t Size );

extern "C"
{
	// String utils.
	//
	int isalpha( int c );
	int isdigit( int  c );
	int isalnum( int c );
	int iscntrl( int c );
	int isgraph( int c );
	int ispunct( int c );
	__declspec( dllimport ) int sscanf_s( const char* buffer, const char* format, ... ); // @ ntoskrnl.lib
	__declspec( dllimport ) int sprintf_s( char* buffer, size_t sizeOfBuffer, const char* format, ... ); // @ ntoskrnl.lib
	double strtod( const char* str, const char** endptr );
	char* strpbrk( const char* s1, const char* s2 );
	int strcoll( const char* a, const char* b );

	// IO utils.
	//
    typedef struct _FILE {} FILE; // Maybe it will be implemented one day.

    FILE* __cdecl __acrt_iob_func( unsigned i );
    FILE* freopen( const char* filename, const char* mode, FILE* stream );
    size_t fwrite( const void* ptr, size_t size, size_t count, FILE* stream );
    size_t fread( void* ptr, size_t size, size_t count, FILE* stream );
    int getc( FILE* stream );
    FILE* fopen( const char* filename, const char* mode );
    int fflush( FILE* stream );
    int ferror( FILE* stream );
    int feof( FILE* stream );
    int fclose( FILE* stream );
    int fprintf( FILE* stream, const char* fmt, ... );

    // Misc. functions.
    //
	void abort();
    char* getenv();
};