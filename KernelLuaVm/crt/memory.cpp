#include "crt.h"

__declspec( restrict ) void* malloc( size_t n )
{
	return ExAllocatePoolWithTag( NonPagedPool, n, 'NLUA' );
}

void free( void* p )
{
	if ( p ) ExFreePoolWithTag( p, 'NLUA' );
}

void* __cdecl operator new( size_t n )
{
	return malloc( n );
}

void* operator new[ ] ( size_t n )
{
	return malloc( n );
}

void __cdecl operator delete[ ] ( void* p )
{
	return free( p );
}

void __cdecl operator delete( void* p, size_t n )
{
	return free( p );
}

void __cdecl operator delete[ ] ( void* p, size_t n )
{
	return free( p );
}

void __cdecl operator delete( void* p )
{
	return free( p );
}

void* operator new( size_t, void* where )
{
	return where;
}