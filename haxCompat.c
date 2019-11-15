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
#include "compat/Hax_string.h"
#include "compat/Hax_stdlib.h"
#else
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#endif

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

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

int
Hax_atoi(const char *nptr)
{
#ifdef HAX_FREESTANDING
    return haxAtoi(nptr);
#else
    return atoi(nptr);
#endif
}

void
Hax_free(void *ptr)
{
#ifdef HAX_FREESTANDING
    haxFree(ptr);
#else
    free(ptr);
#endif
}

void *
Hax_malloc(Size_t size)
{
#ifdef HAX_FREESTANDING
    return haxMalloc(size);
#else
    return malloc(size);
#endif
}

void
Hax_qsort(void *base, Size_t nmemb, Size_t size,
	int (*compar)(const void *, const void *))
{
#ifdef HAX_FREESTANDING
    return haxQsort(base, nmemb, size, compar);
#else
    return qsort(base, nmemb, size, compar);
#endif
}

void *
Hax_realloc(void *ptr, Size_t size)
{
#ifdef HAX_FREESTANDING
    return haxRealloc(ptr, size);
#else
    return realloc(ptr, size);
#endif
}

double
Hax_strtod(const char *nptr, char **endptr)
{
#ifdef HAX_FREESTANDING
    return haxStrtod(nptr, endptr);
#else
    return strtod(nptr, endptr);
#endif
}

long
Hax_strtol(const char *nptr, char **endptr, int base)
{
#ifdef HAX_FREESTANDING
    return haxStrtol(nptr, endptr, base);
#else
    return strtol(nptr, endptr, base);
#endif
}

unsigned long
Hax_strtoul(const char *nptr, char **endptr, int base)
{
#ifdef HAX_FREESTANDING
    return haxStrtoul(nptr, endptr, base);
#else
    return strtoul(nptr, endptr, base);
#endif
}

long long int
Hax_strtoll(const char *nptr, char **endptr, int base)
{
#ifdef HAX_FREESTANDING
    return haxStrtoll(nptr, endptr, base);
#else
    return strtoll(nptr, endptr, base);
#endif
}

/*
 * <string.h> - string operations
 */

char *
Hax_memcpy (void *t, const void *f, Size_t n)
{
#ifdef HAX_FREESTANDING
    return __builtin_memcpy(t, f, n);
#else
    return memcpy(t, f, n);
#endif
}

char *
Hax_memset (void *s, int c, Size_t n)
{
#ifdef HAX_FREESTANDING
    return __builtin_memset(s, c, n);
#else
    return memset(s, c, n);
#endif
}

char *
Hax_strcat (char *dst, const char *src)
{
#ifdef HAX_FREESTANDING
    return haxStrcat(dst, src);
#else
    return strcat(dst, src);
#endif
}

char *
Hax_strchr (const char *string, int c)
{
#ifdef HAX_FREESTANDING
    return haxStrchr(string, c);
#else
    return strchr(string, c);
#endif
}

int
Hax_strcmp (const char *s1, const char *s2)
{
#ifdef HAX_FREESTANDING
    return haxStrcmp(s1, s2);
#else
    return strcmp(s1, s2);
#endif
}

char *
Hax_strcpy (char *dst, const char *src)
{
#ifdef HAX_FREESTANDING
    return haxStrcpy(dst, src);
#else
    return strcpy(dst, src);
#endif
}

Size_t
Hax_strcspn (const char *string, const char *chars)
{
#ifdef HAX_FREESTANDING
    return haxStrcspn(string, chars);
#else
    return strcspn(string, chars);
#endif
}

char *
Hax_strerror (int error)
{
#ifdef HAX_FREESTANDING
    return haxStrerror(error);
#else
    return strerror(error);
#endif
}

Size_t
Hax_strlen (const char *string)
{
#ifdef HAX_FREESTANDING
    return haxStrlen(string);
#else
    return strlen(string);
#endif
}

int
Hax_strncmp (const char *s1, const char *s2, Size_t nChars)
{
#ifdef HAX_FREESTANDING
    return haxStrncmp(s1, s2, nChars);
#else
    return strncmp(s1, s2, nChars);
#endif
}

char *
Hax_strncpy (char *dst, const char *src, Size_t numChars)
{
#ifdef HAX_FREESTANDING
    return haxStrncpy(dst, src, numChars);
#else
    return strncpy(dst, src, numChars);
#endif
}

char *
Hax_strrchr (const char *string, int c)
{
#ifdef HAX_FREESTANDING
    return haxStrrchr(string, c);
#else
    return strrchr(string, c);
#endif
}

char *
Hax_strstr (const char *string, const char *substring)
{
#ifdef HAX_FREESTANDING
    return haxStrstr(string, substring);
#else
    return strstr(string, substring);
#endif
}
