/*
 * haxTest.c --
 *
 *	Test driver for HAX.
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
#include "hax.h"

Hax_Memoryp *memoryp;
Hax_Interp *interp;
Hax_CmdBuf buffer;
char dumpFile[100];
int quitFlag = 0;

char initCmd[] =
    "if [file exists [info library]/init.tcl] {source [info library]/init.tcl}";

	/* ARGSUSED */
int
cmdCheckmem(
    ClientData clientData,
    Hax_Interp *interp,
    int argc,
    char *argv[])
{
    if (argc != 2) {
	Hax_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileName\"", (char *) NULL);
	return HAX_ERROR;
    }
    strcpy(dumpFile, argv[1]);
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

int
main(int argc, char **argv)
{
    char line[1000], *cmd;
    int result, gotPartial;

    memoryp = Hax_CreateMemoryManagement(0, 0, 0, 0, 1);
    interp = Hax_CreateInterp(memoryp);
    Hax_InitMemory(interp);
    Hax_InitUnixCore(interp);
    Hax_CreateCommand(interp, (char *) "echo", cmdEcho, (ClientData) "echo",
	    (Hax_CmdDeleteProc *) NULL);
    Hax_CreateCommand(interp, (char *) "checkmem", cmdCheckmem, (ClientData) 0,
	    (Hax_CmdDeleteProc *) NULL);
    buffer = Hax_CreateCmdBuf(interp);
    result = Hax_Eval(interp, NULL, initCmd, 0, (char **) NULL);
    if (result != HAX_OK) {
	printf("%s\n", interp->result);
	exit(1);
    }

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
		Hax_DeleteInterp(interp);
		Hax_DeleteCmdBuf(interp, buffer);
		Hax_DumpActiveMemory(memoryp, dumpFile);
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
