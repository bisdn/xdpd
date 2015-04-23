#Load DPDK libs in the include_ and library_path

if test "$WITH_DPDK" = "yes"; then

AC_MSG_CHECKING(and adding support for the DPDK library...)
#Adding the includes
DPDK_INCLUDES="-I$XDPD_SRCDIR/libs/dpdk/build/include"

#Library path
DPDK_LDFLAGS="-L$XDPD_SRCDIR/libs/dpdk/build/lib"

#Add them
CPPFLAGS="$DPDK_INCLUDES $CPPFLAGS"
LDFLAGS="$DPDK_LDFLAGS $LDFLAGS "

#
# Determine the OS
#
AC_CANONICAL_HOST

OS=
case $host_os in
	linux*)
	OS=linux
	;;
	*BSD*)
	OS=bsd
	;;
	*)
	#Default Case
	AC_MSG_ERROR([Your platform is not currently supported])
	;;
esac

#Compose DPDK target string
DPDK_TARGET="x86_64-native-${OS}app-${CC}"

AC_SUBST(DPDK_TARGET)
AC_MSG_RESULT([added ($DPDK_TARGET)])

fi
