
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015 The Regents of the University of
California.  All rights reserved.

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

Double Hax_FloatToDouble(Float a)
{
    union Hax_ui32_f32 uA;
    Hax_uint_fast32_t uiA;
    Hax_bool sign;
    Hax_int_fast16_t exp;
    Hax_uint_fast32_t frac;
    struct Hax_commonNaN commonNaN;
    Hax_uint_fast64_t uiZ;
    struct Hax_exp16_sig32 normExpSig;
    union Hax_ui64_f64 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    sign = Hax_signF32UI( uiA );
    exp  = Hax_expF32UI( uiA );
    frac = Hax_fracF32UI( uiA );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( exp == 0xFF ) {
        if ( frac ) {
            Hax_softfloat_f32UIToCommonNaN( uiA, &commonNaN );
            uiZ = Hax_softfloat_commonNaNToF64UI( &commonNaN );
        } else {
            uiZ = Hax_packToF64UI( sign, 0x7FF, 0 );
        }
        goto uiZ;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! exp ) {
        if ( ! frac ) {
            uiZ = Hax_packToF64UI( sign, 0, 0 );
            goto uiZ;
        }
        normExpSig = Hax_softfloat_normSubnormalF32Sig( frac );
        exp = normExpSig.exp - 1;
        frac = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uiZ = Hax_packToF64UI( sign, exp + 0x380, (Hax_uint_fast64_t) frac<<29 );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}
