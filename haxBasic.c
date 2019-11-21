/*
 * haxBasic.c --
 *
 *	Contains the basic facilities for HAX command interpretation,
 *	including interpreter creation and deletion, command creation
 *	and deletion, and command parsing and execution.
 *
 * Copyright 1987-1992 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclBasic.c,v 1.133 92/08/21 15:45:32 ouster Exp $ SPRITE (Berkeley)";
#endif

#include "haxInt.h"

/*
 * The following structure defines all of the commands in the Hax core,
 * and the C procedures that execute them.
 */

typedef struct {
    const char *name;		/* Name of command. */
    Hax_CmdProc *proc;		/* Procedure that executes command. */
} CmdInfo;

/*
 * Built-in commands, and the procedures associated with them:
 */

static CmdInfo builtInCmds[] = {
    /*
     * Commands in the generic core:
     */

    {"append",		Hax_AppendCmd},
    {"array",		Hax_ArrayCmd},
    {"break",		Hax_BreakCmd},
    {"case",		Hax_CaseCmd},
    {"catch",		Hax_CatchCmd},
    {"concat",		Hax_ConcatCmd},
    {"continue",	Hax_ContinueCmd},
    {"error",		Hax_ErrorCmd},
    {"eval",		Hax_EvalCmd},
    {"expr",		Hax_ExprCmd},
    {"for",		Hax_ForCmd},
    {"foreach",		Hax_ForeachCmd},
    {"format",		Hax_FormatCmd},
    {"global",		Hax_GlobalCmd},
    {"if",		Hax_IfCmd},
    {"incr",		Hax_IncrCmd},
    {"info",		Hax_InfoCmd},
    {"join",		Hax_JoinCmd},
    {"lappend",		Hax_LappendCmd},
    {"lindex",		Hax_LindexCmd},
    {"linsert",		Hax_LinsertCmd},
    {"list",		Hax_ListCmd},
    {"llength",		Hax_LlengthCmd},
    {"lrange",		Hax_LrangeCmd},
    {"lreplace",	Hax_LreplaceCmd},
    {"lsearch",		Hax_LsearchCmd},
    {"lsort",		Hax_LsortCmd},
    {"proc",		Hax_ProcCmd},
    {"regexp",		Hax_RegexpCmd},
    {"regsub",		Hax_RegsubCmd},
    {"rename",		Hax_RenameCmd},
    {"return",		Hax_ReturnCmd},
    {"scan",		Hax_ScanCmd},
    {"set",		Hax_SetCmd},
    {"split",		Hax_SplitCmd},
    {"string",		Hax_StringCmd},
    {"trace",		Hax_TraceCmd},
    {"unset",		Hax_UnsetCmd},
    {"uplevel",		Hax_UplevelCmd},
    {"upvar",		Hax_UpvarCmd},
    {"while",		Hax_WhileCmd},
    {NULL,		(Hax_CmdProc *) NULL}
};

/*
 *----------------------------------------------------------------------
 *
 * Hax_CreateInterp --
 *
 *	Create a new HAX command interpreter.
 *
 * Results:
 *	The return value is a token for the interpreter, which may be
 *	used in calls to procedures like Hax_CreateCmd, Hax_Eval, or
 *	Hax_DeleteInterp.
 *
 * Side effects:
 *	The command interpreter is initialized with an empty variable
 *	table and the built-in commands.
 *
 *----------------------------------------------------------------------
 */

Hax_Interp *
Hax_CreateInterp(
    Hax_Memoryp *memoryp)
{
    Interp *iPtr;
    Command *cmdPtr;
    CmdInfo *cmdInfoPtr;
    int i;

    if (memoryp == NULL)
	return NULL;

    iPtr = (Interp *) ckalloc(memoryp, sizeof(Interp));
    iPtr->memoryp = memoryp;
    iPtr->result = iPtr->resultSpace;
    iPtr->freeProc = 0;
    iPtr->errorLine = 0;
    Hax_InitHashTable(&iPtr->commandTable, HAX_STRING_KEYS);
    Hax_InitHashTable(&iPtr->globalTable, HAX_STRING_KEYS);
    iPtr->numLevels = 0;
    iPtr->framePtr = NULL;
    iPtr->varFramePtr = NULL;
    iPtr->activeTracePtr = NULL;
    iPtr->numEvents = 0;
    iPtr->events = NULL;
    iPtr->curEvent = 0;
    iPtr->curEventNum = 0;
    iPtr->revPtr = NULL;
    iPtr->historyFirst = NULL;
    iPtr->revDisables = 1;
    iPtr->evalFirst = iPtr->evalLast = NULL;
    iPtr->appendResult = NULL;
    iPtr->appendAvl = 0;
    iPtr->appendUsed = 0;
    for (i = 0; i < NUM_REGEXPS; i++) {
	iPtr->patterns[i] = NULL;
	iPtr->patLengths[i] = -1;
	iPtr->regexps[i] = NULL;
    }
    iPtr->cmdCount = 0;
    iPtr->noEval = 0;
    iPtr->scriptFile = NULL;
    iPtr->flags = 0;
    iPtr->tracePtr = NULL;
    iPtr->resultSpace[0] = 0;
    iPtr->internalErrno = 0;
#ifdef HAX_LIBRARY
    iPtr->libraryPath = ckalloc(memoryp, strlen(HAX_LIBRARY) + 1);
    strcpy(iPtr->libraryPath, HAX_LIBRARY);
#else
    iPtr->libraryPath = NULL;
#endif

    /*
     * Create the built-in commands.  Do it here, rather than calling
     * Hax_CreateCommand, because it's faster (there's no need to
     * check for a pre-existing command by the same name).
     */

    for (cmdInfoPtr = builtInCmds; cmdInfoPtr->name != NULL; cmdInfoPtr++) {
	int newPtr;
	Hax_HashEntry *hPtr;

	hPtr = Hax_CreateHashEntry((Hax_Interp *) iPtr, &iPtr->commandTable,
		(char *) cmdInfoPtr->name, &newPtr);
	if (newPtr) {
	    cmdPtr = (Command *) ckalloc(memoryp, sizeof(Command));
	    cmdPtr->proc = cmdInfoPtr->proc;
	    cmdPtr->clientData = (ClientData) NULL;
	    cmdPtr->deleteProc = NULL;
	    Hax_SetHashValue(hPtr, cmdPtr);
	}
    }

    return (Hax_Interp *) iPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_DeleteInterp --
 *
 *	Delete an interpreter and free up all of the resources associated
 *	with it.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The interpreter is destroyed.  The caller should never again
 *	use the interp token.
 *
 *----------------------------------------------------------------------
 */

void
Hax_DeleteInterp(
    Hax_Interp *interp		/* Token for command interpreter (returned
				 * by a previous call to Hax_CreateInterp). */)
{
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;
    Hax_HashEntry *hPtr;
    Hax_HashSearch search;
    Command *cmdPtr;
    int i;

    /*
     * If the interpreter is in use, delay the deletion until later.
     */

    iPtr->flags |= DELETED;
    if (iPtr->numLevels != 0) {
	return;
    }

    /*
     * Free up any remaining resources associated with the
     * interpreter.
     */

    for (hPtr = Hax_FirstHashEntry(&iPtr->commandTable, &search);
	    hPtr != NULL; hPtr = Hax_NextHashEntry(&search)) {
	cmdPtr = (Command *) Hax_GetHashValue(hPtr);
	if (cmdPtr->deleteProc != NULL) {
	    (*cmdPtr->deleteProc)(interp, cmdPtr->clientData);
	}
	ckfree(memoryp, (char *) cmdPtr);
    }
    Hax_DeleteHashTable(interp, &iPtr->commandTable);
    HaxDeleteVars(iPtr, &iPtr->globalTable);
    if (iPtr->events != NULL) {
	int i;

	for (i = 0; i < iPtr->numEvents; i++) {
	    ckfree(memoryp, iPtr->events[i].command);
	}
	ckfree(memoryp, (char *) iPtr->events);
    }
    while (iPtr->revPtr != NULL) {
	HistoryRev *nextPtr = iPtr->revPtr->nextPtr;

	ckfree(memoryp, (char *) iPtr->revPtr);
	iPtr->revPtr = nextPtr;
    }
    if (iPtr->appendResult != NULL) {
	ckfree(memoryp, iPtr->appendResult);
    }
    for (i = 0; i < NUM_REGEXPS; i++) {
	if (iPtr->patterns[i] == NULL) {
	    break;
	}
	ckfree(memoryp, iPtr->patterns[i]);
	ckfree(memoryp, (char *) iPtr->regexps[i]);
    }
    while (iPtr->tracePtr != NULL) {
	Trace *nextPtr = iPtr->tracePtr->nextPtr;

	ckfree(memoryp, (char *) iPtr->tracePtr);
	iPtr->tracePtr = nextPtr;
    }

    if (iPtr->libraryPath != NULL) {
	ckfree(memoryp, iPtr->libraryPath);
    }

    ckfree(memoryp, (char *) iPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_CreateCommand --
 *
 *	Define a new command in a command table.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	If a command named cmdName already exists for interp, it is
 *	deleted.  In the future, when cmdName is seen as the name of
 *	a command by Hax_Eval, proc will be called.  When the command
 *	is deleted from the table, deleteProc will be called.  See the
 *	manual entry for details on the calling sequence.
 *
 *----------------------------------------------------------------------
 */

void
Hax_CreateCommand(
    Hax_Interp *interp,		/* Token for command interpreter (returned
				 * by a previous call to Hax_CreateInterp). */
    char *cmdName,		/* Name of command. */
    Hax_CmdProc *proc,		/* Command procedure to associate with
				 * cmdName. */
    ClientData clientData,	/* Arbitrary one-word value to pass to proc. */
    Hax_CmdDeleteProc *deleteProc
				/* If not NULL, gives a procedure to call when
				 * this command is deleted. */)
{
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;
    Command *cmdPtr;
    Hax_HashEntry *hPtr;
    int newPtr;

    hPtr = Hax_CreateHashEntry(interp, &iPtr->commandTable, cmdName, &newPtr);
    if (!newPtr) {
	/*
	 * Command already exists:  delete the old one.
	 */

	cmdPtr = (Command *) Hax_GetHashValue(hPtr);
	if (cmdPtr->deleteProc != NULL) {
	    (*cmdPtr->deleteProc)(interp, cmdPtr->clientData);
	}
    } else {
	cmdPtr = (Command *) ckalloc(memoryp, sizeof(Command));
	Hax_SetHashValue(hPtr, cmdPtr);
    }
    cmdPtr->proc = proc;
    cmdPtr->clientData = clientData;
    cmdPtr->deleteProc = deleteProc;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_DeleteCommand --
 *
 *	Remove the given command from the given interpreter.
 *
 * Results:
 *	0 is returned if the command was deleted successfully.
 *	-1 is returned if there didn't exist a command by that
 *	name.
 *
 * Side effects:
 *	CmdName will no longer be recognized as a valid command for
 *	interp.
 *
 *----------------------------------------------------------------------
 */

int
Hax_DeleteCommand(
    Hax_Interp *interp,		/* Token for command interpreter (returned
				 * by a previous call to Hax_CreateInterp). */
    char *cmdName		/* Name of command to remove. */)
{
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;
    Hax_HashEntry *hPtr;
    Command *cmdPtr;

    hPtr = Hax_FindHashEntry(&iPtr->commandTable, cmdName);
    if (hPtr == NULL) {
	return -1;
    }
    cmdPtr = (Command *) Hax_GetHashValue(hPtr);
    if (cmdPtr->deleteProc != NULL) {
	(*cmdPtr->deleteProc)(interp, cmdPtr->clientData);
    }
    ckfree(memoryp, (char *) cmdPtr);
    Hax_DeleteHashEntry(interp, hPtr);
    return 0;
}

/*
 *-----------------------------------------------------------------
 *
 * Hax_Eval --
 *
 *	Parse and execute a command in the Hax language.
 *
 * Results:
 *	The return value is one of the return codes defined in hax.h
 *	(such as HAX_OK), and interp->result contains a string value
 *	to supplement the return code.  The value of interp->result
 *	will persist only until the next call to Hax_Eval:  copy it or
 *	lose it! *TermPtr is filled in with the character just after
 *	the last one that was part of the command (usually a NULL
 *	character or a closing bracket).
 *
 * Side effects:
 *	Almost certainly;  depends on the command.
 *
 *-----------------------------------------------------------------
 */

int
Hax_Eval(
    Hax_Interp *interp,		/* Token for command interpreter (returned
				 * by a previous call to Hax_CreateInterp). */
    char *scriptFile,		/* Optional argument with a descriptive path
				 * of script loaded and passed to cmd. */
    char *cmd,			/* Pointer to HAX command to interpret. */
    int flags,			/* OR-ed combination of flags like
				 * HAX_BRACKET_TERM and HAX_RECORD_BOUNDS. */
    char **termPtr		/* If non-NULL, fill in the address it points
				 * to with the address of the char. just after
				 * the last one that was part of cmd.  See
				 * the man page for details on this. */)
{
    /*
     * The storage immediately below is used to generate a copy
     * of the command, after all argument substitutions.  Pv will
     * contain the argv values passed to the command procedure.
     */

#   define NUM_CHARS 200
    char copyStorage[NUM_CHARS];
    ParseValue pv;
    char *oldBuffer;

    /*
     * This procedure generates an (argv, argc) array for the command,
     * It starts out with stack-allocated space but uses dynamically-
     * allocated storage to increase it if needed.
     */

#   define NUM_ARGS 10
    char *(argStorage[NUM_ARGS]);
    char **argv = argStorage;
    int argc;
    int argSize = NUM_ARGS;

    char *src;			/* Points to current character
					 * in cmd. */
    char termChar;			/* Return when this character is found
					 * (either ']' or '\0').  Zero means
					 * that newlines terminate commands. */
    int result;				/* Return value. */
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;
    char *oldScriptFile = iPtr->scriptFile;
    Hax_HashEntry *hPtr;
    Command *cmdPtr;
    char *dummy;			/* Make termPtr point here if it was
					 * originally NULL. */
    char *cmdStart;			/* Points to first non-blank char. in
					 * command (used in calling trace
					 * procedures). */
    const char *ellipsis = "";		/* Used in setting errorInfo variable;
					 * set to "..." to indicate that not
					 * all of offending command is included
					 * in errorInfo.  "" means that the
					 * command is all there. */
    Trace *tracePtr;

    /*
     * Set the scriptFile path.
     */
    if (scriptFile)
        iPtr->scriptFile = scriptFile;
    /*
     * Initialize the result to an empty string and clear out any
     * error information.  This makes sure that we return an empty
     * result if there are no commands in the command string.
     */

    Hax_FreeResult(interp);
    iPtr->result = iPtr->resultSpace;
    iPtr->resultSpace[0] = 0;
    result = HAX_OK;

    /*
     * Check depth of nested calls to Hax_Eval:  if this gets too large,
     * it's probably because of an infinite loop somewhere.
     */

    iPtr->numLevels++;
    if (iPtr->numLevels > MAX_NESTING_DEPTH) {
	iPtr->numLevels--;
	iPtr->result =
	    (char *) "too many nested calls to Hax_Eval (infinite loop?)";
	result = HAX_ERROR;
	goto finish;
    }

    /*
     * Initialize the area in which command copies will be assembled.
     */

    pv.buffer = copyStorage;
    pv.end = copyStorage + NUM_CHARS - 1;
    pv.expandProc = HaxExpandParseValue;
    pv.clientData = (ClientData) NULL;

    src = cmd;
    if (flags & HAX_BRACKET_TERM) {
	termChar = ']';
    } else {
	termChar = 0;
    }
    if (termPtr == NULL) {
	termPtr = &dummy;
    }
    *termPtr = src;
    cmdStart = src;

    /*
     * There can be many sub-commands (separated by semi-colons or
     * newlines) in one command string.  This outer loop iterates over
     * individual commands.
     */

    while (*src != termChar) {
	iPtr->flags &= ~(ERR_IN_PROGRESS | ERROR_CODE_SET);

	/*
	 * Skim off leading white space and semi-colons, and skip
	 * comments.
	 */

	while (1) {
	    char c = *src;

	    if ((CHAR_TYPE(c) != HAX_SPACE) && (c != ';') && (c != '\n')) {
		break;
	    }
	    src += 1;
	}
	if (*src == '#') {
	    for (src++; *src != 0; src++) {
		if ((*src == '\n') && (src[-1] != '\\')) {
		    src++;
		    break;
		}
	    }
	    continue;
	}
	cmdStart = src;

	/*
	 * Parse the words of the command, generating the argc and
	 * argv for the command procedure.  May have to call
	 * HaxParseWords several times, expanding the argv array
	 * between calls.
	 */

	pv.next = oldBuffer = pv.buffer;
	argc = 0;
	while (1) {
	    int newArgs, maxArgs;
	    char **newArgv;
	    int i;

	    /*
	     * Note:  the "- 2" below guarantees that we won't use the
	     * last two argv slots here.  One is for a NULL pointer to
	     * mark the end of the list, and the other is to leave room
	     * for inserting the command name "unknown" as the first
	     * argument (see below).
	     */

	    maxArgs = argSize - argc - 2;
	    result = HaxParseWords((Hax_Interp *) iPtr, src, flags,
		    maxArgs, termPtr, &newArgs, &argv[argc], &pv);
	    src = *termPtr;
	    if (result != HAX_OK) {
		ellipsis = "...";
		goto done;
	    }

	    /*
	     * Careful!  Buffer space may have gotten reallocated while
	     * parsing words.  If this happened, be sure to update all
	     * of the older argv pointers to refer to the new space.
	     */

	    if (oldBuffer != pv.buffer) {
		int i;

		for (i = 0; i < argc; i++) {
		    argv[i] = pv.buffer + (argv[i] - oldBuffer);
		}
		oldBuffer = pv.buffer;
	    }
	    argc += newArgs;
	    if (newArgs < maxArgs) {
		argv[argc] = (char *) NULL;
		break;
	    }

	    /*
	     * Args didn't all fit in the current array.  Make it bigger.
	     */

	    argSize *= 2;
	    newArgv = (char **)
		    ckalloc(memoryp, (unsigned) argSize * sizeof(char *));
	    for (i = 0; i < argc; i++) {
		newArgv[i] = argv[i];
	    }
	    if (argv != argStorage) {
		ckfree(memoryp, (char *) argv);
	    }
	    argv = newArgv;
	}

	/*
	 * If this is an empty command (or if we're just parsing
	 * commands without evaluating them), then just skip to the
	 * next command.
	 */

	if ((argc == 0) || iPtr->noEval) {
	    continue;
	}
	argv[argc] = NULL;

	/*
	 * Save information for the history module, if needed.
	 */

	if (flags & HAX_RECORD_BOUNDS) {
	    iPtr->evalFirst = cmdStart;
	    iPtr->evalLast = src-1;
	}

	/*
	 * Find the procedure to execute this command.  If there isn't
	 * one, then see if there is a command "unknown".  If so,
	 * invoke it instead, passing it the words of the original
	 * command as arguments.
	 */

	hPtr = Hax_FindHashEntry(&iPtr->commandTable, argv[0]);
	if (hPtr == NULL) {
	    int i;

	    hPtr = Hax_FindHashEntry(&iPtr->commandTable, (char *) "unknown");
	    if (hPtr == NULL) {
		Hax_ResetResult(interp);
		Hax_AppendResult(interp, "invalid command name: \"",
			argv[0], "\"", (char *) NULL);
		result = HAX_ERROR;
		goto done;
	    }
	    for (i = argc; i >= 0; i--) {
		argv[i+1] = argv[i];
	    }
	    argv[0] = (char *) "unknown";
	    argc++;
	}
	cmdPtr = (Command *) Hax_GetHashValue(hPtr);

	/*
	 * Call trace procedures, if any.
	 */

	for (tracePtr = iPtr->tracePtr; tracePtr != NULL;
		tracePtr = tracePtr->nextPtr) {
	    char saved;

	    if (tracePtr->level < iPtr->numLevels) {
		continue;
	    }
	    saved = *src;
	    *src = 0;
	    (*tracePtr->proc)(tracePtr->clientData, interp, iPtr->numLevels,
		    cmdStart, cmdPtr->proc, cmdPtr->clientData, argc, argv);
	    *src = saved;
	}

	/*
	 * At long last, invoke the command procedure.  Reset the
	 * result to its default empty value first (it could have
	 * gotten changed by earlier commands in the same command
	 * string).
	 */

	iPtr->cmdCount++;
	Hax_FreeResult((Hax_Interp *) iPtr);
	iPtr->result = iPtr->resultSpace;
	iPtr->resultSpace[0] = 0;
	result = (*cmdPtr->proc)(cmdPtr->clientData, interp, argc, argv);
	if (result != HAX_OK) {
	    break;
	}
    }

    /*
     * Free up any extra resources that were allocated.
     */

    done:
    if (pv.buffer != copyStorage) {
	ckfree(memoryp, (char *) pv.buffer);
    }
    if (argv != argStorage) {
	ckfree(memoryp, (char *) argv);
    }
    iPtr->numLevels--;
    if (iPtr->numLevels == 0) {
	if (result == HAX_RETURN) {
	    result = HAX_OK;
	}
	if ((result != HAX_OK) && (result != HAX_ERROR)) {
	    Hax_ResetResult(interp);
	    if (result == HAX_BREAK) {
		iPtr->result = (char *) "invoked \"break\" outside of a loop";
	    } else if (result == HAX_CONTINUE) {
		iPtr->result =
		    (char *) "invoked \"continue\" outside of a loop";
	    } else {
		iPtr->result = iPtr->resultSpace;
		sprintf(iPtr->resultSpace,
		    (char *) "command returned bad code: %d", result);
	    }
	    result = HAX_ERROR;
	}
	if (iPtr->flags & DELETED) {
	    Hax_DeleteInterp(interp);
	}
    }

    /*
     * If an error occurred, record information about what was being
     * executed when the error occurred.
     */

    if ((result == HAX_ERROR) && !(iPtr->flags & ERR_ALREADY_LOGGED)) {
	int numChars;
	char *p;

	/*
	 * Compute the line number where the error occurred.
	 */

	iPtr->errorLine = 1;
	for (p = cmd; p != cmdStart; p++) {
	    if (*p == '\n') {
		iPtr->errorLine++;
	    }
	}
	for ( ; isspace(*p) || (*p == ';'); p++) {
	    if (*p == '\n') {
		iPtr->errorLine++;
	    }
	}

	/*
	 * Figure out how much of the command to print in the error
	 * message (up to a certain number of characters, or up to
	 * the first new-line).
	 */

	numChars = src - cmdStart;
	if (numChars > (NUM_CHARS-50)) {
	    numChars = NUM_CHARS-50;
	    ellipsis = " ...";
	}

	if (!(iPtr->flags & ERR_IN_PROGRESS)) {
	    sprintf(copyStorage, "\n    while executing\n\"%.*s%s\"",
		    numChars, cmdStart, ellipsis);
	} else {
	    sprintf(copyStorage, "\n    invoked from within\n\"%.*s%s\"",
		    numChars, cmdStart, ellipsis);
	}
	Hax_AddErrorInfo(interp, copyStorage);
	iPtr->flags &= ~ERR_ALREADY_LOGGED;
    } else {
	iPtr->flags &= ~ERR_ALREADY_LOGGED;
    }

finish:
    iPtr->scriptFile = oldScriptFile;
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_CreateTrace --
 *
 *	Arrange for a procedure to be called to trace command execution.
 *
 * Results:
 *	The return value is a token for the trace, which may be passed
 *	to Hax_DeleteTrace to eliminate the trace.
 *
 * Side effects:
 *	From now on, proc will be called just before a command procedure
 *	is called to execute a Hax command.  Calls to proc will have the
 *	following form:
 *
 *	void
 *	proc(clientData, interp, level, command, cmdProc, cmdClientData,
 *		argc, argv)
 *	    ClientData clientData;
 *	    Hax_Interp *interp;
 *	    int level;
 *	    char *command;
 *	    int (*cmdProc)();
 *	    ClientData cmdClientData;
 *	    int argc;
 *	    char **argv;
 *	{
 *	}
 *
 *	The clientData and interp arguments to proc will be the same
 *	as the corresponding arguments to this procedure.  Level gives
 *	the nesting level of command interpretation for this interpreter
 *	(0 corresponds to top level).  Command gives the ASCII text of
 *	the raw command, cmdProc and cmdClientData give the procedure that
 *	will be called to process the command and the ClientData value it
 *	will receive, and argc and argv give the arguments to the
 *	command, after any argument parsing and substitution.  Proc
 *	does not return a value.
 *
 *----------------------------------------------------------------------
 */

Hax_Trace
Hax_CreateTrace(
    Hax_Interp *interp,		/* Interpreter in which to create the trace. */
    int level,			/* Only call proc for commands at nesting level
				 * <= level (1 => top level). */
    Hax_CmdTraceProc *proc,	/* Procedure to call before executing each
				 * command. */
    ClientData clientData	/* Arbitrary one-word value to pass to proc. */)
{
    Trace *tracePtr;
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;

    tracePtr = (Trace *) ckalloc(memoryp, sizeof(Trace));
    tracePtr->level = level;
    tracePtr->proc = proc;
    tracePtr->clientData = clientData;
    tracePtr->nextPtr = iPtr->tracePtr;
    iPtr->tracePtr = tracePtr;

    return (Hax_Trace) tracePtr;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_DeleteTrace --
 *
 *	Remove a trace.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	From now on there will be no more calls to the procedure given
 *	in trace.
 *
 *----------------------------------------------------------------------
 */

void
Hax_DeleteTrace(
    Hax_Interp *interp,		/* Interpreter that contains trace. */
    Hax_Trace trace		/* Token for trace (returned previously by
				 * Hax_CreateTrace). */)
{
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;
    Trace *tracePtr = (Trace *) trace;
    Trace *tracePtr2;

    if (iPtr->tracePtr == tracePtr) {
	iPtr->tracePtr = tracePtr->nextPtr;
	ckfree(memoryp, (char *) tracePtr);
    } else {
	for (tracePtr2 = iPtr->tracePtr; tracePtr2 != NULL;
		tracePtr2 = tracePtr2->nextPtr) {
	    if (tracePtr2->nextPtr == tracePtr) {
		tracePtr2->nextPtr = tracePtr->nextPtr;
		ckfree(memoryp, (char *) tracePtr);
		return;
	    }
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_AddErrorInfo --
 *
 *	Add information to a message being accumulated that describes
 *	the current error.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The contents of message are added to the "errorInfo" variable.
 *	If Hax_Eval has been called since the current value of errorInfo
 *	was set, errorInfo is cleared before adding the new message.
 *
 *----------------------------------------------------------------------
 */

void
Hax_AddErrorInfo(
    Hax_Interp *interp,		/* Interpreter to which error information
				 * pertains. */
    char *message		/* Message to record. */)
{
    Interp *iPtr = (Interp *) interp;

    /*
     * If an error is already being logged, then the new errorInfo
     * is the concatenation of the old info and the new message.
     * If this is the first piece of info for the error, then the
     * new errorInfo is the concatenation of the message in
     * interp->result and the new message.
     */

    if (!(iPtr->flags & ERR_IN_PROGRESS)) {
	Hax_SetVar2(interp, (char *) "errorInfo", (char *) NULL,
		interp->result, HAX_GLOBAL_ONLY);
	iPtr->flags |= ERR_IN_PROGRESS;

	/*
	 * If the errorCode variable wasn't set by the code that generated
	 * the error, set it to "NONE".
	 */

	if (!(iPtr->flags & ERROR_CODE_SET)) {
	    (void) Hax_SetVar2(interp, (char *) "errorCode", (char *) NULL,
		    (char *) "NONE", HAX_GLOBAL_ONLY);
	}
    }
    Hax_SetVar2(interp, (char *) "errorInfo", (char *) NULL, message,
	    HAX_GLOBAL_ONLY|HAX_APPEND_VALUE);
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_VarEval --
 *
 *	Given a variable number of string arguments, concatenate them
 *	all together and execute the result as a Hax command.
 *
 * Results:
 *	A standard Hax return result.  An error message or other
 *	result may be left in interp->result.
 *
 * Side effects:
 *	Depends on what was done by the command.
 *
 *----------------------------------------------------------------------
 */
	/* VARARGS2 */ /* ARGSUSED */
int
Hax_VarEval(
    Hax_Interp *interp,		/* Interpreter in which to execute command. */
    ...				/* One or more strings to concatenate,
				 * terminated with a NULL string. */)
{
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;
    va_list argList;
#define FIXED_SIZE 200
    char fixedSpace[FIXED_SIZE+1];
    int spaceAvl, spaceUsed, length;
    char *string, *cmd;
    int result;

    /*
     * Copy the strings one after the other into a single larger
     * string.  Use stack-allocated space for small commands, but if
     * the commands gets too large than call ckalloc to create the
     * space.
     */

    va_start(argList, interp);
    interp = va_arg(argList, Hax_Interp *);
    spaceAvl = FIXED_SIZE;
    spaceUsed = 0;
    cmd = fixedSpace;
    while (1) {
	string = va_arg(argList, char *);
	if (string == NULL) {
	    break;
	}
	length = strlen(string);
	if ((spaceUsed + length) > spaceAvl) {
	    char *newPtr;

	    spaceAvl = spaceUsed + length;
	    spaceAvl += spaceAvl/2;
	    newPtr = (char *) ckalloc(memoryp, (unsigned) spaceAvl);
	    memcpy(newPtr, cmd, spaceUsed);
	    if (cmd != fixedSpace) {
		ckfree(memoryp, cmd);
	    }
	    cmd = newPtr;
	}
	strcpy(cmd + spaceUsed, string);
	spaceUsed += length;
    }
    va_end(argList);
    cmd[spaceUsed] = '\0';

    result = Hax_Eval(interp, NULL, cmd, 0, (char **) NULL);
    if (cmd != fixedSpace) {
	ckfree(memoryp, cmd);
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_GlobalEval --
 *
 *	Evaluate a command at global level in an interpreter.
 *
 * Results:
 *	A standard Hax result is returned, and interp->result is
 *	modified accordingly.
 *
 * Side effects:
 *	The command string is executed in interp, and the execution
 *	is carried out in the variable context of global level (no
 *	procedures active), just as if an "uplevel #0" command were
 *	being executed.
 *
 *----------------------------------------------------------------------
 */

int
Hax_GlobalEval(
    Hax_Interp *interp,		/* Interpreter in which to evaluate command. */
    char *command		/* Command to evaluate. */)
{
    Interp *iPtr = (Interp *) interp;
    int result;
    CallFrame *savedVarFramePtr;

    savedVarFramePtr = iPtr->varFramePtr;
    iPtr->varFramePtr = NULL;
    result = Hax_Eval(interp, NULL, command, 0, (char **) NULL);
    iPtr->varFramePtr = savedVarFramePtr;
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_GetMemoryp --
 *     Retrieve the Memoryp context of an interpreter.
 *
 *----------------------------------------------------------------------
 */
Hax_Memoryp *
Hax_GetMemoryp(
    Hax_Interp *interp)
{
    Interp *iPtr = (Interp *) interp;

    return (Hax_Memoryp *) iPtr->memoryp;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_SetLibraryPath --
 *     Set location of the Hax library.
 *
 *----------------------------------------------------------------------
 */
void
Hax_SetLibraryPath(
    Hax_Interp *interp,
    char *path)
{
    Interp *iPtr = (Interp *) interp;
    Hax_Memoryp *memoryp = iPtr->memoryp;

    if (iPtr->libraryPath != NULL) {
	ckfree(memoryp, iPtr->libraryPath);
    }

    iPtr->libraryPath = ckalloc(memoryp, strlen(path) + 1);
    strcpy(iPtr->libraryPath, path);
}
