
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016 The Regents of the University of
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

Hax_float64_t
 Hax_softfloat_addMagsF64( Hax_uint_fast64_t uiA, Hax_uint_fast64_t uiB, Hax_bool signZ )
{
    Hax_int_fast16_t expA;
    Hax_uint_fast64_t sigA;
    Hax_int_fast16_t expB;
    Hax_uint_fast64_t sigB;
    Hax_int_fast16_t expDiff;
    Hax_uint_fast64_t uiZ;
    Hax_int_fast16_t expZ;
    Hax_uint_fast64_t sigZ;
    union Hax_ui64_f64 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expA = Hax_expF64UI( uiA );
    sigA = Hax_fracF64UI( uiA );
    expB = Hax_expF64UI( uiB );
    sigB = Hax_fracF64UI( uiB );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expDiff = expA - expB;
    if ( ! expDiff ) {
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        if ( ! expA ) {
            uiZ = uiA + sigB;
            goto uiZ;
        }
        if ( expA == 0x7FF ) {
            if ( sigA | sigB ) goto propagateNaN;
            uiZ = uiA;
            goto uiZ;
        }
        expZ = expA;
        sigZ = HAX_UINT64_C( 0x0020000000000000 ) + sigA + sigB;
        sigZ <<= 9;
    } else {
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        sigA <<= 9;
        sigB <<= 9;
        if ( expDiff < 0 ) {
            if ( expB == 0x7FF ) {
                if ( sigB ) goto propagateNaN;
                uiZ = Hax_packToF64UI( signZ, 0x7FF, 0 );
                goto uiZ;
            }
            expZ = expB;
            if ( expA ) {
                sigA += HAX_UINT64_C( 0x2000000000000000 );
            } else {
                sigA <<= 1;
            }
            sigA = Hax_softfloat_shiftRightJam64( sigA, -expDiff );
        } else {
            if ( expA == 0x7FF ) {
                if ( sigA ) goto propagateNaN;
                uiZ = uiA;
                goto uiZ;
            }
            expZ = expA;
            if ( expB ) {
                sigB += HAX_UINT64_C( 0x2000000000000000 );
            } else {
                sigB <<= 1;
            }
            sigB = Hax_softfloat_shiftRightJam64( sigB, expDiff );
        }
        sigZ = HAX_UINT64_C( 0x2000000000000000 ) + sigA + sigB;
        if ( sigZ < HAX_UINT64_C( 0x4000000000000000 ) ) {
            --expZ;
            sigZ <<= 1;
        }
    }
    return Hax_softfloat_roundPackToF64( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = Hax_softfloat_propagateNaNF64UI( uiA, uiB );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}
