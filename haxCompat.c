/*
 * haxCompat.c --
 *
 *	Definition of compat code used by the Hax interpreter.
 *
 * Copyright 2019 Kamil Rytarowski
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The copyright holders
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef _HAXCOMPAT
#define _HAXCOMPAT

#include "haxInt.h"

/*
 * <stdio.h> - standard buffered input/output
 */

int
Hax_sprintf(char *str, const char *format, ...)
{
    int result;
    va_list ap;

    va_start(ap, format);
    result = vsprintf(str, format, ap);
    va_end(ap);

    return result;
}

/*
 * <stdlib.h> - standard library definitions
 */

double
Hax_strtod(const char *nptr, char **endptr)
{
    return strtod(nptr, endptr);
}


#endif /* _HAXCOMPAT */
