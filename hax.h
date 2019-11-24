/*
 * hax.h --
 *
 *	This header file describes the externally-visible facilities
 *	of the Hax interpreter. This header file declares the
 *	facilities provided by the Hax hash table procedures.
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
 * $Header: /sprite/src/lib/tcl/RCS/tclHash.h,v 1.3 91/08/27 11:36:04 ouster Exp $ SPRITE (Berkeley)
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
#   define HAX_EXTERN extern "C"
#else
#   define HAX_EXTERN extern
#endif

/*
 * Miscellaneous declarations (to allow Hax to be used stand-alone,
 * without the rest of Sprite).
 */

#ifndef _CLIENTDATA
typedef void *ClientData;
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

typedef void *Hax_Trace;
typedef void *Hax_CmdBuf;
typedef void *Hax_Memoryp;

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

typedef void (Hax_CmdDeleteProc) (Hax_Interp *interp, ClientData clientData);
typedef int (Hax_CmdProc) (ClientData clientData,
	Hax_Interp *interp, int argc, char *argv[]);
typedef void (Hax_CmdTraceProc) (ClientData clientData,
	Hax_Interp *interp, int level, char *command, Hax_CmdProc *proc,
	ClientData cmdClientData, int argc, char *argv[]);
typedef void (Hax_FreeProc) (char *blockPtr);
typedef char *(Hax_VarTraceProc) (ClientData clientData,
	Hax_Interp *interp, char *part1, char *part2, int flags);
typedef void (Hax_EnvWriteProc) (Hax_Interp *interp, ClientData clientData,
	char *name, char *value);
typedef void (Hax_EnvUnsetProc) (Hax_Interp *interp, ClientData clientData,
	char *name);
typedef void (Hax_EnvDestroyProc) (Hax_Interp *interp, ClientData clientData);

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
 * The following declarations map ckalloc and ckfree to procedures
 * with all sorts of debugging hooks defined in haxCkalloc.c.
 */

HAX_EXTERN void *	Hax_DbCkalloc (Hax_Memoryp *, unsigned long int size,
			    char *file, int line);
HAX_EXTERN int		Hax_DbCkfree (Hax_Memoryp *, void *ptr,
			    char *file, int line);
HAX_EXTERN void *	Hax_DbCkrealloc (Hax_Memoryp *, void *ptr,
			    unsigned long int size, char *file, int line);
HAX_EXTERN int		Hax_DumpActiveMemory (Hax_Memoryp *);
HAX_EXTERN void		Hax_ValidateAllMemory (Hax_Memoryp *, char *file,
			    int line);

#define ckalloc(m,x) Hax_DbCkalloc((m), (x), (char *) __FILE__, __LINE__)
#define ckfree(m,x)  Hax_DbCkfree((m), (x), (char *) __FILE__, __LINE__)
#define ckrealloc(m,x,y)					\
	Hax_DbCkrealloc((m), (x), (y), (char *) __FILE__, __LINE__)
#define ckvalidateallmemory(m)					\
	Hax_ValidateAllMemory((m), (char *) __FILE__, __LINE__)


/*
 * Macro to free up result of interpreter.
 */

#define Hax_FreeResult(interp)					\
    do {							\
	if ((interp)->freeProc != 0) {				\
	    if ((interp)->freeProc == (Hax_FreeProc *) free) {	\
		ckfree(Hax_GetMemoryp(interp),			\
		    (interp)->result);				\
	    } else {						\
		(*(interp)->freeProc)((interp)->result);	\
	    }							\
	    (interp)->freeProc = 0;				\
	}							\
    } while (0)

/*
 * Exported Hax procedures:
 */

HAX_EXTERN void		Hax_AppendElement (Hax_Interp *interp,
			    char *string, int noSep);
HAX_EXTERN void		Hax_AppendResult (Hax_Interp *interp, ...);
HAX_EXTERN char *	Hax_AssembleCmd (Hax_Interp *interp,
			    Hax_CmdBuf buffer, char *string);
HAX_EXTERN void		Hax_AddErrorInfo (Hax_Interp *interp,
			    char *message);
HAX_EXTERN char		Hax_Backslash (char *src,
			    int *readPtr);
HAX_EXTERN int		Hax_CommandComplete(char *cmd);
HAX_EXTERN char *	Hax_Concat (Hax_Interp *interp, int argc, char **argv);
HAX_EXTERN int		Hax_ConvertElement (char *src,
			    char *dst, int flags);
HAX_EXTERN Hax_CmdBuf	Hax_CreateCmdBuf (Hax_Interp *interp);
HAX_EXTERN void		Hax_CreateCommand (Hax_Interp *interp,
			    char *cmdName, Hax_CmdProc *proc,
			    ClientData clientData,
			    Hax_CmdDeleteProc *deleteProc);
HAX_EXTERN Hax_Memoryp *Hax_CreateMemoryManagement (
			    int break_on_malloc, int trace_on_at_malloc,
			    int alloc_tracing, int init_malloced_bodies,
			    int validate_memory);
HAX_EXTERN Hax_Memoryp *Hax_GetMemoryp (Hax_Interp *interp);
HAX_EXTERN void		Hax_SetLibraryPath (Hax_Interp *interp, char *path);
HAX_EXTERN Hax_Interp *	Hax_CreateInterp (Hax_Memoryp *memoryp);
HAX_EXTERN int		Hax_CreatePipeline (Hax_Interp *interp,
			    ClientData clientData,
			    int argc, char **argv, int **pidArrayPtr,
			    int *inPipePtr, int *outPipePtr,
			    int *errFilePtr);
HAX_EXTERN Hax_Trace	Hax_CreateTrace (Hax_Interp *interp,
			    int level, Hax_CmdTraceProc *proc,
			    ClientData clientData);
HAX_EXTERN void		Hax_DeleteCmdBuf (Hax_Interp *interp,
			    Hax_CmdBuf buffer);
HAX_EXTERN int		Hax_DeleteCommand (Hax_Interp *interp,
			    char *cmdName);
HAX_EXTERN void		Hax_DeleteInterp (Hax_Interp *interp);
HAX_EXTERN void		Hax_DeleteTrace (Hax_Interp *interp,
			    Hax_Trace trace);
HAX_EXTERN void		Hax_DetachPids (ClientData clientData,
			    int numPids, int *pidPtr);
HAX_EXTERN char *	Hax_ErrnoId (void);
HAX_EXTERN int		Hax_Eval (Hax_Interp *interp, char *scriptFile,
			    char *cmd, int flags, char **termPtr);
HAX_EXTERN int		Hax_EvalFile (Hax_Interp *interp,
			    ClientData clientData, char *fileName);
HAX_EXTERN int		Hax_ExprBoolean (Hax_Interp *interp,
			    char *string, int *ptr);
HAX_EXTERN int		Hax_ExprDouble (Hax_Interp *interp,
			    char *string, void *ptr);
HAX_EXTERN int		Hax_ExprLong (Hax_Interp *interp,
			    char *string, long int *ptr);
HAX_EXTERN int		Hax_ExprLongLong (Hax_Interp *interp,
			    char *string, long long int *ptr);
HAX_EXTERN int		Hax_ExprString (Hax_Interp *interp,
			    char *string);
HAX_EXTERN int		Hax_Fork (Hax_Interp *interp,
			    ClientData clientData);
HAX_EXTERN int		Hax_GetBoolean (Hax_Interp *interp,
			    char *string, int *boolPtr);
HAX_EXTERN int		Hax_GetDouble (Hax_Interp *interp,
			    char *string, void *doublePtr);
HAX_EXTERN int		Hax_GetInt (Hax_Interp *interp,
			    char *string, int *intPtr);
HAX_EXTERN int		Hax_GetLong (Hax_Interp *interp,
			    char *string, long int *longPtr);
HAX_EXTERN int		Hax_GetLongLong (Hax_Interp *interp,
			    char *string, long long int *llongPtr);
HAX_EXTERN char *	Hax_GetVar (Hax_Interp *interp,
			    char *varName, int flags);
HAX_EXTERN char *	Hax_GetVar2 (Hax_Interp *interp,
			    char *part1, char *part2, int flags);
HAX_EXTERN int		Hax_GlobalEval (Hax_Interp *interp,
			    char *command);
HAX_EXTERN void		Hax_InitHistory (Hax_Interp *interp);
HAX_EXTERN void		Hax_InitMemory (Hax_Interp *interp);
HAX_EXTERN char *	Hax_Merge (Hax_Interp *interp, int argc, char **argv);
HAX_EXTERN char *	Hax_ParseVar (Hax_Interp *interp,
			    char *string, char **termPtr);
HAX_EXTERN int		Hax_RecordAndEval (Hax_Interp *interp,
			    char *cmd, int flags);
HAX_EXTERN void		Hax_ResetResult (Hax_Interp *interp);
#define Hax_Return Hax_SetResult
HAX_EXTERN int		Hax_ScanElement (char *string,
			    int *flagPtr);
HAX_EXTERN void		Hax_SetErrorCode (Hax_Interp *interp, ...);
HAX_EXTERN void		Hax_SetResult (Hax_Interp *interp,
			    char *string, Hax_FreeProc *freeProc);
HAX_EXTERN char *	Hax_SetVar (Hax_Interp *interp,
			    char *varName, char *newValue, int flags);
HAX_EXTERN char *	Hax_SetVar2 (Hax_Interp *interp,
			    char *part1, char *part2, char *newValue,
			    int flags);
HAX_EXTERN char *	Hax_SignalId (int sig);
HAX_EXTERN char *	Hax_SignalMsg (int sig);
HAX_EXTERN int		Hax_SplitList (Hax_Interp *interp,
			    char *list, int *argcPtr, char ***argvPtr);
HAX_EXTERN int		Hax_StringMatch (char *string,
			    char *pattern);
HAX_EXTERN char *	Hax_TildeSubst (Hax_Interp *interp,
			    ClientData clientData, char *name);
HAX_EXTERN int		Hax_TraceVar (Hax_Interp *interp,
			    char *varName, int flags, Hax_VarTraceProc *proc,
			    ClientData clientData);
HAX_EXTERN int		Hax_TraceVar2 (Hax_Interp *interp,
			    char *part1, char *part2, int flags,
			    Hax_VarTraceProc *proc, ClientData clientData);
HAX_EXTERN char *	Hax_UnixError (Hax_Interp *interp);
HAX_EXTERN int		Hax_UnsetVar (Hax_Interp *interp,
			    char *varName, int flags);
HAX_EXTERN int		Hax_UnsetVar2 (Hax_Interp *interp,
			    char *part1, char *part2, int flags);
HAX_EXTERN void		Hax_UntraceVar (Hax_Interp *interp,
			    char *varName, int flags, Hax_VarTraceProc *proc,
			    ClientData clientData);
HAX_EXTERN void		Hax_UntraceVar2 (Hax_Interp *interp,
			    char *part1, char *part2, int flags,
			    Hax_VarTraceProc *proc, ClientData clientData);
HAX_EXTERN int		Hax_VarEval (Hax_Interp *interp, ...);
HAX_EXTERN ClientData	Hax_VarTraceInfo (Hax_Interp *interp,
			    char *varName, int flags,
			    Hax_VarTraceProc *procPtr,
			    ClientData prevClientData);
HAX_EXTERN ClientData	Hax_VarTraceInfo2 (Hax_Interp *interp,
			    char *part1, char *part2, int flags,
			    Hax_VarTraceProc *procPtr,
			    ClientData prevClientData);
HAX_EXTERN int		Hax_WaitPids (ClientData clientData,
			    int numPids, int *pidPtr,
			    int *statusPtr);
HAX_EXTERN ClientData	Hax_InitUnixCore (Hax_Interp *interp);

/*
 * Miscelaous functions that can be overriden in the implementation.
 */

HAX_EXTERN void			Hax_Panic (char *format, ...);
HAX_EXTERN void			Hax_Breakpoint (void);

/*
 * Structure definition for an entry in a hash table.  No-one outside
 * Hax should access any of these fields directly;  use the macros
 * defined below.
 */

typedef struct Hax_HashEntry {
    struct Hax_HashEntry *nextPtr;	/* Pointer to next entry in this
					 * hash bucket, or NULL for end of
					 * chain. */
    struct Hax_HashTable *tablePtr;	/* Pointer to table containing entry. */
    struct Hax_HashEntry **bucketPtr;	/* Pointer to bucket that points to
					 * first entry in this entry's chain:
					 * used for deleting the entry. */
    ClientData clientData;		/* Application stores something here
					 * with Hax_SetHashValue. */
    union {				/* Key has one of these forms: */
	char *oneWordValue;		/* One-word value for key. */
	int words[1];			/* Multiple integer words for key.
					 * The actual size will be as large
					 * as necessary for this table's
					 * keys. */
	char string[4];			/* String for key.  The actual size
					 * will be as large as needed to hold
					 * the key. */
    } key;				/* MUST BE LAST FIELD IN RECORD!! */
} Hax_HashEntry;

/*
 * Structure definition for a hash table.  Must be in hax.h so clients
 * can allocate space for these structures, but clients should never
 * access any fields in this structure.
 */

#define HAX_SMALL_HASH_TABLE 4
typedef struct Hax_HashTable {
    Hax_HashEntry **buckets;		/* Pointer to bucket array.  Each
					 * element points to first entry in
					 * bucket's hash chain, or NULL. */
    Hax_HashEntry *staticBuckets[HAX_SMALL_HASH_TABLE];
					/* Bucket array used for small tables
					 * (to avoid mallocs and frees). */
    int numBuckets;			/* Total number of buckets allocated
					 * at **bucketPtr. */
    int numEntries;			/* Total number of entries present
					 * in table. */
    int rebuildSize;			/* Enlarge table when numEntries gets
					 * to be this large. */
    int downShift;			/* Shift count used in hashing
					 * function.  Designed to use high-
					 * order bits of randomized keys. */
    int mask;				/* Mask value used in hashing
					 * function. */
    int keyType;			/* Type of keys used in this table.
					 * It's either HAX_STRING_KEYS,
					 * HAX_ONE_WORD_KEYS, or an integer
					 * giving the number of ints in a
					 */
    Hax_HashEntry *(*findProc) (struct Hax_HashTable *tablePtr,
	    char *key);
    Hax_HashEntry *(*createProc) (Hax_Interp *interp,
	    struct Hax_HashTable *tablePtr, char *key, int *newPtr);
} Hax_HashTable;

/*
 * Structure definition for information used to keep track of searches
 * through hash tables:
 */

typedef struct Hax_HashSearch {
    Hax_HashTable *tablePtr;		/* Table being searched. */
    int nextIndex;			/* Index of next bucket to be
					 * enumerated after present one. */
    Hax_HashEntry *nextEntryPtr;	/* Next entry to be enumerated in the
					 * the current bucket. */
} Hax_HashSearch;

/*
 * Acceptable key types for hash tables:
 */

#define HAX_STRING_KEYS		0
#define HAX_ONE_WORD_KEYS	1

/*
 * Macros for clients to use to access fields of hash entries:
 */

#define Hax_GetHashValue(h) ((h)->clientData)
#define Hax_SetHashValue(h, value) ((h)->clientData = (ClientData) (value))
#define Hax_GetHashKey(tablePtr, h) \
    ((char *) (((tablePtr)->keyType == HAX_ONE_WORD_KEYS) ? (h)->key.oneWordValue \
						: (h)->key.string))

/*
 * Macros to use for clients to use to invoke find and create procedures
 * for hash tables:
 */

#define Hax_FindHashEntry(tablePtr, key) \
	(*((tablePtr)->findProc))(tablePtr, key)
#define Hax_CreateHashEntry(interp, tablePtr, key, newPtr) \
	(*((tablePtr)->createProc))(interp, tablePtr, key, newPtr)

/*
 * Exported procedures:
 */

HAX_EXTERN void			Hax_DeleteHashEntry (Hax_Interp *interp,
				    Hax_HashEntry *entryPtr);
HAX_EXTERN void			Hax_DeleteHashTable (Hax_Interp *interp,
				    Hax_HashTable *tablePtr);
HAX_EXTERN Hax_HashEntry *	Hax_FirstHashEntry (
				    Hax_HashTable *tablePtr,
				    Hax_HashSearch *searchPtr);
HAX_EXTERN char *		Hax_HashStats (Hax_Interp *interp,
				    Hax_HashTable *tablePtr);
HAX_EXTERN void			Hax_InitHashTable (Hax_HashTable *tablePtr,
				    int keyType);
HAX_EXTERN Hax_HashEntry *	Hax_NextHashEntry (
				    Hax_HashSearch *searchPtr);
/*
 * Exported procedures for the management of environment variables.
 */
HAX_EXTERN int			Hax_FindVariable(
				    ClientData clientData,
				    char *name, int *lengthPtr);
HAX_EXTERN int			Hax_SetEnv(Hax_Interp *interp,
				    ClientData clientData,
				    char *name, char *value, int overwrite);
HAX_EXTERN int			Hax_PutEnv(Hax_Interp *interp,
				    ClientData clientData,
				    char *string);
HAX_EXTERN int			Hax_UnsetEnv(Hax_Interp *interp,
				    ClientData clientData,
				    char *name);
HAX_EXTERN void			Hax_EnvTraceProc(Hax_Interp *interp,
				    ClientData clientData,
				    Hax_EnvWriteProc *writeProc,
				    Hax_EnvUnsetProc *unsetProc,
				    Hax_EnvDestroyProc *destroyProc);

#endif /* _HAX */
