/*
 * Copyright (c) 1983, 1993
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
 * 4. Neither the name of the University nor the names of its contributors
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
 */

#if 0
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)malloc.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#endif

/*
 * malloc.c (Caltech) 2/21/82
 * Chris Kingsley, kingsley@cit-20.
 *
 * This is a very fast storage allocator.  It allocates blocks of a small 
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this 
 * implementation, the available sizes are 2^n-4 (or 2^n-10) bytes long.
 * This is designed for use in a virtual memory environment.
 */

#ifndef NULL
#define	NULL ((void *)0)
#endif

#ifndef INTPTR_T_DEFINED
typedef __INTPTR_TYPE__ Intptr_t;
#define INTPTR_T_DEFINED
#endif

#ifndef U_INT_DEFINED
typedef unsigned int U_int;
#define U_INT_DEFINED
#endif

#ifndef U_SHORT_DEFINED
typedef unsigned short U_short;
#define U_SHORT_DEFINED
#endif

#ifndef U_CHAR_DEFINED
typedef unsigned char U_char;
#define U_CHAR_DEFINED
#endif

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

#ifndef CADDR_T_DEFINED
typedef void *Caddr_t;
#define CADDR_T_DEFINED
#endif

extern void            Hax_Panic(char *format, ...);

#define RSLOP           0

/*
 * The overhead on a block is at least 4 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must
 * be zero.  When in use, the first byte is set to MAGIC, and the second
 * byte is the size index.  The remaining bytes are for alignment.
 * If range checking is enabled then a second word holds the size of the
 * requested block, less 1, rounded up to a multiple of sizeof(RMAGIC).
 * The order of elements is critical: ov_magic must overlay the low order
 * bits of ov_next, and ov_magic can not be a valid ov_next bit pattern.
 */
union	overhead {
	union	overhead *ov_next;	/* when free */
	struct {
		U_char	ovu_magic;	/* magic number */
		U_char	ovu_index;	/* bucket # */
	} ovu;
#define	ov_magic	ovu.ovu_magic
#define	ov_index	ovu.ovu_index
#define	ov_rmagic	ovu.ovu_rmagic
#define	ov_size		ovu.ovu_size
};

static void Hax_morecore(int);
static int Hax_findbucket(union overhead *, int);

#define	MAGIC		0xef		/* magic # on accounting info */
#define RMAGIC		0x5555		/* magic # on range info */

/*
 * Hax_nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information
 * precedes the data area returned to the user.
 */
#define	NBUCKETS 30
static	union overhead *Hax_nextf[NBUCKETS];
extern	void *Hax_sbrk(Intptr_t);

static	int pagesz;			/* page size */
static	int pagebucket;			/* page size bucket */

void *
haxMalloc(Size_t nbytes)
{
  	union overhead *op;
  	int bucket, n;
	unsigned amt;

	/*
	 * First time haxMalloc is called, setup page size and
	 * align break pointer so all data will be page aligned.
	 */
	if (pagesz == 0) {
		pagesz = n = 4096;
		op = (union overhead *)Hax_sbrk(0);
  		n = n - sizeof (*op) - ((Intptr_t)op & (n - 1));
		if (n < 0)
			n += pagesz;
  		if (n) {
  			if (Hax_sbrk(n) == (char *)-1)
				return (NULL);
		}
		bucket = 0;
		amt = 8;
		while (pagesz > amt) {
			amt <<= 1;
			bucket++;
		}
		pagebucket = bucket;
	}
	/*
	 * Convert amount of memory requested into closest block size
	 * stored in hash buckets which satisfies request.
	 * Account for space used per block for accounting.
	 */
	if (nbytes <= (n = pagesz - sizeof (*op) - RSLOP)) {
		amt = 8;	/* size of first bucket */
		bucket = 0;
		n = -((int)sizeof (*op) + RSLOP);
	} else {
		amt = pagesz;
		bucket = pagebucket;
	}
	while (nbytes > amt + n) {
		amt <<= 1;
		if (amt == 0)
			return (NULL);
		bucket++;
	}
	/*
	 * If nothing in hash bucket right now,
	 * request more memory from the system.
	 */
  	if ((op = Hax_nextf[bucket]) == NULL) {
  		Hax_morecore(bucket);
  		if ((op = Hax_nextf[bucket]) == NULL)
  			return (NULL);
	}
	/* remove from linked list */
  	Hax_nextf[bucket] = op->ov_next;
	op->ov_magic = MAGIC;
	op->ov_index = bucket;

  	return ((char *)(op + 1));
}

/*
 * Allocate more memory to the indicated bucket.
 */
static void
Hax_morecore(int bucket)
{
  	union overhead *op;
	int sz;		/* size of desired block */
  	int amt;			/* amount to allocate */
  	int nblks;			/* how many blocks we get */

	/*
	 * sbrk_size <= 0 only for big, FLUFFY, requests (about
	 * 2^30 bytes on a VAX, I think) or for a negative arg.
	 */
	sz = 1 << (bucket + 3);

	if (sz <= 0)
		Hax_Panic("%s() %s:%d sz=%d, not >0\n", __func__, __FILE__, __LINE__, sz);

	if (sz < pagesz) {
		amt = pagesz;
  		nblks = amt / sz;
	} else {
		amt = sz + pagesz;
		nblks = 1;
	}
	op = (union overhead *)Hax_sbrk(amt);
	/* no more room! */
  	if ((char *)op == (char *)-1)
  		return;
	/*
	 * Add new memory allocated to that on
	 * free list for this hash bucket.
	 */
  	Hax_nextf[bucket] = op;
  	while (--nblks > 0) {
		op->ov_next = (union overhead *)((Caddr_t)op + sz);
		op = (union overhead *)((Caddr_t)op + sz);
  	}
}

void
haxFree(void *cp)
{   
  	int size;
	union overhead *op;

  	if (cp == NULL)
  		return;
	op = (union overhead *)((Caddr_t)cp - sizeof (union overhead));

	/* make sure it was in use */
	if (op->ov_magic != MAGIC)
		Hax_Panic("%s() %s:%d op->ov_magic != MAGIC\n", __func__, __FILE__, __LINE__);
  	size = op->ov_index;
	if (size >= NBUCKETS)
		Hax_Panic("%s() %s:%d size >= NBUCKETS\n", __func__, __FILE__, __LINE__);

	op->ov_next = Hax_nextf[size];	/* also clobbers ov_magic */
  	Hax_nextf[size] = op;
}

/*
 * When a program attempts "storage compaction" as mentioned in the
 * old malloc man page, it realloc's an already freed block.  Usually
 * this is the last block it freed; occasionally it might be farther
 * back.  We have to search all the free lists for the block in order
 * to determine its bucket: 1st we make one pass thru the lists
 * checking only the first block in each; if that fails we search
 * ``__realloc_srchlen'' blocks in each list for a match (the variable
 * is extern so the caller can modify it).  If that fails we just copy
 * however many bytes was given to realloc() and hope it's not huge.
 */
int __realloc_srchlen = 4;	/* 4 should be plenty, -1 =>'s whole list */

void *
haxRealloc(void *cp, Size_t nbytes)
{   
  	U_int onb;
	int i;
	union overhead *op;
  	char *res;
	int was_alloced = 0;

  	if (cp == NULL)
  		return (haxMalloc(nbytes));
	op = (union overhead *)((Caddr_t)cp - sizeof (union overhead));
	if (op->ov_magic == MAGIC) {
		was_alloced++;
		i = op->ov_index;
	} else {
		/*
		 * Already free, doing "compaction".
		 *
		 * Search for the old block of memory on the
		 * free list.  First, check the most common
		 * case (last element free'd), then (this failing)
		 * the last ``__realloc_srchlen'' items free'd.
		 * If all lookups fail, then assume the size of
		 * the memory block being realloc'd is the
		 * largest possible (so that all "nbytes" of new
		 * memory are copied into).  Note that this could cause
		 * a memory fault if the old area was tiny, and the moon
		 * is gibbous.  However, that is very unlikely.
		 */
		if ((i = Hax_findbucket(op, 1)) < 0 &&
		    (i = Hax_findbucket(op, __realloc_srchlen)) < 0)
			i = NBUCKETS;
	}
	onb = 1 << (i + 3);
	if (onb < pagesz)
		onb -= sizeof (*op) + RSLOP;
	else
		onb += pagesz - sizeof (*op) - RSLOP;
	/* avoid the copy if same size block */
	if (was_alloced) {
		if (i) {
			i = 1 << (i + 2);
			if (i < pagesz)
				i -= sizeof (*op) + RSLOP;
			else
				i += pagesz - sizeof (*op) - RSLOP;
		}
		if (nbytes <= onb && nbytes > i) {
			return(cp);
		} else
			haxFree(cp);
	}
  	if ((res = haxMalloc(nbytes)) == NULL)
  		return (NULL);
  	if (cp != res)		/* common optimization if "compacting" */
		__builtin_memcpy(cp, res, (nbytes < onb) ? nbytes : onb);
  	return (res);
}

/*
 * Search ``srchlen'' elements of each free list for a block whose
 * header starts at ``freep''.  If srchlen is -1 search the whole list.
 * Return bucket number, or -1 if not found.
 */
static int
Hax_findbucket(union overhead *freep, int srchlen)
{
	union overhead *p;
	int i, j;

	for (i = 0; i < NBUCKETS; i++) {
		j = 0;
		for (p = Hax_nextf[i]; p && j != srchlen; p = p->ov_next) {
			if (p == freep)
				return (i);
			j++;
		}
	}
	return (-1);
}
