/*	NetBSD: strcspn.c,v 1.2 2018/02/04 01:13:45 mrg Exp 	*/

/*-
 * Copyright (c) 2008 Joerg Sonnenberger
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if 0
#include <sys/cdefs.h>
__RCSID("NetBSD: strcspn.c,v 1.2 2018/02/04 01:13:45 mrg Exp ");
#endif

#include "Hax_string.h"

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

#ifndef UINT8_T_DEFINED
typedef __UINT8_TYPE__ Uint8_t;
#define UINT8_T_DEFINED
#endif

/* 64bit version is in strspn.c */
#if __LONG_MAX__ != 0x7fffffffffffffffL

Size_t
haxStrcspn(const char *s, const char *charset)
{
	static const Uint8_t idx[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
	const char *t;
	Uint8_t set[32];
#define UC(a) ((unsigned int)(unsigned char)(a))

	if (charset[0] == '\0')
		return haxStrlen(s);
	if (charset[1] == '\0') {
		for (t = s; *t != '\0'; ++t)
			if (*t == *charset)
				break;
		return t - s;
	}

	(void)__builtin_memset(set, 0, sizeof(set));

	for (; *charset != '\0'; ++charset)
		set[UC(*charset) >> 3] |= idx[UC(*charset) & 7];

	for (t = s; *t != '\0'; ++t)
		if (set[UC(*t) >> 3] & idx[UC(*t) & 7])
			break;
	return t - s;
}

#endif