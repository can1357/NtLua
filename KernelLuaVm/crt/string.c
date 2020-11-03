#pragma once
#include "crt.h"

int isalpha( int c )
{
	return ( 'a' <= c && c <= 'z' ) ||
		( 'A' <= c && c <= 'Z' );
}

int isdigit( int  c )
{
	return ( '0' <= c && c <= '9' );
}

int isalnum( int c )
{
	return isalpha( c ) || isdigit( c );
}

int iscntrl( int c )
{
	return c <= 0x1F || c == 0x7F;
}

int isgraph( int c )
{
	return 0x21 <= c && c <= 0x7E;
}

int ispunct( int c )
{
	for ( auto o : "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~" )
		if ( o == c ) return 1;
	return 0;
}

double strtod( const char* str, const char** endptr )
{
	double v = 0;
	if ( sscanf_s( str, "%lf", &v ) && endptr )
		*endptr = str + strlen( str );
	return v;
}

double atof( const char* str )
{
	const char* endptr = 0;
	return strtod( str, &endptr );
}

char* strpbrk( const char* s1, const char* s2 )
{
	while ( *s1 )
		if ( strchr( s2, *s1++ ) )
			return ( char* )--s1;
	return 0;
}

// haha locale go brrr
//
int strcoll( const char* a, const char* b )
{
	return strcmp( a, b );
}