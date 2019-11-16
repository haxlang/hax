
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

#ifndef softfloat_approxRecip32_1

extern const Hax_uint16_t Hax_softfloat_approxRecip_1k0s[16];
extern const Hax_uint16_t Hax_softfloat_approxRecip_1k1s[16];

Hax_uint32_t Hax_softfloat_approxRecip32_1( Hax_uint32_t a )
{
    int index;
    Hax_uint16_t eps, r0;
    Hax_uint32_t sigma0;
    Hax_uint_fast32_t r;
    Hax_uint32_t sqrSigma0;

    index = a>>27 & 0xF;
    eps = (Hax_uint16_t) (a>>11);
    r0 = Hax_softfloat_approxRecip_1k0s[index]
             - ((Hax_softfloat_approxRecip_1k1s[index] * (Hax_uint_fast32_t) eps)>>20);
    sigma0 = ~(Hax_uint_fast32_t) ((r0 * (Hax_uint_fast64_t) a)>>7);
    r = ((Hax_uint_fast32_t) r0<<16) + ((r0 * (Hax_uint_fast64_t) sigma0)>>24);
    sqrSigma0 = ((Hax_uint_fast64_t) sigma0 * sigma0)>>32;
    r += ((Hax_uint32_t) r * (Hax_uint_fast64_t) sqrSigma0)>>48;
    return r;

}

#endif
