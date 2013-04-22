AC_INIT(xdpd - GNU/Linux fwd-module, 0.3, rofl@bisdn.de, xdpd)
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

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/afa-imp/Makefile
	src/afa-imp/openflow/openflow12/Makefile
	src/io/Makefile
	src/io/packet_classifiers/Makefile
	src/io/ports/Makefile
	src/io/ports/mmap/Makefile
	src/io/ports/mockup/Makefile
	src/io/scheduler/Makefile
	src/pipeline-imp/Makefile
	src/processing/Makefile
	src/util/Makefile

])

#	src/Makefile
#	src/xdpd/Makefile
#	src/xdpd/management/Makefile
	
#	src/xdpd/fwd-modules/Makefile
	
#	src/xdpd/openflow/Makefile
#	src/xdpd/openflow/openflow12/Makefile

#	test/Makefile
#	test/xdpd/Makefile
#	test/xdpd/fwd-modules/Makefile
#	test/xdpd/fwd-modules/gnu_linux/Makefile
#	test/xdpd/fwd-modules/gnu_linux/regression/Makefile
#	test/xdpd/fwd-modules/gnu_linux/regression/io/Makefile
#	test/xdpd/fwd-modules/gnu_linux/regression/processing/Makefile
#	test/xdpd/fwd-modules/gnu_linux/unit/Makefile
#	test/xdpd/fwd-modules/gnu_linux/unit/util/Makefile
#	test/xdpd/fwd-modules/gnu_linux/unit/io/Makefile

#	doc/Makefile
#])

AC_OUTPUT
