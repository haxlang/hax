
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
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

#define _HAXSOFTFLOAT_INTERNAL
#include "haxSoftFloat.h"
#include "haxSoftFloatInternals.h"
#include "haxSoftFloatSpecialize.h"

Hax_int_fast64_t
 Hax_softfloat_roundMToI64(
     Hax_bool sign, Hax_uint32_t *extSigPtr, Hax_uint_fast8_t roundingMode, Hax_bool exact )
{
    Hax_uint64_t sig;
    Hax_uint32_t sigExtra;
    union { Hax_uint64_t ui; Hax_int64_t i; } uZ;
    Hax_int64_t z;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    sig =
        (Hax_uint64_t) extSigPtr[Hax_indexWord( 3, 2 )]<<32
            | extSigPtr[Hax_indexWord( 3, 1 )];
    sigExtra = extSigPtr[Hax_indexWordLo( 3 )];
    if (
        (roundingMode == Hax_softfloat_round_near_maxMag)
            || (roundingMode == Hax_softfloat_round_near_even)
    ) {
        if ( 0x80000000 <= sigExtra ) goto increment;
    } else {
        if (
            sigExtra
                && (sign
                        ? (roundingMode == Hax_softfloat_round_min)
#ifdef SOFTFLOAT_ROUND_ODD
                              || (roundingMode == Hax_softfloat_round_odd)
#endif
                        : (roundingMode == Hax_softfloat_round_max))
        ) {
 increment:
            ++sig;
            if ( !sig ) goto invalid;
            if (
                (sigExtra == 0x80000000)
                    && (roundingMode == Hax_softfloat_round_near_even)
            ) {
                sig &= ~(Hax_uint_fast64_t) 1;
            }
        }
    }
    uZ.ui = sign ? -sig : sig;
    z = uZ.i;
    if ( z && ((z < 0) ^ sign) ) goto invalid;
    if ( sigExtra ) {
#ifdef SOFTFLOAT_ROUND_ODD
        if ( roundingMode == Hax_softfloat_round_odd ) z |= 1;
#endif
        if ( exact ) Hax_softfloat_exceptionFlags |= Hax_softfloat_flag_inexact;
    }
    return z;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 invalid:
    Hax_softfloat_raiseFlags( Hax_softfloat_flag_invalid );
    return sign ? Hax_i64_fromNegOverflow : Hax_i64_fromPosOverflow;

}
