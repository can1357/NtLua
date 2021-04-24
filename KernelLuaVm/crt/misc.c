#include <intrin.h>
#include "crt.h"

void abort()
{
	__debugbreak();
}

char* getenv()
{
	return ( char* ) "virtual://";
}