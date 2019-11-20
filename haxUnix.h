/*
 * haxUnix.h --
 *
 *	This file reads in UNIX-related header files and sets up
 *	UNIX-related macros for Hax's UNIX core.  It should be the
 *	only file that contains #ifdefs to handle different flavors
 *	of UNIX.  This file sets up the union of all UNIX-related
 *	things needed by any of the Hax core files.
 *
 *	The material in this file was originally contributed by
 *	Karl Lehenbauer, Mark Diekhans and Peter da Silva.
 *
 * Copyright 1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that this copyright
 * notice appears in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * $Header: /user6/ouster/tcl/RCS/tclUnix.h,v 1.31 92/12/23 16:49:17 ouster Exp $ SPRITE (Berkeley)
 */

#ifndef _HAXUNIX
#define _HAXUNIX

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*
 * Make sure that MAXPATHLEN is defined.
 */

#ifndef MAXPATHLEN
#   ifdef PATH_MAX
#       define MAXPATHLEN PATH_MAX
#   else
#       define MAXPATHLEN 2048
#   endif
#endif

/*
 * Variables provided by the C library:
 */

extern char **environ;

/*
 * The data structure below defines an open file (or connection to
 * a process pipeline) as returned by the "open" command.
 */

typedef struct OpenFile {
    FILE *f;			/* Stdio file to use for reading and/or
				 * writing. */
    FILE *f2;			/* Normally NULL.  In the special case of
				 * a command pipeline with pipes for both
				 * input and output, this is a stdio file
				 * to use for writing to the pipeline. */
    int readable;		/* Non-zero means file may be read. */
    int writable;		/* Non-zero means file may be written. */
    int numPids;		/* If this is a connection to a process
				 * pipeline, gives number of processes
				 * in pidPtr array below;  otherwise it
				 * is 0. */
    int *pidPtr;		/* Pointer to malloc-ed array of child
				 * process ids (numPids of them), or NULL
				 * if this isn't a connection to a process
				 * pipeline. */
    int errorId;		/* File id of file that receives error
				 * output from pipeline.  -1 means not
				 * used (i.e. this is a normal file). */
} OpenFile;

typedef struct UnixClientData {
    /*
     * Information related to files.
     */

    int numFiles;		/* Number of entries in filePtrArray
				 * below.  0 means array hasn't been
				 * created yet. */
    OpenFile **filePtrArray;	/* Pointer to malloc-ed array of pointers
				 * to information about open files.  Entry
				 * N corresponds to the file with fileno N.
				 * If an entry is NULL then the corresponding
				 * file isn't open.  If filePtrArray is NULL
				 * it means no files have been used, so even
				 * stdin/stdout/stderr entries haven't been
				 * setup yet. */
    int refCount;		/* Reference count of UNIX commands created
				 * and still in use. On termination it will
				 * be decremented and freed with the last
				 * user. */
    /*
     * Used in haxUnixAZ.c
     */
    char *currentDir;		/* The variable caches the name of the current
				 * working directory in order to avoid repeated
				 * calls to getwd.  The string is malloc-ed.
				 * NULL means the cache needs to be refreshed.
				 */
} UnixClientData;

/*
 *----------------------------------------------------------------
 * Procedures shared among Hax modules but not used by the outside
 * world:
 *----------------------------------------------------------------
 */

extern int		HaxGetOpenFile (Hax_Interp *interp,
			    UnixClientData *clientDataPtr,
			    char *string, OpenFile **filePtrPtr);
extern void		HaxMakeFileTable (Hax_Interp *interp,
			    UnixClientData *clientDataPtr, int index);
extern void		HaxSetupEnv (Hax_Interp *interp);

/*
 *----------------------------------------------------------------
 * Command procedures in the UNIX core:
 *----------------------------------------------------------------
 */

extern int	Hax_CdCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_CloseCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_EofCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_ExecCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_ExitCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_FileCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_FlushCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_GetsCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_GlobCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_OpenCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_PutsCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_PwdCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_ReadCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_SeekCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_SourceCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_TellCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);
extern int	Hax_TimeCmd (ClientData clientData,
		    Hax_Interp *interp, int argc, char **argv);

#endif /* _HAXUNIX */
