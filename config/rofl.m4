#
#ROFL Subpackages
#
#AC_CONFIG_SUBDIRS([libs/rofl-common])
#AC_CONFIG_SUBDIRS([libs/rofl-datapath])

AC_MSG_CHECKING(and adding support for ROFL libraries...)

#Adding the includes
ROFL_INCLUDES="-I$XDPD_SRCDIR/libs/rofl-common/src/ -I$XDPD_SRCDIR/libs/rofl-datapath/src/ -I$XDPD_BUILDDIR/libs/rofl-common/build/src/ -I$XDPD_BUILDDIR/libs/rofl-datapath/build/src"

#Library path
ROFL_LDFLAGS="-L$XDPD_BUILDDIR/libs/rofl-common/build/src/rofl/ -L$XDPD_BUILDDIR/libs/rofl-datapath/build/src/rofl/"

CPPFLAGS="$ROFL_INCLUDES $CPPFLAGS "
LDFLAGS="$ROFL_LDFLAGS $LDFLAGS "

AC_MSG_RESULT(added)
