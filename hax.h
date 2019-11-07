/*
 * hax.h --
 *
 *	This header file describes the externally-visible facilities
 *	of the Hax interpreter.
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

#ifndef _HAX
#define _HAX

#define HAX_VERSION "6.7"
#define HAX_MAJOR_VERSION 6
#define HAX_MINOR_VERSION 7

/*
 * Definitions that allow this header to be used with C++.
 */

#ifdef __cplusplus
#   define EXTERN extern "C"
#else
#   define EXTERN extern
#endif

/*
 * Miscellaneous declarations (to allow Hax to be used stand-alone,
 * without the rest of Sprite).
 */

#ifndef _CLIENTDATA
typedef int *ClientData;
#define _CLIENTDATA
#endif

/*
 * Data structures defined opaquely in this module.  The definitions
 * below just provide dummy types.  A few fields are made visible in
 * Hax_Interp structures, namely those for returning string values.
 * Note:  any change to the Hax_Interp definition below must be mirrored
 * in the "real" definition in haxInt.h.
 */

typedef struct Hax_Interp{
    char *result;		/* Points to result string returned by last
				 * command. */
    void (*freeProc) (char *blockPtr);
				/* Zero means result is statically allocated.
				 * If non-zero, gives address of procedure
				 * to invoke to free the result.  Must be
				 * freed by Hax_Eval before executing next
				 * command. */
    int errorLine;		/* When HAX_ERROR is returned, this gives
				 * the line number within the command where
				 * the error occurred (1 means first line). */
} Hax_Interp;

typedef int *Hax_Trace;
typedef int *Hax_CmdBuf;

/*
 * When a HAX command returns, the string pointer interp->result points to
 * a string containing return information from the command.  In addition,
 * the command procedure returns an integer value, which is one of the
 * following:
 *
 * HAX_OK		Command completed normally;  interp->result contains
 *			the command's result.
 * HAX_ERROR		The command couldn't be completed successfully;
 *			interp->result describes what went wrong.
 * HAX_RETURN		The command requests that the current procedure
 *			return;  interp->result contains the procedure's
 *			return value.
 * HAX_BREAK		The command requests that the innermost loop
 *			be exited;  interp->result is meaningless.
 * HAX_CONTINUE		Go on to the next iteration of the current loop;
 *			interp->result is meaninless.
 */

#define HAX_OK		0
#define HAX_ERROR	1
#define HAX_RETURN	2
#define HAX_BREAK	3
#define HAX_CONTINUE	4

#define HAX_RESULT_SIZE 199

/*
 * Procedure types defined by Hax:
 */

typedef void (Hax_CmdDeleteProc) (ClientData clientData);
typedef int (Hax_CmdProc) (ClientData clientData,
	Hax_Interp *interp, int argc, char *argv[]);
typedef void (Hax_CmdTraceProc) (ClientData clientData,
	Hax_Interp *interp, int level, char *command, Hax_CmdProc *proc,
	ClientData cmdClientData, int argc, char *argv[]);
typedef void (Hax_FreeProc) (char *blockPtr);
typedef char *(Hax_VarTraceProc) (ClientData clientData,
	Hax_Interp *interp, char *part1, char *part2, int flags);

/*
 * Flag values passed to Hax_Eval (see the man page for details;  also
 * see haxInt.h for additional flags that are only used internally by
 * Hax):
 */

#define HAX_BRACKET_TERM	1

/*
 * Flag that may be passed to Hax_ConvertElement to force it not to
 * output braces (careful!  if you change this flag be sure to change
 * the definitions at the front of haxUtil.c).
 */

#define HAX_DONT_USE_BRACES	1

/*
 * Flag value passed to Hax_RecordAndEval to request no evaluation
 * (record only).
 */

#define HAX_NO_EVAL		-1

/*
 * Specil freeProc values that may be passed to Hax_SetResult (see
 * the man page for details):
 */

#define HAX_VOLATILE	((Hax_FreeProc *) -1)
#define HAX_STATIC	((Hax_FreeProc *) 0)
#define HAX_DYNAMIC	((Hax_FreeProc *) free)

/*
 * Flag values passed to variable-related procedures.
 */

#define HAX_GLOBAL_ONLY		1
#define HAX_APPEND_VALUE	2
#define HAX_LIST_ELEMENT	4
#define HAX_NO_SPACE		8
#define HAX_TRACE_READS		0x10
#define HAX_TRACE_WRITES	0x20
#define HAX_TRACE_UNSETS	0x40
#define HAX_TRACE_DESTROYED	0x80
#define HAX_INTERP_DESTROYED	0x100
#define HAX_LEAVE_ERR_MSG	0x200

/*
 * Additional flag passed back to variable watchers.  This flag must
 * not overlap any of the HAX_TRACE_* flags defined above or the
 * TRACE_* flags defined in haxInt.h.
 */

#define HAX_VARIABLE_UNDEFINED	8

/*
 * The following declarations either map ckalloc and ckfree to
 * malloc and free, or they map them to procedures with all sorts
 * of debugging hooks defined in haxCkalloc.c.
 */

#ifdef HAX_MEM_DEBUG

EXTERN char *		Hax_DbCkalloc (unsigned int size,
			    char *file, int line);
EXTERN int		Hax_DbCkfree (char *ptr,
			    char *file, int line);
EXTERN char *		Hax_DbCkrealloc (char *ptr,
			    unsigned int size, char *file, int line);
EXTERN int		Hax_DumpActiveMemory (char *fileName);
EXTERN void		Hax_ValidateAllMemory (char *file,
			    int line);
#  define ckalloc(x) Hax_DbCkalloc(x, __FILE__, __LINE__)
#  define ckfree(x)  Hax_DbCkfree(x, __FILE__, __LINE__)
#  define ckrealloc(x,y) Hax_DbCkrealloc((x), (y),__FILE__, __LINE__)

#else

#  define ckalloc(x) malloc(x)
#  define ckfree(x)  free(x)
#  define ckrealloc(x,y) realloc(x,y)
#  define Hax_DumpActiveMemory(x)
#  define Hax_ValidateAllMemory(x,y)

#endif /* HAX_MEM_DEBUG */

/*
 * Macro to free up result of interpreter.
 */

#define Hax_FreeResult(interp)					\
    do {							\
	if ((interp)->freeProc != 0) {				\
	    if ((interp)->freeProc == (Hax_FreeProc *) free) {	\
		ckfree((interp)->result);			\
	    } else {						\
		(*(interp)->freeProc)((interp)->result);	\
	    }							\
	    (interp)->freeProc = 0;				\
	}							\
    } while (0)

/*
 * Exported Hax procedures:
 */

EXTERN void		Hax_AppendElement (Hax_Interp *interp,
			    char *string, int noSep);
EXTERN void		Hax_AppendResult (Hax_Interp *interp, ...);
EXTERN char *		Hax_AssembleCmd (Hax_CmdBuf buffer,
			    char *string);
EXTERN void		Hax_AddErrorInfo (Hax_Interp *interp,
			    char *message);
EXTERN char		Hax_Backslash (char *src,
			    int *readPtr);
EXTERN int		Hax_CommandComplete(char *cmd);
EXTERN char *		Hax_Concat (int argc, char **argv);
EXTERN int		Hax_ConvertElement (char *src,
			    char *dst, int flags);
EXTERN Hax_CmdBuf	Hax_CreateCmdBuf (void);
EXTERN void		Hax_CreateCommand (Hax_Interp *interp,
			    char *cmdName, Hax_CmdProc *proc,
			    ClientData clientData,
			    Hax_CmdDeleteProc *deleteProc);
EXTERN Hax_Interp *	Hax_CreateInterp (void);
EXTERN int		Hax_CreatePipeline (Hax_Interp *interp,
			    int argc, char **argv, int **pidArrayPtr,
			    int *inPipePtr, int *outPipePtr,
			    int *errFilePtr);
EXTERN Hax_Trace	Hax_CreateTrace (Hax_Interp *interp,
			    int level, Hax_CmdTraceProc *proc,
			    ClientData clientData);
EXTERN void		Hax_DeleteCmdBuf (Hax_CmdBuf buffer);
EXTERN int		Hax_DeleteCommand (Hax_Interp *interp,
			    char *cmdName);
EXTERN void		Hax_DeleteInterp (Hax_Interp *interp);
EXTERN void		Hax_DeleteTrace (Hax_Interp *interp,
			    Hax_Trace trace);
EXTERN void		Hax_DetachPids (int numPids, int *pidPtr);
EXTERN char *		Hax_ErrnoId (void);
EXTERN int		Hax_Eval (Hax_Interp *interp, char *cmd,
			    int flags, char **termPtr);
EXTERN int		Hax_EvalFile (Hax_Interp *interp,
			    char *fileName);
EXTERN int		Hax_ExprBoolean (Hax_Interp *interp,
			    char *string, int *ptr);
EXTERN int		Hax_ExprDouble (Hax_Interp *interp,
			    char *string, void *ptr);
EXTERN int		Hax_ExprLong (Hax_Interp *interp,
			    char *string, long *ptr);
EXTERN int		Hax_ExprString (Hax_Interp *interp,
			    char *string);
EXTERN int		Hax_Fork (void);
EXTERN int		Hax_GetBoolean (Hax_Interp *interp,
			    char *string, int *boolPtr);
EXTERN int		Hax_GetDouble (Hax_Interp *interp,
			    char *string, void *doublePtr);
EXTERN int		Hax_GetInt (Hax_Interp *interp,
			    char *string, int *intPtr);
EXTERN char *		Hax_GetVar (Hax_Interp *interp,
			    char *varName, int flags);
EXTERN char *		Hax_GetVar2 (Hax_Interp *interp,
			    char *part1, char *part2, int flags);
EXTERN int		Hax_GlobalEval (Hax_Interp *interp,
			    char *command);
EXTERN void		Hax_InitHistory (Hax_Interp *interp);
EXTERN void		Hax_InitMemory (Hax_Interp *interp);
EXTERN char *		Hax_Merge (int argc, char **argv);
EXTERN char *		Hax_ParseVar (Hax_Interp *interp,
			    char *string, char **termPtr);
EXTERN int		Hax_RecordAndEval (Hax_Interp *interp,
			    char *cmd, int flags);
EXTERN void		Hax_ResetResult (Hax_Interp *interp);
#define Hax_Return Hax_SetResult
EXTERN int		Hax_ScanElement (char *string,
			    int *flagPtr);
EXTERN void		Hax_SetErrorCode (Hax_Interp *interp, ...);
EXTERN void		Hax_SetResult (Hax_Interp *interp,
			    char *string, Hax_FreeProc *freeProc);
EXTERN char *		Hax_SetVar (Hax_Interp *interp,
			    char *varName, char *newValue, int flags);
EXTERN char *		Hax_SetVar2 (Hax_Interp *interp,
			    char *part1, char *part2, char *newValue,
			    int flags);
EXTERN char *		Hax_SignalId (int sig);
EXTERN char *		Hax_SignalMsg (int sig);
EXTERN int		Hax_SplitList (Hax_Interp *interp,
			    char *list, int *argcPtr, char ***argvPtr);
EXTERN int		Hax_StringMatch (char *string,
			    char *pattern);
EXTERN char *		Hax_TildeSubst (Hax_Interp *interp,
			    char *name);
EXTERN int		Hax_TraceVar (Hax_Interp *interp,
			    char *varName, int flags, Hax_VarTraceProc *proc,
			    ClientData clientData);
EXTERN int		Hax_TraceVar2 (Hax_Interp *interp,
			    char *part1, char *part2, int flags,
			    Hax_VarTraceProc *proc, ClientData clientData);
EXTERN char *		Hax_UnixError (Hax_Interp *interp);
EXTERN int		Hax_UnsetVar (Hax_Interp *interp,
			    char *varName, int flags);
EXTERN int		Hax_UnsetVar2 (Hax_Interp *interp,
			    char *part1, char *part2, int flags);
EXTERN void		Hax_UntraceVar (Hax_Interp *interp,
			    char *varName, int flags, Hax_VarTraceProc *proc,
			    ClientData clientData);
EXTERN void		Hax_UntraceVar2 (Hax_Interp *interp,
			    char *part1, char *part2, int flags,
			    Hax_VarTraceProc *proc, ClientData clientData);
EXTERN int		Hax_VarEval (Hax_Interp *interp, ...);
EXTERN ClientData	Hax_VarTraceInfo (Hax_Interp *interp,
			    char *varName, int flags,
			    Hax_VarTraceProc *procPtr,
			    ClientData prevClientData);
EXTERN ClientData	Hax_VarTraceInfo2 (Hax_Interp *interp,
			    char *part1, char *part2, int flags,
			    Hax_VarTraceProc *procPtr,
			    ClientData prevClientData);
EXTERN int		Hax_WaitPids (int numPids, int *pidPtr,
			    int *statusPtr);

#endif /* _HAX */
