AC_INIT(xdpd - Example fwd-module, 0.3, rofl@bisdn.de,  xdpd_example_fm)
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_GNU_SOURCE

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LD
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AUTOCONF_ENV="SHELL=/bin/sh"
LT_INIT
AC_ENABLE_STATIC

# default flags
CFLAGS="-Werror -Wall -Wl,-export-dynamic $CFLAGS"
CXXFLAGS="-Werror -Wall -Wl,-export-dynamic $CXXFLAGS"
AC_DEFINE(__STDC_FORMAT_MACROS)

# Check for debug mode - MUST BE THE FIRST CHECK
AC_MSG_CHECKING(whether to enable debug mode)
debug_default="no"
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug], [turn on debug mode [default=no]])
		, , enable_debug=$debug_default)
if test "$enable_debug" = "yes"; then
	CFLAGS="$CFLAGS -g -O0"
	CXXFLAGS="$CXXFLAGS -g -O0 -fno-inline"
	AC_DEFINE(DEBUG)
	AC_MSG_RESULT(yes)
else
	CFLAGS="$CFLAGS -O3" #--compiler-options -fno-strict-aliasing --compiler-options -fno-inline
	CXXFLAGS="$CXXFLAGS -O3" #-fomit-frame-pointer"
	AC_DEFINE(NDEBUG)
	AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(DEBUG, test "$enable_debug" = yes)

# Application verbosity 
AC_MSG_CHECKING(whether to print debug in verbose mode)
AC_ARG_ENABLE(verbose,
	AS_HELP_STRING([--verbose], [turn on verbose mode [default=no]])
		, verbose_debug="yes", verbose_debug="no")
if test "$verbose_debug" = "yes"; then
	CFLAGS="$CFLAGS -DVERBOSE_DEBUG" 
	CXXFLAGS="$CXXFLAGS -DVERBOSE_DEBUG" 
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

# Check for profiling mode
AC_MSG_CHECKING(whether to enable profiling mode)
profile_default="no"
AC_ARG_ENABLE(profile,
	AS_HELP_STRING([--enable-profile], [turn on profile mode [default=no]])
		, , enable_profile=$profile_default)
if test "$enable_profile" = "yes"; then
	CFLAGS="$( echo $CFLAGS | sed s/-fomit-frame-pointer//g )"
	CXXFLAGS="$( echo $CXXFLAGS | sed s/-fomit-frame-pointer//g )"
	CFLAGS="$CFLAGS -pg"
	CXXFLAGS="$CXXFLAGS -pg"
	LDFLAGS="$LDFLAGS -pg"
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/afa-imp/Makefile
	src/afa-imp/openflow/openflow12/Makefile
	src/pipeline-imp/Makefile
])

AC_OUTPUT
