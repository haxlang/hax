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

#ifndef _STDIO_IMPL_H
#define _STDIO_IMPL_H

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

#ifndef OFF_T_DEFINED
typedef __UINT64_TYPE__ Off_t;
#define OFF_T_DEFINED
#endif

#define UNGET 8

#define FFINALLOCK(f) ((f)->lock>=0 ? __lockfile((f)) : 0)
#define FLOCK(f)
#define FUNLOCK(f)

#define F_PERM 1
#define F_NORD 4
#define F_NOWR 8
#define F_EOF 16
#define F_ERR 32
#define F_SVB 64
#define F_APP 128

struct _IO_FILE;

typedef struct _IO_FILE Hax_FILE;

struct _IO_FILE {
	unsigned flags;
	unsigned char *rpos, *rend;
	int (*close)(Hax_FILE *);
	unsigned char *wend, *wpos;
	unsigned char *mustbezero_1;
	unsigned char *wbase;
	Size_t (*read)(Hax_FILE *, unsigned char *, Size_t);
	Size_t (*write)(Hax_FILE *, const unsigned char *, Size_t);
	Off_t (*seek)(Hax_FILE *, Off_t, int);
	unsigned char *buf;
	Size_t buf_size;
	Hax_FILE *prev, *next;
	int fd;
	int pipe_pid;
	long lockcount;
	int mode;
	volatile int lock;
	int lbf;
	void *cookie;
	Off_t off;
	char *getln_buf;
	void *mustbezero_2;
	unsigned char *shend;
	Off_t shlim, shcnt;
	Hax_FILE *prev_locked, *next_locked;
	struct __locale_struct *locale;
};
#endif
