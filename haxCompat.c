/*
 * haxCompat.c --
 *
 *	Definition of compat code used by the Hax interpreter.
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

#include "haxInt.h"

#ifdef HAX_FREESTANDING
#include "compat/Hax_ctype.h"
#else
#include <ctype.h>
#endif

#include <string.h>


/*
 * <ctype.h> - character types
 */

int
Hax_isalnum(int c)
{
#ifdef HAX_FREESTANDING
    return haxIsalnum(c);
#else
    return isalnum(c);
#endif
}

int
Hax_isdigit(int c)
{
#ifdef HAX_FREESTANDING
    return haxIsdigit(c);
#else
    return isdigit(c);
#endif
}

int
Hax_islower(int c)
{
#ifdef HAX_FREESTANDING
    return haxIslower(c);
#else
    return islower(c);
#endif
}

int
Hax_isprint(int c)
{
#ifdef HAX_FREESTANDING
    return haxIsprint(c);
#else
    return isprint(c);
#endif
}

int
Hax_isspace(int c)
{
#ifdef HAX_FREESTANDING
    return haxIsspace(c);
#else
    return isspace(c);
#endif
}

int
Hax_isupper(int c)
{
#ifdef HAX_FREESTANDING
    return haxIsupper(c);
#else
    return isupper(c);
#endif
}

int
Hax_tolower(int c)
{
#ifdef HAX_FREESTANDING
    return haxTolower(c);
#else
    return tolower(c);
#endif
}

int
Hax_toupper(int c)
{
#ifdef HAX_FREESTANDING
    return haxToupper(c);
#else
    return toupper(c);
#endif
}

int
Hax_isascii(int c)
{
#ifdef HAX_FREESTANDING
    return haxIsascii(c);
#else
    return isascii(c);
#endif
}

/*
 * <stdio.h> - standard buffered input/output
 */

int
Hax_sprintf(char *str, const char *format, ...)
{
    int result;
    Va_list ap;

    Va_start(ap, format);
    result = vsprintf(str, format, ap);
    Va_end(ap);

    return result;
}

/*
 * <stdlib.h> - standard library definitions
 */

double
Hax_strtod(const char *nptr, char **endptr)
{
    return strtod(nptr, endptr);
}

/*
 * <string.h> - string operations
 */

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

char *
Hax_memcpy (void *t, const void *f, Size_t n)
{
    return memcpy(t, f, n);
}

char *
Hax_memset (void *s, int c, Size_t n)
{
    return memset(s, c, n);
}

char *
Hax_strcat (char *dst, const char *src)
{
    return strcat(dst, src);
}

char *
Hax_strchr (const char *string, int c)
{
    return strchr(string, c);
}

int
Hax_strcmp (const char *s1, const char *s2)
{
    return strcmp(s1, s2);
}

char *
Hax_strcpy (char *dst, const char *src)
{
    return strcpy(dst, src);
}

Size_t
Hax_strcspn (const char *string, const char *chars)
{
    return strcspn(string, chars);
}

char *
Hax_strerror (int error)
{
    return strerror(error);
}

Size_t
Hax_strlen (const char *string)
{
    return strlen(string);
}

int
Hax_strncmp (const char *s1, const char *s2, Size_t nChars)
{
    return strncmp(s1, s2, nChars);
}

char *
Hax_strncpy (char *dst, const char *src, Size_t numChars)
{
    return strncpy(dst, src, numChars);
}

char *
Hax_strrchr (const char *string, int c)
{
    return strrchr(string, c);
}

char *
Hax_strstr (const char *string, const char *substring)
{
    return strstr(string, substring);
}
