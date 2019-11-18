/*
 * haxStrtol.c --
 *
 *	Source code for wrapper of the strtol() family of
 *	functions. These functions preserve system errno and
 *	store the function call one in an interpreter local
 *	structure.
 *
 * Copyright 2019 Kamil Rytarowski
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appears in all copies.  The copyright holder
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#include "haxInt.h"

#include <errno.h>

static int ErrnoToInternalErrno(int e);


/*
 *----------------------------------------------------------------------
 *
 * Strtol --
 *
 *	Convert string value to a long integer.
 *
 * Results:
 *	Converted integer on successful operation.
 *
 * Side effects:
 *	On error, errno is stored in the interpreter structure.
 *
 *----------------------------------------------------------------------
 */

long int
Strtol(
    Interp *iPtr,
    const char *nptr,
    char **endptr,
    int base)
{
    long int result;
    int saved_errno;

    saved_errno = errno;
    errno = 0;
    result = strtol(nptr, endptr, base);
    if (iPtr) {
	iPtr->internalErrno = ErrnoToInternalErrno(errno);
    }
    errno = saved_errno;

    return result;
}


/*
 *----------------------------------------------------------------------
 *
 * Strtoll --
 *
 *	Convert string value to a long long integer.
 *
 * Results:
 *	Converted integer on successful operation.
 *
 * Side effects:
 *	On error, errno is stored in the interpreter structure.
 *
 *----------------------------------------------------------------------
 */

long long int
Strtoll(
    Interp *iPtr,
    const char *nptr,
    char **endptr,
    int base)
{
    long long int result;
    int saved_errno;

    saved_errno = errno;
    errno = 0;
    result = strtoll(nptr, endptr, base);
    if (iPtr) {
	iPtr->internalErrno = ErrnoToInternalErrno(errno);
    }
    errno = saved_errno;

    return result;
}


/*
 *----------------------------------------------------------------------
 *
 * Strtoul --
 *
 *	Convert string value to an unsigned long integer.
 *
 * Results:
 *	Converted integer on successful operation.
 *
 * Side effects:
 *	On error, errno is stored in the interpreter structure.
 *
 *----------------------------------------------------------------------
 */

unsigned long int
Strtoul(
    Interp *iPtr,
    const char *nptr,
    char **endptr,
    int base)
{
    unsigned long int result;
    int saved_errno;

    saved_errno = errno;
    errno = 0;
    result = strtoul(nptr, endptr, base);
    if (iPtr) {
	iPtr->internalErrno = ErrnoToInternalErrno(errno);
    }
    errno = saved_errno;

    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * ErrnoToInternalErrno --
 *
 *      Converts system errno code to internal errno code.
 *
 * Results:
 *      Internal errno code.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static int
ErrnoToInternalErrno(int e)
{
    switch (e) {
	case 0:
	    return 0;
	case EINVAL:
	    return HAX_EINVAL;
	case ERANGE:
	    return HAX_ERANGE;
	default:
	    Hax_Panic("Unknown errno=%d\n", e);
    }
    /* NOTREACHABLE */
    return 0;
}
