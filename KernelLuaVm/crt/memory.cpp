#include "crt.h"

__declspec( restrict ) void* malloc( size_t n )
{
	return ExAllocatePool( NonPagedPool, n );
}

void free( void* p )
{
	if ( p ) ExFreePool( p );
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