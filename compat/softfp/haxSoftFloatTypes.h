
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

#ifndef JIM_SOFTFLOAT_TYPES_H
#define JIM_SOFTFLOAT_TYPES_H

#include "haxSoftFloat.h"

/*----------------------------------------------------------------------------
| Auxiliary types.
*----------------------------------------------------------------------------*/

typedef unsigned char Hax_uint8_t;
typedef unsigned short Hax_uint16_t;
typedef unsigned int Hax_uint32_t;
typedef unsigned long long Hax_uint64_t;

typedef signed char Hax_int8_t;
typedef signed short Hax_int16_t;
typedef signed int Hax_int32_t;
typedef signed long long Hax_int64_t;

HAX_CTASSERT(sizeof(Hax_uint8_t) == 1);
HAX_CTASSERT(sizeof(Hax_uint16_t) == 2);
HAX_CTASSERT(sizeof(Hax_uint32_t) == 4);
HAX_CTASSERT(sizeof(Hax_uint64_t) == 8);

HAX_CTASSERT(sizeof(Hax_int8_t) == 1);
HAX_CTASSERT(sizeof(Hax_int16_t) == 2);
HAX_CTASSERT(sizeof(Hax_int32_t) == 4);
HAX_CTASSERT(sizeof(Hax_int64_t) == 8);

/*----------------------------------------------------------------------------
| Compat types
*----------------------------------------------------------------------------*/

typedef unsigned char Hax_uint_fast8_t;
typedef unsigned short Hax_uint_fast16_t;
typedef unsigned int Hax_uint_fast32_t;
typedef unsigned long long Hax_uint_fast64_t;

typedef signed char Hax_int_fast8_t;
typedef signed short Hax_int_fast16_t;
typedef signed int Hax_int_fast32_t;
typedef signed long long Hax_int_fast64_t;

typedef unsigned char Hax_uint_least8_t;
typedef unsigned short Hax_uint_least16_t;
typedef unsigned int Hax_uint_least32_t;
typedef unsigned long long Hax_uint_least64_t;

typedef signed char Hax_int_least8_t;
typedef signed short Hax_int_least16_t;
typedef signed int Hax_int_least32_t;
typedef signed long long Hax_int_least64_t;

typedef int Hax_bool;

typedef struct { unsigned short v; } Hax_float16_t;
typedef Float Hax_float32_t;
typedef Double Hax_float64_t;
typedef struct { Hax_uint64_t v[2]; } Hax_float128_t;

HAX_CTASSERT(sizeof(Hax_float16_t) == 2);
HAX_CTASSERT(sizeof(Hax_float32_t) == 4);
HAX_CTASSERT(sizeof(Hax_float64_t) == 8);
HAX_CTASSERT(sizeof(Hax_float128_t) == 16);

struct Hax_extFloat80M { Hax_uint64_t signif; Hax_uint16_t signExp; };

/*----------------------------------------------------------------------------
| Compat macros
*----------------------------------------------------------------------------*/

#define HAX_INT8_C(c)       c
#define HAX_INT16_C(c)      c
#define HAX_INT32_C(c)      c
#define HAX_INT64_C(c)      c##LL

#define HAX_UINT8_C(c)      c
#define HAX_UINT16_C(c)     c
#define HAX_UINT32_C(c)     c
#define HAX_UINT64_C(c)     c##ULL

#define HAX_INTMAX_C(c)     c##L
#define HAX_UINTMAX_C(c)    c##UL

HAX_CTASSERT(sizeof(long) == sizeof(void *));
HAX_CTASSERT(sizeof(unsigned long) == sizeof(void *));

#define HAX_INLINE static inline
#define HAX_THREAD_LOCAL

#define INLINE_LEVEL 5

/*----------------------------------------------------------------------------
| These macros are used to isolate the differences in word order between big-
| endian and little-endian platforms.
*----------------------------------------------------------------------------*/

#define Hax_wordIncr 1
#define Hax_indexWord( total, n ) (n)
#define Hax_indexWordHi( total ) ((total) - 1)
#define Hax_indexWordLo( total ) 0
#define Hax_indexMultiword( total, m, n ) (n)
#define Hax_indexMultiwordHi( total, n ) ((total) - (n))
#define Hax_indexMultiwordLo( total, n ) 0
#define Hax_indexMultiwordHiBut( total, n ) (n)
#define Hax_indexMultiwordLoBut( total, n ) 0
#define HAX_INIT_UINTM4( v3, v2, v1, v0 ) { v0, v1, v2, v3 }

#endif
