/*
 * haxUnixAZ.c --
 *
 *	This file contains the top-level command procedures for
 *	commands in the Hax core that require UNIX facilities
 *	such as files and process execution.  Much of the code
 *	in this file is based on earlier versions contributed
 *	by Karl Lehenbauer, Mark Diekhans and Peter da Silva.
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
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclUnixAZ.c,v 1.40 93/01/28 16:06:35 ouster Exp $ SPRITE (Berkeley)";
#endif /* not lint */

#include "hax.h"
#include "haxUnix.h"

/*
 * The following structure defines all of the commands in the Hax core,
 * and the C procedures that execute them.
 */

typedef struct {
    const char *name;		/* Name of command. */
    Hax_CmdProc *proc;		/* Procedure that executes command. */
} CmdInfo;

/*
 * Built-in UNIX commands, and the procedures associated with them:
 */

static CmdInfo builtInCmds[] = {
    {"cd",		Hax_CdCmd},
    {"close",		Hax_CloseCmd},
    {"eof",		Hax_EofCmd},
    {"exec",		Hax_ExecCmd},
    {"exit",		Hax_ExitCmd},
    {"file",		Hax_FileCmd},
    {"flush",		Hax_FlushCmd},
    {"gets",		Hax_GetsCmd},
    {"glob",		Hax_GlobCmd},
    {"open",		Hax_OpenCmd},
    {"puts",		Hax_PutsCmd},
    {"pwd",		Hax_PwdCmd},
    {"read",		Hax_ReadCmd},
    {"seek",		Hax_SeekCmd},
    {"source",		Hax_SourceCmd},
    {"tell",		Hax_TellCmd},
    {"time",		Hax_TimeCmd},
    {NULL,		(Hax_CmdProc *) NULL}
};

/*
 * Prototypes for local procedures defined in this file:
 */

static int		CleanupChildren (Hax_Interp *interp,
			    int numPids, int *pidPtr, int errorId);
static char *		GetFileType (int mode);
static int		StoreStatData (Hax_Interp *interp,
			    char *varName, struct stat *statPtr);

/*
 *----------------------------------------------------------------------
 *
 * Hax_CdCmd --
 *
 *	This procedure is invoked to process the "cd" Hax command.
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
Hax_CdCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    UnixClientData *clientDataPtr;
    Hax_Memoryp *memoryp;
    char *dirName;

    memoryp = Hax_GetMemoryp(interp);
    clientDataPtr = (UnixClientData *) clientData;

    if (argc > 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" dirName\"", (char *) NULL);
	return HAX_ERROR;
    }

    if (argc == 2) {
	dirName = argv[1];
    } else {
	dirName = (char *) "~";
    }
    dirName = Hax_TildeSubst(interp, clientData, dirName);
    if (dirName == NULL) {
	return HAX_ERROR;
    }
    if (clientDataPtr->currentDir != NULL) {
	ckfree(memoryp, clientDataPtr->currentDir);
	clientDataPtr->currentDir = NULL;
    }
    if (chdir(dirName) != 0) {
	Hax_AppendResult(interp, "couldn't change working directory to \"",
		dirName, "\": ", Hax_UnixError(interp), (char *) NULL);
	return HAX_ERROR;
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_CloseCmd --
 *
 *	This procedure is invoked to process the "close" Hax command.
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
Hax_CloseCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    Hax_Memoryp *memoryp;
    UnixClientData *clientDataPtr;
    OpenFile *filePtr;
    int result = HAX_OK;

    memoryp = Hax_GetMemoryp(interp);

    if (argc != 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileId\"", (char *) NULL);
	return HAX_ERROR;
    }

    clientDataPtr = (UnixClientData *) clientData;
    if (HaxGetOpenFile(interp, clientDataPtr, argv[1], &filePtr) != HAX_OK) {
	return HAX_ERROR;
    }

    clientDataPtr->filePtrArray[fileno(filePtr->f)] = NULL;

    /*
     * First close the file (in the case of a process pipeline, there may
     * be two files, one for the pipe at each end of the pipeline).
     */

    if (filePtr->f2 != NULL) {
	if (fclose(filePtr->f2) == EOF) {
	    Hax_AppendResult(interp, "error closing \"", argv[1],
		    "\": ", Hax_UnixError(interp), "\n", (char *) NULL);
	    result = HAX_ERROR;
	}
    }
    if (fclose(filePtr->f) == EOF) {
	Hax_AppendResult(interp, "error closing \"", argv[1],
		"\": ", Hax_UnixError(interp), "\n", (char *) NULL);
	result = HAX_ERROR;
    }

    /*
     * If the file was a connection to a pipeline, clean up everything
     * associated with the child processes.
     */

    if (filePtr->numPids > 0) {
	if (CleanupChildren(interp, filePtr->numPids, filePtr->pidPtr,
		filePtr->errorId) != HAX_OK) {
	    result = HAX_ERROR;
	}
    }

    ckfree(memoryp, (char *) filePtr);
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_EofCmd --
 *
 *	This procedure is invoked to process the "eof" Hax command.
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
Hax_EofCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    UnixClientData *clientDataPtr;
    OpenFile *filePtr;

    if (argc != 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileId\"", (char *) NULL);
	return HAX_ERROR;
    }

    clientDataPtr = (UnixClientData *) clientData;
    if (HaxGetOpenFile(interp, clientDataPtr, argv[1], &filePtr) != HAX_OK) {
	return HAX_ERROR;
    }
    if (feof(filePtr->f)) {
	interp->result = (char *) "1";
    } else {
	interp->result = (char *) "0";
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ExecCmd --
 *
 *	This procedure is invoked to process the "exec" Hax command.
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
Hax_ExecCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    Hax_Memoryp *memoryp;
    int outputId;			/* File id for output pipe.  -1
					 * means command overrode. */
    int errorId;			/* File id for temporary file
					 * containing error output. */
    int *pidPtr;
    int numPids, result;

    memoryp = Hax_GetMemoryp(interp);

    /*
     * See if the command is to be run in background;  if so, create
     * the command, detach it, and return.
     */

    if ((argv[argc-1][0] == '&') && (argv[argc-1][1] == 0)) {
	argc--;
	argv[argc] = NULL;
	numPids = Hax_CreatePipeline(interp, clientData, argc-1, argv+1,
		&pidPtr, (int *) NULL, (int *) NULL, (int *) NULL);
	if (numPids < 0) {
	    return HAX_ERROR;
	}
	Hax_DetachPids(numPids, pidPtr);
	ckfree(memoryp, (char *) pidPtr);
	return HAX_OK;
    }

    /*
     * Create the command's pipeline.
     */

    numPids = Hax_CreatePipeline(interp, clientData, argc-1, argv+1, &pidPtr,
	    (int *) NULL, &outputId, &errorId);
    if (numPids < 0) {
	return HAX_ERROR;
    }

    /*
     * Read the child's output (if any) and put it into the result.
     */

    result = HAX_OK;
    if (outputId != -1) {
	while (1) {
#	    define BUFFER_SIZE 1000
	    char buffer[BUFFER_SIZE+1];
	    int count;

	    count = read(outputId, buffer, BUFFER_SIZE);

	    if (count == 0) {
		break;
	    }
	    if (count < 0) {
		Hax_ResetResult(interp);
		Hax_AppendResult(interp,
			"error reading from output pipe: ",
			Hax_UnixError(interp), (char *) NULL);
		result = HAX_ERROR;
		break;
	    }
	    buffer[count] = 0;
	    Hax_AppendResult(interp, buffer, (char *) NULL);
	}
	close(outputId);
    }

    if (CleanupChildren(interp, numPids, pidPtr, errorId) != HAX_OK) {
	result = HAX_ERROR;
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ExitCmd --
 *
 *	This procedure is invoked to process the "exit" Hax command.
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
Hax_ExitCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    int value;

    if ((argc != 1) && (argc != 2)) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" ?returnCode?\"", (char *) NULL);
	return HAX_ERROR;
    }
    if (argc == 1) {
	exit(0);
    }
    if (Hax_GetInt(interp, argv[1], &value) != HAX_OK) {
	return HAX_ERROR;
    }
    exit(value);
    return HAX_OK;			/* Better not ever reach this! */
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_FileCmd --
 *
 *	This procedure is invoked to process the "file" Hax command.
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
Hax_FileCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    char *p;
    int length, statOp;
    int mode = 0;			/* Initialized only to prevent
					 * compiler warning message. */
    struct stat statBuf;
    char *fileName, c;

    if (argc < 3) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" option name ?arg ...?\"", (char *) NULL);
	return HAX_ERROR;
    }
    c = argv[1][0];
    length = strlen(argv[1]);

    /*
     * First handle operations on the file name.
     */

    fileName = Hax_TildeSubst(interp, clientData, argv[2]);
    if (fileName == NULL) {
	return HAX_ERROR;
    }
    if ((c == 'd') && (strncmp(argv[1], "dirname", length) == 0)) {
	if (argc != 3) {
	    argv[1] = (char *) "dirname";
	    not3Args:
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " ", argv[1], " name\"", (char *) NULL);
	    return HAX_ERROR;
	}
	p = strrchr(fileName, '/');
	if (p == NULL) {
	    interp->result = (char *) ".";
	} else if (p == fileName) {
	    interp->result = (char *) "/";
	} else {
	    *p = 0;
	    Hax_SetResult(interp, fileName, HAX_VOLATILE);
	    *p = '/';
	}
	return HAX_OK;
    } else if ((c == 'r') && (strncmp(argv[1], "rootname", length) == 0)
	    && (length >= 2)) {
	char *lastSlash;

	if (argc != 3) {
	    argv[1] = (char *) "rootname";
	    goto not3Args;
	}
	p = strrchr(fileName, '.');
	lastSlash = strrchr(fileName, '/');
	if ((p == NULL) || ((lastSlash != NULL) && (lastSlash > p))) {
	    Hax_SetResult(interp, fileName, HAX_VOLATILE);
	} else {
	    *p = 0;
	    Hax_SetResult(interp, fileName, HAX_VOLATILE);
	    *p = '.';
	}
	return HAX_OK;
    } else if ((c == 'e') && (strncmp(argv[1], "extension", length) == 0)
	    && (length >= 3)) {
	char *lastSlash;

	if (argc != 3) {
	    argv[1] = (char *) "extension";
	    goto not3Args;
	}
	p = strrchr(fileName, '.');
	lastSlash = strrchr(fileName, '/');
	if ((p != NULL) && ((lastSlash == NULL) || (lastSlash < p))) {
	    Hax_SetResult(interp, p, HAX_VOLATILE);
	}
	return HAX_OK;
    } else if ((c == 't') && (strncmp(argv[1], "tail", length) == 0)
	    && (length >= 2)) {
	if (argc != 3) {
	    argv[1] = (char *) "tail";
	    goto not3Args;
	}
	p = strrchr(fileName, '/');
	if (p != NULL) {
	    Hax_SetResult(interp, p+1, HAX_VOLATILE);
	} else {
	    Hax_SetResult(interp, fileName, HAX_VOLATILE);
	}
	return HAX_OK;
    }

    /*
     * Next, handle operations that can be satisfied with the "access"
     * kernel call.
     */

    if (fileName == NULL) {
	return HAX_ERROR;
    }
    if ((c == 'r') && (strncmp(argv[1], "readable", length) == 0)
	    && (length >= 5)) {
	if (argc != 3) {
	    argv[1] = (char *) "readable";
	    goto not3Args;
	}
	mode = R_OK;
	checkAccess:
	if (access(fileName, mode) == -1) {
	    interp->result = (char *) "0";
	} else {
	    interp->result = (char *) "1";
	}
	return HAX_OK;
    } else if ((c == 'w') && (strncmp(argv[1], "writable", length) == 0)) {
	if (argc != 3) {
	    argv[1] = (char *) "writable";
	    goto not3Args;
	}
	mode = W_OK;
	goto checkAccess;
    } else if ((c == 'e') && (strncmp(argv[1], "executable", length) == 0)
	    && (length >= 3)) {
	if (argc != 3) {
	    argv[1] = (char *) "executable";
	    goto not3Args;
	}
	mode = X_OK;
	goto checkAccess;
    } else if ((c == 'e') && (strncmp(argv[1], "exists", length) == 0)
	    && (length >= 3)) {
	if (argc != 3) {
	    argv[1] = (char *) "exists";
	    goto not3Args;
	}
	mode = F_OK;
	goto checkAccess;
    }

    /*
     * Lastly, check stuff that requires the file to be stat-ed.
     */

    if ((c == 'a') && (strncmp(argv[1], "atime", length) == 0)) {
	if (argc != 3) {
	    argv[1] = (char *) "atime";
	    goto not3Args;
	}
	if (stat(fileName, &statBuf) == -1) {
	    goto badStat;
	}
	sprintf(interp->result, "%ld", statBuf.st_atime);
	return HAX_OK;
    } else if ((c == 'i') && (strncmp(argv[1], "isdirectory", length) == 0)
	    && (length >= 3)) {
	if (argc != 3) {
	    argv[1] = (char *) "isdirectory";
	    goto not3Args;
	}
	statOp = 2;
    } else if ((c == 'i') && (strncmp(argv[1], "isfile", length) == 0)
	    && (length >= 3)) {
	if (argc != 3) {
	    argv[1] = (char *) "isfile";
	    goto not3Args;
	}
	statOp = 1;
    } else if ((c == 'l') && (strncmp(argv[1], "lstat", length) == 0)) {
	if (argc != 4) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " lstat name varName\"", (char *) NULL);
	    return HAX_ERROR;
	}

	if (lstat(fileName, &statBuf) == -1) {
	    Hax_AppendResult(interp, "couldn't lstat \"", argv[2],
		    "\": ", Hax_UnixError(interp), (char *) NULL);
	    return HAX_ERROR;
	}
	return StoreStatData(interp, argv[3], &statBuf);
    } else if ((c == 'm') && (strncmp(argv[1], "mtime", length) == 0)) {
	if (argc != 3) {
	    argv[1] = (char *) "mtime";
	    goto not3Args;
	}
	if (stat(fileName, &statBuf) == -1) {
	    goto badStat;
	}
	sprintf(interp->result, "%ld", statBuf.st_mtime);
	return HAX_OK;
    } else if ((c == 'o') && (strncmp(argv[1], "owned", length) == 0)) {
	if (argc != 3) {
	    argv[1] = (char *) "owned";
	    goto not3Args;
	}
	statOp = 0;
    /*
     * This option is only included if symbolic links exist on this system
     * (in which case S_IFLNK should be defined).
     */
    } else if ((c == 'r') && (strncmp(argv[1], "readlink", length) == 0)
	    && (length >= 5)) {
	char linkValue[MAXPATHLEN+1];
	int linkLength;

	if (argc != 3) {
	    argv[1] = (char *) "readlink";
	    goto not3Args;
	}
	linkLength = readlink(fileName, linkValue, sizeof(linkValue) - 1);
	if (linkLength == -1) {
	    Hax_AppendResult(interp, "couldn't readlink \"", argv[2],
		    "\": ", Hax_UnixError(interp), (char *) NULL);
	    return HAX_ERROR;
	}
	linkValue[linkLength] = 0;
	Hax_SetResult(interp, linkValue, HAX_VOLATILE);
	return HAX_OK;
    } else if ((c == 's') && (strncmp(argv[1], "size", length) == 0)
	    && (length >= 2)) {
	if (argc != 3) {
	    argv[1] = (char *) "size";
	    goto not3Args;
	}
	if (stat(fileName, &statBuf) == -1) {
	    goto badStat;
	}
	sprintf(interp->result, "%ld", statBuf.st_size);
	return HAX_OK;
    } else if ((c == 's') && (strncmp(argv[1], "stat", length) == 0)
	    && (length >= 2)) {
	if (argc != 4) {
	    Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " stat name varName\"", (char *) NULL);
	    return HAX_ERROR;
	}

	if (stat(fileName, &statBuf) == -1) {
	    badStat:
	    Hax_AppendResult(interp, "couldn't stat \"", argv[2],
		    "\": ", Hax_UnixError(interp), (char *) NULL);
	    return HAX_ERROR;
	}
	return StoreStatData(interp, argv[3], &statBuf);
    } else if ((c == 't') && (strncmp(argv[1], "type", length) == 0)
	    && (length >= 2)) {
	if (argc != 3) {
	    argv[1] = (char *) "type";
	    goto not3Args;
	}
	if (lstat(fileName, &statBuf) == -1) {
	    goto badStat;
	}
	interp->result = GetFileType((int) statBuf.st_mode);
	return HAX_OK;
    } else {
	Hax_AppendResult(interp, "bad option \"", argv[1],
		"\": should be atime, dirname, executable, exists, ",
		"extension, isdirectory, isfile, lstat, mtime, owned, ",
		"readable, ",
		"readlink, ",
		"root, size, stat, tail, type, ",
		"or writable",
		(char *) NULL);
	return HAX_ERROR;
    }
    if (stat(fileName, &statBuf) == -1) {
	interp->result = (char *) "0";
	return HAX_OK;
    }
    switch (statOp) {
	case 0:
	    mode = (geteuid() == statBuf.st_uid);
	    break;
	case 1:
	    mode = S_ISREG(statBuf.st_mode);
	    break;
	case 2:
	    mode = S_ISDIR(statBuf.st_mode);
	    break;
    }
    if (mode) {
	interp->result = (char *) "1";
    } else {
	interp->result = (char *) "0";
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * StoreStatData --
 *
 *	This is a utility procedure that breaks out the fields of a
 *	"stat" structure and stores them in textual form into the
 *	elements of an associative array.
 *
 * Results:
 *	Returns a standard Hax return value.  If an error occurs then
 *	a message is left in interp->result.
 *
 * Side effects:
 *	Elements of the associative array given by "varName" are modified.
 *
 *----------------------------------------------------------------------
 */

static int
StoreStatData(
    Hax_Interp *interp,			/* Interpreter for error reports. */
    char *varName,			/* Name of associative array variable
					 * in which to store stat results. */
    struct stat *statPtr		/* Pointer to buffer containing
					 * stat data to store in varName. */)
{
    char string[30];

    sprintf(string, "%lld", (long long int) statPtr->st_dev);
    if (Hax_SetVar2(interp, varName, (char *) "dev", string, HAX_LEAVE_ERR_MSG)
	    == NULL) {
	return HAX_ERROR;
    }
    sprintf(string, "%lld", (long long int) statPtr->st_ino);
    if (Hax_SetVar2(interp, varName, (char *) "ino", string, HAX_LEAVE_ERR_MSG)
	    == NULL) {
	return HAX_ERROR;
    }
    sprintf(string, "%d", statPtr->st_mode);
    if (Hax_SetVar2(interp, varName, (char *) "mode", string, HAX_LEAVE_ERR_MSG)
	    == NULL) {
	return HAX_ERROR;
    }
    sprintf(string, "%lld", (long long int) statPtr->st_nlink);
    if (Hax_SetVar2(interp, varName, (char *) "nlink", string,
	    HAX_LEAVE_ERR_MSG)
	    == NULL) {
	return HAX_ERROR;
    }
    sprintf(string, "%d", statPtr->st_uid);
    if (Hax_SetVar2(interp, varName, (char *) "uid", string, HAX_LEAVE_ERR_MSG)
	    == NULL) {
	return HAX_ERROR;
    }
    sprintf(string, "%d", statPtr->st_gid);
    if (Hax_SetVar2(interp, varName, (char *) "gid", string, HAX_LEAVE_ERR_MSG)
	    == NULL) {
	return HAX_ERROR;
    }
    sprintf(string, "%lld", (long long int) statPtr->st_size);
    if (Hax_SetVar2(interp, varName, (char *) "size", string, HAX_LEAVE_ERR_MSG)
	    == NULL) {
	return HAX_ERROR;
    }
    sprintf(string, "%lld", (long long int) statPtr->st_atime);
    if (Hax_SetVar2(interp, varName, (char *) "atime", string,
	    HAX_LEAVE_ERR_MSG)
	    == NULL) {
	return HAX_ERROR;
    }
    sprintf(string, "%lld", (long long int) statPtr->st_mtime);
    if (Hax_SetVar2(interp, varName, (char *) "mtime", string,
	    HAX_LEAVE_ERR_MSG)
	    == NULL) {
	return HAX_ERROR;
    }
    sprintf(string, "%lld", (long long int) statPtr->st_ctime);
    if (Hax_SetVar2(interp, varName, (char *) "ctime", string,
	    HAX_LEAVE_ERR_MSG)
	    == NULL) {
	return HAX_ERROR;
    }
    if (Hax_SetVar2(interp, varName, (char *) "type",
	    GetFileType((int) statPtr->st_mode), HAX_LEAVE_ERR_MSG) == NULL) {
	return HAX_ERROR;
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * GetFileType --
 *
 *	Given a mode word, returns a string identifying the type of a
 *	file.
 *
 * Results:
 *	A static text string giving the file type from mode.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static char *
GetFileType(
    int mode)
{
    if (S_ISREG(mode)) {
	return (char *) "file";
    } else if (S_ISDIR(mode)) {
	return (char *) "directory";
    } else if (S_ISCHR(mode)) {
	return (char *) "characterSpecial";
    } else if (S_ISBLK(mode)) {
	return (char *) "blockSpecial";
    } else if (S_ISFIFO(mode)) {
	return (char *) "fifo";
    } else if (S_ISLNK(mode)) {
	return (char *) "link";
    } else if (S_ISSOCK(mode)) {
	return (char *) "socket";
    }
    return (char *) "unknown";
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_FlushCmd --
 *
 *	This procedure is invoked to process the "flush" Hax command.
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
Hax_FlushCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    UnixClientData *clientDataPtr;
    OpenFile *filePtr;
    FILE *f;

    if (argc != 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileId\"", (char *) NULL);
	return HAX_ERROR;
    }

    clientDataPtr = (UnixClientData *) clientData;
    if (HaxGetOpenFile(interp, clientDataPtr, argv[1], &filePtr) != HAX_OK) {
	return HAX_ERROR;
    }
    if (!filePtr->writable) {
	Hax_AppendResult(interp, "\"", argv[1],
		"\" wasn't opened for writing", (char *) NULL);
	return HAX_ERROR;
    }
    f = filePtr->f2;
    if (f == NULL) {
	f = filePtr->f;
    }
    if (fflush(f) == EOF) {
	Hax_AppendResult(interp, "error flushing \"", argv[1],
		"\": ", Hax_UnixError(interp), (char *) NULL);
	clearerr(f);
	return HAX_ERROR;
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_GetsCmd --
 *
 *	This procedure is invoked to process the "gets" Hax command.
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
Hax_GetsCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
#   define BUF_SIZE 200
    char buffer[BUF_SIZE+1];
    int totalCount, done, flags;
    UnixClientData *clientDataPtr;
    OpenFile *filePtr;
    FILE *f;

    if ((argc != 2) && (argc != 3)) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileId ?varName?\"", (char *) NULL);
	return HAX_ERROR;
    }

    clientDataPtr = (UnixClientData *) clientData;
    if (HaxGetOpenFile(interp, clientDataPtr, argv[1], &filePtr) != HAX_OK) {
	return HAX_ERROR;
    }
    if (!filePtr->readable) {
	Hax_AppendResult(interp, "\"", argv[1],
		"\" wasn't opened for reading", (char *) NULL);
	return HAX_ERROR;
    }

    /*
     * We can't predict how large a line will be, so read it in
     * pieces, appending to the current result or to a variable.
     */

    totalCount = 0;
    done = 0;
    flags = 0;
    f = filePtr->f;
    while (!done) {
	int c, count;
	char *p;

	for (p = buffer, count = 0; count < BUF_SIZE-1; count++, p++) {
	    c = getc(f);
	    if (c == EOF) {
		if (ferror(filePtr->f)) {
		    Hax_ResetResult(interp);
		    Hax_AppendResult(interp, "error reading \"", argv[1],
			    "\": ", Hax_UnixError(interp), (char *) NULL);
		    clearerr(filePtr->f);
		    return HAX_ERROR;
		} else if (feof(filePtr->f)) {
		    if ((totalCount == 0) && (count == 0)) {
			totalCount = -1;
		    }
		    done = 1;
		    break;
		}
	    }
	    if (c == '\n') {
		done = 1;
		break;
	    }
	    *p = c;
	}
	*p = 0;
	if (argc == 2) {
	    Hax_AppendResult(interp, buffer, (char *) NULL);
	} else {
	    if (Hax_SetVar(interp, argv[2], buffer, flags|HAX_LEAVE_ERR_MSG)
		    == NULL) {
		return HAX_ERROR;
	    }
	    flags = HAX_APPEND_VALUE;
	}
	totalCount += count;
    }

    if (argc == 3) {
	sprintf(interp->result, "%d", totalCount);
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_OpenCmd --
 *
 *	This procedure is invoked to process the "open" Hax command.
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
Hax_OpenCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    Hax_Memoryp *memoryp;
    UnixClientData *clientDataPtr;
    int pipeline, fd;
    char *access;
    OpenFile *filePtr;

    memoryp = Hax_GetMemoryp(interp);

    if (argc == 2) {
	access = (char *) "r";
    } else if (argc == 3) {
	access = argv[2];
    } else {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" filename ?access?\"", (char *) NULL);
	return HAX_ERROR;
    }

    filePtr = (OpenFile *) ckalloc(memoryp, sizeof(OpenFile));
    filePtr->f = NULL;
    filePtr->f2 = NULL;
    filePtr->readable = 0;
    filePtr->writable = 0;
    filePtr->numPids = 0;
    filePtr->pidPtr = NULL;
    filePtr->errorId = -1;

    /*
     * Verify the requested form of access.
     */

    pipeline = 0;
    if (argv[1][0] == '|') {
	pipeline = 1;
    }
    switch (access[0]) {
	case 'r':
	    filePtr->readable = 1;
	    break;
	case 'w':
	    filePtr->writable = 1;
	    break;
	case 'a':
	    filePtr->writable = 1;
	    break;
	default:
	    badAccess:
	    Hax_AppendResult(interp, "illegal access mode \"", access,
		    "\"", (char *) NULL);
	    goto error;
    }
    if (access[1] == '+') {
	filePtr->readable = filePtr->writable = 1;
	if (access[2] != 0) {
	    goto badAccess;
	}
    } else if (access[1] != 0) {
	goto badAccess;
    }

    /*
     * Open the file or create a process pipeline.
     */

    if (!pipeline) {
	char *fileName = argv[1];

	if (fileName[0] == '~') {
	    fileName = Hax_TildeSubst(interp, clientData, fileName);
	    if (fileName == NULL) {
		goto error;
	    }
	}
	filePtr->f = fopen(fileName, access);
	if (filePtr->f == NULL) {
	    Hax_AppendResult(interp, "couldn't open \"", argv[1],
		    "\": ", Hax_UnixError(interp), (char *) NULL);
	    goto error;
	}
    } else {
	int *inPipePtr, *outPipePtr;
	int cmdArgc, inPipe, outPipe;
	char **cmdArgv;

	if (Hax_SplitList(interp, argv[1]+1, &cmdArgc, &cmdArgv) != HAX_OK) {
	    goto error;
	}
	inPipePtr = (filePtr->writable) ? &inPipe : NULL;
	outPipePtr = (filePtr->readable) ? &outPipe : NULL;
	inPipe = outPipe = -1;
	filePtr->numPids = Hax_CreatePipeline(interp, clientData, cmdArgc,
		cmdArgv, &filePtr->pidPtr, inPipePtr, outPipePtr,
		&filePtr->errorId);
	ckfree(memoryp, (char *) cmdArgv);
	if (filePtr->numPids < 0) {
	    goto error;
	}
	if (filePtr->readable) {
	    if (outPipe == -1) {
		if (inPipe != -1) {
		    close(inPipe);
		}
		Hax_AppendResult(interp, "can't read output from command:",
			" standard output was redirected", (char *) NULL);
		goto error;
	    }
	    filePtr->f = fdopen(outPipe, "r");
	}
	if (filePtr->writable) {
	    if (inPipe == -1) {
		Hax_AppendResult(interp, "can't write input to command:",
			" standard input was redirected", (char *) NULL);
		goto error;
	    }
	    if (filePtr->f != NULL) {
		filePtr->f2 = fdopen(inPipe, "w");
	    } else {
		filePtr->f = fdopen(inPipe, "w");
	    }
	}
    }

    /*
     * Enter this new OpenFile structure in the table for the
     * interpreter.  May have to expand the table to do this.
     */

    fd = fileno(filePtr->f);
    clientDataPtr = (UnixClientData *)clientData;
    HaxMakeFileTable(interp, clientDataPtr, fd);
    if (clientDataPtr->filePtrArray[fd] != NULL) {
	Hax_Panic((char *) "Hax_OpenCmd found file already open");
    }
    clientDataPtr->filePtrArray[fd] = filePtr;
    sprintf(interp->result, "file%d", fd);
    return HAX_OK;

    error:
    if (filePtr->f != NULL) {
	fclose(filePtr->f);
    }
    if (filePtr->f2 != NULL) {
	fclose(filePtr->f2);
    }
    if (filePtr->numPids > 0) {
	Hax_DetachPids(filePtr->numPids, filePtr->pidPtr);
	ckfree(memoryp, (char *) filePtr->pidPtr);
    }
    if (filePtr->errorId != -1) {
	close(filePtr->errorId);
    }
    ckfree(memoryp, (char *) filePtr);
    return HAX_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_PwdCmd --
 *
 *	This procedure is invoked to process the "pwd" Hax command.
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
Hax_PwdCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    Hax_Memoryp *memoryp;
    UnixClientData *clientDataPtr;
    char buffer[MAXPATHLEN+1];

    memoryp = Hax_GetMemoryp(interp);
    clientDataPtr = (UnixClientData *) clientData;

    if (argc != 1) {
	Hax_AppendResult(interp, "wrong # args: should be \"",
		argv[0], "\"", (char *) NULL);
	return HAX_ERROR;
    }
    if (clientDataPtr->currentDir == NULL) {
	if (getcwd(buffer, MAXPATHLEN) == NULL) {
	    if (errno == ERANGE) {
		interp->result = (char *) "working directory name is too long";
	    } else {
		Hax_AppendResult(interp,
			"error getting working directory name: ",
			Hax_UnixError(interp), (char *) NULL);
	    }
	    return HAX_ERROR;
	}
	clientDataPtr->currentDir = (char *) ckalloc(memoryp, (unsigned)
		(strlen(buffer) + 1));
	strcpy(clientDataPtr->currentDir, buffer);
    }
    interp->result = clientDataPtr->currentDir;
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_PutsCmd --
 *
 *	This procedure is invoked to process the "puts" Hax command.
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
Hax_PutsCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    UnixClientData *clientDataPtr;
    OpenFile *filePtr;
    FILE *f;
    int i, newline;
    char *fileId;

    i = 1;
    newline = 1;
    if ((argc >= 2) && (strcmp(argv[1], "-nonewline") == 0)) {
	newline = 0;
	i++;
    }
    if ((i < (argc-3)) || (i >= argc)) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		"\" ?-nonewline? ?fileId? string", (char *) NULL);
	return HAX_ERROR;
    }

    /*
     * The code below provides backwards compatibility with an old
     * form of the command that is no longer recommended or documented.
     */

    if (i == (argc-3)) {
	if (strncmp(argv[i+2], "nonewline", strlen(argv[i+2])) != 0) {
	    Hax_AppendResult(interp, "bad argument \"", argv[i+2],
		    "\": should be \"nonewline\"", (char *) NULL);
	    return HAX_ERROR;
	}
	newline = 0;
    }
    if (i == (argc-1)) {
	fileId = (char *) "stdout";
    } else {
	fileId = argv[i];
	i++;
    }

    clientDataPtr = (UnixClientData *) clientData;
    if (HaxGetOpenFile(interp, clientDataPtr, fileId, &filePtr) != HAX_OK) {
	return HAX_ERROR;
    }
    if (!filePtr->writable) {
	Hax_AppendResult(interp, "\"", fileId,
		"\" wasn't opened for writing", (char *) NULL);
	return HAX_ERROR;
    }
    f = filePtr->f2;
    if (f == NULL) {
	f = filePtr->f;
    }

    fputs(argv[i], f);
    if (newline) {
	fputc('\n', f);
    }
    if (ferror(f)) {
	Hax_AppendResult(interp, "error writing \"", fileId,
		"\": ", Hax_UnixError(interp), (char *) NULL);
	clearerr(f);
	return HAX_ERROR;
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ReadCmd --
 *
 *	This procedure is invoked to process the "read" Hax command.
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
Hax_ReadCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    UnixClientData *clientDataPtr;
    OpenFile *filePtr;
    long int bytesLeft, bytesRead, count;
#define READ_BUF_SIZE 4096
    char buffer[READ_BUF_SIZE+1];
    int newline, i;

    if ((argc != 2) && (argc != 3)) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileId ?numBytes?\" or \"", argv[0],
		" ?-nonewline? fileId\"", (char *) NULL);
	return HAX_ERROR;
    }
    i = 1;
    newline = 1;
    if ((argc == 3) && (strcmp(argv[1], "-nonewline") == 0)) {
	newline = 0;
	i++;
    }

    clientDataPtr = (UnixClientData *) clientData;
    if (HaxGetOpenFile(interp, clientDataPtr, argv[i], &filePtr) != HAX_OK) {
	return HAX_ERROR;
    }
    if (!filePtr->readable) {
	Hax_AppendResult(interp, "\"", argv[i],
		"\" wasn't opened for reading", (char *) NULL);
	return HAX_ERROR;
    }

    /*
     * Compute how many bytes to read, and see whether the final
     * newline should be dropped.
     */

    if ((argc >= (i + 2)) && isdigit(argv[i+1][0])) {
	if (Hax_GetLong(interp, argv[i+1], &bytesLeft) != HAX_OK) {
	    return HAX_ERROR;
	}
    } else {
	bytesLeft = 1LL<<62;

	/*
	 * The code below provides backward compatibility for an
	 * archaic earlier version of this command.
	 */

	if (argc >= (i + 2)) {
	    if (strncmp(argv[i+1], "nonewline", strlen(argv[i+1])) == 0) {
		newline = 0;
	    } else {
		Hax_AppendResult(interp, "bad argument \"", argv[i+1],
			"\": should be \"nonewline\"", (char *) NULL);
		return HAX_ERROR;
	    }
	}
    }

    /*
     * Read the file in one or more chunks.
     */

    bytesRead = 0;
    while (bytesLeft > 0) {
	count = READ_BUF_SIZE;
	if (bytesLeft < READ_BUF_SIZE) {
	    count = bytesLeft;
	}
	count = fread(buffer, 1, count, filePtr->f);
	if (ferror(filePtr->f)) {
	    Hax_ResetResult(interp);
	    Hax_AppendResult(interp, "error reading \"", argv[i],
		    "\": ", Hax_UnixError(interp), (char *) NULL);
	    clearerr(filePtr->f);
	    return HAX_ERROR;
	}
	if (count == 0) {
	    break;
	}
	buffer[count] = 0;
	Hax_AppendResult(interp, buffer, (char *) NULL);
	bytesLeft -= count;
	bytesRead += count;
    }
    if ((newline == 0) && (bytesRead > 0)
	    && (interp->result[bytesRead-1] == '\n')) {
	interp->result[bytesRead-1] = 0;
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_SeekCmd --
 *
 *	This procedure is invoked to process the "seek" Hax command.
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
Hax_SeekCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    UnixClientData *clientDataPtr;
    OpenFile *filePtr;
    long long int offset;
    int mode;

    if ((argc != 3) && (argc != 4)) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileId offset ?origin?\"", (char *) NULL);
	return HAX_ERROR;
    }

    clientDataPtr = (UnixClientData *) clientData;
    if (HaxGetOpenFile(interp, clientDataPtr, argv[1], &filePtr) != HAX_OK) {
	return HAX_ERROR;
    }
    if (Hax_GetLongLong(interp, argv[2], &offset) != HAX_OK) {
	return HAX_ERROR;
    }
    mode = SEEK_SET;
    if (argc == 4) {
	int length;
	char c;

	length = strlen(argv[3]);
	c = argv[3][0];
	if ((c == 's') && (strncmp(argv[3], "start", length) == 0)) {
	    mode = SEEK_SET;
	} else if ((c == 'c') && (strncmp(argv[3], "current", length) == 0)) {
	    mode = SEEK_CUR;
	} else if ((c == 'e') && (strncmp(argv[3], "end", length) == 0)) {
	    mode = SEEK_END;
	} else {
	    Hax_AppendResult(interp, "bad origin \"", argv[3],
		    "\": should be start, current, or end", (char *) NULL);
	    return HAX_ERROR;
	}
    }
    if (fseeko(filePtr->f, offset, mode) == -1) {
	Hax_AppendResult(interp, "error during seek: ",
		Hax_UnixError(interp), (char *) NULL);
	clearerr(filePtr->f);
	return HAX_ERROR;
    }

    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_SourceCmd --
 *
 *	This procedure is invoked to process the "source" Hax command.
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
Hax_SourceCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    if (argc != 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileName\"", (char *) NULL);
	return HAX_ERROR;
    }
    return Hax_EvalFile(interp, clientData, argv[1]);
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_TellCmd --
 *
 *	This procedure is invoked to process the "tell" Hax command.
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
Hax_TellCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    UnixClientData *clientDataPtr;
    OpenFile *filePtr;

    if (argc != 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileId\"", (char *) NULL);
	return HAX_ERROR;
    }

    clientDataPtr = (UnixClientData *) clientData;
    if (HaxGetOpenFile(interp, clientDataPtr, argv[1], &filePtr) != HAX_OK) {
	return HAX_ERROR;
    }
    sprintf(interp->result, "%lld", (long long int) ftello(filePtr->f));
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_TimeCmd --
 *
 *	This procedure is invoked to process the "time" Hax command.
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
Hax_TimeCmd(
    ClientData clientData,		/* Unix ClientData */
    Hax_Interp *interp,			/* Current interpreter. */
    int argc,				/* Number of arguments. */
    char **argv				/* Argument strings. */)
{
    long long int count, i, result;
    double timePer;
    struct timeval start, stop;
    struct timezone tz;
    int micros;

    if (argc == 2) {
	count = 1;
    } else if (argc == 3) {
	if (Hax_GetLongLong(interp, argv[2], &count) != HAX_OK) {
	    return HAX_ERROR;
	}
    } else {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" command ?count?\"", (char *) NULL);
	return HAX_ERROR;
    }
    gettimeofday(&start, &tz);
    for (i = count ; i > 0; i--) {
	result = Hax_Eval(interp, NULL, argv[1], 0, (char **) NULL);
	if (result != HAX_OK) {
	    if (result == HAX_ERROR) {
		char msg[60];
		sprintf(msg, "\n    (\"time\" body line %d)",
			interp->errorLine);
		Hax_AddErrorInfo(interp, msg);
	    }
	    return result;
	}
    }
    gettimeofday(&stop, &tz);
    micros = (stop.tv_sec - start.tv_sec)*1000000
	    + (stop.tv_usec - start.tv_usec);
    timePer = micros;

    Hax_ResetResult(interp);
    sprintf(interp->result, "%.0f microseconds per iteration", timePer/count);
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * CleanupChildren --
 *
 *	This is a utility procedure used to wait for child processes
 *	to exit, record information about abnormal exits, and then
 *	collect any stderr output generated by them.
 *
 * Results:
 *	The return value is a standard Hax result.  If anything at
 *	weird happened with the child processes, HAX_ERROR is returned
 *	and a message is left in interp->result.
 *
 * Side effects:
 *	If the last character of interp->result is a newline, then it
 *	is removed.  File errorId gets closed, and pidPtr is freed
 *	back to the storage allocator.
 *
 *----------------------------------------------------------------------
 */

static int
CleanupChildren(
    Hax_Interp *interp,		/* Used for error messages. */
    int numPids,		/* Number of entries in pidPtr array. */
    int *pidPtr,		/* Array of process ids of children. */
    int errorId			/* File descriptor index for file containing
				 * stderr output from pipeline.  -1 means
				 * there isn't any stderr output. */)
{
    Hax_Memoryp *memoryp;
    int result = HAX_OK;
    int i, pid, length;
    int waitStatus;

    memoryp = Hax_GetMemoryp(interp);

    for (i = 0; i < numPids; i++) {
	pid = Hax_WaitPids(1, &pidPtr[i], (int *) &waitStatus);
	if (pid == -1) {
	    Hax_AppendResult(interp, "error waiting for process to exit: ",
		    Hax_UnixError(interp), (char *) NULL);
	    continue;
	}

	/*
	 * Create error messages for unusual process exits.  An
	 * extra newline gets appended to each error message, but
	 * it gets removed below (in the same fashion that an
	 * extra newline in the command's output is removed).
	 */

	if (!WIFEXITED(waitStatus) || (WEXITSTATUS(waitStatus) != 0)) {
	    char msg1[20], msg2[20];

	    result = HAX_ERROR;
	    sprintf(msg1, "%d", pid);
	    if (WIFEXITED(waitStatus)) {
		sprintf(msg2, "%d", WEXITSTATUS(waitStatus));
		Hax_SetErrorCode(interp, "CHILDSTATUS", msg1, msg2,
			(char *) NULL);
	    } else if (WIFSIGNALED(waitStatus)) {
		char *p;

		p = Hax_SignalMsg((int) (WTERMSIG(waitStatus)));
		Hax_SetErrorCode(interp, "CHILDKILLED", msg1,
			Hax_SignalId((int) (WTERMSIG(waitStatus))), p,
			(char *) NULL);
		Hax_AppendResult(interp, "child killed: ", p, "\n",
			(char *) NULL);
	    } else if (WIFSTOPPED(waitStatus)) {
		char *p;

		p = Hax_SignalMsg((int) (WSTOPSIG(waitStatus)));
		Hax_SetErrorCode(interp, "CHILDSUSP", msg1,
			Hax_SignalId((int) (WSTOPSIG(waitStatus))), p, (char *) NULL);
		Hax_AppendResult(interp, "child suspended: ", p, "\n",
			(char *) NULL);
	    } else {
		Hax_AppendResult(interp,
			"child wait status didn't make sense\n",
			(char *) NULL);
	    }
	}
    }
    ckfree(memoryp, (char *) pidPtr);

    /*
     * Read the standard error file.  If there's anything there,
     * then return an error and add the file's contents to the result
     * string.
     */

    if (errorId >= 0) {
	while (1) {
#	    define BUFFER_SIZE 1000
	    char buffer[BUFFER_SIZE+1];
	    int count;

	    count = read(errorId, buffer, BUFFER_SIZE);

	    if (count == 0) {
		break;
	    }
	    if (count < 0) {
		Hax_AppendResult(interp,
			"error reading stderr output file: ",
			Hax_UnixError(interp), (char *) NULL);
		break;
	    }
	    buffer[count] = 0;
	    Hax_AppendResult(interp, buffer, (char *) NULL);
	}
	close(errorId);
    }

    /*
     * If the last character of interp->result is a newline, then remove
     * the newline character (the newline would just confuse things).
     */

    length = strlen(interp->result);
    if ((length > 0) && (interp->result[length-1] == '\n')) {
	interp->result[length-1] = '\0';
    }

    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_UnixCoreDelete --
 *     Destroy the UNIX core.
 *
 *----------------------------------------------------------------------
 */

void
Hax_UnixCoreDelete(
    Hax_Interp *interp,
    ClientData clientData)
{
    Hax_Memoryp *memoryp;
    UnixClientData *clientDataPtr;
    int i;

    clientDataPtr = (UnixClientData *)clientData;
    clientDataPtr->refCount--;

    if (clientDataPtr->refCount > 0)
	return;

    memoryp = Hax_GetMemoryp(interp);

    if (clientDataPtr->numFiles > 0) {
	for (i = 0; i < clientDataPtr->numFiles; i++) {
	    OpenFile *filePtr;

	    filePtr = clientDataPtr->filePtrArray[i];
	    if (filePtr == NULL) {
		continue;
	    }
	    if (i >= 3) {
		fclose(filePtr->f);
		if (filePtr->f2 != NULL) {
		    fclose(filePtr->f2);
		}
		if (filePtr->numPids > 0) {
		    Hax_DetachPids(filePtr->numPids, filePtr->pidPtr);
		    ckfree(memoryp, (char *) filePtr->pidPtr);
		}
	    }
	    ckfree(memoryp, (char *) filePtr);
	}
	ckfree(memoryp, (char *) clientDataPtr->filePtrArray);
    }

    if (clientDataPtr->currentDir != NULL) {
	ckfree(memoryp, clientDataPtr->currentDir);
    }

    if (clientDataPtr->curBuf != clientDataPtr->staticBuf) {
	ckfree(memoryp, clientDataPtr->curBuf);
    }

    ckfree(memoryp, (char *) clientDataPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_InitUnixCore --
 *     Initialize the UNIX core.
 *
 *----------------------------------------------------------------------
 */
ClientData
Hax_InitUnixCore(
    Hax_Interp *interp)
{
    Hax_Memoryp *memoryp;
    CmdInfo *cmdInfoPtr;
    UnixClientData *clientDataPtr;
    char *libraryPath;

    memoryp = Hax_GetMemoryp(interp);

    clientDataPtr = (UnixClientData *) ckalloc(memoryp, sizeof(UnixClientData));
    clientDataPtr->numFiles = 0;
    clientDataPtr->filePtrArray = NULL;
    clientDataPtr->refCount = 0;
    clientDataPtr->currentDir = NULL;
    memset(&clientDataPtr->staticBuf, 0, sizeof(clientDataPtr->staticBuf));
    clientDataPtr->curSize = STATIC_BUF_SIZE;
    clientDataPtr->curBuf = clientDataPtr->staticBuf;

    for (cmdInfoPtr = builtInCmds; cmdInfoPtr->name != NULL;
	 cmdInfoPtr++) {
	Hax_CreateCommand (interp, (char *) cmdInfoPtr->name,
	                   cmdInfoPtr->proc, (ClientData) clientDataPtr,
	                   Hax_UnixCoreDelete);

	clientDataPtr->refCount++;
    }

    libraryPath = getenv("HAX_LIBRARY");
    if (libraryPath != NULL) {
	Hax_SetLibraryPath(interp, libraryPath);
    }

    HaxSetupEnv(interp);

    return (ClientData) clientDataPtr;
}
