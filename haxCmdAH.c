/* 
 * haxCmdAH.c --
 *
 *	This file contains the top-level command routines for most of
 *	the Hax built-in commands whose names begin with the letters
 *	A to H.
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
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclCmdAH.c,v 1.79 93/01/29 14:36:00 ouster Exp $ SPRITE (Berkeley)";
#endif

#include "haxInt.h"


/*
 *----------------------------------------------------------------------
 *
 * Hax_BreakCmd --
 *
 *	This procedure is invoked to process the "break" Hax command.
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
Hax_BreakCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    if (argc != 1) {
	Hax_AppendResult(interp, "wrong # args: should be \"",
		argv[0], "\"", (char *) NULL);
	return HAX_ERROR;
    }
    return HAX_BREAK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_CaseCmd --
 *
 *	This procedure is invoked to process the "case" Hax command.
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
Hax_CaseCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int i, result;
    int body;
    char *string;
    int caseArgc, splitArgs;
    char **caseArgv;

    if (argc < 3) {
	Hax_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " string ?in? patList body ... ?default body?\"",
		(char *) NULL);
	return HAX_ERROR;
    }
    string = argv[1];
    body = -1;
    if (strcmp(argv[2], "in") == 0) {
	i = 3;
    } else {
	i = 2;
    }
    caseArgc = argc - i;
    caseArgv = argv + i;

    /*
     * If all of the pattern/command pairs are lumped into a single
     * argument, split them out again.
     */

    splitArgs = 0;
    if (caseArgc == 1) {
	result = Hax_SplitList(interp, caseArgv[0], &caseArgc, &caseArgv);
	if (result != HAX_OK) {
	    return result;
	}
	splitArgs = 1;
    }

    for (i = 0; i < caseArgc; i += 2) {
	int patArgc, j;
	char **patArgv;
	char *p;

	if (i == (caseArgc-1)) {
	    interp->result = (char *) "extra case pattern with no body";
	    result = HAX_ERROR;
	    goto cleanup;
	}

	/*
	 * Check for special case of single pattern (no list) with
	 * no backslash sequences.
	 */

	for (p = caseArgv[i]; *p != 0; p++) {
	    if (isspace(*p) || (*p == '\\')) {
		break;
	    }
	}
	if (*p == 0) {
	    if ((*caseArgv[i] == 'd')
		    && (strcmp(caseArgv[i], "default") == 0)) {
		body = i+1;
	    }
	    if (Hax_StringMatch(string, caseArgv[i])) {
		body = i+1;
		goto match;
	    }
	    continue;
	}

	/*
	 * Break up pattern lists, then check each of the patterns
	 * in the list.
	 */

	result = Hax_SplitList(interp, caseArgv[i], &patArgc, &patArgv);
	if (result != HAX_OK) {
	    goto cleanup;
	}
	for (j = 0; j < patArgc; j++) {
	    if (Hax_StringMatch(string, patArgv[j])) {
		body = i+1;
		break;
	    }
	}
	ckfree((char *) patArgv);
	if (j < patArgc) {
	    break;
	}
    }

    match:
    if (body != -1) {
	result = Hax_Eval(interp, caseArgv[body], 0, (char **) NULL);
	if (result == HAX_ERROR) {
	    char msg[100];
	    sprintf(msg, "\n    (\"%.50s\" arm line %d)", caseArgv[body-1],
		    interp->errorLine);
	    Hax_AddErrorInfo(interp, msg);
	}
	goto cleanup;
    }

    /*
     * Nothing matched:  return nothing.
     */

    result = HAX_OK;

    cleanup:
    if (splitArgs) {
	ckfree((char *) caseArgv);
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_CatchCmd --
 *
 *	This procedure is invoked to process the "catch" Hax command.
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
Hax_CatchCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int result;

    if ((argc != 2) && (argc != 3)) {
	Hax_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " command ?varName?\"", (char *) NULL);
	return HAX_ERROR;
    }
    result = Hax_Eval(interp, argv[1], 0, (char **) NULL);
    if (argc == 3) {
	if (Hax_SetVar(interp, argv[2], interp->result, 0) == NULL) {
	    Hax_SetResult(interp,
		    (char *) "couldn't save command result in variable",
		    HAX_STATIC);
	    return HAX_ERROR;
	}
    }
    Hax_ResetResult(interp);
    sprintf(interp->result, "%d", result);
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ConcatCmd --
 *
 *	This procedure is invoked to process the "concat" Hax command.
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
Hax_ConcatCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    if (argc == 1) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" arg ?arg ...?\"", (char *) NULL);
	return HAX_ERROR;
    }

    interp->result = Hax_Concat(argc-1, argv+1);
    interp->freeProc = (Hax_FreeProc *) free;
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ContinueCmd --
 *
 *	This procedure is invoked to process the "continue" Hax command.
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
Hax_ContinueCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    if (argc != 1) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		"\"", (char *) NULL);
	return HAX_ERROR;
    }
    return HAX_CONTINUE;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ErrorCmd --
 *
 *	This procedure is invoked to process the "error" Hax command.
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
Hax_ErrorCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    Interp *iPtr = (Interp *) interp;

    if ((argc < 2) || (argc > 4)) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" message ?errorInfo? ?errorCode?\"", (char *) NULL);
	return HAX_ERROR;
    }
    if ((argc >= 3) && (argv[2][0] != 0)) {
	Hax_AddErrorInfo(interp, argv[2]);
	iPtr->flags |= ERR_ALREADY_LOGGED;
    }
    if (argc == 4) {
	Hax_SetVar2(interp, (char *) "errorCode", (char *) NULL, argv[3],
		HAX_GLOBAL_ONLY);
	iPtr->flags |= ERROR_CODE_SET;
    }
    Hax_SetResult(interp, argv[1], HAX_VOLATILE);
    return HAX_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_EvalCmd --
 *
 *	This procedure is invoked to process the "eval" Hax command.
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
Hax_EvalCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int result;
    char *cmd;

    if (argc < 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" arg ?arg ...?\"", (char *) NULL);
	return HAX_ERROR;
    }
    if (argc == 2) {
	result = Hax_Eval(interp, argv[1], 0, (char **) NULL);
    } else {
    
	/*
	 * More than one argument:  concatenate them together with spaces
	 * between, then evaluate the result.
	 */
    
	cmd = Hax_Concat(argc-1, argv+1);
	result = Hax_Eval(interp, cmd, 0, (char **) NULL);
	ckfree(cmd);
    }
    if (result == HAX_ERROR) {
	char msg[60];
	sprintf(msg, "\n    (\"eval\" body line %d)", interp->errorLine);
	Hax_AddErrorInfo(interp, msg);
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ExprCmd --
 *
 *	This procedure is invoked to process the "expr" Hax command.
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
Hax_ExprCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    if (argc != 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" expression\"", (char *) NULL);
	return HAX_ERROR;
    }

    return Hax_ExprString(interp, argv[1]);
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ForCmd --
 *
 *	This procedure is invoked to process the "for" Hax command.
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
Hax_ForCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int result, value;

    if (argc != 5) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" start test next command\"", (char *) NULL);
	return HAX_ERROR;
    }

    result = Hax_Eval(interp, argv[1], 0, (char **) NULL);
    if (result != HAX_OK) {
	if (result == HAX_ERROR) {
	    Hax_AddErrorInfo(interp,
		(char *) "\n    (\"for\" initial command)");
	}
	return result;
    }
    while (1) {
	result = Hax_ExprBoolean(interp, argv[2], &value);
	if (result != HAX_OK) {
	    return result;
	}
	if (!value) {
	    break;
	}
	result = Hax_Eval(interp, argv[4], 0, (char **) NULL);
	if (result == HAX_CONTINUE) {
	    result = HAX_OK;
	} else if (result != HAX_OK) {
	    if (result == HAX_ERROR) {
		char msg[60];
		sprintf(msg, "\n    (\"for\" body line %d)", interp->errorLine);
		Hax_AddErrorInfo(interp, msg);
	    }
	    break;
	}
	result = Hax_Eval(interp, argv[3], 0, (char **) NULL);
	if (result == HAX_BREAK) {
	    break;
	} else if (result != HAX_OK) {
	    if (result == HAX_ERROR) {
		Hax_AddErrorInfo(interp,
		    (char *) "\n    (\"for\" loop-end command)");
	    }
	    return result;
	}
    }
    if (result == HAX_BREAK) {
	result = HAX_OK;
    }
    if (result == HAX_OK) {
	Hax_ResetResult(interp);
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ForeachCmd --
 *
 *	This procedure is invoked to process the "foreach" Hax command.
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
Hax_ForeachCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int listArgc, i, result;
    char **listArgv;

    if (argc != 4) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" varName list command\"", (char *) NULL);
	return HAX_ERROR;
    }

    /*
     * Break the list up into elements, and execute the command once
     * for each value of the element.
     */

    result = Hax_SplitList(interp, argv[2], &listArgc, &listArgv);
    if (result != HAX_OK) {
	return result;
    }
    for (i = 0; i < listArgc; i++) {
	if (Hax_SetVar(interp, argv[1], listArgv[i], 0) == NULL) {
	    Hax_SetResult(interp, (char *) "couldn't set loop variable",
		    HAX_STATIC);
	    result = HAX_ERROR;
	    break;
	}

	result = Hax_Eval(interp, argv[3], 0, (char **) NULL);
	if (result != HAX_OK) {
	    if (result == HAX_CONTINUE) {
		result = HAX_OK;
	    } else if (result == HAX_BREAK) {
		result = HAX_OK;
		break;
	    } else if (result == HAX_ERROR) {
		char msg[100];
		sprintf(msg, "\n    (\"foreach\" body line %d)",
			interp->errorLine);
		Hax_AddErrorInfo(interp, msg);
		break;
	    } else {
		break;
	    }
	}
    }
    ckfree((char *) listArgv);
    if (result == HAX_OK) {
	Hax_ResetResult(interp);
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_FormatCmd --
 *
 *	This procedure is invoked to process the "format" Hax command.
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
Hax_FormatCmd(
    ClientData dummy,			/* Not used. */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    char *format;	/* Used to read characters from the format
				 * string. */
    char newFormat[40];		/* A new format specifier is generated here. */
    int width;			/* Field width from field specifier, or 0 if
				 * no width given. */
    int precision;		/* Field precision from field specifier, or 0
				 * if no precision given. */
    int size;			/* Number of bytes needed for result of
				 * conversion, based on type of conversion
				 * ("e", "s", etc.) and width from above. */
    char *oneWordValue = NULL;	/* Used to hold value to pass to sprintf, if
				 * it's a one-word value. */
    double twoWordValue;	/* Used to hold value to pass to sprintf if
				 * it's a two-word value. */
    int useTwoWords;		/* 0 means use oneWordValue, 1 means use
				 * twoWordValue. */
    char *dst = interp->result;	/* Where result is stored.  Starts off at
				 * interp->resultSpace, but may get dynamically
				 * re-allocated if this isn't enough. */
    int dstSize = 0;		/* Number of non-null characters currently
				 * stored at dst. */
    int dstSpace = HAX_RESULT_SIZE;
				/* Total amount of storage space available
				 * in dst (not including null terminator. */
    int noPercent;		/* Special case for speed:  indicates there's
				 * no field specifier, just a string to copy. */
    char **curArg;		/* Remainder of argv array. */
    int useShort;		/* Value to be printed is short (half word). */

    /*
     * This procedure is a bit nasty.  The goal is to use sprintf to
     * do most of the dirty work.  There are several problems:
     * 1. this procedure can't trust its arguments.
     * 2. we must be able to provide a large enough result area to hold
     *    whatever's generated.  This is hard to estimate.
     * 2. there's no way to move the arguments from argv to the call
     *    to sprintf in a reasonable way.  This is particularly nasty
     *    because some of the arguments may be two-word values (doubles).
     * So, what happens here is to scan the format string one % group
     * at a time, making many individual calls to sprintf.
     */

    if (argc < 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" formatString ?arg arg ...?\"", (char *) NULL);
	return HAX_ERROR;
    }
    curArg = argv+2;
    argc -= 2;
    for (format = argv[1]; *format != 0; ) {
	char *newPtr = newFormat;

	width = precision = useTwoWords = noPercent = useShort = 0;

	/*
	 * Get rid of any characters before the next field specifier.
	 * Collapse backslash sequences found along the way.
	 */

	if (*format != '%') {
	    char *p;
	    int bsSize;

	    oneWordValue = p = format;
	    while ((*format != '%') && (*format != 0)) {
		if (*format == '\\') {
		    *p = Hax_Backslash(format, &bsSize);
		    if (*p != 0) {
			p++;
		    }
		    format += bsSize;
		} else {
		    *p = *format;
		    p++;
		    format++;
		}
	    }
	    size = p - oneWordValue;
	    noPercent = 1;
	    goto doField;
	}

	if (format[1] == '%') {
	    oneWordValue = format;
	    size = 1;
	    noPercent = 1;
	    format += 2;
	    goto doField;
	}

	/*
	 * Parse off a field specifier, compute how many characters
	 * will be needed to store the result, and substitute for
	 * "*" size specifiers.
	 */

	*newPtr = '%';
	newPtr++;
	format++;
	while ((*format == '-') || (*format == '#') || (*format == '0')
		|| (*format == ' ') || (*format == '+')) {
	    *newPtr = *format;
	    newPtr++;
	    format++;
	}
	if (isdigit(*format)) {
	    width = atoi(format);
	    do {
		format++;
	    } while (isdigit(*format));
	} else if (*format == '*') {
	    if (argc <= 0) {
		goto notEnoughArgs;
	    }
	    if (Hax_GetInt(interp, *curArg, &width) != HAX_OK) {
		goto fmtError;
	    }
	    argc--;
	    curArg++;
	    format++;
	}
	if (width != 0) {
	    sprintf(newPtr, "%d", width);
	    while (*newPtr != 0) {
		newPtr++;
	    }
	}
	if (*format == '.') {
	    *newPtr = '.';
	    newPtr++;
	    format++;
	}
	if (isdigit(*format)) {
	    precision = atoi(format);
	    do {
		format++;
	    } while (isdigit(*format));
	} else if (*format == '*') {
	    if (argc <= 0) {
		goto notEnoughArgs;
	    }
	    if (Hax_GetInt(interp, *curArg, &precision) != HAX_OK) {
		goto fmtError;
	    }
	    argc--;
	    curArg++;
	    format++;
	}
	if (precision != 0) {
	    sprintf(newPtr, "%d", precision);
	    while (*newPtr != 0) {
		newPtr++;
	    }
	}
	if (*format == 'l') {
	    format++;
	} else if (*format == 'h') {
	    useShort = 1;
	    *newPtr = 'h';
	    newPtr++;
	    format++;
	}
	*newPtr = *format;
	newPtr++;
	*newPtr = 0;
	if (argc <= 0) {
	    goto notEnoughArgs;
	}
	switch (*format) {
	    case 'D':
	    case 'O':
	    case 'U':
		if (!useShort) {
		    newPtr++;
		} else {
		    useShort = 0;
		}
		newPtr[-1] = tolower(*format);
		newPtr[-2] = 'l';
		*newPtr = 0;
	    case 'd':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
		if (Hax_GetLong(interp, *curArg, (long *) &oneWordValue)
			!= HAX_OK) {
		    goto fmtError;
		}
		size = 40;
		break;
	    case 's':
		oneWordValue = *curArg;
		size = strlen(*curArg);
		break;
	    case 'c':
		if (Hax_GetLong(interp, *curArg, (long *) &oneWordValue)
			!= HAX_OK) {
		    goto fmtError;
		}
		size = 1;
		break;
	    case 'F':
		newPtr[-1] = tolower(newPtr[-1]);
	    case 'e':
	    case 'E':
	    case 'f':
	    case 'g':
	    case 'G':
		if (Hax_GetDouble(interp, *curArg, &twoWordValue) != HAX_OK) {
		    goto fmtError;
		}
		useTwoWords = 1;
		size = 320;
		if (precision > 10) {
		    size += precision;
		}
		break;
	    case 0:
		interp->result =
			(char *) "format string ended in middle of field "
			    "specifier";
		goto fmtError;
	    default:
		sprintf(interp->result, "bad field specifier \"%c\"", *format);
		goto fmtError;
	}
	argc--;
	curArg++;
	format++;

	/*
	 * Make sure that there's enough space to hold the formatted
	 * result, then format it.
	 */

	doField:
	if (width > size) {
	    size = width;
	}
	if ((dstSize + size) > dstSpace) {
	    char *newDst;
	    int newSpace;

	    newSpace = 2*(dstSize + size);
	    newDst = (char *) ckalloc((unsigned) newSpace+1);
	    if (dstSize != 0) {
		memcpy(newDst, dst, dstSize);
	    }
	    if (dstSpace != HAX_RESULT_SIZE) {
		ckfree(dst);
	    }
	    dst = newDst;
	    dstSpace = newSpace;
	}
	if (noPercent) {
	    memcpy((dst+dstSize), oneWordValue, size);
	    dstSize += size;
	    dst[dstSize] = 0;
	} else {
	    if (useTwoWords) {
		sprintf(dst+dstSize, newFormat, twoWordValue);
	    } else if (useShort) {
		/*
		 * The double cast below is needed for a few machines
		 * (e.g. Pyramids as of 1/93) that don't like casts
		 * directly from pointers to shorts.
		 */

		sprintf(dst+dstSize, newFormat, (short) (long) oneWordValue);
	    } else {
		sprintf(dst+dstSize, newFormat, (char *) oneWordValue);
	    }
	    dstSize += strlen(dst+dstSize);
	}
    }

    interp->result = dst;
    if (dstSpace != HAX_RESULT_SIZE) {
	interp->freeProc = (Hax_FreeProc *) free;
    } else {
	interp->freeProc = 0;
    }
    return HAX_OK;

    notEnoughArgs:
    interp->result = (char *) "not enough arguments for all format specifiers";
    fmtError:
    if (dstSpace != HAX_RESULT_SIZE) {
	ckfree(dst);
    }
    return HAX_ERROR;
}
