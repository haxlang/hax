/*
 * haxAssem.c --
 *
 *	This file contains procedures to help assemble Hax commands
 *	from an input source  where commands may arrive in pieces, e.g.
 *	several lines of type-in corresponding to one command.
 *
 * Copyright 1990-1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclAssem.c,v 1.13 93/01/29 10:14:45 ouster Exp $ SPRITE (Berkeley)";
#endif /* not lint */

#include "haxInt.h"

/*
 * The structure below is the internal representation for a command
 * buffer, which is used to hold a piece of a command until a full
 * command is available.  When a full command is available, it will
 * be returned to the user, but it will also be retained in the buffer
 * until the NEXT call to Hax_AssembleCmd, at which point it will be
 * removed.
 */

typedef struct {
    char *buffer;		/* Storage for command being assembled.
				 * Malloc-ed, and grows as needed. */
    int bufSize;		/* Total number of bytes in buffer. */
    int bytesUsed;		/* Number of bytes in buffer currently
				 * occupied (0 means there is not a
				 * buffered incomplete command). */
} CmdBuf;

/*
 * Default amount of space to allocate in command buffer:
 */

#define CMD_BUF_SIZE 100

/*
 *----------------------------------------------------------------------
 *
 * Hax_CreateCmdBuf --
 *
 *	Allocate and initialize a command buffer.
 *
 * Results:
 *	The return value is a token that may be passed to
 *	Hax_AssembleCmd and Hax_DeleteCmdBuf.
 *
 * Side effects:
 *	Memory is allocated.
 *
 *----------------------------------------------------------------------
 */

Hax_CmdBuf
Hax_CreateCmdBuf(
    Hax_Interp *interp)
{
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;
    CmdBuf *cbPtr;

    cbPtr = (CmdBuf *) ckalloc(memoryp, sizeof(CmdBuf));
    cbPtr->buffer = (char *) ckalloc(memoryp, CMD_BUF_SIZE);
    cbPtr->buffer[0] = '\0';
    cbPtr->bufSize = CMD_BUF_SIZE;
    cbPtr->bytesUsed = 0;
    return (Hax_CmdBuf) cbPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_DeleteCmdBuf --
 *
 *	Release all of the resources associated with a command buffer.
 *	The caller should never again use buffer again.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory is released.
 *
 *----------------------------------------------------------------------
 */

void
Hax_DeleteCmdBuf(
    Hax_Interp *interp,
    Hax_CmdBuf buffer		/* Token for command buffer (return value
				 * from previous call to Hax_CreateCmdBuf). */)
{
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;
    CmdBuf *cbPtr = (CmdBuf *) buffer;

    ckfree(memoryp, cbPtr->buffer);
    ckfree(memoryp, (char *) cbPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_AssembleCmd --
 *
 *	This is a utility procedure to assist in situations where
 *	commands may be read piece-meal from some input source.  Given
 *	some input text, it adds the text to an input buffer and returns
 *	whole commands when they are ready.
 *
 * Results:
 *	If the addition of string to any currently-buffered information
 *	results in one or more complete Hax commands, then the return value
 *	is a pointer to the complete command(s).  The command value will
 *	only be valid until the next call to this procedure with the
 *	same buffer.  If the addition of string leaves an incomplete
 *	command at the end of the buffer, then NULL is returned.
 *
 * Side effects:
 *	If string leaves a command incomplete, the partial command
 *	information is buffered for use in later calls to this procedure.
 *	Once a command has been returned, that command is deleted from
 *	the buffer on the next call to this procedure.
 *
 *----------------------------------------------------------------------
 */

char *
Hax_AssembleCmd(
    Hax_Interp *interp,
    Hax_CmdBuf buffer,		/* Token for a command buffer previously
				 * created by Hax_CreateCmdBuf.  */
    char *string		/* Bytes to be appended to command stream.
				 * Note:  if the string is zero length,
				 * then whatever is buffered will be
				 * considered to be a complete command
				 * regardless of whether parentheses are
				 * matched or not. */)
{
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;
    CmdBuf *cbPtr = (CmdBuf *) buffer;
    int length, totalLength, c;

    /*
     * If an empty string is passed in, just pretend the current
     * command is complete, whether it really is or not.
     */

    length = strlen(string);
    if (length == 0) {
	cbPtr->buffer[cbPtr->bytesUsed] = 0;
	cbPtr->bytesUsed = 0;
	return cbPtr->buffer;
    }

    /*
     * Add the new information to the buffer.  If the current buffer
     * isn't large enough, grow it by at least a factor of two, or
     * enough to hold the new text.
     */

    length = strlen(string);
    totalLength = cbPtr->bytesUsed + length + 1;
    if (totalLength > cbPtr->bufSize) {
	unsigned int newSize;
	char *newBuf;

	newSize = cbPtr->bufSize*2;
	if (newSize < totalLength) {
	    newSize = totalLength;
	}
	newBuf = (char *) ckalloc(memoryp, newSize);
	strcpy(newBuf, cbPtr->buffer);
	ckfree(memoryp, cbPtr->buffer);
	cbPtr->buffer = newBuf;
	cbPtr->bufSize = newSize;
    }
    strcpy(cbPtr->buffer+cbPtr->bytesUsed, string);
    cbPtr->bytesUsed += length;

    /*
     * See if there is now a complete command in the buffer.
     */

    c = cbPtr->buffer[cbPtr->bytesUsed-1];
    if ((c != '\n') && (c != ';')) {
	return NULL;
    }
    if (Hax_CommandComplete(cbPtr->buffer)) {
	cbPtr->bytesUsed = 0;
	return cbPtr->buffer;
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_CommandComplete --
 *
 *	Given a partial or complete Hax command, this procedure
 *	determines whether the command is complete in the sense
 *	of having matched braces and quotes and brackets.
 *
 * Results:
 *	1 is returned if the command is complete, 0 otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Hax_CommandComplete(
    char *cmd			/* Command to check. */)
{
    char *p = cmd;

    p = cmd;
    while (1) {
	while (isspace(*p)) {
	    p++;
	}
	if (*p == 0) {
	    return 1;
	}
	p = HaxWordEnd(p, 0);
	if (*p == 0) {
	    return 0;
	}
	p++;
    }
}
