/* 
 * haxUnixStr.c --
 *
 *	This file contains procedures that generate strings
 *	corresponding to various UNIX-related codes, such
 *	as errno and signals.
 *
 * Copyright 1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that this copyright
 * notice appears in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /user6/ouster/tcl/RCS/tclUnixStr.c,v 1.14 93/01/29 14:42:51 ouster Exp $ SPRITE (Berkeley)";
#endif /* not lint */

#include "haxInt.h"
#include "haxUnix.h"

/*
 *----------------------------------------------------------------------
 *
 * Hax_ErrnoId --
 *
 *	Return a textual identifier for the current errno value.
 *
 * Results:
 *	This procedure returns a machine-readable textual identifier
 *	that corresponds to the current errno value (e.g. "EPERM").
 *	The identifier is the same as the #define name in errno.h.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

char *
Hax_ErrnoId(void)
{
    switch (errno) {
#ifdef E2BIG
	case E2BIG: return (char *) "E2BIG";
#endif
#ifdef EACCES
	case EACCES: return (char *) "EACCES";
#endif
#ifdef EADDRINUSE
	case EADDRINUSE: return (char *) "EADDRINUSE";
#endif
#ifdef EADDRNOTAVAIL
	case EADDRNOTAVAIL: return (char *) "EADDRNOTAVAIL";
#endif
#ifdef EADV
	case EADV: return (char *) "EADV";
#endif
#ifdef EAFNOSUPPORT
	case EAFNOSUPPORT: return (char *) "EAFNOSUPPORT";
#endif
#ifdef EAGAIN
	case EAGAIN: return (char *) "EAGAIN";
#endif
#ifdef EALIGN
	case EALIGN: return (char *) "EALIGN";
#endif
#ifdef EALREADY
	case EALREADY: return (char *) "EALREADY";
#endif
#ifdef EBADE
	case EBADE: return (char *) "EBADE";
#endif
#ifdef EBADF
	case EBADF: return (char *) "EBADF";
#endif
#ifdef EBADFD
	case EBADFD: return (char *) "EBADFD";
#endif
#ifdef EBADMSG
	case EBADMSG: return (char *) "EBADMSG";
#endif
#ifdef EBADR
	case EBADR: return (char *) "EBADR";
#endif
#ifdef EBADRPC
	case EBADRPC: return (char *) "EBADRPC";
#endif
#ifdef EBADRQC
	case EBADRQC: return (char *) "EBADRQC";
#endif
#ifdef EBADSLT
	case EBADSLT: return (char *) "EBADSLT";
#endif
#ifdef EBFONT
	case EBFONT: return (char *) "EBFONT";
#endif
#ifdef EBUSY
	case EBUSY: return (char *) "EBUSY";
#endif
#ifdef ECHILD
	case ECHILD: return (char *) "ECHILD";
#endif
#ifdef ECHRNG
	case ECHRNG: return (char *) "ECHRNG";
#endif
#ifdef ECOMM
	case ECOMM: return (char *) "ECOMM";
#endif
#ifdef ECONNABORTED
	case ECONNABORTED: return (char *) "ECONNABORTED";
#endif
#ifdef ECONNREFUSED
	case ECONNREFUSED: return (char *) "ECONNREFUSED";
#endif
#ifdef ECONNRESET
	case ECONNRESET: return (char *) "ECONNRESET";
#endif
#if defined(EDEADLK) && (!defined(EWOULDBLOCK) || (EDEADLK != EWOULDBLOCK))
	case EDEADLK: return (char *) "EDEADLK";
#endif
#ifdef EDEADLOCK
	case EDEADLOCK: return (char *) "EDEADLOCK";
#endif
#ifdef EDESTADDRREQ
	case EDESTADDRREQ: return (char *) "EDESTADDRREQ";
#endif
#ifdef EDIRTY
	case EDIRTY: return (char *) "EDIRTY";
#endif
#ifdef EDOM
	case EDOM: return (char *) "EDOM";
#endif
#ifdef EDOTDOT
	case EDOTDOT: return (char *) "EDOTDOT";
#endif
#ifdef EDQUOT
	case EDQUOT: return (char *) "EDQUOT";
#endif
#ifdef EDUPPKG
	case EDUPPKG: return (char *) "EDUPPKG";
#endif
#ifdef EEXIST
	case EEXIST: return (char *) "EEXIST";
#endif
#ifdef EFAULT
	case EFAULT: return (char *) "EFAULT";
#endif
#ifdef EFBIG
	case EFBIG: return (char *) "EFBIG";
#endif
#ifdef EHOSTDOWN
	case EHOSTDOWN: return (char *) "EHOSTDOWN";
#endif
#ifdef EHOSTUNREACH
	case EHOSTUNREACH: return (char *) "EHOSTUNREACH";
#endif
#ifdef EIDRM
	case EIDRM: return (char *) "EIDRM";
#endif
#ifdef EINIT
	case EINIT: return (char *) "EINIT";
#endif
#ifdef EINPROGRESS
	case EINPROGRESS: return (char *) "EINPROGRESS";
#endif
#ifdef EINTR
	case EINTR: return (char *) "EINTR";
#endif
#ifdef EINVAL
	case EINVAL: return (char *) "EINVAL";
#endif
#ifdef EIO
	case EIO: return (char *) "EIO";
#endif
#ifdef EISCONN
	case EISCONN: return (char *) "EISCONN";
#endif
#ifdef EISDIR
	case EISDIR: return (char *) "EISDIR";
#endif
#ifdef EISNAME
	case EISNAM: return (char *) "EISNAM";
#endif
#ifdef ELBIN
	case ELBIN: return (char *) "ELBIN";
#endif
#ifdef EL2HLT
	case EL2HLT: return (char *) "EL2HLT";
#endif
#ifdef EL2NSYNC
	case EL2NSYNC: return (char *) "EL2NSYNC";
#endif
#ifdef EL3HLT
	case EL3HLT: return (char *) "EL3HLT";
#endif
#ifdef EL3RST
	case EL3RST: return (char *) "EL3RST";
#endif
#ifdef ELIBACC
	case ELIBACC: return (char *) "ELIBACC";
#endif
#ifdef ELIBBAD
	case ELIBBAD: return (char *) "ELIBBAD";
#endif
#ifdef ELIBEXEC
	case ELIBEXEC: return (char *) "ELIBEXEC";
#endif
#ifdef ELIBMAX
	case ELIBMAX: return (char *) "ELIBMAX";
#endif
#ifdef ELIBSCN
	case ELIBSCN: return (char *) "ELIBSCN";
#endif
#ifdef ELNRNG
	case ELNRNG: return (char *) "ELNRNG";
#endif
#ifdef ELOOP
	case ELOOP: return (char *) "ELOOP";
#endif
#ifdef EMFILE
	case EMFILE: return (char *) "EMFILE";
#endif
#ifdef EMLINK
	case EMLINK: return (char *) "EMLINK";
#endif
#ifdef EMSGSIZE
	case EMSGSIZE: return (char *) "EMSGSIZE";
#endif
#ifdef EMULTIHOP
	case EMULTIHOP: return (char *) "EMULTIHOP";
#endif
#ifdef ENAMETOOLONG
	case ENAMETOOLONG: return (char *) "ENAMETOOLONG";
#endif
#ifdef ENAVAIL
	case ENAVAIL: return (char *) "ENAVAIL";
#endif
#ifdef ENET
	case ENET: return (char *) "ENET";
#endif
#ifdef ENETDOWN
	case ENETDOWN: return (char *) "ENETDOWN";
#endif
#ifdef ENETRESET
	case ENETRESET: return (char *) "ENETRESET";
#endif
#ifdef ENETUNREACH
	case ENETUNREACH: return (char *) "ENETUNREACH";
#endif
#ifdef ENFILE
	case ENFILE: return (char *) "ENFILE";
#endif
#ifdef ENOANO
	case ENOANO: return (char *) "ENOANO";
#endif
#if defined(ENOBUFS) && (!defined(ENOSR) || (ENOBUFS != ENOSR))
	case ENOBUFS: return (char *) "ENOBUFS";
#endif
#ifdef ENOCSI
	case ENOCSI: return (char *) "ENOCSI";
#endif
#ifdef ENODATA
	case ENODATA: return (char *) "ENODATA";
#endif
#ifdef ENODEV
	case ENODEV: return (char *) "ENODEV";
#endif
#ifdef ENOENT
	case ENOENT: return (char *) "ENOENT";
#endif
#ifdef ENOEXEC
	case ENOEXEC: return (char *) "ENOEXEC";
#endif
#ifdef ENOLCK
	case ENOLCK: return (char *) "ENOLCK";
#endif
#ifdef ENOLINK
	case ENOLINK: return (char *) "ENOLINK";
#endif
#ifdef ENOMEM
	case ENOMEM: return (char *) "ENOMEM";
#endif
#ifdef ENOMSG
	case ENOMSG: return (char *) "ENOMSG";
#endif
#ifdef ENONET
	case ENONET: return (char *) "ENONET";
#endif
#ifdef ENOPKG
	case ENOPKG: return (char *) "ENOPKG";
#endif
#ifdef ENOPROTOOPT
	case ENOPROTOOPT: return (char *) "ENOPROTOOPT";
#endif
#ifdef ENOSPC
	case ENOSPC: return (char *) "ENOSPC";
#endif
#ifdef ENOSR
	case ENOSR: return (char *) "ENOSR";
#endif
#ifdef ENOSTR
	case ENOSTR: return (char *) "ENOSTR";
#endif
#ifdef ENOSYM
	case ENOSYM: return (char *) "ENOSYM";
#endif
#ifdef ENOSYS
	case ENOSYS: return (char *) "ENOSYS";
#endif
#ifdef ENOTBLK
	case ENOTBLK: return (char *) "ENOTBLK";
#endif
#ifdef ENOTCONN
	case ENOTCONN: return (char *) "ENOTCONN";
#endif
#ifdef ENOTDIR
	case ENOTDIR: return (char *) "ENOTDIR";
#endif
#if defined(ENOTEMPTY) && (!defined(EEXIST) || (ENOTEMPTY != EEXIST))
	case ENOTEMPTY: return (char *) "ENOTEMPTY";
#endif
#ifdef ENOTNAM
	case ENOTNAM: return (char *) "ENOTNAM";
#endif
#ifdef ENOTSOCK
	case ENOTSOCK: return (char *) "ENOTSOCK";
#endif
#ifdef ENOTTY
	case ENOTTY: return (char *) "ENOTTY";
#endif
#ifdef ENOTUNIQ
	case ENOTUNIQ: return (char *) "ENOTUNIQ";
#endif
#ifdef ENXIO
	case ENXIO: return (char *) "ENXIO";
#endif
#ifdef EOPNOTSUPP
	case EOPNOTSUPP: return (char *) "EOPNOTSUPP";
#endif
#ifdef EPERM
	case EPERM: return (char *) "EPERM";
#endif
#ifdef EPFNOSUPPORT
	case EPFNOSUPPORT: return (char *) "EPFNOSUPPORT";
#endif
#ifdef EPIPE
	case EPIPE: return (char *) "EPIPE";
#endif
#ifdef EPROCLIM
	case EPROCLIM: return (char *) "EPROCLIM";
#endif
#ifdef EPROCUNAVAIL
	case EPROCUNAVAIL: return (char *) "EPROCUNAVAIL";
#endif
#ifdef EPROGMISMATCH
	case EPROGMISMATCH: return (char *) "EPROGMISMATCH";
#endif
#ifdef EPROGUNAVAIL
	case EPROGUNAVAIL: return (char *) "EPROGUNAVAIL";
#endif
#ifdef EPROTO
	case EPROTO: return (char *) "EPROTO";
#endif
#ifdef EPROTONOSUPPORT
	case EPROTONOSUPPORT: return (char *) "EPROTONOSUPPORT";
#endif
#ifdef EPROTOTYPE
	case EPROTOTYPE: return (char *) "EPROTOTYPE";
#endif
#ifdef ERANGE
	case ERANGE: return (char *) "ERANGE";
#endif
#if defined(EREFUSED) && (!defined(ECONNREFUSED) || (EREFUSED != ECONNREFUSED))
	case EREFUSED: return (char *) "EREFUSED";
#endif
#ifdef EREMCHG
	case EREMCHG: return (char *) "EREMCHG";
#endif
#ifdef EREMDEV
	case EREMDEV: return (char *) "EREMDEV";
#endif
#ifdef EREMOTE
	case EREMOTE: return (char *) "EREMOTE";
#endif
#ifdef EREMOTEIO
	case EREMOTEIO: return (char *) "EREMOTEIO";
#endif
#ifdef EREMOTERELEASE
	case EREMOTERELEASE: return (char *) "EREMOTERELEASE";
#endif
#ifdef EROFS
	case EROFS: return (char *) "EROFS";
#endif
#ifdef ERPCMISMATCH
	case ERPCMISMATCH: return (char *) "ERPCMISMATCH";
#endif
#ifdef ERREMOTE
	case ERREMOTE: return (char *) "ERREMOTE";
#endif
#ifdef ESHUTDOWN
	case ESHUTDOWN: return (char *) "ESHUTDOWN";
#endif
#ifdef ESOCKTNOSUPPORT
	case ESOCKTNOSUPPORT: return (char *) "ESOCKTNOSUPPORT";
#endif
#ifdef ESPIPE
	case ESPIPE: return (char *) "ESPIPE";
#endif
#ifdef ESRCH
	case ESRCH: return (char *) "ESRCH";
#endif
#ifdef ESRMNT
	case ESRMNT: return (char *) "ESRMNT";
#endif
#ifdef ESTALE
	case ESTALE: return (char *) "ESTALE";
#endif
#ifdef ESUCCESS
	case ESUCCESS: return (char *) "ESUCCESS";
#endif
#ifdef ETIME
	case ETIME: return (char *) "ETIME";
#endif
#ifdef ETIMEDOUT
	case ETIMEDOUT: return (char *) "ETIMEDOUT";
#endif
#ifdef ETOOMANYREFS
	case ETOOMANYREFS: return (char *) "ETOOMANYREFS";
#endif
#ifdef ETXTBSY
	case ETXTBSY: return (char *) "ETXTBSY";
#endif
#ifdef EUCLEAN
	case EUCLEAN: return (char *) "EUCLEAN";
#endif
#ifdef EUNATCH
	case EUNATCH: return (char *) "EUNATCH";
#endif
#ifdef EUSERS
	case EUSERS: return (char *) "EUSERS";
#endif
#ifdef EVERSION
	case EVERSION: return (char *) "EVERSION";
#endif
#if defined(EWOULDBLOCK) && (!defined(EAGAIN) || (EWOULDBLOCK != EAGAIN))
	case EWOULDBLOCK: return (char *) "EWOULDBLOCK";
#endif
#ifdef EXDEV
	case EXDEV: return (char *) "EXDEV";
#endif
#ifdef EXFULL
	case EXFULL: return (char *) "EXFULL";
#endif
    }
    return (char *) "unknown error";
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_SignalId --
 *
 *	Return a textual identifier for a signal number.
 *
 * Results:
 *	This procedure returns a machine-readable textual identifier
 *	that corresponds to sig.  The identifier is the same as the
 *	#define name in signal.h.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

char *
Hax_SignalId(
    int sig			/* Number of signal. */)
{
    switch (sig) {
#ifdef SIGABRT
	case SIGABRT: return (char *) "SIGABRT";
#endif
#ifdef SIGALRM
	case SIGALRM: return (char *) "SIGALRM";
#endif
#ifdef SIGBUS
	case SIGBUS: return (char *) "SIGBUS";
#endif
#ifdef SIGCHLD
	case SIGCHLD: return (char *) "SIGCHLD";
#endif
#if defined(SIGCLD) && (!defined(SIGCHLD) || (SIGCLD != SIGCHLD))
	case SIGCLD: return (char *) "SIGCLD";
#endif
#ifdef SIGCONT
	case SIGCONT: return (char *) "SIGCONT";
#endif
#if defined(SIGEMT) && (!defined(SIGXCPU) || (SIGEMT != SIGXCPU))
	case SIGEMT: return (char *) "SIGEMT";
#endif
#ifdef SIGFPE
	case SIGFPE: return (char *) "SIGFPE";
#endif
#ifdef SIGHUP
	case SIGHUP: return (char *) "SIGHUP";
#endif
#ifdef SIGILL
	case SIGILL: return (char *) "SIGILL";
#endif
#ifdef SIGINT
	case SIGINT: return (char *) "SIGINT";
#endif
#ifdef SIGIO
	case SIGIO: return (char *) "SIGIO";
#endif
#if defined(SIGIOT) && (!defined(SIGABRT) || (SIGIOT != SIGABRT))
	case SIGIOT: return (char *) "SIGIOT";
#endif
#ifdef SIGKILL
	case SIGKILL: return (char *) "SIGKILL";
#endif
#if defined(SIGLOST) && (!defined(SIGIOT) || (SIGLOST != SIGIOT)) && (!defined(SIGURG) || (SIGLOST != SIGURG))
	case SIGLOST: return (char *) "SIGLOST";
#endif
#ifdef SIGPIPE
	case SIGPIPE: return (char *) "SIGPIPE";
#endif
#if defined(SIGPOLL) && (!defined(SIGIO) || (SIGPOLL != SIGIO))
	case SIGPOLL: return (char *) "SIGPOLL";
#endif
#ifdef SIGPROF
	case SIGPROF: return (char *) "SIGPROF";
#endif
#if defined(SIGPWR) && (!defined(SIGXFSZ) || (SIGPWR != SIGXFSZ))
	case SIGPWR: return (char *) "SIGPWR";
#endif
#ifdef SIGQUIT
	case SIGQUIT: return (char *) "SIGQUIT";
#endif
#ifdef SIGSEGV
	case SIGSEGV: return (char *) "SIGSEGV";
#endif
#ifdef SIGSTOP
	case SIGSTOP: return (char *) "SIGSTOP";
#endif
#ifdef SIGSYS
	case SIGSYS: return (char *) "SIGSYS";
#endif
#ifdef SIGTERM
	case SIGTERM: return (char *) "SIGTERM";
#endif
#ifdef SIGTRAP
	case SIGTRAP: return (char *) "SIGTRAP";
#endif
#ifdef SIGTSTP
	case SIGTSTP: return (char *) "SIGTSTP";
#endif
#ifdef SIGTTIN
	case SIGTTIN: return (char *) "SIGTTIN";
#endif
#ifdef SIGTTOU
	case SIGTTOU: return (char *) "SIGTTOU";
#endif
#if defined(SIGURG) && (!defined(SIGIO) || (SIGURG != SIGIO))
	case SIGURG: return (char *) "SIGURG";
#endif
#ifdef SIGUSR1
	case SIGUSR1: return (char *) "SIGUSR1";
#endif
#ifdef SIGUSR2
	case SIGUSR2: return (char *) "SIGUSR2";
#endif
#ifdef SIGVTALRM
	case SIGVTALRM: return (char *) "SIGVTALRM";
#endif
#ifdef SIGWINCH
	case SIGWINCH: return (char *) "SIGWINCH";
#endif
#ifdef SIGXCPU
	case SIGXCPU: return (char *) "SIGXCPU";
#endif
#ifdef SIGXFSZ
	case SIGXFSZ: return (char *) "SIGXFSZ";
#endif
    }
    return (char *) "unknown signal";
}

/*
 *----------------------------------------------------------------------
 *
 * Hax_SignalMsg --
 *
 *	Return a human-readable message describing a signal.
 *
 * Results:
 *	This procedure returns a string describing sig that should
 *	make sense to a human.  It may not be easy for a machine
 *	to parse.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

char *
Hax_SignalMsg(
    int sig			/* Number of signal. */)
{
    switch (sig) {
#ifdef SIGABRT
	case SIGABRT: return (char *) "SIGABRT";
#endif
#ifdef SIGALRM
	case SIGALRM: return (char *) "alarm clock";
#endif
#ifdef SIGBUS
	case SIGBUS: return (char *) "bus error";
#endif
#ifdef SIGCHLD
	case SIGCHLD: return (char *) "child status changed";
#endif
#if defined(SIGCLD) && (!defined(SIGCHLD) || (SIGCLD != SIGCHLD))
	case SIGCLD: return (char *) "child status changed";
#endif
#ifdef SIGCONT
	case SIGCONT: return (char *) "continue after stop";
#endif
#if defined(SIGEMT) && (!defined(SIGXCPU) || (SIGEMT != SIGXCPU))
	case SIGEMT: return (char *) "EMT instruction";
#endif
#ifdef SIGFPE
	case SIGFPE: return (char *) "floating-point exception";
#endif
#ifdef SIGHUP
	case SIGHUP: return (char *) "hangup";
#endif
#ifdef SIGILL
	case SIGILL: return (char *) "illegal instruction";
#endif
#ifdef SIGINT
	case SIGINT: return (char *) "interrupt";
#endif
#ifdef SIGIO
	case SIGIO: return (char *) "input/output possible on file";
#endif
#if defined(SIGIOT) && (!defined(SIGABRT) || (SIGABRT != SIGIOT))
	case SIGIOT: return (char *) "IOT instruction";
#endif
#ifdef SIGKILL
	case SIGKILL: return (char *) "kill signal";
#endif
#if defined(SIGLOST) && (!defined(SIGIOT) || (SIGLOST != SIGIOT)) && (!defined(SIGURG) || (SIGLOST != SIGURG))
	case SIGLOST: return (char *) "resource lost";
#endif
#ifdef SIGPIPE
	case SIGPIPE: return (char *) "write on pipe with no readers";
#endif
#if defined(SIGPOLL) && (!defined(SIGIO) || (SIGPOLL != SIGIO))
	case SIGPOLL: return (char *) "input/output possible on file";
#endif
#ifdef SIGPROF
	case SIGPROF: return (char *) "profiling alarm";
#endif
#if defined(SIGPWR) && (!defined(SIGXFSZ) || (SIGPWR != SIGXFSZ))
	case SIGPWR: return (char *) "power-fail restart";
#endif
#ifdef SIGQUIT
	case SIGQUIT: return (char *) "quit signal";
#endif
#ifdef SIGSEGV
	case SIGSEGV: return (char *) "segmentation violation";
#endif
#ifdef SIGSTOP
	case SIGSTOP: return (char *) "stop";
#endif
#ifdef SIGSYS
	case SIGSYS: return (char *) "bad argument to system call";
#endif
#ifdef SIGTERM
	case SIGTERM: return (char *) "software termination signal";
#endif
#ifdef SIGTRAP
	case SIGTRAP: return (char *) "trace trap";
#endif
#ifdef SIGTSTP
	case SIGTSTP: return (char *) "stop signal from tty";
#endif
#ifdef SIGTTIN
	case SIGTTIN: return (char *) "background tty read";
#endif
#ifdef SIGTTOU
	case SIGTTOU: return (char *) "background tty write";
#endif
#if defined(SIGURG) && (!defined(SIGIO) || (SIGURG != SIGIO))
	case SIGURG: return (char *) "urgent I/O condition";
#endif
#ifdef SIGUSR1
	case SIGUSR1: return (char *) "user-defined signal 1";
#endif
#ifdef SIGUSR2
	case SIGUSR2: return (char *) "user-defined signal 2";
#endif
#ifdef SIGVTALRM
	case SIGVTALRM: return (char *) "virtual time alarm";
#endif
#ifdef SIGWINCH
	case SIGWINCH: return (char *) "window changed";
#endif
#ifdef SIGXCPU
	case SIGXCPU: return (char *) "exceeded CPU time limit";
#endif
#ifdef SIGXFSZ
	case SIGXFSZ: return (char *) "exceeded file size limit";
#endif
    }
    return (char *) "unknown signal";
}
