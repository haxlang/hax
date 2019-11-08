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

#include "haxInt.h"
#include "haxUnix.h"

/*
 * The structure below is used to keep track of all of the interpereters
 * for which we're managing the "env" array.  It's needed so that they
 * can all be updated whenever an environment variable is changed
 * anywhere.
 */

typedef struct EnvInterp {
    Hax_Interp *interp;		/* Interpreter for which we're managing
				 * the env array. */
    struct EnvInterp *nextPtr;	/* Next in list of all such interpreters,
				 * or zero. */
} EnvInterp;

static EnvInterp *firstInterpPtr;
				/* First in list of all managed interpreters,
				 * or NULL if none. */

static int environSize = 0;	/* Non-zero means that the all of the
				 * environ-related information is malloc-ed
				 * and the environ array itself has this
				 * many total entries allocated to it (not
				 * all may be in use at once).  Zero means
				 * that the environment array is in its
				 * original static state. */

/*
 * Declarations for local procedures defined in this file:
 */

static void		EnvInit (void);
static char *		EnvTraceProc (ClientData clientData,
			    Hax_Interp *interp, char *name1, char *name2,
			    int flags);
static int		FindVariable (const char *name,
			    int *lengthPtr);
static void		SetEnv (const char *name,
			    const char *value);
static void		UnsetEnv (const char *name);

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
    Hax_Interp *interp		/* Interpreter whose "env" array is to be
				 * managed. */)
{
    EnvInterp *eiPtr;
    int i;

    /*
     * First, initialize our environment-related information, if
     * necessary.
     */

    if (environSize == 0) {
	EnvInit();
    }

    /*
     * Next, add the interpreter to the list of those that we manage.
     */

    eiPtr = (EnvInterp *) ckalloc(sizeof(EnvInterp));
    eiPtr->interp = interp;
    eiPtr->nextPtr = firstInterpPtr;
    firstInterpPtr = eiPtr;

    /*
     * Store the environment variable values into the interpreter's
     * "env" array, and arrange for us to be notified on future
     * writes and unsets to that array.
     */

    (void) Hax_UnsetVar2(interp, "env", (char *) NULL, HAX_GLOBAL_ONLY);
    for (i = 0; ; i++) {
	char *p, *p2;

	p = environ[i];
	if (p == NULL) {
	    break;
	}
	for (p2 = p; *p2 != '='; p2++) {
	    /* Empty loop body. */
	}
	*p2 = 0;
	(void) Hax_SetVar2(interp, "env", p, p2+1, HAX_GLOBAL_ONLY);
	*p2 = '=';
    }
    Hax_TraceVar2(interp, "env", (char *) NULL,
	    HAX_GLOBAL_ONLY | HAX_TRACE_WRITES | HAX_TRACE_UNSETS,
	    EnvTraceProc, (ClientData) NULL);
}

/*
 *----------------------------------------------------------------------
 *
 * FindVariable --
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

static int
FindVariable(
    const char *name,		/* Name of desired environment variable. */
    int *lengthPtr		/* Used to return length of name (for
				 * successful searches) or number of non-NULL
				 * entries in environ (for unsuccessful
				 * searches). */)
{
    int i;
    const char *p1, *p2;

    for (i = 0, p1 = environ[i]; p1 != NULL; i++, p1 = environ[i]) {
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
 * SetEnv --
 *
 *	Set an environment variable, replacing an existing value
 *	or creating a new variable if there doesn't exist a variable
 *	by the given name.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The environ array gets updated, as do all of the interpreters
 *	that we manage.
 *
 *----------------------------------------------------------------------
 */

void
SetEnv(
    const char *name,		/* Name of variable whose value is to be
				 * set. */
    const char *value		/* New value for variable. */)
{
    int index, length, nameLength;
    char *p;
    EnvInterp *eiPtr;

    if (environSize == 0) {
	EnvInit();
    }

    /*
     * Figure out where the entry is going to go.  If the name doesn't
     * already exist, enlarge the array if necessary to make room.  If
     * the name exists, free its old entry.
     */

    index = FindVariable(name, &length);
    if (index == -1) {
	if ((length+2) > environSize) {
	    char **newEnviron;

	    newEnviron = (char **) ckalloc((unsigned)
		    ((length+5) * sizeof(char *)));
	    memcpy(newEnviron, environ,
		    length*sizeof(char *));
	    ckfree(environ);
	    environ = newEnviron;
	    environSize = length+5;
	}
	index = length;
	environ[index+1] = NULL;
	nameLength = strlen(name);
    } else {
	/*
	 * Compare the new value to the existing value.  If they're
	 * the same then quit immediately (e.g. don't rewrite the
	 * value or propagate it to other interpeters).  Otherwise,
	 * when there are N interpreters there will be N! propagations
	 * of the same value among the interpreters.
	 */

	if (strcmp(value, environ[index]+length+1) == 0) {
	    return;
	}
	ckfree(environ[index]);
	nameLength = length;
    }

    /*
     * Create a new entry and enter it into the table.
     */

    p = (char *) ckalloc((unsigned) (nameLength + strlen(value) + 2));
    environ[index] = p;
    strcpy(p, name);
    p += nameLength;
    *p = '=';
    strcpy(p+1, value);

    /*
     * Update all of the interpreters.
     */

    for (eiPtr= firstInterpPtr; eiPtr != NULL; eiPtr = eiPtr->nextPtr) {
	(void) Hax_SetVar2(eiPtr->interp, "env", (char *) name,
		p+1, HAX_GLOBAL_ONLY);
    }
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
 *	The environ array gets updated, as do all of the interpreters
 *	that we manage.
 *
 *----------------------------------------------------------------------
 */

int
PutEnv(
    char *string		/* Info about environment variable in the
				 * form NAME=value. */)
{
    int nameLength;
    char *name, *value;

    if (string == NULL) {
	return 0;
    }

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
    name = ckalloc(nameLength+1);
    memcpy(name, string, nameLength);
    name[nameLength] = 0;
    SetEnv(name, value+1);
    ckfree(name);
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * UnsetEnv --
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

void
UnsetEnv(
    const char *name			/* Name of variable to remove. */)
{
    int index, dummy;
    char **envPtr;
    EnvInterp *eiPtr;

    if (environSize == 0) {
	EnvInit();
    }

    /*
     * Update the environ array.
     */

    index = FindVariable(name, &dummy);
    if (index == -1) {
	return;
    }
    ckfree(environ[index]);
    for (envPtr = environ+index+1; ; envPtr++) {
	envPtr[-1] = *envPtr;
	if (*envPtr == NULL) {
	    break;
       }
    }

    /*
     * Update all of the interpreters.
     */

    for (eiPtr = firstInterpPtr; eiPtr != NULL; eiPtr = eiPtr->nextPtr) {
	(void) Hax_UnsetVar2(eiPtr->interp, "env", (char *) name,
		HAX_GLOBAL_ONLY);
    }
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
    ClientData clientData,	/* Not used. */
    Hax_Interp *interp,		/* Interpreter whose "env" variable is
				 * being modified. */
    char *name1,		/* Better be "env". */
    char *name2,		/* Name of variable being modified, or
				 * NULL if whole array is being deleted. */
    int flags			/* Indicates what's happening. */)
{
    /*
     * First see if the whole "env" variable is being deleted.  If
     * so, just forget about this interpreter.
     */

    if (name2 == NULL) {
	EnvInterp *eiPtr, *prevPtr;

	if ((flags & (HAX_TRACE_UNSETS|HAX_TRACE_DESTROYED))
		!= (HAX_TRACE_UNSETS|HAX_TRACE_DESTROYED)) {
	    Hax_Panic("EnvTraceProc called with confusing arguments");
	}
	eiPtr = firstInterpPtr;
	if (eiPtr->interp == interp) {
	    firstInterpPtr = eiPtr->nextPtr;
	} else {
	    for (prevPtr = eiPtr, eiPtr = eiPtr->nextPtr; ;
		    prevPtr = eiPtr, eiPtr = eiPtr->nextPtr) {
		if (eiPtr == NULL) {
		    Hax_Panic("EnvTraceProc couldn't find interpreter");
		}
		if (eiPtr->interp == interp) {
		    prevPtr->nextPtr = eiPtr->nextPtr;
		    break;
		}
	    }
	}
	ckfree((char *) eiPtr);
	return NULL;
    }

    /*
     * If a value is being set, call setenv to do all of the work.
     */

    if (flags & HAX_TRACE_WRITES) {
	SetEnv(name2, Hax_GetVar2(interp, "env", name2, HAX_GLOBAL_ONLY));
    }

    if (flags & HAX_TRACE_UNSETS) {
	UnsetEnv(name2);
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
EnvInit(void)
{
    char **newEnviron;
    int i, length;

    if (environSize != 0) {
	return;
    }
    for (length = 0; environ[length] != NULL; length++) {
	/* Empty loop body. */
    }
    environSize = length+5;
    newEnviron = (char **) ckalloc((unsigned)
		(environSize * sizeof(char *)));
    for (i = 0; i < length; i++) {
	newEnviron[i] = (char *) ckalloc((unsigned) (strlen(environ[i]) + 1));
	strcpy(newEnviron[i], environ[i]);
    }
    newEnviron[length] = NULL;
    environ = newEnviron;
}
