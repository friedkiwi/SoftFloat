
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3a, by John R. Hauser.

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

/*============================================================================
Modifications to comply with IBM IEEE Binary Floating Point, as defined
in the z/Architecture Principles of Operation, SA22-7832-10, by
Stephen R. Orso.  Said modifications identified by compilation conditioned
on preprocessor variable IBM_IEEE.
All such modifications placed in the public domain by Stephen R. Orso
=============================================================================*/


#ifdef HAVE_PLATFORM_H 
#include "platform.h" 
#endif
#if !defined(false) 
#include <stdbool.h> 
#endif
#if !defined(int32_t) 
#include <stdint.h>             /* C99 standard integers */ 
#endif
#include "internals.h"
#include "softfloat.h"

uint_fast32_t
 softfloat_roundPackToUI32(
     bool sign, uint_fast64_t sig, uint_fast8_t roundingMode, bool exact )
{
    bool roundNearEven;
    uint_fast8_t roundIncrement, roundBits;
    uint_fast32_t z;

    roundNearEven = (roundingMode == softfloat_round_near_even);
    roundIncrement = 0x40;
    if ( ! roundNearEven && (roundingMode != softfloat_round_near_maxMag) ) {
        roundIncrement =
            (roundingMode
                 == (sign ? softfloat_round_min : softfloat_round_max))
                ? 0x7F
                : 0;
    }
    roundBits = sig & 0x7F;
    sig += roundIncrement;
    if ( sig & UINT64_C( 0xFFFFFF8000000000 ) ) goto invalid;
    z = sig>>7;
#ifdef IBM_IEEE
    /* secret sauce below for round to odd                                                          */
    /* if pre-rounding result is exact, no rounding                                                 */
    /* rounding increment for round to odd is always zero, so alternatives are truncation to odd    */
    /* or increment to next odd                                                                     */
    /* if truncated result is already odd, below does not change result.                            */
    /* if truncated result is even, below increases magnitude to next higher magnitute odd value    */
    z |= (uint_fast32_t)(roundBits && (roundingMode == softfloat_round_odd));   /* ensure odd valued result if round to odd   */
#endif  /* IBM_IEEE  */

    z &= ~(uint_fast32_t) (! (roundBits ^ 0x40) & roundNearEven);
    if ( sign && z ) goto invalid;
    if ( exact && roundBits ) {
        softfloat_exceptionFlags |= softfloat_flag_inexact;
    }
    return z;
 invalid:                   /* negative sign and non-zero rounded significand. */
    softfloat_raiseFlags( softfloat_flag_invalid );
#ifdef IBM_IEEE
    return 0x00000000;
#else
    return 0xFFFFFFFF;
#endif   /* IBM_IEEE   */

}

