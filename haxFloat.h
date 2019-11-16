/*
 * haxFloat.h --
 *
 *	This header file maps floating point operations into native
 *	toolchain support for floats.
 *
 * Copyright 2019 Kamil Rytarowski
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 */

#ifndef _HAXFLOAT
#define _HAXFLOAT

#ifndef _HAX
#include <hax.h>
#endif

#ifdef HAX_FREESTANDING
#include "compat/softfp/haxSoftFloat.h"
#else

/*
 * Hax float/double
 */
typedef float Float;
typedef double Double;

/*
 * Integer-to-floating-point conversion routines.
 */

static inline Double
Hax_LongLongToDouble(long long int w)
{
    return (Double) w;
}

/*
 * 32-bit (single-precision) floating-point operations.
 */

static inline Double
Hax_FloatToDouble(Float f)
{
    return (Double)f;
}

Float hax_strtof( const char *, char ** );
int hax_stof( Float, const char *, char *, unsigned int );

/*
 * 64-bit (double-precision) floating-point operations.
 */

static inline long long int
Hax_DoubleToLongLong(Double f)
{
    return (long long int)f;
}

static inline Float
Hax_DoubleToFloat(Double f)
{
    return (Float)f;
}

static inline Double
Hax_DoubleAdd(Double f1, Double f2)
{
    return f1 + f2;
}

static inline Double
Hax_DoubleSub(Double f1, Double f2)
{
    return f1 - f2;
}

static inline Double
Hax_DoubleMul(Double f1, Double f2)
{
    return f1 * f2;
}

static inline Double
Hax_DoubleDiv(Double f1, Double f2)
{
    return f1 / f2;
}

static inline int
Hax_DoubleEq(Double f1, Double f2)
{
    return f1 == f2;
}

static inline int
Hax_DoubleLe(Double f1, Double f2)
{
    return f1 <= f2;
}

static inline int
Hax_DoubleLt(Double f1, Double f2)
{
    return f1 < f2;
}

static inline int
Hax_DoubleNeq(Double f1, Double f2)
{
    return f1 != f2;
}

static inline int
Hax_DoubleGe(Double f1, Double f2)
{
    return f1 >= f2;
}

static inline int
Hax_DoubleGt(Double f1, Double f2)
{
    return f1 > f2;
}

/*
 * Hax soft float/double commonly used constants
 */

#define HAX_DOUBLE_ZERO 0.0
#define HAX_DOUBLE_ONE 1.0
#define HAX_DOUBLE_TWO 2.0
#define HAX_DOUBLE_MINUSONE -1.0

#endif /* HAX_FREESTANDING */

#endif /* _HAXFLOAT */
