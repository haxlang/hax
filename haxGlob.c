/*
 * haxGlob.c --
 *
 *	This file provides procedures and commands for file name
 *	manipulation, such as tilde expansion and globbing.
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
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclGlob.c,v 1.26 92/12/23 11:33:18 ouster Exp $ SPRITE (Berkeley)";
#endif /* not lint */

#include "hax.h"
#include "haxUnix.h"

/*
 * The structure below is used to keep track of a globbing result
 * being built up (i.e. a partial list of file names).  The list
 * grows dynamically to be as big as needed.
 */

typedef struct {
    char *result;		/* Pointer to result area. */
    int totalSpace;		/* Total number of characters allocated
				 * for result. */
    int spaceUsed;		/* Number of characters currently in use
				 * to hold the partial result (not including
				 * the terminating NULL). */
    int dynamic;		/* 0 means result is static space, 1 means
				 * it's dynamic. */
} GlobResult;

/*
 * Declarations for procedures local to this file:
 */

static void		AppendResult (Hax_Interp *interp,
			    char *dir, char *separator, char *name,
			    int nameLength);
static int		DoGlob (Hax_Interp *interp, char *dir,
			    char *rem);

/*
 *----------------------------------------------------------------------
 *
 * AppendResult --
 *
 *	Given two parts of a file name (directory and element within
 *	directory), concatenate the two together and append them to
 *	the result building up in interp.
 *
 * Results:
 *	There is no return value.
 *
 * Side effects:
 *	Interp->result gets extended.
 *
 *----------------------------------------------------------------------
 */

static void
AppendResult(
    Hax_Interp *interp,		/* Interpreter whose result should be
				 * appended to. */
    char *dir,			/* Name of directory, without trailing
				 * slash except for root directory. */
    char *separator,		/* Separator string so use between dir and
				 * name:  either "/" or "" depending on dir. */
    char *name,			/* Name of file withing directory (NOT
				 * necessarily null-terminated!). */
    int nameLength		/* Number of characters in name. */)
{
    Hax_Memoryp *memoryp;
    int dirFlags, nameFlags;
    char *p, saved;

    memoryp = Hax_GetMemoryp (interp);

    /*
     * Next, see if we can put together a valid list element from dir
     * and name by calling Hax_AppendResult.
     */

    if (*dir == 0) {
	dirFlags = 0;
    } else {
	Hax_ScanElement(dir, &dirFlags);
    }
    saved = name[nameLength];
    name[nameLength] = 0;
    Hax_ScanElement(name, &nameFlags);
    if ((dirFlags == 0) && (nameFlags == 0)) {
	if (*interp->result != 0) {
	    Hax_AppendResult(interp, " ", dir, separator, name, (char *) NULL);
	} else {
	    Hax_AppendResult(interp, dir, separator, name, (char *) NULL);
	}
	name[nameLength] = saved;
	return;
    }

    /*
     * This name has weird characters in it, so we have to convert it to
     * a list element.  To do that, we have to merge the characters
     * into a single name.  To do that, malloc a buffer to hold everything.
     */

    p = (char *) ckalloc(memoryp, (unsigned) (strlen(dir) + strlen(separator)
	    + nameLength + 1));
    sprintf(p, "%s%s%s", dir, separator, name);
    name[nameLength] = saved;
    Hax_AppendElement(interp, p, 0);
    ckfree(memoryp, p);
}

/*
 *----------------------------------------------------------------------
 *
 * DoGlob --
 *
 *	This recursive procedure forms the heart of the globbing
 *	code.  It performs a depth-first traversal of the tree
 *	given by the path name to be globbed.
 *
 * Results:
 *	The return value is a standard Hax result indicating whether
 *	an error occurred in globbing.  After a normal return the
 *	result in interp will be set to hold all of the file names
 *	given by the dir and rem arguments.  After an error the
 *	result in interp will hold an error message.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
DoGlob(
    Hax_Interp *interp,			/* Interpreter to use for error
					 * reporting (e.g. unmatched brace). */
    char *dir,				/* Name of a directory at which to
					 * start glob expansion.  This name
					 * is fixed: it doesn't contain any
					 * globbing chars. */
    char *rem				/* Path to glob-expand. */)
{
    /*
     * When this procedure is entered, the name to be globbed may
     * already have been partly expanded by ancestor invocations of
     * DoGlob.  The part that's already been expanded is in "dir"
     * (this may initially be empty), and the part still to expand
     * is in "rem".  This procedure expands "rem" one level, making
     * recursive calls to itself if there's still more stuff left
     * in the remainder.
     */

    Hax_Memoryp *memoryp;
    char *p;
    char c;
    char *openBrace, *closeBrace;
    int gotSpecial, result;
    const char *separator;

    memoryp = Hax_GetMemoryp (interp);

    /*
     * Figure out whether we'll need to add a slash between the directory
     * name and file names within the directory when concatenating them
     * together.
     */

    if ((dir[0] == 0) || ((dir[0] == '/') && (dir[1] == 0))) {
	separator = "";
    } else {
	separator = "/";
    }

    /*
     * When generating information for the next lower call,
     * use static areas if the name is short, and malloc if the name
     * is longer.
     */

#define STATIC_SIZE 200

    /*
     * First, find the end of the next element in rem, checking
     * along the way for special globbing characters.
     */

    gotSpecial = 0;
    openBrace = closeBrace = NULL;
    for (p = rem; ; p++) {
	c = *p;
	if ((c == '\0') || (c == '/')) {
	    break;
	}
	if ((c == '{') && (openBrace == NULL)) {
	    openBrace = p;
	}
	if ((c == '}') && (closeBrace == NULL)) {
	    closeBrace = p;
	}
	if ((c == '*') || (c == '[') || (c == '\\') || (c == '?')) {
	    gotSpecial = 1;
	}
    }

    /*
     * If there is an open brace in the argument, then make a recursive
     * call for each element between the braces.  In this case, the
     * recursive call to DoGlob uses the same "dir" that we got.
     * If there are several brace-pairs in a single name, we just handle
     * one here, and the others will be handled in recursive calls.
     */

    if (openBrace != NULL) {
	int remLength, l1, l2;
	char static1[STATIC_SIZE];
	char *element, *newRem;

	if (closeBrace == NULL) {
	    Hax_ResetResult(interp);
	    interp->result = (char *) "unmatched open-brace in file name";
	    return HAX_ERROR;
	}
	remLength = strlen(rem) + 1;
	if (remLength <= STATIC_SIZE) {
	    newRem = static1;
	} else {
	    newRem = (char *) ckalloc(memoryp, (unsigned) remLength);
	}
	l1 = openBrace-rem;
	strncpy(newRem, rem, l1);
	p = openBrace;
	for (p = openBrace; *p != '}'; ) {
	    element = p+1;
	    for (p = element; ((*p != '}') && (*p != ',')); p++) {
		/* Empty loop body:  just find end of this element. */
	    }
	    l2 = p - element;
	    strncpy(newRem+l1, element, l2);
	    strcpy(newRem+l1+l2, closeBrace+1);
	    if (DoGlob(interp, dir, newRem) != HAX_OK) {
		return HAX_ERROR;
	    }
	}
	if (remLength > STATIC_SIZE) {
	    ckfree(memoryp, newRem);
	}
	return HAX_OK;
    }

    /*
     * If there were any pattern-matching characters, then scan through
     * the directory to find all the matching names.
     */

    if (gotSpecial) {
	DIR *d;
	struct dirent *entryPtr;
	int l1, l2;
	char *pattern, *newDir, *dirName;
	char static1[STATIC_SIZE], static2[STATIC_SIZE];
	struct stat statBuf;

	/*
	 * Be careful not to do any actual file system operations on a
	 * directory named "";  instead, use ".".  This is needed because
	 * some versions of UNIX don't treat "" like "." automatically.
	 */

	if (*dir == '\0') {
	    dirName = (char *) ".";
	} else {
	    dirName = dir;
	}
	if ((stat(dirName, &statBuf) != 0) || !S_ISDIR(statBuf.st_mode)) {
	    return HAX_OK;
	}
	d = opendir(dirName);
	if (d == NULL) {
	    Hax_ResetResult(interp);
	    Hax_AppendResult(interp, "couldn't read directory \"",
		    dirName, "\": ", Hax_UnixError(interp), (char *) NULL);
	    return HAX_ERROR;
	}
	l1 = strlen(dir);
	l2 = (p - rem);
	if (l2 < STATIC_SIZE) {
	    pattern = static2;
	} else {
	    pattern = (char *) ckalloc(memoryp, (unsigned) (l2+1));
	}
	strncpy(pattern, rem, l2);
	pattern[l2] = '\0';
	result = HAX_OK;
	while (1) {
	    entryPtr = readdir(d);
	    if (entryPtr == NULL) {
		break;
	    }

	    /*
	     * Don't match names starting with "." unless the "." is
	     * present in the pattern.
	     */

	    if ((*entryPtr->d_name == '.') && (*pattern != '.')) {
		continue;
	    }
	    if (Hax_StringMatch(entryPtr->d_name, pattern)) {
		int nameLength = strlen(entryPtr->d_name);
		if (*p == 0) {
		    AppendResult(interp, dir, (char *) separator, entryPtr->d_name,
			    nameLength);
		} else {
		    if ((l1+nameLength+2) <= STATIC_SIZE) {
			newDir = static1;
		    } else {
			newDir = (char *) ckalloc(memoryp, (unsigned)
				(l1+nameLength+2));
		    }
		    sprintf(newDir, "%s%s%s", dir, separator, entryPtr->d_name);
		    result = DoGlob(interp, newDir, p+1);
		    if (newDir != static1) {
			ckfree(memoryp, newDir);
		    }
		    if (result != HAX_OK) {
			break;
		    }
		}
	    }
	}
	closedir(d);
	if (pattern != static2) {
	    ckfree(memoryp, pattern);
	}
	return result;
    }

    /*
     * This is the simplest case:  just another path element.  Move
     * it to the dir side and recurse (or just add the name to the
     * list, if we're at the end of the path).
     */

    if (*p == 0) {
	AppendResult(interp, dir, (char *) separator, rem, p-rem);
    } else {
	int l1, l2;
	char *newDir;
	char static1[STATIC_SIZE];

	l1 = strlen(dir);
	l2 = l1 + (p - rem) + 2;
	if (l2 <= STATIC_SIZE) {
	    newDir = static1;
	} else {
	    newDir = (char *) ckalloc(memoryp, (unsigned) l2);
	}
	sprintf(newDir, "%s%s%.*s", dir, separator, (int)(p-rem), rem);
	result = DoGlob(interp, newDir, p+1);
	if (newDir != static1) {
	    ckfree(memoryp, newDir);
	}
	if (result != HAX_OK) {
	    return HAX_ERROR;
	}
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_TildeSubst --
 *
 *	Given a name starting with a tilde, produce a name where
 *	the tilde and following characters have been replaced by
 *	the home directory location for the named user.
 *
 * Results:
 *	The result is a pointer to a static string containing
 *	the new name.  This name will only persist until the next
 *	call to Hax_TildeSubst;  save it if you care about it for
 *	the long term.  If there was an error in processing the
 *	tilde, then an error message is left in interp->result
 *	and the return value is NULL.
 *
 * Side effects:
 *	None that the caller needs to worry about.
 *
 *----------------------------------------------------------------------
 */

char *
Hax_TildeSubst(
    Hax_Interp *interp,		/* Interpreter in which to store error
				 * message (if necessary). */
    ClientData clientData,	/* Unix Client Data */
    char *name			/* File name, which may begin with "~/"
				 * (to indicate current user's home directory)
				 * or "~<user>/" (to indicate any user's
				 * home directory). */)
{
    Hax_Memoryp *memoryp;
    UnixClientData *clientDataPtr = (UnixClientData *) clientData;
    char *dir;
    int length;
    int fromPw = 0;
    char *p;

    if (name[0] != '~') {
	return name;
    }

    memoryp = Hax_GetMemoryp (interp);

    /*
     * First, find the directory name corresponding to the tilde entry.
     */

    if ((name[1] == '/') || (name[1] == '\0')) {
	dir = getenv("HOME");
	if (dir == NULL) {
	    Hax_ResetResult(interp);
	    Hax_AppendResult(interp, "couldn't find HOME environment ",
		    "variable to expand \"", name, "\"", (char *) NULL);
	    return NULL;
	}
	p = name+1;
    } else {
	struct passwd *pwPtr;

	for (p = &name[1]; (*p != 0) && (*p != '/'); p++) {
	    /* Null body;  just find end of name. */
	}
	length = p-&name[1];
	if (length >= clientDataPtr->curSize) {
	    length = clientDataPtr->curSize-1;
	}
	memcpy(clientDataPtr->curBuf, (name+1), length);
	clientDataPtr->curBuf[length] = '\0';
	pwPtr = getpwnam(clientDataPtr->curBuf);
	if (pwPtr == NULL) {
	    endpwent();
	    Hax_ResetResult(interp);
	    Hax_AppendResult(interp, "user \"", clientDataPtr->curBuf,
		    "\" doesn't exist", (char *) NULL);
	    return NULL;
	}
	dir = pwPtr->pw_dir;
	fromPw = 1;
    }

    /*
     * Grow the buffer if necessary to make enough space for the
     * full file name.
     */

    length = strlen(dir) + strlen(p);
    if (length >= clientDataPtr->curSize) {
	if (clientDataPtr->curBuf != clientDataPtr->staticBuf) {
	    ckfree(memoryp, clientDataPtr->curBuf);
	}
	clientDataPtr->curSize = length + 1;
	clientDataPtr->curBuf = ckalloc(memoryp, clientDataPtr->curSize);
    }

    /*
     * Finally, concatenate the directory name with the remainder
     * of the path in the buffer.
     */

    strcpy(clientDataPtr->curBuf, dir);
    strcat(clientDataPtr->curBuf, p);
    if (fromPw) {
	endpwent();
    }
    return clientDataPtr->curBuf;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_GlobCmd --
 *
 *	This procedure is invoked to process the "glob" Hax command.
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
Hax_GlobCmd(
    ClientData clientData,		/* Unix Client Data */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    UnixClientData *clientDataPtr = (UnixClientData *) clientData;
    int i, result, noComplain;

    if (argc < 2) {
	notEnoughArgs:
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" ?-nocomplain? name ?name ...?\"", (char *) NULL);
	return HAX_ERROR;
    }
    noComplain = 0;
    if ((argv[1][0] == '-') && (strcmp(argv[1], "-nocomplain") == 0)) {
	if (argc < 3) {
	    goto notEnoughArgs;
	}
	noComplain = 1;
    }

    for (i = 1 + noComplain; i < argc; i++) {
	char *thisName;

	/*
	 * Do special checks for names starting at the root and for
	 * names beginning with ~.  Then let DoGlob do the rest.
	 */

	thisName = argv[i];
	if (*thisName == '~') {
	    thisName = Hax_TildeSubst(interp, clientDataPtr, thisName);
	    if (thisName == NULL) {
		return HAX_ERROR;
	    }
	}
	if (*thisName == '/') {
	    result = DoGlob(interp, (char *) "/", thisName+1);
	} else {
	    result = DoGlob(interp, (char *) "", thisName);
	}
	if (result != HAX_OK) {
	    return result;
	}
    }
    if ((*interp->result == 0) && !noComplain) {
	const char *sep = "";

	Hax_AppendResult(interp, "no files matched glob pattern",
		(argc == 2) ? " \"" : "s \"", (char *) NULL);
	for (i = 1; i < argc; i++) {
	    Hax_AppendResult(interp, (char *) sep, argv[i], (char *) NULL);
	    sep = " ";
	}
	Hax_AppendResult(interp, "\"", (char *) NULL);
	return HAX_ERROR;
    }
    return HAX_OK;
}
