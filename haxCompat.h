/*
 * haxCompat.h --
 *
 *	Declarations of compat code used by the Hax interpreter.
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

/*
 * <ctype.h> - character types
 */

int			Hax_isalnum(int c);
int			Hax_isdigit(int c);
int			Hax_islower(int c);
int			Hax_isprint(int c);
int			Hax_isspace(int c);
int			Hax_isupper(int c);
int			Hax_tolower(int c);
int			Hax_toupper(int c);
int			Hax_isascii(int c);

/*
 * <stdio.h> - standard buffered input/output
 */

int			Hax_sprintf(char *str, const char *format, ...);

/*
 * <stdlib.h> - standard library definitions
 */

double			Hax_strtod(const char *nptr, char **endptr);

/*
 * <string.h> - string operations
 */

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

char *			Hax_memcpy (void *t, const void *f, Size_t n);
char *			Hax_memset (void *s, int c, Size_t n);
char *			Hax_strcat (char *dst, const char *src);
char *			Hax_strchr (const char *string, int c);
int			Hax_strcmp (const char *s1, const char *s2);
char *			Hax_strcpy (char *dst, const char *src);
Size_t			Hax_strcspn (const char *string, const char *chars);
char *			Hax_strerror (int error);
Size_t			Hax_strlen (const char *string);
int			Hax_strncmp (const char *s1, const char *s2,
			    Size_t nChars);
char *			Hax_strncpy (char *dst, const char *src,
                            Size_t numChars);
char *			Hax_strrchr (const char *string, int c);
char *			Hax_strstr (const char *string, const char *substring);

/*
 * <stdarg.h> - handle variable argument list
 */

#define Va_list __builtin_va_list
#define Va_start __builtin_va_start
#define Va_arg __builtin_va_arg
#define Va_copy __builtin_va_copy
#define Va_end __builtin_va_end

#endif /* _HAXCOMPAT */
