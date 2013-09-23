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

# Check for QMF support 
AC_MSG_CHECKING(whether to add Apache QMF support)
qmf_default="no"
AC_ARG_ENABLE(qmf,
	AS_HELP_STRING([--enable-qmf], [turn on Apache QMF support [default=no]])
		, , enable_qmf=$qmf_default)
if test "$enable_qmf" = "yes"; then 
	AC_DEFINE(HAVE_CONFIG_QMF)
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(ENABLE_CONFIG_QMF, test "$enable_qmf" = yes)


#############################
# CHECKING Hardware support #
#############################
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
AC_ARG_ENABLE(example,
	AS_HELP_STRING([--enable-example], [compile the Forwarding Module example code [default=no]])
			, , enable_example="no")
#[+] Add your platform here


#Make default GNU/Linux if not defined. 
#[+] Makesure you add your platform here
if ( test "$enable_gnu_linux_dpdk" = "no" ) &&
   ( test "$enable_gnu_linux" = "no" ) &&
   ( test "$enable_bcm" = "no" ) &&
   ( test "$enable_octeon" = "no" ) &&
   ( test "$enable_example" = "no" ); then
   
   enable_gnu_linux="yes"

fi

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
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/gnu_linux_dpdk])
fi
if( test "$enable_bcm" = "yes" );then
	msg="$msg Broadcom"
	AC_DEFINE(ENABLE_BCM)
	PLATFORM=bcm
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/bcm])
fi	
if( test "$enable_octeon" = "yes" );then
	msg="$msg OCTEON 5650"
	AC_DEFINE(ENABLE_OCTEON5650)
	PLATFORM=octeon5650
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/octeon5650])
fi
if( test "$enable_example" = "yes" );then
	msg="$msg Example platform"
	AC_DEFINE(ENABLE_EXAMPLE)
	PLATFORM=example
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/example])
fi
#[+]Add your platform here...


#Print fancy message
AC_MSG_RESULT($msg)
AC_SUBST(PLATFORM)


##########################
#CHECKING libs           #
##########################

#Check libs used
AC_CHECK_LIB(pthread, pthread_kill,,AC_MSG_ERROR([pthread library not found])) 
AC_CHECK_LIB(rt, clock_gettime,,[AC_MSG_ERROR([rt library not found])]) 
AC_CHECK_LIB(cli, cli_telnet_protocol,,[AC_MSG_ERROR([cli library not found])]) 
#AC_CHECK_LIB(rofl_pipeline, physical_switch_init,,[AC_MSG_ERROR([rofl_pipeline library not found])]) 
#AC_SEARCH_LIBS(physical_switch_init, lrofl_pipeline,,[AC_MSG_ERROR([rofl_pipeline library not found])],)

# AC_CONFIG_HEADERS([include/config.h])

## Other stuff
# FIX THIS: make the configuration front ends configureable, as soon as we have more than the cli
AC_DEFINE(HAVE_CONFIG_CLI)
AM_CONDITIONAL(ENABLE_CONFIG_CLI,[true])

AM_COND_IF(ENABLE_CONFIG_CLI,[AC_CONFIG_FILES([
	src/xdpd/management/plugins/cli/Makefile
])])

AM_COND_IF(ENABLE_CONFIG_QMF,[AC_CONFIG_FILES([
	src/xdpd/management/plugins/qmf/Makefile
])])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/xdpd/Makefile
	src/xdpd/fwd-modules/Makefile
	src/xdpd/management/Makefile
	src/xdpd/openflow/Makefile
	src/xdpd/openflow/openflow10/Makefile
	src/xdpd/openflow/openflow12/Makefile
])


AC_OUTPUT
