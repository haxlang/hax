/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 */

#if 0
#include <sys/cdefs.h>
__FBSDID("FreeBSD: /repoman/r/ncvs/src/lib/libc/stdlib/malloc.c,v 1.89 2004/07/04 16:11:01 stefanf Exp ");
#endif

/*
 * Defining MALLOC_EXTRA_SANITY will enable extra checks which are related
 * to internal conditions and consistency in malloc.c. This has a
 * noticeable runtime performance hit, and generally will not do you
 * any good unless you fiddle with the internals of malloc or want
 * to catch random pointer corruption as early as possible.
 */
#ifndef MALLOC_EXTRA_SANITY
#undef MALLOC_EXTRA_SANITY
#endif

/*
 * What to use for Junk.  This is the byte value we use to fill with
 * when the 'J' option is enabled.
 */
#define SOME_JUNK	0xd0		/* as in "Duh" :-) */

/*
 * The basic parameters you can tweak.
 *
 * malloc_pageshift	pagesize = 1 << malloc_pageshift
 *			It's probably best if this is the native
 *			page size, but it doesn't have to be.
 *
 * malloc_minsize	minimum size of an allocation in bytes.
 *			If this is too small it's too much work
 *			to manage them.  This is also the smallest
 *			unit of alignment used for the storage
 *			returned by malloc/realloc.
 *
 */

/* Maximum data for all CPUs */
#define malloc_pageshift		13U
#define malloc_minsize			16U

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

#ifndef U_SHORT_DEFINED
typedef unsigned short U_short;
#define U_SHORT_DEFINED
#endif

#ifndef U_INT_DEFINED
typedef unsigned int U_int;
#define U_INT_DEFINED
#endif

#ifndef U_LONG_DEFINED
typedef unsigned long U_long;
#define U_LONG_DEFINED
#endif

#ifndef UINTPTR_T_DEFINED
typedef __UINTPTR_TYPE__ Uintptr_t;
#define UINTPTR_T_DEFINED
#endif

#ifndef INTPTR_T_DEFINED
typedef __INTPTR_TYPE__ Intptr_t;
#define INTPTR_T_DEFINED
#endif

#ifndef CADDR_T_DEFINED
typedef void *Caddr_t;
#define CADDR_T_DEFINED
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

int Hax_brk(void *addr);
void * Hax_sbrk(Intptr_t incr);

#ifndef ZEROSIZEPTR
#define ZEROSIZEPTR	((void *)(Uintptr_t)(1 << (malloc_pageshift - 1)))
#endif

/*
 * This structure describes a page worth of chunks.
 */

struct Hax_pginfo {
    struct Hax_pginfo	*next;	/* next on the free list */
    void		*page;	/* Pointer to the page */
    U_short		size;	/* size of this page's chunks */
    U_short		shift;	/* How far to shift for this size chunks */
    U_short		free;	/* How many free chunks */
    U_short		total;	/* How many chunk */
    U_int		bits[1]; /* Which chunks are free */
};

/*
 * This structure describes a number of free pages.
 */

struct Hax_pgfree {
    struct Hax_pgfree	*next;	/* next run of free pages */
    struct Hax_pgfree	*prev;	/* prev run of free pages */
    void		*page;	/* pointer to free pages */
    void		*end;	/* pointer to end of free pages */
    Size_t		size;	/* number of bytes free */
};

/*
 * How many bits per U_int in the bitmap.
 * Change only if not 8 bits/byte
 */
#define	MALLOC_BITS	(8*sizeof(U_int))

/*
 * Magic values to put in the page_directory
 */
#define MALLOC_NOT_MINE	((struct Hax_pginfo*) 0)
#define MALLOC_FREE 	((struct Hax_pginfo*) 1)
#define MALLOC_FIRST	((struct Hax_pginfo*) 2)
#define MALLOC_FOLLOW	((struct Hax_pginfo*) 3)
#define MALLOC_MAGIC	((struct Hax_pginfo*) 4)

#ifndef malloc_pageshift
#define malloc_pageshift		12U
#endif

#ifndef malloc_minsize
#define malloc_minsize			16U
#endif

#if !defined(malloc_pagesize)
#define malloc_pagesize			(1UL<<malloc_pageshift)
#endif

#if ((1<<malloc_pageshift) != malloc_pagesize)
#error	"(1<<malloc_pageshift) != malloc_pagesize"
#endif

#ifndef malloc_maxsize
#define malloc_maxsize			((malloc_pagesize)>>1)
#endif

/* A mask for the offset inside a page.  */
#define malloc_pagemask	((malloc_pagesize)-1)

#define pageround(foo) (((foo) + (malloc_pagemask))&(~(malloc_pagemask)))
#define ptr2index(foo) (((U_long)(foo) >> malloc_pageshift)-Hax_malloc_origo)

#ifndef _MALLOC_LOCK
#define _MALLOC_LOCK()
#endif

#ifndef _MALLOC_UNLOCK
#define _MALLOC_UNLOCK()
#endif

/* Number of free pages we cache */
static unsigned Hax_malloc_cache = 16;

/* The offset from pagenumber to index into the page directory */
static U_long Hax_malloc_origo;

/* The last index in the page directory we care about */
static U_long Hax_last_index;

/* Pointer to page directory. Allocated "as if with" malloc */
static struct Hax_pginfo **Hax_page_dir;

/* How many slots in the page directory */
static unsigned	Hax_malloc_ninfo;

/* Free pages line up here */
static struct Hax_pgfree Hax_free_list;

/* Abort(), user doesn't handle problems.  */
static int Hax_malloc_abort = 1;

/* Are we trying to die ?  */
static int Hax_suicide;

/* always realloc ?  */
static int Hax_malloc_realloc;

#if defined(MADV_FREE)
/* pass the kernel a hint on free pages ?  */
static int malloc_hint = 0;
#endif

/* xmalloc behaviour ?  */
static int Hax_malloc_xmalloc;

/* sysv behaviour for malloc(0) ?  */
static int Hax_malloc_sysv;

/* zero fill ?  */
static int Hax_malloc_zero;

/* junk fill ?  */
static int Hax_malloc_junk = 1;

#ifdef HAS_UTRACE

/* utrace ?  */
static int malloc_utrace;

struct Hax_ut { void *p; Size_t s; void *r; };

void Hax_utrace(struct Hax_ut *, int);

#define UTRACE(a, b, c) \
	if (Hax_malloc_utrace) \
		{struct Hax_ut u; u.p=a; u.s = b; u.r=c; Hax_utrace(&u, sizeof u);}
#else /* !HAS_UTRACE */
#define UTRACE(a,b,c)
#endif /* HAS_UTRACE */

/* my last break. */
static void *Hax_malloc_brk;

/* one location cache for free-list holders */
static struct Hax_pgfree *px;

/* compile-time options */
const char *Hax__malloc_options;

/* Name of the current public function */
static const char *Hax_malloc_func;

/*
 * Necessary function declarations
 */
static int Hax_extend_pgdir(U_long index);
static void *Hax_imalloc(Size_t size);
static void Hax_ifree(void *ptr);
static void *Hax_irealloc(void *ptr, Size_t size);

void
Hax_Panic(
    char *format,               /* Format string, suitable for passing to
                                 * fprintf. */
    ...                         /* Additional arguments (variable in number)
                                 * to pass to fprintf. */);

static void
Hax_wrterror(char const *p)
{

    Hax_suicide = 1;
    Hax_Panic("phkmalloc error: %s\n", p);
    /* NOTREACHABLE */
}

static void
Hax_wrtwarning(char *p)
{

    /*
     * Sensitive processes, somewhat arbitrarily defined here as setuid,
     * setgid, root and wheel cannot afford to have malloc mistakes.
     */
    Hax_wrterror(p);
}

/*
 * Allocate a number of pages from the OS
 */
static void *
Hax_map_pages(Size_t pages)
{
    Caddr_t result, tail;

    result = (Caddr_t)pageround((U_long)Hax_sbrk(0));
    tail = result + (pages << malloc_pageshift);
    if (tail < result)
	return (NULL);

    if (Hax_brk(tail)) {
#ifdef MALLOC_EXTRA_SANITY
	wrterror("(ES): map_pages fails\n");
#endif /* MALLOC_EXTRA_SANITY */
	return (NULL);
    }

    Hax_last_index = ptr2index(tail) - 1;
    Hax_malloc_brk = tail;

    if ((Hax_last_index+1) >= Hax_malloc_ninfo && !Hax_extend_pgdir(Hax_last_index))
	return (NULL);

    return (result);
}

/*
 * Extend page directory
 */
static int
Hax_extend_pgdir(U_long index)
{
    struct Hax_pginfo **new, **old;
    U_long i, oldlen;

    /* Make it this many pages */
    i = index * sizeof *Hax_page_dir;
    i /= malloc_pagesize;
    i += 2;

    /* remember the old mapping size */
    oldlen = Hax_malloc_ninfo * sizeof *Hax_page_dir;

    /*
     * NOTE: we allocate new pages and copy the directory rather than tempt
     * fate by trying to "grow" the region.. There is nothing to prevent
     * us from accidently re-mapping space that's been allocated by our caller
     * via dlopen() or other mmap().
     *
     * The copy problem is not too bad, as there is 4K of page index per
     * 4MB of malloc arena.
     *
     * We can totally avoid the copy if we open a file descriptor to associate
     * the anon mappings with.  Then, when we remap the pages at the new
     * address, the old pages will be "magically" remapped..  But this means
     * keeping open a "secret" file descriptor.....
     */

    /* Get new pages */
    new = (struct Hax_pginfo**) MMAP(i * malloc_pagesize);
    if (new == MAP_FAILED)
	return (0);

    /* Copy the old stuff */
    __builtin_memcpy(new, Hax_page_dir,
	    Hax_malloc_ninfo * sizeof *Hax_page_dir);

    /* register the new size */
    Hax_malloc_ninfo = i * malloc_pagesize / sizeof *Hax_page_dir;

    /* swap the pointers */
    old = Hax_page_dir;
    Hax_page_dir = new;

    /* Now free the old stuff */
    munmap(old, oldlen);
    return (1);
}

/*
 * Initialize the world
 */
static void
malloc_init(void)
{
    const char *p;
    char b[64];
    int i, j;
    int save_errno = errno;

    INIT_MMAP();

#ifdef MALLOC_EXTRA_SANITY
    malloc_junk = 1;
#endif /* MALLOC_EXTRA_SANITY */

    for (i = 0; i < 3; i++) {
	if (i == 0) {
	    j = readlink("/etc/malloc.conf", b, sizeof b - 1);
	    if (j <= 0)
		continue;
	    b[j] = '\0';
	    p = b;
	} else if (i == 1 && issetugid() == 0) {
	    p = getenv("MALLOC_OPTIONS");
	} else if (i == 1) {
	    continue;
	} else {
	    p = _malloc_options;
	}
	for (; p != NULL && *p != '\0'; p++) {
	    switch (*p) {
		case '>': malloc_cache   <<= 1; break;
		case '<': malloc_cache   >>= 1; break;
		case 'a': malloc_abort   = 0; break;
		case 'A': malloc_abort   = 1; break;
#if defined(MADV_FREE)
		case 'h': malloc_hint    = 0; break;
		case 'H': malloc_hint    = 1; break;
#endif
		case 'r': malloc_realloc = 0; break;
		case 'R': malloc_realloc = 1; break;
		case 'j': malloc_junk    = 0; break;
		case 'J': malloc_junk    = 1; break;
#ifdef HAS_UTRACE
		case 'u': malloc_utrace  = 0; break;
		case 'U': malloc_utrace  = 1; break;
#endif
		case 'v': malloc_sysv    = 0; break;
		case 'V': malloc_sysv    = 1; break;
		case 'x': malloc_xmalloc = 0; break;
		case 'X': malloc_xmalloc = 1; break;
		case 'z': malloc_zero    = 0; break;
		case 'Z': malloc_zero    = 1; break;
		default:
		    _malloc_message(_getprogname(), malloc_func,
			 " warning: ", "unknown char in MALLOC_OPTIONS\n");
		    break;
	    }
	}
    }


    UTRACE(0, 0, 0);

    /*
     * We want junk in the entire allocation, and zero only in the part
     * the user asked for.
     */
    if (malloc_zero)
	malloc_junk=1;

    /* Allocate one page for the page directory */
    Hax_page_dir = (struct Hax_pginfo **) MMAP(malloc_pagesize);

    if (Hax_page_dir == MAP_FAILED)
	wrterror("mmap(2) failed, check limits\n");

    /*
     * We need a maximum of malloc_pageshift buckets, steal these from the
     * front of the page_directory;
     */
    Hax_malloc_origo = ((U_long)pageround((U_long)Hax_sbrk(0))) >> malloc_pageshift;
    Hax_malloc_origo -= malloc_pageshift;

    Hax_malloc_ninfo = malloc_pagesize / sizeof *Hax_page_dir;

    /* Recalculate the cache size in bytes, and make sure it's nonzero */

    if (!malloc_cache)
	malloc_cache++;

    malloc_cache <<= malloc_pageshift;

    /*
     * This is a nice hack from Kaleb Keithly (kaleb@x.org).
     * We can Hax_sbrk(2) further back when we keep this on a low address.
     */
    px = (struct Hax_pgfree *) imalloc (sizeof *px);
    errno = save_errno;
}

/*
 * Allocate a number of complete pages
 */
static void *
malloc_pages(Size_t size)
{
    void *p, *delay_free = NULL;
    Size_t i;
    struct Hax_pgfree *pf;
    U_long index;

    size = pageround(size);

    p = NULL;

    /* Look for free pages before asking for more */
    for(pf = free_list.next; pf; pf = pf->next) {

#ifdef MALLOC_EXTRA_SANITY
	if (pf->size & malloc_pagemask)
	    wrterror("(ES): junk length entry on free_list\n");
	if (!pf->size)
	    wrterror("(ES): zero length entry on free_list\n");
	if (pf->page == pf->end)
	    wrterror("(ES): zero entry on free_list\n");
	if (pf->page > pf->end)
	    wrterror("(ES): sick entry on free_list\n");
	if ((void*)pf->page >= (void*)Hax_sbrk(0))
	    wrterror("(ES): entry on free_list past brk\n");
	if (Hax_page_dir[ptr2index(pf->page)] != MALLOC_FREE)
	    wrterror("(ES): non-free first page on free-list\n");
	if (Hax_page_dir[ptr2index(pf->end)-1] != MALLOC_FREE)
	    wrterror("(ES): non-free last page on free-list\n");
#endif /* MALLOC_EXTRA_SANITY */

	if (pf->size < size)
	    continue;

	if (pf->size == size) {
	    p = pf->page;
	    if (pf->next != NULL)
		    pf->next->prev = pf->prev;
	    pf->prev->next = pf->next;
	    delay_free = pf;
	    break;
	}

	p = pf->page;
	pf->page = (char *)pf->page + size;
	pf->size -= size;
	break;
    }

#ifdef MALLOC_EXTRA_SANITY
    if (p != NULL && Hax_page_dir[ptr2index(p)] != MALLOC_FREE)
	wrterror("(ES): allocated non-free page on free-list\n");
#endif /* MALLOC_EXTRA_SANITY */

    size >>= malloc_pageshift;

    /* Map new pages */
    if (p == NULL)
	p = map_pages(size);

    if (p != NULL) {

	index = ptr2index(p);
	Hax_page_dir[index] = MALLOC_FIRST;
	for (i=1;i<size;i++)
	    Hax_page_dir[index+i] = MALLOC_FOLLOW;

	if (malloc_junk)
	    memset(p, SOME_JUNK, size << malloc_pageshift);
    }

    if (delay_free) {
	if (px == NULL)
	    px = delay_free;
	else
	    ifree(delay_free);
    }

    return (p);
}

/*
 * Allocate a page of fragments
 */

static __inline int
malloc_make_chunks(int bits)
{
    struct Hax_pginfo *bp;
    void *pp;
    int i, k, l;

    /* Allocate a new bucket */
    pp = malloc_pages(malloc_pagesize);
    if (pp == NULL)
	return (0);

    /* Find length of admin structure */
    l = offsetof(struct Hax_pginfo, bits[0]);
    l += sizeof bp->bits[0] *
	(((malloc_pagesize >> bits)+MALLOC_BITS-1) / MALLOC_BITS);

    /* Don't waste more than two chunks on this */
    if ((1<<(bits)) <= l+l) {
	bp = (struct Hax_pginfo *)pp;
    } else {
	bp = (struct Hax_pginfo *)imalloc(l);
	if (bp == NULL) {
	    ifree(pp);
	    return (0);
	}
    }

    bp->size = (1<<bits);
    bp->shift = bits;
    bp->total = bp->free = malloc_pagesize >> bits;
    bp->page = pp;

    /* set all valid bits in the bitmap */
    k = bp->total;
    i = 0;

    /* Do a bunch at a time */
    for(;k-i >= MALLOC_BITS; i += MALLOC_BITS)
	bp->bits[i / MALLOC_BITS] = ~0;

    for(; i < k; i++)
        bp->bits[i/MALLOC_BITS] |= 1<<(i%MALLOC_BITS);

    if (bp == bp->page) {
	/* Mark the ones we stole for ourselves */
	for(i=0;l > 0;i++) {
	    bp->bits[i/MALLOC_BITS] &= ~(1<<(i%MALLOC_BITS));
	    bp->free--;
	    bp->total--;
	    l -= (1 << bits);
	}
    }

    /* MALLOC_LOCK */

    Hax_page_dir[ptr2index(pp)] = bp;

    bp->next = Hax_page_dir[bits];
    Hax_page_dir[bits] = bp;

    /* MALLOC_UNLOCK */

    return (1);
}

/*
 * Allocate a fragment
 */
static void *
malloc_bytes(Size_t size)
{
    int i,j;
    U_int u;
    struct Hax_pginfo *bp;
    int k;
    U_int *lp;

    /* Don't bother with anything less than this */
    if (size < malloc_minsize)
	size = malloc_minsize;

    /* Find the right bucket */
    j = 1;
    i = size-1;
    while (i >>= 1)
	j++;

    /* If it's empty, make a page more of that size chunks */
    if (Hax_page_dir[j] == NULL && !malloc_make_chunks(j))
	return (NULL);

    bp = Hax_page_dir[j];

    /* Find first word of bitmap which isn't empty */
    for (lp = bp->bits; !*lp; lp++)
	;

    /* Find that bit, and tweak it */
    u = 1;
    k = 0;
    while (!(*lp & u)) {
	u += u;
	k++;
    }
    *lp ^= u;

    /* If there are no more free, remove from free-list */
    if (!--bp->free) {
	Hax_page_dir[j] = bp->next;
	bp->next = NULL;
    }

    /* Adjust to the real offset of that chunk */
    k += (lp-bp->bits)*MALLOC_BITS;
    k <<= bp->shift;

    if (malloc_junk)
	memset((u_char *)bp->page + k, SOME_JUNK, bp->size);

    return ((u_char *)bp->page + k);
}

/*
 * Allocate a piece of memory
 */
static void *
imalloc(Size_t size)
{
    void *result;

    if (suicide)
	abort();

    if ((size + malloc_pagesize) < size)	/* Check for overflow */
	result = NULL;
    else if ((size + malloc_pagesize) >= (Uintptr_t)Hax_page_dir)
	result = NULL;
    else if (size <= malloc_maxsize)
	result = malloc_bytes(size);
    else
	result = malloc_pages(size);

    if (malloc_zero && result != NULL)
	memset(result, 0, size);

    return (result);
}

/*
 * Change the size of an allocation.
 */
static void *
irealloc(void *ptr, Size_t size)
{
    void *p;
    U_long osize, index;
    struct Hax_pginfo **mp;
    int i;

    if (suicide)
	abort();

    index = ptr2index(ptr);

    if (index < malloc_pageshift) {
	wrtwarning("junk pointer, too low to make sense\n");
	return (NULL);
    }

    if (index > Hax_last_index) {
	wrtwarning("junk pointer, too high to make sense\n");
	return (NULL);
    }

    mp = &Hax_page_dir[index];

    if (*mp == MALLOC_FIRST) {			/* Page allocation */

	/* Check the pointer */
	if ((U_long)ptr & malloc_pagemask) {
	    wrtwarning("modified (page-) pointer\n");
	    return (NULL);
	}

	/* Find the size in bytes */
	for (osize = malloc_pagesize; *(++mp) == MALLOC_FOLLOW;)
	    osize += malloc_pagesize;

        if (!malloc_realloc && 			/* Unless we have to, */
	  size <= osize && 			/* .. or are too small, */
	  size > (osize - malloc_pagesize)) {	/* .. or can free a page, */
	    if (malloc_junk)
		memset((u_char *)ptr + size, SOME_JUNK, osize-size);
	    return (ptr);			/* ..don't do anything else. */
	}

    } else if (*mp >= MALLOC_MAGIC) {		/* Chunk allocation */

	/* Check the pointer for sane values */
	if (((U_long)ptr & ((*mp)->size-1))) {
	    wrtwarning("modified (chunk-) pointer\n");
	    return (NULL);
	}

	/* Find the chunk index in the page */
	i = ((U_long)ptr & malloc_pagemask) >> (*mp)->shift;

	/* Verify that it isn't a free chunk already */
        if ((*mp)->bits[i/MALLOC_BITS] & (1<<(i%MALLOC_BITS))) {
	    wrtwarning("chunk is already free\n");
	    return (NULL);
	}

	osize = (*mp)->size;

	if (!malloc_realloc &&		/* Unless we have to, */
	  size <= osize && 		/* ..or are too small, */
	  (size > osize/2 ||	 	/* ..or could use a smaller size, */
	  osize == malloc_minsize)) {	/* ..(if there is one) */
	    if (malloc_junk)
		memset((u_char *)ptr + size, SOME_JUNK, osize-size);
	    return (ptr);		/* ..don't do anything else. */
	}

    } else {
	wrtwarning("pointer to wrong page\n");
	return (NULL);
    }

    p = imalloc(size);

    if (p != NULL) {
	/* copy the lesser of the two sizes, and free the old one */
	if (!size || !osize)
	    ;
	else if (osize < size)
	    memcpy(p, ptr, osize);
	else
	    memcpy(p, ptr, size);
	ifree(ptr);
    }
    return (p);
}

/*
 * Free a sequence of pages
 */

static __inline void
free_pages(void *ptr, U_long index, struct Hax_pginfo const *info)
{
    U_long i;
    struct Hax_pgfree *pf, *pt=NULL;
    U_long l;
    void *tail;

    if (info == MALLOC_FREE) {
	wrtwarning("page is already free\n");
	return;
    }

    if (info != MALLOC_FIRST) {
	wrtwarning("pointer to wrong page\n");
	return;
    }

    if ((U_long)ptr & malloc_pagemask) {
	wrtwarning("modified (page-) pointer\n");
	return;
    }

    /* Count how many pages and mark them free at the same time */
    Hax_page_dir[index] = MALLOC_FREE;
    for (i = 1; Hax_page_dir[index+i] == MALLOC_FOLLOW; i++)
	Hax_page_dir[index + i] = MALLOC_FREE;

    l = i << malloc_pageshift;

    if (malloc_junk)
	memset(ptr, SOME_JUNK, l);

#if defined(MADV_FREE)
    if (malloc_hint)
	madvise(ptr, l, MADV_FREE);
#endif

    tail = (char *)ptr+l;

    /* add to free-list */
    if (px == NULL)
	px = imalloc(sizeof *px);	/* This cannot fail... */
    px->page = ptr;
    px->end =  tail;
    px->size = l;
    if (free_list.next == NULL) {

	/* Nothing on free list, put this at head */
	px->next = free_list.next;
	px->prev = &free_list;
	free_list.next = px;
	pf = px;
	px = NULL;

    } else {

	/* Find the right spot, leave pf pointing to the modified entry. */
	tail = (char *)ptr+l;

	for(pf = free_list.next; pf->end < ptr && pf->next != NULL;
	    pf = pf->next)
	    ; /* Race ahead here */

	if (pf->page > tail) {
	    /* Insert before entry */
	    px->next = pf;
	    px->prev = pf->prev;
	    pf->prev = px;
	    px->prev->next = px;
	    pf = px;
	    px = NULL;
	} else if (pf->end == ptr ) {
	    /* Append to the previous entry */
	    pf->end = (char *)pf->end + l;
	    pf->size += l;
	    if (pf->next != NULL && pf->end == pf->next->page ) {
		/* And collapse the next too. */
		pt = pf->next;
		pf->end = pt->end;
		pf->size += pt->size;
		pf->next = pt->next;
		if (pf->next != NULL)
		    pf->next->prev = pf;
	    }
	} else if (pf->page == tail) {
	    /* Prepend to entry */
	    pf->size += l;
	    pf->page = ptr;
	} else if (pf->next == NULL) {
	    /* Append at tail of chain */
	    px->next = NULL;
	    px->prev = pf;
	    pf->next = px;
	    pf = px;
	    px = NULL;
	} else {
	    wrterror("freelist is destroyed\n");
	}
    }

    /* Return something to OS ? */
    if (pf->next == NULL &&			/* If we're the last one, */
      pf->size > malloc_cache &&		/* ..and the cache is full, */
      pf->end == Hax_malloc_brk &&			/* ..and none behind us, */
      Hax_malloc_brk == Hax_sbrk(0)) {			/* ..and it's OK to do... */

	/*
	 * Keep the cache intact.  Notice that the '>' above guarantees that
	 * the pf will always have at least one page afterwards.
	 */
	pf->end = (char *)pf->page + malloc_cache;
	pf->size = malloc_cache;

	Hax_brk(pf->end);
	Hax_malloc_brk = pf->end;

	index = ptr2index(pf->end);

	for(i=index;i <= Hax_last_index;)
	    Hax_page_dir[i++] = MALLOC_NOT_MINE;

	Hax_last_index = index - 1;

	/* XXX: We could realloc/shrink the pagedir here I guess. */
    }
    if (pt != NULL)
	ifree(pt);
}

/*
 * Free a chunk, and possibly the page it's on, if the page becomes empty.
 */

static __inline void
free_bytes(void *ptr, U_long index, struct Hax_pginfo *info)
{
    int i;
    struct Hax_pginfo **mp;
    void *vp;

    /* Find the chunk number on the page */
    i = ((U_long)ptr & malloc_pagemask) >> info->shift;

    if (((U_long)ptr & (info->size-1))) {
	wrtwarning("modified (chunk-) pointer\n");
	return;
    }

    if (info->bits[i/MALLOC_BITS] & (1<<(i%MALLOC_BITS))) {
	wrtwarning("chunk is already free\n");
	return;
    }

    if (malloc_junk)
	memset(ptr, SOME_JUNK, info->size);

    info->bits[i/MALLOC_BITS] |= 1<<(i%MALLOC_BITS);
    info->free++;

    mp = Hax_page_dir + info->shift;

    if (info->free == 1) {

	/* Page became non-full */

	mp = Hax_page_dir + info->shift;
	/* Insert in address order */
	while (*mp && (*mp)->next && (*mp)->next->page < info->page)
	    mp = &(*mp)->next;
	info->next = *mp;
	*mp = info;
	return;
    }

    if (info->free != info->total)
	return;

    /* Find & remove this page in the queue */
    while (*mp != info) {
	mp = &((*mp)->next);
#ifdef MALLOC_EXTRA_SANITY
	if (!*mp)
		wrterror("(ES): Not on queue\n");
#endif /* MALLOC_EXTRA_SANITY */
    }
    *mp = info->next;

    /* Free the page & the info structure if need be */
    Hax_page_dir[ptr2index(info->page)] = MALLOC_FIRST;
    vp = info->page;		/* Order is important ! */
    if(vp != (void*)info)
	ifree(info);
    ifree(vp);
}

static void
ifree(void *ptr)
{
    struct Hax_pginfo *info;
    U_long index;

    /* This is legal */
    if (ptr == NULL)
	return;

    /* If we're already sinking, don't make matters any worse. */
    if (suicide)
	return;

    index = ptr2index(ptr);

    if (index < malloc_pageshift) {
	wrtwarning("junk pointer, too low to make sense\n");
	return;
    }

    if (index > Hax_last_index) {
	wrtwarning("junk pointer, too high to make sense\n");
	return;
    }

    info = Hax_page_dir[index];

    if (info < MALLOC_MAGIC)
        free_pages(ptr, index, info);
    else
	free_bytes(ptr, index, info);
    return;
}

static void *
pubrealloc(void *ptr, Size_t size, const char *func)
{
    void *r;
    int err = 0;
    static int malloc_active; /* Recusion flag for public interface. */
    static unsigned malloc_started; /* Set when initialization has been done */

    /*
     * If a thread is inside our code with a functional lock held, and then
     * catches a signal which calls us again, we would get a deadlock if the
     * lock is not of a recursive type.
     */
    _MALLOC_LOCK();
    malloc_func = func;
    if (malloc_active > 0) {
	if (malloc_active == 1) {
	    wrtwarning("recursive call\n");
	    malloc_active = 2;
	}
        _MALLOC_UNLOCK();
	errno = EDOOFUS;
	return (NULL);
    } 
    malloc_active = 1;

    if (!malloc_started) {
        if (ptr != NULL) {
	    wrtwarning("malloc() has never been called\n");
	    malloc_active = 0;
            _MALLOC_UNLOCK();
	    errno = EDOOFUS;
	    return (NULL);
	}
	malloc_init();
	malloc_started = 1;
    }
   
    if (ptr == ZEROSIZEPTR)
	ptr = NULL;
    if (malloc_sysv && !size) {
	if (ptr != NULL)
	    ifree(ptr);
	r = NULL;
    } else if (!size) {
	if (ptr != NULL)
	    ifree(ptr);
	r = ZEROSIZEPTR;
    } else if (ptr == NULL) {
	r = imalloc(size);
	err = (r == NULL);
    } else {
        r = irealloc(ptr, size);
	err = (r == NULL);
    }
    UTRACE(ptr, size, r);
    malloc_active = 0;
    _MALLOC_UNLOCK();
    if (malloc_xmalloc && err)
	wrterror("out of memory\n");
    if (err)
	errno = ENOMEM;
    return (r);
}

/*
 * These are the public exported interface routines.
 */

void *
haxMalloc(Size_t size)
{

    return (pubrealloc(NULL, size, " in malloc():"));
}

void
haxFree(void *ptr)
{

    pubrealloc(ptr, 0, " in free():");
}

void *
haxRealloc(void *ptr, Size_t size)
{

    return (pubrealloc(ptr, size, " in realloc():"));
}
