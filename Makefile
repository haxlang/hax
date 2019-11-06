#
# This Makefile is for use when distributing Hax to the outside world.
#
# Some changes you may wish to make here:
#
# 1. To compile for non-UNIX systems (so that only the non-UNIX-specific
# commands are available), change the OBJS line below so it doesn't
# include ${UNIX_OBJS}.  Also, add the switch "-DTCL_GENERIC_ONLY" to
# CFLAGS.  Lastly, you'll have to provide your own replacement for the
# "panic" procedure (see panic.c for what the current one does).

# 2. If you want to put Hax-related information in non-standard places,
# change the following definitions below to reflect where you want
# things (all must be specified as full rooted path names):
#
#    PREFIX		Top-level directory in which to install;  contains
#			each of the other directories below.
#    TCL_LIBRARY	Directory in which to install the library of Tcl
#			scripts.  Note: if the TCL_LIBRARY environment
#			variable is specified at run-time then Tcl looks
#			there rather than in the place specified here.
#    LIB_DIR		Directory in which to install the archive libtcl.a
#    INCLUDE_DIR	Directory in which to install include files.
#    MAN_DIR		Directory in which to install manual pages.
#    MAN3_DIR		Directory in which to install manual entries for
#			library procedures such as Tcl_Eval.
#    MANN_DIR		Directory in which to install manual entries for
#			miscellaneous things such as the Tcl overview
#			manual entry.
#

# 3. If you want to alter the default build programs and flags, change:
#
#    CC			Path to cc(1).
#    AR			Path to ar(1).
#    RANLIB		Path to ranlib(1).
#    RANLIB		Path to ranlib(1).
#
#    CFLAGS		Flags passed to a compiler.
#    LDFLAGS		Flags passed to a linker.
#

.POSIX:
.SUFFIXES:
AR	= ar
RANLIB	= ranlib
CC	= cc
CFLAGS	=
LDFLAGS	=

PREFIX ?=	/usr/local
TCL_LIBRARY ?=	lib/tcl
LIB_DIR ?=	lib
INCLUDE_DIR ?=	include
MAN_DIR ?=	man
MAN3_DIR ?=	$(MAN_DIR)/man3
MANN_DIR ?=	$(MAN_DIR)/mann

all: libtcl.a

GENERIC_OBJS =	regexp.o tclAssem.o tclBasic.o tclCkalloc.o \
	tclCmdAH.o tclCmdIL.o tclCmdMZ.o tclExpr.o tclGet.o \
	tclHash.o tclHistory.o tclParse.o tclProc.o tclUtil.o \
	tclVar.o

UNIX_OBJS = panic.o tclEnv.o tclGlob.o tclUnixAZ.o tclUnixStr.o \
	tclUnixUtil.o 

COMPAT_OBJS =

OBJS = $(GENERIC_OBJS) $(UNIX_OBJS) $(COMPAT_OBJS)

libtcl.a: $(OBJS)
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

tclTest: tclTest.o libtcl.a
	$(CC) $(LDFLAGS) -o $@ tclTest.o libtcl.a

install: libtcl.a
	install -d $(DESTDIR)$(PREFIX)/$(LIB_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(TCL_LIBRARY)
	install -d $(DESTDIR)$(PREFIX)/$(INCLUDE_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(MAN3_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(MANN_DIR)

	cd library; for i in *.tcl; do \
		install $$i $(DESTDIR)$(PREFIX)/$(TCL_LIBRARY); \
	done

	install libtcl.a $(DESTDIR)$(PREFIX)/$(LIB_DIR)

	install tcl.h $(DESTDIR)$(PREFIX)/$(INCLUDE_DIR)
	install tclHash.h $(DESTDIR)$(PREFIX)/$(INCLUDE_DIR)

	cd doc; for i in *.3; do \
		sed -e '/man\.macros/r man.macros' -e '/man\.macros/d' \
			$$i > $$i.tmp; \
		install $$i.tmp $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i; \
		rm -f $$i.tmp; \
	done

	cd doc; for i in *.n; do \
		sed -e '/man\.macros/r man.macros' -e '/man\.macros/d' \
			$$i > $$i.tmp; \
		install $$i.tmp $(DESTDIR)$(PREFIX)/$(MANN_DIR)/$$i; \
		rm -f $$i.tmp; \
	done

test: tclTest
	( echo cd tests ; echo source all ) | ./tclTest

clean:
	rm -f $(OBJS) libtcl.a tclTest.o tclTest

$(OBJS): tcl.h tclHash.h tclInt.h
$(UNIX_OJBS): tclUnix.h

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -I. -DTCL_LIBRARY=\"$(PREFIX)/$(TCL_LIBRARY)\" -c $<
