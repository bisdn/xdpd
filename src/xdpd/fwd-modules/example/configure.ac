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
link_all_deplibs=yes
link_all_deplibs_CXX=yes
AC_ENABLE_STATIC
LIBTOOL="$LIBTOOL --preserve-dup-deps"
AC_SUBST(LIBTOOL)dnl


# default flags
CFLAGS="-Werror -Wall -Wl,-export-dynamic $CFLAGS"
CXXFLAGS="-Werror -Wall -Wl,-export-dynamic $CXXFLAGS"
AC_DEFINE(__STDC_FORMAT_MACROS)

#Debug
m4_include([../../../../config/debug.m4])

# Application verbosity 
m4_include([../../../../config/verbose.m4])

# Check for profiling mode
m4_include([../../../../config/profiling.m4])

AC_DEFINE(FWD_MOD_NAME,"example_platform")

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/afa-imp/Makefile
	src/afa-imp/openflow/openflow1x/Makefile
	src/pipeline-imp/Makefile
])

AC_OUTPUT
