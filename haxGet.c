/*
 * haxGet.c --
 *
 *	This file contains procedures to convert strings into
 *	other forms, like integers or floating-point numbers or
 *	booleans, doing syntax checking along the way.
 *
 * Copyright 1990-1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclGet.c,v 1.11 92/02/29 16:13:14 ouster Exp $ SPRITE (Berkeley)";
#endif /* not lint */

#include "haxInt.h"

/*
 *----------------------------------------------------------------------
 *
 * Hax_GetInt --
 *
 *	Given a string, produce the corresponding integer value.
 *
 * Results:
 *	The return value is normally HAX_OK;  in this case *intPtr
 *	will be set to the integer value equivalent to string.  If
 *	string is improperly formed then HAX_ERROR is returned and
 *	an error message will be left in interp->result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Hax_GetInt(
    Hax_Interp *interp,		/* Interpreter to use for error reporting. */
    char *string,		/* String containing a (possibly signed)
				 * integer in a form acceptable to strtol. */
    int *intPtr			/* Place to store converted result. */)
{
    char *end;
    int i;

    i = strtol(string, &end, 0);
    while ((*end != '\0') && isspace(*end)) {
	end++;
    }
    if ((end == string) || (*end != 0)) {
	Hax_AppendResult(interp, "expected integer but got \"", string,
		"\"", (char *) NULL);
	return HAX_ERROR;
    }
    *intPtr = i;
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_GetLong --
 *
 *	Given a string, produce the corresponding long integer
 *	value.
 *
 * Results:
 *	The return value is normally HAX_OK;  in this case *llongPtr
 *	will be set to the integer value equivalent to string.  If
 *	string is improperly formed then HAX_ERROR is returned and
 *	an error message will be left in interp->result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Hax_GetLong(
    Hax_Interp *interp,		/* Interpreter to use for error reporting. */
    char *string,		/* String containing a (possibly signed)
				 * integer in a form acceptable to strtol. */
    long int *longPtr		/* Place to store converted result. */)
{
    char *end;
    long int i;

    i = strtol(string, &end, 0);
    while ((*end != '\0') && isspace(*end)) {
	end++;
    }
    if ((end == string) || (*end != 0)) {
	Hax_AppendResult(interp, "expected integer but got \"", string,
		"\"", (char *) NULL);
	return HAX_ERROR;
    }
    *longPtr = i;
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_GetLongLong --
 *
 *	Given a string, produce the corresponding long long integer
 *	value.
 *
 * Results:
 *	The return value is normally HAX_OK;  in this case *llongPtr
 *	will be set to the integer value equivalent to string.  If
 *	string is improperly formed then HAX_ERROR is returned and
 *	an error message will be left in interp->result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Hax_GetLongLong(
    Hax_Interp *interp,		/* Interpreter to use for error reporting. */
    char *string,		/* String containing a (possibly signed)
				 * integer in a form acceptable to strtol. */
    long long int *llongPtr	/* Place to store converted result. */)
{
    char *end;
    long long int i;

    i = strtoll(string, &end, 0);
    while ((*end != '\0') && isspace(*end)) {
	end++;
    }
    if ((end == string) || (*end != 0)) {
	Hax_AppendResult(interp, "expected integer but got \"", string,
		"\"", (char *) NULL);
	return HAX_ERROR;
    }
    *llongPtr = i;
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_GetDouble --
 *
 *	Given a string, produce the corresponding double-precision
 *	floating-point value.
 *
 * Results:
 *	The return value is normally HAX_OK;  in this case *doublePtr
 *	will be set to the double-precision value equivalent to string.
 *	If string is improperly formed then HAX_ERROR is returned and
 *	an error message will be left in interp->result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Hax_GetDouble(
    Hax_Interp *interp,		/* Interpreter to use for error reporting. */
    char *string,		/* String containing a floating-point number
				 * in a form acceptable to strtod. */
    void *doublePtr		/* Place to store converted result. */)
{
    char *end;
    double d;

    d = strtod(string, &end);
    while ((*end != '\0') && isspace(*end)) {
	end++;
    }
    if ((end == string) || (*end != 0)) {
	Hax_AppendResult(interp, "expected floating-point number but got \"",
		string, "\"", (char *) NULL);
	return HAX_ERROR;
    }
    *(double *)doublePtr = d;
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_GetBoolean --
 *
 *	Given a string, return a 0/1 boolean value corresponding
 *	to the string.
 *
 * Results:
 *	The return value is normally HAX_OK;  in this case *boolPtr
 *	will be set to the 0/1 value equivalent to string.  If
 *	string is improperly formed then HAX_ERROR is returned and
 *	an error message will be left in interp->result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Hax_GetBoolean(
    Hax_Interp *interp,		/* Interpreter to use for error reporting. */
    char *string,		/* String containing a boolean number
				 * specified either as 1/0 or true/false or
				 * yes/no. */
    int *boolPtr		/* Place to store converted result, which
				 * will be 0 or 1. */)
{
    char c;
    char lowerCase[10];
    int i, length;

    /*
     * Convert the input string to all lower-case.
     */

    for (i = 0; i < 9; i++) {
	c = string[i];
	if (c == 0) {
	    break;
	}
	if ((c >= 'A') && (c <= 'Z')) {
	    c += 'a' - 'A';
	}
	lowerCase[i] = c;
    }
    lowerCase[i] = 0;

    length = strlen(lowerCase);
    c = lowerCase[0];
    if ((c == '0') && (lowerCase[1] == '\0')) {
	*boolPtr = 0;
    } else if ((c == '1') && (lowerCase[1] == '\0')) {
	*boolPtr = 1;
    } else if ((c == 'y') && (strncmp(lowerCase, "yes", length) == 0)) {
	*boolPtr = 1;
    } else if ((c == 'n') && (strncmp(lowerCase, "no", length) == 0)) {
	*boolPtr = 0;
    } else if ((c == 't') && (strncmp(lowerCase, "true", length) == 0)) {
	*boolPtr = 1;
    } else if ((c == 'f') && (strncmp(lowerCase, "false", length) == 0)) {
	*boolPtr = 0;
    } else if ((c == 'o') && (length >= 2)) {
	if (strncmp(lowerCase, "on", length) == 0) {
	    *boolPtr = 1;
	} else if (strncmp(lowerCase, "off", length) == 0) {
	    *boolPtr = 0;
	}
    } else {
	Hax_AppendResult(interp, "expected boolean value but got \"",
		string, "\"", (char *) NULL);
	return HAX_ERROR;
    }
    return HAX_OK;
}
