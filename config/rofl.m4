#
#ROFL Subpackages
#
#AC_CONFIG_SUBDIRS([libs/rofl-common])
#AC_CONFIG_SUBDIRS([libs/rofl-datapath])

#Adding the includes
ROFL_INCLUDES="-I$XDPD_SRCDIR/libs/rofl-common/src/ -I$XDPD_SRCDIR/libs/rofl-datapath/src/ -I$XDPD_SRCDIR/libs/rofl-common/build/src/ -I$XDPD_SRCDIR/libs/rofl-datapath/build/src"

#Library path
ROFL_LDFLAGS="-L$XDPD_SRCDIR/libs/rofl-common/build/src/rofl/ -L$XDPD_SRCDIR/libs/rofl-datapath/build/src/rofl/"

CPPFLAGS="$CPPFLAGS $ROFL_INCLUDES"
LDFLAGS="$LDFLAGS $ROFL_LDFLAGS"
