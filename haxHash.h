/*
 * haxHash.h --
 *
 *	This header file declares the facilities provided by the
 *	Hax hash table procedures.
 *
 * Copyright 1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * $Header: /sprite/src/lib/tcl/RCS/tclHash.h,v 1.3 91/08/27 11:36:04 ouster Exp $ SPRITE (Berkeley)
 */

#ifndef _HAXHASH
#define _HAXHASH

#ifndef _HAX
#include <hax.h>
#endif

/*
 * Structure definition for an entry in a hash table.  No-one outside
 * Hax should access any of these fields directly;  use the macros
 * defined below.
 */

typedef struct Hax_HashEntry {
    struct Hax_HashEntry *nextPtr;	/* Pointer to next entry in this
					 * hash bucket, or NULL for end of
					 * chain. */
    struct Hax_HashTable *tablePtr;	/* Pointer to table containing entry. */
    struct Hax_HashEntry **bucketPtr;	/* Pointer to bucket that points to
					 * first entry in this entry's chain:
					 * used for deleting the entry. */
    ClientData clientData;		/* Application stores something here
					 * with Hax_SetHashValue. */
    union {				/* Key has one of these forms: */
	char *oneWordValue;		/* One-word value for key. */
	int words[1];			/* Multiple integer words for key.
					 * The actual size will be as large
					 * as necessary for this table's
					 * keys. */
	char string[4];			/* String for key.  The actual size
					 * will be as large as needed to hold
					 * the key. */
    } key;				/* MUST BE LAST FIELD IN RECORD!! */
} Hax_HashEntry;

/*
 * Structure definition for a hash table.  Must be in hax.h so clients
 * can allocate space for these structures, but clients should never
 * access any fields in this structure.
 */

#define HAX_SMALL_HASH_TABLE 4
typedef struct Hax_HashTable {
    Hax_HashEntry **buckets;		/* Pointer to bucket array.  Each
					 * element points to first entry in
					 * bucket's hash chain, or NULL. */
    Hax_HashEntry *staticBuckets[HAX_SMALL_HASH_TABLE];
					/* Bucket array used for small tables
					 * (to avoid mallocs and frees). */
    int numBuckets;			/* Total number of buckets allocated
					 * at **bucketPtr. */
    int numEntries;			/* Total number of entries present
					 * in table. */
    int rebuildSize;			/* Enlarge table when numEntries gets
					 * to be this large. */
    int downShift;			/* Shift count used in hashing
					 * function.  Designed to use high-
					 * order bits of randomized keys. */
    int mask;				/* Mask value used in hashing
					 * function. */
    int keyType;			/* Type of keys used in this table. 
					 * It's either HAX_STRING_KEYS,
					 * HAX_ONE_WORD_KEYS, or an integer
					 * giving the number of ints in a
					 */
    Hax_HashEntry *(*findProc) (struct Hax_HashTable *tablePtr,
	    char *key);
    Hax_HashEntry *(*createProc) (struct Hax_HashTable *tablePtr,
	    char *key, int *newPtr);
} Hax_HashTable;

/*
 * Structure definition for information used to keep track of searches
 * through hash tables:
 */

typedef struct Hax_HashSearch {
    Hax_HashTable *tablePtr;		/* Table being searched. */
    int nextIndex;			/* Index of next bucket to be
					 * enumerated after present one. */
    Hax_HashEntry *nextEntryPtr;	/* Next entry to be enumerated in the
					 * the current bucket. */
} Hax_HashSearch;

/*
 * Acceptable key types for hash tables:
 */

#define HAX_STRING_KEYS		0
#define HAX_ONE_WORD_KEYS	1

/*
 * Macros for clients to use to access fields of hash entries:
 */

#define Hax_GetHashValue(h) ((h)->clientData)
#define Hax_SetHashValue(h, value) ((h)->clientData = (ClientData) (value))
#define Hax_GetHashKey(tablePtr, h) \
    ((char *) (((tablePtr)->keyType == HAX_ONE_WORD_KEYS) ? (h)->key.oneWordValue \
						: (h)->key.string))

/*
 * Macros to use for clients to use to invoke find and create procedures
 * for hash tables:
 */

#define Hax_FindHashEntry(tablePtr, key) \
	(*((tablePtr)->findProc))(tablePtr, key)
#define Hax_CreateHashEntry(tablePtr, key, newPtr) \
	(*((tablePtr)->createProc))(tablePtr, key, newPtr)

/*
 * Exported procedures:
 */

HAX_EXTERN void			Hax_DeleteHashEntry (
				    Hax_HashEntry *entryPtr);
HAX_EXTERN void			Hax_DeleteHashTable (
				    Hax_HashTable *tablePtr);
HAX_EXTERN Hax_HashEntry *	Hax_FirstHashEntry (
				    Hax_HashTable *tablePtr,
				    Hax_HashSearch *searchPtr);
HAX_EXTERN char *		Hax_HashStats (Hax_HashTable *tablePtr);
HAX_EXTERN void			Hax_InitHashTable (Hax_HashTable *tablePtr,
				    int keyType);
HAX_EXTERN Hax_HashEntry *	Hax_NextHashEntry (
				    Hax_HashSearch *searchPtr);

#endif /* _HAXHASH */
