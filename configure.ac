AC_INIT(xdpd, 0.3, xdpd@bisdn.de, xdpd)
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


#Default flags
CFLAGS="-Werror -Wall -Wl,--as-needed -Wl,-export-dynamic $CFLAGS"
CXXFLAGS="-Werror -Wall -Wl,--as-needed -Wl,-export-dynamic $CXXFLAGS"
AC_DEFINE(__STDC_FORMAT_MACROS)

#Check libs used
AC_CHECK_LIB(pthread, pthread_kill,,AC_MSG_ERROR([pthread library not found])) 
AC_CHECK_LIB(rt, clock_gettime,,[AC_MSG_ERROR([rt library not found])]) 
#AC_CHECK_LIB(rofl_pipeline, physical_switch_init,,[AC_MSG_ERROR([rofl_pipeline library not found])]) 
#AC_CHECK_LIB(rofl_pipeline, physical_switch_init,,[AC_MSG_ERROR([rofl_core library not found])]) 

PKG_CHECK_MODULES([ROFL], [rofl >= 0.3.0], [have_rofl=yes], [have_rofl=no])
if (test "${have_rofl}" = "yes"); then
        CPPFLAGS+="$ROFL_CFLAGS"
        LIBS+="$ROFL_LIBS"
fi


#Debug
m4_include([config/debug.m4])

# Application verbosity 
m4_include([config/verbose.m4])

# Check for profiling mode
m4_include([config/profiling.m4])

#Plugin main m4 macros
m4_include([config/plugins.m4])

#Check hardware support
m4_include([config/hw.m4])

#Check hardware support
m4_include([config/versioning.m4])

#Output the files
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/xdpd/Makefile
	src/xdpd/fwd-modules/Makefile
	src/xdpd/management/Makefile
	src/xdpd/openflow/Makefile
	src/xdpd/openflow/openflow10/Makefile
	src/xdpd/openflow/openflow12/Makefile
	test/Makefile
	test/grouptest/Makefile
])

# Doxygen (here to be the last Makefile) 
m4_include([config/doxygen.m4])

AC_OUTPUT
