
/*============================================================================

This C header file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016, 2018 The Regents of the
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

#ifndef HAX_SOFTFLOAT_SPECIALIZE_H
#define HAX_SOFTFLOAT_SPECIALIZE_H

#include "haxSoftFloatPrimitives.h"
#include "haxSoftFloatTypes.h"

/*----------------------------------------------------------------------------
| Default value for 'Hax_softfloat_detectTininess'.
*----------------------------------------------------------------------------*/
#define Hax_init_detectTininess Hax_softfloat_tininess_afterRounding

/*----------------------------------------------------------------------------
| The values to return on conversions to 32-bit integer formats that raise an
| invalid exception.
*----------------------------------------------------------------------------*/
#define Hax_ui32_fromPosOverflow 0xFFFFFFFF
#define Hax_ui32_fromNegOverflow 0xFFFFFFFF
#define Hax_ui32_fromNaN         0xFFFFFFFF
#define Hax_i32_fromPosOverflow  (-0x7FFFFFFF - 1)
#define Hax_i32_fromNegOverflow  (-0x7FFFFFFF - 1)
#define Hax_i32_fromNaN          (-0x7FFFFFFF - 1)

/*----------------------------------------------------------------------------
| The values to return on conversions to 64-bit integer formats that raise an
| invalid exception.
*----------------------------------------------------------------------------*/
#define Hax_ui64_fromPosOverflow HAX_UINT64_C( 0xFFFFFFFFFFFFFFFF )
#define Hax_ui64_fromNegOverflow HAX_UINT64_C( 0xFFFFFFFFFFFFFFFF )
#define Hax_ui64_fromNaN         HAX_UINT64_C( 0xFFFFFFFFFFFFFFFF )
#define Hax_i64_fromPosOverflow  (-HAX_INT64_C( 0x7FFFFFFFFFFFFFFF ) - 1)
#define Hax_i64_fromNegOverflow  (-HAX_INT64_C( 0x7FFFFFFFFFFFFFFF ) - 1)
#define Hax_i64_fromNaN          (-HAX_INT64_C( 0x7FFFFFFFFFFFFFFF ) - 1)

/*----------------------------------------------------------------------------
| "Common NaN" structure, used to transfer NaN representations from one format
| to another.
*----------------------------------------------------------------------------*/
struct Hax_commonNaN {
    Hax_bool sign;
    Hax_uint64_t v0, v64;
};

/*----------------------------------------------------------------------------
| The bit pattern for a default generated 16-bit floating-point NaN.
*----------------------------------------------------------------------------*/
#define Hax_defaultNaNF16UI 0xFE00

/*----------------------------------------------------------------------------
| Returns true when 16-bit unsigned integer 'uiA' has the bit pattern of a
| 16-bit floating-point signaling NaN.
| Note:  This macro evaluates its argument more than once.
*----------------------------------------------------------------------------*/
#define Hax_softfloat_isSigNaNF16UI( uiA ) ((((uiA) & 0x7E00) == 0x7C00) && ((uiA) & 0x01FF))

/*----------------------------------------------------------------------------
| Assuming 'uiA' has the bit pattern of a 16-bit floating-point NaN, converts
| this NaN to the common NaN form, and stores the resulting common NaN at the
| location pointed to by 'zPtr'.  If the NaN is a signaling NaN, the invalid
| exception is raised.
*----------------------------------------------------------------------------*/
void Hax_softfloat_f16UIToCommonNaN( Hax_uint_fast16_t uiA, struct Hax_commonNaN *zPtr );

/*----------------------------------------------------------------------------
| Converts the common NaN pointed to by 'aPtr' into a 16-bit floating-point
| NaN, and returns the bit pattern of this value as an unsigned integer.
*----------------------------------------------------------------------------*/
Hax_uint_fast16_t Hax_softfloat_commonNaNToF16UI( const struct Hax_commonNaN *aPtr );

/*----------------------------------------------------------------------------
| Interpreting 'uiA' and 'uiB' as the bit patterns of two 16-bit floating-
| point values, at least one of which is a NaN, returns the bit pattern of
| the combined NaN result.  If either 'uiA' or 'uiB' has the pattern of a
| signaling NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/
Hax_uint_fast16_t
 Hax_softfloat_propagateNaNF16UI( Hax_uint_fast16_t uiA, Hax_uint_fast16_t uiB );

/*----------------------------------------------------------------------------
| The bit pattern for a default generated 32-bit floating-point NaN.
*----------------------------------------------------------------------------*/
#define Hax_defaultNaNF32UI 0xFFC00000

/*----------------------------------------------------------------------------
| Returns true when 32-bit unsigned integer 'uiA' has the bit pattern of a
| 32-bit floating-point signaling NaN.
| Note:  This macro evaluates its argument more than once.
*----------------------------------------------------------------------------*/
#define Hax_softfloat_isSigNaNF32UI( uiA ) ((((uiA) & 0x7FC00000) == 0x7F800000) && ((uiA) & 0x003FFFFF))

/*----------------------------------------------------------------------------
| Assuming 'uiA' has the bit pattern of a 32-bit floating-point NaN, converts
| this NaN to the common NaN form, and stores the resulting common NaN at the
| location pointed to by 'zPtr'.  If the NaN is a signaling NaN, the invalid
| exception is raised.
*----------------------------------------------------------------------------*/
void Hax_softfloat_f32UIToCommonNaN( Hax_uint_fast32_t uiA, struct Hax_commonNaN *zPtr );

/*----------------------------------------------------------------------------
| Converts the common NaN pointed to by 'aPtr' into a 32-bit floating-point
| NaN, and returns the bit pattern of this value as an unsigned integer.
*----------------------------------------------------------------------------*/
Hax_uint_fast32_t Hax_softfloat_commonNaNToF32UI( const struct Hax_commonNaN *aPtr );

/*----------------------------------------------------------------------------
| Interpreting 'uiA' and 'uiB' as the bit patterns of two 32-bit floating-
| point values, at least one of which is a NaN, returns the bit pattern of
| the combined NaN result.  If either 'uiA' or 'uiB' has the pattern of a
| signaling NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/
Hax_uint_fast32_t
 Hax_softfloat_propagateNaNF32UI( Hax_uint_fast32_t uiA, Hax_uint_fast32_t uiB );

/*----------------------------------------------------------------------------
| The bit pattern for a default generated 64-bit floating-point NaN.
*----------------------------------------------------------------------------*/
#define Hax_defaultNaNF64UI HAX_UINT64_C( 0xFFF8000000000000 )

/*----------------------------------------------------------------------------
| Returns true when 64-bit unsigned integer 'uiA' has the bit pattern of a
| 64-bit floating-point signaling NaN.
| Note:  This macro evaluates its argument more than once.
*----------------------------------------------------------------------------*/
#define Hax_softfloat_isSigNaNF64UI( uiA ) ((((uiA) & HAX_UINT64_C( 0x7FF8000000000000 )) == HAX_UINT64_C( 0x7FF0000000000000 )) && ((uiA) & HAX_UINT64_C( 0x0007FFFFFFFFFFFF )))

/*----------------------------------------------------------------------------
| Assuming 'uiA' has the bit pattern of a 64-bit floating-point NaN, converts
| this NaN to the common NaN form, and stores the resulting common NaN at the
| location pointed to by 'zPtr'.  If the NaN is a signaling NaN, the invalid
| exception is raised.
*----------------------------------------------------------------------------*/
void Hax_softfloat_f64UIToCommonNaN( Hax_uint_fast64_t uiA, struct Hax_commonNaN *zPtr );

/*----------------------------------------------------------------------------
| Converts the common NaN pointed to by 'aPtr' into a 64-bit floating-point
| NaN, and returns the bit pattern of this value as an unsigned integer.
*----------------------------------------------------------------------------*/
Hax_uint_fast64_t Hax_softfloat_commonNaNToF64UI( const struct Hax_commonNaN *aPtr );

/*----------------------------------------------------------------------------
| Interpreting 'uiA' and 'uiB' as the bit patterns of two 64-bit floating-
| point values, at least one of which is a NaN, returns the bit pattern of
| the combined NaN result.  If either 'uiA' or 'uiB' has the pattern of a
| signaling NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/
Hax_uint_fast64_t
 Hax_softfloat_propagateNaNF64UI( Hax_uint_fast64_t uiA, Hax_uint_fast64_t uiB );

/*----------------------------------------------------------------------------
| The bit pattern for a default generated 80-bit extended floating-point NaN.
*----------------------------------------------------------------------------*/
#define Hax_defaultNaNExtF80UI64 0xFFFF
#define Hax_defaultNaNExtF80UI0  HAX_UINT64_C( 0xC000000000000000 )

/*----------------------------------------------------------------------------
| Returns true when the 80-bit unsigned integer formed from concatenating
| 16-bit 'uiA64' and 64-bit 'uiA0' has the bit pattern of an 80-bit extended
| floating-point signaling NaN.
| Note:  This macro evaluates its arguments more than once.
*----------------------------------------------------------------------------*/
#define Hax_softfloat_isSigNaNExtF80UI( uiA64, uiA0 ) ((((uiA64) & 0x7FFF) == 0x7FFF) && ! ((uiA0) & HAX_UINT64_C( 0x4000000000000000 )) && ((uiA0) & HAX_UINT64_C( 0x3FFFFFFFFFFFFFFF )))

#ifdef SOFTFLOAT_FAST_INT64

/*----------------------------------------------------------------------------
| The following functions are needed only when 'SOFTFLOAT_FAST_INT64' is
| defined.
*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
| Assuming the unsigned integer formed from concatenating 'uiA64' and 'uiA0'
| has the bit pattern of an 80-bit extended floating-point NaN, converts
| this NaN to the common NaN form, and stores the resulting common NaN at the
| location pointed to by 'zPtr'.  If the NaN is a signaling NaN, the invalid
| exception is raised.
*----------------------------------------------------------------------------*/
void
 Hax_softfloat_extF80UIToCommonNaN(
     Hax_uint_fast16_t uiA64, Hax_uint_fast64_t uiA0, struct Hax_commonNaN *zPtr );

/*----------------------------------------------------------------------------
| Converts the common NaN pointed to by 'aPtr' into an 80-bit extended
| floating-point NaN, and returns the bit pattern of this value as an unsigned
| integer.
*----------------------------------------------------------------------------*/
struct Hax_uint128 Hax_softfloat_commonNaNToExtF80UI( const struct Hax_commonNaN *aPtr );

/*----------------------------------------------------------------------------
| Interpreting the unsigned integer formed from concatenating 'uiA64' and
| 'uiA0' as an 80-bit extended floating-point value, and likewise interpreting
| the unsigned integer formed from concatenating 'uiB64' and 'uiB0' as another
| 80-bit extended floating-point value, and assuming at least on of these
| floating-point values is a NaN, returns the bit pattern of the combined NaN
| result.  If either original floating-point value is a signaling NaN, the
| invalid exception is raised.
*----------------------------------------------------------------------------*/
struct Hax_uint128
 Hax_softfloat_propagateNaNExtF80UI(
     Hax_uint_fast16_t uiA64,
     Hax_uint_fast64_t uiA0,
     Hax_uint_fast16_t uiB64,
     Hax_uint_fast64_t uiB0
 );

/*----------------------------------------------------------------------------
| The bit pattern for a default generated 128-bit floating-point NaN.
*----------------------------------------------------------------------------*/
#define Hax_defaultNaNF128UI64 HAX_UINT64_C( 0xFFFF800000000000 )
#define Hax_defaultNaNF128UI0  HAX_UINT64_C( 0 )

/*----------------------------------------------------------------------------
| Returns true when the 128-bit unsigned integer formed from concatenating
| 64-bit 'uiA64' and 64-bit 'uiA0' has the bit pattern of a 128-bit floating-
| point signaling NaN.
| Note:  This macro evaluates its arguments more than once.
*----------------------------------------------------------------------------*/
#define Hax_softfloat_isSigNaNF128UI( uiA64, uiA0 ) ((((uiA64) & HAX_UINT64_C( 0x7FFF800000000000 )) == HAX_UINT64_C( 0x7FFF000000000000 )) && ((uiA0) || ((uiA64) & HAX_UINT64_C( 0x00007FFFFFFFFFFF ))))

/*----------------------------------------------------------------------------
| Assuming the unsigned integer formed from concatenating 'uiA64' and 'uiA0'
| has the bit pattern of a 128-bit floating-point NaN, converts this NaN to
| the common NaN form, and stores the resulting common NaN at the location
| pointed to by 'zPtr'.  If the NaN is a signaling NaN, the invalid exception
| is raised.
*----------------------------------------------------------------------------*/
void
 Hax_softfloat_f128UIToCommonNaN(
     Hax_uint_fast64_t uiA64, Hax_uint_fast64_t uiA0, struct Hax_commonNaN *zPtr );

/*----------------------------------------------------------------------------
| Converts the common NaN pointed to by 'aPtr' into a 128-bit floating-point
| NaN, and returns the bit pattern of this value as an unsigned integer.
*----------------------------------------------------------------------------*/
struct Hax_uint128 Hax_softfloat_commonNaNToF128UI( const struct Hax_commonNaN * );

/*----------------------------------------------------------------------------
| Interpreting the unsigned integer formed from concatenating 'uiA64' and
| 'uiA0' as a 128-bit floating-point value, and likewise interpreting the
| unsigned integer formed from concatenating 'uiB64' and 'uiB0' as another
| 128-bit floating-point value, and assuming at least on of these floating-
| point values is a NaN, returns the bit pattern of the combined NaN result.
| If either original floating-point value is a signaling NaN, the invalid
| exception is raised.
*----------------------------------------------------------------------------*/
struct Hax_uint128
 Hax_softfloat_propagateNaNF128UI(
     Hax_uint_fast64_t uiA64,
     Hax_uint_fast64_t uiA0,
     Hax_uint_fast64_t uiB64,
     Hax_uint_fast64_t uiB0
 );

#else

/*----------------------------------------------------------------------------
| The following functions are needed only when 'SOFTFLOAT_FAST_INT64' is not
| defined.
*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
| Assuming the 80-bit extended floating-point value pointed to by 'aSPtr' is
| a NaN, converts this NaN to the common NaN form, and stores the resulting
| common NaN at the location pointed to by 'zPtr'.  If the NaN is a signaling
| NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/
void
 Hax_softfloat_extF80MToCommonNaN(
     const struct Hax_extFloat80M *aSPtr, struct Hax_commonNaN *zPtr );

/*----------------------------------------------------------------------------
| Converts the common NaN pointed to by 'aPtr' into an 80-bit extended
| floating-point NaN, and stores this NaN at the location pointed to by
| 'zSPtr'.
*----------------------------------------------------------------------------*/
void
 Hax_softfloat_commonNaNToExtF80M(
     const struct Hax_commonNaN *aPtr, struct Hax_extFloat80M *zSPtr );

/*----------------------------------------------------------------------------
| Assuming at least one of the two 80-bit extended floating-point values
| pointed to by 'aSPtr' and 'bSPtr' is a NaN, stores the combined NaN result
| at the location pointed to by 'zSPtr'.  If either original floating-point
| value is a signaling NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/
void
 Hax_softfloat_propagateNaNExtF80M(
     const struct Hax_extFloat80M *aSPtr,
     const struct Hax_extFloat80M *bSPtr,
     struct Hax_extFloat80M *zSPtr
 );

/*----------------------------------------------------------------------------
| The bit pattern for a default generated 128-bit floating-point NaN.
*----------------------------------------------------------------------------*/
#define Hax_defaultNaNF128UI96 0xFFFF8000
#define Hax_defaultNaNF128UI64 0
#define Hax_defaultNaNF128UI32 0
#define Hax_defaultNaNF128UI0  0

/*----------------------------------------------------------------------------
| Assuming the 128-bit floating-point value pointed to by 'aWPtr' is a NaN,
| converts this NaN to the common NaN form, and stores the resulting common
| NaN at the location pointed to by 'zPtr'.  If the NaN is a signaling NaN,
| the invalid exception is raised.  Argument 'aWPtr' points to an array of
| four 32-bit elements that concatenate in the platform's normal endian order
| to form a 128-bit floating-point value.
*----------------------------------------------------------------------------*/
void
 Hax_softfloat_f128MToCommonNaN( const Hax_uint32_t *aWPtr, struct Hax_commonNaN *zPtr );

/*----------------------------------------------------------------------------
| Converts the common NaN pointed to by 'aPtr' into a 128-bit floating-point
| NaN, and stores this NaN at the location pointed to by 'zWPtr'.  Argument
| 'zWPtr' points to an array of four 32-bit elements that concatenate in the
| platform's normal endian order to form a 128-bit floating-point value.
*----------------------------------------------------------------------------*/
void
 Hax_softfloat_commonNaNToF128M( const struct Hax_commonNaN *aPtr, Hax_uint32_t *zWPtr );

/*----------------------------------------------------------------------------
| Assuming at least one of the two 128-bit floating-point values pointed to by
| 'aWPtr' and 'bWPtr' is a NaN, stores the combined NaN result at the location
| pointed to by 'zWPtr'.  If either original floating-point value is a
| signaling NaN, the invalid exception is raised.  Each of 'aWPtr', 'bWPtr',
| and 'zWPtr' points to an array of four 32-bit elements that concatenate in
| the platform's normal endian order to form a 128-bit floating-point value.
*----------------------------------------------------------------------------*/
void
 Hax_softfloat_propagateNaNF128M(
     const Hax_uint32_t *aWPtr, const Hax_uint32_t *bWPtr, Hax_uint32_t *zWPtr );

#endif

#endif
