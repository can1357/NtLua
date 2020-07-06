#include "..\libmd.h"

typedef union
{
    double d;
    struct
    {
        uint64_t m : 52;
        uint64_t e : 11;
        uint64_t s : 1;
    };
} double_s_t;

double fabs( double x )
{
    double_s_t dx = { x };
    dx.s = 0;
    return dx.d;
}