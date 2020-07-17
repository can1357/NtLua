#include "../logger.hpp"
#include "crt.h"

FILE* __cdecl __acrt_iob_func( unsigned i )
{
    if ( i == 1 ) return FILE_STDOUT;
    if ( i == 2 ) return FILE_STDERR;
    return 0;
}

FILE* freopen( const char* filename, const char* mode, FILE* stream )
{
    return 0;
}

size_t fwrite( const void* ptr, size_t size, size_t count, FILE* stream )
{
    if ( stream == FILE_STDOUT )
    {
        logger::logs.append( ptr, size * count );
        return count;
    }
    else if ( stream == FILE_STDERR )
    {
        logger::errors.append( ptr, size * count );
        return count;
    }
    return 0;
}

size_t fread( void* ptr, size_t size, size_t count, FILE* stream )
{
    return 0;
}

int getc( FILE* stream )
{
    return -1;
}

FILE* fopen( const char* filename, const char* mode )
{
    return 0;
}

int fflush( FILE* stream )
{
    return 0;
}

int ferror( FILE* stream )
{
    return 1;
}

int feof( FILE* stream )
{
    return 1;
}

int fclose( FILE* stream )
{
    return 1;
}

#include <stdarg.h>

extern "C" __declspec(dllimport) int _vsnprintf( char* dest, size_t count, const char* fmt, va_list args );

int fprintf( FILE* stream, const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );
    char buf[512] = "";
    auto n = _vsnprintf( buf, sizeof( buf ), fmt, args );
    if ( n >= sizeof(buf) )
    {
        // todo: allocate things
        __debugbreak();
    }
    va_end( args );
    fwrite( buf, n, 1, stream );
    return 1;
}