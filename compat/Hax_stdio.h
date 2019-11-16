/*
 * Source from: MUSL
 * Commit f5eee489f7662b08ad1bba4b1267e34eb9565bba
 * Author: Rich Felker <dalias@aerifal.cx>
 * Date:   Fri Sep 13 14:17:36 2019 -0400
 *
 * musl as a whole is licensed under the following standard MIT license
 *
 * 2005-2019 Rich Felker, et al.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _HAX_STDIO_H
#define _HAX_STDIO_H

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

#ifndef SSIZE_T_DEFINED
typedef __PTRDIFF_TYPE__ Ssize_t;
#define SSIZE_T_DEFINED
#endif

#include "Hax_stdio_impl.h"

int haxVfprintf(Hax_FILE *, const char *, __builtin_va_list);
int haxVsprintf(char *, const char *, __builtin_va_list);
int haxVsnprintf(char *, Size_t, const char *, __builtin_va_list);

#endif
