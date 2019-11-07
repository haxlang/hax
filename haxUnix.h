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

#endif /* _HAXUNIX */
