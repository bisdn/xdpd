#Setups the build enviornment for xDPd drivers

#Assume relative path first
XDPD_SRCDIR="$PWD/$srcdir/../../../../"
XDPD_BUILDDIR="$PWD/../../../../"

AC_MSG_CHECKING(and setting xDPd driver enviornment...)
if test ! -d "$XDPD_SRCDIR" ; then
	#Try absolute
	XDPD_SRCDIR="$srcdir/../../../../"
	if test ! -d "$XDPD_SRCDIR" ; then
		AC_ERROR("Unable to detect srcdir!  This is a bug in the build system, please report it. To workaround it, you can try manually specifying --srcdir to configure")
	fi
fi
AC_MSG_RESULT([done])
