#
#ROFL Subpackages
#
AC_CONFIG_SUBDIRS([libs/rofl-common])
AC_CONFIG_SUBDIRS([libs/rofl-datapath])

#Adding the includes
ROFL_INCLUDES="-I$XDPD_BUILDDIR/libs/rofl-common/src/ -I$XDPD_BUILDDIR/libs/rofl-datapath/src/ -I$XDPD_SRCDIR/libs/rofl-common/src/ -I$XDPD_SRCDIR/libs/rofl-datapath/src"

#Library path
ROFL_LDFLAGS="-L$XDPD_BUILDDIR/libs/rofl-common/src/rofl/ -L$XDPD_BUILDDIR/libs/rofl-datapath/src/rofl/"

CPPFLAGS="$CPPFLAGS $ROFL_INCLUDES"
LDFLAGS="$LDFLAGS $ROFL_LDFLAGS"
