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
#include "Hax_errno.h"

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

Size_t __fwritex(const unsigned char * s, Size_t l, Hax_FILE * f);

#define EOF -1
#define NULL ((void *)0)

struct cookie {
	char *s;
	Size_t n;
};

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static Size_t sn_write(Hax_FILE *f, const unsigned char *s, Size_t l)
{
	struct cookie *c = f->cookie;
	Size_t k = MIN(c->n, f->wpos - f->wbase);
	if (k) {
		__builtin_memcpy(c->s, f->wbase, k);
		c->s += k;
		c->n -= k;
	}
	k = MIN(c->n, l);
	if (k) {
		__builtin_memcpy(c->s, s, k);
		c->s += k;
		c->n -= k;
	}
	*c->s = 0;
	f->wpos = f->wbase = f->buf;
	/* pretend to succeed, even if we discarded extra data */
	return l;
}

int haxVsnprintf(char * s, Size_t n, const char * fmt, __builtin_va_list ap)
{
	unsigned char buf[1];
	char dummy[1];
	struct cookie c = { .s = n ? s : dummy, .n = n ? n-1 : 0 };
	Hax_FILE f = {
		.lbf = EOF,
		.write = sn_write,
		.lock = -1,
		.buf = buf,
		.cookie = &c,
	};

	if (n > __INT_MAX__) {
		haxErrno = EOVERFLOW;
		return -1;
	}

	*c.s = 0;
	return haxVfprintf(&f, fmt, ap);
}
