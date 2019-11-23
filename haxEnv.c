/*
 * haxEnv.c --
 *
 *	Hax support for environment variables, including a setenv
 *	procedure.
 *
 * Copyright 1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that this copyright
 * notice appears in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclEnv.c,v 1.10 93/02/01 16:23:26 ouster Exp $ SPRITE (Berkeley)";
#endif /* not lint */

#include "hax.h"
#include "haxUnix.h"

/*
 * Declarations for local procedures defined in this file:
 */

static void		EnvInit (Hax_Interp *interp,
			    UnixClientData *unixClientData);
static char *		EnvTraceProc (ClientData clientData,
			    Hax_Interp *interp, char *name1, char *name2,
			    int flags);

/*
 *----------------------------------------------------------------------
 *
 * HaxSetupEnv --
 *
 *	This procedure is invoked for an interpreter to make environment
 *	variables accessible from that interpreter via the "env"
 *	associative array.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The interpreter is added to a list of interpreters managed
 *	by us, so that its view of envariables can be kept consistent
 *	with the view in other interpreters.  If this is the first
 *	call to Hax_SetupEnv, then additional initialization happens,
 *	such as copying the environment to dynamically-allocated space
 *	for ease of management.
 *
 *----------------------------------------------------------------------
 */

void
HaxSetupEnv(
    Hax_Interp *interp,		/* Interpreter whose "env" array is to be
				 * managed. */
    UnixClientData *unixClientData	/* Unix Client Data */)
{
    Hax_Memoryp *memoryp;
    int i;

    /*
     * First, initialize our environment-related information, if
     * necessary.
     */

    if (unixClientData->environSize != 0) {
	Hax_Panic ((char *) "Unexpected initialized environ");
    }

    EnvInit(interp, unixClientData);

    /*
     * Next, get the pointer to Memoryp.
     */

    memoryp = Hax_GetMemoryp(interp);

    /*
     * Store the environment variable values into the interpreter's
     * "env" array, and arrange for us to be notified on future
     * writes and unsets to that array.
     */

    (void) Hax_UnsetVar2(interp, (char *) "env", (char *) NULL, HAX_GLOBAL_ONLY);
    for (i = 0; ; i++) {
	char *p, *p2;

	p = unixClientData->haxEnviron[i];
	if (p == NULL) {
	    break;
	}
	for (p2 = p; *p2 != '='; p2++) {
	    /* Empty loop body. */
	}
	*p2 = 0;
	(void) Hax_SetVar2(interp, (char *) "env", p, p2+1, HAX_GLOBAL_ONLY);
	*p2 = '=';
    }
    Hax_TraceVar2(interp, (char *) "env", (char *) NULL,
	    HAX_GLOBAL_ONLY | HAX_TRACE_WRITES | HAX_TRACE_UNSETS,
	    EnvTraceProc, (ClientData) unixClientData);
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_FindVariable --
 *
 *	Locate the entry in environ for a given name.
 *
 * Results:
 *	The return value is the index in environ of an entry with the
 *	name "name", or -1 if there is no such entry.   The integer at
 *	*lengthPtr is filled in with the length of name (if a matching
 *	entry is found) or the length of the environ array (if no matching
 *	entry is found).
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Hax_FindVariable(
    ClientData clientData,	/* Unix Client Data */
    char *name,			/* Name of desired environment variable. */
    int *lengthPtr		/* Used to return length of name (for
				 * successful searches) or number of non-NULL
				 * entries in environ (for unsuccessful
				 * searches). */)
{
    UnixClientData *unixClientData = (UnixClientData *) clientData;
    int i;
    const char *p1, *p2;

    if (unixClientData->environSize == 0) {
	Hax_Panic ((char *) "Unexpected uninitialized environ");
    }

    for (i = 0, p1 = unixClientData->haxEnviron[i]; p1 != NULL; i++,
	p1 = unixClientData->haxEnviron[i]) {
	for (p2 = name; *p2 == *p1; p1++, p2++) {
	    /* NULL loop body. */
	}
	if ((*p1 == '=') && (*p2 == '\0')) {
	    *lengthPtr = p2-name;
	    return i;
	}
    }
    *lengthPtr = i;
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_SetEnv --
 *
 *	Set an environment variable, replacing an existing value
 *	or creating a new variable if there doesn't exist a variable
 *	by the given name.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The environ array gets updated.
 *
 *----------------------------------------------------------------------
 */

int
Hax_SetEnv(
    Hax_Interp *interp,
    ClientData clientData,	/* Unix Client Data */
    char *name,			/* Name of variable whose value is to be
				 * set. */
    char *value,		/* New value for variable. */
    int overwrite)
{
    UnixClientData *unixClientData = (UnixClientData *) clientData;
    Hax_Memoryp *memoryp;
    int index, length, nameLength;
    char *p;

    if (unixClientData->environSize == 0) {
	Hax_Panic ((char *) "Unexpected uninitialized environ");
    }

    memoryp = Hax_GetMemoryp(interp);

    /*
     * Figure out where the entry is going to go.  If the name doesn't
     * already exist, enlarge the array if necessary to make room.  If
     * the name exists, free its old entry.
     */

    index = Hax_FindVariable((ClientData) unixClientData, name, &length);
    if (index == -1) {
	if ((length+2) > unixClientData->environSize) {
	    char **newEnviron;

	    newEnviron = (char **) ckalloc(memoryp, (unsigned)
		    ((length+5) * sizeof(char *)));
	    memcpy(newEnviron, environ,
		    length*sizeof(char *));
	    ckfree(memoryp, unixClientData->haxEnviron);
	    unixClientData->haxEnviron = newEnviron;
	    unixClientData->environSize = length+5;
	}
	index = length;
	unixClientData->haxEnviron[index+1] = NULL;
	nameLength = strlen(name);
    } else {
	/*
	 * Compare the new value to the existing value.  If they're
	 * the same then quit immediately (e.g. don't rewrite the
	 * value or propagate it to other interpeters).  Otherwise,
	 * when there are N interpreters there will be N! propagations
	 * of the same value among the interpreters.
	 */

	if (strcmp(value, unixClientData->haxEnviron[index]+length+1) == 0) {
	    return HAX_OK;
	}

	/*
	 * If the variable does exist, the argument overwrite is tested;
	 * if overwrite is zero, the variable is not reset, otherwise it is
	 * reset to the given value.
	 */
	if (!overwrite) {
	    return HAX_OK;
	}
	ckfree(memoryp, unixClientData->haxEnviron[index]);
	nameLength = length;
    }

    /*
     * Create a new entry and enter it into the table.
     */

    p = ckalloc(memoryp, nameLength + strlen(value) + 2);
    unixClientData->haxEnviron[index] = p;
    strcpy(p, name);
    p += nameLength;
    *p = '=';
    strcpy(p+1, value);

    Hax_SetVar2(interp, (char *) "env", (char *) name,
	p+1, HAX_GLOBAL_ONLY);

    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * PutEnv --
 *
 *	Set an environment variable.  Similar to setenv except that
 *	the information is passed in a single string of the form
 *	NAME=value, rather than as separate name strings.  This procedure
 *	is a stand-in for the standard UNIX procedure by the same name,
 *	so that applications using that procedure will interface
 *	properly to Hax.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The environ array gets updated.
 *
 *----------------------------------------------------------------------
 */

int
Hax_PutEnv(
    Hax_Interp *interp,
    ClientData clientData,	/* Unix Client Data */
    char *string		/* Info about environment variable in the
				 * form NAME=value. */)
{
    UnixClientData *unixClientData = (UnixClientData *) clientData;
    Hax_Memoryp *memoryp;
    int nameLength;
    char *name, *value;

    if (unixClientData->environSize == 0) {
	Hax_Panic ((char *) "Unexpected uninitialized environ");
    }

    if (string == NULL || strchr(name, '=') == NULL || string[0] == '=' ||
	strchr(strchr(name, '='), '=') != NULL /* two '=' detected */ ) {
	Hax_AppendResult(interp, "bad args: should be \""
                " NAME=value\"", (char *) NULL);
	return HAX_ERROR;
    }

    memoryp = Hax_GetMemoryp(interp);

    /*
     * Separate the string into name and value parts, then call
     * setenv to do all of the real work.
     */

    value = strchr(string, '=');
    if (value == NULL) {
	return 0;
    }
    nameLength = value - string;
    if (nameLength == 0) {
	return 0;
    }
    name = (char *) ckalloc(memoryp, nameLength+1);
    memcpy(name, string, nameLength);
    name[nameLength] = 0;
    Hax_SetEnv(interp, clientData, name, value+1, 1);
    ckfree(memoryp, name);
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_UnsetEnv --
 *
 *	Remove an environment variable, updating the "env" arrays
 *	in all interpreters managed by us.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Interpreters are updated, as is environ.
 *
 *----------------------------------------------------------------------
 */

int
Hax_UnsetEnv(
    Hax_Interp *interp,
    ClientData clientData,		/* Unix Client Data */
    char *name				/* Name of variable to remove. */)
{
    UnixClientData *unixClientData = (UnixClientData *) clientData;
    Hax_Memoryp *memoryp;
    int index, dummy;
    char **envPtr;

    if (unixClientData->environSize == 0) {
	Hax_Panic ((char *) "Unexpected uninitialized environ");
    }

    if (name == NULL || strchr(name, '=') != NULL) {
	Hax_AppendResult(interp, "invalid name: should be \""
                " name\"", (char *) NULL);
	return HAX_ERROR;
    }

    memoryp = Hax_GetMemoryp(interp);

    /*
     * Update the environ array.
     */

    index = Hax_FindVariable((ClientData) unixClientData, name, &dummy);
    if (index == -1) {
	return HAX_OK;
    }
    ckfree(memoryp, unixClientData->haxEnviron[index]);
    for (envPtr = unixClientData->haxEnviron+index+1; ; envPtr++) {
	envPtr[-1] = *envPtr;
	if (*envPtr == NULL) {
	    break;
	}
    }

    (void) Hax_UnsetVar2(interp, (char *) "env", (char *) name,
	HAX_GLOBAL_ONLY);

    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * EnvTraceProc --
 *
 *	This procedure is invoked whenever an environment variable
 *	is modified or deleted.  It propagates the change to the
 *	"environ" array and to any other interpreters for whom
 *	we're managing an "env" array.
 *
 * Results:
 *	Always returns NULL to indicate success.
 *
 * Side effects:
 *	Environment variable changes get propagated.  If the whole
 *	"env" array is deleted, then we stop managing things for
 *	this interpreter (usually this happens because the whole
 *	interpreter is being deleted).
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
static char *
EnvTraceProc(
    ClientData clientData,	/* Unix Client Data */
    Hax_Interp *interp,		/* Interpreter whose "env" variable is
				 * being modified. */
    char *name1,		/* Better be "env". */
    char *name2,		/* Name of variable being modified, or
				 * NULL if whole array is being deleted. */
    int flags			/* Indicates what's happening. */)
{
    UnixClientData *unixClientData = (UnixClientData *) clientData;
    Hax_Memoryp *memoryp;
    char *value;

    memoryp = Hax_GetMemoryp(interp);

    /*
     * First see if the whole "env" variable is being deleted.  If
     * so, just forget about this interpreter.
     */

    if (name2 == NULL) {
	if ((flags & (HAX_TRACE_UNSETS|HAX_TRACE_DESTROYED))
		!= (HAX_TRACE_UNSETS|HAX_TRACE_DESTROYED)) {
	    Hax_Panic((char *) "EnvTraceProc called with confusing arguments");
	}
	ckfree(memoryp, unixClientData->haxEnviron);
	unixClientData->environSize = 0;
	unixClientData->haxEnviron = NULL;
	if (unixClientData->destroyProc) {
	    (*unixClientData->destroyProc)(interp, unixClientData);
	}
	return NULL;
    }

    /*
     * If a value is being set, call setenv to do all of the work.
     */

    if (flags & HAX_TRACE_WRITES) {
	value = Hax_GetVar2(interp, (char *) "env", name2, HAX_GLOBAL_ONLY);
	Hax_SetEnv(interp, (ClientData) unixClientData, name2, value, 1);
	if (unixClientData->writeProc) {
	    (*unixClientData->writeProc)(interp, unixClientData, name2, value);
	}
    }

    if (flags & HAX_TRACE_UNSETS) {
	Hax_UnsetEnv(interp, (ClientData) unixClientData, name2);
	if (unixClientData->unsetProc) {
	    (*unixClientData->unsetProc)(interp, unixClientData, name2);
	}
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * EnvInit --
 *
 *	This procedure is called to initialize our management
 *	of the environ array.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Environ gets copied to malloc-ed storage, so that in
 *	the future we don't have to worry about which entries
 *	are malloc-ed and which are static.
 *
 *----------------------------------------------------------------------
 */

static void
EnvInit(
    Hax_Interp *interp,
    UnixClientData *unixClientData)
{
    Hax_Memoryp *memoryp;
    int i, length;

    if (unixClientData->environSize != 0) {
	Hax_Panic ((char *) "Unexpected double initialization of environ");
    }

    memoryp = Hax_GetMemoryp(interp);

    for (length = 0; environ[length] != NULL; length++) {
	/* Empty loop body. */
    }
    unixClientData->environSize = length+5;
    unixClientData->haxEnviron = ckalloc(memoryp,
		unixClientData->environSize * sizeof(char *));
    for (i = 0; i < length; i++) {
	unixClientData->haxEnviron[i] = ckalloc(memoryp,
		strlen(environ[i]) + 1);
	strcpy(unixClientData->haxEnviron[i], environ[i]);
    }
    unixClientData->haxEnviron[length] = NULL;
}

void
Hax_EnvTraceProc(
    Hax_Interp *interp,
    ClientData clientData,
    Hax_EnvWriteProc *writeProc,
    Hax_EnvUnsetProc *unsetProc,
    Hax_EnvDestroyProc *destroyProc)
{
    UnixClientData *unixClientData = (UnixClientData *) clientData;

    unixClientData->writeProc = writeProc;
    unixClientData->unsetProc = unsetProc;
    unixClientData->destroyProc = destroyProc;
}
