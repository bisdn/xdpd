AC_INIT(xdpd, m4_esyscmd_s(cat VERSION), xdpd-devel@xdpd.org, xdpd, http://www.xdpd.org)

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_GNU_SOURCE

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LD
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

PKG_PROG_PKG_CONFIG

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
AC_DEFINE(__STDC_CONSTANT_MACROS)

#Check libs used
AC_CHECK_LIB(pthread, pthread_kill,,AC_MSG_ERROR([pthread library not found])) 
AC_CHECK_LIB(rt, clock_gettime,,[AC_MSG_ERROR([rt library not found])]) 

PKG_CHECK_MODULES([ROFL], [rofl >= 0.4.0], [have_rofl=yes], AC_MSG_ERROR([Revised OpenFlow Libraries(ROFL) not found]))
if (test "${have_rofl}" = "yes"); then
        CPPFLAGS+="$ROFL_CFLAGS"
        LIBS+="$ROFL_LIBS"
fi

#Debug
m4_include([config/debug.m4])

#Application verbosity 
m4_include([config/verbose.m4])

#Check for profiling mode
m4_include([config/profiling.m4])

#Experimental code
m4_include([config/experimental.m4])

#Checking OpenSSL
m4_include([config/openssl.m4])

#Boost
m4_include([config/boost.m4])

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
	src/xdpd/drivers/Makefile
	src/xdpd/management/Makefile
	src/xdpd/management/plugins/Makefile
	src/xdpd/openflow/Makefile
	src/xdpd/openflow/openflow10/Makefile
	src/xdpd/openflow/openflow12/Makefile
	src/xdpd/openflow/openflow13/Makefile
	test/Makefile
])

# Doxygen (here to be the last Makefile) 
m4_include([config/doxygen.m4])

AC_OUTPUT
