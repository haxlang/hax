/*
 * haxCallocArena.c --
 *
 *	Source code for the "Hax_CallocArena" library procedure
 *	for Hax; individual applications will probably override
 *	this with an application-specific calloc procedure.
 *
 * Copyright 2019 Kamil Rytarowski
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appears in all copies.  The copyright holders
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#include <stdlib.h>

/*
 *----------------------------------------------------------------------
 *
 * Hax_CallocArena --
 *
 *	Allocate arena for the slab allocator.
 *
 * Results:
 *	Pointer to allocated memory if successul.
 *
 * Side effects:
 *	The process dies, entering the debugger if possible.
 *
 *----------------------------------------------------------------------
 */

void *
Hax_CallocArena(
    size_t number,		/* Number of objects to allocate */
    size_t size			/* Size of each object */)
{
    void *ptr;

    ptr = calloc(number, size);
    if (ptr == NULL)
	abort();

    return ptr;
}
