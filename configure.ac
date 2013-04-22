AC_INIT(xdpd, 0.3, rofl@bisdn.de, xdpd)
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

# Make verbosity 
AC_MSG_CHECKING(whether to output in verbose mode)
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


##########################
#CHECKING Hardware FLAGS #
##########################
AC_MSG_CHECKING(hardware support to be compiled)

AC_ARG_ENABLE(gnu-linux,
	AS_HELP_STRING([--enable-gnu-linux], [compile GNU/Linux hardware support [default=no]])
			, , enable_gnu_linux="no")
AC_ARG_ENABLE(gnu-linux-dpdk,
	AS_HELP_STRING([--enable-gnu-linux-dpdk], [compile GNU/Linux Intel DPDK support [default=no]])
			, , enable_gnu_linux_dpdk="no")
AC_ARG_ENABLE(bcm,
	AS_HELP_STRING([--enable-bcm], [compile Broadcom hardware support [default=no]])
			, , enable_bcm="no")
AC_ARG_ENABLE(octeon,
	AS_HELP_STRING([--enable-octeon], [compile OCTEON 5650 hardware support [default=no]])
			, , enable_octeon="no")
#Make default GNU/Linux if not defined
if ( test "$enable_gnu_linux_dpdk" = "no" ) && ( test "$enable_gnu_linux" = "no" ) && ( test "$enable_bcm" = "no" ) && ( test "$enable_octeon" = "no" ); then
	enable_gnu_linux="yes"
fi

#Settings conditionals
AM_CONDITIONAL([ENABLE_GNU_LINUX_SUPPORT],[test "$enable_gnu_linux" = "yes"])
AM_CONDITIONAL([ENABLE_GNU_LINUX_DPDK_SUPPORT],[test "$enable_gnu_linux_dpdk" = "yes"])
AM_CONDITIONAL([ENABLE_BCM_SUPPORT],[test "$enable_bcm" = "yes"])
AM_CONDITIONAL([ENABLE_OCTEON_SUPPORT],[test "$enable_octeon" = "yes"])

PLATFORM=gnu_linux

#Print messages
if ( test "$enable_gnu_linux" = "yes" );then
	msg="$msg GNU/Linux"
	AC_DEFINE(ENABLE_GNU_LINUX)
	PLATFORM=gnu_linux
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/gnu_linux])
fi
if ( test "$enable_gnu_linux_dpdk" = "yes" );then
	msg="$msg GNU/Linux Intel DPDK"
	AC_DEFINE(ENABLE_GNU_LINUX_DPDK)
	PLATFORM=gnu_linux_dpdk
fi
if( test "$enable_bcm" = "yes" );then
	msg="$msg Broadcom"
	AC_DEFINE(ENABLE_BCM)
	PLATFORM=bcm
fi	
if( test "$enable_octeon" = "yes" );then
	msg="$msg OCTEON 5650"
	AC_DEFINE(ENABLE_OCTEON5650)
	PLATFORM=octeon5650
fi

AC_MSG_RESULT($msg)
AC_SUBST(PLATFORM)

# Makefiles for gnu_linux
#AM_COND_IF(ENABLE_GNU_LINUX_SUPPORT,[AC_CONFIG_FILES([
#	src/xdpd/fwd-modules/gnu_linux/Makefile
#	src/xdpd/fwd-modules/gnu_linux/afa-imp/Makefile
#	src/xdpd/fwd-modules/gnu_linux/afa-imp/openflow/openflow12/Makefile
#	src/xdpd/fwd-modules/gnu_linux/io/Makefile
#	src/xdpd/fwd-modules/gnu_linux/io/packet_classifiers/Makefile
#	src/xdpd/fwd-modules/gnu_linux/io/ports/Makefile
#	src/xdpd/fwd-modules/gnu_linux/io/ports/mmap/Makefile
#	src/xdpd/fwd-modules/gnu_linux/io/ports/mockup/Makefile
#	src/xdpd/fwd-modules/gnu_linux/io/scheduler/Makefile
#	src/xdpd/fwd-modules/gnu_linux/pipeline-imp/Makefile
#	src/xdpd/fwd-modules/gnu_linux/processing/Makefile
#	src/xdpd/fwd-modules/gnu_linux/util/Makefile
#])])

# Makefiles for gnu_linux_dpdk
#AM_COND_IF(ENABLE_GNU_LINUX_DPDK_SUPPORT,[AC_CONFIG_FILES([
#	src/xdpd/fwd-modules/gnu_linux_dpdk/Makefile
#])])

# Makefiles for octeon
#AM_COND_IF(ENABLE_OCTEON_SUPPORT,[AC_CONFIG_FILES([
#	src/xdpd/fwd-modules/octeon5650/Makefile
#])])


# TODO: make the configuration front ends configureable, as soon as we have more than the cli
AC_DEFINE(HAVE_CONFIG_CLI)
AM_CONDITIONAL(ENABLE_CONFIG_CLI,[true])

AM_COND_IF(ENABLE_CONFIG_CLI,[AC_CONFIG_FILES([
	src/xdpd/management/adapter/cli/Makefile
])])

#Check libs used
AC_CHECK_LIB(pthread, pthread_kill,,AC_MSG_ERROR([pthread library not found])) 
AC_CHECK_LIB(rt, clock_gettime,,[AC_MSG_ERROR([rt library not found])]) 
AC_CHECK_LIB(cli, cli_telnet_protocol,,[AC_MSG_ERROR([cli library not found])]) 
#AC_CHECK_LIB(rofl_pipeline, physical_switch_init,,[AC_MSG_ERROR([rofl_pipeline library not found])]) 
#AC_SEARCH_LIBS(physical_switch_init, lrofl_pipeline,,[AC_MSG_ERROR([rofl_pipeline library not found])],)

# AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/xdpd/Makefile
	src/xdpd/management/Makefile
	src/xdpd/openflow/Makefile
	src/xdpd/openflow/openflow12/Makefile

	test/Makefile
	test/xdpd/Makefile
	test/xdpd/fwd-modules/Makefile
	test/xdpd/fwd-modules/gnu_linux/Makefile
	test/xdpd/fwd-modules/gnu_linux/regression/Makefile
	test/xdpd/fwd-modules/gnu_linux/regression/io/Makefile
	test/xdpd/fwd-modules/gnu_linux/regression/processing/Makefile
	test/xdpd/fwd-modules/gnu_linux/unit/Makefile
	test/xdpd/fwd-modules/gnu_linux/unit/util/Makefile
	test/xdpd/fwd-modules/gnu_linux/unit/io/Makefile

	doc/Makefile
])

AC_OUTPUT
