dnl Process this file with autoconf to produce a configure script.
AC_INIT(giflib, [4.1.3], [abadger1999@sourceforge.net], giflib)
AC_CONFIG_SRCDIR([lib/dgif_lib.c])
AM_INIT_AUTOMAKE([gnu check-news dist-bzip2 -Wall])
AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_PATH_XTRA

dnl Checks for libraries.

    AC_CHECK_LIB(m, pow, [MATH_LIB="${MATH_LIB} -lm"
                 COMPILABLE_EXTRAS="${COMPILABLE_EXTRAS} gifclrmp gifrotat"] )

if test ${MATH_LIB}; then
    AC_CHECK_LIB(rle, rle_hdr_init, [RLE_LIB="${RLE_LIBS} -lrle -lm"
                 COMPILABLE_EXTRAS="${COMPILABLE_EXTRAS} gif2rle rle2gif"],
                 ,"-lm" )
fi

dnl I don't have this on my system.  Could someone else configure it and
dnl check that it works?
    AC_CHECK_LIB(gl_s, main, [GL_S_LIB="${GL_S_LIB} -lgl_s $X_LIBS"
                             AC_DEFINE(HAVE_LIBGL_S, [1], [Define if the gl_s library is installed (SGI GL library)])
                 COMPILABLE_EXTRAS="${COMPILABLE_EXTRAS} gif2iris"],, $X_LIBS)

    AC_CHECK_LIB(X11, main, [X11_LIB="${X11_LIB} ${X_PRE_LIBS} $X_LIBS ${X_EXTRA_LIBS} -lX11"
                             AC_DEFINE(HAVE_LIBX11, [1], [Define if the X11 library is installed])
                 COMPILABLE_EXTRAS="${COMPILABLE_EXTRAS} gif2x11"],, $X_LIBS)
    DEVS="${GL_S_LIB} ${X11_LIB}" 
    AC_SUBST(DEVS)
    AC_SUBST(COMPILABLE_EXTRAS)
    AC_SUBST(GL_S_LIB)
    AC_SUBST(X11_LIB)
    AC_SUBST(X_CFLAGS)
    AC_SUBST(MATH_LIB)
    AC_SUBST(RLE_LIB)

dnl Checks for header files.
AC_HEADER_STDC
dnl * HEADER_STDC checks for stdargs.h so this is a bit redundant.
dnl * The code will prefer stdargs.h if available.
dnl   If not available, we will try to fallback to using varargs.h
AC_CHECK_HEADERS([stdarg.h varargs.h], break,
		 AC_MSG_ERROR([giflib 4.x must have stdarg.h or varargs.h]))

dnl Can we survive without these?
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(fcntl.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strdup)

AC_OUTPUT(util/Makefile lib/Makefile Makefile doc/Makefile pic/Makefile)
