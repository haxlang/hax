/* 
 * haxCmdIL.c --
 *
 *	This file contains the top-level command routines for most of
 *	the Hax built-in commands whose names begin with the letters
 *	I through L.  It contains only commands in the generic core
 *	(i.e. those that don't depend much upon UNIX facilities).
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
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclCmdIL.c,v 1.89 93/01/22 15:17:42 ouster Exp $ SPRITE (Berkeley)";
#endif

#include "haxInt.h"

/*
 * Forward declarations for procedures defined in this file:
 */

static int		SortCompareProc (const void *first,
			    const void *second);

/*
 *----------------------------------------------------------------------
 *
 * Hax_IfCmd --
 *
 *	This procedure is invoked to process the "if" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_IfCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int i, result, value;

    i = 1;
    while (1) {
	/*
	 * At this point in the loop, argv and argc refer to an expression
	 * to test, either for the main expression or an expression
	 * following an "elseif".  The arguments after the expression must
	 * be "then" (optional) and a script to execute if the expression is
	 * true.
	 */

	if (i >= argc) {
	    Hax_AppendResult(interp, "wrong # args: no expression after \"",
		    argv[i-1], "\" argument", (char *) NULL);
	    return HAX_ERROR;
	}
	result = Hax_ExprBoolean(interp, argv[i], &value);
	if (result != HAX_OK) {
	    return result;
	}
	i++;
	if ((i < argc) && (strcmp(argv[i], "then") == 0)) {
	    i++;
	}
	if (i >= argc) {
	    Hax_AppendResult(interp, "wrong # args: no script following \"",
		    argv[i-1], "\" argument", (char *) NULL);
	    return HAX_ERROR;
	}
	if (value) {
	    return Hax_Eval(interp, argv[i], 0, (char **) NULL);
	}

	/*
	 * The expression evaluated to false.  Skip the command, then
	 * see if there is an "else" or "elseif" clause.
	 */

	i++;
	if (i >= argc) {
	    return HAX_OK;
	}
	if ((argv[i][0] == 'e') && (strcmp(argv[i], "elseif") == 0)) {
	    i++;
	    continue;
	}
	break;
    }

    /*
     * Couldn't find a "then" or "elseif" clause to execute.  Check now
     * for an "else" clause.  We know that there's at least one more
     * argument when we get here.
     */

    if (strcmp(argv[i], "else") == 0) {
	i++;
	if (i >= argc) {
	    Hax_AppendResult(interp,
		    "wrong # args: no script following \"else\" argument",
		    (char *) NULL);
	    return HAX_ERROR;
	}
    }
    return Hax_Eval(interp, argv[i], 0, (char **) NULL);
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_IncrCmd --
 *
 *	This procedure is invoked to process the "incr" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

    /* ARGSUSED */
int
Hax_IncrCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int value;
    char *oldString, *result;
    char newString[30];

    if ((argc != 2) && (argc != 3)) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" varName ?increment?\"", (char *) NULL);
	return HAX_ERROR;
    }

    oldString = Hax_GetVar(interp, argv[1], HAX_LEAVE_ERR_MSG);
    if (oldString == NULL) {
	return HAX_ERROR;
    }
    if (Hax_GetInt(interp, oldString, &value) != HAX_OK) {
	Hax_AddErrorInfo(interp,
		(char *) "\n    (reading value of variable to increment)");
	return HAX_ERROR;
    }
    if (argc == 2) {
	value += 1;
    } else {
	int increment;

	if (Hax_GetInt(interp, argv[2], &increment) != HAX_OK) {
	    Hax_AddErrorInfo(interp,
		    (char *) "\n    (reading increment)");
	    return HAX_ERROR;
	}
	value += increment;
    }
    sprintf(newString, "%d", value);
    result = Hax_SetVar(interp, argv[1], newString, HAX_LEAVE_ERR_MSG);
    if (result == NULL) {
	return HAX_ERROR;
    }
    interp->result = result;
    return HAX_OK; 
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_InfoCmd --
 *
 *	This procedure is invoked to process the "info" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_InfoCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    Interp *iPtr = (Interp *) interp;
    int length;
    char c;
    Arg *argPtr;
    Proc *procPtr;
    Var *varPtr;
    Command *cmdPtr;
    Hax_HashEntry *hPtr;
    Hax_HashSearch search;

    if (argc < 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" option ?arg arg ...?\"", (char *) NULL);
	return HAX_ERROR;
    }
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'a') && (strncmp(argv[1], "args", length)) == 0) {
	if (argc != 3) {
	    Hax_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " args procname\"", (char *) NULL);
	    return HAX_ERROR;
	}
	procPtr = HaxFindProc(iPtr, argv[2]);
	if (procPtr == NULL) {
	    infoNoSuchProc:
	    Hax_AppendResult(interp, "\"", argv[2],
		    "\" isn't a procedure", (char *) NULL);
	    return HAX_ERROR;
	}
	for (argPtr = procPtr->argPtr; argPtr != NULL;
		argPtr = argPtr->nextPtr) {
	    Hax_AppendElement(interp, argPtr->name, 0);
	}
	return HAX_OK;
    } else if ((c == 'b') && (strncmp(argv[1], "body", length)) == 0) {
	if (argc != 3) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " body procname\"", (char *) NULL);
	    return HAX_ERROR;
	}
	procPtr = HaxFindProc(iPtr, argv[2]);
	if (procPtr == NULL) {
	    goto infoNoSuchProc;
	}
	iPtr->result = procPtr->command;
	return HAX_OK;
    } else if ((c == 'c') && (strncmp(argv[1], "cmdcount", length) == 0)
	    && (length >= 2)) {
	if (argc != 2) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " cmdcount\"", (char *) NULL);
	    return HAX_ERROR;
	}
	sprintf(iPtr->result, "%d", iPtr->cmdCount);
	return HAX_OK;
    } else if ((c == 'c') && (strncmp(argv[1], "commands", length) == 0)
	    && (length >= 4)) {
	if (argc > 3) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " commands [pattern]\"", (char *) NULL);
	    return HAX_ERROR;
	}
	for (hPtr = Hax_FirstHashEntry(&iPtr->commandTable, &search);
		hPtr != NULL; hPtr = Hax_NextHashEntry(&search)) {
	    char *name = Hax_GetHashKey(&iPtr->commandTable, hPtr);
	    if ((argc == 3) && !Hax_StringMatch(name, argv[2])) {
		continue;
	    }
	    Hax_AppendElement(interp, name, 0);
	}
	return HAX_OK;
    } else if ((c == 'c') && (strncmp(argv[1], "complete", length) == 0)
	    && (length >= 4)) {
	if (argc != 3) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " complete command\"", (char *) NULL);
	    return HAX_ERROR;
	}
	if (Hax_CommandComplete(argv[2])) {
	    interp->result = (char *) "1";
	} else {
	    interp->result = (char *) "0";
	}
	return HAX_OK;
    } else if ((c == 'd') && (strncmp(argv[1], "default", length)) == 0) {
	if (argc != 5) {
	    Hax_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " default procname arg varname\"",
		    (char *) NULL);
	    return HAX_ERROR;
	}
	procPtr = HaxFindProc(iPtr, argv[2]);
	if (procPtr == NULL) {
	    goto infoNoSuchProc;
	}
	for (argPtr = procPtr->argPtr; ; argPtr = argPtr->nextPtr) {
	    if (argPtr == NULL) {
		Hax_AppendResult(interp, "procedure \"", argv[2],
			"\" doesn't have an argument \"", argv[3],
			"\"", (char *) NULL);
		return HAX_ERROR;
	    }
	    if (strcmp(argv[3], argPtr->name) == 0) {
		if (argPtr->defValue != NULL) {
		    if (Hax_SetVar((Hax_Interp *) iPtr, argv[4],
			    argPtr->defValue, 0) == NULL) {
			defStoreError:
			Hax_AppendResult(interp,
				"couldn't store default value in variable \"",
				argv[4], "\"", (char *) NULL);
			return HAX_ERROR;
		    }
		    iPtr->result = (char *) "1";
		} else {
		    if (Hax_SetVar((Hax_Interp *) iPtr, argv[4], (char *) "", 0)
			    == NULL) {
			goto defStoreError;
		    }
		    iPtr->result = (char *) "0";
		}
		return HAX_OK;
	    }
	}
    } else if ((c == 'e') && (strncmp(argv[1], "exists", length) == 0)) {
	char *p;
	if (argc != 3) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " exists varName\"", (char *) NULL);
	    return HAX_ERROR;
	}
	p = Hax_GetVar((Hax_Interp *) iPtr, argv[2], 0);

	/*
	 * The code below handles the special case where the name is for
	 * an array:  Hax_GetVar will reject this since you can't read
	 * an array variable without an index.
	 */

	if (p == NULL) {
	    Hax_HashEntry *hPtr;
	    Var *varPtr;

	    if (strchr(argv[2], '(') != NULL) {
		noVar:
		iPtr->result = (char *) "0";
		return HAX_OK;
	    }
	    if (iPtr->varFramePtr == NULL) {
		hPtr = Hax_FindHashEntry(&iPtr->globalTable, argv[2]);
	    } else {
		hPtr = Hax_FindHashEntry(&iPtr->varFramePtr->varTable, argv[2]);
	    }
	    if (hPtr == NULL) {
		goto noVar;
	    }
	    varPtr = (Var *) Hax_GetHashValue(hPtr);
	    if (varPtr->flags & VAR_UPVAR) {
		varPtr = (Var *) Hax_GetHashValue(varPtr->value.upvarPtr);
	    }
	    if (!(varPtr->flags & VAR_ARRAY)) {
		goto noVar;
	    }
	}
	iPtr->result = (char *) "1";
	return HAX_OK;
    } else if ((c == 'g') && (strncmp(argv[1], "globals", length) == 0)) {
	char *name;

	if (argc > 3) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " globals [pattern]\"", (char *) NULL);
	    return HAX_ERROR;
	}
	for (hPtr = Hax_FirstHashEntry(&iPtr->globalTable, &search);
		hPtr != NULL; hPtr = Hax_NextHashEntry(&search)) {
	    varPtr = (Var *) Hax_GetHashValue(hPtr);
	    if (varPtr->flags & VAR_UNDEFINED) {
		continue;
	    }
	    name = Hax_GetHashKey(&iPtr->globalTable, hPtr);
	    if ((argc == 3) && !Hax_StringMatch(name, argv[2])) {
		continue;
	    }
	    Hax_AppendElement(interp, name, 0);
	}
	return HAX_OK;
    } else if ((c == 'l') && (strncmp(argv[1], "level", length) == 0)
	    && (length >= 2)) {
	if (argc == 2) {
	    if (iPtr->varFramePtr == NULL) {
		iPtr->result = (char *) "0";
	    } else {
		sprintf(iPtr->result, "%d", iPtr->varFramePtr->level);
	    }
	    return HAX_OK;
	} else if (argc == 3) {
	    int level;
	    CallFrame *framePtr;

	    if (Hax_GetInt(interp, argv[2], &level) != HAX_OK) {
		return HAX_ERROR;
	    }
	    if (level <= 0) {
		if (iPtr->varFramePtr == NULL) {
		    levelError:
		    Hax_AppendResult(interp, "bad level \"", argv[2],
			    "\"", (char *) NULL);
		    return HAX_ERROR;
		}
		level += iPtr->varFramePtr->level;
	    }
	    for (framePtr = iPtr->varFramePtr; framePtr != NULL;
		    framePtr = framePtr->callerVarPtr) {
		if (framePtr->level == level) {
		    break;
		}
	    }
	    if (framePtr == NULL) {
		goto levelError;
	    }
	    iPtr->result = Hax_Merge(framePtr->argc, framePtr->argv);
	    iPtr->freeProc = (Hax_FreeProc *) free;
	    return HAX_OK;
	}
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" level [number]\"", (char *) NULL);
	return HAX_ERROR;
    } else if ((c == 'l') && (strncmp(argv[1], "library", length) == 0)
	    && (length >= 2)) {
	if (argc != 2) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " library\"", (char *) NULL);
	    return HAX_ERROR;
	}
	interp->result = getenv("HAX_LIBRARY");
	if (interp->result == NULL) {
#ifdef HAX_LIBRARY
	    interp->result = (char *) HAX_LIBRARY;
#else
	    interp->result =
		(char *) "there is no Hax library at this installation";
	    return HAX_ERROR;
#endif
	}
	return HAX_OK;
    } else if ((c == 'l') && (strncmp(argv[1], "locals", length) == 0)
	    && (length >= 2)) {
	char *name;

	if (argc > 3) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " locals [pattern]\"", (char *) NULL);
	    return HAX_ERROR;
	}
	if (iPtr->varFramePtr == NULL) {
	    return HAX_OK;
	}
	for (hPtr = Hax_FirstHashEntry(&iPtr->varFramePtr->varTable, &search);
		hPtr != NULL; hPtr = Hax_NextHashEntry(&search)) {
	    varPtr = (Var *) Hax_GetHashValue(hPtr);
	    if (varPtr->flags & (VAR_UNDEFINED|VAR_UPVAR)) {
		continue;
	    }
	    name = Hax_GetHashKey(&iPtr->varFramePtr->varTable, hPtr);
	    if ((argc == 3) && !Hax_StringMatch(name, argv[2])) {
		continue;
	    }
	    Hax_AppendElement(interp, name, 0);
	}
	return HAX_OK;
    } else if ((c == 'p') && (strncmp(argv[1], "procs", length)) == 0) {
	if (argc > 3) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " procs [pattern]\"", (char *) NULL);
	    return HAX_ERROR;
	}
	for (hPtr = Hax_FirstHashEntry(&iPtr->commandTable, &search);
		hPtr != NULL; hPtr = Hax_NextHashEntry(&search)) {
	    char *name = Hax_GetHashKey(&iPtr->commandTable, hPtr);

	    cmdPtr = (Command *) Hax_GetHashValue(hPtr);
	    if (!HaxIsProc(cmdPtr)) {
		continue;
	    }
	    if ((argc == 3) && !Hax_StringMatch(name, argv[2])) {
		continue;
	    }
	    Hax_AppendElement(interp, name, 0);
	}
	return HAX_OK;
    } else if ((c == 's') && (strncmp(argv[1], "script", length) == 0)) {
	if (argc != 2) {
	    Hax_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " script\"", (char *) NULL);
	    return HAX_ERROR;
	}
	if (iPtr->scriptFile != NULL) {
	    interp->result = iPtr->scriptFile;
	}
	return HAX_OK;
    } else if ((c == 't') && (strncmp(argv[1], "tclversion", length) == 0)) {
	if (argc != 2) {
	    Hax_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " tclversion\"", (char *) NULL);
	    return HAX_ERROR;
	}

	/*
	 * Note:  HAX_VERSION below is expected to be set with a "-D"
	 * switch in the Makefile.
	 */

	strcpy(iPtr->result, HAX_VERSION);
	return HAX_OK;
    } else if ((c == 'v') && (strncmp(argv[1], "vars", length)) == 0) {
	Hax_HashTable *tablePtr;
	char *name;

	if (argc > 3) {
	    Hax_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " vars [pattern]\"", (char *) NULL);
	    return HAX_ERROR;
	}
	if (iPtr->varFramePtr == NULL) {
	    tablePtr = &iPtr->globalTable;
	} else {
	    tablePtr = &iPtr->varFramePtr->varTable;
	}
	for (hPtr = Hax_FirstHashEntry(tablePtr, &search);
		hPtr != NULL; hPtr = Hax_NextHashEntry(&search)) {
	    varPtr = (Var *) Hax_GetHashValue(hPtr);
	    if (varPtr->flags & VAR_UNDEFINED) {
		continue;
	    }
	    name = Hax_GetHashKey(tablePtr, hPtr);
	    if ((argc == 3) && !Hax_StringMatch(name, argv[2])) {
		continue;
	    }
	    Hax_AppendElement(interp, name, 0);
	}
	return HAX_OK;
    } else {
	Hax_AppendResult(interp, "bad option \"", argv[1],
		"\": should be args, body, cmdcount, commands, ",
		"complete, default, ",
		"exists, globals, level, library, locals, procs, ",
		"script, tclversion, or vars",
		(char *) NULL);
	return HAX_ERROR;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_JoinCmd --
 *
 *	This procedure is invoked to process the "join" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_JoinCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    char *joinString;
    char **listArgv;
    int listArgc, i;

    if (argc == 2) {
	joinString = (char *) " ";
    } else if (argc == 3) {
	joinString = argv[2];
    } else {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" list ?joinString?\"", (char *) NULL);
	return HAX_ERROR;
    }

    if (Hax_SplitList(interp, argv[1], &listArgc, &listArgv) != HAX_OK) {
	return HAX_ERROR;
    }
    for (i = 0; i < listArgc; i++) {
	if (i == 0) {
	    Hax_AppendResult(interp, listArgv[0], (char *) NULL);
	} else  {
	    Hax_AppendResult(interp, joinString, listArgv[i], (char *) NULL);
	}
    }
    ckfree((char *) listArgv);
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_LindexCmd --
 *
 *	This procedure is invoked to process the "lindex" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

    /* ARGSUSED */
int
Hax_LindexCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    char *p, *element;
    int index, size, parenthesized, result;

    if (argc != 3) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" list index\"", (char *) NULL);
	return HAX_ERROR;
    }
    if (Hax_GetInt(interp, argv[2], &index) != HAX_OK) {
	return HAX_ERROR;
    }
    if (index < 0) {
	return HAX_OK;
    }
    for (p = argv[1] ; index >= 0; index--) {
	result = HaxFindElement(interp, p, &element, &p, &size,
		&parenthesized);
	if (result != HAX_OK) {
	    return result;
	}
    }
    if (size == 0) {
	return HAX_OK;
    }
    if (size >= HAX_RESULT_SIZE) {
	interp->result = (char *) ckalloc((unsigned) size+1);
	interp->freeProc = (Hax_FreeProc *) free;
    }
    if (parenthesized) {
	memcpy(interp->result, element, size);
	interp->result[size] = 0;
    } else {
	HaxCopyAndCollapse(size, element, interp->result);
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_LinsertCmd --
 *
 *	This procedure is invoked to process the "linsert" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_LinsertCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    char *p, *element, savedChar;
    int i, index, count, result, size;

    if (argc < 4) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" list index element ?element ...?\"", (char *) NULL);
	return HAX_ERROR;
    }
    if (Hax_GetInt(interp, argv[2], &index) != HAX_OK) {
	return HAX_ERROR;
    }

    /*
     * Skip over the first "index" elements of the list, then add
     * all of those elements to the result.
     */

    size = 0;
    element = argv[1];
    for (count = 0, p = argv[1]; (count < index) && (*p != 0); count++) {
	result = HaxFindElement(interp, p, &element, &p, &size, (int *) NULL);
	if (result != HAX_OK) {
	    return result;
	}
    }
    if (*p == 0) {
	Hax_AppendResult(interp, argv[1], (char *) NULL);
    } else {
	char *end;

	end = element+size;
	if (element != argv[1]) {
	    while ((*end != 0) && !isspace(*end)) {
		end++;
	    }
	}
	savedChar = *end;
	*end = 0;
	Hax_AppendResult(interp, argv[1], (char *) NULL);
	*end = savedChar;
    }

    /*
     * Add the new list elements.
     */

    for (i = 3; i < argc; i++) {
	Hax_AppendElement(interp, argv[i], 0);
    }

    /*
     * Append the remainder of the original list.
     */

    if (*p != 0) {
	Hax_AppendResult(interp, " ", p, (char *) NULL);
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ListCmd --
 *
 *	This procedure is invoked to process the "list" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_ListCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    if (argc < 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" arg ?arg ...?\"", (char *) NULL);
	return HAX_ERROR;
    }
    interp->result = Hax_Merge(argc-1, argv+1);
    interp->freeProc = (Hax_FreeProc *) free;
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_LlengthCmd --
 *
 *	This procedure is invoked to process the "llength" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_LlengthCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int count, result;
    char *element, *p;

    if (argc != 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" list\"", (char *) NULL);
	return HAX_ERROR;
    }
    for (count = 0, p = argv[1]; *p != 0 ; count++) {
	result = HaxFindElement(interp, p, &element, &p, (int *) NULL,
		(int *) NULL);
	if (result != HAX_OK) {
	    return result;
	}
	if (*element == 0) {
	    break;
	}
    }
    sprintf(interp->result, "%d", count);
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_LrangeCmd --
 *
 *	This procedure is invoked to process the "lrange" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_LrangeCmd(
    ClientData notUsed,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int first, last, result;
    char *begin, *end, c, *dummy;
    int count;

    if (argc != 4) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" list first last\"", (char *) NULL);
	return HAX_ERROR;
    }
    if (Hax_GetInt(interp, argv[2], &first) != HAX_OK) {
	return HAX_ERROR;
    }
    if (first < 0) {
	first = 0;
    }
    if ((*argv[3] == 'e') && (strncmp(argv[3], "end", strlen(argv[3])) == 0)) {
	last = 1000000;
    } else {
	if (Hax_GetInt(interp, argv[3], &last) != HAX_OK) {
	    Hax_ResetResult(interp);
	    Hax_AppendResult(interp,
		    "expected integer or \"end\" but got \"",
		    argv[3], "\"", (char *) NULL);
	    return HAX_ERROR;
	}
    }
    if (first > last) {
	return HAX_OK;
    }

    /*
     * Extract a range of fields.
     */

    for (count = 0, begin = argv[1]; count < first; count++) {
	result = HaxFindElement(interp, begin, &dummy, &begin, (int *) NULL,
		(int *) NULL);
	if (result != HAX_OK) {
	    return result;
	}
	if (*begin == 0) {
	    break;
	}
    }
    for (count = first, end = begin; (count <= last) && (*end != 0);
	    count++) {
	result = HaxFindElement(interp, end, &dummy, &end, (int *) NULL,
		(int *) NULL);
	if (result != HAX_OK) {
	    return result;
	}
    }

    /*
     * Chop off trailing spaces.
     */

    while (isspace(end[-1])) {
	end--;
    }
    c = *end;
    *end = 0;
    Hax_SetResult(interp, begin, HAX_VOLATILE);
    *end = c;
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_LreplaceCmd --
 *
 *	This procedure is invoked to process the "lreplace" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_LreplaceCmd(
    ClientData notUsed,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    char *p1, *p2, *element, savedChar, *dummy;
    int i, first, last, count, result, size;

    if (argc < 4) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" list first last ?element element ...?\"", (char *) NULL);
	return HAX_ERROR;
    }
    if (Hax_GetInt(interp, argv[2], &first) != HAX_OK) {
	return HAX_ERROR;
    }
    if (HaxGetListIndex(interp, argv[3], &last) != HAX_OK) {
	return HAX_ERROR;
    }
    if (first < 0) {
	first = 0;
    }
    if (last < 0) {
	last = 0;
    }
    if (first > last) {
	Hax_AppendResult(interp, "first index must not be greater than second",
		(char *) NULL);
	return HAX_ERROR;
    }

    /*
     * Skip over the elements of the list before "first".
     */

    size = 0;
    element = argv[1];
    for (count = 0, p1 = argv[1]; (count < first) && (*p1 != 0); count++) {
	result = HaxFindElement(interp, p1, &element, &p1, &size,
		(int *) NULL);
	if (result != HAX_OK) {
	    return result;
	}
    }
    if (*p1 == 0) {
	Hax_AppendResult(interp, "list doesn't contain element ",
		argv[2], (char *) NULL);
	return HAX_ERROR;
    }

    /*
     * Skip over the elements of the list up through "last".
     */

    for (p2 = p1 ; (count <= last) && (*p2 != 0); count++) {
	result = HaxFindElement(interp, p2, &dummy, &p2, (int *) NULL,
		(int *) NULL);
	if (result != HAX_OK) {
	    return result;
	}
    }

    /*
     * Add the elements before "first" to the result.  Be sure to
     * include quote or brace characters that might terminate the
     * last of these elements.
     */

    p1 = element+size;
    if (element != argv[1]) {
	while ((*p1 != 0) && !isspace(*p1)) {
	    p1++;
	}
    }
    savedChar = *p1;
    *p1 = 0;
    Hax_AppendResult(interp, argv[1], (char *) NULL);
    *p1 = savedChar;

    /*
     * Add the new list elements.
     */

    for (i = 4; i < argc; i++) {
	Hax_AppendElement(interp, argv[i], 0);
    }

    /*
     * Append the remainder of the original list.
     */

    if (*p2 != 0) {
	if (*interp->result == 0) {
	    Hax_SetResult(interp, p2, HAX_VOLATILE);
	} else {
	    Hax_AppendResult(interp, " ", p2, (char *) NULL);
	}
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_LsearchCmd --
 *
 *	This procedure is invoked to process the "lsearch" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_LsearchCmd(
    ClientData notUsed,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int listArgc;
    char **listArgv;
    int i, match;

    if (argc != 3) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" list pattern\"", (char *) NULL);
	return HAX_ERROR;
    }
    if (Hax_SplitList(interp, argv[1], &listArgc, &listArgv) != HAX_OK) {
	return HAX_ERROR;
    }
    match = -1;
    for (i = 0; i < listArgc; i++) {
	if (Hax_StringMatch(listArgv[i], argv[2])) {
	    match = i;
	    break;
	}
    }
    sprintf(interp->result, "%d", match);
    ckfree((char *) listArgv);
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_LsortCmd --
 *
 *	This procedure is invoked to process the "lsort" Hax command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Hax result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Hax_LsortCmd(
    ClientData notUsed,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int listArgc;
    char **listArgv;

    if (argc != 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" list\"", (char *) NULL);
	return HAX_ERROR;
    }
    if (Hax_SplitList(interp, argv[1], &listArgc, &listArgv) != HAX_OK) {
	return HAX_ERROR;
    }
    qsort(listArgv, listArgc, sizeof (char *), SortCompareProc);
    interp->result = Hax_Merge(listArgc, listArgv);
    interp->freeProc = (Hax_FreeProc *) free;
    ckfree((char *) listArgv);
    return HAX_OK;
}

/*
 * The procedure below is called back by qsort to determine
 * the proper ordering between two elements.
 */

static int
SortCompareProc(
    const void *first, const void *second /* Elements to be compared. */)
{
    return strcmp(*((char **) first), *((char **) second));
}
