/*
 * haxCkalloc.c --
 *    Interface to malloc and free that provides support for debugging problems
 *    involving overwritten, double freeing memory and loss of memory.
 *
 * Copyright 1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * This code contributed by Karl Lehenbauer and Mark Diekhans
 *
 */

#include "haxInt.h"

#define FALSE	0
#define TRUE	1

#define GUARD_SIZE 8

struct mem_header {
        unsigned long int  length;
        char              *file;
        int                line;
        struct mem_header *flink;
        struct mem_header *blink;
        unsigned char      low_guard[GUARD_SIZE];
        char               body[1];
};

typedef struct Memoryp {
        struct mem_header *allocHead; /* List of allocated structures */
        long long int      total_mallocs;
        long long int      total_frees;
        unsigned long int  current_bytes_malloced;
        unsigned long int  maximum_bytes_malloced;
        unsigned long int  current_malloc_packets;
        unsigned long int  maximum_malloc_packets;
        int                break_on_malloc;
        int                trace_on_at_malloc;
        int                alloc_tracing;
        int                init_malloced_bodies;
        int                validate_memory;
} Memoryp;

#define GUARD_VALUE  0341

/* static char high_guard[] = {0x89, 0xab, 0xcd, 0xef}; */


/*
 *----------------------------------------------------------------------
 *
 * dump_memory_info --
 *     Display the global memory management statistics.
 *
 *----------------------------------------------------------------------
 */
static void
dump_memory_info(
    Memoryp *memoryp,
    FILE *outFile)
{
        fprintf(outFile,"total mallocs             %10lld\n",
                memoryp->total_mallocs);
        fprintf(outFile,"total frees               %10lld\n",
                memoryp->total_frees);
        fprintf(outFile,"current packets allocated %10lu\n",
                memoryp->current_malloc_packets);
        fprintf(outFile,"current bytes allocated   %10lu\n",
                memoryp->current_bytes_malloced);
        fprintf(outFile,"maximum packets allocated %10lu\n",
                memoryp->maximum_malloc_packets);
        fprintf(outFile,"maximum bytes allocated   %10lu\n",
                memoryp->maximum_bytes_malloced);
}

/*
 *----------------------------------------------------------------------
 *
 * ValidateMemory --
 *     Procedure to validate allocted memory guard zones.
 *
 *----------------------------------------------------------------------
 */
static void
ValidateMemory (
    Memoryp           *memoryp,
    struct mem_header *memHeaderP,
    char              *file,
    int                line,
    int                nukeGuards)
{
    unsigned char *hiPtr;
    int   idx;
    int   guard_failed = FALSE;
    int byte;

    for (idx = 0; idx < GUARD_SIZE; idx++) {
        byte = *(memHeaderP->low_guard + idx);
        if (byte != GUARD_VALUE) {
            guard_failed = TRUE;
            fflush (stdout);
	    byte &= 0xff;
            fprintf(stderr, "low guard byte %d is 0x%x  \t%c\n", idx, byte,
	    	    (isprint(byte) ? byte : ' '));
        }
    }
    if (guard_failed) {
        dump_memory_info (memoryp, stderr);
        fprintf (stderr, "low guard failed at %p, %s %d\n",
                 memHeaderP->body, file, line);
        fflush (stderr);  /* In case name pointer is bad. */
        fprintf (stderr, "%lu bytes allocated at (%s %d)\n", memHeaderP->length,
		memHeaderP->file, memHeaderP->line);
        Hax_Panic ((char *) "Memory validation failure");
    }

    hiPtr = (unsigned char *)memHeaderP->body + memHeaderP->length;
    for (idx = 0; idx < GUARD_SIZE; idx++) {
        byte = *(hiPtr + idx);
        if (byte != GUARD_VALUE) {
            guard_failed = TRUE;
            fflush (stdout);
	    byte &= 0xff;
            fprintf(stderr, "hi guard byte %d is 0x%x  \t%c\n", idx, byte,
	    	    (isprint(byte) ? byte : ' '));
        }
    }

    if (guard_failed) {
        dump_memory_info (memoryp, stderr);
        fprintf (stderr, "high guard failed at %p, %s %d\n",
                 memHeaderP->body, file, line);
        fflush (stderr);  /* In case name pointer is bad. */
        fprintf (stderr, "%lu bytes allocated at (%s %d)\n", memHeaderP->length,
		memHeaderP->file, memHeaderP->line);
        Hax_Panic ((char *) "Memory validation failure");
    }

    if (nukeGuards) {
        memset ((char *) memHeaderP->low_guard, 0, GUARD_SIZE);
        memset ((char *) hiPtr, 0, GUARD_SIZE);
    }

}

/*
 *----------------------------------------------------------------------
 *
 * Hax_ValidateAllMemory --
 *     Validates guard regions for all allocated memory.
 *
 *----------------------------------------------------------------------
 */
void
Hax_ValidateAllMemory (
    Hax_Memoryp *memoryp,
    char  *file,
    int    line)
{
    Memoryp *memCtx = (Memoryp *) memoryp;
    struct mem_header *memScanP;

    for (memScanP = memCtx->allocHead; memScanP != NULL;
	 memScanP = memScanP->flink)
        ValidateMemory (memCtx, memScanP, file, line, FALSE);

}

/*
 *----------------------------------------------------------------------
 *
 * Hax_DumpActiveMemory --
 *     Displays all allocated memory to stdout.
 *
 * Results:
 *     Return HAX_ERROR if an error accessing the file occures, `errno'
 *     will have the file error number left in it.
 *----------------------------------------------------------------------
 */
int
Hax_DumpActiveMemory (
    Hax_Memoryp *memoryp)
{
    Memoryp           *memCtx = (Memoryp *) memoryp;
    struct mem_header *memScanP;
    char              *address;

    for (memScanP = memCtx->allocHead; memScanP != NULL;
	 memScanP = memScanP->flink) {
        address = &memScanP->body [0];
        printf ("%p - %p  %7ld @ %s %d", address,
                 address + memScanP->length - 1, memScanP->length,
                 memScanP->file, memScanP->line);
        if (strcmp(memScanP->file, "haxHash.c") == 0 && memScanP->line == 514){
	    printf("\t|%s|", ((Hax_HashEntry *) address)->key.string);
	}
	(void) putchar('\n');
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_DbCkalloc - debugging ckalloc
 *
 *        Allocate the requested amount of space plus some extra for
 *        guard bands at both ends of the request, plus a size, panicing
 *        if there isn't enough space, then write in the guard bands
 *        and return the address of the space in the middle that the
 *        user asked for.
 *
 *        The second and third arguments are file and line, these contain
 *        the filename and line number corresponding to the caller.
 *        These are sent by the ckalloc macro; it uses the preprocessor
 *        autodefines __FILE__ and __LINE__.
 *
 *----------------------------------------------------------------------
 */
void *
Hax_DbCkalloc(
    Hax_Memoryp *memoryp,
    unsigned long int size,
    char        *file,
    int          line)
{
    Memoryp           *memCtx = (Memoryp *) memoryp;
    struct mem_header *result;

    if (memCtx->validate_memory)
        Hax_ValidateAllMemory (memoryp, file, line);

    result = (struct mem_header *)malloc(size +
                              sizeof(struct mem_header) + GUARD_SIZE);
    if (result == NULL) {
        fflush(stdout);
        dump_memory_info(memCtx, stderr);
        Hax_Panic((char *) "unable to alloc %lu bytes, %s line %d", size, file,
              line);
    }

    /*
     * Fill in guard zones and size.  Link into allocated list.
     */
    result->length = size;
    result->file = file;
    result->line = line;
    memset ((char *) result->low_guard, GUARD_VALUE, GUARD_SIZE);
    memset (result->body + size, GUARD_VALUE, GUARD_SIZE);
    result->flink = memCtx->allocHead;
    result->blink = NULL;
    if (memCtx->allocHead != NULL)
        memCtx->allocHead->blink = result;
    memCtx->allocHead = result;

    memCtx->total_mallocs++;
    if (memCtx->trace_on_at_malloc &&
	(memCtx->total_mallocs >= memCtx->trace_on_at_malloc)) {
        (void) fflush(stdout);
        fprintf(stderr, "reached malloc trace enable point (%lld)\n",
                memCtx->total_mallocs);
        fflush(stderr);
        memCtx->alloc_tracing = TRUE;
        memCtx->trace_on_at_malloc = 0;
    }

    if (memCtx->alloc_tracing)
        fprintf(stderr,"ckalloc %p %lu %s %d\n", result->body, size,
                file, line);

    if (memCtx->break_on_malloc &&
	(memCtx->total_mallocs >= memCtx->break_on_malloc)) {
        memCtx->break_on_malloc = 0;
        (void) fflush(stdout);
        fprintf(stderr,"reached malloc break limit (%lld)\n",
                memCtx->total_mallocs);
        fprintf(stderr, "program will now enter C debugger\n");
        (void) fflush(stderr);
	Hax_Breakpoint();
    }

    memCtx->current_malloc_packets++;
    if (memCtx->current_malloc_packets > memCtx->maximum_malloc_packets)
        memCtx->maximum_malloc_packets = memCtx->current_malloc_packets;
    memCtx->current_bytes_malloced += size;
    if (memCtx->current_bytes_malloced > memCtx->maximum_bytes_malloced)
        memCtx->maximum_bytes_malloced = memCtx->current_bytes_malloced;

    if (memCtx->init_malloced_bodies)
        memset (result->body, 0xff, (int) size);

    return result->body;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_DbCkfree - debugging ckfree
 *
 *        Verify that the low and high guards are intact, and if so
 *        then free the buffer else panic.
 *
 *        The guards are erased after being checked to catch duplicate
 *        frees.
 *
 *        The second and third arguments are file and line, these contain
 *        the filename and line number corresponding to the caller.
 *        These are sent by the ckfree macro; it uses the preprocessor
 *        autodefines __FILE__ and __LINE__.
 *
 *----------------------------------------------------------------------
 */

int
Hax_DbCkfree(
    Hax_Memoryp *memoryp,
    void        *ptr,
    char        *file,
    int          line)
{
    Memoryp *memCtx = (Memoryp *) memoryp;
    struct mem_header *memp = 0;  /* Must be zero for size calc */

    if (ptr == NULL) {
	Hax_Panic ((char *) "Cannot free memory at addres 0x0, %s line %d",
	    file, line);
    }

    /*
     * Since header ptr is zero, body offset will be size
     */
    memp = (struct mem_header *)(((char *) ptr) - (long)memp->body);

    if (memCtx->alloc_tracing)
        fprintf(stderr, "ckfree %p %lu %s %d\n", memp->body,
                memp->length, file, line);

    if (memCtx->validate_memory)
        Hax_ValidateAllMemory (memoryp, file, line);

    ValidateMemory (memCtx, memp, file, line, TRUE);

    memCtx->total_frees++;
    memCtx->current_malloc_packets--;
    memCtx->current_bytes_malloced -= memp->length;

    /*
     * Delink from allocated list
     */
    if (memp->flink != NULL)
        memp->flink->blink = memp->blink;
    if (memp->blink != NULL)
        memp->blink->flink = memp->flink;
    if (memCtx->allocHead == memp)
        memCtx->allocHead = memp->flink;
    free((char *) memp);
    return 0;
}

/*
 *--------------------------------------------------------------------
 *
 * Hax_DbCkrealloc - debugging ckrealloc
 *
 *	Reallocate a chunk of memory by allocating a new one of the
 *	right size, copying the old data to the new location, and then
 *	freeing the old memory space, using all the memory checking
 *	features of this package.
 *
 *--------------------------------------------------------------------
 */
void *
Hax_DbCkrealloc(
    Hax_Memoryp *memoryp,
    void *ptr,
    unsigned long int size,
    char *file,
    int line)
{
    void *newPtr;

    newPtr = Hax_DbCkalloc(memoryp, size, file, line);
    memcpy(newPtr, ptr, (int) size);
    Hax_DbCkfree(memoryp, ptr, file, line);
    return newPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * MemoryCmd --
 *     Implements the HAX memory command:
 *       memory info
 *       memory display
 *       break_on_malloc count
 *       trace_on_at_malloc count
 *       trace on|off
 *       validate on|off
 *
 * Results:
 *     Standard HAX results.
 *
 *----------------------------------------------------------------------
 */
	/* ARGSUSED */
static int
MemoryCmd (
    ClientData  clientData,
    Hax_Interp *interp,
    int         argc,
    char      **argv)
{
    Interp *iPtr = (Interp *) interp;
    Memoryp *memCtx = (Memoryp *) iPtr->memoryp;

    if (argc < 2) {
	Hax_AppendResult(interp, "wrong # args:  should be \"",
		argv[0], " option [args..]\"", (char *) NULL);
	return HAX_ERROR;
    }

    if (strcmp(argv[1],"trace") == 0) {
        if (argc != 3)
            goto bad_suboption;
        memCtx->alloc_tracing = (strcmp(argv[2],"on") == 0);
        return HAX_OK;
    }
    if (strcmp(argv[1],"init") == 0) {
        if (argc != 3)
            goto bad_suboption;
        memCtx->init_malloced_bodies = (strcmp(argv[2],"on") == 0);
        return HAX_OK;
    }
    if (strcmp(argv[1],"validate") == 0) {
        if (argc != 3)
             goto bad_suboption;
        memCtx->validate_memory = (strcmp(argv[2],"on") == 0);
        return HAX_OK;
    }
    if (strcmp(argv[1],"trace_on_at_malloc") == 0) {
        if (argc != 3)
            goto argError;
        if (Hax_GetInt(interp, argv[2], &memCtx->trace_on_at_malloc) != HAX_OK)
                return HAX_ERROR;
         return HAX_OK;
    }
    if (strcmp(argv[1],"break_on_malloc") == 0) {
        if (argc != 3)
            goto argError;
        if (Hax_GetInt(interp, argv[2], &memCtx->break_on_malloc) != HAX_OK)
                return HAX_ERROR;
        return HAX_OK;
    }

    if (strcmp(argv[1],"info") == 0) {
        dump_memory_info(memCtx, stdout);
        return HAX_OK;
    }
    if (strcmp(argv[1],"active") == 0) {
        if (argc != 2) {
	    Hax_AppendResult(interp, "wrong # args:  should be \"",
		    argv[0], " active", (char *) NULL);
	    return HAX_ERROR;
	}
        if (Hax_DumpActiveMemory (iPtr->memoryp) != HAX_OK) {
	    Hax_AppendResult(interp, "error dumping active memory ",
		    (char *) NULL);
	    return HAX_ERROR;
	}
	return HAX_OK;
    }
    Hax_AppendResult(interp, "bad option \"", argv[1],
	    "\":  should be info, init, active, break_on_malloc, ",
	    "trace_on_at_malloc, trace, or validate", (char *) NULL);
    return HAX_ERROR;

argError:
    Hax_AppendResult(interp, "wrong # args:  should be \"", argv[0],
	    " ", argv[1], "count\"", (char *) NULL);
    return HAX_ERROR;

bad_suboption:
    Hax_AppendResult(interp, "wrong # args:  should be \"", argv[0],
	    " ", argv[1], " on|off\"", (char *) NULL);
    return HAX_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_InitMemory --
 *     Initialize the memory command.
 *
 *----------------------------------------------------------------------
 */
void
Hax_InitMemory(
    Hax_Interp *interp)
{
Hax_CreateCommand (interp, (char *) "memory", MemoryCmd, (ClientData)NULL,
                  (Hax_CmdDeleteProc *) NULL);
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_CreateMemoryManagement --
 *     Create the Memoryp context.
 *
 *----------------------------------------------------------------------
 */
Hax_Memoryp *
Hax_CreateMemoryManagement(
    int break_on_malloc,
    int trace_on_at_malloc,
    int alloc_tracing,
    int init_malloced_bodies,
    int validate_memory)
{
    Memoryp *memoryp;

    memoryp = (Memoryp *) malloc(sizeof(Memoryp));
    memset(memoryp, 0, sizeof(Memoryp));

    memoryp->break_on_malloc = break_on_malloc;
    memoryp->trace_on_at_malloc = trace_on_at_malloc;
    memoryp->alloc_tracing = alloc_tracing;
    memoryp->init_malloced_bodies = init_malloced_bodies;
    memoryp->validate_memory = validate_memory;

    return (Hax_Memoryp *) memoryp;
}
