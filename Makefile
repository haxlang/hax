#
# This Makefile is for use when distributing Hax to the outside world.
#
# Some changes you may wish to make here:
#
# 1. To compile for non-UNIX systems (so that only the non-UNIX-specific
# commands are available), change the OBJS line below so it doesn't
# include ${UNIX_OBJS}.  Also, add the switch "-DHAX_GENERIC_ONLY" to
# CFLAGS.  Lastly, you'll have to provide your own replacement for the
# "Hax_Panic" procedure (see haxPanic.c for what the current one does).

# 2. Add the switch "-DHAX_FREESTANDING" to build a standalone version
# of Hax.

# 3. If you want to put Hax-related information in non-standard places,
# change the following definitions below to reflect where you want
# things (all must be specified as full rooted path names):
#
#    PREFIX		Top-level directory in which to install;  contains
#			each of the other directories below.
#    HAX_LIBRARY	Directory in which to install the library of Hax
#			scripts.  Note: if the HAX_LIBRARY environment
#			variable is specified at run-time then Hax looks
#			there rather than in the place specified here.
#    LIB_DIR		Directory in which to install the archive libhax.a
#    INCLUDE_DIR	Directory in which to install include files.
#    MAN_DIR		Directory in which to install manual pages.
#    MAN3_DIR		Directory in which to install manual entries for
#			library procedures such as Hax_Eval.
#    MANN_DIR		Directory in which to install manual entries for
#			miscellaneous things such as the Hax overview
#			manual entry.
#

# 4. If you want to alter the default build programs and flags, change:
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
HAX_LIBRARY ?=	lib/hax
BIN_DIR ?=	bin
LIB_DIR ?=	lib
INCLUDE_DIR ?=	include
MAN_DIR ?=	man
MAN3_DIR ?=	$(MAN_DIR)/man3
MANN_DIR ?=	$(MAN_DIR)/mann

all: libhax.a haxsh

GENERIC_OBJS =	haxRegexp.o haxAssem.o haxBasic.o haxCkalloc.o \
	haxCmdAH.o haxCmdIL.o haxCmdMZ.o haxExpr.o haxGet.o \
	haxHash.o haxHistory.o haxParse.o haxProc.o haxUtil.o \
	haxVar.o haxCompat.o haxPanic.o

UNIX_OBJS = haxEnv.o haxGlob.o haxUnixAZ.o haxUnixStr.o haxUnixUtil.o

COMPAT_OBJS = compat/Hax_ctype_.o compat/Hax_tolower_.o compat/Hax_toupper_.o \
	compat/Hax_strcat.o compat/Hax_strchr.o compat/Hax_strcmp.o \
	compat/Hax_strcpy.o compat/Hax_strcspn.o compat/Hax_strspn.o \
	compat/Hax_strlen.o compat/Hax_strncpy.o compat/Hax_strstr.o \
	compat/Hax_strerror.o compat/Hax_strncmp.o compat/Hax_strrchr.o \
	compat/Hax_errlist.o compat/Hax_atoi.o compat/Hax_strtol.o \
	compat/Hax_qsort.o compat/Hax_strtoul.o compat/Hax_strtoll.o \
	compat/Hax_vsnprintf.o compat/Hax_vsprintf.o compat/Hax_vfprintf.o \
	compat/Hax_strnlen.o compat/Hax_signbitd_ieee754.o \
	compat/Hax_isfinited_ieee754.o compat/Hax_compat_frexp_ieee754.o \
	compat/Hax_errno.o compat/Hax_fwrite.o compat/Hax___towrite.o \
	compat/Hax_fwrite.o \
	compat/gdtoa/Hax_dmisc.o compat/gdtoa/Hax_dtoa.o \
	compat/gdtoa/Hax_g_Qfmt.o compat/gdtoa/Hax_g_Qfmt_p.o \
	compat/gdtoa/Hax_g__fmt.o compat/gdtoa/Hax_g_ddfmt.o \
	compat/gdtoa/Hax_g_ddfmt_p.o compat/gdtoa/Hax_g_dfmt.o \
	compat/gdtoa/Hax_g_dfmt_p.o compat/gdtoa/Hax_g_ffmt.o \
	compat/gdtoa/Hax_g_ffmt_p.o compat/gdtoa/Hax_g_xLfmt.o \
	compat/gdtoa/Hax_g_xLfmt_p.o compat/gdtoa/Hax_g_xfmt.o \
	compat/gdtoa/Hax_g_xfmt_p.o compat/gdtoa/Hax_gdtoa.o \
	compat/gdtoa/Hax_gethex.o compat/gdtoa/Hax_gmisc.o \
	compat/gdtoa/Hax_hd_init.o compat/gdtoa/Hax_hexnan.o \
	compat/gdtoa/Hax_misc.o compat/gdtoa/Hax_smisc.o \
	compat/gdtoa/Hax_strtoIQ.o compat/gdtoa/Hax_strtoId.o \
	compat/gdtoa/Hax_strtoIdd.o compat/gdtoa/Hax_strtoIf.o \
	compat/gdtoa/Hax_strtoIg.o compat/gdtoa/Hax_strtoIx.o \
	compat/gdtoa/Hax_strtoIxL.o compat/gdtoa/Hax_strtod.o \
	compat/gdtoa/Hax_strtodI.o compat/gdtoa/Hax_strtodg.o \
	compat/gdtoa/Hax_strtof.o compat/gdtoa/Hax_strtopQ.o \
	compat/gdtoa/Hax_strtopd.o compat/gdtoa/Hax_strtopdd.o \
	compat/gdtoa/Hax_strtopf.o compat/gdtoa/Hax_strtopx.o \
	compat/gdtoa/Hax_strtopxL.o compat/gdtoa/Hax_strtorQ.o \
	compat/gdtoa/Hax_strtord.o compat/gdtoa/Hax_strtordd.o \
	compat/gdtoa/Hax_strtorf.o compat/gdtoa/Hax_strtorx.o \
	compat/gdtoa/Hax_strtorxL.o compat/gdtoa/Hax_sum.o \
	compat/gdtoa/Hax_ulp.o \
	compat/Hax_malloc.o

compat/Hax_errlist.c: compat/errlist.awk
	awk -f compat/errlist.awk /usr/src/sys/sys/errno.h > $@

OBJS = $(GENERIC_OBJS) $(UNIX_OBJS)

libhax.a: $(OBJS)
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

haxsh: haxsh.o libhax.a
	$(CC) $(LDFLAGS) -o $@ haxsh.o libhax.a

install: libhax.a
	install -d $(DESTDIR)$(PREFIX)/$(BIN_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(LIB_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(HAX_LIBRARY)
	install -d $(DESTDIR)$(PREFIX)/$(INCLUDE_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(MAN3_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(MANN_DIR)

	install haxsh $(DESTDIR)$(PREFIX)/$(BIN_DIR)

	cd library; for i in *.tcl; do \
		install $$i $(DESTDIR)$(PREFIX)/$(HAX_LIBRARY); \
	done

	install libhax.a $(DESTDIR)$(PREFIX)/$(LIB_DIR)

	install hax.h $(DESTDIR)$(PREFIX)/$(INCLUDE_DIR)
	install haxHash.h $(DESTDIR)$(PREFIX)/$(INCLUDE_DIR)

	cd doc; for i in *.3; do \
		sed -e '/man\.macros/r man.macros' -e '/man\.macros/d' \
			$$i > Hax_$$i; \
		install Hax_$$i $(DESTDIR)$(PREFIX)/$(MAN3_DIR); \
		rm -f Hax_$$i; \
	done

	cd doc; \
		sed -e '/man\.macros/r man.macros' \
		    -e '/man\.macros/d' Hax.n > Hax.n.tmp; \
		install Hax.n.tmp $(DESTDIR)$(PREFIX)/$(MANN_DIR)/Hax.n; \
		rm -f Hax.n.tmp

	cd doc; \
		sed -e '/man\.macros/r man.macros' \
		    -e '/man\.macros/d' library.n > Hax_library.n; \
		install Hax_library.n $(DESTDIR)$(PREFIX)/$(MANN_DIR); \
		rm -f Hax_library.n

test: haxsh
	( echo cd tests ; echo source all ) | ./haxsh

clean:
	rm -f $(OBJS) libhax.a haxsh.o haxsh compat/Hax_errlist.c

$(OBJS): hax.h haxHash.h haxInt.h
$(UNIX_OJBS): haxUnix.h

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -I. -DHAX_LIBRARY=\"$(PREFIX)/$(HAX_LIBRARY)\" -o $@ -c $<
