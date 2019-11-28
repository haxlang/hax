/*
 * haxsh.c --
 *
 *	Interactive driver for Hax.
 *
 * Copyright 1987-1991 Regents of the University of California
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appears in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /user6/ouster/tcl/tclTest/RCS/tclTest.c,v 1.22 92/12/18 10:30:56 ouster Exp $ SPRITE (Berkeley)";
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifndef RHAXSH
#include <unistd.h>
#endif

#include "hax.h"

Hax_Memoryp *memoryp;
Hax_Interp *interp;
Hax_CmdBuf buffer;
int quitFlag = 0;

#ifndef RHAXSH
extern char **environ;
ClientData unixClientData;

char initCmd[] =
    "if [file exists [info library]/init.tcl] {source [info library]/init.tcl}";

static void
usage(void)
{
    fprintf(stderr, "haxsh: [-f fileName] args...\n");
    exit(1);
}
#endif

	/* ARGSUSED */
int
cmdCheckmem(
    ClientData clientData,
    Hax_Interp *interp,
    int argc,
    char *argv[])
{
    if (argc != 1) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		"\"", (char *) NULL);
	return HAX_ERROR;
    }
    quitFlag = 1;
    return HAX_OK;
}

	/* ARGSUSED */
int
cmdEcho(
    ClientData clientData,
    Hax_Interp *interp,
    int argc,
    char *argv[])
{
    int i;

    for (i = 1; ; i++) {
	if (argv[i] == NULL) {
	    if (i != argc) {
		echoError:
		sprintf(interp->result,
		    "argument list wasn't properly NULL-terminated in \"%s\" command",
		    argv[0]);
	    }
	    break;
	}
	if (i >= argc) {
	    goto echoError;
	}
	fputs(argv[i], stdout);
	if (i < (argc-1)) {
	    printf(" ");
	}
    }
    printf("\n");
    return HAX_OK;
}

#ifndef RHAXSH
static void
writeEnv(
    Hax_Interp *interp,
    ClientData clientData,
    char *name,
    char *value)
{
    setenv(name, value, 1);
}

static void
unsetEnv(
    Hax_Interp *interp,
    ClientData clientData,
    char *name)
{
    unsetenv(name);
}

static void
destroyEnv(
    Hax_Interp *interp,
    ClientData clientData)
{
    environ = NULL;
}
#endif

int
main(int argc, char **argv)
{
    int i;
    char line[1000], *cmd;
    int result, gotPartial;
    char *argv0 = argv[0];
#ifndef RHAXSH
    int ch;
    char *fileName = NULL;
#endif

    memoryp = Hax_CreateMemoryManagement(0, 0, 0, 0, 1);
    interp = Hax_CreateInterp(memoryp);
    Hax_InitMemory(interp);
#ifndef RHAXSH
    unixClientData = (ClientData) Hax_InitUnixCore(interp);
    Hax_EnvTraceProc(interp, unixClientData, writeEnv, unsetEnv, destroyEnv);
#endif
    Hax_CreateCommand(interp, (char *) "echo", cmdEcho, (ClientData) "echo",
	    (Hax_CmdDeleteProc *) NULL);
    Hax_CreateCommand(interp, (char *) "checkmem", cmdCheckmem, (ClientData) 0,
	    (Hax_CmdDeleteProc *) NULL);
    buffer = Hax_CreateCmdBuf(interp);

#ifndef RHAXSH
    while ((ch = getopt(argc, argv, "f:")) != -1) {
	switch (ch) {
	case 'f':
	    if (fileName != NULL) {
		usage();
	    }
	    fileName = optarg;
	    argv0 = fileName;
	    break;
	default:
	    usage();
	    /* NOTREACHED */
	}
    }

    argc -= optind;
    argv += optind;
#endif

    Hax_SetVar(interp, (char *) "argv0", argv0, HAX_GLOBAL_ONLY);
    Hax_SetVar(interp, (char *) "argv", "", HAX_GLOBAL_ONLY);
    for (i = 0; i < argc; i++) {
	Hax_SetVar(interp, (char *) "argv", argv[i], HAX_APPEND_VALUE | HAX_LIST_ELEMENT);
    }
    sprintf(line, "%d", argc);
    Hax_SetVar(interp, (char *) "argc", line, HAX_GLOBAL_ONLY);

#ifndef RHAXSH
    result = Hax_Eval(interp, NULL, initCmd, 0, (char **) NULL);
    if (result != HAX_OK) {
	printf("%s\n", interp->result);
	exit(1);
    }

    if (fileName != NULL) {
	Hax_EvalFile(interp, unixClientData, fileName);
	if (result != HAX_OK) {
	    printf("%s\n", interp->result);
	    exit(1);
	}
	exit(0);
    }
#endif

    gotPartial = 0;
    while (1) {
	clearerr(stdin);
	if (!gotPartial) {
	    fputs("% ", stdout);
	    fflush(stdout);
	}
	if (fgets(line, 1000, stdin) == NULL) {
	    if (!gotPartial) {
		exit(0);
	    }
	    line[0] = 0;
	}
	cmd = Hax_AssembleCmd(interp, buffer, line);
	if (cmd == NULL) {
	    gotPartial = 1;
	    continue;
	}

	gotPartial = 0;
	result = Hax_RecordAndEval(interp, cmd, 0);
	if (result == HAX_OK) {
	    if (*interp->result != 0) {
		printf("%s\n", interp->result);
	    }
	    if (quitFlag) {
		Hax_DeleteCmdBuf(interp, buffer);
		Hax_DeleteInterp(interp);
		Hax_DumpActiveMemory(memoryp);
		exit(0);
	    }
	} else {
	    if (result == HAX_ERROR) {
		printf("Error");
	    } else {
		printf("Error %d", result);
	    }
	    if (*interp->result != 0) {
		printf(": %s\n", interp->result);
	    } else {
		printf("\n");
	    }
	}
    }
}
