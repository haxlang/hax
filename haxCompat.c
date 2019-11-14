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

#ifndef _HAXCOMPAT
#define _HAXCOMPAT

#ifndef _HAXINT
#include "haxInt.h"
#endif

#include <ctype.h>
#include <string.h>

/*
 * <ctype.h> - character types
 */

int
Hax_isalnum(int c)
{
    return isalnum(c);
}

int
Hax_isdigit(int c)
{
    return isdigit(c);
}

int
Hax_islower(int c)
{
    return islower(c);
}

int
Hax_isprint(int c)
{
    return isprint(c);
}

int
Hax_isspace(int c)
{
    return isspace(c);
}

int
Hax_isupper(int c)
{
    return isupper(c);
}

int
Hax_tolower(int c)
{
    return tolower(c);
}

int
Hax_toupper(int c)
{
    return toupper(c);
}

int
Hax_isascii(int c)
{
    return isascii(c);
}

/*
 * <stdio.h> - standard buffered input/output
 */

int
Hax_sprintf(char *str, const char *format, ...)
{
    int result;
    va_list ap;

    va_start(ap, format);
    result = vsprintf(str, format, ap);
    va_end(ap);

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

#endif /* _HAXCOMPAT */
