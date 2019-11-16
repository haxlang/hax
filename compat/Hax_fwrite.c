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

#include "Hax_stdio.h"
#include "Hax_string.h"

#ifndef SIZE_T_DEFINED   
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED   
#endif

int __towrite(Hax_FILE *f);

Size_t __fwritex(const unsigned char * s, Size_t l, Hax_FILE * f)
{
	Size_t i=0;

	if (!f->wend && __towrite(f)) return 0;

	if (l > f->wend - f->wpos) return f->write(f, s, l);

	if (f->lbf >= 0) {
		/* Match /^(.*\n|)/ */
		for (i=l; i && s[i-1] != '\n'; i--);
		if (i) {
			Size_t n = f->write(f, s, i);
			if (n < i) return n;
			s += i;
			l -= i;
		}
	}

	__builtin_memcpy(f->wpos, s, l);
	f->wpos += l;
	return l+i;
}
