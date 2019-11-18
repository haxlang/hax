/*
 * Hax_sbrk.c --
 *
 *      Implementation of sbrk()/brk() emulation.
 *
 * Copyright 2019 Kamil Rytarowski
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The copyright holders
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#include "Hax_errno.h"

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

#ifndef INTPTR_T_DEFINED   
typedef __INTPTR_TYPE__ Intptr_t;
#define INTPTR_T_DEFINED   
#endif

void *Hax_CallocArena(Size_t number, Size_t size);

static int haxsbrk_inited;
static void *haxArena;
static Intptr_t Hax_current_brk;
static Intptr_t Hax_max_brk;

static void
Hax_init_sbrk(void)
{
    Size_t number;
    Size_t size;

    number = 256;
    size = 4096;    
    haxArena = Hax_CallocArena(number, size); /* 1MB */
    Hax_max_brk = 256 * 4096;
}

int
Hax_brk(void *addr)
{
    if (!haxsbrk_inited) {
	Hax_init_sbrk();
	haxsbrk_inited = 1;
    }

    if (haxArena < addr || ((char *)haxArena + Hax_max_brk) >= (char *)addr) {
	haxErrno = ENOMEM;
	return -1;
    }
    Hax_current_brk = (char *)haxArena - (char *)addr;

    return 0;
}

void *
Hax_sbrk(Intptr_t incr)
{
    Intptr_t saved_brk;

    if (!haxsbrk_inited) {
	Hax_init_sbrk();
	haxsbrk_inited = 1;
    }

    saved_brk = Hax_current_brk;

    Hax_current_brk += incr;
    if (Hax_current_brk < 0)
	Hax_current_brk = 0;
    else if (Hax_current_brk >= Hax_max_brk) {
	haxErrno = ENOMEM;
	return (void *)-1;
    }

    return (char *)haxArena + saved_brk;
}
