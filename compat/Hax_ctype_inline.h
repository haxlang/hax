/* NetBSD: ctype_inline.h,v 1.4 2015/02/24 13:17:27 joerg Exp  */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)ctype.h	5.3 (Berkeley) 4/3/91
 *	NetBSD: ctype.h,v 1.30 2010/05/22 06:38:15 tnozaki Exp
 */

#ifndef _HAX_CTYPE_INLINE_H_
#define _HAX_CTYPE_INLINE_H_

#include "Hax_ctype_bits.h"

#define	haxIsalnum(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & (_CTYPE_A|_CTYPE_D)))
#define	haxIsalpha(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & _CTYPE_A))
#define	haxIscntrl(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & _CTYPE_C))
#define	haxIsdigit(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & _CTYPE_D))
#define	haxIsgraph(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & _CTYPE_G))
#define	haxIslower(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & _CTYPE_L))
#define	haxIsprint(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & _CTYPE_R))
#define	haxIspunct(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & _CTYPE_P))
#define	haxIsspace(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & _CTYPE_S))
#define	haxIsupper(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & _CTYPE_U))
#define	haxIsxdigit(c)	((int)((Hax__ctype_tab_ + 1)[(c)] & _CTYPE_X))
#define	haxTolower(c)	((int)((Hax__tolower_tab_ + 1)[(c)]))
#define	haxToupper(c)	((int)((Hax__toupper_tab_ + 1)[(c)]))

#define	haxIsascii(c)	((unsigned)(c) <= 0177)

#endif /* !_HAX_CTYPE_INLINE_H_ */
