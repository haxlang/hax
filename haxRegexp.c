/*
 * RegComp and RegExec -- RegSub and RegError are elsewhere
 *
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 *
 * Beware that some of this code is subtly aware of the way operator
 * precedence is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink.
 *
 * *** NOTE: this code has been altered slightly for use in Tcl. ***
 * *** The only change is to use ckalloc and ckfree instead of   ***
 * *** malloc and free.						 ***
 *
 * 2019-11-07 Remove another compat definition of strcspn()
 * 2019-11-07 Switch to ANSI function definitions
 * 2019-11-07 Change FAIL() to standard do {} while(0) form
 * 2019-11-07 Remove superfluous break in switch()
 * 2019-11-07 Rename regex functions to disable clash with libc
 * 2019-11-08 Correct build issued with a C++ compiler (g++, clang++)
 * 2019-11-14 Convert libc calls from symbol() to Hax_symbol()
 */
#include "haxInt.h"

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart	char that must begin a match; '\0' if none obvious
 * reganch	is the match anchored (at beginning-of-line only)?
 * regmust	string (pointer into program) that match must include, or NULL
 * regmlen	length of regmust string
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that RegComp() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in RegExec() needs it and RegComp() is computing
 * it anyway.
 */

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are:
 */

/* definition	number	opnd?	meaning */
#define	END	0	/* no	End of program. */
#define	BOL	1	/* no	Match "" at beginning of line. */
#define	EOL	2	/* no	Match "" at end of line. */
#define	ANY	3	/* no	Match any one character. */
#define	ANYOF	4	/* str	Match any character in this string. */
#define	ANYBUT	5	/* str	Match any character not in this string. */
#define	BRANCH	6	/* node	Match this alternative, or the next... */
#define	BACK	7	/* no	Match "", "next" ptr points backward. */
#define	EXACTLY	8	/* str	Match this string. */
#define	NOTHING	9	/* no	Match empty string. */
#define	STAR	10	/* node	Match this (simple) thing 0 or more times. */
#define	PLUS	11	/* node	Match this (simple) thing 1 or more times. */
#define	OPEN	20	/* no	Mark this point in input as start of #n. */
			/*	OPEN+1 is number 1, etc. */
#define	CLOSE	30	/* no	Analogous to OPEN. */

/*
 * Opcode notes:
 *
 * BRANCH	The set of branches constituting a single choice are hooked
 *		together with their "next" pointers, since precedence prevents
 *		anything being concatenated to any individual branch.  The
 *		"next" pointer of the last BRANCH in a choice points to the
 *		thing following the whole choice.  This is also where the
 *		final "next" pointer of each individual branch points; each
 *		branch starts with the operand node of a BRANCH node.
 *
 * BACK		Normal "next" pointers all implicitly point forward; BACK
 *		exists to make loop structures possible.
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 *		BRANCH structures using BACK.  Simple cases (one character
 *		per match) are implemented with STAR and PLUS for speed
 *		and to minimize recursive plunges.
 *
 * OPEN,CLOSE	...are numbered at compile time.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */
#define	OP(p)	(*(p))
#define	NEXT(p)	(((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
#define	OPERAND(p)	((p) + 3)

/*
 * See regmagic.h for one further detail of program structure.
 */


/*
 * Utility definitions.
 */
#ifndef CHARBITS
#define	UCHARAT(p)	((int)*(unsigned char *)(p))
#else
#define	UCHARAT(p)	((int)*(p)&CHARBITS)
#endif

#define	FAIL(m)	do { RegError(m); return(NULL); } while (0)
#define	ISMULT(c)	((c) == '*' || (c) == '+' || (c) == '?')
#define	META	"^$.[()|?+*\\"

/*
 * Flags to be passed up and down.
 */
#define	HASWIDTH	01	/* Known never to match null string. */
#define	SIMPLE		02	/* Simple enough to be STAR/PLUS operand. */
#define	SPSTART		04	/* Starts with * or +. */
#define	WORST		0	/* Worst case. */

/*
 * Global work variables for Regcomp().
 */
static char *regparse;		/* Input-scan pointer. */
static int regnpar;		/* () count. */
static char regdummy;
static char *regcode;		/* Code-emit pointer; &regdummy = don't. */
static long regsize;		/* Code size. */

/*
 * The first byte of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */
#define	MAGIC	0234


/*
 * Forward declarations for Regcomp()'s friends.
 */
static char *Reg(int paren, int *flagp);
static char *RegBranch(int *flagp);
static char *RegPiece(int *flagp);
static char *RegAtom(int *flagp);
static char *RegNode(char op);
static char *RegNext(char *p);
static void RegC(char b);
static void RegInsert(char op, char *opnd);
static void RegTail(char *p, char *val);
static void RegOpTail(char *p, char *val);

/*
 - Regcomp - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because free() must be able to free it all.)
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.
 */
regexp *
RegComp(char *exp)
{
	regexp *r;
	char *scan;
	char *longest;
	int len;
	int flags;

	if (exp == NULL)
		FAIL((char *) "NULL argument");

	/* First pass: determine size, legality. */
	regparse = exp;
	regnpar = 1;
	regsize = 0L;
	regcode = &regdummy;
	RegC(MAGIC);
	if (Reg(0, &flags) == NULL)
		return(NULL);

	/* Small enough for pointer-storage convention? */
	if (regsize >= 32767L)		/* Probably could be 65535L. */
		FAIL((char *) "regexp too big");

	/* Allocate space. */
	r = (regexp *)ckalloc(sizeof(regexp) + (unsigned)regsize);
	if (r == NULL)
		FAIL((char *) "out of space");

	/* Second pass: emit code. */
	regparse = exp;
	regnpar = 1;
	regcode = r->program;
	RegC(MAGIC);
	if (Reg(0, &flags) == NULL)
		return(NULL);

	/* Dig out information for optimizations. */
	r->regstart = '\0';	/* Worst-case defaults. */
	r->reganch = 0;
	r->regmust = NULL;
	r->regmlen = 0;
	scan = r->program+1;			/* First BRANCH. */
	if (OP(RegNext(scan)) == END) {		/* Only one top-level choice. */
		scan = OPERAND(scan);

		/* Starting-point info. */
		if (OP(scan) == EXACTLY)
			r->regstart = *OPERAND(scan);
		else if (OP(scan) == BOL)
			r->reganch++;

		/*
		 * If there's something expensive in the r.e., find the
		 * longest literal string that must appear and make it the
		 * regmust.  Resolve ties in favor of later strings, since
		 * the regstart check works with the beginning of the r.e.
		 * and avoiding duplication strengthens checking.  Not a
		 * strong reason, but sufficient in the absence of others.
		 */
		if (flags&SPSTART) {
			longest = NULL;
			len = 0;
			for (; scan != NULL; scan = RegNext(scan))
				if (OP(scan) == EXACTLY && Hax_strlen(OPERAND(scan)) >= len) {
					longest = OPERAND(scan);
					len = Hax_strlen(OPERAND(scan));
				}
			r->regmust = longest;
			r->regmlen = len;
		}
	}

	return(r);
}

/*
 - Reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */
static char *
Reg(
int paren,			/* Parenthesized? */
int *flagp)
{
	char *ret;
	char *br;
	char *ender;
	int parno = 0;
	int flags;

	*flagp = HASWIDTH;	/* Tentatively. */

	/* Make an OPEN node, if parenthesized. */
	if (paren) {
		if (regnpar >= NSUBEXP)
			FAIL((char *) "too many ()");
		parno = regnpar;
		regnpar++;
		ret = RegNode(OPEN+parno);
	} else
		ret = NULL;

	/* Pick up the branches, linking them together. */
	br = RegBranch(&flags);
	if (br == NULL)
		return(NULL);
	if (ret != NULL)
		RegTail(ret, br);	/* OPEN -> first. */
	else
		ret = br;
	if (!(flags&HASWIDTH))
		*flagp &= ~HASWIDTH;
	*flagp |= flags&SPSTART;
	while (*regparse == '|') {
		regparse++;
		br = RegBranch(&flags);
		if (br == NULL)
			return(NULL);
		RegTail(ret, br);	/* BRANCH -> BRANCH. */
		if (!(flags&HASWIDTH))
			*flagp &= ~HASWIDTH;
		*flagp |= flags&SPSTART;
	}

	/* Make a closing node, and hook it on the end. */
	ender = RegNode((paren) ? CLOSE+parno : END);
	RegTail(ret, ender);

	/* Hook the tails of the branches to the closing node. */
	for (br = ret; br != NULL; br = RegNext(br))
		RegOpTail(br, ender);

	/* Check for proper termination. */
	if (paren && *regparse++ != ')') {
		FAIL((char *) "unmatched ()");
	} else if (!paren && *regparse != '\0') {
		if (*regparse == ')') {
			FAIL((char *) "unmatched ()");
		} else
			FAIL((char *) "junk on end");	/* "Can't happen". */
		/* NOTREACHED */
	}

	return(ret);
}

/*
 - RegBranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
static char *
RegBranch(
int *flagp)
{
	char *ret;
	char *chain;
	char *latest;
	int flags;

	*flagp = WORST;		/* Tentatively. */

	ret = RegNode(BRANCH);
	chain = NULL;
	while (*regparse != '\0' && *regparse != '|' && *regparse != ')') {
		latest = RegPiece(&flags);
		if (latest == NULL)
			return(NULL);
		*flagp |= flags&HASWIDTH;
		if (chain == NULL)	/* First piece. */
			*flagp |= flags&SPSTART;
		else
			RegTail(chain, latest);
		chain = latest;
	}
	if (chain == NULL)	/* Loop ran zero times. */
		(void) RegNode(NOTHING);

	return(ret);
}

/*
 - RegPiece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */
static char *
RegPiece(
int *flagp)
{
	char *ret;
	char op;
	char *next;
	int flags;

	ret = RegAtom(&flags);
	if (ret == NULL)
		return(NULL);

	op = *regparse;
	if (!ISMULT(op)) {
		*flagp = flags;
		return(ret);
	}

	if (!(flags&HASWIDTH) && op != '?')
		FAIL((char *) "*+ operand could be empty");
	*flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);

	if (op == '*' && (flags&SIMPLE))
		RegInsert(STAR, ret);
	else if (op == '*') {
		/* Emit x* as (x&|), where & means "self". */
		RegInsert(BRANCH, ret);			/* Either x */
		RegOpTail(ret, RegNode(BACK));		/* and loop */
		RegOpTail(ret, ret);			/* back */
		RegTail(ret, RegNode(BRANCH));		/* or */
		RegTail(ret, RegNode(NOTHING));		/* null. */
	} else if (op == '+' && (flags&SIMPLE))
		RegInsert(PLUS, ret);
	else if (op == '+') {
		/* Emit x+ as x(&|), where & means "self". */
		next = RegNode(BRANCH);			/* Either */
		RegTail(ret, next);
		RegTail(RegNode(BACK), ret);		/* loop back */
		RegTail(next, RegNode(BRANCH));		/* or */
		RegTail(ret, RegNode(NOTHING));		/* null. */
	} else if (op == '?') {
		/* Emit x? as (x|) */
		RegInsert(BRANCH, ret);			/* Either x */
		RegTail(ret, RegNode(BRANCH));		/* or */
		next = RegNode(NOTHING);		/* null. */
		RegTail(ret, next);
		RegOpTail(ret, next);
	}
	regparse++;
	if (ISMULT(*regparse))
		FAIL((char *) "nested *?+");

	return(ret);
}

/*
 - RegAtom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.  Backslashed characters are exceptions, each becoming a
 * separate node; the code is simpler that way and it's not worth fixing.
 */
static char *
RegAtom(
int *flagp)
{
	char *ret;
	int flags;

	*flagp = WORST;		/* Tentatively. */

	switch (*regparse++) {
	case '^':
		ret = RegNode(BOL);
		break;
	case '$':
		ret = RegNode(EOL);
		break;
	case '.':
		ret = RegNode(ANY);
		*flagp |= HASWIDTH|SIMPLE;
		break;
	case '[': {
			int clss;
			int classend;

			if (*regparse == '^') {	/* Complement of range. */
				ret = RegNode(ANYBUT);
				regparse++;
			} else
				ret = RegNode(ANYOF);
			if (*regparse == ']' || *regparse == '-')
				RegC(*regparse++);
			while (*regparse != '\0' && *regparse != ']') {
				if (*regparse == '-') {
					regparse++;
					if (*regparse == ']' || *regparse == '\0')
						RegC('-');
					else {
						clss = UCHARAT(regparse-2)+1;
						classend = UCHARAT(regparse);
						if (clss > classend+1)
							FAIL((char *) "invalid [] range");
						for (; clss <= classend; clss++)
							RegC(clss);
						regparse++;
					}
				} else
					RegC(*regparse++);
			}
			RegC('\0');
			if (*regparse != ']')
				FAIL((char *) "unmatched []");
			regparse++;
			*flagp |= HASWIDTH|SIMPLE;
		}
		break;
	case '(':
		ret = Reg(1, &flags);
		if (ret == NULL)
			return(NULL);
		*flagp |= flags&(HASWIDTH|SPSTART);
		break;
	case '\0':
	case '|':
	case ')':
		FAIL((char *) "internal urp");	/* Supposed to be caught earlier. */
		/* NOTREACHED */
		break;
	case '?':
	case '+':
	case '*':
		FAIL((char *) "?+* follows nothing");
		/* NOTREACHED */
		break;
	case '\\':
		if (*regparse == '\0')
			FAIL((char *) "trailing \\");
		ret = RegNode(EXACTLY);
		RegC(*regparse++);
		RegC('\0');
		*flagp |= HASWIDTH|SIMPLE;
		break;
	default: {
			int len;
			char ender;

			regparse--;
			len = Hax_strcspn(regparse, META);
			if (len <= 0)
				FAIL((char *) "internal disaster");
			ender = *(regparse+len);
			if (len > 1 && ISMULT(ender))
				len--;		/* Back off clear of ?+* operand. */
			*flagp |= HASWIDTH;
			if (len == 1)
				*flagp |= SIMPLE;
			ret = RegNode(EXACTLY);
			while (len > 0) {
				RegC(*regparse++);
				len--;
			}
			RegC('\0');
		}
		break;
	}

	return(ret);
}

/*
 - RegNode - emit a node
 */
static char *			/* Location. */
RegNode(
char op)
{
	char *ret;
	char *ptr;

	ret = regcode;
	if (ret == &regdummy) {
		regsize += 3;
		return(ret);
	}

	ptr = ret;
	*ptr++ = op;
	*ptr++ = '\0';		/* Null "next" pointer. */
	*ptr++ = '\0';
	regcode = ptr;

	return(ret);
}

/*
 - Regc - emit (if appropriate) a byte of code
 */
static void
RegC(
char b)
{
	if (regcode != &regdummy)
		*regcode++ = b;
	else
		regsize++;
}

/*
 - reginsert - insert an operator in front of already-emitted operand
 *
 * Means relocating the operand.
 */
static void
RegInsert(
char op,
char *opnd)
{
	char *src;
	char *dst;
	char *place;

	if (regcode == &regdummy) {
		regsize += 3;
		return;
	}

	src = regcode;
	regcode += 3;
	dst = regcode;
	while (src > opnd)
		*--dst = *--src;

	place = opnd;		/* Op node, where operand used to be. */
	*place++ = op;
	*place++ = '\0';
	*place++ = '\0';
}

/*
 - regtail - set the next-pointer at the end of a node chain
 */
static void
RegTail(
char *p,
char *val)
{
	char *scan;
	char *temp;
	int offset;

	if (p == &regdummy)
		return;

	/* Find last node. */
	scan = p;
	for (;;) {
		temp = RegNext(scan);
		if (temp == NULL)
			break;
		scan = temp;
	}

	if (OP(scan) == BACK)
		offset = scan - val;
	else
		offset = val - scan;
	*(scan+1) = (offset>>8)&0377;
	*(scan+2) = offset&0377;
}

/*
 - RegOpTail - regtail on operand of first argument; nop if operandless
 */
static void
RegOpTail(
char *p,
char *val)
{
	/* "Operandless" and "op != BRANCH" are synonymous in practice. */
	if (p == NULL || p == &regdummy || OP(p) != BRANCH)
		return;
	RegTail(OPERAND(p), val);
}

/*
 * RegExec and friends
 */

/*
 * Global work variables for RegExec().
 */
static char *reginput;		/* String-input pointer. */
static char *regbol;		/* Beginning of input, for ^ check. */
static char **regstartp;	/* Pointer to startp array. */
static char **regendp;		/* Ditto for endp. */

/*
 * Forwards.
 */
static int RegTry(regexp *prog, char *string);
static int RegMatch(char *prog);
static int RegRepeat(char *p);

#ifdef DEBUG
int regnarrate = 0;
void RegDump(regexp *r);
static char *RegProp(char *op);
#endif

/*
 - Regexec - match a regexp against a string
 */
int
RegExec(
regexp *prog,
char *string)
{
	char *s;

	/* Be paranoid... */
	if (prog == NULL || string == NULL) {
		RegError((char *) "NULL parameter");
		return(0);
	}

	/* Check validity of program. */
	if (UCHARAT(prog->program) != MAGIC) {
		RegError((char *) "corrupted program");
		return(0);
	}

	/* If there is a "must appear" string, look for it. */
	if (prog->regmust != NULL) {
		s = string;
		while ((s = Hax_strchr(s, prog->regmust[0])) != NULL) {
			if (Hax_strncmp(s, prog->regmust, prog->regmlen) == 0)
				break;	/* Found it. */
			s++;
		}
		if (s == NULL)	/* Not present. */
			return(0);
	}

	/* Mark beginning of line for ^ . */
	regbol = string;

	/* Simplest case:  anchored match need be tried only once. */
	if (prog->reganch)
		return(RegTry(prog, string));

	/* Messy cases:  unanchored match. */
	s = string;
	if (prog->regstart != '\0')
		/* We know what char it must start with. */
		while ((s = Hax_strchr(s, prog->regstart)) != NULL) {
			if (RegTry(prog, s))
				return(1);
			s++;
		}
	else
		/* We don't -- general case. */
		do {
			if (RegTry(prog, s))
				return(1);
		} while (*s++ != '\0');

	/* Failure. */
	return(0);
}

/*
 - RegTry - try match at specific point
 */
static int			/* 0 failure, 1 success */
RegTry(
regexp *prog,
char *string)
{
	int i;
	char **sp;
	char **ep;

	reginput = string;
	regstartp = prog->startp;
	regendp = prog->endp;

	sp = prog->startp;
	ep = prog->endp;
	for (i = NSUBEXP; i > 0; i--) {
		*sp++ = NULL;
		*ep++ = NULL;
	}
	if (RegMatch(prog->program + 1)) {
		prog->startp[0] = string;
		prog->endp[0] = reginput;
		return(1);
	} else
		return(0);
}

/*
 - RegMatch - main matching routine
 *
 * Conceptually the strategy is simple:  check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  In practice we make some effort to avoid
 * recursion, in particular by going through "ordinary" nodes (that don't
 * need to know whether the rest of the match failed) by a loop instead of
 * by recursion.
 */
static int			/* 0 failure, 1 success */
RegMatch(
char *prog)
{
	char *scan;	/* Current node. */
	char *next;		/* Next node. */

	scan = prog;
#ifdef DEBUG
	if (scan != NULL && regnarrate)
		fprintf(stderr, "%s(\n", Regprop(scan));
#endif
	while (scan != NULL) {
#ifdef DEBUG
		if (regnarrate)
			fprintf(stderr, "%s...\n", Regprop(scan));
#endif
		next = RegNext(scan);

		switch (OP(scan)) {
		case BOL:
			if (reginput != regbol)
				return(0);
			break;
		case EOL:
			if (*reginput != '\0')
				return(0);
			break;
		case ANY:
			if (*reginput == '\0')
				return(0);
			reginput++;
			break;
		case EXACTLY: {
				int len;
				char *opnd;

				opnd = OPERAND(scan);
				/* Inline the first character, for speed. */
				if (*opnd != *reginput)
					return(0);
				len = Hax_strlen(opnd);
				if (len > 1 && Hax_strncmp(opnd, reginput, len) != 0)
					return(0);
				reginput += len;
			}
			break;
		case ANYOF:
 			if (*reginput == '\0' || Hax_strchr(OPERAND(scan), *reginput) == NULL)
				return(0);
			reginput++;
			break;
		case ANYBUT:
 			if (*reginput == '\0' || Hax_strchr(OPERAND(scan), *reginput) != NULL)
				return(0);
			reginput++;
			break;
		case NOTHING:
			break;
		case BACK:
			break;
		case OPEN+1:
		case OPEN+2:
		case OPEN+3:
		case OPEN+4:
		case OPEN+5:
		case OPEN+6:
		case OPEN+7:
		case OPEN+8:
		case OPEN+9: {
				int no;
				char *save;

				no = OP(scan) - OPEN;
				save = reginput;

				if (RegMatch(next)) {
					/*
					 * Don't set startp if some later
					 * invocation of the same parentheses
					 * already has.
					 */
					if (regstartp[no] == NULL)
						regstartp[no] = save;
					return(1);
				} else
					return(0);
			}
			/* NOTREACHED */
		case CLOSE+1:
		case CLOSE+2:
		case CLOSE+3:
		case CLOSE+4:
		case CLOSE+5:
		case CLOSE+6:
		case CLOSE+7:
		case CLOSE+8:
		case CLOSE+9: {
				int no;
				char *save;

				no = OP(scan) - CLOSE;
				save = reginput;

				if (RegMatch(next)) {
					/*
					 * Don't set endp if some later
					 * invocation of the same parentheses
					 * already has.
					 */
					if (regendp[no] == NULL)
						regendp[no] = save;
					return(1);
				} else
					return(0);
			}
			/* NOTREACHED */
		case BRANCH: {
				char *save;

				if (OP(next) != BRANCH)		/* No choice. */
					next = OPERAND(scan);	/* Avoid recursion. */
				else {
					do {
						save = reginput;
						if (RegMatch(OPERAND(scan)))
							return(1);
						reginput = save;
						scan = RegNext(scan);
					} while (scan != NULL && OP(scan) == BRANCH);
					return(0);
					/* NOTREACHED */
				}
			}
			/* NOTREACHED */
			break;
		case STAR:
		case PLUS: {
				char nextch;
				int no;
				char *save;
				int min;

				/*
				 * Lookahead to avoid useless match attempts
				 * when we know what character comes next.
				 */
				nextch = '\0';
				if (OP(next) == EXACTLY)
					nextch = *OPERAND(next);
				min = (OP(scan) == STAR) ? 0 : 1;
				save = reginput;
				no = RegRepeat(OPERAND(scan));
				while (no >= min) {
					/* If it could work, try it. */
					if (nextch == '\0' || *reginput == nextch)
						if (RegMatch(next))
							return(1);
					/* Couldn't or didn't -- back up. */
					no--;
					reginput = save + no;
				}
				return(0);
			}
			/* NOTREACHED */
		case END:
			return(1);	/* Success! */
		default:
			RegError((char *) "memory corruption");
			return(0);
		}

		scan = next;
	}

	/*
	 * We get here only if there's trouble -- normally "case END" is
	 * the terminating point.
	 */
	RegError((char *) "corrupted pointers");
	return(0);
}

/*
 - RegRepeat - repeatedly match something simple, report how many
 */
static int
RegRepeat(
char *p)
{
	int count = 0;
	char *scan;
	char *opnd;

	scan = reginput;
	opnd = OPERAND(p);
	switch (OP(p)) {
	case ANY:
		count = Hax_strlen(scan);
		scan += count;
		break;
	case EXACTLY:
		while (*opnd == *scan) {
			count++;
			scan++;
		}
		break;
	case ANYOF:
		while (*scan != '\0' && Hax_strchr(opnd, *scan) != NULL) {
			count++;
			scan++;
		}
		break;
	case ANYBUT:
		while (*scan != '\0' && Hax_strchr(opnd, *scan) == NULL) {
			count++;
			scan++;
		}
		break;
	default:		/* Oh dear.  Called inappropriately. */
		RegError((char *) "internal foulup");
		count = 0;	/* Best compromise. */
		break;
	}
	reginput = scan;

	return(count);
}

/*
 - RegNext - dig the "next" pointer out of a node
 */
static char *
RegNext(
char *p)
{
	int offset;

	if (p == &regdummy)
		return(NULL);

	offset = NEXT(p);
	if (offset == 0)
		return(NULL);

	if (OP(p) == BACK)
		return(p-offset);
	else
		return(p+offset);
}

#ifdef DEBUG

static char *RegProp(char *op);

/*
 - Regdump - dump a regexp onto stdout in vaguely comprehensible form
 */
void
RegDump(
regexp *r)
{
	char *s;
	char op = EXACTLY;	/* Arbitrary non-END op. */
	char *next;


	s = r->program + 1;
	while (op != END) {	/* While that wasn't END last time... */
		op = OP(s);
		printf("%2d%s", s-r->program, Regprop(s));	/* Where, what. */
		next = RegNext(s);
		if (next == NULL)		/* Next ptr. */
			printf("(0)");
		else
			printf("(%d)", (s-r->program)+(next-s));
		s += 3;
		if (op == ANYOF || op == ANYBUT || op == EXACTLY) {
			/* Literal string, where present. */
			while (*s != '\0') {
				putchar(*s);
				s++;
			}
			s++;
		}
		putchar('\n');
	}

	/* Header fields of interest. */
	if (r->regstart != '\0')
		printf("start `%c' ", r->regstart);
	if (r->reganch)
		printf("anchored ");
	if (r->regmust != NULL)
		printf("must have \"%s\"", r->regmust);
	printf("\n");
}

/*
 - Regprop - printable representation of opcode
 */
static char *
RegProp(
char *op)
{
	char *p;
	static char buf[50];

	(void) Hax_strcpy(buf, ":");

	switch (OP(op)) {
	case BOL:
		p = "BOL";
		break;
	case EOL:
		p = "EOL";
		break;
	case ANY:
		p = "ANY";
		break;
	case ANYOF:
		p = "ANYOF";
		break;
	case ANYBUT:
		p = "ANYBUT";
		break;
	case BRANCH:
		p = "BRANCH";
		break;
	case EXACTLY:
		p = "EXACTLY";
		break;
	case NOTHING:
		p = "NOTHING";
		break;
	case BACK:
		p = "BACK";
		break;
	case END:
		p = "END";
		break;
	case OPEN+1:
	case OPEN+2:
	case OPEN+3:
	case OPEN+4:
	case OPEN+5:
	case OPEN+6:
	case OPEN+7:
	case OPEN+8:
	case OPEN+9:
		Hax_sprintf(buf+Hax_strlen(buf), "OPEN%d", OP(op)-OPEN);
		p = NULL;
		break;
	case CLOSE+1:
	case CLOSE+2:
	case CLOSE+3:
	case CLOSE+4:
	case CLOSE+5:
	case CLOSE+6:
	case CLOSE+7:
	case CLOSE+8:
	case CLOSE+9:
		Hax_sprintf(buf+Hax_strlen(buf), "CLOSE%d", OP(op)-CLOSE);
		p = NULL;
		break;
	case STAR:
		p = "STAR";
		break;
	case PLUS:
		p = "PLUS";
		break;
	default:
		RegError((char *) "corrupted opcode");
		break;
	}
	if (p != NULL)
		(void) Hax_strcat(buf, p);
	return(buf);
}
#endif
