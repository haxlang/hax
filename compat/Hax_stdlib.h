/*	NetBSD: stdlib.h,v 1.121 2019/01/05 09:16:46 maya Exp 	*/

/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)stdlib.h	8.5 (Berkeley) 5/19/95
 */

#ifndef _HAX_STDLIB_H_
#define _HAX_STDLIB_H_

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

int	 haxAtoi(const char *);
void	 haxFree(void *);
void	*haxMalloc(Size_t);
void	 haxQsort(void *, Size_t, Size_t, int (*)(const void *, const void *));
void	*haxRealloc(void *, Size_t);
double	 haxStrtod(const char *, char **);
long	 haxStrtol(const char *, char **, int);
unsigned long
	 haxStrtoul(const char *, char **, int);
long long int	haxStrtoll(const char *, char **, int);

#endif /* !_HAX_STDLIB_H_ */
