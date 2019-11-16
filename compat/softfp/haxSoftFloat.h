
/*============================================================================

This C header file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016, 2017 The Regents of the
University of California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions, and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions, and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. Neither the name of the University nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#ifndef _HAXSOFTFLOAT
#define _HAXSOFTFLOAT

#ifndef HAX_FREESTANDING
#error SoftFloat IEEE emulation not found
#endif

/*
 * Compile Time Assert
 */

#ifdef __COUNTER__
#define HAX_CTASSERT(x)		HAX_CTASSERT0(x, __haxctassert, __COUNTER__)
#else
#define HAX_CTASSERT(x)		HAX_CTASSERT99(x, __INCLUDE_LEVEL__, __LINE__)
#define HAX_CTASSERT99(x, a, b)	HAX_CTASSERT0(x, __hax_ctassert ## a, _ ## b)
#endif
#define HAX_CTASSERT0(x, y, z)	HAX_CTASSERT1(x, y, z)
#define HAX_CTASSERT1(x, y, z)					\
    typedef struct {						\
        unsigned int y ## z : /*CONSTCOND*/(x) ? 1 : -1;	\
    } y ## z ## _hax_struct

/*
 * SoftFloat implementation assumptions
 */

HAX_CTASSERT(sizeof(long long int) == 8);

/*
 * Hax soft float/double
 */

typedef struct { unsigned int v; } Float;
typedef struct { unsigned long long int v; } Double;

HAX_CTASSERT(sizeof(Float) == 4);
HAX_CTASSERT(sizeof(Double) == 8);

/*
 * Integer-to-floating-point conversion routines.
 */

Double Hax_LongLongToDouble(long long int w);

/*
 * 32-bit (single-precision) floating-point operations.
 */

Double			Hax_FloatToDouble(Float f);

/*
 * 64-bit (double-precision) floating-point operations.
 */

long long int		Hax_DoubleToLongLong(Double f);
Float			Hax_DoubleToFloat(Double f);

Double			Hax_DoubleAdd(Double a, Double b);
Double			Hax_DoubleSub(Double a, Double b);
Double			Hax_DoubleMul(Double a, Double b);
Double			Hax_DoubleDiv(Double a, Double b);

int			Hax_DoubleEq(Double a, Double b);
int			Hax_DoubleLe(Double a, Double b);
int			Hax_DoubleLt(Double a, Double b);
int			Hax_DoubleNeq(Double a, Double b);
int			Hax_DoubleGe(Double a, Double b);
int			Hax_DoubleGt(Double a, Double b);

/*
 * Hax soft float/double commonly used constants
 */

#define HAX_DOUBLE_ZERO Hax_LongLongToDouble(0)
#define HAX_DOUBLE_ONE Hax_LongLongToDouble(1)
#define HAX_DOUBLE_TWO Hax_LongLongToDouble(2)
#define HAX_DOUBLE_MINUSONE Hax_LongLongToDouble(-1)

#endif
