#
# This Makefile is for use when distributing Hax to the outside world.
#
# Some changes you may wish to make here:
#
# 1. To compile for non-UNIX systems (so that only the non-UNIX-specific
# commands are available), disable libhaxunix.a build rules below.

# 2. If you want to put Hax-related information in non-standard places,
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
HAX_LIBRARY ?=	lib/hax
BIN_DIR ?=	bin
LIB_DIR ?=	lib
INCLUDE_DIR ?=	include
MAN_DIR ?=	man
MAN3_DIR ?=	$(MAN_DIR)/man3
MANN_DIR ?=	$(MAN_DIR)/mann

all: libhax.a libhaxunix.a haxsh rhaxsh

GENERIC_OBJS =	haxRegexp.o haxAssem.o haxBasic.o haxCkalloc.o \
	haxCmdAH.o haxCmdIL.o haxCmdMZ.o haxExpr.o haxGet.o \
	haxHash.o haxHistory.o haxParse.o haxProc.o haxUtil.o \
	haxVar.o haxPanic.o haxCkalloc.o haxBreakpoint.o haxStrtol.o

UNIX_OBJS = haxEnv.o haxGlob.o haxUnixAZ.o haxUnixStr.o haxUnixUtil.o

COMPAT_OBJS =

OBJS = $(GENERIC_OBJS) $(COMPAT_OBJS)

libhax.a: $(OBJS)
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

libhaxunix.a: $(UNIX_OBJS)
	$(AR) cr $@ $(UNIX_OBJS)
	$(RANLIB) $@

rhaxsh: rhaxsh.o libhax.a
	$(CC) $(LDFLAGS) -o $@ rhaxsh.o libhax.a

haxsh: haxsh.o libhax.a libhaxunix.a
	$(CC) $(LDFLAGS) -o $@ haxsh.o libhax.a libhaxunix.a

install: libhax.a libhaxunix.a haxsh rhaxsh
	install -d $(DESTDIR)$(PREFIX)/$(BIN_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(LIB_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(HAX_LIBRARY)
	install -d $(DESTDIR)$(PREFIX)/$(INCLUDE_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(MAN3_DIR)
	install -d $(DESTDIR)$(PREFIX)/$(MANN_DIR)

	install haxsh $(DESTDIR)$(PREFIX)/$(BIN_DIR)
	install rhaxsh $(DESTDIR)$(PREFIX)/$(BIN_DIR)

	cd library; for i in *.tcl; do \
		install $$i $(DESTDIR)$(PREFIX)/$(HAX_LIBRARY); \
	done

	install libhax.a $(DESTDIR)$(PREFIX)/$(LIB_DIR)
	install libhaxunix.a $(DESTDIR)$(PREFIX)/$(LIB_DIR)

	install hax.h $(DESTDIR)$(PREFIX)/$(INCLUDE_DIR)

	cd doc; for i in *.3; do \
		sed -e '/man\.macros/r man.macros' -e '/man\.macros/d' \
			$$i > Hax_$$i; \
		install Hax_$$i $(DESTDIR)$(PREFIX)/$(MAN3_DIR); \
		rm -f Hax_$$i; \
	done

	for i in Hax_AddErrorInfo Hax_SetErrorCode Hax_UnixError; do \
		ln -fs Hax_AddErrInfo.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_CreateCmdBuf Hax_DeleteCmdBuf Hax_AssembleCmd \
		Hax_CommandComplete; do \
		ln -fs Hax_AssembCmd.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_CreateCommand Hax_DeleteCommand; do \
		ln -fs Hax_CrtCommand.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_CreateInterp Hax_DeleteInterp; do \
		ln -fs Hax_CrtInterp.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_CreatePipeline; do \
		ln -fs Hax_CrtPipelin.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_CreateTrace Hax_DeleteTrace; do \
		ln -fs Hax_CrtTrace.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_VarEval Hax_EvalFile Hax_GlobalEval; do \
		ln -fs Hax_Eval.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_ExprLong Hax_ExprLongLong Hax_ExprDouble Hax_ExprBoolean \
		Hax_ExprString; do \
		ln -fs Hax_ExprLong.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_WaitPids Hax_DetachPids; do \
		ln -fs Hax_Fork.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_GetInt Hax_GetLong Hax_GetLongLong Hax_GetDouble \
		Hax_GetBoolean; do \
		ln -fs Hax_GetInt.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_InitHashTable Hax_DeleteHashTable Hax_CreateHashEntry \
		Hax_DeleteHashEntry Hax_FindHashEntry Hax_GetHashValue \
		Hax_SetHashValue Hax_GetHashKey Hax_FirstHashEntry \
		Hax_NextHashEntry Hax_HashStats; do \
		ln -fs Hax_Hash.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_InitHistory Hax_RecordAndEval; do \
		ln -fs Hax_History.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_AppendResult Hax_AppendElement \
		Hax_ResetResult Hax_FreeResult; do \
		ln -fs Hax_SetResult.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_SetVar2 Hax_GetVar Hax_GetVar2 Hax_UnsetVar \
		Hax_UnsetVar2; do \
		ln -fs Hax_SetVar.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_Merge Hax_ScanElement Hax_ConvertElement; do \
		ln -fs Hax_SplitList.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_StringMatch; do \
		ln -fs Hax_StrMatch.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
	done

	for i in Hax_TraceVar2 Hax_UnTraceVar Hax_UnTraceVar2 \
		Hax_VarTraceInfo Hax_VarTraceInfo2; do \
		ln -fs Hax_TraceVar.3 $(DESTDIR)$(PREFIX)/$(MAN3_DIR)/$$i.3; \
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
	rm -f $(OBJS) $(UNIX_OBJS) libhax.a libhaxunix.a \
		haxsh.o haxsh rhaxsh.o rhaxsh libfuzzer

libfuzzer: libhax.a libfuzzer.o
	$(CC) $(LDFLAGS) -o $@ libfuzzer.o libhax.a

$(OBJS): hax.h haxInt.h
$(UNIX_OJBS): hax.h haxUnix.h

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -I. -DHAX_LIBRARY=\"$(PREFIX)/$(HAX_LIBRARY)\" -o $@ -c $<
