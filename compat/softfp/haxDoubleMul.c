
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

Double Hax_DoubleMul(Double a, Double b)
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
    Hax_uint_fast64_t magBits;
    struct Hax_exp16_sig64 normExpSig;
    Hax_int_fast16_t expZ;
#ifdef SOFTFLOAT_FAST_INT64
    struct Hax_uint128 sig128Z;
#else
    Hax_uint32_t sig128Z[4];
#endif
    Hax_uint_fast64_t sigZ, uiZ;
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
        if ( sigA || ((expB == 0x7FF) && sigB) ) goto propagateNaN;
        magBits = expB | sigB;
        goto infArg;
    }
    if ( expB == 0x7FF ) {
        if ( sigB ) goto propagateNaN;
        magBits = expA | sigA;
        goto infArg;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expA ) {
        if ( ! sigA ) goto zero;
        normExpSig = Hax_softfloat_normSubnormalF64Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    if ( ! expB ) {
        if ( ! sigB ) goto zero;
        normExpSig = Hax_softfloat_normSubnormalF64Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expZ = expA + expB - 0x3FF;
    sigA = (sigA | HAX_UINT64_C( 0x0010000000000000 ))<<10;
    sigB = (sigB | HAX_UINT64_C( 0x0010000000000000 ))<<11;
#ifdef SOFTFLOAT_FAST_INT64
    sig128Z = Hax_softfloat_mul64To128( sigA, sigB );
    sigZ = sig128Z.v64 | (sig128Z.v0 != 0);
#else
    Hax_softfloat_mul64To128M( sigA, sigB, sig128Z );
    sigZ =
        (Hax_uint64_t) sig128Z[Hax_indexWord( 4, 3 )]<<32 | sig128Z[Hax_indexWord( 4, 2 )];
    if ( sig128Z[Hax_indexWord( 4, 1 )] || sig128Z[Hax_indexWord( 4, 0 )] ) sigZ |= 1;
#endif
    if ( sigZ < HAX_UINT64_C( 0x4000000000000000 ) ) {
        --expZ;
        sigZ <<= 1;
    }
    return Hax_softfloat_roundPackToF64( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = Hax_softfloat_propagateNaNF64UI( uiA, uiB );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 infArg:
    if ( ! magBits ) {
        Hax_softfloat_raiseFlags( Hax_softfloat_flag_invalid );
        uiZ = Hax_defaultNaNF64UI;
    } else {
        uiZ = Hax_packToF64UI( signZ, 0x7FF, 0 );
    }
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 zero:
    uiZ = Hax_packToF64UI( signZ, 0, 0 );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}
