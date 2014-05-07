AC_INIT(xdpd - NetFPGA-10G driver, m4_esyscmd_s(cat VERSION), rofl@bisdn.de,  xdpd_netfpga10g_fm)

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

PKG_CHECK_MODULES([ROFL], [rofl >= 0.4.0], [have_rofl=yes], [have_rofl=no])
if (test "${have_rofl}" = "yes"); then
        CPPFLAGS+="$ROFL_CFLAGS"
        LIBS+="$ROFL_LIBS"
fi

#Debug
m4_include([../../../../config/debug.m4])

# Application verbosity 
m4_include([../../../../config/verbose.m4])

# Check for profiling mode
m4_include([../../../../config/profiling.m4])

# pkt classifier 
m4_include([config/pktclassifier.m4])

# Ensure rofl-pipeline platform functions are NOT inlined 
m4_include([../../../../config/no_pipeline_inline.m4])


AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/hal-imp/Makefile
	src/hal-imp/openflow/openflow1x/Makefile
	src/io/Makefile
	src/io/packet_classifiers/Makefile
	src/io/packet_classifiers/c_pktclassifier/Makefile
	src/netfpga/Makefile
	src/pipeline-imp/Makefile
	src/util/Makefile
])

AC_OUTPUT
