/*
 * haxProc.c --
 *
 *	This file contains routines that implement Hax procedures,
 *	including the "proc" and "uplevel" commands.
 *
 * Copyright 1987-1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclProc.c,v 1.60 92/09/14 15:42:07 ouster Exp $ SPRITE (Berkeley)";
#endif

#include "haxInt.h"

/*
 * Forward references to procedures defined later in this file:
 */

static  int	InterpProc (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
static  void	ProcDeleteProc (ClientData clientData);

/*
 *----------------------------------------------------------------------
 *
 * Hax_ProcCmd --
 *
 *	This procedure is invoked to process the "proc" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result value.
 *
 * Side effects:
 *	A new procedure gets created.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_ProcCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    Interp *iPtr = (Interp *) interp;
    Proc *procPtr;
    int result, argCount, i;
    char **argArray = NULL;
    Arg *lastArgPtr;
    Arg *argPtr = NULL;	/* Initialization not needed, but
					 * prevents compiler warning. */

    if (argc != 4) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" name args body\"", (char *) NULL);
	return HAX_ERROR;
    }

    procPtr = (Proc *) ckalloc(sizeof(Proc));
    procPtr->iPtr = iPtr;
    procPtr->command = (char *) ckalloc((unsigned) strlen(argv[3]) + 1);
    strcpy(procPtr->command, argv[3]);
    procPtr->argPtr = NULL;

    /*
     * Break up the argument list into argument specifiers, then process
     * each argument specifier.
     */

    result = Hax_SplitList(interp, argv[2], &argCount, &argArray);
    if (result != HAX_OK) {
	goto procError;
    }
    lastArgPtr = NULL;
    for (i = 0; i < argCount; i++) {
	int fieldCount, nameLength, valueLength;
	char **fieldValues;

	/*
	 * Now divide the specifier up into name and default.
	 */

	result = Hax_SplitList(interp, argArray[i], &fieldCount,
		&fieldValues);
	if (result != HAX_OK) {
	    goto procError;
	}
	if (fieldCount > 2) {
	    ckfree((char *) fieldValues);
	    Hax_AppendResult(interp,
		    "too many fields in argument specifier \"",
		    argArray[i], "\"", (char *) NULL);
	    result = HAX_ERROR;
	    goto procError;
	}
	if ((fieldCount == 0) || (*fieldValues[0] == 0)) {
	    ckfree((char *) fieldValues);
	    Hax_AppendResult(interp, "procedure \"", argv[1],
		    "\" has argument with no name", (char *) NULL);
	    result = HAX_ERROR;
	    goto procError;
	}
	nameLength = strlen(fieldValues[0]) + 1;
	if (fieldCount == 2) {
	    valueLength = strlen(fieldValues[1]) + 1;
	} else {
	    valueLength = 0;
	}
	argPtr = (Arg *) ckalloc((unsigned)
		(sizeof(Arg) - sizeof(argPtr->name) + nameLength
		+ valueLength));
	if (lastArgPtr == NULL) {
	    procPtr->argPtr = argPtr;
	} else {
	    lastArgPtr->nextPtr = argPtr;
	}
	lastArgPtr = argPtr;
	argPtr->nextPtr = NULL;
	strcpy(argPtr->name, fieldValues[0]);
	if (fieldCount == 2) {
	    argPtr->defValue = argPtr->name + nameLength;
	    strcpy(argPtr->defValue, fieldValues[1]);
	} else {
	    argPtr->defValue = NULL;
	}
	ckfree((char *) fieldValues);
    }

    Hax_CreateCommand(interp, argv[1], InterpProc, (ClientData) procPtr,
	    ProcDeleteProc);
    ckfree((char *) argArray);
    return HAX_OK;

    procError:
    ckfree(procPtr->command);
    while (procPtr->argPtr != NULL) {
	argPtr = procPtr->argPtr;
	procPtr->argPtr = argPtr->nextPtr;
	ckfree((char *) argPtr);
    }
    ckfree((char *) procPtr);
    if (argArray != NULL) {
	ckfree((char *) argArray);
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * HaxGetFrame --
 *
 *	Given a description of a procedure frame, such as the first
 *	argument to an "uplevel" or "upvar" command, locate the
 *	call frame for the appropriate level of procedure.
 *
 * Results:
 *	The return value is -1 if an error occurred in finding the
 *	frame (in this case an error message is left in interp->result).
 *	1 is returned if string was either a number or a number preceded
 *	by "#" and it specified a valid frame.  0 is returned if string
 *	isn't one of the two things above (in this case, the lookup
 *	acts as if string were "1").  The variable pointed to by
 *	framePtrPtr is filled in with the address of the desired frame
 *	(unless an error occurs, in which case it isn't modified).
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
HaxGetFrame(
    Hax_Interp *interp,		/* Interpreter in which to find frame. */
    char *string,		/* String describing frame. */
    CallFrame **framePtrPtr	/* Store pointer to frame here (or NULL
				 * if global frame indicated). */)
{
    Interp *iPtr = (Interp *) interp;
    int level, result;
    CallFrame *framePtr;

    if (iPtr->varFramePtr == NULL) {
	iPtr->result = (char *) "already at top level";
	return -1;
    }

    /*
     * Parse string to figure out which level number to go to.
     */

    result = 1;
    if (*string == '#') {
	if (Hax_GetInt(interp, string+1, &level) != HAX_OK) {
	    return -1;
	}
	if (level < 0) {
	    levelError:
	    Hax_AppendResult(interp, "bad level \"", string, "\"",
		    (char *) NULL);
	    return -1;
	}
    } else if (isdigit(*string)) {
	if (Hax_GetInt(interp, string, &level) != HAX_OK) {
	    return -1;
	}
	level = iPtr->varFramePtr->level - level;
    } else {
	level = iPtr->varFramePtr->level - 1;
	result = 0;
    }

    /*
     * Figure out which frame to use, and modify the interpreter so
     * its variables come from that frame.
     */

    if (level == 0) {
	framePtr = NULL;
    } else {
	for (framePtr = iPtr->varFramePtr; framePtr != NULL;
		framePtr = framePtr->callerVarPtr) {
	    if (framePtr->level == level) {
		break;
	    }
	}
	if (framePtr == NULL) {
	    goto levelError;
	}
    }
    *framePtrPtr = framePtr;
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_UplevelCmd --
 *
 *	This procedure is invoked to process the "uplevel" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result value.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_UplevelCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    Interp *iPtr = (Interp *) interp;
    int result;
    CallFrame *savedVarFramePtr, *framePtr;

    if (argc < 2) {
	uplevelSyntax:
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" ?level? command ?arg ...?\"", (char *) NULL);
	return HAX_ERROR;
    }

    /*
     * Find the level to use for executing the command.
     */

    result = HaxGetFrame(interp, argv[1], &framePtr);
    if (result == -1) {
	return HAX_ERROR;
    }
    argc -= (result+1);
    if (argc == 0) {
	goto uplevelSyntax;
    }
    argv += (result+1);

    /*
     * Modify the interpreter state to execute in the given frame.
     */

    savedVarFramePtr = iPtr->varFramePtr;
    iPtr->varFramePtr = framePtr;

    /*
     * Execute the residual arguments as a command.
     */

    if (argc == 1) {
	result = Hax_Eval(interp, NULL, argv[0], 0, (char **) NULL);
    } else {
	char *cmd;

	cmd = Hax_Concat(argc, argv);
	result = Hax_Eval(interp, NULL, cmd, 0, (char **) NULL);
	ckfree(cmd);
    }
    if (result == HAX_ERROR) {
	char msg[60];
	sprintf(msg, "\n    (\"uplevel\" body line %d)", interp->errorLine);
	Hax_AddErrorInfo(interp, msg);
    }

    /*
     * Restore the variable frame, and return.
     */

    iPtr->varFramePtr = savedVarFramePtr;
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * HaxFindProc --
 *
 *	Given the name of a procedure, return a pointer to the
 *	record describing the procedure.
 *
 * Results:
 *	NULL is returned if the name doesn't correspond to any
 *	procedure.  Otherwise the return value is a pointer to
 *	the procedure's record.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Proc *
HaxFindProc(
    Interp *iPtr,		/* Interpreter in which to look. */
    char *procName		/* Name of desired procedure. */)
{
    Hax_HashEntry *hPtr;
    Command *cmdPtr;

    hPtr = Hax_FindHashEntry(&iPtr->commandTable, procName);
    if (hPtr == NULL) {
	return NULL;
    }
    cmdPtr = (Command *) Hax_GetHashValue(hPtr);
    if (cmdPtr->proc != InterpProc) {
	return NULL;
    }
    return (Proc *) cmdPtr->clientData;
}

/*
 *----------------------------------------------------------------------
 *
 * HaxIsProc --
 *
 *	Tells whether a command is a Hax procedure or not.
 *
 * Results:
 *	If the given command is actuall a Hax procedure, the
 *	return value is the address of the record describing
 *	the procedure.  Otherwise the return value is 0.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Proc *
HaxIsProc(
    Command *cmdPtr		/* Command to test. */)
{
    if (cmdPtr->proc == InterpProc) {
	return (Proc *) cmdPtr->clientData;
    }
    return (Proc *) 0;
}

/*
 *----------------------------------------------------------------------
 *
 * InterpProc --
 *
 *	When a Hax procedure gets invoked, this routine gets invoked
 *	to interpret the procedure.
 *
 * Results:
 *	A standard Hax result value, usually HAX_OK.
 *
 * Side effects:
 *	Depends on the commands in the procedure.
 *
 *----------------------------------------------------------------------
 */

static int
InterpProc(
    ClientData clientData,	/* Record describing procedure to be
				 * interpreted. */
    Hax_Interp *interp,		/* Interpreter in which procedure was
				 * invoked. */
    int argc,			/* Count of number of arguments to this
				 * procedure. */
    char **argv			/* Argument values. */)
{
    Proc *procPtr = (Proc *) clientData;
    Arg *argPtr;
    Interp *iPtr = (Interp *) interp;
    char **args;
    CallFrame frame;
    char *value, *end;
    int result;

    /*
     * Set up a call frame for the new procedure invocation.
     */

    iPtr = procPtr->iPtr;
    Hax_InitHashTable(&frame.varTable, HAX_STRING_KEYS);
    if (iPtr->varFramePtr != NULL) {
	frame.level = iPtr->varFramePtr->level + 1;
    } else {
	frame.level = 1;
    }
    frame.argc = argc;
    frame.argv = argv;
    frame.callerPtr = iPtr->framePtr;
    frame.callerVarPtr = iPtr->varFramePtr;
    iPtr->framePtr = &frame;
    iPtr->varFramePtr = &frame;

    /*
     * Match the actual arguments against the procedure's formal
     * parameters to compute local variables.
     */

    for (argPtr = procPtr->argPtr, args = argv+1, argc -= 1;
	    argPtr != NULL;
	    argPtr = argPtr->nextPtr, args++, argc--) {

	/*
	 * Handle the special case of the last formal being "args".  When
	 * it occurs, assign it a list consisting of all the remaining
	 * actual arguments.
	 */

	if ((argPtr->nextPtr == NULL)
		&& (strcmp(argPtr->name, "args") == 0)) {
	    if (argc < 0) {
		argc = 0;
	    }
	    value = Hax_Merge(argc, args);
	    Hax_SetVar(interp, argPtr->name, value, 0);
	    ckfree(value);
	    argc = 0;
	    break;
	} else if (argc > 0) {
	    value = *args;
	} else if (argPtr->defValue != NULL) {
	    value = argPtr->defValue;
	} else {
	    Hax_AppendResult(interp, "no value given for parameter \"",
		    argPtr->name, "\" to \"", argv[0], "\"",
		    (char *) NULL);
	    result = HAX_ERROR;
	    goto procDone;
	}
	Hax_SetVar(interp, argPtr->name, value, 0);
    }
    if (argc > 0) {
	Hax_AppendResult(interp, "called \"", argv[0],
		"\" with too many arguments", (char *) NULL);
	result = HAX_ERROR;
	goto procDone;
    }

    /*
     * Invoke the commands in the procedure's body.
     */

    result = Hax_Eval(interp, NULL, procPtr->command, 0, &end);
    if (result == HAX_RETURN) {
	result = HAX_OK;
    } else if (result == HAX_ERROR) {
	char msg[100];

	/*
	 * Record information telling where the error occurred.
	 */

	sprintf(msg, "\n    (procedure \"%.50s\" line %d)", argv[0],
		iPtr->errorLine);
	Hax_AddErrorInfo(interp, msg);
    } else if (result == HAX_BREAK) {
	iPtr->result = (char *) "invoked \"break\" outside of a loop";
	result = HAX_ERROR;
    } else if (result == HAX_CONTINUE) {
	iPtr->result = (char *) "invoked \"continue\" outside of a loop";
	result = HAX_ERROR;
    }

    /*
     * Delete the call frame for this procedure invocation (it's
     * important to remove the call frame from the interpreter
     * before deleting it, so that traces invoked during the
     * deletion don't see the partially-deleted frame).
     */

    procDone:
    iPtr->framePtr = frame.callerPtr;
    iPtr->varFramePtr = frame.callerVarPtr;
    HaxDeleteVars(iPtr, &frame.varTable);
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * ProcDeleteProc --
 *
 *	This procedure is invoked just before a command procedure is
 *	removed from an interpreter.  Its job is to release all the
 *	resources allocated to the procedure.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory gets freed.
 *
 *----------------------------------------------------------------------
 */

static void
ProcDeleteProc(
    ClientData clientData		/* Procedure to be deleted. */)
{
    Proc *procPtr = (Proc *) clientData;
    Arg *argPtr;

    ckfree((char *) procPtr->command);
    for (argPtr = procPtr->argPtr; argPtr != NULL; ) {
	Arg *nextPtr = argPtr->nextPtr;

	ckfree((char *) argPtr);
	argPtr = nextPtr;
    }
    ckfree((char *) procPtr);
}
