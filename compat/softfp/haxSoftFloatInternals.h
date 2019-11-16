
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


#ifndef HAX_SOFTFLOAT_INTERNALS_H
#define HAX_SOFTFLOAT_INTERNALS_H

#include "haxSoftFloatPrimitives.h"
#include "haxSoftFloatTypes.h"

/*----------------------------------------------------------------------------
| Software floating-point underflow tininess-detection mode.
*----------------------------------------------------------------------------*/
extern HAX_THREAD_LOCAL Hax_uint_fast8_t Hax_softfloat_detectTininess;
enum {
    Hax_softfloat_tininess_beforeRounding = 0,
    Hax_softfloat_tininess_afterRounding  = 1
};

/*----------------------------------------------------------------------------
| Software floating-point rounding mode.  (Mode "odd" is supported only if
| SoftFloat is compiled with macro 'SOFTFLOAT_ROUND_ODD' defined.)
*----------------------------------------------------------------------------*/
extern HAX_THREAD_LOCAL Hax_uint_fast8_t Hax_softfloat_roundingMode;
enum {
    Hax_softfloat_round_near_even   = 0,
    Hax_softfloat_round_minMag      = 1,
    Hax_softfloat_round_min         = 2,
    Hax_softfloat_round_max         = 3,
    Hax_softfloat_round_near_maxMag = 4,
    Hax_softfloat_round_odd         = 6
};

/*----------------------------------------------------------------------------
| Software floating-point exception flags.
*----------------------------------------------------------------------------*/
extern HAX_THREAD_LOCAL Hax_uint_fast8_t Hax_softfloat_exceptionFlags;
enum {
    Hax_softfloat_flag_inexact   =  1,
    Hax_softfloat_flag_underflow =  2,
    Hax_softfloat_flag_overflow  =  4,
    Hax_softfloat_flag_infinite  =  8,
    Hax_softfloat_flag_invalid   = 16
};

/*----------------------------------------------------------------------------
| Routine to raise any or all of the software floating-point exception flags.
*----------------------------------------------------------------------------*/
void Hax_softfloat_raiseFlags( Hax_uint_fast8_t );

/*----------------------------------------------------------------------------
| Misc
*----------------------------------------------------------------------------*/

union Hax_ui16_f16 { Hax_uint16_t ui; Hax_float16_t f; };
union Hax_ui32_f32 { Hax_uint32_t ui; Hax_float32_t f; };
union Hax_ui64_f64 { Hax_uint64_t ui; Hax_float64_t f; };
 
#ifdef SOFTFLOAT_FAST_INT64
union Hax_extF80M_extF80 { struct Hax_extFloat80M fM; Hax_extFloat80_t f; };
union Hax_ui128_f128 { struct Hax_uint128 ui; Hax_float128_t f; };
#endif

enum {
    Hax_softfloat_mulAdd_subC    = 1,
    Hax_softfloat_mulAdd_subProd = 2
};

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/
Hax_uint_fast32_t Hax_softfloat_roundToUI32( Hax_bool, Hax_uint_fast64_t, Hax_uint_fast8_t, Hax_bool );

#ifdef SOFTFLOAT_FAST_INT64
Hax_uint_fast64_t
 Hax_softfloat_roundToUI64(
     Hax_bool, Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast8_t, Hax_bool );
#else
Hax_uint_fast64_t Hax_softfloat_roundMToUI64( Hax_bool, Hax_uint32_t *, Hax_uint_fast8_t, Hax_bool );
#endif

Hax_int_fast32_t Hax_softfloat_roundToI32( Hax_bool, Hax_uint_fast64_t, Hax_uint_fast8_t, Hax_bool );

#ifdef SOFTFLOAT_FAST_INT64
Hax_int_fast64_t
 Hax_softfloat_roundToI64(
     Hax_bool, Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast8_t, Hax_bool );
#else
Hax_int_fast64_t Hax_softfloat_roundMToI64( Hax_bool, Hax_uint32_t *, Hax_uint_fast8_t, Hax_bool );
#endif

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/
#define Hax_signF16UI( a ) ((Hax_bool) ((Hax_uint16_t) (a)>>15))
#define Hax_expF16UI( a ) ((Hax_int_fast8_t) ((a)>>10) & 0x1F)
#define Hax_fracF16UI( a ) ((a) & 0x03FF)
#define Hax_packToF16UI( sign, exp, sig ) (((Hax_uint16_t) (sign)<<15) + ((Hax_uint16_t) (exp)<<10) + (sig))

#define Hax_isNaNF16UI( a ) (((~(a) & 0x7C00) == 0) && ((a) & 0x03FF))

struct Hax_exp8_sig16 { Hax_int_fast8_t exp; Hax_uint_fast16_t sig; };
struct Hax_exp8_sig16 Hax_softfloat_normSubnormalF16Sig( Hax_uint_fast16_t );

Hax_float16_t Hax_softfloat_roundPackToF16( Hax_bool, Hax_int_fast16_t, Hax_uint_fast16_t );
Hax_float16_t Hax_softfloat_normRoundPackToF16( Hax_bool, Hax_int_fast16_t, Hax_uint_fast16_t );

Hax_float16_t Hax_softfloat_addMagsF16( Hax_uint_fast16_t, Hax_uint_fast16_t );
Hax_float16_t Hax_softfloat_subMagsF16( Hax_uint_fast16_t, Hax_uint_fast16_t );
Hax_float16_t
 Hax_softfloat_mulAddF16(
     Hax_uint_fast16_t, Hax_uint_fast16_t, Hax_uint_fast16_t, Hax_uint_fast8_t );

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/
#define Hax_signF32UI( a ) ((Hax_bool) ((Hax_uint32_t) (a)>>31))
#define Hax_expF32UI( a ) ((Hax_int_fast16_t) ((a)>>23) & 0xFF)
#define Hax_fracF32UI( a ) ((a) & 0x007FFFFF)
#define Hax_packToF32UI( sign, exp, sig ) (((Hax_uint32_t) (sign)<<31) + ((Hax_uint32_t) (exp)<<23) + (sig))

#define Hax_isNaNF32UI( a ) (((~(a) & 0x7F800000) == 0) && ((a) & 0x007FFFFF))

struct Hax_exp16_sig32 { Hax_int_fast16_t exp; Hax_uint_fast32_t sig; };
struct Hax_exp16_sig32 Hax_softfloat_normSubnormalF32Sig( Hax_uint_fast32_t );

Hax_float32_t Hax_softfloat_roundPackToF32( Hax_bool, Hax_int_fast16_t, Hax_uint_fast32_t );
Hax_float32_t Hax_softfloat_normRoundPackToF32( Hax_bool, Hax_int_fast16_t, Hax_uint_fast32_t );

Hax_float32_t Hax_softfloat_addMagsF32( Hax_uint_fast32_t, Hax_uint_fast32_t );
Hax_float32_t Hax_softfloat_subMagsF32( Hax_uint_fast32_t, Hax_uint_fast32_t );
Hax_float32_t
 Hax_softfloat_mulAddF32(
     Hax_uint_fast32_t, Hax_uint_fast32_t, Hax_uint_fast32_t, Hax_uint_fast8_t );

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/
#define Hax_signF64UI( a ) ((Hax_bool) ((Hax_uint64_t) (a)>>63))
#define Hax_expF64UI( a ) ((Hax_int_fast16_t) ((a)>>52) & 0x7FF)
#define Hax_fracF64UI( a ) ((a) & HAX_UINT64_C( 0x000FFFFFFFFFFFFF ))
#define Hax_packToF64UI( sign, exp, sig ) ((Hax_uint64_t) (((Hax_uint_fast64_t) (sign)<<63) + ((Hax_uint_fast64_t) (exp)<<52) + (sig)))

#define Hax_isNaNF64UI( a ) (((~(a) & HAX_UINT64_C( 0x7FF0000000000000 )) == 0) && ((a) & HAX_UINT64_C( 0x000FFFFFFFFFFFFF )))

struct Hax_exp16_sig64 { Hax_int_fast16_t exp; Hax_uint_fast64_t sig; };
struct Hax_exp16_sig64 Hax_softfloat_normSubnormalF64Sig( Hax_uint_fast64_t );

Hax_float64_t Hax_softfloat_roundPackToF64( Hax_bool, Hax_int_fast16_t, Hax_uint_fast64_t );
Hax_float64_t Hax_softfloat_normRoundPackToF64( Hax_bool, Hax_int_fast16_t, Hax_uint_fast64_t );

Hax_float64_t Hax_softfloat_addMagsF64( Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_bool );
Hax_float64_t Hax_softfloat_subMagsF64( Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_bool );
Hax_float64_t
 Hax_softfloat_mulAddF64(
     Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast8_t );

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/
#define Hax_signExtF80UI64( a64 ) ((Hax_bool) ((Hax_uint16_t) (a64)>>15))
#define Hax_expExtF80UI64( a64 ) ((a64) & 0x7FFF)
#define Hax_packToExtF80UI64( sign, exp ) ((Hax_uint_fast16_t) (sign)<<15 | (exp))

#define Hax_isNaNExtF80UI( a64, a0 ) ((((a64) & 0x7FFF) == 0x7FFF) && ((a0) & HAX_UINT64_C( 0x7FFFFFFFFFFFFFFF )))

#ifdef SOFTFLOAT_FAST_INT64

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/

struct Hax_exp32_sig64 { Hax_int_fast32_t exp; Hax_uint64_t sig; };
struct Hax_exp32_sig64 Hax_softfloat_normSubnormalExtF80Sig( Hax_uint_fast64_t );

Hax_extFloat80_t
 Hax_softfloat_roundPackToExtF80(
     Hax_bool, int_fast32_t, Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast8_t );
Hax_extFloat80_t
 Hax_softfloat_normRoundPackToExtF80(
     Hax_bool, int_fast32_t, Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast8_t );

Hax_extFloat80_t
 Hax_softfloat_addMagsExtF80(
     Hax_uint_fast16_t, Hax_uint_fast64_t, Hax_uint_fast16_t, Hax_uint_fast64_t, Hax_bool );
Hax_extFloat80_t
 Hax_softfloat_subMagsExtF80(
     Hax_uint_fast16_t, Hax_uint_fast64_t, Hax_uint_fast16_t, Hax_uint_fast64_t, Hax_bool );

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/
#define Hax_signF128UI64( a64 ) ((Hax_bool) ((Hax_uint64_t) (a64)>>63))
#define Hax_expF128UI64( a64 ) ((Hax_int_fast32_t) ((a64)>>48) & 0x7FFF)
#define Hax_fracF128UI64( a64 ) ((a64) & HAX_UINT64_C( 0x0000FFFFFFFFFFFF ))
#define Hax_packToF128UI64( sign, exp, sig64 ) (((Hax_uint_fast64_t) (sign)<<63) + ((Hax_uint_fast64_t) (exp)<<48) + (sig64))

#define Hax_isNaNF128UI( a64, a0 ) (((~(a64) & HAX_UINT64_C( 0x7FFF000000000000 )) == 0) && (a0 || ((a64) & HAX_UINT64_C( 0x0000FFFFFFFFFFFF ))))

struct Hax_exp32_sig128 { Hax_int_fast32_t exp; struct Hax_uint128 sig; };
struct Hax_exp32_sig128
 Hax_softfloat_normSubnormalF128Sig( Hax_uint_fast64_t, Hax_uint_fast64_t );

Hax_float128_t
 Hax_softfloat_roundPackToF128(
     Hax_bool, Hax_int_fast32_t, Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast64_t );
Hax_float128_t
 Hax_softfloat_normRoundPackToF128(
     Hax_bool, Hax_int_fast32_t, Hax_uint_fast64_t, Hax_uint_fast64_t );

Hax_float128_t
 Hax_softfloat_addMagsF128(
     Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_bool );
Hax_float128_t
 Hax_softfloat_subMagsF128(
     Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_uint_fast64_t, Hax_bool );
Hax_float128_t
 Hax_softfloat_mulAddF128(
     Hax_uint_fast64_t,
     Hax_uint_fast64_t,
     Hax_uint_fast64_t,
     Hax_uint_fast64_t,
     Hax_uint_fast64_t,
     Hax_uint_fast64_t,
     Hax_uint_fast8_t
 );

#else

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/

Hax_bool
 Hax_softfloat_tryPropagateNaNExtF80M(
     const struct Hax_extFloat80M *,
     const struct Hax_extFloat80M *,
     struct Hax_extFloat80M *
 );
void Hax_softfloat_invalidExtF80M( struct Hax_extFloat80M * );

int Hax_softfloat_normExtF80SigM( Hax_uint64_t * );

void
 Hax_softfloat_roundPackMToExtF80M(
     Hax_bool, Hax_int32_t, Hax_uint32_t *, Hax_uint_fast8_t, struct Hax_extFloat80M * );
void
 Hax_softfloat_normRoundPackMToExtF80M(
     Hax_bool, Hax_int32_t, Hax_uint32_t *, Hax_uint_fast8_t, struct Hax_extFloat80M * );

void
 Hax_softfloat_addExtF80M(
     const struct Hax_extFloat80M *,
     const struct Hax_extFloat80M *,
     struct Hax_extFloat80M *,
     Hax_bool
 );

int
 Hax_softfloat_compareNonnormExtF80M(
     const struct Hax_extFloat80M *, const struct Hax_extFloat80M * );

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/
#define Hax_signF128UI96( a96 ) ((Hax_bool) ((Hax_uint32_t) (a96)>>31))
#define Hax_expF128UI96( a96 ) ((Hax_int32_t) ((a96)>>16) & 0x7FFF)
#define Hax_fracF128UI96( a96 ) ((a96) & 0x0000FFFF)
#define Hax_packToF128UI96( sign, exp, sig96 ) (((Hax_uint32_t) (sign)<<31) + ((Hax_uint32_t) (exp)<<16) + (sig96))

Hax_bool Hax_softfloat_isNaNF128M( const Hax_uint32_t * );

Hax_bool
 Hax_softfloat_tryPropagateNaNF128M(
     const Hax_uint32_t *, const Hax_uint32_t *, Hax_uint32_t * );
void Hax_softfloat_invalidF128M( Hax_uint32_t * );

int Hax_softfloat_shiftNormSigF128M( const Hax_uint32_t *, Hax_uint_fast8_t, Hax_uint32_t * );

void Hax_softfloat_roundPackMToF128M( Hax_bool, Hax_int32_t, Hax_uint32_t *, Hax_uint32_t * );
void Hax_softfloat_normRoundPackMToF128M( Hax_bool, Hax_int32_t, Hax_uint32_t *, Hax_uint32_t * );

void
 Hax_softfloat_addF128M( const Hax_uint32_t *, const Hax_uint32_t *, Hax_uint32_t *, Hax_bool );
void
 Hax_softfloat_mulAddF128M(
     const Hax_uint32_t *,
     const Hax_uint32_t *,
     const Hax_uint32_t *,
     Hax_uint32_t *,
     Hax_uint_fast8_t
 );

#endif

#endif
