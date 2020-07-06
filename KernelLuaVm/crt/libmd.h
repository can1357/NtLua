#pragma once
#include <math.h>
#include "stdint.h"

extern "C"
{
    extern char _fltused;

    typedef double double_t;
    typedef float float_t;

    #define FLT_EVAL_METHOD 0

    #ifndef _HUGE_ENUF
    #define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
    #endif

    #define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))
    #define HUGE_VAL   ((double)INFINITY)
    #define HUGE_VALF  ((float)INFINITY)
    #define HUGE_VALL  ((long double)INFINITY)
    #define NAN        ((float)(INFINITY * 0.0F))

    #define _DENORM    (-2)
    #define _FINITE    (-1)
    #define _INFCODE   1
    #define _NANCODE   2

    #define FP_INFINITE  _INFCODE
    #define FP_NAN       _NANCODE
    #define FP_NORMAL    _FINITE
    #define FP_SUBNORMAL _DENORM
    #define FP_ZERO      0

    #define isinf(x)  (fpclassify(x)==FP_INFINITE)
    #define isnan(x)  (fpclassify(x)==FP_NAN)
    #define isfinite(x) (fpclassify(x)!=FP_NAN && fpclassify(x)!=FP_INFINITE )
    #define isnormal(x)  (fpclassify(x)==FP_NORMAL)


    #define _C2          1  // 0 if not 2's complement
    #define FP_ILOGB0   (-0x7fffffff - _C2)
    #define FP_ILOGBNAN 0x7fffffff

    #define MATH_ERRNO        1
    #define MATH_ERREXCEPT    2
    #define math_errhandling  (MATH_ERRNO | MATH_ERREXCEPT)

    // Values for use as arguments to the _fperrraise function
    #define _FE_DIVBYZERO 0x04
    #define _FE_INEXACT   0x20
    #define _FE_INVALID   0x01
    #define _FE_OVERFLOW  0x08
    #define _FE_UNDERFLOW 0x10

    #define _D0_C  3 // little-endian, small long doubles
    #define _D1_C  2
    #define _D2_C  1
    #define _D3_C  0

    #define _DBIAS 0x3fe
    #define _DOFF  4

    #define _F0_C  1 // little-endian
    #define _F1_C  0

    #define _FBIAS 0x7e
    #define _FOFF  7
    #define _FRND  1

    #define _L0_C  3 // little-endian, 64-bit long doubles
    #define _L1_C  2
    #define _L2_C  1
    #define _L3_C  0

    #define _LBIAS 0x3fe
    #define _LOFF  4

    // IEEE 754 double properties
    #define _DFRAC  ((unsigned short)((1 << _DOFF) - 1))
    #define _DMASK  ((unsigned short)(0x7fff & ~_DFRAC))
    #define _DMAX   ((unsigned short)((1 << (15 - _DOFF)) - 1))
    #define _DSIGN  ((unsigned short)0x8000)

    // IEEE 754 float properties
    #define _FFRAC  ((unsigned short)((1 << _FOFF) - 1))
    #define _FMASK  ((unsigned short)(0x7fff & ~_FFRAC))
    #define _FMAX   ((unsigned short)((1 << (15 - _FOFF)) - 1))
    #define _FSIGN  ((unsigned short)0x8000)

    // IEEE 754 long double properties
    #define _LFRAC  ((unsigned short)(-1))
    #define _LMASK  ((unsigned short)0x7fff)
    #define _LMAX   ((unsigned short)0x7fff)
    #define _LSIGN  ((unsigned short)0x8000)

    #define _DHUGE_EXP (int)(_DMAX * 900L / 1000)
    #define _FHUGE_EXP (int)(_FMAX * 900L / 1000)
    #define _LHUGE_EXP (int)(_LMAX * 900L / 1000)

    #define _DSIGN_C(_Val)  (((_double_val *)(char*)&(_Val))->_Sh[_D0_C] & _DSIGN)
    #define _FSIGN_C(_Val)  (((_float_val  *)(char*)&(_Val))->_Sh[_F0_C] & _FSIGN)
    #define _LSIGN_C(_Val)  (((_ldouble_val*)(char*)&(_Val))->_Sh[_L0_C] & _LSIGN)


    #define FORCE_EVAL(x) do {                        \
	    if (sizeof(x) == sizeof(float)) {         \
		    volatile float __x;               \
		    __x = (x);                        \
                    (void)__x;                        \
	    } else if (sizeof(x) == sizeof(double)) { \
		    volatile double __x;              \
		    __x = (x);                        \
                    (void)__x;                        \
	    } else {                                  \
		    volatile long double __x;         \
		    __x = (x);                        \
                    (void)__x;                        \
	    }                                         \
    } while(0)

    /* Get two 32 bit ints from a double.  */
    #define EXTRACT_WORDS(hi,lo,d)                    \
    do {                                              \
      union {double f; uint64_t i;} __u;              \
      __u.f = (d);                                    \
      (hi) = __u.i >> 32;                             \
      (lo) = (uint32_t)__u.i;                         \
    } while (0)

    /* Get the more significant 32 bit int from a double.  */
    #define GET_HIGH_WORD(hi,d)                       \
    do {                                              \
      union {double f; uint64_t i;} __u;              \
      __u.f = (d);                                    \
      (hi) = __u.i >> 32;                             \
    } while (0)

    /* Get the less significant 32 bit int from a double.  */
    #define GET_LOW_WORD(lo,d)                        \
    do {                                              \
      union {double f; uint64_t i;} __u;              \
      __u.f = (d);                                    \
      (lo) = (uint32_t)__u.i;                         \
    } while (0)

    /* Set a double from two 32 bit ints.  */
    #define INSERT_WORDS(d,hi,lo)                     \
    do {                                              \
      union {double f; uint64_t i;} __u;              \
      __u.i = ((uint64_t)(hi)<<32) | (uint32_t)(lo);  \
      (d) = __u.f;                                    \
    } while (0)

    /* Set the more significant 32 bits of a double from an int.  */
    #define SET_HIGH_WORD(d,hi)                       \
    do {                                              \
      union {double f; uint64_t i;} __u;              \
      __u.f = (d);                                    \
      __u.i &= 0xffffffff;                            \
      __u.i |= (uint64_t)(hi) << 32;                  \
      (d) = __u.f;                                    \
    } while (0)

    /* Set the less significant 32 bits of a double from an int.  */
    #define SET_LOW_WORD(d,lo)                        \
    do {                                              \
      union {double f; uint64_t i;} __u;              \
      __u.f = (d);                                    \
      __u.i &= 0xffffffff00000000ull;                 \
      __u.i |= (uint32_t)(lo);                        \
      (d) = __u.f;                                    \
    } while (0)

    #define DBL_EPSILON 2.22044604925031308085e-16

    double trunc( double x );
    double tgamma( double x );
    double __lgamma_r( double x, int* sign );
    double sqrt( double x );
    double tanh( double x );
    double tan( double x );
    double sinh( double x );
    double sin( double x );
    double cosh( double x );
    double cos( double x );
    double atanh( double x );
    double atan( double x );
    double asinh( double x );
    double asin( double x );
    double acosh( double x );
    double acos( double x );
    double atan2( double y, double x );
    double scalbn( double x, int n );
    double rint( double x );
    double pow( double x, double y );
    double nearbyint( double x );
    double modf( double x, double* iptr );
    double log( double x );
    double log1p( double x );
    double log10( double x );
    double lgamma( double x );
    double ldexp( double x, int n );
    double frexp( double x, int* e );
    double fmod( double x, double y );
    double floor( double x );
    double expm1( double x );
    double exp( double x );
    double erf( double x );
    double copysign( double x, double y );
    double ceil( double x );
    double __tan( double x, double y, int odd );
    double __sin( double x, double y, int iy );
    int signbit( double x );
    int __rem_pio2_large( double* x, double* y, int e0, int nx, int prec );
    int __rem_pio2( double x, double* y );
    int fpclassify( double x );
    double __expo2( double x );
    double __cos( double x, double y );
    double fabs( double x );
};