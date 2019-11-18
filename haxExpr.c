/* 
 * haxExpr.c --
 *
 *	This file contains the code to evaluate expressions for
 *	Hax.
 *
 *	This implementation of floating-point support was modelled
 *	after an initial implementation by Bill Carpenter.
 *
 * Copyright 1987-1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclExpr.c,v 1.36 92/08/16 13:25:34 ouster Exp $ SPRITE (Berkeley)";
#endif

#include "haxInt.h"

/*
 * The stuff below is a bit of a hack so that this file can be used
 * in environments that include no UNIX, i.e. no errno.  Just define
 * errno here.
 */

extern int errno;
#define ERANGE 34

/*
 * The data structure below is used to describe an expression value,
 * which can be either an integer (the usual case), a double-precision
 * floating-point value, or a string.  A given number has only one
 * value at a time.
 */

#define STATIC_STRING_SPACE 150

typedef struct {
    long long int llongValue;	/* Integer value, if any. */
    double  doubleValue;	/* Floating-point value, if any. */
    ParseValue pv;		/* Used to hold a string value, if any. */
    char staticSpace[STATIC_STRING_SPACE];
				/* Storage for small strings;  large ones
				 * are malloc-ed. */
    int type;			/* Type of value:  TYPE_LLONG, TYPE_DOUBLE,
				 * or TYPE_STRING. */
} Value;

/*
 * Valid values for type:
 */

#define TYPE_LLONG	0
#define TYPE_DOUBLE	1
#define TYPE_STRING	2


/*
 * The data structure below describes the state of parsing an expression.
 * It's passed among the routines in this module.
 */

typedef struct {
    char *originalExpr;		/* The entire expression, as originally
				 * passed to Hax_Expr. */
    char *expr;			/* Position to the next character to be
				 * scanned from the expression string. */
    int token;			/* Type of the last token to be parsed from
				 * expr.  See below for definitions.
				 * Corresponds to the characters just
				 * before expr. */
} ExprInfo;

/*
 * The token types are defined below.  In addition, there is a table
 * associating a precedence with each operator.  The order of types
 * is important.  Consult the code before changing it.
 */

#define VALUE		0
#define OPEN_PAREN	1
#define CLOSE_PAREN	2
#define END		3
#define UNKNOWN		4

/*
 * Binary operators:
 */

#define MULT		8
#define DIVIDE		9
#define MOD		10
#define PLUS		11
#define MINUS		12
#define LEFT_SHIFT	13
#define RIGHT_SHIFT	14
#define LESS		15
#define GREATER		16
#define LEQ		17
#define GEQ		18
#define EQUAL		19
#define NEQ		20
#define BIT_AND		21
#define BIT_XOR		22
#define BIT_OR		23
#define AND		24
#define OR		25
#define QUESTY		26
#define COLON		27

/*
 * Unary operators:
 */

#define	UNARY_MINUS	28
#define NOT		29
#define BIT_NOT		30

/*
 * Precedence table.  The values for non-operator token types are ignored.
 */

int precTable[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    11, 11, 11,				/* MULT, DIVIDE, MOD */
    10, 10,				/* PLUS, MINUS */
    9, 9,				/* LEFT_SHIFT, RIGHT_SHIFT */
    8, 8, 8, 8,				/* LESS, GREATER, LEQ, GEQ */
    7, 7,				/* EQUAL, NEQ */
    6,					/* BIT_AND */
    5,					/* BIT_XOR */
    4,					/* BIT_OR */
    3,					/* AND */
    2,					/* OR */
    1, 1,				/* QUESTY, COLON */
    12, 12, 12				/* UNARY_MINUS, NOT, BIT_NOT */
};

/*
 * Mapping from operator numbers to strings;  used for error messages.
 */

const char *operatorStrings[] = {
    "VALUE", "(", ")", "END", "UNKNOWN", "5", "6", "7",
    "*", "/", "%", "+", "-", "<<", ">>", "<", ">", "<=",
    ">=", "==", "!=", "&", "^", "|", "&&", "||", "?", ":",
    "-", "!", "~"
};

/*
 * Declarations for local procedures to this file:
 */

static int		ExprGetValue (Hax_Interp *interp,
			    ExprInfo *infoPtr, int prec, Value *valuePtr);
static int		ExprLex (Hax_Interp *interp,
			    ExprInfo *infoPtr, Value *valuePtr);
static void		ExprMakeString (Value *valuePtr);
static int		ExprParseString (Hax_Interp *interp,
			    char *string, Value *valuePtr);
static int		ExprTopLevel (Hax_Interp *interp,
			    char *string, Value *valuePtr);

/*
 *--------------------------------------------------------------
 *
 * ExprParseString --
 *
 *	Given a string (such as one coming from command or variable
 *	substitution), make a Value based on the string.  The value
 *	will be a floating-point or integer, if possible, or else it
 *	will just be a copy of the string.
 *
 * Results:
 *	HAX_OK is returned under normal circumstances, and HAX_ERROR
 *	is returned if a floating-point overflow or underflow occurred
 *	while reading in a number.  The value at *valuePtr is modified
 *	to hold a number, if possible.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

static int
ExprParseString(
    Hax_Interp *interp,		/* Where to store error message. */
    char *string,		/* String to turn into value. */
    Value *valuePtr		/* Where to store value information.
				 * Caller must have initialized pv field. */)
{
    char c;

    /*
     * Try to convert the string to a number.
     */

    c = *string;
    if (((c >= '0') && (c <= '9')) || (c == '-') || (c == '.')) {
	char *term;

	valuePtr->type = TYPE_LLONG;
	errno = 0;
	valuePtr->llongValue = strtol(string, &term, 0);
	c = *term;
	if ((c == '\0') && (errno != ERANGE)) {
	    return HAX_OK;
	}
	if ((c == '.') || (c == 'e') || (c == 'E') || (errno == ERANGE)) {
	    errno = 0;
	    valuePtr->doubleValue = strtod(string, &term);
	    if (errno == ERANGE) {
		Hax_ResetResult(interp);
		if (valuePtr->doubleValue == 0.0) {
		    Hax_AppendResult(interp, "floating-point value \"",
			    string, "\" too small to represent",
			    (char *) NULL);
		} else {
		    Hax_AppendResult(interp, "floating-point value \"",
			    string, "\" too large to represent",
			    (char *) NULL);
		}
		return HAX_ERROR;
	    }
	    if (*term == '\0') {
		valuePtr->type = TYPE_DOUBLE;
		return HAX_OK;
	    }
	}
    }

    /*
     * Not a valid number.  Save a string value (but don't do anything
     * if it's already the value).
     */

    valuePtr->type = TYPE_STRING;
    if (string != valuePtr->pv.buffer) {
	int length, shortfall;

	length = strlen(string);
	valuePtr->pv.next = valuePtr->pv.buffer;
	shortfall = length - (valuePtr->pv.end - valuePtr->pv.buffer);
	if (shortfall > 0) {
	    (*valuePtr->pv.expandProc)(&valuePtr->pv, shortfall);
	}
	strcpy(valuePtr->pv.buffer, string);
    }
    return HAX_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ExprLex --
 *
 *	Lexical analyzer for expression parser:  parses a single value,
 *	operator, or other syntactic element from an expression string.
 *
 * Results:
 *	HAX_OK is returned unless an error occurred while doing lexical
 *	analysis or executing an embedded command.  In that case a
 *	standard Hax error is returned, using interp->result to hold
 *	an error message.  In the event of a successful return, the token
 *	and field in infoPtr is updated to refer to the next symbol in
 *	the expression string, and the expr field is advanced past that
 *	token;  if the token is a value, then the value is stored at
 *	valuePtr.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ExprLex(
    Hax_Interp *interp,			/* Interpreter to use for error
					 * reporting. */
    ExprInfo *infoPtr,		/* Describes the state of the parse. */
    Value *valuePtr		/* Where to store value, if that is
					 * what's parsed from string.  Caller
					 * must have initialized pv field
					 * correctly. */)
{
    char *p, c;
    char *var, *term;
    int result;

    p = infoPtr->expr;
    c = *p;
    while (isspace(c)) {
	p++;
	c = *p;
    }
    infoPtr->expr = p+1;
    switch (c) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '.':

	    /*
	     * Number.  First read an integer.  Then if it looks like
	     * there's a floating-point number (or if it's too big a
	     * number to fit in an integer), parse it as a floating-point
	     * number.
	     */

	    infoPtr->token = VALUE;
	    valuePtr->type = TYPE_LLONG;
	    errno = 0;
	    valuePtr->llongValue = strtoul(p, &term, 0);
	    c = *term;
	    if ((c == '.') || (c == 'e') || (c == 'E') || (errno == ERANGE)) {
		char *term2;

		errno = 0;
		valuePtr->doubleValue = strtod(p, &term2);
		if (errno == ERANGE) {
		    Hax_ResetResult(interp);
		    if (valuePtr->doubleValue == 0.0) {
			interp->result =
				(char *) "floating-point value too small to "
				    "represent";
		    } else {
			interp->result =
				(char *) "floating-point value too large to "
				    "represent";
		    }
		    return HAX_ERROR;
		}
		if (term2 == infoPtr->expr) {
		    interp->result =
			(char *) "poorly-formed floating-point value";
		    return HAX_ERROR;
		}
		valuePtr->type = TYPE_DOUBLE;
		infoPtr->expr = term2;
	    } else {
		infoPtr->expr = term;
	    }
	    return HAX_OK;

	case '$':

	    /*
	     * Variable.  Fetch its value, then see if it makes sense
	     * as an integer or floating-point number.
	     */

	    infoPtr->token = VALUE;
	    var = Hax_ParseVar(interp, p, &infoPtr->expr);
	    if (var == NULL) {
		return HAX_ERROR;
	    }
	    if (((Interp *) interp)->noEval) {
		valuePtr->type = TYPE_LLONG;
		valuePtr->llongValue = 0;
		return HAX_OK;
	    }
	    return ExprParseString(interp, var, valuePtr);

	case '[':
	    infoPtr->token = VALUE;
	    result = Hax_Eval(interp, NULL, p+1, HAX_BRACKET_TERM,
		    &infoPtr->expr);
	    if (result != HAX_OK) {
		return result;
	    }
	    infoPtr->expr++;
	    if (((Interp *) interp)->noEval) {
		valuePtr->type = TYPE_LLONG;
		valuePtr->llongValue = 0;
		Hax_ResetResult(interp);
		return HAX_OK;
	    }
	    result = ExprParseString(interp, interp->result, valuePtr);
	    if (result != HAX_OK) {
		return result;
	    }
	    Hax_ResetResult(interp);
	    return HAX_OK;

	case '"':
	    infoPtr->token = VALUE;
	    result = HaxParseQuotes(interp, infoPtr->expr, '"', 0,
		    &infoPtr->expr, &valuePtr->pv);
	    if (result != HAX_OK) {
		return result;
	    }
	    return ExprParseString(interp, valuePtr->pv.buffer, valuePtr);

	case '{':
	    infoPtr->token = VALUE;
	    result = HaxParseBraces(interp, infoPtr->expr, &infoPtr->expr,
		    &valuePtr->pv);
	    if (result != HAX_OK) {
		return result;
	    }
	    return ExprParseString(interp, valuePtr->pv.buffer, valuePtr);

	case '(':
	    infoPtr->token = OPEN_PAREN;
	    return HAX_OK;

	case ')':
	    infoPtr->token = CLOSE_PAREN;
	    return HAX_OK;

	case '*':
	    infoPtr->token = MULT;
	    return HAX_OK;

	case '/':
	    infoPtr->token = DIVIDE;
	    return HAX_OK;

	case '%':
	    infoPtr->token = MOD;
	    return HAX_OK;

	case '+':
	    infoPtr->token = PLUS;
	    return HAX_OK;

	case '-':
	    infoPtr->token = MINUS;
	    return HAX_OK;

	case '?':
	    infoPtr->token = QUESTY;
	    return HAX_OK;

	case ':':
	    infoPtr->token = COLON;
	    return HAX_OK;

	case '<':
	    switch (p[1]) {
		case '<':
		    infoPtr->expr = p+2;
		    infoPtr->token = LEFT_SHIFT;
		    break;
		case '=':
		    infoPtr->expr = p+2;
		    infoPtr->token = LEQ;
		    break;
		default:
		    infoPtr->token = LESS;
		    break;
	    }
	    return HAX_OK;

	case '>':
	    switch (p[1]) {
		case '>':
		    infoPtr->expr = p+2;
		    infoPtr->token = RIGHT_SHIFT;
		    break;
		case '=':
		    infoPtr->expr = p+2;
		    infoPtr->token = GEQ;
		    break;
		default:
		    infoPtr->token = GREATER;
		    break;
	    }
	    return HAX_OK;

	case '=':
	    if (p[1] == '=') {
		infoPtr->expr = p+2;
		infoPtr->token = EQUAL;
	    } else {
		infoPtr->token = UNKNOWN;
	    }
	    return HAX_OK;

	case '!':
	    if (p[1] == '=') {
		infoPtr->expr = p+2;
		infoPtr->token = NEQ;
	    } else {
		infoPtr->token = NOT;
	    }
	    return HAX_OK;

	case '&':
	    if (p[1] == '&') {
		infoPtr->expr = p+2;
		infoPtr->token = AND;
	    } else {
		infoPtr->token = BIT_AND;
	    }
	    return HAX_OK;

	case '^':
	    infoPtr->token = BIT_XOR;
	    return HAX_OK;

	case '|':
	    if (p[1] == '|') {
		infoPtr->expr = p+2;
		infoPtr->token = OR;
	    } else {
		infoPtr->token = BIT_OR;
	    }
	    return HAX_OK;

	case '~':
	    infoPtr->token = BIT_NOT;
	    return HAX_OK;

	case 0:
	    infoPtr->token = END;
	    infoPtr->expr = p;
	    return HAX_OK;

	default:
	    infoPtr->expr = p+1;
	    infoPtr->token = UNKNOWN;
	    return HAX_OK;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ExprGetValue --
 *
 *	Parse a "value" from the remainder of the expression in infoPtr.
 *
 * Results:
 *	Normally HAX_OK is returned.  The value of the expression is
 *	returned in *valuePtr.  If an error occurred, then interp->result
 *	contains an error message and HAX_ERROR is returned.
 *	InfoPtr->token will be left pointing to the token AFTER the
 *	expression, and infoPtr->expr will point to the character just
 *	after the terminating token.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ExprGetValue(
    Hax_Interp *interp,			/* Interpreter to use for error
					 * reporting. */
    ExprInfo *infoPtr,		/* Describes the state of the parse
					 * just before the value (i.e. ExprLex
					 * will be called to get first token
					 * of value). */
    int prec,				/* Treat any un-parenthesized operator
					 * with precedence <= this as the end
					 * of the expression. */
    Value *valuePtr			/* Where to store the value of the
					 * expression.   Caller must have
					 * initialized pv field. */)
{
    Interp *iPtr = (Interp *) interp;
    Value value2;			/* Second operand for current
					 * operator.  */
    int op;				/* Current operator (either unary
					 * or binary). */
    int badType;			/* Type of offending argument;  used
					 * for error messages. */
    int gotOp;				/* Non-zero means already lexed the
					 * operator (while picking up value
					 * for unary operator).  Don't lex
					 * again. */
    int result;

    /*
     * There are two phases to this procedure.  First, pick off an initial
     * value.  Then, parse (binary operator, value) pairs until done.
     */

    gotOp = 0;
    value2.pv.buffer = value2.pv.next = value2.staticSpace;
    value2.pv.end = value2.pv.buffer + STATIC_STRING_SPACE - 1;
    value2.pv.expandProc = HaxExpandParseValue;
    value2.pv.clientData = (ClientData) NULL;
    result = ExprLex(interp, infoPtr, valuePtr);
    if (result != HAX_OK) {
	goto done;
    }
    if (infoPtr->token == OPEN_PAREN) {

	/*
	 * Parenthesized sub-expression.
	 */

	result = ExprGetValue(interp, infoPtr, -1, valuePtr);
	if (result != HAX_OK) {
	    goto done;
	}
	if (infoPtr->token != CLOSE_PAREN) {
	    Hax_ResetResult(interp);
	    Hax_AppendResult(interp,
		    "unmatched parentheses in expression \"",
		    infoPtr->originalExpr, "\"", (char *) NULL);
	    result = HAX_ERROR;
	    goto done;
	}
    } else {
	if (infoPtr->token == MINUS) {
	    infoPtr->token = UNARY_MINUS;
	}
	if (infoPtr->token >= UNARY_MINUS) {

	    /*
	     * Process unary operators.
	     */

	    op = infoPtr->token;
	    result = ExprGetValue(interp, infoPtr, precTable[infoPtr->token],
		    valuePtr);
	    if (result != HAX_OK) {
		goto done;
	    }
	    switch (op) {
		case UNARY_MINUS:
		    if (valuePtr->type == TYPE_LLONG) {
			valuePtr->llongValue = -valuePtr->llongValue;
		    } else if (valuePtr->type == TYPE_DOUBLE){
			valuePtr->doubleValue = -valuePtr->doubleValue;
		    } else {
			badType = valuePtr->type;
			goto illegalType;
		    } 
		    break;
		case NOT:
		    if (valuePtr->type == TYPE_LLONG) {
			valuePtr->llongValue = !valuePtr->llongValue;
		    } else if (valuePtr->type == TYPE_DOUBLE) {
			/*
			 * Theoretically, should be able to use
			 * "!valuePtr->llongValue", but apparently some
			 * compilers can't handle it.
			 */
			if (valuePtr->doubleValue == 0.0) {
			    valuePtr->llongValue = 1;
			} else {
			    valuePtr->llongValue = 0;
			}
			valuePtr->type = TYPE_LLONG;
		    } else {
			badType = valuePtr->type;
			goto illegalType;
		    }
		    break;
		case BIT_NOT:
		    if (valuePtr->type == TYPE_LLONG) {
			valuePtr->llongValue = ~valuePtr->llongValue;
		    } else {
			badType  = valuePtr->type;
			goto illegalType;
		    }
		    break;
	    }
	    gotOp = 1;
	} else if (infoPtr->token != VALUE) {
	    goto syntaxError;
	}
    }

    /*
     * Got the first operand.  Now fetch (operator, operand) pairs.
     */

    if (!gotOp) {
	result = ExprLex(interp, infoPtr, &value2);
	if (result != HAX_OK) {
	    goto done;
	}
    }
    while (1) {
	op = infoPtr->token;
	value2.pv.next = value2.pv.buffer;
	if ((op < MULT) || (op >= UNARY_MINUS)) {
	    if ((op == END) || (op == CLOSE_PAREN)) {
		result = HAX_OK;
		goto done;
	    } else {
		goto syntaxError;
	    }
	}
	if (precTable[op] <= prec) {
	    result = HAX_OK;
	    goto done;
	}

	/*
	 * If we're doing an AND or OR and the first operand already
	 * determines the result, don't execute anything in the
	 * second operand:  just parse.  Same style for ?: pairs.
	 */

	if ((op == AND) || (op == OR) || (op == QUESTY)) {
	    if (valuePtr->type == TYPE_DOUBLE) {
		valuePtr->llongValue = valuePtr->doubleValue != 0;
		valuePtr->type = TYPE_LLONG;
	    } else if (valuePtr->type == TYPE_STRING) {
		badType = TYPE_STRING;
		goto illegalType;
	    }
	    if (((op == AND) && !valuePtr->llongValue)
		    || ((op == OR) && valuePtr->llongValue)) {
		iPtr->noEval++;
		result = ExprGetValue(interp, infoPtr, precTable[op],
			&value2);
		iPtr->noEval--;
	    } else if (op == QUESTY) {
		if (valuePtr->llongValue != 0) {
		    valuePtr->pv.next = valuePtr->pv.buffer;
		    result = ExprGetValue(interp, infoPtr, precTable[op],
			    valuePtr);
		    if (result != HAX_OK) {
			goto done;
		    }
		    if (infoPtr->token != COLON) {
			goto syntaxError;
		    }
		    value2.pv.next = value2.pv.buffer;
		    iPtr->noEval++;
		    result = ExprGetValue(interp, infoPtr, precTable[op],
			    &value2);
		    iPtr->noEval--;
		} else {
		    iPtr->noEval++;
		    result = ExprGetValue(interp, infoPtr, precTable[op],
			    &value2);
		    iPtr->noEval--;
		    if (result != HAX_OK) {
			goto done;
		    }
		    if (infoPtr->token != COLON) {
			goto syntaxError;
		    }
		    valuePtr->pv.next = valuePtr->pv.buffer;
		    result = ExprGetValue(interp, infoPtr, precTable[op],
			    valuePtr);
		}
	    } else {
		result = ExprGetValue(interp, infoPtr, precTable[op],
			&value2);
	    }
	} else {
	    result = ExprGetValue(interp, infoPtr, precTable[op],
		    &value2);
	}
	if (result != HAX_OK) {
	    goto done;
	}
	if ((infoPtr->token < MULT) && (infoPtr->token != VALUE)
		&& (infoPtr->token != END)
		&& (infoPtr->token != CLOSE_PAREN)) {
	    goto syntaxError;
	}

	/*
	 * At this point we've got two values and an operator.  Check
	 * to make sure that the particular data types are appropriate
	 * for the particular operator, and perform type conversion
	 * if necessary.
	 */

	switch (op) {

	    /*
	     * For the operators below, no strings are allowed and
	     * ints get converted to floats if necessary.
	     */

	    case MULT: case DIVIDE: case PLUS: case MINUS:
		if ((valuePtr->type == TYPE_STRING)
			|| (value2.type == TYPE_STRING)) {
		    badType = TYPE_STRING;
		    goto illegalType;
		}
		if (valuePtr->type == TYPE_DOUBLE) {
		    if (value2.type == TYPE_LLONG) {
			value2.doubleValue = value2.llongValue;
			value2.type = TYPE_DOUBLE;
		    }
		} else if (value2.type == TYPE_DOUBLE) {
		    if (valuePtr->type == TYPE_LLONG) {
			valuePtr->doubleValue = valuePtr->llongValue;
			valuePtr->type = TYPE_DOUBLE;
		    }
		}
		break;

	    /*
	     * For the operators below, only integers are allowed.
	     */

	    case MOD: case LEFT_SHIFT: case RIGHT_SHIFT:
	    case BIT_AND: case BIT_XOR: case BIT_OR:
		 if (valuePtr->type != TYPE_LLONG) {
		     badType = valuePtr->type;
		     goto illegalType;
		 } else if (value2.type != TYPE_LLONG) {
		     badType = value2.type;
		     goto illegalType;
		 }
		 break;

	    /*
	     * For the operators below, any type is allowed but the
	     * two operands must have the same type.  Convert integers
	     * to floats and either to strings, if necessary.
	     */

	    case LESS: case GREATER: case LEQ: case GEQ:
	    case EQUAL: case NEQ:
		if (valuePtr->type == TYPE_STRING) {
		    if (value2.type != TYPE_STRING) {
			ExprMakeString(&value2);
		    }
		} else if (value2.type == TYPE_STRING) {
		    if (valuePtr->type != TYPE_STRING) {
			ExprMakeString(valuePtr);
		    }
		} else if (valuePtr->type == TYPE_DOUBLE) {
		    if (value2.type == TYPE_LLONG) {
			value2.doubleValue = value2.llongValue;
			value2.type = TYPE_DOUBLE;
		    }
		} else if (value2.type == TYPE_DOUBLE) {
		     if (valuePtr->type == TYPE_LLONG) {
			valuePtr->doubleValue = valuePtr->llongValue;
			valuePtr->type = TYPE_DOUBLE;
		    }
		}
		break;

	    /*
	     * For the operators below, no strings are allowed, but
	     * no int->double conversions are performed.
	     */

	    case AND: case OR:
		if (valuePtr->type == TYPE_STRING) {
		    badType = valuePtr->type;
		    goto illegalType;
		}
		if (value2.type == TYPE_STRING) {
		    badType = value2.type;
		    goto illegalType;
		}
		break;

	    /*
	     * For the operators below, type and conversions are
	     * irrelevant:  they're handled elsewhere.
	     */

	    case QUESTY: case COLON:
		break;

	    /*
	     * Any other operator is an error.
	     */

	    default:
		interp->result = (char *) "unknown operator in expression";
		result = HAX_ERROR;
		goto done;
	}

	/*
	 * If necessary, convert one of the operands to the type
	 * of the other.  If the operands are incompatible with
	 * the operator (e.g. "+" on strings) then return an
	 * error.
	 */

	switch (op) {
	    case MULT:
		if (valuePtr->type == TYPE_LLONG) {
		    valuePtr->llongValue *= value2.llongValue;
		} else {
		    valuePtr->doubleValue *= value2.doubleValue;
		}
		break;
	    case DIVIDE:
		if (valuePtr->type == TYPE_LLONG) {
		    if (value2.llongValue == 0) {
			divideByZero:
			interp->result = (char *) "divide by zero";
			result = HAX_ERROR;
			goto done;
		    }
		    valuePtr->llongValue /= value2.llongValue;
		} else {
		    if (value2.doubleValue == 0.0) {
			goto divideByZero;
		    }
		    valuePtr->doubleValue /= value2.doubleValue;
		}
		break;
	    case MOD:
		if (value2.llongValue == 0) {
		    goto divideByZero;
		}
		valuePtr->llongValue %= value2.llongValue;
		break;
	    case PLUS:
		if (valuePtr->type == TYPE_LLONG) {
		    valuePtr->llongValue += value2.llongValue;
		} else {
		    valuePtr->doubleValue += value2.doubleValue;
		}
		break;
	    case MINUS:
		if (valuePtr->type == TYPE_LLONG) {
		    valuePtr->llongValue -= value2.llongValue;
		} else {
		    valuePtr->doubleValue -= value2.doubleValue;
		}
		break;
	    case LEFT_SHIFT:
		valuePtr->llongValue <<= value2.llongValue;
		break;
	    case RIGHT_SHIFT:
		/*
		 * The following code is a bit tricky:  it ensures that
		 * right shifts propagate the sign bit even on machines
		 * where ">>" won't do it by default.
		 */

		if (valuePtr->llongValue < 0) {
		    valuePtr->llongValue =
			    ~((~valuePtr->llongValue) >> value2.llongValue);
		} else {
		    valuePtr->llongValue >>= value2.llongValue;
		}
		break;
	    case LESS:
		if (valuePtr->type == TYPE_LLONG) {
		    valuePtr->llongValue =
			valuePtr->llongValue < value2.llongValue;
		} else if (valuePtr->type == TYPE_DOUBLE) {
		    valuePtr->llongValue =
			valuePtr->doubleValue < value2.doubleValue;
		} else {
		    valuePtr->llongValue =
			    strcmp(valuePtr->pv.buffer, value2.pv.buffer) < 0;
		}
		valuePtr->type = TYPE_LLONG;
		break;
	    case GREATER:
		if (valuePtr->type == TYPE_LLONG) {
		    valuePtr->llongValue =
			valuePtr->llongValue > value2.llongValue;
		} else if (valuePtr->type == TYPE_DOUBLE) {
		    valuePtr->llongValue =
			valuePtr->doubleValue > value2.doubleValue;
		} else {
		    valuePtr->llongValue =
			    strcmp(valuePtr->pv.buffer, value2.pv.buffer) > 0;
		}
		valuePtr->type = TYPE_LLONG;
		break;
	    case LEQ:
		if (valuePtr->type == TYPE_LLONG) {
		    valuePtr->llongValue =
			valuePtr->llongValue <= value2.llongValue;
		} else if (valuePtr->type == TYPE_DOUBLE) {
		    valuePtr->llongValue =
			valuePtr->doubleValue <= value2.doubleValue;
		} else {
		    valuePtr->llongValue =
			    strcmp(valuePtr->pv.buffer, value2.pv.buffer) <= 0;
		}
		valuePtr->type = TYPE_LLONG;
		break;
	    case GEQ:
		if (valuePtr->type == TYPE_LLONG) {
		    valuePtr->llongValue =
			valuePtr->llongValue >= value2.llongValue;
		} else if (valuePtr->type == TYPE_DOUBLE) {
		    valuePtr->llongValue =
			valuePtr->doubleValue >= value2.doubleValue;
		} else {
		    valuePtr->llongValue =
			    strcmp(valuePtr->pv.buffer, value2.pv.buffer) >= 0;
		}
		valuePtr->type = TYPE_LLONG;
		break;
	    case EQUAL:
		if (valuePtr->type == TYPE_LLONG) {
		    valuePtr->llongValue =
			valuePtr->llongValue == value2.llongValue;
		} else if (valuePtr->type == TYPE_DOUBLE) {
		    valuePtr->llongValue =
			valuePtr->doubleValue == value2.doubleValue;
		} else {
		    valuePtr->llongValue =
			    strcmp(valuePtr->pv.buffer, value2.pv.buffer) == 0;
		}
		valuePtr->type = TYPE_LLONG;
		break;
	    case NEQ:
		if (valuePtr->type == TYPE_LLONG) {
		    valuePtr->llongValue =
			valuePtr->llongValue != value2.llongValue;
		} else if (valuePtr->type == TYPE_DOUBLE) {
		    valuePtr->llongValue =
			valuePtr->doubleValue != value2.doubleValue;
		} else {
		    valuePtr->llongValue =
			    strcmp(valuePtr->pv.buffer, value2.pv.buffer) != 0;
		}
		valuePtr->type = TYPE_LLONG;
		break;
	    case BIT_AND:
		valuePtr->llongValue &= value2.llongValue;
		break;
	    case BIT_XOR:
		valuePtr->llongValue ^= value2.llongValue;
		break;
	    case BIT_OR:
		valuePtr->llongValue |= value2.llongValue;
		break;

	    /*
	     * For AND and OR, we know that the first value has already
	     * been converted to an integer.  Thus we need only consider
	     * the possibility of int vs. double for the second value.
	     */

	    case AND:
		if (value2.type == TYPE_DOUBLE) {
		    value2.llongValue = value2.doubleValue != 0;
		    value2.type = TYPE_LLONG;
		}
		valuePtr->llongValue = valuePtr->llongValue && value2.llongValue;
		break;
	    case OR:
		if (value2.type == TYPE_DOUBLE) {
		    value2.llongValue = value2.doubleValue != 0;
		    value2.type = TYPE_LLONG;
		}
		valuePtr->llongValue = valuePtr->llongValue || value2.llongValue;
		break;

	    case COLON:
		interp->result =
		    (char *) "can't have : operator without ? first";
		result = HAX_ERROR;
		goto done;
	}
    }

    done:
    if (value2.pv.buffer != value2.staticSpace) {
	ckfree(value2.pv.buffer);
    }
    return result;

    syntaxError:
    Hax_ResetResult(interp);
    Hax_AppendResult(interp, "syntax error in expression \"",
	    infoPtr->originalExpr, "\"", (char *) NULL);
    result = HAX_ERROR;
    goto done;

    illegalType:
    Hax_AppendResult(interp, "can't use ", (badType == TYPE_DOUBLE) ?
	    "floating-point value" : "non-numeric string",
	    " as operand of \"", operatorStrings[op], "\"",
	    (char *) NULL);
    result = HAX_ERROR;
    goto done;
}

/*
 *--------------------------------------------------------------
 *
 * ExprMakeString --
 *
 *	Convert a value from int or long long int or double
 *	representation to a string.
 *
 * Results:
 *	The information at *valuePtr gets converted to string
 *	format, if it wasn't that way already.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

static void
ExprMakeString(
    Value *valuePtr		/* Value to be converted. */)
{
    int shortfall;

    shortfall = 150 - (valuePtr->pv.end - valuePtr->pv.buffer);
    if (shortfall > 0) {
	(*valuePtr->pv.expandProc)(&valuePtr->pv, shortfall);
    }
    if (valuePtr->type == TYPE_LLONG) {
	sprintf(valuePtr->pv.buffer, "%lld", valuePtr->llongValue);
    } else if (valuePtr->type == TYPE_DOUBLE) {
	sprintf(valuePtr->pv.buffer, "%g", valuePtr->doubleValue);
    }
    valuePtr->type = TYPE_STRING;
}

/*
 *--------------------------------------------------------------
 *
 * ExprTopLevel --
 *
 *	This procedure provides top-level functionality shared by
 *	procedures like Hax_ExprInt, Hax_ExprDouble, etc.
 *
 * Results:
 *	The result is a standard Hax return value.  If an error
 *	occurs then an error message is left in interp->result.
 *	The value of the expression is returned in *valuePtr, in
 *	whatever form it ends up in (could be string or integer
 *	or double).  Caller may need to convert result.  Caller
 *	is also responsible for freeing string memory in *valuePtr,
 *	if any was allocated.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

static int
ExprTopLevel(
    Hax_Interp *interp,			/* Context in which to evaluate the
					 * expression. */
    char *string,			/* Expression to evaluate. */
    Value *valuePtr			/* Where to store result.  Should
					 * not be initialized by caller. */)
{
    ExprInfo info;
    int result;

    info.originalExpr = string;
    info.expr = string;
    valuePtr->pv.buffer = valuePtr->pv.next = valuePtr->staticSpace;
    valuePtr->pv.end = valuePtr->pv.buffer + STATIC_STRING_SPACE - 1;
    valuePtr->pv.expandProc = HaxExpandParseValue;
    valuePtr->pv.clientData = (ClientData) NULL;

    result = ExprGetValue(interp, &info, -1, valuePtr);
    if (result != HAX_OK) {
	return result;
    }
    if (info.token != END) {
	Hax_AppendResult(interp, "syntax error in expression \"",
		string, "\"", (char *) NULL);
	return HAX_ERROR;
    }
    return HAX_OK;
}

/*
 *--------------------------------------------------------------
 *
 * Hax_ExprLong, Hax_ExprLongLong, Hax_ExprDouble,
 * Hax_ExprBoolean --
 *
 *	Procedures to evaluate an expression and return its value
 *	in a particular form.
 *
 * Results:
 *	Each of the procedures below returns a standard Hax result.
 *	If an error occurs then an error message is left in
 *	interp->result.  Otherwise the value of the expression,
 *	in the appropriate form, is stored at *resultPtr.  If
 *	the expression had a result that was incompatible with the
 *	desired form then an error is returned.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

int
Hax_ExprLong(
    Hax_Interp *interp,			/* Context in which to evaluate the
					 * expression. */
    char *string,			/* Expression to evaluate. */
    long *ptr				/* Where to store result. */)
{
    Value value;
    int result;

    result = ExprTopLevel(interp, string, &value);
    if (result == HAX_OK) {
	if (value.type == TYPE_LLONG) {
	    *ptr = value.llongValue;
	} else if (value.type == TYPE_DOUBLE) {
	    *ptr = value.doubleValue;
	} else {
	    interp->result = (char *) "expression didn't have numeric value";
	    result = HAX_ERROR;
	}
    }
    if (value.pv.buffer != value.staticSpace) {
	ckfree(value.pv.buffer);
    }
    return result;
}

int
Hax_ExprLongLong(
    Hax_Interp *interp,			/* Context in which to evaluate the
					 * expression. */
    char *string,			/* Expression to evaluate. */
    long long int *ptr			/* Where to store result. */)
{
    Value value;
    int result;

    result = ExprTopLevel(interp, string, &value);
    if (result == HAX_OK) {
	if (value.type == TYPE_LLONG) {
	    *ptr = value.llongValue;
	} else if (value.type == TYPE_DOUBLE) {
	    *ptr = value.doubleValue;
	} else {
	    interp->result = (char *) "expression didn't have numeric value";
	    result = HAX_ERROR;
	}
    }
    if (value.pv.buffer != value.staticSpace) {
	ckfree(value.pv.buffer);
    }
    return result;
}

int
Hax_ExprDouble(
    Hax_Interp *interp,			/* Context in which to evaluate the
					 * expression. */
    char *string,			/* Expression to evaluate. */
    void *ptr				/* Where to store result. */)
{
    Value value;
    int result;

    result = ExprTopLevel(interp, string, &value);
    if (result == HAX_OK) {
	if (value.type == TYPE_LLONG) {
	    *(double *)ptr = value.llongValue;
	} else if (value.type == TYPE_DOUBLE) {
	    *(double *)ptr = value.doubleValue;
	} else {
	    interp->result = (char *) "expression didn't have numeric value";
	    result = HAX_ERROR;
	}
    }
    if (value.pv.buffer != value.staticSpace) {
	ckfree(value.pv.buffer);
    }
    return result;
}

int
Hax_ExprBoolean(
    Hax_Interp *interp,			/* Context in which to evaluate the
					 * expression. */
    char *string,			/* Expression to evaluate. */
    int *ptr				/* Where to store 0/1 result. */)
{
    Value value;
    int result;

    result = ExprTopLevel(interp, string, &value);
    if (result == HAX_OK) {
	if (value.type == TYPE_LLONG) {
	    *ptr = value.llongValue != 0;
	} else if (value.type == TYPE_DOUBLE) {
	    *ptr = value.doubleValue != 0.0;
	} else {
	    interp->result = (char *) "expression didn't have numeric value";
	    result = HAX_ERROR;
	}
    }
    if (value.pv.buffer != value.staticSpace) {
	ckfree(value.pv.buffer);
    }
    return result;
}

/*
 *--------------------------------------------------------------
 *
 * Hax_ExprString --
 *
 *	Evaluate an expression and return its value in string form.
 *
 * Results:
 *	A standard Hax result.  If the result is HAX_OK, then the
 *	interpreter's result is set to the string value of the
 *	expression.  If the result is HAX_OK, then interp->result
 *	contains an error message.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

int
Hax_ExprString(
    Hax_Interp *interp,			/* Context in which to evaluate the
					 * expression. */
    char *string			/* Expression to evaluate. */)
{
    Value value;
    int result;

    result = ExprTopLevel(interp, string, &value);
    if (result == HAX_OK) {
	if (value.type == TYPE_LLONG) {
	    sprintf(interp->result, "%lld", value.llongValue);
	} else if (value.type == TYPE_DOUBLE) {
	    sprintf(interp->result, "%g", value.doubleValue);
	} else {
	    if (value.pv.buffer != value.staticSpace) {
		interp->result = value.pv.buffer;
		interp->freeProc = (Hax_FreeProc *) free;
		value.pv.buffer = value.staticSpace;
	    } else {
		Hax_SetResult(interp, value.pv.buffer, HAX_VOLATILE);
	    }
	}
    }
    if (value.pv.buffer != value.staticSpace) {
	ckfree(value.pv.buffer);
    }
    return result;
}
