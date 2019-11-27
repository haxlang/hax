/*
 * libfuzzer.c --
 *
 *      Libfuzzer driver for Hax.
 *
 * Copyright 2019 Kamil Rytarowski
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appears in all copies.  The copyright holder
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "hax.h"

static Hax_Memoryp *memoryp;
static Hax_Interp *interp;

static void
Initialize(void)
{
    memoryp = Hax_CreateMemoryManagement(0, 0, 0, 0, 1);
    interp = Hax_CreateInterp(memoryp);
}

int
LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    static bool Initialized;

    if (!Initialized) {
	Initialize();
	Initialized = true;
    }

    if (Size < 1)
	return 0;

    char *buf = (char *) ckalloc(memoryp, Size);
    memcpy(buf, Data, Size - 1);
    buf[Size - 1] = 0;
    Hax_Eval(interp, NULL, buf, 0, (char **) NULL);
    ckfree(memoryp, buf);

    return 0;
}
