/*
 * tcl.h --
 *
 *	This header file describes the externally-visible facilities
 *	of the Tcl interpreter.
 *
 * Copyright 1987-1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * $Header: /user6/ouster/tcl/RCS/tcl.h,v 1.92 93/02/04 15:48:05 ouster Exp $ SPRITE (Berkeley)
 */

#ifndef _TCL
#define _TCL

#define TCL_VERSION "6.7"
#define TCL_MAJOR_VERSION 6
#define TCL_MINOR_VERSION 7

/*
 * Definitions that allow this header to be used with C++.
 */

#ifdef __cplusplus
#   define EXTERN extern "C"
#else
#   define EXTERN extern
#endif

/*
 * Miscellaneous declarations (to allow Tcl to be used stand-alone,
 * without the rest of Sprite).
 */

#ifndef NULL
#define NULL 0
#endif

#ifndef _CLIENTDATA
typedef int *ClientData;
#define _CLIENTDATA
#endif

/*
 * Data structures defined opaquely in this module.  The definitions
 * below just provide dummy types.  A few fields are made visible in
 * Tcl_Interp structures, namely those for returning string values.
 * Note:  any change to the Tcl_Interp definition below must be mirrored
 * in the "real" definition in tclInt.h.
 */

typedef struct Tcl_Interp{
    char *result;		/* Points to result string returned by last
				 * command. */
    void (*freeProc) (char *blockPtr);
				/* Zero means result is statically allocated.
				 * If non-zero, gives address of procedure
				 * to invoke to free the result.  Must be
				 * freed by Tcl_Eval before executing next
				 * command. */
    int errorLine;		/* When TCL_ERROR is returned, this gives
				 * the line number within the command where
				 * the error occurred (1 means first line). */
} Tcl_Interp;

typedef int *Tcl_Trace;
typedef int *Tcl_CmdBuf;

/*
 * When a TCL command returns, the string pointer interp->result points to
 * a string containing return information from the command.  In addition,
 * the command procedure returns an integer value, which is one of the
 * following:
 *
 * TCL_OK		Command completed normally;  interp->result contains
 *			the command's result.
 * TCL_ERROR		The command couldn't be completed successfully;
 *			interp->result describes what went wrong.
 * TCL_RETURN		The command requests that the current procedure
 *			return;  interp->result contains the procedure's
 *			return value.
 * TCL_BREAK		The command requests that the innermost loop
 *			be exited;  interp->result is meaningless.
 * TCL_CONTINUE		Go on to the next iteration of the current loop;
 *			interp->result is meaninless.
 */

#define TCL_OK		0
#define TCL_ERROR	1
#define TCL_RETURN	2
#define TCL_BREAK	3
#define TCL_CONTINUE	4

#define TCL_RESULT_SIZE 199

/*
 * Procedure types defined by Tcl:
 */

typedef void (Tcl_CmdDeleteProc) (ClientData clientData);
typedef int (Tcl_CmdProc) (ClientData clientData,
	Tcl_Interp *interp, int argc, char *argv[]);
typedef void (Tcl_CmdTraceProc) (ClientData clientData,
	Tcl_Interp *interp, int level, char *command, Tcl_CmdProc *proc,
	ClientData cmdClientData, int argc, char *argv[]);
typedef void (Tcl_FreeProc) (char *blockPtr);
typedef char *(Tcl_VarTraceProc) (ClientData clientData,
	Tcl_Interp *interp, char *part1, char *part2, int flags);

/*
 * Flag values passed to Tcl_Eval (see the man page for details;  also
 * see tclInt.h for additional flags that are only used internally by
 * Tcl):
 */

#define TCL_BRACKET_TERM	1

/*
 * Flag that may be passed to Tcl_ConvertElement to force it not to
 * output braces (careful!  if you change this flag be sure to change
 * the definitions at the front of tclUtil.c).
 */

#define TCL_DONT_USE_BRACES	1

/*
 * Flag value passed to Tcl_RecordAndEval to request no evaluation
 * (record only).
 */

#define TCL_NO_EVAL		-1

/*
 * Specil freeProc values that may be passed to Tcl_SetResult (see
 * the man page for details):
 */

#define TCL_VOLATILE	((Tcl_FreeProc *) -1)
#define TCL_STATIC	((Tcl_FreeProc *) 0)
#define TCL_DYNAMIC	((Tcl_FreeProc *) free)

/*
 * Flag values passed to variable-related procedures.
 */

#define TCL_GLOBAL_ONLY		1
#define TCL_APPEND_VALUE	2
#define TCL_LIST_ELEMENT	4
#define TCL_NO_SPACE		8
#define TCL_TRACE_READS		0x10
#define TCL_TRACE_WRITES	0x20
#define TCL_TRACE_UNSETS	0x40
#define TCL_TRACE_DESTROYED	0x80
#define TCL_INTERP_DESTROYED	0x100
#define TCL_LEAVE_ERR_MSG	0x200

/*
 * Additional flag passed back to variable watchers.  This flag must
 * not overlap any of the TCL_TRACE_* flags defined above or the
 * TRACE_* flags defined in tclInt.h.
 */

#define TCL_VARIABLE_UNDEFINED	8

/*
 * The following declarations either map ckalloc and ckfree to
 * malloc and free, or they map them to procedures with all sorts
 * of debugging hooks defined in tclCkalloc.c.
 */

#ifdef TCL_MEM_DEBUG

EXTERN char *		Tcl_DbCkalloc (unsigned int size,
			    char *file, int line);
EXTERN int		Tcl_DbCkfree (char *ptr,
			    char *file, int line);
EXTERN char *		Tcl_DbCkrealloc (char *ptr,
			    unsigned int size, char *file, int line);
EXTERN int		Tcl_DumpActiveMemory (char *fileName);
EXTERN void		Tcl_ValidateAllMemory (char *file,
			    int line);
#  define ckalloc(x) Tcl_DbCkalloc(x, __FILE__, __LINE__)
#  define ckfree(x)  Tcl_DbCkfree(x, __FILE__, __LINE__)
#  define ckrealloc(x,y) Tcl_DbCkrealloc((x), (y),__FILE__, __LINE__)

#else

#  define ckalloc(x) malloc(x)
#  define ckfree(x)  free(x)
#  define ckrealloc(x,y) realloc(x,y)
#  define Tcl_DumpActiveMemory(x)
#  define Tcl_ValidateAllMemory(x,y)

#endif /* TCL_MEM_DEBUG */

/*
 * Macro to free up result of interpreter.
 */

#define Tcl_FreeResult(interp)					\
    if ((interp)->freeProc != 0) {				\
	if ((interp)->freeProc == (Tcl_FreeProc *) free) {	\
	    ckfree((interp)->result);				\
	} else {						\
	    (*(interp)->freeProc)((interp)->result);		\
	}							\
	(interp)->freeProc = 0;					\
    }

/*
 * Exported Tcl procedures:
 */

EXTERN void		Tcl_AppendElement (Tcl_Interp *interp,
			    char *string, int noSep);
EXTERN void		Tcl_AppendResult (Tcl_Interp *interp, ...);
EXTERN char *		Tcl_AssembleCmd (Tcl_CmdBuf buffer,
			    char *string);
EXTERN void		Tcl_AddErrorInfo (Tcl_Interp *interp,
			    char *message);
EXTERN char		Tcl_Backslash (char *src,
			    int *readPtr);
EXTERN int		Tcl_CommandComplete(char *cmd);
EXTERN char *		Tcl_Concat (int argc, char **argv);
EXTERN int		Tcl_ConvertElement (char *src,
			    char *dst, int flags);
EXTERN Tcl_CmdBuf	Tcl_CreateCmdBuf (void);
EXTERN void		Tcl_CreateCommand (Tcl_Interp *interp,
			    char *cmdName, Tcl_CmdProc *proc,
			    ClientData clientData,
			    Tcl_CmdDeleteProc *deleteProc);
EXTERN Tcl_Interp *	Tcl_CreateInterp (void);
EXTERN int		Tcl_CreatePipeline (Tcl_Interp *interp,
			    int argc, char **argv, int **pidArrayPtr,
			    int *inPipePtr, int *outPipePtr,
			    int *errFilePtr);
EXTERN Tcl_Trace	Tcl_CreateTrace (Tcl_Interp *interp,
			    int level, Tcl_CmdTraceProc *proc,
			    ClientData clientData);
EXTERN void		Tcl_DeleteCmdBuf (Tcl_CmdBuf buffer);
EXTERN int		Tcl_DeleteCommand (Tcl_Interp *interp,
			    char *cmdName);
EXTERN void		Tcl_DeleteInterp (Tcl_Interp *interp);
EXTERN void		Tcl_DeleteTrace (Tcl_Interp *interp,
			    Tcl_Trace trace);
EXTERN void		Tcl_DetachPids (int numPids, int *pidPtr);
EXTERN char *		Tcl_ErrnoId (void);
EXTERN int		Tcl_Eval (Tcl_Interp *interp, char *cmd,
			    int flags, char **termPtr);
EXTERN int		Tcl_EvalFile (Tcl_Interp *interp,
			    char *fileName);
EXTERN int		Tcl_ExprBoolean (Tcl_Interp *interp,
			    char *string, int *ptr);
EXTERN int		Tcl_ExprDouble (Tcl_Interp *interp,
			    char *string, double *ptr);
EXTERN int		Tcl_ExprLong (Tcl_Interp *interp,
			    char *string, long *ptr);
EXTERN int		Tcl_ExprString (Tcl_Interp *interp,
			    char *string);
EXTERN int		Tcl_Fork (void);
EXTERN int		Tcl_GetBoolean (Tcl_Interp *interp,
			    char *string, int *boolPtr);
EXTERN int		Tcl_GetDouble (Tcl_Interp *interp,
			    char *string, double *doublePtr);
EXTERN int		Tcl_GetInt (Tcl_Interp *interp,
			    char *string, int *intPtr);
EXTERN char *		Tcl_GetVar (Tcl_Interp *interp,
			    char *varName, int flags);
EXTERN char *		Tcl_GetVar2 (Tcl_Interp *interp,
			    char *part1, char *part2, int flags);
EXTERN int		Tcl_GlobalEval (Tcl_Interp *interp,
			    char *command);
EXTERN void		Tcl_InitHistory (Tcl_Interp *interp);
EXTERN void		Tcl_InitMemory (Tcl_Interp *interp);
EXTERN char *		Tcl_Merge (int argc, char **argv);
EXTERN char *		Tcl_ParseVar (Tcl_Interp *interp,
			    char *string, char **termPtr);
EXTERN int		Tcl_RecordAndEval (Tcl_Interp *interp,
			    char *cmd, int flags);
EXTERN void		Tcl_ResetResult (Tcl_Interp *interp);
#define Tcl_Return Tcl_SetResult
EXTERN int		Tcl_ScanElement (char *string,
			    int *flagPtr);
EXTERN void		Tcl_SetErrorCode (Tcl_Interp *interp, ...);
EXTERN void		Tcl_SetResult (Tcl_Interp *interp,
			    char *string, Tcl_FreeProc *freeProc);
EXTERN char *		Tcl_SetVar (Tcl_Interp *interp,
			    char *varName, char *newValue, int flags);
EXTERN char *		Tcl_SetVar2 (Tcl_Interp *interp,
			    char *part1, char *part2, char *newValue,
			    int flags);
EXTERN char *		Tcl_SignalId (int sig);
EXTERN char *		Tcl_SignalMsg (int sig);
EXTERN int		Tcl_SplitList (Tcl_Interp *interp,
			    char *list, int *argcPtr, char ***argvPtr);
EXTERN int		Tcl_StringMatch (char *string,
			    char *pattern);
EXTERN char *		Tcl_TildeSubst (Tcl_Interp *interp,
			    char *name);
EXTERN int		Tcl_TraceVar (Tcl_Interp *interp,
			    char *varName, int flags, Tcl_VarTraceProc *proc,
			    ClientData clientData);
EXTERN int		Tcl_TraceVar2 (Tcl_Interp *interp,
			    char *part1, char *part2, int flags,
			    Tcl_VarTraceProc *proc, ClientData clientData);
EXTERN char *		Tcl_UnixError (Tcl_Interp *interp);
EXTERN int		Tcl_UnsetVar (Tcl_Interp *interp,
			    char *varName, int flags);
EXTERN int		Tcl_UnsetVar2 (Tcl_Interp *interp,
			    char *part1, char *part2, int flags);
EXTERN void		Tcl_UntraceVar (Tcl_Interp *interp,
			    char *varName, int flags, Tcl_VarTraceProc *proc,
			    ClientData clientData);
EXTERN void		Tcl_UntraceVar2 (Tcl_Interp *interp,
			    char *part1, char *part2, int flags,
			    Tcl_VarTraceProc *proc, ClientData clientData);
EXTERN int		Tcl_VarEval (Tcl_Interp *interp, ...);
EXTERN ClientData	Tcl_VarTraceInfo (Tcl_Interp *interp,
			    char *varName, int flags,
			    Tcl_VarTraceProc *procPtr,
			    ClientData prevClientData);
EXTERN ClientData	Tcl_VarTraceInfo2 (Tcl_Interp *interp,
			    char *part1, char *part2, int flags,
			    Tcl_VarTraceProc *procPtr,
			    ClientData prevClientData);
EXTERN int		Tcl_WaitPids (int numPids, int *pidPtr,
			    int *statusPtr);

#endif /* _TCL */
