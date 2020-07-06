#include "..\libmd.h"

int signbit(double x)
{
	union {
		double d;
		uint64_t i;
	} y = { x };
	return y.i>>63;
}


