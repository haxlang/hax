/*
 * haxBreakpoint.c --
 *
 *	Source code for the "Hax_Breakpoint" library procedure
 *	for Hax; individual applications will probably override
 *	this with an application-specific breakpoint procedure.
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

#include <signal.h>
#include <unistd.h>

/*
 *----------------------------------------------------------------------
 *
 * Hax_Breakpoint --
 *
 *	Raise event and enter into the debugger.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The process interrupts, entering the debugger if possible.
 *
 *----------------------------------------------------------------------
 */


void
Hax_Breakpoint(void)
{
    kill (getpid(), SIGINT);
}
