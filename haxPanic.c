/* 
 * haxPanic.c --
 *
 *	Source code for the "Hax_Panic" library procedure for Hax;
 *	individual applications will probably override this with
 *	an application-specific panic procedure.
 *
 * Copyright 1988-1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appears in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/panic.c,v 1.3 91/10/10 11:25:51 ouster Exp $ SPRITE (Berkeley)";
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*
 *----------------------------------------------------------------------
 *
 * Hax_Panic --
 *
 *	Print an error message and kill the process.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The process dies, entering the debugger if possible.
 *
 *----------------------------------------------------------------------
 */

	/* VARARGS ARGSUSED */
void
Hax_Panic(
    char *format,		/* Format string, suitable for passing to
				 * fprintf. */
    ...				/* Additional arguments (variable in number)
				 * to pass to fprintf. */)
{
    va_list va;
    va_start(va, format);
    (void) vfprintf(stderr, format, va);
    (void) fflush(stderr);
    abort();
}
