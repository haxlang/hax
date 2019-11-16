
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014 The Regents of the University of California.
All rights reserved.

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

Double Hax_DoubleDiv(Double a, Double b)
{
    union Hax_ui64_f64 uA;
    Hax_uint_fast64_t uiA;
    Hax_bool signA;
    Hax_int_fast16_t expA;
    Hax_uint_fast64_t sigA;
    union Hax_ui64_f64 uB;
    Hax_uint_fast64_t uiB;
    Hax_bool signB;
    Hax_int_fast16_t expB;
    Hax_uint_fast64_t sigB;
    Hax_bool signZ;
    struct Hax_exp16_sig64 normExpSig;
    Hax_int_fast16_t expZ;
    Hax_uint32_t recip32, sig32Z, doubleTerm;
    Hax_uint_fast64_t rem;
    Hax_uint32_t q;
    Hax_uint_fast64_t sigZ;
    Hax_uint_fast64_t uiZ;
    union Hax_ui64_f64 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    signA = Hax_signF64UI( uiA );
    expA  = Hax_expF64UI( uiA );
    sigA  = Hax_fracF64UI( uiA );
    uB.f = b;
    uiB = uB.ui;
    signB = Hax_signF64UI( uiB );
    expB  = Hax_expF64UI( uiB );
    sigB  = Hax_fracF64UI( uiB );
    signZ = signA ^ signB;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( expA == 0x7FF ) {
        if ( sigA ) goto propagateNaN;
        if ( expB == 0x7FF ) {
            if ( sigB ) goto propagateNaN;
            goto invalid;
        }
        goto infinity;
    }
    if ( expB == 0x7FF ) {
        if ( sigB ) goto propagateNaN;
        goto zero;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expB ) {
        if ( ! sigB ) {
            if ( ! (expA | sigA) ) goto invalid;
            Hax_softfloat_raiseFlags( Hax_softfloat_flag_infinite );
            goto infinity;
        }
        normExpSig = Hax_softfloat_normSubnormalF64Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }
    if ( ! expA ) {
        if ( ! sigA ) goto zero;
        normExpSig = Hax_softfloat_normSubnormalF64Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expZ = expA - expB + 0x3FE;
    sigA |= HAX_UINT64_C( 0x0010000000000000 );
    sigB |= HAX_UINT64_C( 0x0010000000000000 );
    if ( sigA < sigB ) {
        --expZ;
        sigA <<= 11;
    } else {
        sigA <<= 10;
    }
    sigB <<= 11;
    recip32 = Hax_softfloat_approxRecip32_1( sigB>>32 ) - 2;
    sig32Z = ((Hax_uint32_t) (sigA>>32) * (Hax_uint_fast64_t) recip32)>>32;
    doubleTerm = sig32Z<<1;
    rem =
        ((sigA - (Hax_uint_fast64_t) doubleTerm * (Hax_uint32_t) (sigB>>32))<<28)
            - (Hax_uint_fast64_t) doubleTerm * ((Hax_uint32_t) sigB>>4);
    q = (((Hax_uint32_t) (rem>>32) * (Hax_uint_fast64_t) recip32)>>32) + 4;
    sigZ = ((Hax_uint_fast64_t) sig32Z<<32) + ((Hax_uint_fast64_t) q<<4);
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( (sigZ & 0x1FF) < 4<<4 ) {
        q &= ~7;
        sigZ &= ~(Hax_uint_fast64_t) 0x7F;
        doubleTerm = q<<1;
        rem =
            ((rem - (Hax_uint_fast64_t) doubleTerm * (Hax_uint32_t) (sigB>>32))<<28)
                - (Hax_uint_fast64_t) doubleTerm * ((Hax_uint32_t) sigB>>4);
        if ( rem & HAX_UINT64_C( 0x8000000000000000 ) ) {
            sigZ -= 1<<7;
        } else {
            if ( rem ) sigZ |= 1;
        }
    }
    return Hax_softfloat_roundPackToF64( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = Hax_softfloat_propagateNaNF64UI( uiA, uiB );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 invalid:
    Hax_softfloat_raiseFlags( Hax_softfloat_flag_invalid );
    uiZ = Hax_defaultNaNF64UI;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 infinity:
    uiZ = Hax_packToF64UI( signZ, 0x7FF, 0 );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 zero:
    uiZ = Hax_packToF64UI( signZ, 0, 0 );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}
